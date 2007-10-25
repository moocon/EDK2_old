/** @file
  Copyright (c) 2006, Intel Corporation
  All rights reserved. This program and the accompanying materials
  are licensed and made available under the terms and conditions of the BSD License
  which accompanies this distribution.  The full text of the license may be found at
  http://opensource.org/licenses/bsd-license.php

  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

#include "AtapiPassThru.h"


static SCSI_COMMAND_SET     gEndTable = { 0xff, (DATA_DIRECTION) 0xff };

///
/// This table contains all the supported ATAPI commands.
///
static SCSI_COMMAND_SET     gSupportedATAPICommands[] = {
  { OP_INQUIRY,                     DataIn  },
  { OP_LOAD_UNLOAD_CD,              NoData  },
  { OP_MECHANISM_STATUS,            DataIn  },
  { OP_MODE_SELECT_10,              DataOut },
  { OP_MODE_SENSE_10,               DataIn  },
  { OP_PAUSE_RESUME,                NoData  },
  { OP_PLAY_AUDIO_10,               DataIn  },
  { OP_PLAY_AUDIO_MSF,              DataIn  },
  { OP_PLAY_CD,                     DataIn  },
  { OP_PLAY_CD_MSF,                 DataIn  },
  { OP_PREVENT_ALLOW_MEDIUM_REMOVAL,NoData  },
  { OP_READ_10,                     DataIn  },
  { OP_READ_12,                     DataIn  },
  { OP_READ_CAPACITY,               DataIn  },
  { OP_READ_CD,                     DataIn  },
  { OP_READ_CD_MSF,                 DataIn  },
  { OP_READ_HEADER,                 DataIn  },
  { OP_READ_SUB_CHANNEL,            DataIn  },
  { OP_READ_TOC,                    DataIn  },
  { OP_REQUEST_SENSE,               DataIn  },
  { OP_SCAN,                        NoData  },
  { OP_SEEK_10,                     NoData  },
  { OP_SET_CD_SPEED,                DataOut },
  { OP_STOPPLAY_SCAN,               NoData  },
  { OP_START_STOP_UNIT,             NoData  },
  { OP_TEST_UNIT_READY,             NoData  },
  { OP_FORMAT_UNIT,                 DataOut },
  { OP_READ_FORMAT_CAPACITIES,      DataIn  },
  { OP_VERIFY,                      DataOut },
  { OP_WRITE_10,                    DataOut },
  { OP_WRITE_12,                    DataOut },
  { OP_WRITE_AND_VERIFY,            DataOut },
  { 0xff,                           (DATA_DIRECTION) 0xff    }
};

static CHAR16               *gControllerNameString  = (CHAR16 *) L"ATAPI Controller";
static CHAR16               *gAtapiChannelString    = (CHAR16 *) L"ATAPI Channel";

EFI_DRIVER_BINDING_PROTOCOL gAtapiScsiPassThruDriverBinding = {
  AtapiScsiPassThruDriverBindingSupported,
  AtapiScsiPassThruDriverBindingStart,
  AtapiScsiPassThruDriverBindingStop,
  0xa,
  NULL,
  NULL
};

/**
  Supported.

  (Standard DriverBinding Protocol Supported() function)

  @return EFI_STATUS

  @todo    This - add argument and description to function comment
  @todo    Controller - add argument and description to function comment
  @todo    RemainingDevicePath - add argument and description to function comment
  @todo    EFI_UNSUPPORTED - add return value to function comment
**/
EFI_STATUS
EFIAPI
AtapiScsiPassThruDriverBindingSupported (
  IN EFI_DRIVER_BINDING_PROTOCOL  *This,
  IN EFI_HANDLE                   Controller,
  IN EFI_DEVICE_PATH_PROTOCOL     *RemainingDevicePath
  )
{
  EFI_STATUS          Status;
  EFI_PCI_IO_PROTOCOL *PciIo;
  PCI_TYPE00          Pci;


  //
  // Open the IO Abstraction(s) needed to perform the supported test
  //
  Status = gBS->OpenProtocol (
                  Controller,
                  &gEfiPciIoProtocolGuid,
                  (VOID **) &PciIo,
                  This->DriverBindingHandle,
                  Controller,
                  EFI_OPEN_PROTOCOL_BY_DRIVER
                  );
  if (EFI_ERROR (Status)) {
    return Status;
  }
  //
  // Use the PCI I/O Protocol to see if Controller is a IDE Controller that
  // can be managed by this driver.  Read the PCI Configuration Header
  // for this device.
  //
  Status = PciIo->Pci.Read (
                        PciIo,
                        EfiPciIoWidthUint32,
                        0,
                        sizeof (Pci) / sizeof (UINT32),
                        &Pci
                        );
  if (EFI_ERROR (Status)) {
    gBS->CloseProtocol (
           Controller,
           &gEfiPciIoProtocolGuid,
           This->DriverBindingHandle,
           Controller
           );
    return EFI_UNSUPPORTED;
  }

  if (Pci.Hdr.ClassCode[2] != PCI_CLASS_MASS_STORAGE || Pci.Hdr.ClassCode[1] != PCI_CLASS_IDE) {

    Status = EFI_UNSUPPORTED;
  }

  gBS->CloseProtocol (
         Controller,
         &gEfiPciIoProtocolGuid,
         This->DriverBindingHandle,
         Controller
         );

  return Status;
}

/**
  Create handles for IDE channels specified by RemainingDevicePath.
  Install SCSI Pass Thru Protocol onto each created handle.

  (Standard DriverBinding Protocol Start() function)

  @return EFI_STATUS

  @todo    This - add argument and description to function comment
  @todo    Controller - add argument and description to function comment
  @todo    RemainingDevicePath - add argument and description to function comment
**/
EFI_STATUS
EFIAPI
AtapiScsiPassThruDriverBindingStart (
  IN EFI_DRIVER_BINDING_PROTOCOL  *This,
  IN EFI_HANDLE                   Controller,
  IN EFI_DEVICE_PATH_PROTOCOL     *RemainingDevicePath
  )
{
  EFI_STATUS          Status;
  EFI_PCI_IO_PROTOCOL *PciIo;
  UINT64              Supports;
  UINT64              OriginalPciAttributes;

  PciIo = NULL;
  Status = gBS->OpenProtocol (
                  Controller,
                  &gEfiPciIoProtocolGuid,
                  (VOID **) &PciIo,
                  This->DriverBindingHandle,
                  Controller,
                  EFI_OPEN_PROTOCOL_BY_DRIVER
                  );
  if (EFI_ERROR (Status)) {
    return Status;
  }

  //
  // Save original PCI attributes
  //
  Status = PciIo->Attributes (
                    PciIo,
                    EfiPciIoAttributeOperationGet,
                    0,
                    &OriginalPciAttributes
                    );

  if (EFI_ERROR (Status)) {
    return Status;
  }

  Status = PciIo->Attributes (
                    PciIo,
                    EfiPciIoAttributeOperationSupported,
                    0,
                    &Supports
                    );
  if (!EFI_ERROR (Status)) {
    Supports &= (EFI_PCI_DEVICE_ENABLE               |
                 EFI_PCI_IO_ATTRIBUTE_IDE_PRIMARY_IO |
                 EFI_PCI_IO_ATTRIBUTE_IDE_SECONDARY_IO);
    Status = PciIo->Attributes (
                      PciIo,
                      EfiPciIoAttributeOperationEnable,
                      Supports,
                      NULL
                      );
  }
  if (EFI_ERROR (Status)) {
    goto Done;
  }

  //
  // Create SCSI Pass Thru instance for the IDE channel.
  //
  Status = RegisterAtapiScsiPassThru (This, Controller, PciIo, OriginalPciAttributes);

Done:
  if (EFI_ERROR (Status)) {
    //
    // Restore original PCI attributes
    //
    PciIo->Attributes (
                    PciIo,
                    EfiPciIoAttributeOperationSet,
                    OriginalPciAttributes,
                    NULL
                    );

    gBS->CloseProtocol (
           Controller,
           &gEfiPciIoProtocolGuid,
           This->DriverBindingHandle,
           Controller
           );
  }

  return Status;
}

/**
  Stop.

  (Standard DriverBinding Protocol Stop() function)

  @return EFI_STATUS

  @todo    This - add argument and description to function comment
  @todo    Controller - add argument and description to function comment
  @todo    NumberOfChildren - add argument and description to function comment
  @todo    ChildHandleBuffer - add argument and description to function comment
  @todo    EFI_SUCCESS - add return value to function comment
**/
EFI_STATUS
EFIAPI
AtapiScsiPassThruDriverBindingStop (
  IN  EFI_DRIVER_BINDING_PROTOCOL     *This,
  IN  EFI_HANDLE                      Controller,
  IN  UINTN                           NumberOfChildren,
  IN  EFI_HANDLE                      *ChildHandleBuffer
  )
{
  EFI_STATUS                  Status;
  EFI_SCSI_PASS_THRU_PROTOCOL *ScsiPassThru;
  ATAPI_SCSI_PASS_THRU_DEV    *AtapiScsiPrivate;

  Status = gBS->OpenProtocol (
                  Controller,
                  &gEfiScsiPassThruProtocolGuid,
                  (VOID **) &ScsiPassThru,
                  This->DriverBindingHandle,
                  Controller,
                  EFI_OPEN_PROTOCOL_GET_PROTOCOL
                  );
  if (EFI_ERROR (Status)) {
    return Status;
  }

  AtapiScsiPrivate = ATAPI_SCSI_PASS_THRU_DEV_FROM_THIS (ScsiPassThru);

  Status = gBS->UninstallProtocolInterface (
                  Controller,
                  &gEfiScsiPassThruProtocolGuid,
                  &AtapiScsiPrivate->ScsiPassThru
                  );
  if (EFI_ERROR (Status)) {
    return Status;
  }

  //
  // Restore original PCI attributes
  //
  AtapiScsiPrivate->PciIo->Attributes (
                  AtapiScsiPrivate->PciIo,
                  EfiPciIoAttributeOperationSet,
                  AtapiScsiPrivate->OriginalPciAttributes,
                  NULL
                  );

  gBS->CloseProtocol (
         Controller,
         &gEfiPciIoProtocolGuid,
         This->DriverBindingHandle,
         Controller
         );

  gBS->FreePool (AtapiScsiPrivate);

  return EFI_SUCCESS;
}

/**
  Attaches SCSI Pass Thru Protocol for specified IDE channel.

  @param Controller:            Parent device handle to the IDE channel.
  @param PciIo:                 PCI I/O protocol attached on the "Controller".
  @param OriginalPciAttributes  Original PCI attributes


  @return EFI_SUCCESS Always returned unless installing SCSI Pass Thru Protocol failed.

  @todo    This - add argument and description to function comment
  @todo    Controller - add argument and description to function comment
  @todo    PciIo - add argument and description to function comment
  @todo    EFI_OUT_OF_RESOURCES - add return value to function comment
**/
EFI_STATUS
RegisterAtapiScsiPassThru (
  IN EFI_DRIVER_BINDING_PROTOCOL  *This,
  IN  EFI_HANDLE                  Controller,
  IN  EFI_PCI_IO_PROTOCOL         *PciIo,
  IN  UINT64                      OriginalPciAttributes
  )
{
  EFI_STATUS                Status;
  ATAPI_SCSI_PASS_THRU_DEV  *AtapiScsiPrivate;
  IDE_REGISTERS_BASE_ADDR   IdeRegsBaseAddr[ATAPI_MAX_CHANNEL];

  AtapiScsiPrivate = AllocateZeroPool (sizeof (ATAPI_SCSI_PASS_THRU_DEV));
  if (AtapiScsiPrivate == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }

  CopyMem (AtapiScsiPrivate->ChannelName, gAtapiChannelString, sizeof (gAtapiChannelString));

  AtapiScsiPrivate->Signature = ATAPI_SCSI_PASS_THRU_DEV_SIGNATURE;
  AtapiScsiPrivate->Handle    = Controller;

  //
  // will reset the IoPort inside each API function.
  //
  AtapiScsiPrivate->IoPort                = NULL;
  AtapiScsiPrivate->PciIo                 = PciIo;
  AtapiScsiPrivate->OriginalPciAttributes = OriginalPciAttributes;

  //
  // Obtain IDE IO port registers' base addresses
  //
  Status = GetIdeRegistersBaseAddr (PciIo, IdeRegsBaseAddr);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  InitAtapiIoPortRegisters(AtapiScsiPrivate, IdeRegsBaseAddr);

  // initialize SCSI Pass Thru Protocol interface
  //
  AtapiScsiPrivate->ScsiPassThru.Mode             = &AtapiScsiPrivate->ScsiPassThruMode;
  AtapiScsiPrivate->ScsiPassThru.PassThru         = AtapiScsiPassThruFunction;
  AtapiScsiPrivate->ScsiPassThru.GetNextDevice    = AtapiScsiPassThruGetNextDevice;
  AtapiScsiPrivate->ScsiPassThru.BuildDevicePath  = AtapiScsiPassThruBuildDevicePath;
  AtapiScsiPrivate->ScsiPassThru.GetTargetLun     = AtapiScsiPassThruGetTargetLun;
  AtapiScsiPrivate->ScsiPassThru.ResetChannel     = AtapiScsiPassThruResetChannel;
  AtapiScsiPrivate->ScsiPassThru.ResetTarget      = AtapiScsiPassThruResetTarget;

  //
  // Set Mode
  //
  CopyMem (AtapiScsiPrivate->ControllerName, gControllerNameString, sizeof (gControllerNameString));

  AtapiScsiPrivate->ScsiPassThruMode.ControllerName = AtapiScsiPrivate->ControllerName;
  AtapiScsiPrivate->ScsiPassThruMode.ChannelName    = AtapiScsiPrivate->ChannelName;
  AtapiScsiPrivate->ScsiPassThruMode.AdapterId      = 4;
  //
  // non-RAID SCSI controllers should set both physical and logical attributes
  //
  AtapiScsiPrivate->ScsiPassThruMode.Attributes = EFI_SCSI_PASS_THRU_ATTRIBUTES_PHYSICAL |
                                                  EFI_SCSI_PASS_THRU_ATTRIBUTES_LOGICAL;
  AtapiScsiPrivate->ScsiPassThruMode.IoAlign = 0;

  //
  // Initialize the LatestTargetId.
  //
  AtapiScsiPrivate->LatestTargetId  = 4;
  AtapiScsiPrivate->LatestLun       = 0;

  Status = gBS->InstallProtocolInterface (
                  &Controller,
                  &gEfiScsiPassThruProtocolGuid,
                  EFI_NATIVE_INTERFACE,
                  &AtapiScsiPrivate->ScsiPassThru
                  );
  return Status;
}

/**
  Implements EFI_SCSI_PASS_THRU_PROTOCOL.PassThru() function.

  @param This     The EFI_SCSI_PASS_THRU_PROTOCOL instance.
  @param Target   The Target ID of the ATAPI device to send the SCSI
  Request Packet. To ATAPI devices attached on an IDE
  Channel, Target ID 0 indicates Master device;Target
  ID 1 indicates Slave device.
  @param Lun      The LUN of the ATAPI device to send the SCSI Request
  Packet. To the ATAPI device, Lun is always 0.
  @param Packet   The SCSI Request Packet to send to the ATAPI device
  specified by Target and Lun.
  @param Event    If non-blocking I/O is not supported then Event is ignored,
  and blocking I/O is performed.<br>
  If Event is NULL, then blocking I/O is performed.<br>
  If Event is not NULL and non blocking I/O is supported,
  then non-blocking I/O is performed, and Event will be signaled
  when the SCSI Request Packet completes.

  @todo    This - add argument and description to function comment
  @todo    EFI_INVALID_PARAMETER - add return value to function comment
  @todo    EFI_SUCCESS - add return value to function comment
**/
EFI_STATUS
EFIAPI
AtapiScsiPassThruFunction (
  IN EFI_SCSI_PASS_THRU_PROTOCOL                        *This,
  IN UINT32                                             Target,
  IN UINT64                                             Lun,
  IN OUT EFI_SCSI_PASS_THRU_SCSI_REQUEST_PACKET         *Packet,
  IN EFI_EVENT                                          Event OPTIONAL
  )
{
  ATAPI_SCSI_PASS_THRU_DEV               *AtapiScsiPrivate;
  EFI_STATUS                             Status;

  AtapiScsiPrivate = ATAPI_SCSI_PASS_THRU_DEV_FROM_THIS (This);

  //
  // Target is not allowed beyond MAX_TARGET_ID
  //
  if ((Target > MAX_TARGET_ID) || (Lun != 0)) {
    return EFI_INVALID_PARAMETER;
  }

  //
  // check the data fields in Packet parameter.
  //
  Status = CheckSCSIRequestPacket (Packet);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  //
  // If Request Packet targets at the IDE channel itself,
  // do nothing.
  //
  if (Target == This->Mode->AdapterId) {
    Packet->TransferLength = 0;
    return EFI_SUCCESS;
  }

  //
  // According to Target ID, reset the Atapi I/O Register mapping
  // (Target Id in [0,1] area, using AtapiIoPortRegisters[0],
  //  Target Id in [2,3] area, using AtapiIoPortRegisters[1]
  //
  if ((Target / 2) == 0) {
    Target = Target % 2;
    AtapiScsiPrivate->IoPort = &AtapiScsiPrivate->AtapiIoPortRegisters[0];
  } else {
    Target = Target % 2;
    AtapiScsiPrivate->IoPort = &AtapiScsiPrivate->AtapiIoPortRegisters[1];
  }

  //
  // the ATAPI SCSI interface does not support non-blocking I/O
  // ignore the Event parameter
  //
  // Performs blocking I/O.
  //
  Status = SubmitBlockingIoCommand (AtapiScsiPrivate, Target, Packet);
  return Status;
}

/**
  Used to retrieve the list of legal Target IDs for SCSI devices
  on a SCSI channel.

  @param  This Protocol instance pointer.
  @param  Target On input, a pointer to the Target ID of a SCSI
  device present on the SCSI channel.  On output,
  a pointer to the Target ID of the next SCSI device
  present on a SCSI channel.  An input value of
  0xFFFFFFFF retrieves the Target ID of the first
  SCSI device present on a SCSI channel.
  @param  Lun On input, a pointer to the LUN of a SCSI device
  present on the SCSI channel. On output, a pointer
  to the LUN of the next SCSI device present on
  a SCSI channel.

  @retval  EFI_SUCCESS The Target ID and Lun of the next SCSI device
  on the SCSI channel was returned in Target and Lun.
  @retval  EFI_NOT_FOUND There are no more SCSI devices on this SCSI channel.
  @retval  EFI_INVALID_PARAMETER Target is not 0xFFFFFFFF,and Target and Lun were not
  returned on a previous call to GetNextDevice().

**/
EFI_STATUS
EFIAPI
AtapiScsiPassThruGetNextDevice (
  IN  EFI_SCSI_PASS_THRU_PROTOCOL    *This,
  IN OUT UINT32                      *Target,
  IN OUT UINT64                      *Lun
  )
{
  ATAPI_SCSI_PASS_THRU_DEV  *AtapiScsiPrivate;

  //
  // Retrieve Device Private Data Structure.
  //
  AtapiScsiPrivate = ATAPI_SCSI_PASS_THRU_DEV_FROM_THIS (This);

  //
  // Check whether Target is valid.
  //
  if (Target == NULL || Lun == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  if ((*Target != 0xFFFFFFFF) &&
      ((*Target != AtapiScsiPrivate->LatestTargetId) ||
      (*Lun != AtapiScsiPrivate->LatestLun))) {
    return EFI_INVALID_PARAMETER;
  }

  if (*Target == MAX_TARGET_ID) {
    return EFI_NOT_FOUND;
  }

  if (*Target == 0xFFFFFFFF) {
    *Target = 0;
  } else {
    *Target = AtapiScsiPrivate->LatestTargetId + 1;
  }

  *Lun = 0;

  //
  // Update the LatestTargetId.
  //
  AtapiScsiPrivate->LatestTargetId  = *Target;
  AtapiScsiPrivate->LatestLun       = *Lun;

  return EFI_SUCCESS;

}

/**
  Used to allocate and build a device path node for a SCSI device
  on a SCSI channel. Would not build device path for a SCSI Host Controller.

  @param  This Protocol instance pointer.
  @param  Target The Target ID of the SCSI device for which
  a device path node is to be allocated and built.
  @param  Lun The LUN of the SCSI device for which a device
  path node is to be allocated and built.
  @param  DevicePath A pointer to a single device path node that
  describes the SCSI device specified by
  Target and Lun. This function is responsible
  for allocating the buffer DevicePath with the boot
  service AllocatePool().  It is the caller's
  responsibility to free DevicePath when the caller
  is finished with DevicePath.

  @retval  EFI_SUCCESS The device path node that describes the SCSI device
  specified by Target and Lun was allocated and
  returned in DevicePath.
  @retval  EFI_NOT_FOUND The SCSI devices specified by Target and Lun does
  not exist on the SCSI channel.
  @retval  EFI_INVALID_PARAMETER DevicePath is NULL.
  @retval  EFI_OUT_OF_RESOURCES There are not enough resources to allocate
  DevicePath.

**/
EFI_STATUS
EFIAPI
AtapiScsiPassThruBuildDevicePath (
  IN     EFI_SCSI_PASS_THRU_PROTOCOL    *This,
  IN     UINT32                         Target,
  IN     UINT64                         Lun,
  IN OUT EFI_DEVICE_PATH_PROTOCOL       **DevicePath
  )
{
  EFI_DEV_PATH              *Node;


  //
  // Validate parameters passed in.
  //

  if (DevicePath == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  //
  // can not build device path for the SCSI Host Controller.
  //
  if ((Target > (MAX_TARGET_ID - 1)) || (Lun != 0)) {
    return EFI_NOT_FOUND;
  }

  Node = AllocateZeroPool (sizeof (EFI_DEV_PATH));
  if (Node == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }

  Node->DevPath.Type    = MESSAGING_DEVICE_PATH;
  Node->DevPath.SubType = MSG_ATAPI_DP;
  SetDevicePathNodeLength (&Node->DevPath, sizeof (ATAPI_DEVICE_PATH));

  Node->Atapi.PrimarySecondary  = (UINT8) (Target / 2);
  Node->Atapi.SlaveMaster       = (UINT8) (Target % 2);
  Node->Atapi.Lun               = (UINT16) Lun;

  *DevicePath                   = (EFI_DEVICE_PATH_PROTOCOL *) Node;

  return EFI_SUCCESS;
}

/**
  Used to translate a device path node to a Target ID and LUN.

  @param  This Protocol instance pointer.
  @param  DevicePath A pointer to the device path node that
  describes a SCSI device on the SCSI channel.
  @param  Target A pointer to the Target ID of a SCSI device
  on the SCSI channel.
  @param  Lun A pointer to the LUN of a SCSI device on
  the SCSI channel.

  @retval  EFI_SUCCESS DevicePath was successfully translated to a
  Target ID and LUN, and they were returned
  in Target and Lun.
  @retval  EFI_INVALID_PARAMETER DevicePath is NULL.
  @retval  EFI_INVALID_PARAMETER Target is NULL.
  @retval  EFI_INVALID_PARAMETER Lun is NULL.
  @retval  EFI_UNSUPPORTED This driver does not support the device path
  node type in DevicePath.
  @retval  EFI_NOT_FOUND A valid translation from DevicePath to a
  Target ID and LUN does not exist.

**/
EFI_STATUS
EFIAPI
AtapiScsiPassThruGetTargetLun (
  IN  EFI_SCSI_PASS_THRU_PROTOCOL    *This,
  IN  EFI_DEVICE_PATH_PROTOCOL       *DevicePath,
  OUT UINT32                         *Target,
  OUT UINT64                         *Lun
  )
{
  EFI_DEV_PATH  *Node;

  //
  // Validate parameters passed in.
  //
  if (DevicePath == NULL || Target == NULL || Lun == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  //
  // Check whether the DevicePath belongs to SCSI_DEVICE_PATH
  //
  if ((DevicePath->Type != MESSAGING_DEVICE_PATH) ||
      (DevicePath->SubType != MSG_ATAPI_DP) ||
      (DevicePathNodeLength(DevicePath) != sizeof(ATAPI_DEVICE_PATH))) {
    return EFI_UNSUPPORTED;
  }

  Node    = (EFI_DEV_PATH *) DevicePath;

  *Target = Node->Atapi.PrimarySecondary * 2 + Node->Atapi.SlaveMaster;
  *Lun    = Node->Atapi.Lun;

  if (*Target > (MAX_TARGET_ID - 1) || *Lun != 0) {
    return EFI_NOT_FOUND;
  }

  return EFI_SUCCESS;
}

/**
  Resets a SCSI channel.This operation resets all the
  SCSI devices connected to the SCSI channel.

  @param  This Protocol instance pointer.

  @retval  EFI_SUCCESS The SCSI channel was reset.
  @retval  EFI_UNSUPPORTED The SCSI channel does not support
  a channel reset operation.
  @retval  EFI_DEVICE_ERROR A device error occurred while
  attempting to reset the SCSI channel.
  @retval  EFI_TIMEOUT A timeout occurred while attempting
  to reset the SCSI channel.

**/
EFI_STATUS
EFIAPI
AtapiScsiPassThruResetChannel (
  IN  EFI_SCSI_PASS_THRU_PROTOCOL   *This
  )
{
  UINT8                     DeviceControlValue;
  ATAPI_SCSI_PASS_THRU_DEV  *AtapiScsiPrivate;
  UINT8                     Index;
  BOOLEAN                   ResetFlag;

  AtapiScsiPrivate = ATAPI_SCSI_PASS_THRU_DEV_FROM_THIS (This);
  ResetFlag = FALSE;

  //
  // Reset both Primary channel and Secondary channel.
  // so, the IoPort pointer must point to the right I/O Register group
  //
  for (Index = 0; Index < 2; Index++) {
    //
    // Reset
    //
    AtapiScsiPrivate->IoPort  = &AtapiScsiPrivate->AtapiIoPortRegisters[Index];

    DeviceControlValue        = 0;
    //
    // set SRST bit to initiate soft reset
    //
    DeviceControlValue |= SRST;
    //
    // disable Interrupt
    //
    DeviceControlValue |= bit (1);
    WritePortB (
      AtapiScsiPrivate->PciIo,
      AtapiScsiPrivate->IoPort->Alt.DeviceControl,
      DeviceControlValue
      );

    //
    // Wait 10us
    //
    gBS->Stall (10);

    //
    // Clear SRST bit
    // 0xfb:1111,1011
    //
    DeviceControlValue &= 0xfb;

    WritePortB (AtapiScsiPrivate->PciIo, AtapiScsiPrivate->IoPort->Alt.DeviceControl, DeviceControlValue);

    //
    // slave device needs at most 31s to clear BSY
    //
    if (StatusWaitForBSYClear (AtapiScsiPrivate, 31000000) != EFI_TIMEOUT) {
      ResetFlag = TRUE;
    }
  }

  if (ResetFlag) {
    return EFI_SUCCESS;
  }

  return EFI_TIMEOUT;
}

/**
  Resets a SCSI device that is connected to a SCSI channel.

  @param  This Protocol instance pointer.
  @param  Target The Target ID of the SCSI device to reset.
  @param  Lun The LUN of the SCSI device to reset.

  @retval  EFI_SUCCESS The SCSI device specified by Target and
  Lun was reset.
  @retval  EFI_UNSUPPORTED The SCSI channel does not support a target
  reset operation.
  @retval  EFI_INVALID_PARAMETER Target or Lun are invalid.
  @retval  EFI_DEVICE_ERROR A device error occurred while attempting
  to reset the SCSI device specified by Target
  and Lun.
  @retval  EFI_TIMEOUT A timeout occurred while attempting to reset
  the SCSI device specified by Target and Lun.

**/
EFI_STATUS
EFIAPI
AtapiScsiPassThruResetTarget (
  IN EFI_SCSI_PASS_THRU_PROTOCOL    *This,
  IN UINT32                         Target,
  IN UINT64                         Lun
  )
{
  ATAPI_SCSI_PASS_THRU_DEV  *AtapiScsiPrivate;
  UINT8                     Command;
  UINT8                     DeviceSelect;

  AtapiScsiPrivate = ATAPI_SCSI_PASS_THRU_DEV_FROM_THIS (This);

  if ((Target > MAX_TARGET_ID) || (Lun != 0)) {
    return EFI_INVALID_PARAMETER;
  }
  //
  // Directly return EFI_SUCCESS if want to reset the host controller
  //
  if (Target == This->Mode->AdapterId) {
    return EFI_SUCCESS;
  }

  //
  // According to Target ID, reset the Atapi I/O Register mapping
  // (Target Id in [0,1] area, using AtapiIoPortRegisters[0],
  //  Target Id in [2,3] area, using AtapiIoPortRegisters[1]
  //
  if ((Target / 2) == 0) {
    AtapiScsiPrivate->IoPort = &AtapiScsiPrivate->AtapiIoPortRegisters[0];
  } else {
    AtapiScsiPrivate->IoPort = &AtapiScsiPrivate->AtapiIoPortRegisters[1];
  }

  //
  // for ATAPI device, no need to wait DRDY ready after device selecting.
  //
  // bit7 and bit5 are both set to 1 for backward compatibility
  //
  DeviceSelect = (UINT8) (((bit (7) | bit (5)) | (Target << 4)));
  WritePortB (AtapiScsiPrivate->PciIo, AtapiScsiPrivate->IoPort->Head, DeviceSelect);

  Command = ATAPI_SOFT_RESET_CMD;
  WritePortB (AtapiScsiPrivate->PciIo, AtapiScsiPrivate->IoPort->Reg.Command, Command);

  //
  // BSY clear is the only status return to the host by the device
  // when reset is complete.
  // slave device needs at most 31s to clear BSY
  //
  if (EFI_ERROR (StatusWaitForBSYClear (AtapiScsiPrivate, 31000000))) {
    return EFI_TIMEOUT;
  }

  //
  // stall 5 seconds to make the device status stable
  //
  gBS->Stall (5000000);

  return EFI_SUCCESS;
}

EFI_STATUS
GetIdeRegistersBaseAddr (
  IN  EFI_PCI_IO_PROTOCOL         *PciIo,
  OUT IDE_REGISTERS_BASE_ADDR     *IdeRegsBaseAddr
  )
/*++

Routine Description:
  Get IDE IO port registers' base addresses by mode. In 'Compatibility' mode,
  use fixed addresses. In Native-PCI mode, get base addresses from BARs in
  the PCI IDE controller's Configuration Space.

Arguments:
  PciIo             - Pointer to the EFI_PCI_IO_PROTOCOL instance
  IdeRegsBaseAddr   - Pointer to IDE_REGISTERS_BASE_ADDR to
                      receive IDE IO port registers' base addresses

Returns:

  EFI_STATUS

--*/
{
  EFI_STATUS  Status;
  PCI_TYPE00  PciData;

  Status = PciIo->Pci.Read (
                        PciIo,
                        EfiPciIoWidthUint8,
                        0,
                        sizeof (PciData),
                        &PciData
                        );

  if (EFI_ERROR (Status)) {
    return Status;
  }

  if ((PciData.Hdr.ClassCode[0] & IDE_PRIMARY_OPERATING_MODE) == 0) {
    IdeRegsBaseAddr[IdePrimary].CommandBlockBaseAddr  = 0x1f0;
    IdeRegsBaseAddr[IdePrimary].ControlBlockBaseAddr  = 0x3f6;
  } else {
    //
    // The BARs should be of IO type
    //
    if ((PciData.Device.Bar[0] & BIT0) == 0 ||
        (PciData.Device.Bar[1] & BIT0) == 0) {
      return EFI_UNSUPPORTED;
    }

    IdeRegsBaseAddr[IdePrimary].CommandBlockBaseAddr  =
    (UINT16) (PciData.Device.Bar[0] & 0x0000fff8);
    IdeRegsBaseAddr[IdePrimary].ControlBlockBaseAddr  =
    (UINT16) ((PciData.Device.Bar[1] & 0x0000fffc) + 2);
  }

  if ((PciData.Hdr.ClassCode[0] & IDE_SECONDARY_OPERATING_MODE) == 0) {
    IdeRegsBaseAddr[IdeSecondary].CommandBlockBaseAddr  = 0x170;
    IdeRegsBaseAddr[IdeSecondary].ControlBlockBaseAddr  = 0x376;
  } else {
    //
    // The BARs should be of IO type
    //
    if ((PciData.Device.Bar[2] & BIT0) == 0 ||
        (PciData.Device.Bar[3] & BIT0) == 0) {
      return EFI_UNSUPPORTED;
    }

    IdeRegsBaseAddr[IdeSecondary].CommandBlockBaseAddr  =
    (UINT16) (PciData.Device.Bar[2] & 0x0000fff8);
    IdeRegsBaseAddr[IdeSecondary].ControlBlockBaseAddr  =
    (UINT16) ((PciData.Device.Bar[3] & 0x0000fffc) + 2);
  }

  return EFI_SUCCESS;
}

VOID
InitAtapiIoPortRegisters (
  IN  ATAPI_SCSI_PASS_THRU_DEV     *AtapiScsiPrivate,
  IN  IDE_REGISTERS_BASE_ADDR      *IdeRegsBaseAddr
  )
/*++

Routine Description:

  Initialize each Channel's Base Address of CommandBlock and ControlBlock.

Arguments:

  AtapiScsiPrivate            - The pointer of ATAPI_SCSI_PASS_THRU_DEV
  IdeRegsBaseAddr             - The pointer of IDE_REGISTERS_BASE_ADDR

Returns:

  None

--*/
{

  UINT8               IdeChannel;
  UINT16              CommandBlockBaseAddr;
  UINT16              ControlBlockBaseAddr;
  IDE_BASE_REGISTERS  *RegisterPointer;


  for (IdeChannel = 0; IdeChannel < ATAPI_MAX_CHANNEL; IdeChannel++) {

    RegisterPointer =  &AtapiScsiPrivate->AtapiIoPortRegisters[IdeChannel];

    //
    // Initialize IDE IO port addresses, including Command Block registers
    // and Control Block registers
    //
    CommandBlockBaseAddr = IdeRegsBaseAddr[IdeChannel].CommandBlockBaseAddr;
    ControlBlockBaseAddr = IdeRegsBaseAddr[IdeChannel].ControlBlockBaseAddr;

    RegisterPointer->Data = CommandBlockBaseAddr;
    (*(UINT16 *) &RegisterPointer->Reg1) = (UINT16) (CommandBlockBaseAddr + 0x01);
    RegisterPointer->SectorCount = (UINT16) (CommandBlockBaseAddr + 0x02);
    RegisterPointer->SectorNumber = (UINT16) (CommandBlockBaseAddr + 0x03);
    RegisterPointer->CylinderLsb = (UINT16) (CommandBlockBaseAddr + 0x04);
    RegisterPointer->CylinderMsb = (UINT16) (CommandBlockBaseAddr + 0x05);
    RegisterPointer->Head = (UINT16) (CommandBlockBaseAddr + 0x06);
    (*(UINT16 *) &RegisterPointer->Reg) = (UINT16) (CommandBlockBaseAddr + 0x07);

    (*(UINT16 *) &RegisterPointer->Alt) = ControlBlockBaseAddr;
    RegisterPointer->DriveAddress = (UINT16) (ControlBlockBaseAddr + 0x01);
  }

}


EFI_STATUS
CheckSCSIRequestPacket (
  EFI_SCSI_PASS_THRU_SCSI_REQUEST_PACKET      *Packet
  )
/*++

Routine Description:

  Checks the parameters in the SCSI Request Packet to make sure
  they are valid for a SCSI Pass Thru request.

Arguments:

  Packet         -  The pointer of EFI_SCSI_PASS_THRU_SCSI_REQUEST_PACKET

Returns:

  EFI_STATUS

--*/
{
  if (Packet == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  if (!ValidCdbLength (Packet->CdbLength)) {
    return EFI_INVALID_PARAMETER;
  }

  if (Packet->Cdb == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  //
  // Checks whether the request command is supported.
  //
  if (!IsCommandValid (Packet)) {
    return EFI_UNSUPPORTED;
  }

  return EFI_SUCCESS;
}

/**
  Checks the requested SCSI command:
  Is it supported by this driver?
  Is the Data transfer direction reasonable?

  @todo function comment is missing 'Routine Description:'
  @todo function comment is missing 'Arguments:'
  @todo function comment is missing 'Returns:'
  @todo    Packet - add argument and description to function comment
**/
BOOLEAN
IsCommandValid (
  EFI_SCSI_PASS_THRU_SCSI_REQUEST_PACKET   *Packet
  )
{
  UINT8 Index;
  UINT8 *OpCode;

  OpCode = (UINT8 *) (Packet->Cdb);

  for (Index = 0; CompareMem (&gSupportedATAPICommands[Index], &gEndTable, sizeof (SCSI_COMMAND_SET)); Index++) {

    if (*OpCode == gSupportedATAPICommands[Index].OpCode) {
      //
      // Check whether the requested Command is supported by this driver
      //
      if (Packet->DataDirection == DataIn) {
        //
        // Check whether the requested data direction conforms to
        // what it should be.
        //
        if (gSupportedATAPICommands[Index].Direction == DataOut) {
          return FALSE;
        }
      }

      if (Packet->DataDirection == DataOut) {
        //
        // Check whether the requested data direction conforms to
        // what it should be.
        //
        if (gSupportedATAPICommands[Index].Direction == DataIn) {
          return FALSE;
        }
      }

      return TRUE;
    }
  }

  return FALSE;
}

/**
  Performs blocking I/O request.

  @param AtapiScsiPrivate   Private data structure for the specified channel.
  @param Target             The Target ID of the ATAPI device to send the SCSI
  Request Packet. To ATAPI devices attached on an IDE
  Channel, Target ID 0 indicates Master device;Target
  ID 1 indicates Slave device.
  @param Packet             The SCSI Request Packet to send to the ATAPI device
  specified by Target.

  @todo    AtapiScsiPrivate - add argument and description to function comment
**/
EFI_STATUS
SubmitBlockingIoCommand (
  ATAPI_SCSI_PASS_THRU_DEV                  *AtapiScsiPrivate,
  UINT32                                    Target,
  EFI_SCSI_PASS_THRU_SCSI_REQUEST_PACKET    *Packet
  )
{
  UINT8       PacketCommand[12];
  UINT64      TimeoutInMicroSeconds;
  EFI_STATUS  PacketCommandStatus;

  //
  // Fill ATAPI Command Packet according to CDB
  //
  ZeroMem (&PacketCommand, 12);
  CopyMem (&PacketCommand, Packet->Cdb, Packet->CdbLength);

  //
  // Timeout is 100ns unit, convert it to 1000ns (1us) unit.
  //
  TimeoutInMicroSeconds = DivU64x32 (Packet->Timeout, (UINT32) 10);

  //
  // Submit ATAPI Command Packet
  //
  PacketCommandStatus = AtapiPacketCommand (
                          AtapiScsiPrivate,
                          Target,
                          PacketCommand,
                          Packet->DataBuffer,
                          &(Packet->TransferLength),
                          (DATA_DIRECTION) Packet->DataDirection,
                          TimeoutInMicroSeconds
                          );
  if (!EFI_ERROR (PacketCommandStatus) || (Packet->SenseData == NULL)) {
    Packet->SenseDataLength = 0;
    return PacketCommandStatus;
  }

  //
  // Check if SenseData meets the alignment requirement.
  //
  if ((AtapiScsiPrivate->ScsiPassThru.Mode->IoAlign != 0)     \
    && (AtapiScsiPrivate->ScsiPassThru.Mode->IoAlign != 1)) {
    if (((UINTN)Packet->SenseData % AtapiScsiPrivate->ScsiPassThru.Mode->IoAlign) != 0) {
      return EFI_INVALID_PARAMETER;
    }
  }


  //
  // Return SenseData if PacketCommandStatus matches
  // the following return codes.
  //
  if ((PacketCommandStatus ==  EFI_BAD_BUFFER_SIZE) ||
      (PacketCommandStatus == EFI_DEVICE_ERROR) ||
      (PacketCommandStatus == EFI_TIMEOUT)) {

    //
    // avoid submit request sense command continuously.
    //
    if (PacketCommand[0] == OP_REQUEST_SENSE) {
      Packet->SenseDataLength = 0;
      return PacketCommandStatus;
    }

    RequestSenseCommand (
      AtapiScsiPrivate,
      Target,
      Packet->Timeout,
      Packet->SenseData,
      &Packet->SenseDataLength
      );
  }

  return PacketCommandStatus;
}

/**
  RequestSenseCommand

  @param  AtapiScsiPrivate
  @param  Target
  @param  Timeout
  @param  SenseData
  @param  SenseDataLength

  @todo Add function description
  @todo  AtapiScsiPrivate TODO: add argument description
  @todo  Target TODO: add argument description
  @todo  Timeout TODO: add argument description
  @todo  SenseData TODO: add argument description
  @todo  SenseDataLength TODO: add argument description
  @todo add return values
**/
EFI_STATUS
RequestSenseCommand (
  ATAPI_SCSI_PASS_THRU_DEV    *AtapiScsiPrivate,
  UINT32                      Target,
  UINT64                      Timeout,
  VOID                        *SenseData,
  UINT8                       *SenseDataLength
  )
{
  EFI_SCSI_PASS_THRU_SCSI_REQUEST_PACKET  Packet;
  UINT8                                   Cdb[12];
  EFI_STATUS                              Status;

  ZeroMem (&Packet, sizeof (EFI_SCSI_PASS_THRU_SCSI_REQUEST_PACKET));
  ZeroMem (Cdb, 12);

  Cdb[0]                = OP_REQUEST_SENSE;
  Cdb[4]                = (UINT8) (*SenseDataLength);

  Packet.Timeout        = Timeout;
  Packet.DataBuffer     = SenseData;
  Packet.SenseData      = NULL;
  Packet.Cdb            = Cdb;
  Packet.TransferLength = *SenseDataLength;
  Packet.CdbLength      = 12;
  Packet.DataDirection  = DataIn;

  Status                = SubmitBlockingIoCommand (AtapiScsiPrivate, Target, &Packet);
  *SenseDataLength      = (UINT8) (Packet.TransferLength);
  return Status;
}

/**
  Submits ATAPI command packet to the specified ATAPI device.

  @param AtapiScsiPrivate:   Private data structure for the specified channel.
  @param Target:             The Target ID of the ATAPI device to send the SCSI
  Request Packet. To ATAPI devices attached on an IDE
  Channel, Target ID 0 indicates Master device;Target
  ID 1 indicates Slave device.
  @param PacketCommand:      Points to the ATAPI command packet.
  @param Buffer:             Points to the transferred data.
  @param ByteCount:          When input,indicates the buffer size; when output,
  indicates the actually transferred data size.
  @param Direction:          Indicates the data transfer direction.
  @param TimeoutInMicroSeconds: The timeout, in micro second units,
  to use for the execution of this ATAPI command.
  A TimeoutInMicroSeconds value of 0 means that
  this function will wait indefinitely for the ATAPI
  command to execute.
  <P>
  If TimeoutInMicroSeconds is greater than zero, then
  this function will return EFI_TIMEOUT if the time
  required to execute the ATAPI command is greater
  than TimeoutInMicroSeconds.
  </P>

  @todo    AtapiScsiPrivate - add argument and description to function comment
  @todo    PacketCommand - add argument and description to function comment
  @todo    Buffer - add argument and description to function comment
  @todo    ByteCount - add argument and description to function comment
  @todo    Direction - add argument and description to function comment
**/
EFI_STATUS
AtapiPacketCommand (
  ATAPI_SCSI_PASS_THRU_DEV    *AtapiScsiPrivate,
  UINT32                      Target,
  UINT8                       *PacketCommand,
  VOID                        *Buffer,
  UINT32                      *ByteCount,
  DATA_DIRECTION              Direction,
  UINT64                      TimeoutInMicroSeconds
  )
{

  UINT16      *CommandIndex;
  UINT8       Count;
  EFI_STATUS  Status;

  //
  // Check if the buffer meets the alignment requirement.
  //
  if ((AtapiScsiPrivate->ScsiPassThru.Mode->IoAlign != 0)     \
    && (AtapiScsiPrivate->ScsiPassThru.Mode->IoAlign != 1)) {
    if (((UINTN)Buffer % AtapiScsiPrivate->ScsiPassThru.Mode->IoAlign) != 0) {
      return EFI_INVALID_PARAMETER;
    }
  }

  //
  // Set all the command parameters by fill related registers.
  // Before write to all the following registers, BSY must be 0.
  //
  Status = StatusWaitForBSYClear (AtapiScsiPrivate, TimeoutInMicroSeconds);
  if (EFI_ERROR (Status)) {
    return EFI_DEVICE_ERROR;
  }


  //
  // Select device via Device/Head Register.
  // "Target = 0" indicates device 0; "Target = 1" indicates device 1
  //
  WritePortB (
    AtapiScsiPrivate->PciIo,
    AtapiScsiPrivate->IoPort->Head,
    (UINT8) ((Target << 4) | DEFAULT_CMD) // DEFAULT_CMD: 0xa0 (1010,0000)
    );

  //
  // Set all the command parameters by fill related registers.
  // Before write to all the following registers, BSY DRQ must be 0.
  //
   Status =  StatusDRQClear(AtapiScsiPrivate,  TimeoutInMicroSeconds);

  if (EFI_ERROR (Status)) {
    if (Status == EFI_ABORTED) {
      Status = EFI_DEVICE_ERROR;
    }
    *ByteCount = 0;
    return Status;
  }

  //
  // No OVL; No DMA (by setting feature register)
  //
  WritePortB (
    AtapiScsiPrivate->PciIo,
    AtapiScsiPrivate->IoPort->Reg1.Feature,
    0x00
    );

  //
  // set the transfersize to MAX_ATAPI_BYTE_COUNT to let the device
  // determine how much data should be transfered.
  //
  WritePortB (
    AtapiScsiPrivate->PciIo,
    AtapiScsiPrivate->IoPort->CylinderLsb,
    (UINT8) (MAX_ATAPI_BYTE_COUNT & 0x00ff)
    );
  WritePortB (
    AtapiScsiPrivate->PciIo,
    AtapiScsiPrivate->IoPort->CylinderMsb,
    (UINT8) (MAX_ATAPI_BYTE_COUNT >> 8)
    );

  //
  //  DEFAULT_CTL:0x0a (0000,1010)
  //  Disable interrupt
  //
  WritePortB (
    AtapiScsiPrivate->PciIo,
    AtapiScsiPrivate->IoPort->Alt.DeviceControl,
    DEFAULT_CTL
    );

  //
  // Send Packet command to inform device
  // that the following data bytes are command packet.
  //
  WritePortB (
    AtapiScsiPrivate->PciIo,
    AtapiScsiPrivate->IoPort->Reg.Command,
    PACKET_CMD
    );

  //
  // Before data transfer, BSY should be 0 and DRQ should be 1.
  // if they are not in specified time frame,
  // retrieve Sense Key from Error Register before return.
  //
  Status = StatusDRQReady (AtapiScsiPrivate, TimeoutInMicroSeconds);
  if (EFI_ERROR (Status)) {
    if (Status == EFI_ABORTED) {
      Status = EFI_DEVICE_ERROR;
    }

    *ByteCount = 0;
    return Status;
  }

  //
  // Send out command packet
  //
  CommandIndex = (UINT16 *) PacketCommand;
  for (Count = 0; Count < 6; Count++, CommandIndex++) {
    WritePortW (AtapiScsiPrivate->PciIo, AtapiScsiPrivate->IoPort->Data, *CommandIndex);
  }

  //
  // call AtapiPassThruPioReadWriteData() function to get
  // requested transfer data form device.
  //
  return AtapiPassThruPioReadWriteData (
          AtapiScsiPrivate,
          Buffer,
          ByteCount,
          Direction,
          TimeoutInMicroSeconds
          );
}

/**
  Performs data transfer between ATAPI device and host after the
  ATAPI command packet is sent.

  @param AtapiScsiPrivate:   Private data structure for the specified channel.
  @param Buffer:             Points to the transferred data.
  @param ByteCount:          When input,indicates the buffer size; when output,
  indicates the actually transferred data size.
  @param Direction:          Indicates the data transfer direction.
  @param TimeoutInMicroSeconds: The timeout, in micro second units,
  to use for the execution of this ATAPI command.
  A TimeoutInMicroSeconds value of 0 means that
  this function will wait indefinitely for the ATAPI
  command to execute.
  <P>
  If TimeoutInMicroSeconds is greater than zero, then
  this function will return EFI_TIMEOUT if the time
  required to execute the ATAPI command is greater
  than TimeoutInMicroSeconds.
  </P>

  @todo    AtapiScsiPrivate - add argument and description to function comment
  @todo    Buffer - add argument and description to function comment
  @todo    ByteCount - add argument and description to function comment
  @todo    Direction - add argument and description to function comment
  @todo    EFI_DEVICE_ERROR - add return value to function comment
  @todo    EFI_DEVICE_ERROR - add return value to function comment
  @todo    EFI_WARN_BUFFER_TOO_SMALL - add return value to function comment
**/
EFI_STATUS
AtapiPassThruPioReadWriteData (
  ATAPI_SCSI_PASS_THRU_DEV  *AtapiScsiPrivate,
  UINT16                    *Buffer,
  UINT32                    *ByteCount,
  DATA_DIRECTION            Direction,
  UINT64                    TimeoutInMicroSeconds
  )
{
  UINT32      Index;
  UINT32      RequiredWordCount;
  UINT32      ActualWordCount;

  UINT32      WordCount;
  EFI_STATUS  Status;
  UINT16      *ptrBuffer;

  Status = EFI_SUCCESS;

  //
  // Non Data transfer request is also supported.
  //
  if (*ByteCount == 0 || Buffer == NULL) {
    *ByteCount = 0;
    if (EFI_ERROR (StatusWaitForBSYClear (AtapiScsiPrivate, TimeoutInMicroSeconds))) {
      return EFI_DEVICE_ERROR;
    }
  }

  ptrBuffer         = Buffer;
  RequiredWordCount = *ByteCount / 2;

  //
  // ActuralWordCount means the word count of data really transfered.
  //
  ActualWordCount = 0;

  while (ActualWordCount < RequiredWordCount) {
    //
    // before each data transfer stream, the host should poll DRQ bit ready,
    // which indicates device's ready for data transfer .
    //
    Status = StatusDRQReady (AtapiScsiPrivate, TimeoutInMicroSeconds);
    if (EFI_ERROR (Status)) {
      *ByteCount = ActualWordCount * 2;

      AtapiPassThruCheckErrorStatus (AtapiScsiPrivate);

      if (ActualWordCount == 0) {
        return EFI_DEVICE_ERROR;
      }
      //
      // ActualWordCount > 0
      //
      if (ActualWordCount < RequiredWordCount) {
        return EFI_BAD_BUFFER_SIZE;
      }
    }
    //
    // get current data transfer size from Cylinder Registers.
    //
    WordCount = ReadPortB (AtapiScsiPrivate->PciIo, AtapiScsiPrivate->IoPort->CylinderMsb) << 8;
    WordCount = WordCount | ReadPortB (AtapiScsiPrivate->PciIo, AtapiScsiPrivate->IoPort->CylinderLsb);
    WordCount = WordCount & 0xffff;
    WordCount /= 2;

    //
    // perform a series data In/Out.
    //
    for (Index = 0; (Index < WordCount) && (ActualWordCount < RequiredWordCount); Index++, ActualWordCount++) {

      if (Direction == DataIn) {

        *ptrBuffer = ReadPortW (AtapiScsiPrivate->PciIo, AtapiScsiPrivate->IoPort->Data);
      } else {

        WritePortW (AtapiScsiPrivate->PciIo, AtapiScsiPrivate->IoPort->Data, *ptrBuffer);
      }

      ptrBuffer++;

    }
  }
  //
  // After data transfer is completed, normally, DRQ bit should clear.
  //
  StatusDRQClear (AtapiScsiPrivate, TimeoutInMicroSeconds);

  //
  // read status register to check whether error happens.
  //
  Status      = AtapiPassThruCheckErrorStatus (AtapiScsiPrivate);

  *ByteCount  = ActualWordCount * 2;

  return Status;
}


/**
  Read one byte from a specified I/O port.

  @todo function comment is missing 'Routine Description:'
  @todo function comment is missing 'Arguments:'
  @todo function comment is missing 'Returns:'
  @todo    PciIo - add argument and description to function comment
  @todo    Port - add argument and description to function comment
**/
UINT8
ReadPortB (
  IN  EFI_PCI_IO_PROTOCOL   *PciIo,
  IN  UINT16                Port
  )
{
  UINT8 Data;

  Data = 0;
  PciIo->Io.Read (
              PciIo,
              EfiPciIoWidthUint8,
              EFI_PCI_IO_PASS_THROUGH_BAR,
              (UINT64) Port,
              1,
              &Data
              );
  return Data;
}


/**
  Read one word from a specified I/O port.

  @todo function comment is missing 'Routine Description:'
  @todo function comment is missing 'Arguments:'
  @todo function comment is missing 'Returns:'
  @todo    PciIo - add argument and description to function comment
  @todo    Port - add argument and description to function comment
**/
UINT16
ReadPortW (
  IN  EFI_PCI_IO_PROTOCOL   *PciIo,
  IN  UINT16                Port
  )
{
  UINT16  Data;

  Data = 0;
  PciIo->Io.Read (
              PciIo,
              EfiPciIoWidthUint16,
              EFI_PCI_IO_PASS_THROUGH_BAR,
              (UINT64) Port,
              1,
              &Data
              );
  return Data;
}


/**
  Write one byte to a specified I/O port.

  @todo function comment is missing 'Routine Description:'
  @todo function comment is missing 'Arguments:'
  @todo function comment is missing 'Returns:'
  @todo    PciIo - add argument and description to function comment
  @todo    Port - add argument and description to function comment
  @todo    Data - add argument and description to function comment
**/
VOID
WritePortB (
  IN  EFI_PCI_IO_PROTOCOL   *PciIo,
  IN  UINT16                Port,
  IN  UINT8                 Data
  )
{

  PciIo->Io.Write (
              PciIo,
              EfiPciIoWidthUint8,
              EFI_PCI_IO_PASS_THROUGH_BAR,
              (UINT64) Port,
              1,
              &Data
              );

}


/**
  Write one word to a specified I/O port.

  @todo function comment is missing 'Routine Description:'
  @todo function comment is missing 'Arguments:'
  @todo function comment is missing 'Returns:'
  @todo    PciIo - add argument and description to function comment
  @todo    Port - add argument and description to function comment
  @todo    Data - add argument and description to function comment
**/
VOID
WritePortW (
  IN  EFI_PCI_IO_PROTOCOL   *PciIo,
  IN  UINT16                Port,
  IN  UINT16                Data
  )
{

  PciIo->Io.Write (
              PciIo,
              EfiPciIoWidthUint16,
              EFI_PCI_IO_PASS_THROUGH_BAR,
              (UINT64) Port,
              1,
              &Data
              );
}

/**
  Check whether DRQ is clear in the Status Register. (BSY must also be cleared)
  If TimeoutInMicroSeconds is zero, this routine should wait infinitely for
  DRQ clear. Otherwise, it will return EFI_TIMEOUT when specified time is
  elapsed.

  @todo function comment is missing 'Routine Description:'
  @todo function comment is missing 'Arguments:'
  @todo function comment is missing 'Returns:'
  @todo    AtapiScsiPrivate - add argument and description to function comment
  @todo    TimeoutInMicroSeconds - add argument and description to function comment
  @todo    EFI_ABORTED - add return value to function comment
  @todo    EFI_TIMEOUT - add return value to function comment
  @todo    EFI_SUCCESS - add return value to function comment
**/
EFI_STATUS
StatusDRQClear (
  ATAPI_SCSI_PASS_THRU_DEV        *AtapiScsiPrivate,
  UINT64                          TimeoutInMicroSeconds
  )
{
  UINT64  Delay;
  UINT8   StatusRegister;
  UINT8   ErrRegister;

  if (TimeoutInMicroSeconds == 0) {
    Delay = 2;
  } else {
    Delay = DivU64x32 (TimeoutInMicroSeconds, (UINT32) 30) + 1;
  }

  do {

    StatusRegister = ReadPortB (
                      AtapiScsiPrivate->PciIo,
                      AtapiScsiPrivate->IoPort->Reg.Status
                      );

    //
    // wait for BSY == 0 and DRQ == 0
    //
    if ((StatusRegister & (DRQ | BSY)) == 0) {
      break;
    }
    //
    // check whether the command is aborted by the device
    //
    if ((StatusRegister & (BSY | ERR)) == ERR) {

      ErrRegister = ReadPortB (
                      AtapiScsiPrivate->PciIo,
                      AtapiScsiPrivate->IoPort->Reg1.Error
                      );
      if ((ErrRegister & ABRT_ERR) == ABRT_ERR) {

        return EFI_ABORTED;
      }
    }
    //
    //  Stall for 30 us
    //
    gBS->Stall (30);

    //
    // Loop infinitely if not meeting expected condition
    //
    if (TimeoutInMicroSeconds == 0) {
      Delay = 2;
    }

    Delay--;
  } while (Delay);

  if (Delay == 0) {
    return EFI_TIMEOUT;
  }

  return EFI_SUCCESS;
}

/**
  Check whether DRQ is clear in the Alternate Status Register.
  (BSY must also be cleared).
  If TimeoutInMicroSeconds is zero, this routine should wait infinitely for
  DRQ clear. Otherwise, it will return EFI_TIMEOUT when specified time is
  elapsed.

  @todo function comment is missing 'Routine Description:'
  @todo function comment is missing 'Arguments:'
  @todo function comment is missing 'Returns:'
  @todo    AtapiScsiPrivate - add argument and description to function comment
  @todo    TimeoutInMicroSeconds - add argument and description to function comment
  @todo    EFI_ABORTED - add return value to function comment
  @todo    EFI_TIMEOUT - add return value to function comment
  @todo    EFI_SUCCESS - add return value to function comment
**/
EFI_STATUS
AltStatusDRQClear (
  ATAPI_SCSI_PASS_THRU_DEV        *AtapiScsiPrivate,
  UINT64                          TimeoutInMicroSeconds
  )
{
  UINT64  Delay;
  UINT8   AltStatusRegister;
  UINT8   ErrRegister;

  if (TimeoutInMicroSeconds == 0) {
    Delay = 2;
  } else {
    Delay = DivU64x32 (TimeoutInMicroSeconds, (UINT32) 30) + 1;
  }

  do {

    AltStatusRegister = ReadPortB (
                          AtapiScsiPrivate->PciIo,
                          AtapiScsiPrivate->IoPort->Alt.AltStatus
                          );

    //
    // wait for BSY == 0 and DRQ == 0
    //
    if ((AltStatusRegister & (DRQ | BSY)) == 0) {
      break;
    }

    if ((AltStatusRegister & (BSY | ERR)) == ERR) {

      ErrRegister = ReadPortB (
                      AtapiScsiPrivate->PciIo,
                      AtapiScsiPrivate->IoPort->Reg1.Error
                      );
      if ((ErrRegister & ABRT_ERR) == ABRT_ERR) {

        return EFI_ABORTED;
      }
    }
    //
    //  Stall for 30 us
    //
    gBS->Stall (30);

    //
    // Loop infinitely if not meeting expected condition
    //
    if (TimeoutInMicroSeconds == 0) {
      Delay = 2;
    }

    Delay--;
  } while (Delay);

  if (Delay == 0) {
    return EFI_TIMEOUT;
  }

  return EFI_SUCCESS;
}

/**
  Check whether DRQ is ready in the Status Register. (BSY must also be cleared)
  If TimeoutInMicroSeconds is zero, this routine should wait infinitely for
  DRQ ready. Otherwise, it will return EFI_TIMEOUT when specified time is
  elapsed.

  @todo function comment is missing 'Routine Description:'
  @todo function comment is missing 'Arguments:'
  @todo function comment is missing 'Returns:'
  @todo    AtapiScsiPrivate - add argument and description to function comment
  @todo    TimeoutInMicroSeconds - add argument and description to function comment
  @todo    EFI_ABORTED - add return value to function comment
  @todo    EFI_TIMEOUT - add return value to function comment
  @todo    EFI_SUCCESS - add return value to function comment
**/
EFI_STATUS
StatusDRQReady (
  ATAPI_SCSI_PASS_THRU_DEV        *AtapiScsiPrivate,
  UINT64                          TimeoutInMicroSeconds
  )
{
  UINT64  Delay;
  UINT8   StatusRegister;
  UINT8   ErrRegister;

  if (TimeoutInMicroSeconds == 0) {
    Delay = 2;
  } else {
    Delay = DivU64x32 (TimeoutInMicroSeconds, (UINT32) 30) + 1;
  }

  do {
    //
    //  read Status Register will clear interrupt
    //
    StatusRegister = ReadPortB (
                      AtapiScsiPrivate->PciIo,
                      AtapiScsiPrivate->IoPort->Reg.Status
                      );

    //
    //  BSY==0,DRQ==1
    //
    if ((StatusRegister & (BSY | DRQ)) == DRQ) {
      break;
    }

    if ((StatusRegister & (BSY | ERR)) == ERR) {

      ErrRegister = ReadPortB (
                      AtapiScsiPrivate->PciIo,
                      AtapiScsiPrivate->IoPort->Reg1.Error
                      );
      if ((ErrRegister & ABRT_ERR) == ABRT_ERR) {
        return EFI_ABORTED;
      }
    }

    //
    // Stall for 30 us
    //
    gBS->Stall (30);

    //
    // Loop infinitely if not meeting expected condition
    //
    if (TimeoutInMicroSeconds == 0) {
      Delay = 2;
    }

    Delay--;
  } while (Delay);

  if (Delay == 0) {
    return EFI_TIMEOUT;
  }

  return EFI_SUCCESS;
}

/**
  Check whether DRQ is ready in the Alternate Status Register.
  (BSY must also be cleared)
  If TimeoutInMicroSeconds is zero, this routine should wait infinitely for
  DRQ ready. Otherwise, it will return EFI_TIMEOUT when specified time is
  elapsed.

  @todo function comment is missing 'Routine Description:'
  @todo function comment is missing 'Arguments:'
  @todo function comment is missing 'Returns:'
  @todo    AtapiScsiPrivate - add argument and description to function comment
  @todo    TimeoutInMicroSeconds - add argument and description to function comment
  @todo    EFI_ABORTED - add return value to function comment
  @todo    EFI_TIMEOUT - add return value to function comment
  @todo    EFI_SUCCESS - add return value to function comment
**/
EFI_STATUS
AltStatusDRQReady (
  ATAPI_SCSI_PASS_THRU_DEV        *AtapiScsiPrivate,
  UINT64                          TimeoutInMicroSeconds
  )
{
  UINT64  Delay;
  UINT8   AltStatusRegister;
  UINT8   ErrRegister;

  if (TimeoutInMicroSeconds == 0) {
    Delay = 2;
  } else {
    Delay = DivU64x32 (TimeoutInMicroSeconds, (UINT32) 30) + 1;
  }

  do {
    //
    //  read Status Register will clear interrupt
    //
    AltStatusRegister = ReadPortB (
                          AtapiScsiPrivate->PciIo,
                          AtapiScsiPrivate->IoPort->Alt.AltStatus
                          );
    //
    //  BSY==0,DRQ==1
    //
    if ((AltStatusRegister & (BSY | DRQ)) == DRQ) {
      break;
    }

    if ((AltStatusRegister & (BSY | ERR)) == ERR) {

      ErrRegister = ReadPortB (
                      AtapiScsiPrivate->PciIo,
                      AtapiScsiPrivate->IoPort->Reg1.Error
                      );
      if ((ErrRegister & ABRT_ERR) == ABRT_ERR) {
        return EFI_ABORTED;
      }
    }

    //
    // Stall for 30 us
    //
    gBS->Stall (30);

    //
    // Loop infinitely if not meeting expected condition
    //
    if (TimeoutInMicroSeconds == 0) {
      Delay = 2;
    }

    Delay--;
  } while (Delay);

  if (Delay == 0) {
    return EFI_TIMEOUT;
  }

  return EFI_SUCCESS;
}

/**
  Check whether BSY is clear in the Status Register.
  If TimeoutInMicroSeconds is zero, this routine should wait infinitely for
  BSY clear. Otherwise, it will return EFI_TIMEOUT when specified time is
  elapsed.

  @todo function comment is missing 'Routine Description:'
  @todo function comment is missing 'Arguments:'
  @todo function comment is missing 'Returns:'
  @todo    AtapiScsiPrivate - add argument and description to function comment
  @todo    TimeoutInMicroSeconds - add argument and description to function comment
  @todo    EFI_TIMEOUT - add return value to function comment
  @todo    EFI_SUCCESS - add return value to function comment
**/
EFI_STATUS
StatusWaitForBSYClear (
  ATAPI_SCSI_PASS_THRU_DEV    *AtapiScsiPrivate,
  UINT64                      TimeoutInMicroSeconds
  )
{
  UINT64  Delay;
  UINT8   StatusRegister;

  if (TimeoutInMicroSeconds == 0) {
    Delay = 2;
  } else {
    Delay = DivU64x32 (TimeoutInMicroSeconds, (UINT32) 30) + 1;
  }

  do {

    StatusRegister = ReadPortB (
                      AtapiScsiPrivate->PciIo,
                      AtapiScsiPrivate->IoPort->Reg.Status
                      );
    if ((StatusRegister & BSY) == 0x00) {
      break;
    }

    //
    // Stall for 30 us
    //
    gBS->Stall (30);

    //
    // Loop infinitely if not meeting expected condition
    //
    if (TimeoutInMicroSeconds == 0) {
      Delay = 2;
    }

    Delay--;
  } while (Delay);

  if (Delay == 0) {
    return EFI_TIMEOUT;
  }

  return EFI_SUCCESS;
}

/**
  Check whether BSY is clear in the Alternate Status Register.
  If TimeoutInMicroSeconds is zero, this routine should wait infinitely for
  BSY clear. Otherwise, it will return EFI_TIMEOUT when specified time is
  elapsed.

  @todo function comment is missing 'Routine Description:'
  @todo function comment is missing 'Arguments:'
  @todo function comment is missing 'Returns:'
  @todo    AtapiScsiPrivate - add argument and description to function comment
  @todo    TimeoutInMicroSeconds - add argument and description to function comment
  @todo    EFI_TIMEOUT - add return value to function comment
  @todo    EFI_SUCCESS - add return value to function comment
**/
EFI_STATUS
AltStatusWaitForBSYClear (
  ATAPI_SCSI_PASS_THRU_DEV    *AtapiScsiPrivate,
  UINT64                      TimeoutInMicroSeconds
  )
{
  UINT64  Delay;
  UINT8   AltStatusRegister;

  if (TimeoutInMicroSeconds == 0) {
    Delay = 2;
  } else {
    Delay = DivU64x32 (TimeoutInMicroSeconds, (UINT32) 30) + 1;
  }

  do {

    AltStatusRegister = ReadPortB (
                          AtapiScsiPrivate->PciIo,
                          AtapiScsiPrivate->IoPort->Alt.AltStatus
                          );
    if ((AltStatusRegister & BSY) == 0x00) {
      break;
    }

    //
    // Stall for 30 us
    //
    gBS->Stall (30);
    //
    // Loop infinitely if not meeting expected condition
    //
    if (TimeoutInMicroSeconds == 0) {
      Delay = 2;
    }

    Delay--;
  } while (Delay);

  if (Delay == 0) {
    return EFI_TIMEOUT;
  }

  return EFI_SUCCESS;
}

/**
  Check whether DRDY is ready in the Status Register.
  (BSY must also be cleared)
  If TimeoutInMicroSeconds is zero, this routine should wait infinitely for
  DRDY ready. Otherwise, it will return EFI_TIMEOUT when specified time is
  elapsed.

  @todo function comment is missing 'Routine Description:'
  @todo function comment is missing 'Arguments:'
  @todo function comment is missing 'Returns:'
  @todo    AtapiScsiPrivate - add argument and description to function comment
  @todo    TimeoutInMicroSeconds - add argument and description to function comment
  @todo    EFI_ABORTED - add return value to function comment
  @todo    EFI_TIMEOUT - add return value to function comment
  @todo    EFI_SUCCESS - add return value to function comment
**/
EFI_STATUS
StatusDRDYReady (
  ATAPI_SCSI_PASS_THRU_DEV     *AtapiScsiPrivate,
  UINT64                       TimeoutInMicroSeconds
  )
{
  UINT64  Delay;
  UINT8   StatusRegister;
  UINT8   ErrRegister;

  if (TimeoutInMicroSeconds == 0) {
    Delay = 2;
  } else {
    Delay = DivU64x32 (TimeoutInMicroSeconds, (UINT32) 30) + 1;
  }

  do {
    StatusRegister = ReadPortB (
                      AtapiScsiPrivate->PciIo,
                      AtapiScsiPrivate->IoPort->Reg.Status
                      );
    //
    //  BSY == 0 , DRDY == 1
    //
    if ((StatusRegister & (DRDY | BSY)) == DRDY) {
      break;
    }

    if ((StatusRegister & (BSY | ERR)) == ERR) {

      ErrRegister = ReadPortB (
                      AtapiScsiPrivate->PciIo,
                      AtapiScsiPrivate->IoPort->Reg1.Error
                      );
      if ((ErrRegister & ABRT_ERR) == ABRT_ERR) {
        return EFI_ABORTED;
      }
    }

    //
    // Stall for 30 us
    //
    gBS->Stall (30);
    //
    // Loop infinitely if not meeting expected condition
    //
    if (TimeoutInMicroSeconds == 0) {
      Delay = 2;
    }

    Delay--;
  } while (Delay);

  if (Delay == 0) {
    return EFI_TIMEOUT;
  }

  return EFI_SUCCESS;
}

/**
  Check whether DRDY is ready in the Alternate Status Register.
  (BSY must also be cleared)
  If TimeoutInMicroSeconds is zero, this routine should wait infinitely for
  DRDY ready. Otherwise, it will return EFI_TIMEOUT when specified time is
  elapsed.

  @todo function comment is missing 'Routine Description:'
  @todo function comment is missing 'Arguments:'
  @todo function comment is missing 'Returns:'
  @todo    AtapiScsiPrivate - add argument and description to function comment
  @todo    TimeoutInMicroSeconds - add argument and description to function comment
  @todo    EFI_ABORTED - add return value to function comment
  @todo    EFI_TIMEOUT - add return value to function comment
  @todo    EFI_SUCCESS - add return value to function comment
**/
EFI_STATUS
AltStatusDRDYReady (
  ATAPI_SCSI_PASS_THRU_DEV     *AtapiScsiPrivate,
  UINT64                       TimeoutInMicroSeconds
  )
{
  UINT64  Delay;
  UINT8   AltStatusRegister;
  UINT8   ErrRegister;

  if (TimeoutInMicroSeconds == 0) {
    Delay = 2;
  } else {
    Delay = DivU64x32 (TimeoutInMicroSeconds, (UINT32) 30) + 1;
  }

  do {
    AltStatusRegister = ReadPortB (
                          AtapiScsiPrivate->PciIo,
                          AtapiScsiPrivate->IoPort->Alt.AltStatus
                          );
    //
    //  BSY == 0 , DRDY == 1
    //
    if ((AltStatusRegister & (DRDY | BSY)) == DRDY) {
      break;
    }

    if ((AltStatusRegister & (BSY | ERR)) == ERR) {

      ErrRegister = ReadPortB (
                      AtapiScsiPrivate->PciIo,
                      AtapiScsiPrivate->IoPort->Reg1.Error
                      );
      if ((ErrRegister & ABRT_ERR) == ABRT_ERR) {
        return EFI_ABORTED;
      }
    }

    //
    // Stall for 30 us
    //
    gBS->Stall (30);
    //
    // Loop infinitely if not meeting expected condition
    //
    if (TimeoutInMicroSeconds == 0) {
      Delay = 2;
    }

    Delay--;
  } while (Delay);

  if (Delay == 0) {
    return EFI_TIMEOUT;
  }

  return EFI_SUCCESS;
}

/**
  Check Error Register for Error Information.

  @todo function comment is missing 'Routine Description:'
  @todo function comment is missing 'Arguments:'
  @todo function comment is missing 'Returns:'
  @todo    AtapiScsiPrivate - add argument and description to function comment
  @todo    EFI_SUCCESS - add return value to function comment
  @todo    EFI_DEVICE_ERROR - add return value to function comment
**/
EFI_STATUS
AtapiPassThruCheckErrorStatus (
  ATAPI_SCSI_PASS_THRU_DEV        *AtapiScsiPrivate
  )
{
  UINT8 StatusRegister;
  UINT8 ErrorRegister;

  StatusRegister = ReadPortB (
                    AtapiScsiPrivate->PciIo,
                    AtapiScsiPrivate->IoPort->Reg.Status
                    );

  DEBUG_CODE_BEGIN ();

    if (StatusRegister & DWF) {
      DEBUG (
        (EFI_D_BLKIO,
        "AtapiPassThruCheckErrorStatus()-- %02x : Error : Write Fault\n",
        StatusRegister)
        );
    }

    if (StatusRegister & CORR) {
      DEBUG (
        (EFI_D_BLKIO,
        "AtapiPassThruCheckErrorStatus()-- %02x : Error : Corrected Data\n",
        StatusRegister)
        );
    }

    if (StatusRegister & ERR) {
      ErrorRegister = ReadPortB (AtapiScsiPrivate->PciIo, AtapiScsiPrivate->IoPort->Reg1.Error);


      if (ErrorRegister & BBK_ERR) {
        DEBUG (
          (EFI_D_BLKIO,
          "AtapiPassThruCheckErrorStatus()-- %02x : Error : Bad Block Detected\n",
          ErrorRegister)
          );
      }

      if (ErrorRegister & UNC_ERR) {
        DEBUG (
          (EFI_D_BLKIO,
          "AtapiPassThruCheckErrorStatus()-- %02x : Error : Uncorrectable Data\n",
          ErrorRegister)
          );
      }

      if (ErrorRegister & MC_ERR) {
        DEBUG (
          (EFI_D_BLKIO,
          "AtapiPassThruCheckErrorStatus()-- %02x : Error : Media Change\n",
          ErrorRegister)
          );
      }

      if (ErrorRegister & ABRT_ERR) {
        DEBUG (
          (EFI_D_BLKIO,
          "AtapiPassThruCheckErrorStatus()-- %02x : Error : Abort\n",
          ErrorRegister)
          );
      }

      if (ErrorRegister & TK0NF_ERR) {
        DEBUG (
          (EFI_D_BLKIO,
          "AtapiPassThruCheckErrorStatus()-- %02x : Error : Track 0 Not Found\n",
          ErrorRegister)
          );
      }

      if (ErrorRegister & AMNF_ERR) {
        DEBUG (
          (EFI_D_BLKIO,
          "AtapiPassThruCheckErrorStatus()-- %02x : Error : Address Mark Not Found\n",
          ErrorRegister)
          );
       }
    }

  DEBUG_CODE_END ();

  if ((StatusRegister & (ERR | DWF | CORR)) == 0) {
    return EFI_SUCCESS;
  }


  return EFI_DEVICE_ERROR;
}

/**
  The user Entry Point for module AtapiPassThru. The user code starts with this function.

  @param[in] ImageHandle    The firmware allocated handle for the EFI image.
  @param[in] SystemTable    A pointer to the EFI System Table.

  @retval EFI_SUCCESS       The entry point is executed successfully.
  @retval other             Some error occurs when executing this entry point.

**/
EFI_STATUS
EFIAPI
InitializeAtapiPassThru(
  IN EFI_HANDLE           ImageHandle,
  IN EFI_SYSTEM_TABLE     *SystemTable
  )
{
  EFI_STATUS              Status;

  //
  // Install driver model protocol(s).
  //
  Status = EfiLibInstallDriverBindingComponentName2 (
             ImageHandle,
             SystemTable,
             &gAtapiScsiPassThruDriverBinding,
             ImageHandle,
             &gAtapiScsiPassThruComponentName,
             &gAtapiScsiPassThruComponentName2
             );
  ASSERT_EFI_ERROR (Status);

  return Status;
}
