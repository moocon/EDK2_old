/*++

Copyright (c)  2007 Intel Corporation. All rights reserved
This software and associated documentation (if any) is furnished
under a license and may only be used or copied in accordance
with the terms of the license. Except as permitted by such
license, no part of this software or documentation may be
reproduced, stored in a retrieval system, or transmitted in any
form or by any means without the express written consent of
Intel Corporation.

Module Name:

  IScsiIbft.c

Abstract:

  Implementation for iSCSI Boot Firmware Table publication.

--*/

#include "IScsiImpl.h"

STATIC
VOID
IScsiInitIbfTableHeader (
  IN EFI_ACPI_ISCSI_BOOT_FIRMWARE_TABLE_HEADER  *Header
  )
/*++

Routine Description:

  Initialize the header of the iSCSI Boot Firmware Table.
  
Arguments:

  Header - The header of the iSCSI Boot Firmware Table.

Returns:

  None.

--*/
{
  NetZeroMem (Header, sizeof (EFI_ACPI_ISCSI_BOOT_FIRMWARE_TABLE_HEADER));

  Header->Signature = EFI_ACPI_3_0_ISCSI_BOOT_FIRMWARE_TABLE_SIGNATURE;
  Header->Length    = IBFT_HEAP_OFFSET;
  Header->Revision  = EFI_ACPI_ISCSI_BOOT_FIRMWARE_TABLE_REVISION;
  Header->Checksum  = 0;

  Header->OemId[0]  = 'I';
  Header->OemId[1]  = 'N';
  Header->OemId[2]  = 'T';
  Header->OemId[3]  = 'E';
  Header->OemId[4]  = 'L';
}

STATIC
VOID
IScsiInitControlSection (
  IN EFI_ACPI_ISCSI_BOOT_FIRMWARE_TABLE_HEADER  *Table,
  IN UINTN                                      HandleCount
  )
/*++

Routine Description:

  Initialize the control section of the iSCSI Boot Firmware Table.
  
Arguments:

  Table       - The ACPI table.
  HandleCount - The number of the handles associated with iSCSI sessions, it's
                equal to the number of iSCSI sessions.

Returns:

  None.

--*/
{
  EFI_ACPI_ISCSI_BOOT_FIRMWARE_TABLE_CONTROL_STRUCTURE  *Control;
  UINTN                                                 NumOffset;

  Control = (EFI_ACPI_ISCSI_BOOT_FIRMWARE_TABLE_CONTROL_STRUCTURE *) (Table + 1);

  NetZeroMem (Control, sizeof (EFI_ACPI_ISCSI_BOOT_FIRMWARE_TABLE_CONTROL_STRUCTURE));

  Control->Header.StructureId = EFI_ACPI_ISCSI_BOOT_FIRMWARE_TABLE_CONTROL_STRUCTURE_ID;
  Control->Header.Version     = EFI_ACPI_ISCSI_BOOT_FIRMWARE_TABLE_CONTROL_STRUCTURE_VERSION;
  Control->Header.Length      = sizeof (EFI_ACPI_ISCSI_BOOT_FIRMWARE_TABLE_CONTROL_STRUCTURE);

  //
  // Each session occupies two offsets, one for the NIC section,
  // the other for the Target section.
  //
  NumOffset = 2 * HandleCount;
  if (NumOffset > 4) {
    //
    // Need expand the control section if more than 2 NIC/Target sections
    // exist.
    //
    Control->Header.Length += (UINT16) (NumOffset - 4) * sizeof (UINT16);
  }
}

STATIC
VOID
IScsiAddHeapItem (
  IN OUT UINT8  **Heap,
  IN     VOID   *Data,
  IN     UINTN  Len
  )
/*++

Routine Description:

  Add one item into the heap.
  
Arguments:

  Heap - On input, the current address of the heap; On output, the address of
         the heap after the item is added.
  Data - The data to add into the heap.
  Len  - Length of the Data in byte.

Returns:

  None.

--*/
{
  //
  // Add one byte for the NULL delimiter.
  //
  *Heap -= Len + 1;

  NetCopyMem (*Heap, Data, Len);
  *(*Heap + Len) = 0;
}

STATIC
VOID
IScsiFillInitiatorSection (
  IN     EFI_ACPI_ISCSI_BOOT_FIRMWARE_TABLE_HEADER  *Table,
  IN OUT UINT8                                      **Heap,
  IN     EFI_HANDLE                                 Handle
  )
/*++

Routine Description:

  Fill the Initiator section of the iSCSI Boot Firmware Table.
  
Arguments:

  Table  - The ACPI table.
  Heap   - The heap.
  Handle - The handle associated with the iSCSI session.

Returns:

  None.

--*/
{
  EFI_ACPI_ISCSI_BOOT_FIRMWARE_TABLE_CONTROL_STRUCTURE    *Control;
  EFI_ACPI_ISCSI_BOOT_FIRMWARE_TABLE_INITIATOR_STRUCTURE  *Initiator;
  ISCSI_DRIVER_DATA                                       *DriverData;
  ISCSI_SESSION                                           *Session;
  ISCSI_PRIVATE_PROTOCOL                                  *IScsiIdentifier;
  EFI_STATUS                                              Status;

  Control = (EFI_ACPI_ISCSI_BOOT_FIRMWARE_TABLE_CONTROL_STRUCTURE *) (Table + 1);

  //
  // Initiator section immediately follows the control section.
  //
  Initiator = (EFI_ACPI_ISCSI_BOOT_FIRMWARE_TABLE_INITIATOR_STRUCTURE *) ((UINT8 *) Control + IBFT_ROUNDUP (Control->Header.Length));

  Control->InitiatorOffset = (UINT16) ((UINTN) Initiator - (UINTN) Table);

  NetZeroMem (Initiator, sizeof (EFI_ACPI_ISCSI_BOOT_FIRMWARE_TABLE_INITIATOR_STRUCTURE));

  Initiator->Header.StructureId = EFI_ACPI_ISCSI_BOOT_FIRMWARE_TABLE_INITIATOR_STRUCTURE_ID;
  Initiator->Header.Version     = EFI_ACPI_ISCSI_BOOT_FIRMWARE_TABLE_INITIATOR_STRUCTURE_VERSION;
  Initiator->Header.Length      = sizeof (EFI_ACPI_ISCSI_BOOT_FIRMWARE_TABLE_INITIATOR_STRUCTURE);
  Initiator->Header.Flags       = EFI_ACPI_ISCSI_BOOT_FIRMWARE_TABLE_INITIATOR_STRUCTURE_FLAG_BLOCK_VALID | EFI_ACPI_ISCSI_BOOT_FIRMWARE_TABLE_INITIATOR_STRUCTURE_FLAG_BOOT_SELECTED;

  //
  // Get the identifier from the handle.
  //
  Status = gBS->HandleProtocol (Handle, &mIScsiPrivateGuid, &IScsiIdentifier);
  if (EFI_ERROR (Status)) {
    ASSERT (FALSE);
    return ;
  }

  DriverData  = ISCSI_DRIVER_DATA_FROM_IDENTIFIER (IScsiIdentifier);
  Session     = &DriverData->Session;

  //
  // Fill the iSCSI Initiator Name into the heap.
  //
  IScsiAddHeapItem (Heap, Session->InitiatorName, Session->InitiatorNameLength - 1);

  Initiator->IScsiNameLength  = (UINT16) (Session->InitiatorNameLength - 1);
  Initiator->IScsiNameOffset  = (UINT16) ((UINTN) *Heap - (UINTN) Table);
}

STATIC
VOID
IScsiMapV4ToV6Addr (
  IN  EFI_IPv4_ADDRESS *V4,
  OUT EFI_IPv6_ADDRESS *V6
  )
/*++

Routine Description:

  Map the v4 IP address into v6 IP address.
  
Arguments:

  V4 - The v4 IP address.
  V6 - The v6 IP address.

Returns:

  None.

--*/
{
  UINTN Index;

  NetZeroMem (V6, sizeof (EFI_IPv6_ADDRESS));

  V6->Addr[10]  = 0xff;
  V6->Addr[11]  = 0xff;

  for (Index = 0; Index < 4; Index++) {
    V6->Addr[12 + Index] = V4->Addr[Index];
  }
}

STATIC
UINT16
IScsiGetNICPciLocation (
  IN EFI_HANDLE  Controller
  )
/*++

Routine Description:

  Get the NIC's PCI location and return it accroding to the composited
  format defined in iSCSI Boot Firmware Table.
  
Arguments:

  Controller - The handle of the controller.

Returns:

  UINT16 - The composited representation of the NIC PCI location.

--*/
{
  EFI_STATUS                Status;
  EFI_DEVICE_PATH_PROTOCOL  *DevicePath;
  EFI_HANDLE                PciIoHandle;
  EFI_PCI_IO_PROTOCOL       *PciIo;
  UINTN                     Segment;
  UINTN                     Bus;
  UINTN                     Device;
  UINTN                     Function;

  Status = gBS->HandleProtocol (
                  Controller,
                  &gEfiDevicePathProtocolGuid,
                  &DevicePath
                  );
  if (EFI_ERROR (Status)) {
    return 0;
  }

  Status = gBS->LocateDevicePath (
                  &gEfiPciIoProtocolGuid,
                  &DevicePath,
                  &PciIoHandle
                  );
  if (EFI_ERROR (Status)) {
    return 0;
  }

  Status = gBS->HandleProtocol (PciIoHandle, &gEfiPciIoProtocolGuid, &PciIo);
  if (EFI_ERROR (Status)) {
    return 0;
  }

  Status = PciIo->GetLocation (PciIo, &Segment, &Bus, &Device, &Function);
  if (EFI_ERROR (Status)) {
    return 0;
  }

  return (UINT16) ((Bus << 8) | (Device << 3) | Function);
}

STATIC
EFI_MAC_ADDRESS *
IScsiGetMacAddress (
  IN EFI_HANDLE  Controller
  )
/*++

Routine Description:

  Get the MAC address of the controller.
  
Arguments:

  Controller - The handle of the controller.

Returns:

  EFI_MAC_ADDRESS * - The mac address.

--*/
{
  EFI_STATUS                  Status;
  EFI_SIMPLE_NETWORK_PROTOCOL *Snp;

  Status = gBS->HandleProtocol (
                  Controller,
                  &gEfiSimpleNetworkProtocolGuid,
                  &Snp
                  );
  ASSERT_EFI_ERROR (Status);

  return &Snp->Mode->PermanentAddress;
}

STATIC
VOID
IScsiFillNICAndTargetSections (
  IN     EFI_ACPI_ISCSI_BOOT_FIRMWARE_TABLE_HEADER  *Table,
  IN OUT UINT8                                      **Heap,
  IN     UINTN                                      HandleCount,
  IN     EFI_HANDLE                                 *Handles
  )
/*++

Routine Description:

  Fill the NIC and target sections in iSCSI Boot Firmware Table.

Arguments:

  Table       - The buffer of the ACPI table.
  Heap        - The heap buffer used to store the variable length parameters such as iSCSI name.
  HandleCount - The number of handles having iSCSI private protocol installed.
  Handles     - The handle buffer.

Returns:

  None.

--*/
{
  EFI_ACPI_ISCSI_BOOT_FIRMWARE_TABLE_CONTROL_STRUCTURE  *Control;
  EFI_ACPI_ISCSI_BOOT_FIRMWARE_TABLE_NIC_STRUCTURE      *Nic;
  EFI_ACPI_ISCSI_BOOT_FIRMWARE_TABLE_TARGET_STRUCTURE   *Target;
  ISCSI_DRIVER_DATA                                     *DriverData;
  ISCSI_SESSION_CONFIG_DATA                             *SessionConfigData;
  ISCSI_CHAP_AUTH_CONFIG_NVDATA                         *AuthConfig;
  UINT16                                                *SectionOffset;
  UINTN                                                 Index;
  UINT16                                                Length;
  EFI_MAC_ADDRESS                                       *Mac;
  ISCSI_PRIVATE_PROTOCOL                                *IScsiIdentifier;
  EFI_STATUS                                            Status;

  //
  // Get the offset of the first Nic and Target section.
  //
  Control = (EFI_ACPI_ISCSI_BOOT_FIRMWARE_TABLE_CONTROL_STRUCTURE *) (Table + 1);
  Nic     = (EFI_ACPI_ISCSI_BOOT_FIRMWARE_TABLE_NIC_STRUCTURE *) ((UINTN) Table +
          Control->InitiatorOffset + IBFT_ROUNDUP (sizeof (EFI_ACPI_ISCSI_BOOT_FIRMWARE_TABLE_INITIATOR_STRUCTURE)));
  Target  = (EFI_ACPI_ISCSI_BOOT_FIRMWARE_TABLE_TARGET_STRUCTURE *) ((UINTN) Nic +
          IBFT_ROUNDUP (sizeof (EFI_ACPI_ISCSI_BOOT_FIRMWARE_TABLE_NIC_STRUCTURE)));

  SectionOffset = &Control->NIC0Offset;

  for (Index = 0; Index < HandleCount; Index++) {
    Status = gBS->HandleProtocol (Handles[Index], &mIScsiPrivateGuid, &IScsiIdentifier);
    if (EFI_ERROR (Status)) {
      ASSERT (FALSE);
      return ;
    }

    DriverData        = ISCSI_DRIVER_DATA_FROM_IDENTIFIER (IScsiIdentifier);
    SessionConfigData = &DriverData->Session.ConfigData;
    AuthConfig        = &DriverData->Session.AuthData.AuthConfig;

    //
    // Fill the Nic section.
    //
    NetZeroMem (Nic, sizeof (EFI_ACPI_ISCSI_BOOT_FIRMWARE_TABLE_NIC_STRUCTURE));

    Nic->Header.StructureId = EFI_ACPI_ISCSI_BOOT_FIRMWARE_TABLE_NIC_STRUCTURE_ID;
    Nic->Header.Version     = EFI_ACPI_ISCSI_BOOT_FIRMWARE_TABLE_NIC_STRUCTURE_VERSION;
    Nic->Header.Length      = sizeof (EFI_ACPI_ISCSI_BOOT_FIRMWARE_TABLE_NIC_STRUCTURE);
    Nic->Header.Index       = (UINT8) Index;
    Nic->Header.Flags       = EFI_ACPI_ISCSI_BOOT_FIRMWARE_TABLE_NIC_STRUCTURE_FLAG_BLOCK_VALID |
                            EFI_ACPI_ISCSI_BOOT_FIRMWARE_TABLE_NIC_STRUCTURE_FLAG_BOOT_SELECTED |
                            EFI_ACPI_ISCSI_BOOT_FIRMWARE_TABLE_NIC_STRUCTURE_FLAG_GLOBAL;

    //
    // Get the subnet mask prefix length.
    //
    Nic->SubnetMaskPrefixLength = IScsiGetSubnetMaskPrefixLength (&SessionConfigData->NvData.SubnetMask);

    if (SessionConfigData->NvData.InitiatorInfoFromDhcp) {
      Nic->Origin = IpPrefixOriginDhcp;
    } else {
      Nic->Origin = IpPrefixOriginManual;
    }
    //
    // Map the various v4 addresses into v6 addresses.
    //
    IScsiMapV4ToV6Addr (&SessionConfigData->NvData.LocalIp, &Nic->Ip);
    IScsiMapV4ToV6Addr (&SessionConfigData->NvData.Gateway, &Nic->Gateway);
    IScsiMapV4ToV6Addr (&SessionConfigData->PrimaryDns, &Nic->PrimaryDns);
    IScsiMapV4ToV6Addr (&SessionConfigData->SecondaryDns, &Nic->SecondaryDns);
    IScsiMapV4ToV6Addr (&SessionConfigData->DhcpServer, &Nic->DhcpServer);

    Mac = IScsiGetMacAddress (DriverData->Controller);
    NetCopyMem (Nic->Mac, Mac, sizeof (Nic->Mac));

    //
    // Get the PCI location of the Nic.
    //
    Nic->PciLocation  = IScsiGetNICPciLocation (DriverData->Controller);

    *SectionOffset    = (UINT16) ((UINTN) Nic - (UINTN) Table);
    SectionOffset++;

    //
    // Fill the Target section.
    //
    NetZeroMem (Target, sizeof (EFI_ACPI_ISCSI_BOOT_FIRMWARE_TABLE_TARGET_STRUCTURE));

    Target->Header.StructureId  = EFI_ACPI_ISCSI_BOOT_FIRMWARE_TABLE_TARGET_STRUCTURE_ID;
    Target->Header.Version      = EFI_ACPI_ISCSI_BOOT_FIRMWARE_TABLE_TARGET_STRUCTURE_VERSION;
    Target->Header.Length       = sizeof (EFI_ACPI_ISCSI_BOOT_FIRMWARE_TABLE_TARGET_STRUCTURE);
    Target->Header.Index        = (UINT8) Index;
    Target->Header.Flags        = EFI_ACPI_ISCSI_BOOT_FIRMWARE_TABLE_TARGET_STRUCTURE_FLAG_BLOCK_VALID | EFI_ACPI_ISCSI_BOOT_FIRMWARE_TABLE_TARGET_STRUCTURE_FLAG_BOOT_SELECTED;
    Target->Port                = SessionConfigData->NvData.TargetPort;
    Target->CHAPType            = AuthConfig->CHAPType;
    Target->NicIndex            = (UINT8) Index;

    IScsiMapV4ToV6Addr (&SessionConfigData->NvData.TargetIp, &Target->Ip);
    NetCopyMem (Target->BootLun, SessionConfigData->NvData.BootLun, sizeof (Target->BootLun));

    //
    // Target iSCSI Name, CHAP name/secret, reverse CHAP name/secret.
    //
    Length = (UINT16) AsciiStrLen (SessionConfigData->NvData.TargetName);
    IScsiAddHeapItem (Heap, SessionConfigData->NvData.TargetName, Length);

    Target->IScsiNameLength = Length;
    Target->IScsiNameOffset = (UINT16) ((UINTN) *Heap - (UINTN) Table);

    if (Target->CHAPType != ISCSI_CHAP_NONE) {
      //
      // CHAP Name
      //
      Length = (UINT16) AsciiStrLen (AuthConfig->CHAPName);
      IScsiAddHeapItem (Heap, AuthConfig->CHAPName, Length);
      Target->CHAPNameLength  = Length;
      Target->CHAPNameOffset  = (UINT16) ((UINTN) *Heap - (UINTN) Table);

      //
      // CHAP Secret
      //
      Length = (UINT16) AsciiStrLen (AuthConfig->CHAPSecret);
      IScsiAddHeapItem (Heap, AuthConfig->CHAPSecret, Length);
      Target->CHAPSecretLength  = Length;
      Target->CHAPSecretOffset  = (UINT16) ((UINTN) *Heap - (UINTN) Table);

      if (Target->CHAPType == ISCSI_CHAP_MUTUAL) {
        //
        // Reverse CHAP Name
        //
        Length = (UINT16) AsciiStrLen (AuthConfig->ReverseCHAPName);
        IScsiAddHeapItem (Heap, AuthConfig->ReverseCHAPName, Length);
        Target->ReverseCHAPNameLength = Length;
        Target->ReverseCHAPNameOffset = (UINT16) ((UINTN) *Heap - (UINTN) Table);

        //
        // Reverse CHAP Secret
        //
        Length = (UINT16) AsciiStrLen (AuthConfig->ReverseCHAPSecret);
        IScsiAddHeapItem (Heap, AuthConfig->ReverseCHAPSecret, Length);
        Target->ReverseCHAPSecretLength = Length;
        Target->ReverseCHAPSecretOffset = (UINT16) ((UINTN) *Heap - (UINTN) Table);
      }
    }

    *SectionOffset = (UINT16) ((UINTN) Target - (UINTN) Table);
    SectionOffset++;

    //
    // Advance to the next NIC/Target pair
    //
    Nic    = (EFI_ACPI_ISCSI_BOOT_FIRMWARE_TABLE_NIC_STRUCTURE *) ((UINTN) Target +
           IBFT_ROUNDUP (sizeof (EFI_ACPI_ISCSI_BOOT_FIRMWARE_TABLE_TARGET_STRUCTURE)));
    Target = (EFI_ACPI_ISCSI_BOOT_FIRMWARE_TABLE_TARGET_STRUCTURE *) ((UINTN) Nic +
           IBFT_ROUNDUP (sizeof (EFI_ACPI_ISCSI_BOOT_FIRMWARE_TABLE_NIC_STRUCTURE)));
  }
}

VOID
IScsiPublishIbft (
  IN VOID
  )
/*++

Routine Description:

  Publish and remove the iSCSI Boot Firmware Table according to the iSCSI
  session status.

Arguments:

  None.

Returns:

  None.

--*/
{
  EFI_STATUS                                Status;
  UINTN                                     TableHandle;
  EFI_ACPI_SUPPORT_PROTOCOL                 *AcpiSupport;
  EFI_ACPI_ISCSI_BOOT_FIRMWARE_TABLE_HEADER *Table;
  UINTN                                     HandleCount;
  EFI_HANDLE                                *HandleBuffer;
  UINT8                                     *Heap;
  INTN                                      Index;
  EFI_ACPI_TABLE_VERSION                    Version;
  UINT32                                    Signature;

  Status = gBS->LocateProtocol (&gEfiAcpiSupportProtocolGuid, NULL, &AcpiSupport);
  if (EFI_ERROR (Status)) {
    return ;
  }
  //
  // Try to remove the old iSCSI Boot Firmware Table.
  //
  for (Index = 0;; Index++) {
    Status = AcpiSupport->GetAcpiTable (
                            AcpiSupport,
                            Index,
                            &Table,
                            &Version,
                            &TableHandle
                            );
    if (EFI_ERROR (Status)) {
      break;
    }

    Signature = Table->Signature;
    NetFreePool (Table);

    if (Signature == EFI_ACPI_3_0_ISCSI_BOOT_FIRMWARE_TABLE_SIGNATURE) {
      //
      // Remove the table.
      //
      Status = AcpiSupport->SetAcpiTable (
                              AcpiSupport,
                              NULL,
                              FALSE,
                              Version,
                              &TableHandle
                              );
      if (EFI_ERROR (Status)) {
        return ;
      }

      break;
    }
  }
  //
  // Get all iSCSI private protocols.
  //
  Status = gBS->LocateHandleBuffer (
                  ByProtocol,
                  &mIScsiPrivateGuid,
                  NULL,
                  &HandleCount,
                  &HandleBuffer
                  );
  if (EFI_ERROR (Status)) {
    return ;
  }
  //
  // Allocate 4k bytes to hold the ACPI table.
  //
  Table = NetAllocatePool (IBFT_MAX_SIZE);
  if (Table == NULL) {
    return ;
  }

  Heap = (CHAR8 *) Table + IBFT_HEAP_OFFSET;

  //
  // Fill in the various section of the iSCSI Boot Firmware Table.
  //
  IScsiInitIbfTableHeader (Table);
  IScsiInitControlSection (Table, HandleCount);
  IScsiFillInitiatorSection (Table, &Heap, HandleBuffer[0]);
  IScsiFillNICAndTargetSections (Table, &Heap, HandleCount, HandleBuffer);

  NetFreePool (HandleBuffer);

  TableHandle = 0;

  //
  // Install or update the iBFT table.
  //
  Status = AcpiSupport->SetAcpiTable (
                          AcpiSupport,
                          Table,
                          TRUE,
                          EFI_ACPI_TABLE_VERSION_3_0,
                          &TableHandle
                          );
  if (!EFI_ERROR (Status)) {
    AcpiSupport->PublishTables (AcpiSupport, EFI_ACPI_TABLE_VERSION_3_0);
  }

  NetFreePool (Table);
}
