/** @file

Copyright (c) 2005 - 2006, Intel Corporation
All rights reserved. This program and the accompanying materials
are licensed and made available under the terms and conditions of the BSD License
which accompanies this distribution.  The full text of the license may be found at
http://opensource.org/licenses/bsd-license.php

THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

Module Name:

  Ip4Driver.c

Abstract:

  The driver binding and service binding protocol for IP4 driver.


**/

#include "Ip4Impl.h"

EFI_DRIVER_BINDING_PROTOCOL gIp4DriverBinding = {
  Ip4DriverBindingSupported,
  Ip4DriverBindingStart,
  Ip4DriverBindingStop,
  0xa,
  NULL,
  NULL
};

//@MT: EFI_DRIVER_ENTRY_POINT (Ip4DriverEntryPoint)

EFI_STATUS
EFIAPI
Ip4DriverEntryPoint (
  IN EFI_HANDLE             ImageHandle,
  IN EFI_SYSTEM_TABLE       *SystemTable
  )
/*++

Routine Description:

  The entry point for IP4 driver which install the driver
  binding and component name protocol on its image.

Arguments:

  ImageHandle - The image handle of the driver
  SystemTable - The system table

Returns:

  EFI_SUCCESS if the driver binding and component name protocols
  are successfully installed, otherwise if failed.

--*/
{
  return NetLibInstallAllDriverProtocols (
           ImageHandle,
           SystemTable,
           &gIp4DriverBinding,
           ImageHandle,
           &gIp4ComponentName,
           NULL,
           NULL
           );
}


/**
  Test to see if this driver supports ControllerHandle.

  @param  This                   Protocol instance pointer.
  @param  ControllerHandle       Handle of device to test
  @param  RemainingDevicePath    Optional parameter use to pick a specific child
                                 device to start.

  @retval EFI_SUCCES             This driver supports this device
  @retval EFI_ALREADY_STARTED    This driver is already running on this device
  @retval other                  This driver does not support this device

**/
EFI_STATUS
EFIAPI
Ip4DriverBindingSupported (
  IN EFI_DRIVER_BINDING_PROTOCOL  * This,
  IN EFI_HANDLE                   ControllerHandle,
  IN EFI_DEVICE_PATH_PROTOCOL     * RemainingDevicePath OPTIONAL
  )
{
  EFI_STATUS                Status;

  //
  // Test for the MNP service binding Protocol
  //
  Status = gBS->OpenProtocol (
                  ControllerHandle,
                  &gEfiManagedNetworkServiceBindingProtocolGuid,
                  NULL,
                  This->DriverBindingHandle,
                  ControllerHandle,
                  EFI_OPEN_PROTOCOL_TEST_PROTOCOL
                  );

  if (EFI_ERROR (Status)) {
    return Status;
  }

  //
  // Test for the Arp service binding Protocol
  //
  Status = gBS->OpenProtocol (
                  ControllerHandle,
                  &gEfiArpServiceBindingProtocolGuid,
                  NULL,
                  This->DriverBindingHandle,
                  ControllerHandle,
                  EFI_OPEN_PROTOCOL_TEST_PROTOCOL
                  );

  return Status;
}

STATIC
EFI_STATUS
Ip4CleanService (
  IN IP4_SERVICE            *IpSb
  );


/**
  Create a new IP4 driver service binding protocol

  @param  Controller             The controller that has MNP service binding
                                 installed
  @param  ImageHandle            The IP4 driver's image handle
  @param  Service                The variable to receive the newly created IP4
                                 service.

  @retval EFI_OUT_OF_RESOURCES   Failed to allocate some resource
  @retval EFI_SUCCESS            A new IP4 service binding private is created.

**/
STATIC
EFI_STATUS
Ip4CreateService (
  IN  EFI_HANDLE            Controller,
  IN  EFI_HANDLE            ImageHandle,
  OUT IP4_SERVICE           **Service
  )
{
  IP4_SERVICE               *IpSb;
  EFI_STATUS                Status;

  ASSERT (Service != NULL);

  *Service = NULL;

  //
  // allocate a service private data then initialize all the filed to
  // empty resources, so if any thing goes wrong when allocating
  // resources, Ip4CleanService can be called to clean it up.
  //
  IpSb = NetAllocatePool (sizeof (IP4_SERVICE));

  if (IpSb == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }

  IpSb->Signature                   = IP4_SERVICE_SIGNATURE;
  IpSb->ServiceBinding.CreateChild  = Ip4ServiceBindingCreateChild;
  IpSb->ServiceBinding.DestroyChild = Ip4ServiceBindingDestroyChild;
  IpSb->State                       = IP4_SERVICE_UNSTARTED;
  IpSb->InDestory                   = FALSE;

  IpSb->NumChildren                 = 0;
  NetListInit (&IpSb->Children);

  NetListInit (&IpSb->Interfaces);
  IpSb->DefaultInterface            = NULL;
  IpSb->DefaultRouteTable           = NULL;

  Ip4InitAssembleTable (&IpSb->Assemble);

  IpSb->IgmpCtrl.Igmpv1QuerySeen    = 0;
  NetListInit (&IpSb->IgmpCtrl.Groups);

  IpSb->Image                       = ImageHandle;
  IpSb->Controller                  = Controller;

  IpSb->MnpChildHandle              = NULL;
  IpSb->Mnp                         = NULL;

  IpSb->MnpConfigData.ReceivedQueueTimeoutValue = 0;
  IpSb->MnpConfigData.TransmitQueueTimeoutValue = 0;
  IpSb->MnpConfigData.ProtocolTypeFilter        = IP4_ETHER_PROTO;
  IpSb->MnpConfigData.EnableUnicastReceive      = TRUE;
  IpSb->MnpConfigData.EnableMulticastReceive    = TRUE;
  IpSb->MnpConfigData.EnableBroadcastReceive    = TRUE;
  IpSb->MnpConfigData.EnablePromiscuousReceive  = FALSE;
  IpSb->MnpConfigData.FlushQueuesOnReset        = TRUE;
  IpSb->MnpConfigData.EnableReceiveTimestamps   = FALSE;
  IpSb->MnpConfigData.DisableBackgroundPolling  = FALSE;

  NetZeroMem (&IpSb->SnpMode, sizeof (EFI_SIMPLE_NETWORK_MODE));

  IpSb->Timer                       = NULL;
  IpSb->Ip4Config                   = NULL;
  IpSb->DoneEvent                   = NULL;
  IpSb->ReconfigEvent               = NULL;

  //
  // Create various resources. First create the route table, timer
  // event and MNP child. IGMP, interface's initialization depend
  // on the MNP child.
  //
  IpSb->DefaultRouteTable = Ip4CreateRouteTable ();

  if (IpSb->DefaultRouteTable == NULL) {
    Status = EFI_OUT_OF_RESOURCES;
    goto ON_ERROR;
  }

  Status = gBS->CreateEvent (
                  EVT_NOTIFY_SIGNAL | EVT_TIMER,
                  TPL_CALLBACK,
                  Ip4TimerTicking,
                  IpSb,
                  &IpSb->Timer
                  );

  if (EFI_ERROR (Status)) {
    goto ON_ERROR;
  }

  Status = NetLibCreateServiceChild (
             Controller,
             ImageHandle,
             &gEfiManagedNetworkServiceBindingProtocolGuid,
             &IpSb->MnpChildHandle
             );

  if (EFI_ERROR (Status)) {
    goto ON_ERROR;
  }

  Status = gBS->OpenProtocol (
                  IpSb->MnpChildHandle,
                  &gEfiManagedNetworkProtocolGuid,
                  (VOID **) &IpSb->Mnp,
                  ImageHandle,
                  Controller,
                  EFI_OPEN_PROTOCOL_BY_DRIVER
                  );

  if (EFI_ERROR (Status)) {
    goto ON_ERROR;
  }

  Status = Ip4ServiceConfigMnp (IpSb, TRUE);

  if (EFI_ERROR (Status)) {
    goto ON_ERROR;
  }

  Status = IpSb->Mnp->GetModeData (IpSb->Mnp, NULL, &IpSb->SnpMode);

  if (EFI_ERROR (Status)) {
    goto ON_ERROR;
  }

  Status = Ip4InitIgmp (IpSb);

  if (EFI_ERROR (Status)) {
    goto ON_ERROR;
  }

  IpSb->DefaultInterface = Ip4CreateInterface (IpSb->Mnp, Controller, ImageHandle);

  if (IpSb->DefaultInterface == NULL) {
    Status = EFI_OUT_OF_RESOURCES;
    goto ON_ERROR;
  }

  NetListInsertHead (&IpSb->Interfaces, &IpSb->DefaultInterface->Link);

  IpSb->MacString = NULL;

  *Service = IpSb;
  return EFI_SUCCESS;

ON_ERROR:
  Ip4CleanService (IpSb);
  NetFreePool (IpSb);

  return Status;
}


/**
  Clean up a IP4 service binding instance. It will release all
  the resource allocated by the instance. The instance may be
  partly initialized, or partly destoried. If a resource is
  destoried, it is marked as that in case the destory failed and
  being called again later.

  @param  IpSb                   The IP4 serviceing binding instance to clean up

  @retval EFI_SUCCESS            The resource used by the instance are cleaned up
  @retval Others                 Failed to clean up some of the resources.

**/
EFI_STATUS
Ip4CleanService (
  IN IP4_SERVICE            *IpSb
  )
{
  EFI_STATUS                Status;

  if (IpSb->DefaultInterface != NULL) {
    Status = Ip4FreeInterface (IpSb->DefaultInterface, NULL);

    if (EFI_ERROR (Status)) {
      return Status;
    }

    IpSb->DefaultInterface = NULL;
  }

  if (IpSb->DefaultRouteTable != NULL) {
    Ip4FreeRouteTable (IpSb->DefaultRouteTable);
    IpSb->DefaultRouteTable = NULL;
  }

  Ip4CleanAssembleTable (&IpSb->Assemble);

  if (IpSb->MnpChildHandle != NULL) {
    if (IpSb->Mnp) {
      gBS->CloseProtocol (
            IpSb->MnpChildHandle,
            &gEfiManagedNetworkProtocolGuid,
            IpSb->Image,
            IpSb->Controller
            );

      IpSb->Mnp = NULL;
    }

    NetLibDestroyServiceChild (
      IpSb->Controller,
      IpSb->Image,
      &gEfiManagedNetworkServiceBindingProtocolGuid,
      IpSb->MnpChildHandle
      );

    IpSb->MnpChildHandle = NULL;
  }

  if (IpSb->Timer != NULL) {
    gBS->SetTimer (IpSb->Timer, TimerCancel, 0);
    gBS->CloseEvent (IpSb->Timer);

    IpSb->Timer = NULL;
  }

  if (IpSb->Ip4Config != NULL) {
    IpSb->Ip4Config->Stop (IpSb->Ip4Config);

    gBS->CloseProtocol (
          IpSb->Controller,
          &gEfiIp4ConfigProtocolGuid,
          IpSb->Image,
          IpSb->Controller
          );

    gBS->CloseEvent (IpSb->DoneEvent);
    gBS->CloseEvent (IpSb->ReconfigEvent);
    IpSb->Ip4Config = NULL;
  }

  return EFI_SUCCESS;
}


/**
  Start this driver on ControllerHandle.

  @param  This                   Protocol instance pointer.
  @param  ControllerHandle       Handle of device to bind driver to
  @param  RemainingDevicePath    Optional parameter use to pick a specific child
                                 device to start.

  @retval EFI_SUCCES             This driver is added to ControllerHandle
  @retval EFI_ALREADY_STARTED    This driver is already running on ControllerHandle
  @retval other                  This driver does not support this device

**/
EFI_STATUS
EFIAPI
Ip4DriverBindingStart (
  IN EFI_DRIVER_BINDING_PROTOCOL  * This,
  IN EFI_HANDLE                   ControllerHandle,
  IN EFI_DEVICE_PATH_PROTOCOL     * RemainingDevicePath OPTIONAL
  )
{
  IP4_SERVICE               *IpSb;
  EFI_STATUS                Status;

  //
  // Test for the Ip4 service binding protocol
  //
  Status = gBS->OpenProtocol (
                  ControllerHandle,
                  &gEfiIp4ServiceBindingProtocolGuid,
                  NULL,
                  This->DriverBindingHandle,
                  ControllerHandle,
                  EFI_OPEN_PROTOCOL_TEST_PROTOCOL
                  );

  if (Status == EFI_SUCCESS) {
    return EFI_ALREADY_STARTED;
  }

  Status = Ip4CreateService (ControllerHandle, This->DriverBindingHandle, &IpSb);

  if (EFI_ERROR (Status)) {
    return Status;
  }

  //
  // Install the Ip4ServiceBinding Protocol onto ControlerHandle
  //
  Status = gBS->InstallMultipleProtocolInterfaces (
                  &ControllerHandle,
                  &gEfiIp4ServiceBindingProtocolGuid,
                  &IpSb->ServiceBinding,
                  NULL
                  );

  if (EFI_ERROR (Status)) {
    goto FREE_SERVICE;
  }

  //
  // ready to go: start the receiving and timer
  //
  Status = Ip4ReceiveFrame (IpSb->DefaultInterface, NULL, Ip4AccpetFrame, IpSb);

  if (EFI_ERROR (Status)) {
    goto UNINSTALL_PROTOCOL;
  }

  Status = gBS->SetTimer (IpSb->Timer, TimerPeriodic, TICKS_PER_SECOND);

  if (EFI_ERROR (Status)) {
    goto UNINSTALL_PROTOCOL;
  }

  //
  // Initialize the IP4 ID
  //
  mIp4Id = (UINT16)NET_RANDOM (NetRandomInitSeed ());

  Ip4SetVariableData (IpSb);

  return Status;

UNINSTALL_PROTOCOL:
  gBS->UninstallProtocolInterface (
         ControllerHandle,
         &gEfiIp4ServiceBindingProtocolGuid,
         &IpSb->ServiceBinding
         );

FREE_SERVICE:
  Ip4CleanService (IpSb);
  NetFreePool (IpSb);

  return Status;
}


/**
  Stop this driver on ControllerHandle.

  @param  This                   Protocol instance pointer.
  @param  ControllerHandle       Handle of device to stop driver on
  @param  NumberOfChildren       Number of Handles in ChildHandleBuffer. If  number
                                 of children is zero stop the entire  bus driver.
  @param  ChildHandleBuffer      List of Child Handles to Stop.

  @retval EFI_SUCCES             This driver is removed ControllerHandle
  @retval other                  This driver was not removed from this device

**/
EFI_STATUS
EFIAPI
Ip4DriverBindingStop (
  IN  EFI_DRIVER_BINDING_PROTOCOL  *This,
  IN  EFI_HANDLE                   ControllerHandle,
  IN  UINTN                        NumberOfChildren,
  IN  EFI_HANDLE                   *ChildHandleBuffer
  )
{
  EFI_SERVICE_BINDING_PROTOCOL  *ServiceBinding;
  IP4_SERVICE                   *IpSb;
  IP4_PROTOCOL                  *IpInstance;
  EFI_HANDLE                    NicHandle;
  EFI_STATUS                    Status;
  EFI_TPL                       OldTpl;
  INTN                          State;

  //
  // IP4 driver opens the MNP child, ARP children or the IP4_CONFIG protocol
  // by driver. So the ControllerHandle may be the MNP child handle, ARP child
  // handle, or the NIC (UNDI) handle because IP4_CONFIG protocol is installed
  // in the NIC handle.
  //
  //
  // First, check whether it is the IP4_CONFIG protocol being uninstalled.
  // IP4_CONFIG protocol is installed on the NIC handle. It isn't necessary
  // to clean up the default configuration if IP4_CONFIG is being stopped.
  //
  Status = gBS->OpenProtocol (
                  ControllerHandle,
                  &gEfiIp4ConfigProtocolGuid,
                  NULL,
                  This->DriverBindingHandle,
                  ControllerHandle,
                  EFI_OPEN_PROTOCOL_TEST_PROTOCOL
                  );

  if (Status == EFI_SUCCESS) {
    //
    // Retrieve the IP4 service binding protocol. If failed, it is
    // likely that Ip4 ServiceBinding is uninstalled already. In this
    // case, return immediately.
    //
    Status = gBS->OpenProtocol (
                    ControllerHandle,
                    &gEfiIp4ServiceBindingProtocolGuid,
                    (VOID **) &ServiceBinding,
                    This->DriverBindingHandle,
                    ControllerHandle,
                    EFI_OPEN_PROTOCOL_GET_PROTOCOL
                    );

    if (EFI_ERROR (Status)) {
      return EFI_SUCCESS;
    }

    IpSb = IP4_SERVICE_FROM_PROTOCOL (ServiceBinding);

    OldTpl = NET_RAISE_TPL (NET_TPL_LOCK);

    if (IpSb->Ip4Config && (IpSb->State != IP4_SERVICE_DESTORY)) {

      IpSb->Ip4Config->Stop (IpSb->Ip4Config);

      Status = gBS->CloseProtocol (
                      ControllerHandle,
                      &gEfiIp4ConfigProtocolGuid,
                      IpSb->Image,
                      ControllerHandle
                      );

      if (EFI_ERROR (Status)) {
        NET_RESTORE_TPL (OldTpl);
        return Status;
      }

      //
      // If the auto configure hasn't complete, mark it as not started.
      //
      if (IpSb->State == IP4_SERVICE_STARTED) {
        IpSb->State = IP4_SERVICE_UNSTARTED;
      }

      IpSb->Ip4Config = NULL;
      gBS->CloseEvent (IpSb->DoneEvent);
      gBS->CloseEvent (IpSb->ReconfigEvent);
    }

    NET_RESTORE_TPL (OldTpl);
    return EFI_SUCCESS;
  }

  //
  // Either MNP or ARP protocol is being uninstalled. The controller
  // handle is either the MNP child or ARP child. But, the IP4's
  // service binding is installed on the NIC handle. So, need to open
  // the protocol info to find the NIC handle.
  //
  NicHandle = NetLibGetNicHandle (ControllerHandle, &gEfiManagedNetworkProtocolGuid);

  if (NicHandle == NULL) {
    NicHandle = NetLibGetNicHandle (ControllerHandle, &gEfiArpProtocolGuid);
  }

  if (NicHandle == NULL) {
    return EFI_SUCCESS;
  }

  //
  // Retrieve the IP4 service binding protocol
  //
  Status = gBS->OpenProtocol (
                  NicHandle,
                  &gEfiIp4ServiceBindingProtocolGuid,
                  (VOID **) &ServiceBinding,
                  This->DriverBindingHandle,
                  NicHandle,
                  EFI_OPEN_PROTOCOL_GET_PROTOCOL
                  );

  if (EFI_ERROR (Status)) {
    return EFI_DEVICE_ERROR;
  }

  IpSb   = IP4_SERVICE_FROM_PROTOCOL (ServiceBinding);

  OldTpl = NET_RAISE_TPL (NET_TPL_LOCK);

  if (IpSb->InDestory) {
    NET_RESTORE_TPL (OldTpl);
    return EFI_SUCCESS;
  }

  IpSb->InDestory = TRUE;

  State           = IpSb->State;
  IpSb->State     = IP4_SERVICE_DESTORY;

  //
  // Destory all the children first. If not all children are destoried,
  // the IP driver can operate correctly, so restore it state. Don't
  // use NET_LIST_FOR_EACH_SAFE here, because it will cache the next
  // pointer, which may point to the child that has already been destoried.
  // For example, if there are two child in the list, the first is UDP
  // listen child, the send is the MTFTP's child. When Udp child is
  // destoried, it will destory the MTFTP's child. Then Next point to
  // a invalid child.
  //
  while (!NetListIsEmpty (&IpSb->Children)) {
    IpInstance = NET_LIST_HEAD (&IpSb->Children, IP4_PROTOCOL, Link);
    Ip4ServiceBindingDestroyChild (ServiceBinding, IpInstance->Handle);
  }

  if (IpSb->NumChildren != 0) {
    IpSb->State = State;
    Status      = EFI_DEVICE_ERROR;
    goto ON_ERROR;
  }

  //
  // Clear the variable data.
  //
  Ip4ClearVariableData (IpSb);

  //
  // OK, clean other resources then uninstall the service binding protocol.
  //
  Status = Ip4CleanService (IpSb);

  if (EFI_ERROR (Status)) {
    goto ON_ERROR;
  }

  Status = gBS->UninstallProtocolInterface (
                  NicHandle,
                  &gEfiIp4ServiceBindingProtocolGuid,
                  ServiceBinding
                  );

  if (EFI_ERROR (Status)) {
    goto ON_ERROR;
  }

  NET_RESTORE_TPL (OldTpl);
  NetFreePool (IpSb);
  return EFI_SUCCESS;

ON_ERROR:
  IpSb->InDestory = FALSE;
  NET_RESTORE_TPL (OldTpl);
  return Status;
}


/**
  Creates a child handle with a set of I/O services.

  @param  This                   Protocol instance pointer.
  @param  ChildHandle            Pointer to the handle of the child to create.   If
                                 it is NULL, then a new handle is created.   If it
                                 is not NULL, then the I/O services are  added to
                                 the existing child handle.

  @retval EFI_SUCCES             The child handle was created with the I/O services
  @retval EFI_OUT_OF_RESOURCES   There are not enough resources availabe to create
                                 the child
  @retval other                  The child handle was not created

**/
EFI_STATUS
EFIAPI
Ip4ServiceBindingCreateChild (
  IN EFI_SERVICE_BINDING_PROTOCOL  *This,
  IN EFI_HANDLE                    *ChildHandle
  )
{
  IP4_SERVICE               *IpSb;
  IP4_PROTOCOL              *IpInstance;
  EFI_TPL                   OldTpl;
  EFI_STATUS                Status;
  VOID                      *Mnp;

  if ((This == NULL) || (ChildHandle == NULL)) {
    return EFI_INVALID_PARAMETER;
  }

  IpSb       = IP4_SERVICE_FROM_PROTOCOL (This);
  IpInstance = NetAllocatePool (sizeof (IP4_PROTOCOL));

  if (IpInstance == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }

  Ip4InitProtocol (IpSb, IpInstance);

  //
  // Install Ip4 onto ChildHandle
  //
  Status = gBS->InstallMultipleProtocolInterfaces (
                  ChildHandle,
                  &gEfiIp4ProtocolGuid,
                  &IpInstance->Ip4Proto,
                  NULL
                  );

  if (EFI_ERROR (Status)) {
    goto ON_ERROR;
  }

  IpInstance->Handle = *ChildHandle;

  //
  // Open the Managed Network protocol BY_CHILD.
  //
  Status = gBS->OpenProtocol (
                  IpSb->MnpChildHandle,
                  &gEfiManagedNetworkProtocolGuid,
                  (VOID **) &Mnp,
                  gIp4DriverBinding.DriverBindingHandle,
                  IpInstance->Handle,
                  EFI_OPEN_PROTOCOL_BY_CHILD_CONTROLLER
                  );
  if (EFI_ERROR (Status)) {
    gBS->UninstallMultipleProtocolInterfaces (
           ChildHandle,
           &gEfiIp4ProtocolGuid,
           &IpInstance->Ip4Proto,
           NULL
           );

    goto ON_ERROR;
  }

  //
  // Insert it into the service binding instance.
  //
  OldTpl = NET_RAISE_TPL (NET_TPL_LOCK);

  NetListInsertTail (&IpSb->Children, &IpInstance->Link);
  IpSb->NumChildren++;

  NET_RESTORE_TPL (OldTpl);

ON_ERROR:

  if (EFI_ERROR (Status)) {

    Ip4CleanProtocol (IpInstance);

    NetFreePool (IpInstance);
  }

  return Status;
}


/**
  Destroys a child handle with a set of I/O services.

  @param  This                   Protocol instance pointer.
  @param  ChildHandle            Handle of the child to destroy

  @retval EFI_SUCCES             The I/O services were removed from the child
                                 handle
  @retval EFI_UNSUPPORTED        The child handle does not support the I/O services
                                  that are being removed
  @retval EFI_INVALID_PARAMETER  Child handle is not a valid EFI Handle.
  @retval EFI_ACCESS_DENIED      The child handle could not be destroyed because
                                 its  I/O services are being used.
  @retval other                  The child handle was not destroyed

**/
EFI_STATUS
EFIAPI
Ip4ServiceBindingDestroyChild (
  IN EFI_SERVICE_BINDING_PROTOCOL  *This,
  IN EFI_HANDLE                    ChildHandle
  )
{
  EFI_STATUS                Status;
  IP4_SERVICE               *IpSb;
  IP4_PROTOCOL              *IpInstance;
  EFI_IP4_PROTOCOL          *Ip4;
  EFI_TPL                   OldTpl;
  INTN                      State;

  if ((This == NULL) || (ChildHandle == NULL)) {
    return EFI_INVALID_PARAMETER;
  }

  //
  // Retrieve the private context data structures
  //
  IpSb   = IP4_SERVICE_FROM_PROTOCOL (This);

  Status = gBS->OpenProtocol (
                  ChildHandle,
                  &gEfiIp4ProtocolGuid,
                  (VOID **) &Ip4,
                  gIp4DriverBinding.DriverBindingHandle,
                  ChildHandle,
                  EFI_OPEN_PROTOCOL_GET_PROTOCOL
                  );

  if (EFI_ERROR (Status)) {
    return EFI_UNSUPPORTED;
  }

  IpInstance = IP4_INSTANCE_FROM_PROTOCOL (Ip4);

  if (IpInstance->Service != IpSb) {
    return EFI_INVALID_PARAMETER;
  }

  OldTpl = NET_RAISE_TPL (NET_TPL_LOCK);

  //
  // A child can be destoried more than once. For example,
  // Ip4DriverBindingStop will destory all of its children.
  // when UDP driver is being stopped, it will destory all
  // the IP child it opens.
  //
  if (IpInstance->State == IP4_STATE_DESTORY) {
    NET_RESTORE_TPL (OldTpl);
    return EFI_SUCCESS;
  }

  State             = IpInstance->State;
  IpInstance->State = IP4_STATE_DESTORY;

  //
  // Close the Managed Network protocol.
  //
  gBS->CloseProtocol (
         IpSb->MnpChildHandle,
         &gEfiManagedNetworkProtocolGuid,
         gIp4DriverBinding.DriverBindingHandle,
         ChildHandle
         );

  //
  // Uninstall the IP4 protocol first. Many thing happens during
  // this:
  // 1. The consumer of the IP4 protocol will be stopped if it
  // opens the protocol BY_DRIVER. For eaxmple, if MNP driver is
  // stopped, IP driver's stop function will be called, and uninstall
  // EFI_IP4_PROTOCOL will trigger the UDP's stop function. This
  // makes it possible to create the network stack bottom up, and
  // stop it top down.
  // 2. the upper layer will recycle the received packet. The recycle
  // event's TPL is higher than this function. The recycle events
  // will be called back before preceeding. If any packets not recycled,
  // that means there is a resource leak.
  //
  Status = gBS->UninstallProtocolInterface (
                  ChildHandle,
                  &gEfiIp4ProtocolGuid,
                  &IpInstance->Ip4Proto
                  );

  if (EFI_ERROR (Status)) {
    goto ON_ERROR;
  }

  Status = Ip4CleanProtocol (IpInstance);

  Ip4SetVariableData (IpSb);

  if (EFI_ERROR (Status)) {
    gBS->InstallMultipleProtocolInterfaces (
           &ChildHandle,
           &gEfiIp4ProtocolGuid,
           Ip4,
           NULL
           );

    goto ON_ERROR;
  }

  NetListRemoveEntry (&IpInstance->Link);
  IpSb->NumChildren--;

  NET_RESTORE_TPL (OldTpl);

  NetFreePool (IpInstance);
  return EFI_SUCCESS;

ON_ERROR:
  IpInstance->State = State;
  NET_RESTORE_TPL (OldTpl);

  return Status;
}
