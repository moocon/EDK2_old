/** @file

Copyright (c) 2006 - 2007, Intel Corporation
All rights reserved. This program and the accompanying materials
are licensed and made available under the terms and conditions of the BSD License
which accompanies this distribution.  The full text of the license may be found at
http://opensource.org/licenses/bsd-license.php

THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

Module Name:

  Mtftp4Driver.c

Abstract:


**/

#include "Mtftp4Impl.h"

EFI_DRIVER_BINDING_PROTOCOL   gMtftp4DriverBinding = {
  Mtftp4DriverBindingSupported,
  Mtftp4DriverBindingStart,
  Mtftp4DriverBindingStop,
  0xa,
  NULL,
  NULL
};

EFI_SERVICE_BINDING_PROTOCOL  gMtftp4ServiceBindingTemplete = {
  Mtftp4ServiceBindingCreateChild,
  Mtftp4ServiceBindingDestroyChild
};

EFI_STATUS
EFIAPI
Mtftp4DriverEntryPoint (
  IN EFI_HANDLE             ImageHandle,
  IN EFI_SYSTEM_TABLE       *SystemTable
  )
/*++

Routine Description:

  The driver entry point which installs multiple protocols to the ImageHandle.

Arguments:

  ImageHandle - The MTFTP's image handle
  SystemTable - The system table

Returns:

  EFI_SUCCESS - The handles are successfully installed on the image. Otherwise
  some EFI_ERROR.

--*/
{
  return EfiLibInstallDriverBindingComponentName2 (
           ImageHandle,
           SystemTable,
           &gMtftp4DriverBinding,
           ImageHandle,
           &gMtftp4ComponentName,
           &gMtftp4ComponentName2
           );
}


/**
  Test whether MTFTP driver support this controller.

  @param  This                   The MTFTP driver binding instance
  @param  Controller             The controller to test
  @param  RemainingDevicePath    The remaining device path

  @retval EFI_SUCCESS            The controller has UDP service binding protocol
                                 installed, MTFTP can support it.
  @retval Others                 MTFTP can't support the controller.

**/
EFI_STATUS
Mtftp4DriverBindingSupported (
  IN EFI_DRIVER_BINDING_PROTOCOL    *This,
  IN EFI_HANDLE                     Controller,
  IN EFI_DEVICE_PATH_PROTOCOL       *RemainingDevicePath
  )
{
  EFI_STATUS  Status;

  Status = gBS->OpenProtocol (
                  Controller,
                  &gEfiUdp4ServiceBindingProtocolGuid,
                  NULL,
                  This->DriverBindingHandle,
                  Controller,
                  EFI_OPEN_PROTOCOL_TEST_PROTOCOL
                  );

  return Status;
}


/**
  Config a NULL UDP that is used to keep the connection between UDP
  and MTFTP. Just leave the Udp child unconfigured. When UDP is
  unloaded, MTFTP will be informed with DriverBinding Stop.

  @param  UdpIo                  The UDP port to configure
  @param  Context                The opaque parameter to the callback

  @retval EFI_SUCCESS            It always return EFI_SUCCESS directly.

**/
EFI_STATUS
Mtftp4ConfigNullUdp (
  IN UDP_IO_PORT            *UdpIo,
  IN VOID                   *Context
  )
{
  return EFI_SUCCESS;
}


/**
  Create then initialize a MTFTP service binding instance.

  @param  Controller             The controller to install the MTFTP service
                                 binding on
  @param  Image                  The driver binding image of the MTFTP driver
  @param  Service                The variable to receive the created service
                                 binding instance.

  @retval EFI_OUT_OF_RESOURCES   Failed to allocate resource to create the instance
  @retval EFI_DEVICE_ERROR       Failed to create a NULL UDP port to keep
                                 connection  with UDP.
  @retval EFI_SUCCESS            The service instance is created for the
                                 controller.

**/
EFI_STATUS
Mtftp4CreateService (
  IN  EFI_HANDLE            Controller,
  IN  EFI_HANDLE            Image,
  OUT MTFTP4_SERVICE        **Service
  )
{
  MTFTP4_SERVICE            *MtftpSb;
  EFI_STATUS                Status;

  *Service  = NULL;
  MtftpSb   = NetAllocatePool (sizeof (MTFTP4_SERVICE));

  if (MtftpSb == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }

  MtftpSb->Signature      = MTFTP4_SERVICE_SIGNATURE;
  MtftpSb->ServiceBinding = gMtftp4ServiceBindingTemplete;
  MtftpSb->InDestory      = FALSE;
  MtftpSb->ChildrenNum    = 0;
  NetListInit (&MtftpSb->Children);

  MtftpSb->Timer          = NULL;
  MtftpSb->TimerToGetMap  = NULL;
  MtftpSb->Controller     = Controller;
  MtftpSb->Image          = Image;
  MtftpSb->ConnectUdp     = NULL;

  //
  // Create the timer and a udp to be notified when UDP is uninstalled
  //
  Status = gBS->CreateEvent (
                  EVT_NOTIFY_SIGNAL | EVT_TIMER,
                  NET_TPL_TIMER,
                  Mtftp4OnTimerTick,
                  MtftpSb,
                  &MtftpSb->Timer
                  );

  if (EFI_ERROR (Status)) {
    NetFreePool (MtftpSb);
    return Status;
  }

  //
  // Create the timer used to time out the procedure which is used to
  // get the default IP address.
  //
  Status = gBS->CreateEvent (
                  EVT_TIMER,
                  NET_TPL_TIMER,
                  NULL,
                  NULL,
                  &MtftpSb->TimerToGetMap
                  );
  if (EFI_ERROR (Status)) {
    gBS->CloseEvent (MtftpSb->Timer);
    NetFreePool (MtftpSb);
    return Status;
  }

  MtftpSb->ConnectUdp = UdpIoCreatePort (Controller, Image, Mtftp4ConfigNullUdp, NULL);

  if (MtftpSb->ConnectUdp == NULL) {
    gBS->CloseEvent (MtftpSb->TimerToGetMap);
    gBS->CloseEvent (MtftpSb->Timer);
    NetFreePool (MtftpSb);
    return EFI_DEVICE_ERROR;
  }

  *Service = MtftpSb;
  return EFI_SUCCESS;
}


/**
  Release all the resource used the MTFTP service binding instance.

  @param  MtftpSb                The MTFTP service binding instance.

  @return None

**/
VOID
Mtftp4CleanService (
  IN MTFTP4_SERVICE     *MtftpSb
  )
{
  UdpIoFreePort (MtftpSb->ConnectUdp);
  gBS->CloseEvent (MtftpSb->TimerToGetMap);
  gBS->CloseEvent (MtftpSb->Timer);
}


/**
  Start the MTFTP driver on this controller. MTFTP driver will
  install a MTFTP SERVICE BINDING protocol on the supported
  controller, which can be used to create/destroy MTFTP children.

  @param  This                   The MTFTP driver binding protocol.
  @param  Controller             The controller to manage.
  @param  RemainingDevicePath    Remaining device path.

  @retval EFI_ALREADY_STARTED    The MTFTP service binding protocol has been
                                 started  on the controller.
  @retval EFI_SUCCESS            The MTFTP service binding is installed on the
                                 controller.

**/
EFI_STATUS
Mtftp4DriverBindingStart (
  IN EFI_DRIVER_BINDING_PROTOCOL  *This,
  IN EFI_HANDLE                   Controller,
  IN EFI_DEVICE_PATH_PROTOCOL     *RemainingDevicePath
  )
{
  MTFTP4_SERVICE            *MtftpSb;
  EFI_STATUS                Status;

  //
  // Directly return if driver is already running.
  //
  Status = gBS->OpenProtocol (
                  Controller,
                  &gEfiMtftp4ServiceBindingProtocolGuid,
                  NULL,
                  This->DriverBindingHandle,
                  Controller,
                  EFI_OPEN_PROTOCOL_TEST_PROTOCOL
                  );

  if (Status == EFI_SUCCESS) {
    return EFI_ALREADY_STARTED;
  }

  Status = Mtftp4CreateService (Controller, This->DriverBindingHandle, &MtftpSb);

  if (EFI_ERROR (Status)) {
    return Status;
  }

  Status = gBS->SetTimer (MtftpSb->Timer, TimerPeriodic, TICKS_PER_SECOND);

  if (EFI_ERROR (Status)) {
    goto ON_ERROR;
  }

  //
  // Install the Mtftp4ServiceBinding Protocol onto Controller
  //
  Status = gBS->InstallMultipleProtocolInterfaces (
                  &Controller,
                  &gEfiMtftp4ServiceBindingProtocolGuid,
                  &MtftpSb->ServiceBinding,
                  NULL
                  );

  if (EFI_ERROR (Status)) {
    goto ON_ERROR;
  }

  return EFI_SUCCESS;

ON_ERROR:
  Mtftp4CleanService (MtftpSb);
  NetFreePool (MtftpSb);

  return Status;
}


/**
  Stop the MTFTP driver on controller. The controller is a UDP
  child handle.

  @param  This                   The MTFTP driver binding protocol
  @param  Controller             The controller to stop
  @param  NumberOfChildren       The number of children
  @param  ChildHandleBuffer      The array of the child handle.

  @retval EFI_SUCCESS            The driver is stopped on the controller.
  @retval EFI_DEVICE_ERROR       Failed to stop the driver on the controller.

**/
EFI_STATUS
Mtftp4DriverBindingStop (
  IN  EFI_DRIVER_BINDING_PROTOCOL *This,
  IN  EFI_HANDLE                  Controller,
  IN  UINTN                       NumberOfChildren,
  IN  EFI_HANDLE                  *ChildHandleBuffer
  )
{
  EFI_SERVICE_BINDING_PROTOCOL  *ServiceBinding;
  MTFTP4_SERVICE                *MtftpSb;
  MTFTP4_PROTOCOL               *Instance;
  EFI_HANDLE                    NicHandle;
  EFI_STATUS                    Status;
  EFI_TPL                       OldTpl;

  //
  // MTFTP driver opens UDP child, So, Controller is a UDP
  // child handle. Locate the Nic handle first. Then get the
  // MTFTP private data back.
  //
  NicHandle = NetLibGetNicHandle (Controller, &gEfiUdp4ProtocolGuid);

  if (NicHandle == NULL) {
    return EFI_SUCCESS;
  }

  Status = gBS->OpenProtocol (
                  NicHandle,
                  &gEfiMtftp4ServiceBindingProtocolGuid,
                  (VOID **) &ServiceBinding,
                  This->DriverBindingHandle,
                  NicHandle,
                  EFI_OPEN_PROTOCOL_GET_PROTOCOL
                  );

  if (EFI_ERROR (Status)) {
    return EFI_DEVICE_ERROR;
  }

  MtftpSb = MTFTP4_SERVICE_FROM_THIS (ServiceBinding);

  if (MtftpSb->InDestory) {
    return EFI_SUCCESS;
  }

  OldTpl             = NET_RAISE_TPL (NET_TPL_LOCK);
  MtftpSb->InDestory = TRUE;

  while (!NetListIsEmpty (&MtftpSb->Children)) {
    Instance = NET_LIST_HEAD (&MtftpSb->Children, MTFTP4_PROTOCOL, Link);
    Mtftp4ServiceBindingDestroyChild (ServiceBinding, Instance->Handle);
  }

  if (MtftpSb->ChildrenNum != 0) {
    Status = EFI_DEVICE_ERROR;
    goto ON_ERROR;
  }

  Status = gBS->UninstallProtocolInterface (
                  NicHandle,
                  &gEfiMtftp4ServiceBindingProtocolGuid,
                  ServiceBinding
                  );

  if (EFI_ERROR (Status)) {
    goto ON_ERROR;
  }

  Mtftp4CleanService (MtftpSb);
  NetFreePool (MtftpSb);

  NET_RESTORE_TPL (OldTpl);
  return EFI_SUCCESS;

ON_ERROR:
  MtftpSb->InDestory = FALSE;

  NET_RESTORE_TPL (OldTpl);
  return Status;
}


/**
  Initialize a MTFTP protocol instance which is the child of MtftpSb.

  @param  MtftpSb                The MTFTP service binding protocol.
  @param  Instance               The MTFTP instance to initialize.

  @return None

**/
VOID
Mtftp4InitProtocol (
  IN MTFTP4_SERVICE         *MtftpSb,
  IN MTFTP4_PROTOCOL        *Instance
  )
{
  NetZeroMem (Instance, sizeof (MTFTP4_PROTOCOL));

  Instance->Signature = MTFTP4_PROTOCOL_SIGNATURE;
  NetListInit (&Instance->Link);
  CopyMem (&Instance->Mtftp4, &gMtftp4ProtocolTemplate, sizeof (Instance->Mtftp4));
  Instance->State     = MTFTP4_STATE_UNCONFIGED;
  Instance->Indestory = FALSE;
  Instance->Service   = MtftpSb;

  NetListInit (&Instance->Blocks);
}


/**
  Create a MTFTP child for the service binding instance, then
  install the MTFTP protocol to the ChildHandle.

  @param  This                   The MTFTP service binding instance.
  @param  ChildHandle            The Child handle to install the MTFTP protocol.

  @retval EFI_INVALID_PARAMETER  The parameter is invalid.
  @retval EFI_OUT_OF_RESOURCES   Failed to allocate resource for the new child.
  @retval EFI_SUCCESS            The child is successfully create.

**/
EFI_STATUS
Mtftp4ServiceBindingCreateChild (
  IN EFI_SERVICE_BINDING_PROTOCOL  *This,
  IN OUT EFI_HANDLE                *ChildHandle
  )
{
  MTFTP4_SERVICE            *MtftpSb;
  MTFTP4_PROTOCOL           *Instance;
  EFI_STATUS                Status;
  EFI_TPL                   OldTpl;
  VOID                      *Udp4;

  if ((This == NULL) || (ChildHandle == NULL)) {
    return EFI_INVALID_PARAMETER;
  }

  Instance = NetAllocatePool (sizeof (*Instance));

  if (Instance == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }

  MtftpSb = MTFTP4_SERVICE_FROM_THIS (This);

  Mtftp4InitProtocol (MtftpSb, Instance);

  Instance->UnicastPort = UdpIoCreatePort (
                            MtftpSb->Controller,
                            MtftpSb->Image,
                            Mtftp4ConfigNullUdp,
                            Instance
                            );

  if (Instance->UnicastPort == NULL) {
    NetFreePool (Instance);
    return EFI_OUT_OF_RESOURCES;
  }

  //
  // Install the MTFTP protocol onto ChildHandle
  //
  Status = gBS->InstallMultipleProtocolInterfaces (
                  ChildHandle,
                  &gEfiMtftp4ProtocolGuid,
                  &Instance->Mtftp4,
                  NULL
                  );

  if (EFI_ERROR (Status)) {
    goto ON_ERROR;
  }

  Instance->Handle  = *ChildHandle;

  //
  // Open the Udp4 protocol BY_CHILD.
  //
  Status = gBS->OpenProtocol (
                  MtftpSb->ConnectUdp->UdpHandle,
                  &gEfiUdp4ProtocolGuid,
                  (VOID **) &Udp4,
                  gMtftp4DriverBinding.DriverBindingHandle,
                  Instance->Handle,
                  EFI_OPEN_PROTOCOL_BY_CHILD_CONTROLLER
                  );
  if (EFI_ERROR (Status)) {
    gBS->UninstallMultipleProtocolInterfaces (
           Instance->Handle,
           &gEfiMtftp4ProtocolGuid,
           &Instance->Mtftp4,
           NULL
           );

    goto ON_ERROR;
  }

  //
  // Add it to the parent's child list.
  //
  OldTpl = NET_RAISE_TPL (NET_TPL_LOCK);

  NetListInsertTail (&MtftpSb->Children, &Instance->Link);
  MtftpSb->ChildrenNum++;

  NET_RESTORE_TPL (OldTpl);

ON_ERROR:

  if (EFI_ERROR (Status)) {
    UdpIoFreePort (Instance->UnicastPort);
    NetFreePool (Instance);
  }

  return Status;
}


/**
  Destory one of the service binding's child.

  @param  This                   The service binding instance
  @param  ChildHandle            The child handle to destory

  @retval EFI_INVALID_PARAMETER  The parameter is invaid.
  @retval EFI_UNSUPPORTED        The child may have already been destoried.
  @retval EFI_SUCCESS            The child is destoried and removed from the
                                 parent's child list.

**/
EFI_STATUS
Mtftp4ServiceBindingDestroyChild (
  IN EFI_SERVICE_BINDING_PROTOCOL *This,
  IN EFI_HANDLE                   ChildHandle
  )
{
  MTFTP4_SERVICE            *MtftpSb;
  MTFTP4_PROTOCOL           *Instance;
  EFI_MTFTP4_PROTOCOL       *Mtftp4;
  EFI_STATUS                Status;
  EFI_TPL                   OldTpl;

  if ((This == NULL) || (ChildHandle == NULL)) {
    return EFI_INVALID_PARAMETER;
  }

  //
  // Retrieve the private context data structures
  //
  Status = gBS->OpenProtocol (
                  ChildHandle,
                  &gEfiMtftp4ProtocolGuid,
                  (VOID **) &Mtftp4,
                  gMtftp4DriverBinding.DriverBindingHandle,
                  ChildHandle,
                  EFI_OPEN_PROTOCOL_GET_PROTOCOL
                  );

  if (EFI_ERROR (Status)) {
    return EFI_UNSUPPORTED;
  }

  Instance  = MTFTP4_PROTOCOL_FROM_THIS (Mtftp4);
  MtftpSb   = MTFTP4_SERVICE_FROM_THIS (This);

  if (Instance->Service != MtftpSb) {
    return EFI_INVALID_PARAMETER;
  }

  if (Instance->Indestory) {
    return EFI_SUCCESS;
  }

  Instance->Indestory = TRUE;

  //
  // Close the Udp4 protocol.
  //
  gBS->CloseProtocol (
         MtftpSb->ConnectUdp->UdpHandle,
         &gEfiUdp4ProtocolGuid,
         gMtftp4DriverBinding.DriverBindingHandle,
         ChildHandle
         );

  //
  // Uninstall the MTFTP4 protocol first to enable a top down destruction.
  //
  Status = gBS->UninstallProtocolInterface (
                  ChildHandle,
                  &gEfiMtftp4ProtocolGuid,
                  Mtftp4
                  );

  if (EFI_ERROR (Status)) {
    Instance->Indestory = FALSE;
    return Status;
  }

  OldTpl = NET_RAISE_TPL (NET_TPL_LOCK);

  Mtftp4CleanOperation (Instance, EFI_DEVICE_ERROR);
  UdpIoFreePort (Instance->UnicastPort);

  NetListRemoveEntry (&Instance->Link);
  MtftpSb->ChildrenNum--;

  NET_RESTORE_TPL (OldTpl);

  NetFreePool (Instance);
  return EFI_SUCCESS;
}
