/** @file

  Copyright (c) 2011, ARM Limited. All rights reserved.

  This program and the accompanying materials
  are licensed and made available under the terms and conditions of the BSD License
  which accompanies this distribution.  The full text of the license may be found at
  http://opensource.org/licenses/bsd-license.php

  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

#include <Base.h>
#include <Library/ArmLib.h>
#include <Library/ArmCpuLib.h>
#include <Library/ArmGicLib.h>
#include <Library/ArmV7ArchTimerLib.h>
#include <Library/DebugLib.h>
#include <Library/IoLib.h>
#include <Library/PcdLib.h>

#include <Chipset/ArmV7.h>

VOID
ArmCpuSynchronizeSignal (
  IN ARM_CPU_SYNCHRONIZE_EVENT   Event
  )
{
  if (Event == ARM_CPU_EVENT_BOOT_MEM_INIT) {
    // Do nothing, Cortex A15 secondary cores are waiting for the GIC Distributor
    // to be enabled (done by the Sec module itself) as a way to know when the Init Boot
    // Mem as been initialized
  } else {
    // Send SGI to all Secondary core to wake them up from WFI state.
    ArmGicSendSgiTo (PcdGet32(PcdGicDistributorBase), ARM_GIC_ICDSGIR_FILTER_EVERYONEELSE, 0x0E);
  }
}

VOID
CArmCpuSynchronizeWait (
  IN ARM_CPU_SYNCHRONIZE_EVENT   Event
  )
{
  // Waiting for the SGI from the primary core
  ArmCallWFI ();

  // Acknowledge the interrupt and send End of Interrupt signal.
  ArmGicAcknowledgeSgiFrom (PcdGet32(PcdGicInterruptInterfaceBase), PRIMARY_CORE_ID);
}

VOID
ArmCpuSetup (
  IN  UINTN         MpId
  )
{
  // Check if Architectural Timer frequency is valid number (should not be 0)
  ASSERT (PcdGet32 (PcdArmArchTimerFreqInHz));
  ASSERT(ArmIsArchTimerImplemented () != 0);

  // Enable SWP instructions
  ArmEnableSWPInstruction ();

  // Enable program flow prediction, if supported.
  ArmEnableBranchPrediction ();

  // Note: System Counter frequency can only be set in Secure privileged mode,
  // if security extensions are implemented.
  ArmArchTimerSetTimerFreq (PcdGet32 (PcdArmArchTimerFreqInHz));

  /*// If MPCore then Enable the SCU
  if (ArmIsMpCore()) {
    ArmEnableScu ();
  }*/
}


VOID
ArmCpuSetupSmpNonSecure (
  IN  UINTN         MpId
  )
{
  //ArmSetAuxCrBit (A15_FEATURE_SMP);

  /*// Make the SCU accessible in Non Secure world
  if (IS_PRIMARY_CORE(MpId)) {
    ScuBase = ArmGetScuBaseAddress();

    // Allow NS access to SCU register
    MmioOr32 (ScuBase + A9_SCU_SACR_OFFSET, 0xf);
    // Allow NS access to Private Peripherals
    MmioOr32 (ScuBase + A9_SCU_SSACR_OFFSET, 0xfff);
  }*/
}
