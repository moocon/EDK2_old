/** @file

Copyright (c) 2004 - 2007, Intel Corporation
All rights reserved. This program and the accompanying materials
are licensed and made available under the terms and conditions of the BSD License
which accompanies this distribution.  The full text of the license may be found at
http://opensource.org/licenses/bsd-license.php

THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

  Module Name:

    UsbMouse.c

  Abstract:


**/

#include "UsbMouse.h"

#include <Library/DebugLib.h>
#include <IndustryStandard/Usb.h>


//
// Driver Consumed Protocol Prototypes
//
//@MT:#include EFI_PROTOCOL_DEFINITION (DriverBinding)
//@MT:#include EFI_PROTOCOL_DEFINITION (UsbIo)

//
// Driver Produced Protocol Prototypes
//
//@MT:#include EFI_PROTOCOL_DEFINITION (SimplePointer)

//@MT:#include "UsbDxeLib.h"
//@MT:#include "hid.h"
#include "usbmouse.h"
#include "mousehid.h"

//
// Prototypes
// Driver model protocol interface
//
EFI_STATUS
EFIAPI
USBMouseDriverBindingEntryPoint (
  IN EFI_HANDLE           ImageHandle,
  IN EFI_SYSTEM_TABLE     *SystemTable
  );

EFI_STATUS
EFIAPI
USBMouseDriverBindingSupported (
  IN EFI_DRIVER_BINDING_PROTOCOL    *This,
  IN EFI_HANDLE                     Controller,
  IN EFI_DEVICE_PATH_PROTOCOL       *RemainingDevicePath
  );

EFI_STATUS
EFIAPI
USBMouseDriverBindingStart (
  IN EFI_DRIVER_BINDING_PROTOCOL    *This,
  IN EFI_HANDLE                     Controller,
  IN EFI_DEVICE_PATH_PROTOCOL       *RemainingDevicePath
  );

EFI_STATUS
EFIAPI
USBMouseDriverBindingStop (
  IN  EFI_DRIVER_BINDING_PROTOCOL   *This,
  IN  EFI_HANDLE                    Controller,
  IN  UINTN                         NumberOfChildren,
  IN  EFI_HANDLE                    *ChildHandleBuffer
  );

EFI_GUID  gEfiUsbMouseDriverGuid = {
  0x290156b5, 0x6a05, 0x4ac0, 0xb8, 0x0, 0x51, 0x27, 0x55, 0xad, 0x14, 0x29
};

EFI_DRIVER_BINDING_PROTOCOL gUsbMouseDriverBinding = {
  USBMouseDriverBindingSupported,
  USBMouseDriverBindingStart,
  USBMouseDriverBindingStop,
  0xa,
  NULL,
  NULL
};

//
// helper functions
//
STATIC
BOOLEAN
IsUsbMouse (
  IN  EFI_USB_IO_PROTOCOL     *UsbIo
  );

STATIC
EFI_STATUS
InitializeUsbMouseDevice (
  IN  USB_MOUSE_DEV           *UsbMouseDev
  );

STATIC
VOID
EFIAPI
UsbMouseWaitForInput (
  IN  EFI_EVENT               Event,
  IN  VOID                    *Context
  );

//
// Mouse interrupt handler
//
STATIC
EFI_STATUS
EFIAPI
OnMouseInterruptComplete (
  IN  VOID        *Data,
  IN  UINTN       DataLength,
  IN  VOID        *Context,
  IN  UINT32      Result
  );

//
// Mouse Protocol
//
STATIC
EFI_STATUS
EFIAPI
GetMouseState (
  IN   EFI_SIMPLE_POINTER_PROTOCOL  *This,
  OUT  EFI_SIMPLE_POINTER_STATE     *MouseState
  );

STATIC
EFI_STATUS
EFIAPI
UsbMouseReset (
  IN EFI_SIMPLE_POINTER_PROTOCOL    *This,
  IN BOOLEAN                        ExtendedVerification
  );

//
// Driver start here
//
//@MT: EFI_DRIVER_ENTRY_POINT (USBMouseDriverBindingEntryPoint)

EFI_STATUS
EFIAPI
USBMouseDriverBindingEntryPoint (
  IN EFI_HANDLE           ImageHandle,
  IN EFI_SYSTEM_TABLE     *SystemTable
  )
/*++

  Routine Description:
    Entry point for EFI drivers.

  Arguments:
   ImageHandle - EFI_HANDLE
   SystemTable - EFI_SYSTEM_TABLE
  Returns:
    EFI_SUCCESS
    others

--*/
{
  return EfiLibInstallAllDriverProtocols (
          ImageHandle,
          SystemTable,
          &gUsbMouseDriverBinding,
          ImageHandle,
          &gUsbMouseComponentName,
          NULL,
          NULL
          );
}


/**
  Test to see if this driver supports ControllerHandle. Any ControllerHandle
  that has UsbHcProtocol installed will be supported.

  @param  This                  Protocol instance pointer.
  @param  Controller            Handle of device to test
  @param  RemainingDevicePath   Not used

  @retval EFI_SUCCESS           This driver supports this device.
  @retval EFI_UNSUPPORTED       This driver does not support this device.

**/
EFI_STATUS
EFIAPI
USBMouseDriverBindingSupported (
  IN EFI_DRIVER_BINDING_PROTOCOL    *This,
  IN EFI_HANDLE                     Controller,
  IN EFI_DEVICE_PATH_PROTOCOL       *RemainingDevicePath
  )
{
  EFI_STATUS          OpenStatus;
  EFI_USB_IO_PROTOCOL *UsbIo;
  EFI_STATUS          Status;

  OpenStatus = gBS->OpenProtocol (
                      Controller,
                      &gEfiUsbIoProtocolGuid,
                      &UsbIo,
                      This->DriverBindingHandle,
                      Controller,
                      EFI_OPEN_PROTOCOL_BY_DRIVER
                      );
  if (EFI_ERROR (OpenStatus) && (OpenStatus != EFI_ALREADY_STARTED)) {
    return EFI_UNSUPPORTED;
  }

  if (OpenStatus == EFI_ALREADY_STARTED) {
    return EFI_ALREADY_STARTED;
  }

  //
  // Use the USB I/O protocol interface to see the Controller is
  // the Mouse controller that can be managed by this driver.
  //
  Status = EFI_SUCCESS;
  if (!IsUsbMouse (UsbIo)) {
    Status = EFI_UNSUPPORTED;
  }

  gBS->CloseProtocol (
        Controller,
        &gEfiUsbIoProtocolGuid,
        This->DriverBindingHandle,
        Controller
        );
  return Status;
}


/**
  Starting the Usb Bus Driver

  @param  This                  Protocol instance pointer.
  @param  Controller            Handle of device to test
  @param  RemainingDevicePath   Not used

  @retval EFI_SUCCESS           This driver supports this device.
  @retval EFI_UNSUPPORTED       This driver does not support this device.
  @retval EFI_DEVICE_ERROR      This driver cannot be started due to device Error
                                EFI_OUT_OF_RESOURCES- Can't allocate memory
                                resources
  @retval EFI_ALREADY_STARTED   Thios driver has been started

**/
EFI_STATUS
EFIAPI
USBMouseDriverBindingStart (
  IN EFI_DRIVER_BINDING_PROTOCOL    *This,
  IN EFI_HANDLE                     Controller,
  IN EFI_DEVICE_PATH_PROTOCOL       *RemainingDevicePath
  )
{
  EFI_STATUS                  Status;
  EFI_USB_IO_PROTOCOL         *UsbIo;
  EFI_USB_ENDPOINT_DESCRIPTOR *EndpointDesc;
  USB_MOUSE_DEV               *UsbMouseDevice;
  UINT8                       EndpointNumber;
  UINT8                       Index;
  UINT8                       EndpointAddr;
  UINT8                       PollingInterval;
  UINT8                       PacketSize;

  UsbMouseDevice  = NULL;
  Status          = EFI_SUCCESS;

  Status = gBS->OpenProtocol (
                  Controller,
                  &gEfiUsbIoProtocolGuid,
                  &UsbIo,
                  This->DriverBindingHandle,
                  Controller,
                  EFI_OPEN_PROTOCOL_BY_DRIVER
                  );
  if (EFI_ERROR (Status)) {
    goto ErrorExit;
  }

  UsbMouseDevice = AllocateZeroPool (sizeof (USB_MOUSE_DEV));
  if (UsbMouseDevice == NULL) {
    Status = EFI_OUT_OF_RESOURCES;
    goto ErrorExit;
  }

  UsbMouseDevice->UsbIo               = UsbIo;

  UsbMouseDevice->Signature           = USB_MOUSE_DEV_SIGNATURE;

  UsbMouseDevice->InterfaceDescriptor = AllocatePool (sizeof (EFI_USB_INTERFACE_DESCRIPTOR));
  if (UsbMouseDevice->InterfaceDescriptor == NULL) {
    Status = EFI_OUT_OF_RESOURCES;
    goto ErrorExit;
  }

  EndpointDesc = AllocatePool (sizeof (EFI_USB_ENDPOINT_DESCRIPTOR));
  if (EndpointDesc == NULL) {
    Status = EFI_OUT_OF_RESOURCES;
    goto ErrorExit;
  }
  //
  // Get the Device Path Protocol on Controller's handle
  //
  Status = gBS->OpenProtocol (
                  Controller,
                  &gEfiDevicePathProtocolGuid,
                  (VOID **) &UsbMouseDevice->DevicePath,
                  This->DriverBindingHandle,
                  Controller,
                  EFI_OPEN_PROTOCOL_GET_PROTOCOL
                  );

  if (EFI_ERROR (Status)) {
    goto ErrorExit;
  }
  //
  // Get interface & endpoint descriptor
  //
  UsbIo->UsbGetInterfaceDescriptor (
          UsbIo,
          UsbMouseDevice->InterfaceDescriptor
          );

  EndpointNumber = UsbMouseDevice->InterfaceDescriptor->NumEndpoints;

  for (Index = 0; Index < EndpointNumber; Index++) {
    UsbIo->UsbGetEndpointDescriptor (
            UsbIo,
            Index,
            EndpointDesc
            );

    if ((EndpointDesc->Attributes & 0x03) == 0x03) {

      //
      // We only care interrupt endpoint here
      //
      UsbMouseDevice->IntEndpointDescriptor = EndpointDesc;
    }
  }

  if (UsbMouseDevice->IntEndpointDescriptor == NULL) {
    //
    // No interrupt endpoint, then error
    //
    Status = EFI_UNSUPPORTED;
    goto ErrorExit;
  }

  Status = InitializeUsbMouseDevice (UsbMouseDevice);
  if (EFI_ERROR (Status)) {
    MouseReportStatusCode (
      UsbMouseDevice->DevicePath,
      EFI_ERROR_CODE | EFI_ERROR_MINOR,
      PcdGet32 (PcdStatusCodeValueMouseInterfaceError)
      );

    goto ErrorExit;
  }

  UsbMouseDevice->SimplePointerProtocol.GetState  = GetMouseState;
  UsbMouseDevice->SimplePointerProtocol.Reset     = UsbMouseReset;
  UsbMouseDevice->SimplePointerProtocol.Mode      = &UsbMouseDevice->Mode;

  Status = gBS->CreateEvent (
                  EVT_NOTIFY_WAIT,
                  TPL_NOTIFY,
                  UsbMouseWaitForInput,
                  UsbMouseDevice,
                  &((UsbMouseDevice->SimplePointerProtocol).WaitForInput)
                  );
  if (EFI_ERROR (Status)) {
    goto ErrorExit;
  }

  Status = gBS->InstallProtocolInterface (
                  &Controller,
                  &gEfiSimplePointerProtocolGuid,
                  EFI_NATIVE_INTERFACE,
                  &UsbMouseDevice->SimplePointerProtocol
                  );

  if (EFI_ERROR (Status)) {
    Status = EFI_DEVICE_ERROR;
    goto ErrorExit;
  }

  //
  // After Enabling Async Interrupt Transfer on this mouse Device
  // we will be able to get key data from it. Thus this is deemed as
  // the enable action of the mouse
  //

  MouseReportStatusCode (
    UsbMouseDevice->DevicePath,
    EFI_PROGRESS_CODE,
    PcdGet32 (PcdStatusCodeValueMouseEnable)
    );

  //
  // submit async interrupt transfer
  //
  EndpointAddr    = UsbMouseDevice->IntEndpointDescriptor->EndpointAddress;
  PollingInterval = UsbMouseDevice->IntEndpointDescriptor->Interval;
  PacketSize      = (UINT8) (UsbMouseDevice->IntEndpointDescriptor->MaxPacketSize);

  Status = UsbIo->UsbAsyncInterruptTransfer (
                    UsbIo,
                    EndpointAddr,
                    TRUE,
                    PollingInterval,
                    PacketSize,
                    OnMouseInterruptComplete,
                    UsbMouseDevice
                    );

  if (!EFI_ERROR (Status)) {

    UsbMouseDevice->ControllerNameTable = NULL;
    AddUnicodeString (
      "eng",
      gUsbMouseComponentName.SupportedLanguages,
      &UsbMouseDevice->ControllerNameTable,
      L"Generic Usb Mouse"
      );

    return EFI_SUCCESS;
  }

  //
  // If submit error, uninstall that interface
  //
  Status = EFI_DEVICE_ERROR;
  gBS->UninstallProtocolInterface (
        Controller,
        &gEfiSimplePointerProtocolGuid,
        &UsbMouseDevice->SimplePointerProtocol
        );

ErrorExit:
  if (EFI_ERROR (Status)) {
    gBS->CloseProtocol (
          Controller,
          &gEfiUsbIoProtocolGuid,
          This->DriverBindingHandle,
          Controller
          );

    if (UsbMouseDevice != NULL) {
      if (UsbMouseDevice->InterfaceDescriptor != NULL) {
        gBS->FreePool (UsbMouseDevice->InterfaceDescriptor);
      }

      if (UsbMouseDevice->IntEndpointDescriptor != NULL) {
        gBS->FreePool (UsbMouseDevice->IntEndpointDescriptor);
      }

      if ((UsbMouseDevice->SimplePointerProtocol).WaitForInput != NULL) {
        gBS->CloseEvent ((UsbMouseDevice->SimplePointerProtocol).WaitForInput);
      }

      gBS->FreePool (UsbMouseDevice);
      UsbMouseDevice = NULL;
    }
  }

  return Status;
}


/**
  Stop this driver on ControllerHandle. Support stoping any child handles
  created by this driver.

  @param  This                  Protocol instance pointer.
  @param  Controller            Handle of device to stop driver on
  @param  NumberOfChildren      Number of Children in the ChildHandleBuffer
  @param  ChildHandleBuffer     List of handles for the children we need to stop.

  @return EFI_SUCCESS
  @return EFI_DEVICE_ERROR
  @return others

**/
EFI_STATUS
EFIAPI
USBMouseDriverBindingStop (
  IN  EFI_DRIVER_BINDING_PROTOCOL   *This,
  IN  EFI_HANDLE                    Controller,
  IN  UINTN                         NumberOfChildren,
  IN  EFI_HANDLE                    *ChildHandleBuffer
  )
{
  EFI_STATUS                  Status;
  USB_MOUSE_DEV               *UsbMouseDevice;
  EFI_SIMPLE_POINTER_PROTOCOL *SimplePointerProtocol;
  EFI_USB_IO_PROTOCOL         *UsbIo;

  //
  // Get our context back.
  //
  Status = gBS->OpenProtocol (
                  Controller,
                  &gEfiSimplePointerProtocolGuid,
                  &SimplePointerProtocol,
                  This->DriverBindingHandle,
                  Controller,
                  EFI_OPEN_PROTOCOL_GET_PROTOCOL
                  );

  if (EFI_ERROR (Status)) {
    return EFI_UNSUPPORTED;
  }

  UsbMouseDevice = USB_MOUSE_DEV_FROM_MOUSE_PROTOCOL (SimplePointerProtocol);

  gBS->CloseProtocol (
        Controller,
        &gEfiSimplePointerProtocolGuid,
        This->DriverBindingHandle,
        Controller
        );

  UsbIo = UsbMouseDevice->UsbIo;

  //
  // Uninstall the Asyn Interrupt Transfer from this device
  // will disable the mouse data input from this device
  //
  MouseReportStatusCode (
    UsbMouseDevice->DevicePath,
    EFI_PROGRESS_CODE,
    PcdGet32 (PcdStatusCodeValueMouseDisable)
    );

  //
  // Delete Mouse Async Interrupt Transfer
  //
  UsbIo->UsbAsyncInterruptTransfer (
          UsbIo,
          UsbMouseDevice->IntEndpointDescriptor->EndpointAddress,
          FALSE,
          UsbMouseDevice->IntEndpointDescriptor->Interval,
          0,
          NULL,
          NULL
          );

  gBS->CloseEvent (UsbMouseDevice->SimplePointerProtocol.WaitForInput);

  if (UsbMouseDevice->DelayedRecoveryEvent) {
    gBS->CloseEvent (UsbMouseDevice->DelayedRecoveryEvent);
    UsbMouseDevice->DelayedRecoveryEvent = 0;
  }

  Status = gBS->UninstallProtocolInterface (
                  Controller,
                  &gEfiSimplePointerProtocolGuid,
                  &UsbMouseDevice->SimplePointerProtocol
                  );
  if (EFI_ERROR (Status)) {
    return Status;
  }

  gBS->CloseProtocol (
        Controller,
        &gEfiUsbIoProtocolGuid,
        This->DriverBindingHandle,
        Controller
        );

  gBS->FreePool (UsbMouseDevice->InterfaceDescriptor);
  gBS->FreePool (UsbMouseDevice->IntEndpointDescriptor);

  if (UsbMouseDevice->ControllerNameTable) {
    FreeUnicodeStringTable (UsbMouseDevice->ControllerNameTable);
  }

  gBS->FreePool (UsbMouseDevice);

  return EFI_SUCCESS;

}


/**
  Tell if a Usb Controller is a mouse

  @param  UsbIo                 Protocol instance pointer.

  @retval TRUE                  It is a mouse
  @retval FALSE                 It is not a mouse

**/
BOOLEAN
IsUsbMouse (
  IN  EFI_USB_IO_PROTOCOL     *UsbIo
  )
{
  EFI_STATUS                    Status;
  EFI_USB_INTERFACE_DESCRIPTOR  InterfaceDescriptor;

  //
  // Get the Default interface descriptor, now we only
  // suppose it is interface 1
  //
  Status = UsbIo->UsbGetInterfaceDescriptor (
                    UsbIo,
                    &InterfaceDescriptor
                    );

  if (EFI_ERROR (Status)) {
    return FALSE;
  }

  if ((InterfaceDescriptor.InterfaceClass == CLASS_HID) &&
      (InterfaceDescriptor.InterfaceSubClass == SUBCLASS_BOOT) &&
      (InterfaceDescriptor.InterfaceProtocol == PROTOCOL_MOUSE)
      ) {
    return TRUE;
  }

  return FALSE;
}


/**
  Initialize the Usb Mouse Device.

  @param  UsbMouseDev           Device instance to be initialized

  @retval EFI_SUCCESS           Success
  @retval EFI_DEVICE_ERROR      Init error. EFI_OUT_OF_RESOURCES- Can't allocate
                                memory

**/
STATIC
EFI_STATUS
InitializeUsbMouseDevice (
  IN  USB_MOUSE_DEV           *UsbMouseDev
  )
{
  EFI_USB_IO_PROTOCOL     *UsbIo;
  UINT8                   Protocol;
  EFI_STATUS              Status;
  EFI_USB_HID_DESCRIPTOR  MouseHidDesc;
  UINT8                   *ReportDesc;

  UsbIo = UsbMouseDev->UsbIo;

  //
  // Get HID descriptor
  //
  Status = UsbGetHidDescriptor (
            UsbIo,
            UsbMouseDev->InterfaceDescriptor->InterfaceNumber,
            &MouseHidDesc
            );

  if (EFI_ERROR (Status)) {
    return Status;
  }

  //
  // Get Report descriptor
  //
  if (MouseHidDesc.HidClassDesc[0].DescriptorType != 0x22) {
    return EFI_UNSUPPORTED;
  }

  ReportDesc = AllocateZeroPool (MouseHidDesc.HidClassDesc[0].DescriptorLength);
  if (ReportDesc == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }

  Status = UsbGetReportDescriptor (
            UsbIo,
            UsbMouseDev->InterfaceDescriptor->InterfaceNumber,
            MouseHidDesc.HidClassDesc[0].DescriptorLength,
            ReportDesc
            );

  if (EFI_ERROR (Status)) {
    gBS->FreePool (ReportDesc);
    return Status;
  }

  //
  // Parse report descriptor
  //
  Status = ParseMouseReportDescriptor (
            UsbMouseDev,
            ReportDesc,
            MouseHidDesc.HidClassDesc[0].DescriptorLength
            );

  if (EFI_ERROR (Status)) {
    gBS->FreePool (ReportDesc);
    return Status;
  }

  if (UsbMouseDev->NumberOfButtons >= 1) {
    UsbMouseDev->Mode.LeftButton = TRUE;
  }

  if (UsbMouseDev->NumberOfButtons > 1) {
    UsbMouseDev->Mode.RightButton = TRUE;
  }

  UsbMouseDev->Mode.ResolutionX = 8;
  UsbMouseDev->Mode.ResolutionY = 8;
  UsbMouseDev->Mode.ResolutionZ = 0;
  //
  // Here we just assume interface 0 is the mouse interface
  //
  UsbGetProtocolRequest (
    UsbIo,
    0,
    &Protocol
    );

  if (Protocol != BOOT_PROTOCOL) {
    Status = UsbSetProtocolRequest (
              UsbIo,
              0,
              BOOT_PROTOCOL
              );

    if (EFI_ERROR (Status)) {
      gBS->FreePool (ReportDesc);
      return EFI_DEVICE_ERROR;
    }
  }

  //
  // Set indefinite Idle rate for USB Mouse
  //
  UsbSetIdleRequest (
    UsbIo,
    0,
    0,
    0
    );

  gBS->FreePool (ReportDesc);

  if (UsbMouseDev->DelayedRecoveryEvent) {
    gBS->CloseEvent (UsbMouseDev->DelayedRecoveryEvent);
    UsbMouseDev->DelayedRecoveryEvent = 0;
  }

  Status = gBS->CreateEvent (
                  EVT_TIMER | EVT_NOTIFY_SIGNAL,
                  TPL_NOTIFY,
                  USBMouseRecoveryHandler,
                  UsbMouseDev,
                  &UsbMouseDev->DelayedRecoveryEvent
                  );

  return EFI_SUCCESS;
}


/**
  It is called whenever there is data received from async interrupt
  transfer.

  @param  Data                  Data received.
  @param  DataLength            Length of Data
  @param  Context               Passed in context
  @param  Result                Async Interrupt Transfer result

  @return EFI_SUCCESS
  @return EFI_DEVICE_ERROR

**/
STATIC
EFI_STATUS
EFIAPI
OnMouseInterruptComplete (
  IN  VOID        *Data,
  IN  UINTN       DataLength,
  IN  VOID        *Context,
  IN  UINT32      Result
  )
{
  USB_MOUSE_DEV       *UsbMouseDevice;
  EFI_USB_IO_PROTOCOL *UsbIo;
  UINT8               EndpointAddr;
  UINT32              UsbResult;

  UsbMouseDevice  = (USB_MOUSE_DEV *) Context;
  UsbIo           = UsbMouseDevice->UsbIo;

  if (Result != EFI_USB_NOERROR) {
    //
    // Some errors happen during the process
    //
    MouseReportStatusCode (
      UsbMouseDevice->DevicePath,
      EFI_ERROR_CODE | EFI_ERROR_MINOR,
      PcdGet32 (PcdStatusCodeValueMouseInputError)
      );

    if ((Result & EFI_USB_ERR_STALL) == EFI_USB_ERR_STALL) {
      EndpointAddr = UsbMouseDevice->IntEndpointDescriptor->EndpointAddress;

      UsbClearEndpointHalt (
        UsbIo,
        EndpointAddr,
        &UsbResult
        );
    }

    UsbIo->UsbAsyncInterruptTransfer (
            UsbIo,
            UsbMouseDevice->IntEndpointDescriptor->EndpointAddress,
            FALSE,
            0,
            0,
            NULL,
            NULL
            );

    gBS->SetTimer (
          UsbMouseDevice->DelayedRecoveryEvent,
          TimerRelative,
          EFI_USB_INTERRUPT_DELAY
          );
    return EFI_DEVICE_ERROR;
  }

  if (DataLength == 0 || Data == NULL) {
    return EFI_SUCCESS;
  }

  UsbMouseDevice->StateChanged = TRUE;

  //
  // Check mouse Data
  //
  UsbMouseDevice->State.LeftButton  = (BOOLEAN) (*(UINT8 *) Data & 0x01);
  UsbMouseDevice->State.RightButton = (BOOLEAN) (*(UINT8 *) Data & 0x02);
  UsbMouseDevice->State.RelativeMovementX += *((INT8 *) Data + 1);
  UsbMouseDevice->State.RelativeMovementY += *((INT8 *) Data + 2);

  if (DataLength > 3) {
    UsbMouseDevice->State.RelativeMovementZ += *((INT8 *) Data + 3);
  }

  return EFI_SUCCESS;
}

/*
STATIC VOID
PrintMouseState(
    IN  EFI_MOUSE_STATE *MouseState
    )
{
    Aprint("(%x: %x, %x)\n",
        MouseState->ButtonStates,
        MouseState->dx,
        MouseState->dy
        );
}
*/

/**
  Get the mouse state, see SIMPLE POINTER PROTOCOL.

  @param  This                  Protocol instance pointer.
  @param  MouseState            Current mouse state

  @return EFI_SUCCESS
  @return EFI_DEVICE_ERROR
  @return EFI_NOT_READY

**/
STATIC
EFI_STATUS
EFIAPI
GetMouseState (
  IN   EFI_SIMPLE_POINTER_PROTOCOL  *This,
  OUT  EFI_SIMPLE_POINTER_STATE     *MouseState
  )
{
  USB_MOUSE_DEV *MouseDev;

  if (MouseState == NULL) {
    return EFI_DEVICE_ERROR;
  }

  MouseDev = USB_MOUSE_DEV_FROM_MOUSE_PROTOCOL (This);

  if (!MouseDev->StateChanged) {
    return EFI_NOT_READY;
  }

  CopyMem (
    MouseState,
    &MouseDev->State,
    sizeof (EFI_SIMPLE_POINTER_STATE)
    );

  //
  // Clear previous move state
  //
  MouseDev->State.RelativeMovementX = 0;
  MouseDev->State.RelativeMovementY = 0;
  MouseDev->State.RelativeMovementZ = 0;

  MouseDev->StateChanged            = FALSE;

  return EFI_SUCCESS;
}


/**
  Reset the mouse device, see SIMPLE POINTER PROTOCOL.

  @param  This                  Protocol instance pointer.
  @param  ExtendedVerification  Ignored here/

  @return EFI_SUCCESS

**/
STATIC
EFI_STATUS
EFIAPI
UsbMouseReset (
  IN EFI_SIMPLE_POINTER_PROTOCOL    *This,
  IN BOOLEAN                        ExtendedVerification
  )
{
  USB_MOUSE_DEV       *UsbMouseDevice;
  EFI_USB_IO_PROTOCOL *UsbIo;

  UsbMouseDevice  = USB_MOUSE_DEV_FROM_MOUSE_PROTOCOL (This);

  UsbIo           = UsbMouseDevice->UsbIo;

  MouseReportStatusCode (
    UsbMouseDevice->DevicePath,
    EFI_PROGRESS_CODE,
    PcdGet32 (PcdStatusCodeValueMouseReset)

    );

  ZeroMem (
    &UsbMouseDevice->State,
    sizeof (EFI_SIMPLE_POINTER_STATE)
    );
  UsbMouseDevice->StateChanged = FALSE;

  return EFI_SUCCESS;
}


/**
  Event notification function for SIMPLE_POINTER.WaitForInput event
  Signal the event if there is input from mouse

  @param  Event                 Wait Event
  @param  Context               Passed parameter to event handler
 VOID

**/
STATIC
VOID
EFIAPI
UsbMouseWaitForInput (
  IN  EFI_EVENT               Event,
  IN  VOID                    *Context
  )
{
  USB_MOUSE_DEV *UsbMouseDev;

  UsbMouseDev = (USB_MOUSE_DEV *) Context;

  //
  // Someone is waiting on the mouse event, if there's
  // input from mouse, signal the event
  //
  if (UsbMouseDev->StateChanged) {
    gBS->SignalEvent (Event);
  }
}


/**
  Timer handler for Delayed Recovery timer.

  @param  Event                 The Delayed Recovery event.
  @param  Context               Points to the USB_KB_DEV instance.


**/
VOID
EFIAPI
USBMouseRecoveryHandler (
  IN    EFI_EVENT    Event,
  IN    VOID         *Context
  )
{
  USB_MOUSE_DEV       *UsbMouseDev;
  EFI_USB_IO_PROTOCOL *UsbIo;

  UsbMouseDev = (USB_MOUSE_DEV *) Context;

  UsbIo       = UsbMouseDev->UsbIo;

  UsbIo->UsbAsyncInterruptTransfer (
          UsbIo,
          UsbMouseDev->IntEndpointDescriptor->EndpointAddress,
          TRUE,
          UsbMouseDev->IntEndpointDescriptor->Interval,
          UsbMouseDev->IntEndpointDescriptor->MaxPacketSize,
          OnMouseInterruptComplete,
          UsbMouseDev
          );
}


/**
  Report Status Code in Usb Bot Driver

  @param  DevicePath            Use this to get Device Path
  @param  CodeType              Status Code Type
  @param  CodeValue             Status Code Value

  @return None

**/
VOID
MouseReportStatusCode (
  IN EFI_DEVICE_PATH_PROTOCOL  *DevicePath,
  IN EFI_STATUS_CODE_TYPE      CodeType,
  IN EFI_STATUS_CODE_VALUE     Value
  )
{
  REPORT_STATUS_CODE_WITH_DEVICE_PATH (
    CodeType,
    Value,
    DevicePath
    );
}
