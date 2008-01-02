/** @file

Copyright (c) 2007, Intel Corporation
All rights reserved. This program and the accompanying materials
are licensed and made available under the terms and conditions of the BSD License
which accompanies this distribution.  The full text of the license may be found at
http://opensource.org/licenses/bsd-license.php

THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

  Module Name:

    UsbEnumer.c

  Abstract:

    Usb bus enumeration support

  Revision History


**/

#include "UsbBus.h"


/**
  Return the endpoint descriptor in this interface

  @param  UsbIf                 The interface to search in
  @param  EpAddr                The address of the endpoint to return

  @return The endpoint descriptor or NULL

**/
USB_ENDPOINT_DESC *
UsbGetEndpointDesc (
  IN USB_INTERFACE        *UsbIf,
  IN UINT8                EpAddr
  )
{
  USB_ENDPOINT_DESC       *EpDesc;
  UINTN                   Index;

  for (Index = 0; Index < UsbIf->IfSetting->Desc.NumEndpoints; Index++) {
    EpDesc = UsbIf->IfSetting->Endpoints[Index];

    if (EpDesc->Desc.EndpointAddress == EpAddr) {
      return EpDesc;
    }
  }

  return NULL;
}


/**
  Free the resource used by USB interface

  @param  UsbIf                 The USB interface to free

  @return None

**/
STATIC
VOID
UsbFreeInterface (
  IN USB_INTERFACE        *UsbIf
  )
{
  UsbCloseHostProtoByChild (UsbIf->Device->Bus, UsbIf->Handle);

  gBS->UninstallMultipleProtocolInterfaces (
         UsbIf->Handle,
         &gEfiDevicePathProtocolGuid,
         UsbIf->DevicePath,
         &gEfiUsbIoProtocolGuid,
         &UsbIf->UsbIo,
         NULL
         );

  if (UsbIf->DevicePath != NULL) {
    gBS->FreePool (UsbIf->DevicePath);
  }

  gBS->FreePool (UsbIf);
}


/**
  Create an interface for the descriptor IfDesc. Each
  device's configuration can have several interfaces.

  @param  Device                The device has the interface descriptor
  @param  IfDesc                The interface descriptor

  @return The created USB interface for the descriptor, or NULL.

**/
STATIC
USB_INTERFACE *
UsbCreateInterface (
  IN USB_DEVICE           *Device,
  IN USB_INTERFACE_DESC   *IfDesc
  )
{
  USB_DEVICE_PATH         UsbNode;
  USB_INTERFACE           *UsbIf;
  USB_INTERFACE           *HubIf;
  EFI_STATUS              Status;

  UsbIf = AllocateZeroPool (sizeof (USB_INTERFACE));

  if (UsbIf == NULL) {
    return NULL;
  }

  UsbIf->Signature  = USB_INTERFACE_SIGNATURE;
  UsbIf->Device     = Device;
  UsbIf->IfDesc     = IfDesc;
  UsbIf->IfSetting  = IfDesc->Settings[IfDesc->ActiveIndex];

  CopyMem (
    &(UsbIf->UsbIo),
    &mUsbIoProtocol,
    sizeof (EFI_USB_IO_PROTOCOL)
    );

  //
  // Install protocols for USBIO and device path
  //
  UsbNode.Header.Type       = MESSAGING_DEVICE_PATH;
  UsbNode.Header.SubType    = MSG_USB_DP;
  UsbNode.ParentPortNumber  = Device->ParentPort;
  UsbNode.InterfaceNumber   = UsbIf->IfSetting->Desc.InterfaceNumber;

  SetDevicePathNodeLength (&UsbNode.Header, sizeof (UsbNode));

  HubIf = Device->ParentIf;
  ASSERT (HubIf != NULL);

  UsbIf->DevicePath = AppendDevicePathNode (HubIf->DevicePath, &UsbNode.Header);

  if (UsbIf->DevicePath == NULL) {
    DEBUG ((EFI_D_ERROR, "UsbCreateInterface: failed to create device path\n"));

    Status = EFI_OUT_OF_RESOURCES;
    goto ON_ERROR;
  }

  Status = gBS->InstallMultipleProtocolInterfaces (
                  &UsbIf->Handle,
                  &gEfiDevicePathProtocolGuid,
                  UsbIf->DevicePath,
                  &gEfiUsbIoProtocolGuid,
                  &UsbIf->UsbIo,
                  NULL
                  );

  if (EFI_ERROR (Status)) {
    DEBUG ((EFI_D_ERROR, "UsbCreateInterface: failed to install UsbIo - %r\n", Status));
    goto ON_ERROR;
  }

  //
  // Open USB Host Controller Protocol by Child
  //
  Status = UsbOpenHostProtoByChild (Device->Bus, UsbIf->Handle);

  if (EFI_ERROR (Status)) {
    gBS->UninstallMultipleProtocolInterfaces (
           &UsbIf->Handle,
           &gEfiDevicePathProtocolGuid,
           UsbIf->DevicePath,
           &gEfiUsbIoProtocolGuid,
           &UsbIf->UsbIo,
           NULL
           );

    DEBUG ((EFI_D_ERROR, "UsbCreateInterface: failed to open host for child - %r\n", Status));
    goto ON_ERROR;
  }

  return UsbIf;

ON_ERROR:
  if (UsbIf->DevicePath) {
    gBS->FreePool (UsbIf->DevicePath);
  }

  gBS->FreePool (UsbIf);
  return NULL;
}


/**
  Free the resource used by this USB device

  @param  Device                The USB device to free

  @return None

**/
STATIC
VOID
UsbFreeDevice (
  IN USB_DEVICE           *Device
  )
{
  if (Device->DevDesc != NULL) {
    UsbFreeDevDesc (Device->DevDesc);
  }

  gBS->FreePool (Device);
}


/**
  Create a device which is on the parent's ParentPort port.

  @param  ParentIf              The parent HUB interface
  @param  ParentPort            The port on the HUB this device is connected to

  @return Created USB device

**/
STATIC
USB_DEVICE *
UsbCreateDevice (
  IN USB_INTERFACE        *ParentIf,
  IN UINT8                ParentPort
  )
{
  USB_DEVICE              *Device;

  ASSERT (ParentIf != NULL);

  Device = AllocateZeroPool (sizeof (USB_DEVICE));

  if (Device == NULL) {
    return NULL;
  }

  Device->Bus         = ParentIf->Device->Bus;
  Device->MaxPacket0  = 8;
  Device->ParentAddr  = ParentIf->Device->Address;
  Device->ParentIf    = ParentIf;
  Device->ParentPort  = ParentPort;
  return Device;
}


/**
  Connect the USB interface with its driver. EFI USB bus will
  create a USB interface for each seperate interface descriptor.

  @param  UsbIf                 The interface to connect driver to

  @return EFI_SUCCESS : Interface is managed by some driver
  @return Others      : Failed to locate a driver for this interface

**/
STATIC
EFI_STATUS
UsbConnectDriver (
  IN USB_INTERFACE        *UsbIf
  )
{
  EFI_STATUS              Status;
  EFI_TPL                 OldTpl;

  Status = EFI_SUCCESS;

  //
  // Hub is maintained by the USB bus driver. Otherwise try to
  // connect drivers with this interface
  //
  if (UsbIsHubInterface (UsbIf)) {
    DEBUG ((EFI_D_INFO, "UsbConnectDriver: found a hub device\n"));
    Status = mUsbHubApi.Init (UsbIf);

  } else {
    //
    // This function is called in both UsbIoControlTransfer and
    // the timer callback in hub enumeration. So, at least it is
    // called at TPL_CALLBACK. Some driver sitting on USB has
    // twisted TPL used. It should be no problem for us to connect
    // or disconnect at CALLBACK.
    //
    
    //
    // Only recursively wanted usb child device
    //
    if (UsbBusIsWantedUsbIO (UsbIf->Device->Bus, UsbIf)) {
      OldTpl            = UsbGetCurrentTpl ();
      DEBUG ((EFI_D_INFO, "UsbConnectDriver: TPL before connect is %d\n", OldTpl));

      gBS->RestoreTPL (TPL_CALLBACK);

      Status            = gBS->ConnectController (UsbIf->Handle, NULL, NULL, TRUE);
      UsbIf->IsManaged  = (BOOLEAN)!EFI_ERROR (Status);

      DEBUG ((EFI_D_INFO, "UsbConnectDriver: TPL after connect is %d\n", UsbGetCurrentTpl()));
      ASSERT (UsbGetCurrentTpl () == TPL_CALLBACK);

      gBS->RaiseTPL (OldTpl);
    }
  }

  return Status;
}


/**
  Select an alternate setting for the interface.
  Each interface can have several mutually exclusive
  settings. Only one setting is active. It will
  also reset its endpoints' toggle to zero.

  @param  IfDesc                The interface descriptor to set
  @param  Alternate             The alternate setting number to locate

  @retval EFI_NOT_FOUND         There is no setting with this alternate index
  @retval EFI_SUCCESS           The interface is set to Alternate setting.

**/
EFI_STATUS
UsbSelectSetting (
  IN USB_INTERFACE_DESC   *IfDesc,
  IN UINT8                Alternate
  )
{
  USB_INTERFACE_SETTING   *Setting;
  UINT8                   Index;

  //
  // Locate the active alternate setting
  //
  Setting = NULL;

  for (Index = 0; Index < IfDesc->NumOfSetting; Index++) {
    Setting = IfDesc->Settings[Index];

    if (Setting->Desc.AlternateSetting == Alternate) {
      break;
    }
  }

  if (Index == IfDesc->NumOfSetting) {
    return EFI_NOT_FOUND;
  }

  IfDesc->ActiveIndex = Index;

  DEBUG ((EFI_D_INFO, "UsbSelectSetting: setting %d selected for interface %d\n",
              Alternate, Setting->Desc.InterfaceNumber));

  //
  // Reset the endpoint toggle to zero
  //
  for (Index = 0; Index < Setting->Desc.NumEndpoints; Index++) {
    Setting->Endpoints[Index]->Toggle = 0;
  }

  return EFI_SUCCESS;
}


/**
  Select a new configuration for the device. Each
  device may support several configurations.

  @param  Device                The device to select configuration
  @param  ConfigValue           The index of the configuration ( != 0)

  @retval EFI_NOT_FOUND         There is no configuration with the index
  @retval EFI_OUT_OF_RESOURCES  Failed to allocate resource
  @retval EFI_SUCCESS           The configuration is selected.

**/
EFI_STATUS
UsbSelectConfig (
  IN USB_DEVICE           *Device,
  IN UINT8                ConfigValue
  )
{
  USB_DEVICE_DESC         *DevDesc;
  USB_CONFIG_DESC         *ConfigDesc;
  USB_INTERFACE_DESC      *IfDesc;
  USB_INTERFACE           *UsbIf;
  EFI_STATUS              Status;
  UINT8                   Index;

  //
  // Locate the active config, then set the device's pointer
  //
  DevDesc     = Device->DevDesc;
  ConfigDesc  = NULL;

  for (Index = 0; Index < DevDesc->Desc.NumConfigurations; Index++) {
    ConfigDesc = DevDesc->Configs[Index];

    if (ConfigDesc->Desc.ConfigurationValue == ConfigValue) {
      break;
    }
  }

  if (Index == DevDesc->Desc.NumConfigurations) {
    return EFI_NOT_FOUND;
  }

  Device->ActiveConfig = ConfigDesc;

  DEBUG ((EFI_D_INFO, "UsbSelectConfig: config %d selected for device %d\n",
              ConfigValue, Device->Address));

  //
  // Create interfaces for each USB interface descriptor.
  //
  for (Index = 0; Index < ConfigDesc->Desc.NumInterfaces; Index++) {
    //
    // First select the default interface setting, and reset
    // the endpoint toggles to zero for its endpoints.
    //
    IfDesc = ConfigDesc->Interfaces[Index];
    UsbSelectSetting (IfDesc, IfDesc->Settings[0]->Desc.AlternateSetting);

    //
    // Create a USB_INTERFACE and install USB_IO and other protocols
    //
    UsbIf = UsbCreateInterface (Device, ConfigDesc->Interfaces[Index]);

    if (UsbIf == NULL) {
      return EFI_OUT_OF_RESOURCES;
    }

    Device->Interfaces[Index] = UsbIf;

    //
    // Connect the device to drivers, if it failed, ignore
    // the error. Don't let the unsupported interfaces to block
    // the supported interfaces.
    //
    Status = UsbConnectDriver (UsbIf);

    if (EFI_ERROR (Status)) {
      DEBUG ((EFI_D_ERROR, "UsbSelectConfig: failed to connect driver %r, ignored\n", Status));
    }
  }

  Device->NumOfInterface = Index;

  return EFI_SUCCESS;
}



/**
  Disconnect the USB interface with its driver.

  @param  UsbIf                 The interface to disconnect driver from

  @return None

**/
STATIC
VOID
UsbDisconnectDriver (
  IN USB_INTERFACE        *UsbIf
  )
{
  EFI_TPL                 OldTpl;

  //
  // Release the hub if it's a hub controller, otherwise
  // disconnect the driver if it is managed by other drivers.
  //
  if (UsbIf->IsHub) {
    UsbIf->HubApi->Release (UsbIf);

  } else if (UsbIf->IsManaged) {
    //
    // This function is called in both UsbIoControlTransfer and
    // the timer callback in hub enumeration. So, at least it is
    // called at TPL_CALLBACK. Some driver sitting on USB has
    // twisted TPL used. It should be no problem for us to connect
    // or disconnect at CALLBACK.
    //
    OldTpl           = UsbGetCurrentTpl ();
    DEBUG ((EFI_D_INFO, "UsbDisconnectDriver: old TPL is %d\n", OldTpl));

    gBS->RestoreTPL (TPL_CALLBACK);

    gBS->DisconnectController (UsbIf->Handle, NULL, NULL);
    UsbIf->IsManaged = FALSE;

    DEBUG (( EFI_D_INFO, "UsbDisconnectDriver: TPL after disconnect is %d\n", UsbGetCurrentTpl()));
    ASSERT (UsbGetCurrentTpl () == TPL_CALLBACK);

    gBS->RaiseTPL (OldTpl);
  }
}



/**
  Remove the current device configuration

  @param  Device                The USB device to remove configuration from

  @return None

**/
VOID
UsbRemoveConfig (
  IN USB_DEVICE           *Device
  )
{
  USB_INTERFACE           *UsbIf;
  UINTN                   Index;

  //
  // Remove each interface of the device
  //
  for (Index = 0; Index < Device->NumOfInterface; Index++) {
    UsbIf = Device->Interfaces[Index];

    if (UsbIf == NULL) {
      continue;
    }

    UsbDisconnectDriver (UsbIf);
    UsbFreeInterface (UsbIf);
    Device->Interfaces[Index] = NULL;
  }

  Device->ActiveConfig    = NULL;
  Device->NumOfInterface  = 0;
}



/**
  Remove the device and all its children from the bus.

  @param  Device                The device to remove

  @retval EFI_SUCCESS           The device is removed

**/
EFI_STATUS
UsbRemoveDevice (
  IN USB_DEVICE           *Device
  )
{
  USB_BUS                 *Bus;
  USB_DEVICE              *Child;
  EFI_STATUS              Status;
  UINT8                   Index;

  Bus = Device->Bus;

  //
  // Remove all the devices on its downstream ports. Search from devices[1].
  // Devices[0] is the root hub.
  //
  for (Index = 1; Index < USB_MAX_DEVICES; Index++) {
    Child = Bus->Devices[Index];

    if ((Child == NULL) || (Child->ParentAddr != Device->Address)) {
      continue;
    }

    Status = UsbRemoveDevice (Child);

    if (EFI_ERROR (Status)) {
      DEBUG ((EFI_D_ERROR, "UsbRemoveDevice: failed to remove child, ignore error\n"));
      Bus->Devices[Index] = NULL;
    }
  }

  UsbRemoveConfig (Device);

  DEBUG (( EFI_D_INFO, "UsbRemoveDevice: device %d removed\n", Device->Address));

  Bus->Devices[Device->Address] = NULL;
  UsbFreeDevice (Device);

  return EFI_SUCCESS;
}


/**
  Find the child device on the hub's port

  @param  HubIf                 The hub interface
  @param  Port                  The port of the hub this child is connected to

  @return The device on the hub's port, or NULL if there is none

**/
STATIC
USB_DEVICE *
UsbFindChild (
  IN USB_INTERFACE        *HubIf,
  IN UINT8                Port
  )
{
  USB_DEVICE              *Device;
  USB_BUS                 *Bus;
  UINTN                   Index;

  Bus = HubIf->Device->Bus;

  //
  // Start checking from device 1, device 0 is the root hub
  //
  for (Index = 1; Index < USB_MAX_DEVICES; Index++) {
    Device = Bus->Devices[Index];

    if ((Device != NULL) && (Device->ParentAddr == HubIf->Device->Address) &&
        (Device->ParentPort == Port)) {

      return Device;
    }
  }

  return NULL;
}



/**
  Enumerate and configure the new device on the port of this HUB interface.

  @param  HubIf                 The HUB that has the device connected
  @param  Port                  The port index of the hub (started with zero)

  @retval EFI_SUCCESS           The device is enumerated (added or removed)
  @retval EFI_OUT_OF_RESOURCES  Failed to allocate resource for the device
  @retval Others                Failed to enumerate the device

**/
STATIC
EFI_STATUS
UsbEnumerateNewDev (
  IN USB_INTERFACE        *HubIf,
  IN UINT8                Port
  )
{
  USB_BUS                 *Bus;
  USB_HUB_API             *HubApi;
  USB_DEVICE              *Child;
  USB_DEVICE              *Parent;
  EFI_USB_PORT_STATUS     PortState;
  UINT8                   Address;
  UINT8                   Config;
  EFI_STATUS              Status;

  Address = USB_MAX_DEVICES;
  Parent  = HubIf->Device;
  Bus     = Parent->Bus;
  HubApi  = HubIf->HubApi;
  
  gBS->Stall (USB_WAIT_PORT_STABLE_STALL);
  
  //
  // Hub resets the device for at least 10 milliseconds.
  // Host learns device speed. If device is of low/full speed
  // and the hub is a EHCI root hub, ResetPort will release
  // the device to its companion UHCI and return an error.
  //
  Status = HubApi->ResetPort (HubIf, Port);

  if (EFI_ERROR (Status)) {
    DEBUG ((EFI_D_ERROR, "UsbEnumerateNewDev: failed to reset port %d - %r\n", Port, Status));

    return Status;
  }

  DEBUG (( EFI_D_INFO, "UsbEnumerateNewDev: hub port %d is reset\n", Port));

  Child = UsbCreateDevice (HubIf, Port);

  if (Child == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }

  //
  // OK, now identify the device speed. After reset, hub
  // fully knows the actual device speed.
  //
  Status = HubApi->GetPortStatus (HubIf, Port, &PortState);

  if (EFI_ERROR (Status)) {
    DEBUG ((EFI_D_ERROR, "UsbEnumerateNewDev: failed to get speed of port %d\n", Port));
    goto ON_ERROR;
  }

  if (USB_BIT_IS_SET (PortState.PortStatus, USB_PORT_STAT_LOW_SPEED)) {
    Child->Speed = EFI_USB_SPEED_LOW;

  } else if (USB_BIT_IS_SET (PortState.PortStatus, USB_PORT_STAT_HIGH_SPEED)) {
    Child->Speed = EFI_USB_SPEED_HIGH;

  } else {
    Child->Speed = EFI_USB_SPEED_FULL;
  }

  DEBUG (( EFI_D_INFO, "UsbEnumerateNewDev: device is of %d speed\n", Child->Speed));

  if (Child->Speed != EFI_USB_SPEED_HIGH) {
    //
    // If the child isn't a high speed device, it is necessary to
    // set the transaction translator. This is quite simple:
    //  1. if parent is of high speed, then parent is our translator
    //  2. otherwise use parent's translator.
    //
    if (Parent->Speed == EFI_USB_SPEED_HIGH) {
      Child->Translator.TranslatorHubAddress  = Parent->Address;
      Child->Translator.TranslatorPortNumber  = Port;

    } else {
      Child->Translator = Parent->Translator;
    }

    DEBUG (( EFI_D_INFO, "UsbEnumerateNewDev: device uses translator (%d, %d)\n",
                Child->Translator.TranslatorHubAddress,
                Child->Translator.TranslatorPortNumber));
  }

  //
  // After port is reset, hub establishes a signal path between
  // the device and host (DEFALUT state). Device��s registers are
  // reset, use default address 0 (host enumerates one device at
  // a time) , and ready to respond to control transfer at EP 0.
  //

  //
  // Host sends a Get_Descriptor request to learn the max packet
  // size of default pipe (only part of the device��s descriptor).
  //
  Status = UsbGetMaxPacketSize0 (Child);

  if (EFI_ERROR (Status)) {
    DEBUG ((EFI_D_ERROR, "UsbEnumerateNewDev: failed to get max packet for EP 0 - %r\n", Status));
    goto ON_ERROR;
  }

  DEBUG (( EFI_D_INFO, "UsbEnumerateNewDev: max packet size for EP 0 is %d\n", Child->MaxPacket0));

  //
  // Host assigns an address to the device. Device completes the
  // status stage with default address, then switches to new address.
  // ADDRESS state. Address zero is reserved for root hub.
  //
  for (Address = 1; Address < USB_MAX_DEVICES; Address++) {
    if (Bus->Devices[Address] == NULL) {
      break;
    }
  }

  if (Address == USB_MAX_DEVICES) {
    DEBUG ((EFI_D_ERROR, "UsbEnumerateNewDev: address pool is full for port %d\n", Port));

    Status = EFI_ACCESS_DENIED;
    goto ON_ERROR;
  }

  Bus->Devices[Address] = Child;
  Status                = UsbSetAddress (Child, Address);
  Child->Address        = Address;

  if (EFI_ERROR (Status)) {
    DEBUG ((EFI_D_ERROR, "UsbEnumerateNewDev: failed to set device address - %r\n", Status));
    goto ON_ERROR;
  }
  
  gBS->Stall (USB_SET_DEVICE_ADDRESS_STALL);

  DEBUG ((EFI_D_INFO, "UsbEnumerateNewDev: device is now ADDRESSED at %d\n", Address));

  //
  // Host learns about the device��s abilities by requesting device's
  // entire descriptions.
  //
  Status = UsbBuildDescTable (Child);

  if (EFI_ERROR (Status)) {
    DEBUG ((EFI_D_ERROR, "UsbEnumerateNewDev: failed to build descriptor table - %r\n", Status));
    goto ON_ERROR;
  }

  //
  // Select a default configuration: UEFI must set the configuration
  // before the driver can connect to the device.
  //
  Config = Child->DevDesc->Configs[0]->Desc.ConfigurationValue;
  Status = UsbSetConfig (Child, Config);

  if (EFI_ERROR (Status)) {
    DEBUG ((EFI_D_ERROR, "UsbEnumerateNewDev: failed to set configure %d - %r\n", Config, Status));
    goto ON_ERROR;
  }

  DEBUG (( EFI_D_INFO, "UsbEnumerateNewDev: device %d is now in CONFIGED state\n", Address));

  //
  // Host assigns and loads a device driver.
  //
  Status = UsbSelectConfig (Child, Config);

  if (EFI_ERROR (Status)) {
    DEBUG ((EFI_D_ERROR, "UsbEnumerateNewDev: failed to create interfaces - %r\n", Status));
    goto ON_ERROR;
  }

  return EFI_SUCCESS;

ON_ERROR:
  if (Address != USB_MAX_DEVICES) {
    Bus->Devices[Address] = NULL;
  }

  if (Child != NULL) {
    UsbFreeDevice (Child);
  }

  return Status;
}



/**
  Process the events on the port.

  @param  HubIf                 The HUB that has the device connected
  @param  Port                  The port index of the hub (started with zero)

  @retval EFI_SUCCESS           The device is enumerated (added or removed)
  @retval EFI_OUT_OF_RESOURCES  Failed to allocate resource for the device
  @retval Others                Failed to enumerate the device

**/
STATIC
EFI_STATUS
UsbEnumeratePort (
  IN USB_INTERFACE        *HubIf,
  IN UINT8                Port
  )
{
  USB_HUB_API             *HubApi;
  USB_DEVICE              *Child;
  EFI_USB_PORT_STATUS     PortState;
  EFI_STATUS              Status;

  Child   = NULL;
  HubApi  = HubIf->HubApi;

  //
  // Host learns of the new device by polling the hub for port changes.
  //
  Status = HubApi->GetPortStatus (HubIf, Port, &PortState);

  if (EFI_ERROR (Status)) {
    DEBUG ((EFI_D_ERROR, "UsbEnumeratePort: failed to get state of port %d\n", Port));
    return Status;
  }

  if (PortState.PortChangeStatus == 0) {
    return EFI_SUCCESS;
  }

  DEBUG (( EFI_D_INFO, "UsbEnumeratePort: port %d state - %x, change - %x\n",
              Port, PortState.PortStatus, PortState.PortChangeStatus));

  //
  // This driver only process two kinds of events now: over current and
  // connect/disconnect. Other three events are: ENABLE, SUSPEND, RESET.
  // ENABLE/RESET is used to reset port. SUSPEND isn't supported.
  //
  
  if (USB_BIT_IS_SET (PortState.PortChangeStatus, USB_PORT_STAT_C_OVERCURRENT)) {     

    if (USB_BIT_IS_SET (PortState.PortStatus, USB_PORT_STAT_OVERCURRENT)) {
      //
      // Case1:
      //   Both OverCurrent and OverCurrentChange set, means over current occurs, 
      //   which probably is caused by short circuit. It has to wait system hardware
      //   to perform recovery.
      //
      DEBUG (( EFI_D_ERROR, "UsbEnumeratePort: Critical Over Current\n", Port));
      return EFI_DEVICE_ERROR;
      
    } 
    //
    // Case2:
    //   Only OverCurrentChange set, means system has been recoveried from 
    //   over current. As a result, all ports are nearly power-off, so
    //   it's necessary to detach and enumerate all ports again. 
    //
    DEBUG (( EFI_D_ERROR, "UsbEnumeratePort: 2.0 device Recovery Over Current\n", Port)); 
  }

  if (USB_BIT_IS_SET (PortState.PortChangeStatus, USB_PORT_STAT_C_ENABLE)) {  
    //
    // Case3:
    //   1.1 roothub port reg doesn't reflect over-current state, while its counterpart
    //   on 2.0 roothub does. When over-current has influence on 1.1 device, the port 
    //   would be disabled, so it's also necessary to detach and enumerate again.
    //
    DEBUG (( EFI_D_ERROR, "UsbEnumeratePort: 1.1 device Recovery Over Current\n", Port));
  }
  
  if (USB_BIT_IS_SET (PortState.PortChangeStatus, USB_PORT_STAT_C_CONNECTION)) {
    //
    // Case4:
    //   Device connected or disconnected normally. 
    //
    DEBUG ((EFI_D_ERROR, "UsbEnumeratePort: Device Connect/Discount Normally\n", Port));
  }

  // 
  // Following as the above cases, it's safety to remove and create again.
  //
  Child = UsbFindChild (HubIf, Port);
  
  if (Child != NULL) {
    DEBUG (( EFI_D_INFO, "UsbEnumeratePort: device at port %d removed from system\n", Port));
    UsbRemoveDevice (Child);
  }
  
  if (USB_BIT_IS_SET (PortState.PortStatus, USB_PORT_STAT_CONNECTION)) {
    //
    // Now, new device connected, enumerate and configure the device 
    //
    DEBUG (( EFI_D_INFO, "UsbEnumeratePort: new device connected at port %d\n", Port));
    Status = UsbEnumerateNewDev (HubIf, Port);
  
  } else {
    DEBUG (( EFI_D_INFO, "UsbEnumeratePort: device disconnected event on port %d\n", Port));
  }
  
  HubApi->ClearPortChange (HubIf, Port);
  return Status;
}


/**
  Enumerate all the changed hub ports

  @param  Event                 The event that is triggered
  @param  Context               The context to the event

  @return None

**/
VOID
UsbHubEnumeration (
  IN EFI_EVENT            Event,
  IN VOID                 *Context
  )
{
  USB_INTERFACE           *HubIf;
  UINT8                   Byte;
  UINT8                   Bit;
  UINT8                   Index;

  ASSERT (Context);

  HubIf = (USB_INTERFACE *) Context;

  if (HubIf->ChangeMap == NULL) {
    return ;
  }

  //
  // HUB starts its port index with 1.
  //
  Byte  = 0;
  Bit   = 1;

  for (Index = 0; Index < HubIf->NumOfPort; Index++) {
    if (USB_BIT_IS_SET (HubIf->ChangeMap[Byte], USB_BIT (Bit))) {
      UsbEnumeratePort (HubIf, Index);
    }

    USB_NEXT_BIT (Byte, Bit);
  }

  UsbHubAckHubStatus (HubIf->Device);

  gBS->FreePool (HubIf->ChangeMap);
  HubIf->ChangeMap = NULL;
  return ;
}


/**
  Enumerate all the changed hub ports

  @param  Event                 The event that is triggered
  @param  Context               The context to the event

  @return None

**/
VOID
EFIAPI
UsbRootHubEnumeration (
  IN EFI_EVENT            Event,
  IN VOID                 *Context
  )
{
  USB_INTERFACE           *RootHub;
  UINT8                   Index;

  RootHub = (USB_INTERFACE *) Context;

  for (Index = 0; Index < RootHub->NumOfPort; Index++) {
    UsbEnumeratePort (RootHub, Index);
  }
}
