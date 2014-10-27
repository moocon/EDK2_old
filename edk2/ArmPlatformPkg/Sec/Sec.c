/** @file
*  Main file supporting the SEC Phase on ARM Platforms
*
*  Copyright (c) 2011-2014, ARM Limited. All rights reserved.
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

#include <Library/ArmTrustedMonitorLib.h>
#include <Library/DebugAgentLib.h>
#include <Library/PrintLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/SerialPortLib.h>
#include <Library/ArmGicLib.h>
#include <Library/ArmPlatformLib.h>

#include "SecInternal.h"

#define SerialPrint(txt)  SerialPortWrite ((UINT8*)txt, AsciiStrLen(txt)+1);

VOID
CEntryPoint (
  IN  UINTN                     MpId,
  IN  UINTN                     SecBootMode
  )
{
  CHAR8           Buffer[100];
  UINTN           CharCount;
  UINTN           JumpAddress;

  // Invalidate the data cache. Doesn't have to do the Data cache clean.
  ArmInvalidateDataCache ();

  // Invalidate Instruction Cache
  ArmInvalidateInstructionCache ();

  // Invalidate I & D TLBs
  ArmInvalidateTlb ();

  // CPU specific settings
  ArmCpuSetup (MpId);

  // Enable Floating Point Coprocessor if supported by the platform
  if (FixedPcdGet32 (PcdVFPEnabled)) {
    ArmEnableVFP ();
  }

  // Initialize peripherals that must be done at the early stage
  // Example: Some L2 controller, interconnect, clock, DMC, etc
  ArmPlatformSecInitialize (MpId);

  // Primary CPU clears out the SCU tag RAMs, secondaries wait
  if (ArmPlatformIsPrimaryCore (MpId) && (SecBootMode == ARM_SEC_COLD_BOOT)) {
    if (ArmIsMpCore()) {
      // Signal for the initial memory is configured (event: BOOT_MEM_INIT)
      ArmCallSEV ();
    }

    // SEC phase needs to run library constructors by hand. This assumes we are linked against the SerialLib
    // In non SEC modules the init call is in autogenerated code.
    SerialPortInitialize ();

    // Start talking
    if (FixedPcdGetBool (PcdTrustzoneSupport)) {
      CharCount = AsciiSPrint (Buffer,sizeof (Buffer),"Secure firmware (version %s built at %a on %a)\n\r",
          (CHAR16*)PcdGetPtr(PcdFirmwareVersionString), __TIME__, __DATE__);
    } else {
      CharCount = AsciiSPrint (Buffer,sizeof (Buffer),"Boot firmware (version %s built at %a on %a)\n\r",
          (CHAR16*)PcdGetPtr(PcdFirmwareVersionString), __TIME__, __DATE__);
    }
    SerialPortWrite ((UINT8 *) Buffer, CharCount);

    // Initialize the Debug Agent for Source Level Debugging
    InitializeDebugAgent (DEBUG_AGENT_INIT_PREMEM_SEC, NULL, NULL);
    SaveAndSetDebugTimerInterrupt (TRUE);

    // Enable the GIC distributor and CPU Interface
    // - no other Interrupts are enabled,  doesn't have to worry about the priority.
    // - all the cores are in secure state, use secure SGI's
    ArmGicEnableDistributor (PcdGet32(PcdGicDistributorBase));
    ArmGicEnableInterruptInterface (PcdGet32(PcdGicInterruptInterfaceBase));
  } else {
    // Enable the GIC CPU Interface
    ArmGicEnableInterruptInterface (PcdGet32(PcdGicInterruptInterfaceBase));
  }

  // Enable Full Access to CoProcessors
  ArmWriteCpacr (CPACR_CP_FULL_ACCESS);

  // Test if Trustzone is supported on this platform
  if (FixedPcdGetBool (PcdTrustzoneSupport)) {
    if (ArmIsMpCore ()) {
      // Setup SMP in Non Secure world
      ArmCpuSetupSmpNonSecure (GET_CORE_ID(MpId));
    }

    // Either we use the Secure Stacks for Secure Monitor (in this case (Base == 0) && (Size == 0))
    // Or we use separate Secure Monitor stacks (but (Base != 0) && (Size != 0))
    ASSERT (((PcdGet32(PcdCPUCoresSecMonStackBase) == 0) && (PcdGet32(PcdCPUCoreSecMonStackSize) == 0)) ||
            ((PcdGet32(PcdCPUCoresSecMonStackBase) != 0) && (PcdGet32(PcdCPUCoreSecMonStackSize) != 0)));

    // Enter Monitor Mode
    enter_monitor_mode (
      (UINTN)TrustedWorldInitialization, MpId, SecBootMode,
      (VOID*) (PcdGet32 (PcdCPUCoresSecMonStackBase) +
          (PcdGet32 (PcdCPUCoreSecMonStackSize) * (ArmPlatformGetCorePosition (MpId) + 1)))
      );
  } else {
    if (ArmPlatformIsPrimaryCore (MpId)) {
      SerialPrint ("Trust Zone Configuration is disabled\n\r");
    }

    // With Trustzone support the transition from Sec to Normal world is done by return_from_exception().
    // If we want to keep this function call we need to ensure the SVC's SPSR point to the same Program
    // Status Register as the the current one (CPSR).
    copy_cpsr_into_spsr ();

    // Call the Platform specific function to execute additional actions if required
    JumpAddress = PcdGet32 (PcdFvBaseAddress);
    ArmPlatformSecExtraAction (MpId, &JumpAddress);

    NonTrustedWorldTransition (MpId, JumpAddress);
  }
  ASSERT (0); // We must never return from the above function
}

VOID
TrustedWorldInitialization (
  IN  UINTN                     MpId,
  IN  UINTN                     SecBootMode
  )
{
  UINTN   JumpAddress;

  //-------------------- Monitor Mode ---------------------

  // Set up Monitor World (Vector Table, etc)
  ArmSecureMonitorWorldInitialize ();

  // Transfer the interrupt to Non-secure World
  ArmGicSetupNonSecure (MpId, PcdGet32(PcdGicDistributorBase), PcdGet32(PcdGicInterruptInterfaceBase));

  // Initialize platform specific security policy
  ArmPlatformSecTrustzoneInit (MpId);

  // Setup the Trustzone Chipsets
  if (SecBootMode == ARM_SEC_COLD_BOOT) {
    if (ArmPlatformIsPrimaryCore (MpId)) {
      if (ArmIsMpCore()) {
        // Signal the secondary core the Security settings is done (event: EVENT_SECURE_INIT)
        ArmCallSEV ();
      }
    } else {
      // The secondary cores need to wait until the Trustzone chipsets configuration is done
      // before switching to Non Secure World

      // Wait for the Primary Core to finish the initialization of the Secure World (event: EVENT_SECURE_INIT)
      ArmCallWFE ();
    }
  }

  // Call the Platform specific function to execute additional actions if required
  JumpAddress = PcdGet32 (PcdFvBaseAddress);
  ArmPlatformSecExtraAction (MpId, &JumpAddress);

  // Initialize architecture specific security policy
  ArmSecArchTrustzoneInit ();

  // CP15 Secure Configuration Register
  ArmWriteScr (PcdGet32 (PcdArmScr));

  NonTrustedWorldTransition (MpId, JumpAddress);
}

VOID
NonTrustedWorldTransition (
  IN  UINTN                     MpId,
  IN  UINTN                     JumpAddress
  )
{
  // If PcdArmNonSecModeTransition is defined then set this specific mode to CPSR before the transition
  // By not set, the mode for Non Secure World is SVC
  if (PcdGet32 (PcdArmNonSecModeTransition) != 0) {
    set_non_secure_mode ((ARM_PROCESSOR_MODE)PcdGet32 (PcdArmNonSecModeTransition));
  }

  return_from_exception (JumpAddress);
  //-------------------- Non Secure Mode ---------------------

  // PEI Core should always load and never return
  ASSERT (FALSE);
}

