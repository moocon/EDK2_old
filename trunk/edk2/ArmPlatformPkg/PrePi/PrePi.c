/** @file
*
*  Copyright (c) 2011, ARM Limited. All rights reserved.
*
*  This program and the accompanying materials
*  are licensed and made available under the terms and conditions of the BSD License
*  which accompanies this distribution.  The full text of the license may be found at
*  http://opensource.org/licenses/bsd-license.php
*
*  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
*  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.
*
**/

#include <PiPei.h>

#include <Library/DebugAgentLib.h>
#include <Library/PrePiLib.h>
#include <Library/IoLib.h>
#include <Library/PrintLib.h>
#include <Library/PeCoffGetEntryPointLib.h>
#include <Library/TimerLib.h>
#include <Library/PerformanceLib.h>

#include <Ppi/GuidedSectionExtraction.h>
#include <Guid/LzmaDecompress.h>

#include "PrePi.h"
#include "LzmaDecompress.h"

VOID
PrePiCommonExceptionEntry (
  IN UINT32 Entry,
  IN UINT32 LR
  );

EFI_STATUS
EFIAPI
ExtractGuidedSectionLibConstructor (
  VOID
  );

EFI_STATUS
EFIAPI
LzmaDecompressLibConstructor (
  VOID
  );

VOID
PrePiMain (
  IN  UINTN                     UefiMemoryBase,
  IN  UINTN                     StackBase,
  IN  UINT64                    StartTimeStamp
  )
{
  EFI_HOB_HANDOFF_INFO_TABLE**   PrePiHobBase;
  EFI_STATUS                    Status;
  CHAR8                         Buffer[100];
  UINTN                         CharCount;

  // Enable program flow prediction, if supported.
  ArmEnableBranchPrediction ();

  if (FixedPcdGet32(PcdVFPEnabled)) {
    ArmEnableVFP();
  }

  // Initialize the Serial Port
  SerialPortInitialize ();
  CharCount = AsciiSPrint (Buffer,sizeof (Buffer),"UEFI firmware built at %a on %a\n\r",__TIME__, __DATE__);
  SerialPortWrite ((UINT8 *) Buffer, CharCount);

  // Initialize the Debug Agent for Source Level Debugging
  InitializeDebugAgent (DEBUG_AGENT_INIT_POSTMEM_SEC, NULL, NULL);
  SaveAndSetDebugTimerInterrupt (TRUE);

  PrePiHobBase = (EFI_HOB_HANDOFF_INFO_TABLE**)(PcdGet32 (PcdCPUCoresNonSecStackBase) + (PcdGet32 (PcdCPUCoresNonSecStackSize) / 2) - PcdGet32 (PcdHobListPtrGlobalOffset));

  // We leave UINT32 at the top of UEFI memory for PcdPrePiHobBase
  *PrePiHobBase = HobConstructor (
    (VOID*)UefiMemoryBase,
    FixedPcdGet32 (PcdSystemMemoryUefiRegionSize),
    (VOID*)UefiMemoryBase,
    (VOID*)(UefiMemoryBase + FixedPcdGet32 (PcdSystemMemoryUefiRegionSize) - sizeof(UINT32)));

  // Initialize MMU and Memory HOBs (Resource Descriptor HOBs)
  Status = MemoryPeim (UefiMemoryBase, FixedPcdGet32 (PcdSystemMemoryUefiRegionSize));
  ASSERT_EFI_ERROR (Status);

  // Create the Stack HOB
  BuildStackHob (StackBase, FixedPcdGet32(PcdCPUCoresNonSecStackSize));

  // Set the Boot Mode
  SetBootMode (ArmPlatformGetBootMode ());

  // Initialize Platform HOBs (CpuHob and FvHob)
  Status = PlatformPeim ();
  ASSERT_EFI_ERROR (Status);

  BuildMemoryTypeInformationHob ();

  //InitializeDebugAgent (DEBUG_AGENT_INIT_PREMEM_SEC, NULL, NULL);
  //SaveAndSetDebugTimerInterrupt (TRUE);

  // Now, the HOB List has been initialized, we can register performance information
  PERF_START (NULL, "PEI", NULL, StartTimeStamp);

  // SEC phase needs to run library constructors by hand.
  ExtractGuidedSectionLibConstructor ();
  LzmaDecompressLibConstructor ();

  // Build HOBs to pass up our version of stuff the DXE Core needs to save space
  BuildPeCoffLoaderHob ();
  BuildExtractSectionHob (
    &gLzmaCustomDecompressGuid,
    LzmaGuidedSectionGetInfo,
    LzmaGuidedSectionExtraction
    );

  // Assume the FV that contains the SEC (our code) also contains a compressed FV.
  Status = DecompressFirstFv ();
  ASSERT_EFI_ERROR (Status);

  // Load the DXE Core and transfer control to it
  Status = LoadDxeCoreFromFv (NULL, 0);
  ASSERT_EFI_ERROR (Status);
}

VOID
CEntryPoint (
  IN  UINTN                     CoreId,
  IN  UINTN                     UefiMemoryBase,
  IN  UINTN                     StackBase
  )
{
  UINT64   StartTimeStamp;

  StartTimeStamp = 0;

  if ((CoreId == 0) && PerformanceMeasurementEnabled ()) {
    // Initialize the Timer Library to setup the Timer HW controller
    TimerConstructor ();
    // We cannot call yet the PerformanceLib because the HOB List has not been initialized
    StartTimeStamp = GetPerformanceCounter ();
  }

  //Clean Data cache
  ArmCleanInvalidateDataCache();

  //Invalidate instruction cache
  ArmInvalidateInstructionCache();

  //TODO:Drain Write Buffer

  // Enable Instruction & Data caches
  ArmEnableDataCache();
  ArmEnableInstructionCache();

  // Write VBAR - The Vector table must be 32-byte aligned
  ASSERT(((UINT32)PrePiVectorTable & ((1 << 5)-1)) == 0);
  ArmWriteVBar((UINT32)PrePiVectorTable);

  //If not primary Jump to Secondary Main
  if(0 == CoreId) {
    // Goto primary Main.
    PrimaryMain (UefiMemoryBase, StackBase, StartTimeStamp);
  } else {
    SecondaryMain (CoreId);
  }

  // DXE Core should always load and never return
  ASSERT (FALSE);
}

VOID
PrePiCommonExceptionEntry (
  IN UINT32 Entry,
  IN UINT32 LR
  )
{
  CHAR8           Buffer[100];
  UINTN           CharCount;

  switch (Entry) {
  case 0:
    CharCount = AsciiSPrint (Buffer,sizeof (Buffer),"Reset Exception at 0x%X\n\r",LR);
    break;
  case 1:
    CharCount = AsciiSPrint (Buffer,sizeof (Buffer),"Undefined Exception at 0x%X\n\r",LR);
    break;
  case 2:
    CharCount = AsciiSPrint (Buffer,sizeof (Buffer),"SWI Exception at 0x%X\n\r",LR);
    break;
  case 3:
    CharCount = AsciiSPrint (Buffer,sizeof (Buffer),"PrefetchAbort Exception at 0x%X\n\r",LR);
    break;
  case 4:
    CharCount = AsciiSPrint (Buffer,sizeof (Buffer),"DataAbort Exception at 0x%X\n\r",LR);
    break;
  case 5:
    CharCount = AsciiSPrint (Buffer,sizeof (Buffer),"Reserved Exception at 0x%X\n\r",LR);
    break;
  case 6:
    CharCount = AsciiSPrint (Buffer,sizeof (Buffer),"IRQ Exception at 0x%X\n\r",LR);
    break;
  case 7:
    CharCount = AsciiSPrint (Buffer,sizeof (Buffer),"FIQ Exception at 0x%X\n\r",LR);
    break;
  default:
    CharCount = AsciiSPrint (Buffer,sizeof (Buffer),"Unknown Exception at 0x%X\n\r",LR);
    break;
  }
  SerialPortWrite ((UINT8 *) Buffer, CharCount);
  while(1);
}
