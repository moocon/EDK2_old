/** @file
  OEM hook status code library functions with no library constructor/destructor

  Copyright (c) 2006 - 2008, Intel Corporation
  All rights reserved. This program and the accompanying materials
  are licensed and made available under the terms and conditions of the BSD License
  which accompanies this distribution.  The full text of the license may be found at
  http://opensource.org/licenses/bsd-license.php

  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

  Module Name:  UnixOemHookStatusCodeLib.c

**/
#include "PiPei.h"
#include <Guid/StatusCodeDataTypeId.h>
#include <Ppi/UnixThunk.h>
#include <FrameworkModuleBase.h>
#include <Library/OemHookStatusCodeLib.h>
#include <Library/DebugLib.h>
#include <Library/PrintLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/ReportStatusCodeLib.h>
#include <Library/PeiServicesLib.h>
#include <DebugInfo.h>

//
// Cache of UnixThunk protocol 
//
STATIC
EFI_UNIX_THUNK_PROTOCOL   *mUnix;

//
// Cache of standard output handle . 
//
STATIC
int                      mStdOut;

/**

  Initialize OEM status code device .

  @return    Always return EFI_SUCCESS.

**/
EFI_STATUS
EFIAPI
OemHookStatusCodeInitialize (
  VOID
  )
{
  PEI_UNIX_THUNK_PPI  *UnixThunkPpi;
  EFI_STATUS        Status;

  
  //
  // Locate Unix ThunkPpi for retrieving standard output handle
  //
  Status = PeiServicesLocatePpi (
              &gPeiUnixThunkPpiGuid,
              0,
              NULL,
              (VOID **) &UnixThunkPpi
              );

  ASSERT_EFI_ERROR (Status);

  mUnix  = (EFI_UNIX_THUNK_PROTOCOL *) UnixThunkPpi->UnixThunk ();
  
  //
  // Cache standard output handle.
  //
  mStdOut = 1;

  return EFI_SUCCESS;
}

/**
  Report status code to OEM device.
 
  @param  CodeType      Indicates the type of status code being reported.  Type EFI_STATUS_CODE_TYPE is defined in "Related Definitions" below.
 
  @param  Value         Describes the current status of a hardware or software entity.  
                        This included information about the class and subclass that is used to classify the entity 
                        as well as an operation.  For progress codes, the operation is the current activity. 
                        For error codes, it is the exception.  For debug codes, it is not defined at this time. 
                        Type EFI_STATUS_CODE_VALUE is defined in "Related Definitions" below.  
                        Specific values are discussed in the Intel? Platform Innovation Framework for EFI Status Code Specification.
 
  @param  Instance      The enumeration of a hardware or software entity within the system.  
                        A system may contain multiple entities that match a class/subclass pairing. 
                        The instance differentiates between them.  An instance of 0 indicates that instance information is unavailable, 
                        not meaningful, or not relevant.  Valid instance numbers start with 1.


  @param  CallerId      This optional parameter may be used to identify the caller. 
                        This parameter allows the status code driver to apply different rules to different callers. 
                        Type EFI_GUID is defined in InstallProtocolInterface() in the EFI 1.10 Specification.


  @param  Data          This optional parameter may be used to pass additional data
 
  @return               The function always return EFI_SUCCESS.

**/
EFI_STATUS
EFIAPI
OemHookStatusCodeReport (
  IN EFI_STATUS_CODE_TYPE     CodeType,
  IN EFI_STATUS_CODE_VALUE    Value,
  IN UINT32                   Instance,
  IN EFI_GUID                 *CallerId, OPTIONAL
  IN EFI_STATUS_CODE_DATA     *Data      OPTIONAL
  )
{
  CHAR8           *Filename;
  CHAR8           *Description;
  CHAR8           *Format;
  CHAR8           Buffer[EFI_STATUS_CODE_DATA_MAX_SIZE];
  UINT32          ErrorLevel;
  UINT32          LineNumber;
  UINTN           CharCount;
  VA_LIST         Marker;
  EFI_DEBUG_INFO  *DebugInfo;

  Buffer[0] = '\0';

  if (Data != NULL &&
      ReportStatusCodeExtractAssertInfo (CodeType, Value, Data, &Filename, &Description, &LineNumber)) {
    //
    // Print ASSERT() information into output buffer.
    //
    CharCount = AsciiSPrint (
                  Buffer,
                  EFI_STATUS_CODE_DATA_MAX_SIZE,
                  "\n\rASSERT!: %a (%d): %a\n\r",
                  Filename,
                  LineNumber,
                  Description
                  );

    //
    // Callout to standard output.
    //
    mUnix->Write (
              mStdOut,
              Buffer,
              CharCount
	      );

    return EFI_SUCCESS;

  } else if (Data != NULL &&
             ReportStatusCodeExtractDebugInfo (Data, &ErrorLevel, &Marker, &Format)) {
    //
    // Print DEBUG() information into output buffer.
    //
    CharCount = AsciiVSPrint (
                  Buffer, 
                  EFI_STATUS_CODE_DATA_MAX_SIZE, 
                  Format, 
                  Marker
                  );
  } else if (Data != NULL && 
             CompareGuid (&Data->Type, &gEfiStatusCodeSpecificDataGuid) &&
             (CodeType & EFI_STATUS_CODE_TYPE_MASK) == EFI_DEBUG_CODE) {
    //
    // Print specific data into output buffer.
    //
    DebugInfo = (EFI_DEBUG_INFO *) (Data + 1);
    Marker    = (VA_LIST) (DebugInfo + 1);
    Format    = (CHAR8 *) (((UINT64 *) Marker) + 12);

    CharCount = AsciiVSPrint (Buffer, EFI_STATUS_CODE_DATA_MAX_SIZE, Format, Marker);
  } else if ((CodeType & EFI_STATUS_CODE_TYPE_MASK) == EFI_ERROR_CODE) {
    //
    // Print ERROR information into output buffer.
    //
    CharCount = AsciiSPrint (
                  Buffer, 
                  EFI_STATUS_CODE_DATA_MAX_SIZE, 
                  "ERROR: C%x:V%x I%x", 
                  CodeType, 
                  Value, 
                  Instance
                  );

    //
    // Make sure we don't try to print values that weren't intended to be printed, especially NULL GUID pointers.
    //
    
    if (CallerId != NULL) {
      CharCount += AsciiSPrint (
                     &Buffer[CharCount - 1],
                     (EFI_STATUS_CODE_DATA_MAX_SIZE - (sizeof (Buffer[0]) * CharCount)),
                     " %g",
                     CallerId
                     );
    }

    if (Data != NULL) {
      CharCount += AsciiSPrint (
                     &Buffer[CharCount - 1],
                     (EFI_STATUS_CODE_DATA_MAX_SIZE - (sizeof (Buffer[0]) * CharCount)),
                     " %x",
                     Data
                     );
    }

    CharCount += AsciiSPrint (
                   &Buffer[CharCount - 1],
                   (EFI_STATUS_CODE_DATA_MAX_SIZE - (sizeof (Buffer[0]) * CharCount)),
                   "\n\r"
                   );
  } else if ((CodeType & EFI_STATUS_CODE_TYPE_MASK) == EFI_PROGRESS_CODE) {
    CharCount = AsciiSPrint (
                  Buffer, 
                  EFI_STATUS_CODE_DATA_MAX_SIZE, 
                  "PROGRESS CODE: V%x I%x\n\r", 
                  Value, 
                  Instance
                  );
  } else {
    CharCount = AsciiSPrint (
                  Buffer, 
                  EFI_STATUS_CODE_DATA_MAX_SIZE, 
                  "Undefined: C%x:V%x I%x\n\r", 
                  CodeType, 
                  Value, 
                  Instance
                  );
  }

  //
  // Callout to standard output.
  //
  mUnix->Write (
            mStdOut,
            Buffer,
            CharCount
            );

  return EFI_SUCCESS;
}

