/*++

Copyright (c) 2006, Intel Corporation
All rights reserved. This program and the accompanying materials
are licensed and made available under the terms and conditions of the BSD License
which accompanies this distribution.  The full text of the license may be found at
http://opensource.org/licenses/bsd-license.php

THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

Module Name:

  MiscSubclassDriverEntryPoint.c

Abstract:

  This driver parses the mMiscSubclassDataTable structure and reports
  any generated data to the DataHub.

--*/

#include "MiscSubClassDriver.h"


extern UINT8  MiscSubClassStrings[];

//
//
//
EFI_STATUS
LogRecordDataToDataHub (
  EFI_DATA_HUB_PROTOCOL *DataHub,
  UINT32                RecordType,
  UINT32                RecordLen,
  VOID                  *RecordData
  )
/*++
Description:

Parameters:

  DataHub
    %%TBD

  RecordType
    %%TBD

  RecordLen
    %%TBD

  RecordData
    %%TBD

Returns:

  EFI_INVALID_PARAMETER

  EFI_SUCCESS

  Other Data Hub errors

--*/
{
  EFI_MISC_SUBCLASS_DRIVER_DATA MiscSubclass;
  EFI_STATUS                    EfiStatus;

  //
  // Do nothing if data parameters are not valid.
  //
  if (RecordLen == 0 || RecordData == NULL) {
    DEBUG (
      (EFI_D_ERROR,
      "RecordLen == %d  RecordData == %xh\n",
      RecordLen,
      RecordData)
      );

    return EFI_INVALID_PARAMETER;
  }
  //
  // Assemble Data Hub record.
  //
  MiscSubclass.Header.Version     = EFI_MISC_SUBCLASS_VERSION;
  MiscSubclass.Header.HeaderSize  = sizeof (EFI_SUBCLASS_TYPE1_HEADER);
  MiscSubclass.Header.Instance    = 1;
  MiscSubclass.Header.SubInstance = 1;
  MiscSubclass.Header.RecordType  = RecordType;

  CopyMem (
    &MiscSubclass.Record,
    RecordData,
    RecordLen
    );

  //
  // Log Data Hub record.
  //
  EfiStatus = DataHub->LogData (
                        DataHub,
                        &gEfiMiscSubClassGuid,
                        &gEfiMiscSubClassGuid,
                        EFI_DATA_RECORD_CLASS_DATA,
                        &MiscSubclass,
                        sizeof (EFI_SUBCLASS_TYPE1_HEADER) + RecordLen
                        );

  if (EFI_ERROR (EfiStatus)) {
    DEBUG (
      (EFI_D_ERROR,
      "LogData(%d bytes) == %r\n",
      sizeof (EFI_SUBCLASS_TYPE1_HEADER) + RecordLen,
      EfiStatus)
      );
  }

  return EfiStatus;
}


EFI_STATUS
EFIAPI
MiscSubclassDriverEntryPoint (
  IN EFI_HANDLE         ImageHandle,
  IN EFI_SYSTEM_TABLE   *SystemTable
  )
/*++
Description:

  Standard EFI driver point.  This driver parses the mMiscSubclassDataTable
  structure and reports any generated data to the DataHub.

Arguments:

  ImageHandle
    Handle for the image of this driver

  SystemTable
    Pointer to the EFI System Table

Returns:

  EFI_SUCCESS
    The data was successfully reported to the Data Hub.

--*/
{
  EFI_MISC_SUBCLASS_DRIVER_DATA     RecordData;
  EFI_DATA_HUB_PROTOCOL             *DataHub;
  EFI_HII_HANDLE                    HiiHandle;
  EFI_STATUS                        EfiStatus;
  UINTN                             Index;
  BOOLEAN                           LogRecordData;
  EFI_MEMORY_SUBCLASS_DRIVER_DATA   MemorySubClassData; 
  UINT64                            TotalMemorySize;
  CHAR16                            *UnixMemString;



  //
  // Initialize constant portion of subclass header.
  //
  RecordData.Header.Version     = EFI_MISC_SUBCLASS_VERSION;
  RecordData.Header.HeaderSize  = sizeof (EFI_SUBCLASS_TYPE1_HEADER);
  RecordData.Header.Instance    = 1;
  RecordData.Header.SubInstance = 1;

  //
  // Locate data hub protocol.
  //
  EfiStatus = gBS->LocateProtocol (&gEfiDataHubProtocolGuid, NULL, (VOID**) &DataHub);

  if (EFI_ERROR (EfiStatus)) {
    DEBUG ((EFI_D_ERROR, "Could not locate DataHub protocol.  %r\n", EfiStatus));
    return EfiStatus;
  } else if (DataHub == NULL) {
    DEBUG ((EFI_D_ERROR, "LocateProtocol(DataHub) returned NULL pointer!\n"));
    return EFI_DEVICE_ERROR;
  }

  //
  // Add our default strings to the HII database. They will be modified later.
  //
  HiiHandle = HiiAddPackages (
                &gEfiMiscSubClassGuid,
                NULL,
                MiscSubclassStrings,
                NULL
                );
  if (HiiHandle == NULL) {
    DEBUG ((EFI_D_ERROR, "Could not log default strings to Hii.  %r\n", EfiStatus));
    return EFI_OUT_OF_RESOURCES;
  }

  //
  //
  //
  for (Index = 0; Index < mMiscSubclassDataTableEntries; ++Index) {
    //
    // Stupidity check!  Do nothing if RecordLen is zero.
    // %%TBD - Should this be an error or a mechanism for ignoring
    // records in the Data Table?
    //
    if (mMiscSubclassDataTable[Index].RecordLen == 0) {
      DEBUG (
        (EFI_D_ERROR,
        "mMiscSubclassDataTable[%d].RecordLen == 0\n",
        Index)
        );

      continue;
    }
    //
    // Initialize per-record portion of subclass header and
    // copy static data into data portion of subclass record.
    //
    RecordData.Header.RecordType = mMiscSubclassDataTable[Index].RecordType;

    if (mMiscSubclassDataTable[Index].RecordData == NULL) {
      ZeroMem (
        &RecordData.Record,
        mMiscSubclassDataTable[Index].RecordLen
        );
    } else {
      CopyMem (
        &RecordData.Record,
        mMiscSubclassDataTable[Index].RecordData,
        mMiscSubclassDataTable[Index].RecordLen
        );
    }
    //
    // If the entry does not have a function pointer, just log the data.
    //
    if (mMiscSubclassDataTable[Index].Function == NULL) {
      //
      // Log RecordData to Data Hub.
      //
      EfiStatus = DataHub->LogData (
                            DataHub,
                            &gEfiMiscSubClassGuid,
                            &gEfiMiscSubClassGuid,
                            EFI_DATA_RECORD_CLASS_DATA,
                            &RecordData,
                            sizeof (EFI_SUBCLASS_TYPE1_HEADER) + mMiscSubclassDataTable[Index].RecordLen
                            );

      if (EFI_ERROR (EfiStatus)) {
        DEBUG (
          (EFI_D_ERROR,
          "LogData(%d bytes) == %r\n",
          sizeof (EFI_SUBCLASS_TYPE1_HEADER) + mMiscSubclassDataTable[Index].RecordLen,
          EfiStatus)
          );
      }

      continue;
    }
    //
    // The entry has a valid function pointer.
    // Keep calling the function and logging data until there
    // is no more data to log.
    //
    for (;;) {
      EfiStatus = (*mMiscSubclassDataTable[Index].Function)(mMiscSubclassDataTable[Index].RecordType, &mMiscSubclassDataTable[Index].RecordLen, &RecordData.Record, &LogRecordData);

      if (EFI_ERROR (EfiStatus)) {
        break;
      }

      if (!LogRecordData) {
        break;
      }
      //
      //
      //
      EfiStatus = DataHub->LogData (
                            DataHub,
                            &gEfiMiscSubClassGuid,
                            &gEfiMiscSubClassGuid,
                            EFI_DATA_RECORD_CLASS_DATA,
                            &RecordData,
                            sizeof (EFI_SUBCLASS_TYPE1_HEADER) + mMiscSubclassDataTable[Index].RecordLen
                            );

      if (EFI_ERROR (EfiStatus)) {
        DEBUG (
          (EFI_D_ERROR,
          "LogData(%d bytes) == %r\n",
          sizeof (EFI_SUBCLASS_TYPE1_HEADER) + mMiscSubclassDataTable[Index].RecordLen,
          EfiStatus)
          );
      }
    }
  }
  
  //
  // Log Memory Size info based on PCD setting.
  //
  MemorySubClassData.Header.Instance    = 1;
  MemorySubClassData.Header.SubInstance = EFI_SUBCLASS_INSTANCE_NON_APPLICABLE;
  MemorySubClassData.Header.RecordType  = EFI_MEMORY_ARRAY_START_ADDRESS_RECORD_NUMBER;

  //
  // Process Memory String in form size!size ...
  // So 64!64 is 128 MB
  //
  UnixMemString   = PcdGetPtr (PcdUnixMemorySize);
  for (TotalMemorySize = 0; *UnixMemString != '\0';) {
    TotalMemorySize += StrDecimalToUint64 (UnixMemString);
    while (*UnixMemString != '\0') {
      if (*UnixMemString == '!') {
        UnixMemString++;       
        break;
      }
      UnixMemString++;
    }
  }

  MemorySubClassData.Record.ArrayStartAddress.MemoryArrayStartAddress               = 0;
  MemorySubClassData.Record.ArrayStartAddress.MemoryArrayEndAddress                 = LShiftU64 (TotalMemorySize, 20) - 1;  
  CopyGuid (&MemorySubClassData.Record.ArrayStartAddress.PhysicalMemoryArrayLink.ProducerName, &gEfiCallerIdGuid);
  MemorySubClassData.Record.ArrayStartAddress.PhysicalMemoryArrayLink.Instance      = 1;
  MemorySubClassData.Record.ArrayStartAddress.PhysicalMemoryArrayLink.SubInstance   = EFI_SUBCLASS_INSTANCE_NON_APPLICABLE;
  MemorySubClassData.Record.ArrayStartAddress.MemoryArrayPartitionWidth             = 0;

  //
  // Store memory size data record to data hub.
  //
  EfiStatus = DataHub->LogData (
                      DataHub,
                      &gEfiMemorySubClassGuid,
                      &gEfiCallerIdGuid,
                      EFI_DATA_RECORD_CLASS_DATA,
                      &MemorySubClassData,
                      sizeof (EFI_SUBCLASS_TYPE1_HEADER) + sizeof (EFI_MEMORY_ARRAY_START_ADDRESS_DATA)
                      );

  return EFI_SUCCESS;
}
