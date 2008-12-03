/** @file
  Disk Info protocol is used to export Inquiry Data for a drive.
  It supports low level formating of drives in a DOS compatible manner.

Copyright (c) 2006 - 2008, Intel Corporation. <BR>
All rights reserved. This program and the accompanying materials
are licensed and made available under the terms and conditions of the BSD License
which accompanies this distribution.  The full text of the license may be found at
http://opensource.org/licenses/bsd-license.php

THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

#ifndef __DISK_INFO_H__
#define __DISK_INFO_H__

#define EFI_DISK_INFO_PROTOCOL_GUID \
  { \
    0xd432a67f, 0x14dc, 0x484b, {0xb3, 0xbb, 0x3f, 0x2, 0x91, 0x84, 0x93, 0x27 } \
  }

//
// Forward reference for pure ANSI compatability
//
typedef struct _EFI_DISK_INFO_PROTOCOL  EFI_DISK_INFO_PROTOCOL;

/**
  Return the results of the Inquiry command to a drive in InquiryData.
  Data format of Inquiry data is defined by the Interface GUID.

  @param  This                  Protocol instance pointer. 
  @param  InquiryData           Results of Inquiry command to device 
  @param  InquiryDataSize       Size of InquiryData in bytes. 

  @retval EFI_SUCCESS           InquiryData valid 
  @retval EFI_NOT_FOUND         Device does not support this data class 
  @retval EFI_DEVICE_ERROR      Error reading InquiryData from device 
  @retval EFI_BUFFER_TOO_SMALL  IntquiryDataSize not big enough 

**/
typedef
EFI_STATUS
(EFIAPI *EFI_DISK_INFO_INQUIRY)(
  IN EFI_DISK_INFO_PROTOCOL           * This,
  IN OUT VOID                         *InquiryData,
  IN OUT UINT32                       *IntquiryDataSize
  );


/**
  Return the results of the Identify command to a drive in IdentifyData.
  Data format of Identify data is defined by the Interface GUID.

  @param  This                  Protocol instance pointer. 
  @param  IdentifyData          Results of Identify command to device 
  @param  IdentifyDataSize      Size of IdentifyData in bytes. 

  @retval EFI_SUCCESS           IdentifyData valid 
  @retval EFI_NOT_FOUND         Device does not support this data class 
  @retval EFI_DEVICE_ERROR      Error reading IdentifyData from device 
  @retval EFI_BUFFER_TOO_SMALL  IdentifyDataSize not big enough 

**/
typedef
EFI_STATUS
(EFIAPI *EFI_DISK_INFO_IDENTIFY)(
  IN EFI_DISK_INFO_PROTOCOL           * This,
  IN OUT VOID                         *IdentifyData,
  IN OUT UINT32                       *IdentifyDataSize
  );


/**
  Return the results of the Request Sense command to a drive in SenseData.
  Data format of Sense data is defined by the Interface GUID.

  @param  This                  Protocol instance pointer. 
  @param  SenseData             Results of Request Sense command to device 
  @param  SenseDataSize         Size of SenseData in bytes. 
  @param  SenseDataNumber       Type of SenseData 

  @retval EFI_SUCCESS           InquiryData valid 
  @retval EFI_NOT_FOUND         Device does not support this data class 
  @retval EFI_DEVICE_ERROR      Error reading InquiryData from device 
  @retval EFI_BUFFER_TOO_SMALL  SenseDataSize not big enough 

**/
typedef
EFI_STATUS
(EFIAPI *EFI_DISK_INFO_SENSE_DATA)(
  IN EFI_DISK_INFO_PROTOCOL           * This,
  IN OUT VOID                         *SenseData,
  IN OUT UINT32                       *SenseDataSize,
  OUT UINT8                           *SenseDataNumber
  );

/**
  Return the IDE device information.

  @param  This                  Protocol instance pointer. 
  @param  IdeChannel            Primary or Secondary 
  @param  IdeDevice             Master or Slave 

  @retval EFI_SUCCESS           IdeChannel and IdeDevice are valid 
  @retval EFI_UNSUPPORTED       This is not an IDE device 

**/
typedef
EFI_STATUS
(EFIAPI *EFI_DISK_INFO_WHICH_IDE)(
  IN EFI_DISK_INFO_PROTOCOL           * This,
  OUT UINT32                          *IdeChannel,
  OUT UINT32                          *IdeDevice
  );

//
// GUIDs for EFI_DISK_INFO_PROTOCOL.Interface. Defines the format of the
// buffers returned by member functions.
//
#define EFI_DISK_INFO_IDE_INTERFACE_GUID \
  { \
    0x5e948fe3, 0x26d3, 0x42b5, {0xaf, 0x17, 0x61, 0x2, 0x87, 0x18, 0x8d, 0xec } \
  }
extern EFI_GUID gEfiDiskInfoIdeInterfaceGuid;

#define EFI_DISK_INFO_SCSI_INTERFACE_GUID \
  { \
    0x8f74baa, 0xea36, 0x41d9, {0x95, 0x21, 0x21, 0xa7, 0xf, 0x87, 0x80, 0xbc } \
  }
extern EFI_GUID gEfiDiskInfoScsiInterfaceGuid;

#define EFI_DISK_INFO_USB_INTERFACE_GUID \
  { \
    0xcb871572, 0xc11a, 0x47b5, {0xb4, 0x92, 0x67, 0x5e, 0xaf, 0xa7, 0x77, 0x27 } \
  }
extern EFI_GUID gEfiDiskInfoUsbInterfaceGuid;

#define EFI_DISK_INFO_AHCI_INTERFACE_GUID \
  { \
    0x9e498932, 0x4abc, 0x45af, {0xa3, 0x4d, 0x2, 0x47, 0x78, 0x7b, 0xe7, 0xc6} \
  }
extern EFI_GUID gEfiDiskInfoAhciInterfaceGuid;

struct _EFI_DISK_INFO_PROTOCOL {
  EFI_GUID                  Interface;  /// The format of the buffers returned by member functions.
  EFI_DISK_INFO_INQUIRY     Inquiry;
  EFI_DISK_INFO_IDENTIFY    Identify;
  EFI_DISK_INFO_SENSE_DATA  SenseData;
  EFI_DISK_INFO_WHICH_IDE   WhichIde;
};

extern EFI_GUID gEfiDiskInfoProtocolGuid;

#endif


