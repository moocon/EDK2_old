/** @file
  IPMI Platform Management FRU Information Storage Definitions

  This file contains the definitions for:
    Common Header Format (Chapter 8)
    MultiRecord Header (Section 16.1)

  Copyright (c) 2018, Intel Corporation. All rights reserved.<BR>
  This program and the accompanying materials
  are licensed and made available under the terms and conditions of the BSD License
  which accompanies this distribution.  The full text of the license may be found at
  http://opensource.org/licenses/bsd-license.php

  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

  @par Revision Reference:
    - IPMI Platform Management FRU Information Storage Definition v1.0 Revision
      1.3, Dated March 24, 2015.
      https://www.intel.com/content/dam/www/public/us/en/documents/specification-updates/ipmi-platform-mgt-fru-info-storage-def-v1-0-rev-1-3-spec-update.pdf
**/

#ifndef _IPMI_FRU_INFORMATION_STORAGE_H_
#define _IPMI_FRU_INFORMATION_STORAGE_H_

#pragma pack(1)

//
//  Structure definition for FRU Common Header
//
typedef struct {
  UINT8    FormatVersionNumber:4;
  UINT8    Reserved:4;
  UINT8    InternalUseStartingOffset;
  UINT8    ChassisInfoStartingOffset;
  UINT8    BoardAreaStartingOffset;
  UINT8    ProductInfoStartingOffset;
  UINT8    MultiRecInfoStartingOffset;
  UINT8    Pad;
  UINT8    Checksum;
} IPMI_FRU_COMMON_HEADER;

//
//  Structure definition for FRU MultiRecord Header
//
typedef struct {
  UINT8    RecordTypeId;
  UINT8    RecordFormatVersion:4;
  UINT8    Reserved:3;
  UINT8    EndofList:1;
  UINT8    RecordLength;
  UINT8    RecordChecksum;
  UINT8    HeaderChecksum;
} IPMI_FRU_MULTI_RECORD_HEADER;

//
//  Structure definition for System UUID Subrecord with checksum.
//
typedef struct {
  UINT8       RecordCheckSum;
  UINT8       SubRecordId;
  EFI_GUID    Uuid;
} IPMI_SYSTEM_UUID_SUB_RECORD_WITH_CHECKSUM;

#pragma pack()
#endif
