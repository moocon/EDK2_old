/*++

Copyright (c) 2009, Intel Corporation. All rights reserved. <BR> 
This software and associated documentation (if any) is furnished
under a license and may only be used or copied in accordance
with the terms of the license. Except as permitted by such
license, no part of this software or documentation may be
reproduced, stored in a retrieval system, or transmitted in any
form or by any means without the express written consent of
Intel Corporation.


Module Name:

  MiscSystemSlotDesignatorFunction.c
  
Abstract: 

  BIOS system slot designator information boot time changes.
  SMBIOS type 9.

--*/

#include "MiscSubClassDriver.h"
/**
  This function makes boot time changes to the contents of the
  MiscSystemSlotDesignator structure (Type 9).

  @param  RecordData                 Pointer to copy of RecordData from the Data Table.  

  @retval EFI_SUCCESS                All parameters were valid.
  @retval EFI_UNSUPPORTED            Unexpected RecordType value.
  @retval EFI_INVALID_PARAMETER      Invalid parameter was found.

**/
MISC_SMBIOS_TABLE_FUNCTION(MiscSystemSlotDesignation)
{
  CHAR8                              *OptionalStrStart;
  UINTN                              SlotDesignationStrLen;
  EFI_STATUS                         Status;
  EFI_STRING                         SlotDesignation;
  STRING_REF                         TokenToGet;
  SMBIOS_TABLE_TYPE9                 *SmbiosRecord;
  EFI_SMBIOS_HANDLE                  SmbiosHandle;
  EFI_MISC_SYSTEM_SLOT_DESIGNATION*  ForType9InputData;

  ForType9InputData = (EFI_MISC_SYSTEM_SLOT_DESIGNATION *)RecordData;

  //
  // First check for invalid parameters.
  //
  if (RecordData == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  TokenToGet = 0;
  switch (ForType9InputData->SlotDesignation) {
    case STR_MISC_SYSTEM_SLOT_DESIGNATION: 
      TokenToGet = STRING_TOKEN (STR_MISC_SYSTEM_SLOT_DESIGNATION);
      break;
    default:
      break;
  }

  SlotDesignation = HiiGetPackageString(&gEfiCallerIdGuid, TokenToGet, NULL);
  SlotDesignationStrLen = StrLen(SlotDesignation);
  if (SlotDesignationStrLen > SMBIOS_STRING_MAX_LENGTH) {
    return EFI_UNSUPPORTED;
  }
  
  //
  // Two zeros following the last string.
  //
  SmbiosRecord = AllocatePool(sizeof (SMBIOS_TABLE_TYPE9) + SlotDesignationStrLen + 1 + 1);
  ZeroMem(SmbiosRecord, sizeof (SMBIOS_TABLE_TYPE9) +SlotDesignationStrLen + 1 + 1);

  SmbiosRecord->Hdr.Type = EFI_SMBIOS_TYPE_SYSTEM_SLOTS;
  SmbiosRecord->Hdr.Length = sizeof (SMBIOS_TABLE_TYPE9);
  SmbiosRecord->Hdr.Handle = 0; 
  SmbiosRecord->SlotDesignation = 1;
  SmbiosRecord->SlotType = ForType9InputData->SlotType;
  SmbiosRecord->SlotDataBusWidth = ForType9InputData->SlotDataBusWidth;
  SmbiosRecord->CurrentUsage = ForType9InputData->SlotUsage;
  SmbiosRecord->SlotLength = ForType9InputData->SlotLength;
  SmbiosRecord->SlotID = ForType9InputData->SlotId;
  
  //
  // Slot Characteristics
  //
  CopyMem ((UINT8 *) &SmbiosRecord->SlotCharacteristics1,(UINT8 *) &ForType9InputData->SlotCharacteristics,2);
  OptionalStrStart = (CHAR8 *)(SmbiosRecord + 1);
  UnicodeStrToAsciiStr(SlotDesignation, OptionalStrStart);
  //  
  // Now we have got the full smbios record, call smbios protocol to add this record.
  //
  SmbiosHandle = 0;
  Status = Smbios-> Add(
                      Smbios, 
                      NULL,
                      &SmbiosHandle, 
                      (EFI_SMBIOS_TABLE_HEADER *) SmbiosRecord
                      );
  FreePool(SmbiosRecord);
  return Status;
}
