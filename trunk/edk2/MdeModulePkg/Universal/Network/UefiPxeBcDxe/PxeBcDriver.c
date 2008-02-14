/** @file

Copyright (c) 2007 - 2008, Intel Corporation
All rights reserved. This program and the accompanying materials
are licensed and made available under the terms and conditions of the BSD License
which accompanies this distribution.  The full text of the license may be found at
http://opensource.org/licenses/bsd-license.php

THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

Module Name:

  PxeBcDriver.c

Abstract:

  The driver binding for IP4 CONFIG protocol.


**/


#include "PxeBcImpl.h"


/**
  The entry point for PxeBc driver which install the driver
  binding and component name protocol on its image.

  @param  ImageHandle          The Image handle of the driver
  @param  SystemTable          The system table

  @return EFI_SUCCESS
  @return Others

**/
EFI_STATUS
PxeBcDriverEntryPoint (
  IN EFI_HANDLE             ImageHandle,
  IN EFI_SYSTEM_TABLE       *SystemTable
  )
{
  return EfiLibInstallDriverBindingComponentName2 (
          ImageHandle,
          SystemTable,
          &gPxeBcDriverBinding,
          ImageHandle,
          &gPxeBcComponentName,
          &gPxeBcComponentName2
          );
}


/**
  Test to see if this driver supports ControllerHandle.

  @param  This                 Protocol instance pointer.
  @param  ControllerHandle     Handle of device to test
  @param  RemainingDevicePath  Optional parameter use to pick a specific child
                               device to start.

  @return EFI_SUCCES
  @return EFI_ALREADY_STARTED
  @return Others

**/
EFI_STATUS
EFIAPI
PxeBcDriverBindingSupported (
  IN EFI_DRIVER_BINDING_PROTOCOL  * This,
  IN EFI_HANDLE                   ControllerHandle,
  IN EFI_DEVICE_PATH_PROTOCOL     * RemainingDevicePath OPTIONAL
  )
{
  EFI_PXE_BASE_CODE_PROTOCOL  *PxeBc;
  EFI_STATUS                  Status;

  Status = gBS->OpenProtocol (
                  ControllerHandle,
                  &gEfiPxeBaseCodeProtocolGuid,
                  (VOID **) &PxeBc,
                  This->DriverBindingHandle,
                  ControllerHandle,
                  EFI_OPEN_PROTOCOL_GET_PROTOCOL
                  );

  if (!EFI_ERROR (Status)) {
    return EFI_ALREADY_STARTED;
  }

  Status = gBS->OpenProtocol (
                  ControllerHandle,
                  &gEfiDhcp4ServiceBindingProtocolGuid,
                  NULL,
                  This->DriverBindingHandle,
                  ControllerHandle,
                  EFI_OPEN_PROTOCOL_TEST_PROTOCOL
                  );

  if (!EFI_ERROR (Status)) {

    Status = gBS->OpenProtocol (
                    ControllerHandle,
                    &gEfiMtftp4ServiceBindingProtocolGuid,
                    NULL,
                    This->DriverBindingHandle,
                    ControllerHandle,
                    EFI_OPEN_PROTOCOL_TEST_PROTOCOL
                    );

  }

  return Status;
}


/**
  Start this driver on ControllerHandle.

  @param  This                 Protocol instance pointer.
  @param  ControllerHandle     Handle of device to bind driver to
  @param  RemainingDevicePath  Optional parameter use to pick a specific child
                               device to start.

  @return EFI_SUCCES
  @return EFI_ALREADY_STARTED
  @return EFI_OUT_OF_RESOURCES
  @return Others

**/
EFI_STATUS
EFIAPI
PxeBcDriverBindingStart (
  IN EFI_DRIVER_BINDING_PROTOCOL  * This,
  IN EFI_HANDLE                   ControllerHandle,
  IN EFI_DEVICE_PATH_PROTOCOL     * RemainingDevicePath OPTIONAL
  )
{
  PXEBC_PRIVATE_DATA  *Private;
  UINTN               Index;
  EFI_STATUS          Status;

  Private = AllocateZeroPool (sizeof (PXEBC_PRIVATE_DATA));
  if (Private == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }

  Private->Signature                    = PXEBC_PRIVATE_DATA_SIGNATURE;
  Private->Controller                   = ControllerHandle;
  Private->Image                        = This->DriverBindingHandle;
  CopyMem (&Private->PxeBc, &mPxeBcProtocolTemplate, sizeof (Private->PxeBc));
  Private->PxeBc.Mode                   = &Private->Mode;
  CopyMem (&Private->LoadFile, &mLoadFileProtocolTemplate, sizeof (Private->LoadFile));

  Private->ProxyOffer.Packet.Offer.Size = PXEBC_CACHED_DHCP4_PACKET_MAX_SIZE;
  Private->Dhcp4Ack.Packet.Ack.Size     = PXEBC_CACHED_DHCP4_PACKET_MAX_SIZE;
  Private->PxeReply.Packet.Ack.Size     = PXEBC_CACHED_DHCP4_PACKET_MAX_SIZE;

  for (Index = 0; Index < PXEBC_MAX_OFFER_NUM; Index++) {
    Private->Dhcp4Offers[Index].Packet.Offer.Size = PXEBC_CACHED_DHCP4_PACKET_MAX_SIZE;
  }

  //
  // Get the NII interface
  //
  Status = gBS->OpenProtocol (
                  ControllerHandle,
                  &gEfiNetworkInterfaceIdentifierProtocolGuid_31,
                  (VOID **) &Private->Nii,
                  This->DriverBindingHandle,
                  ControllerHandle,
                  EFI_OPEN_PROTOCOL_GET_PROTOCOL
                  );
  if (EFI_ERROR (Status)) {
    goto ON_ERROR;
  }

  Status = NetLibCreateServiceChild (
            ControllerHandle,
            This->DriverBindingHandle,
            &gEfiDhcp4ServiceBindingProtocolGuid,
            &Private->Dhcp4Child
            );
  if (EFI_ERROR (Status)) {
    goto ON_ERROR;
  }

  Status = gBS->OpenProtocol (
                  Private->Dhcp4Child,
                  &gEfiDhcp4ProtocolGuid,
                  (VOID **) &Private->Dhcp4,
                  This->DriverBindingHandle,
                  ControllerHandle,
                  EFI_OPEN_PROTOCOL_BY_DRIVER
                  );
  if (EFI_ERROR (Status)) {
    goto ON_ERROR;
  }

  Status = NetLibCreateServiceChild (
             ControllerHandle,
             This->DriverBindingHandle,
             &gEfiMtftp4ServiceBindingProtocolGuid,
             &Private->Mtftp4Child
             );

  if (EFI_ERROR (Status)) {
    goto ON_ERROR;
  }

  Status = gBS->OpenProtocol (
                  Private->Mtftp4Child,
                  &gEfiMtftp4ProtocolGuid,
                  (VOID **) &Private->Mtftp4,
                  This->DriverBindingHandle,
                  ControllerHandle,
                  EFI_OPEN_PROTOCOL_BY_DRIVER
                  );

  if (EFI_ERROR (Status)) {
    goto ON_ERROR;
  }

  Status = NetLibCreateServiceChild (
             ControllerHandle,
             This->DriverBindingHandle,
             &gEfiUdp4ServiceBindingProtocolGuid,
             &Private->Udp4Child
             );

  if (EFI_ERROR (Status)) {
    goto ON_ERROR;
  }

  Status = gBS->OpenProtocol (
                  Private->Udp4Child,
                  &gEfiUdp4ProtocolGuid,
                  (VOID **) &Private->Udp4,
                  This->DriverBindingHandle,
                  ControllerHandle,
                  EFI_OPEN_PROTOCOL_BY_DRIVER
                  );

  if (EFI_ERROR (Status)) {
    goto ON_ERROR;
  }

  ZeroMem (&Private->Udp4CfgData, sizeof (EFI_UDP4_CONFIG_DATA));
  Private->Udp4CfgData.AcceptBroadcast    = TRUE;
  Private->Udp4CfgData.AcceptPromiscuous  = FALSE;
  Private->Udp4CfgData.AcceptAnyPort      = FALSE;
  Private->Udp4CfgData.AllowDuplicatePort = TRUE;
  Private->Udp4CfgData.TypeOfService      = DEFAULT_ToS;
  Private->Udp4CfgData.TimeToLive         = DEFAULT_TTL;
  Private->Udp4CfgData.DoNotFragment      = FALSE;
  Private->Udp4CfgData.ReceiveTimeout     = 10000;  // 10 milliseconds
  Private->Udp4CfgData.UseDefaultAddress  = FALSE;

  PxeBcInitSeedPacket (&Private->SeedPacket, Private->Udp4);

  Status = gBS->InstallMultipleProtocolInterfaces (
                  &ControllerHandle,
                  &gEfiPxeBaseCodeProtocolGuid,
                  &Private->PxeBc,
                  &gEfiLoadFileProtocolGuid,
                  &Private->LoadFile,
                  NULL
                  );
  if (EFI_ERROR (Status)) {
    goto ON_ERROR;
  }

  return EFI_SUCCESS;

ON_ERROR:

  if (Private->Udp4Child != NULL) {
    gBS->CloseProtocol (
          Private->Udp4Child,
          &gEfiUdp4ProtocolGuid,
          This->DriverBindingHandle,
          ControllerHandle
          );
    NetLibDestroyServiceChild (
      ControllerHandle,
      This->DriverBindingHandle,
      &gEfiUdp4ServiceBindingProtocolGuid,
      Private->Udp4Child
      );
  }

  if (Private->Mtftp4Child != NULL) {
    gBS->CloseProtocol (
          Private->Mtftp4Child,
          &gEfiMtftp4ProtocolGuid,
          This->DriverBindingHandle,
          ControllerHandle
          );

    NetLibDestroyServiceChild (
      ControllerHandle,
      This->DriverBindingHandle,
      &gEfiMtftp4ServiceBindingProtocolGuid,
      Private->Mtftp4Child
      );
  }

  if (Private->Dhcp4Child != NULL) {
    gBS->CloseProtocol (
          Private->Dhcp4Child,
          &gEfiDhcp4ProtocolGuid,
          This->DriverBindingHandle,
          ControllerHandle
          );

    NetLibDestroyServiceChild (
      ControllerHandle,
      This->DriverBindingHandle,
      &gEfiDhcp4ServiceBindingProtocolGuid,
      Private->Dhcp4Child
      );
  }

  gBS->FreePool (Private);

  return Status;
}


/**
  Stop this driver on ControllerHandle.

  @param  This                 Protocol instance pointer.
  @param  ControllerHandle     Handle of device to stop driver on
  @param  NumberOfChildren     Number of Handles in ChildHandleBuffer. If number of
                                children is zero stop the entire bus driver.
  @param  ChildHandleBuffer    List of Child Handles to Stop.

  @return EFI_SUCCESS
  @return EFI_DEVICE_ERROR
  @return Others

**/
EFI_STATUS
EFIAPI
PxeBcDriverBindingStop (
  IN EFI_DRIVER_BINDING_PROTOCOL  *This,
  IN EFI_HANDLE                   ControllerHandle,
  IN UINTN                        NumberOfChildren,
  IN EFI_HANDLE                   *ChildHandleBuffer
  )
{
  PXEBC_PRIVATE_DATA          *Private;
  EFI_PXE_BASE_CODE_PROTOCOL  *PxeBc;
  EFI_HANDLE                  NicHandle;
  EFI_STATUS                  Status;

  NicHandle = NetLibGetNicHandle (ControllerHandle, &gEfiDhcp4ProtocolGuid);

  if (NicHandle == NULL) {

    NicHandle = NetLibGetNicHandle (ControllerHandle, &gEfiMtftp4ProtocolGuid);

    if (NicHandle == NULL) {

      return EFI_DEVICE_ERROR;
    }
  }

  Status = gBS->OpenProtocol (
                  NicHandle,
                  &gEfiPxeBaseCodeProtocolGuid,
                  (VOID **) &PxeBc,
                  This->DriverBindingHandle,
                  ControllerHandle,
                  EFI_OPEN_PROTOCOL_GET_PROTOCOL
                  );

  if (EFI_ERROR (Status)) {
    return Status;
  }

  Private = PXEBC_PRIVATE_DATA_FROM_PXEBC (PxeBc);

  Status = gBS->UninstallMultipleProtocolInterfaces (
                  NicHandle,
                  &gEfiPxeBaseCodeProtocolGuid,
                  &Private->PxeBc,
                  &gEfiLoadFileProtocolGuid,
                  &Private->LoadFile,
                  NULL
                  );

  if (!EFI_ERROR (Status)) {

    gBS->CloseProtocol (
          Private->Udp4Child,
          &gEfiUdp4ProtocolGuid,
          This->DriverBindingHandle,
          NicHandle
          );
    NetLibDestroyServiceChild (
      NicHandle,
      This->DriverBindingHandle,
      &gEfiUdp4ServiceBindingProtocolGuid,
      Private->Udp4Child
      );

    gBS->CloseProtocol (
          Private->Dhcp4Child,
          &gEfiDhcp4ProtocolGuid,
          This->DriverBindingHandle,
          NicHandle
          );
    NetLibDestroyServiceChild (
      NicHandle,
      This->DriverBindingHandle,
      &gEfiDhcp4ServiceBindingProtocolGuid,
      Private->Dhcp4Child
      );

    gBS->CloseProtocol (
          Private->Mtftp4Child,
          &gEfiMtftp4ProtocolGuid,
          This->DriverBindingHandle,
          NicHandle
          );
    NetLibDestroyServiceChild (
      NicHandle,
      This->DriverBindingHandle,
      &gEfiMtftp4ServiceBindingProtocolGuid,
      Private->Mtftp4Child
      );

    gBS->FreePool (Private);
  }

  return Status;
}

EFI_DRIVER_BINDING_PROTOCOL gPxeBcDriverBinding = {
  PxeBcDriverBindingSupported,
  PxeBcDriverBindingStart,
  PxeBcDriverBindingStop,
  0xa,
  NULL,
  NULL
};


