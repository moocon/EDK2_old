/** @file
*  Main file supporting the SEC Phase for Versatile Express
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

#include <Library/DebugLib.h>
#include <Library/DebugAgentLib.h>
#include <Library/PcdLib.h>
#include <Library/PrintLib.h>
#include <Library/BaseLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/ArmLib.h>
#include <Library/SerialPortLib.h>
#include <Library/ArmPlatformLib.h>

#include <Chipset/ArmV7.h>
#include <Library/ArmGicLib.h>

#define SerialPrint(txt)  SerialPortWrite ((UINT8*)txt, AsciiStrLen(txt)+1);

extern VOID *monitor_vector_table;

VOID
ArmSetupGicNonSecure (
  IN  INTN          GicDistributorBase,
  IN  INTN          GicInterruptInterfaceBase
);

// Vector Table for Sec Phase
VOID
SecVectorTable (
  VOID
  );

VOID
NonSecureWaitForFirmware (
  VOID
  );

VOID
enter_monitor_mode(
  IN VOID* Stack
  );

VOID
return_from_exception (
  IN UINTN NonSecureBase
  );

VOID
copy_cpsr_into_spsr (
  VOID
  );

VOID
CEntryPoint (
  IN  UINTN                     MpId
  )
{
  CHAR8           Buffer[100];
  UINTN           CharCount;
  UINTN           JumpAddress;

  // Primary CPU clears out the SCU tag RAMs, secondaries wait
  if (IS_PRIMARY_CORE(MpId)) {
    if (FixedPcdGet32(PcdMPCoreSupport)) {
      ArmInvalidScu();
    }

    // SEC phase needs to run library constructors by hand. This assumes we are linked against the SerialLib
    // In non SEC modules the init call is in autogenerated code.
    SerialPortInitialize ();

    // Start talking
    CharCount = AsciiSPrint (Buffer,sizeof (Buffer),"UEFI firmware built at %a on %a\n\r",__TIME__, __DATE__);
    SerialPortWrite ((UINT8 *) Buffer, CharCount);

    // Initialize the Debug Agent for Source Level Debugging
    InitializeDebugAgent (DEBUG_AGENT_INIT_PREMEM_SEC, NULL, NULL);
    SaveAndSetDebugTimerInterrupt (TRUE);

    // Now we've got UART, make the check:
    // - The Vector table must be 32-byte aligned
    ASSERT(((UINT32)SecVectorTable & ((1 << 5)-1)) == 0);
  }

  // Invalidate the data cache. Doesn't have to do the Data cache clean.
  ArmInvalidateDataCache();

  //Invalidate Instruction Cache
  ArmInvalidateInstructionCache();

  //Invalidate I & D TLBs
  ArmInvalidateInstructionAndDataTlb();

  // Enable Full Access to CoProcessors
  ArmWriteCPACR (CPACR_CP_FULL_ACCESS);

  // Enable SWP instructions
  ArmEnableSWPInstruction();

  // Enable program flow prediction, if supported.
  ArmEnableBranchPrediction();

  if (FixedPcdGet32(PcdVFPEnabled)) {
    ArmEnableVFP();
  }

  if (IS_PRIMARY_CORE(MpId)) {
    // Initialize peripherals that must be done at the early stage
    // Example: Some L2x0 controllers must be initialized in Secure World
    ArmPlatformSecInitialize ();

    // If we skip the PEI Core we could want to initialize the DRAM in the SEC phase.
    // If we are in standalone, we need the initialization to copy the UEFI firmware into DRAM
    if (FeaturePcdGet(PcdSystemMemoryInitializeInSec)) {
      // Initialize system memory (DRAM)
      ArmPlatformInitializeSystemMemory ();
    }

    // Some platform can change their physical memory mapping
    ArmPlatformBootRemapping ();
  }

  // Test if Trustzone is supported on this platform
  if (ArmPlatformTrustzoneSupported()) {
    if (FixedPcdGet32(PcdMPCoreSupport)) {
      // Setup SMP in Non Secure world
      ArmSetupSmpNonSecure (GET_CORE_ID(MpId));
    }

    // Enter Monitor Mode
    enter_monitor_mode ((VOID*)(PcdGet32(PcdCPUCoresSecMonStackBase) + (PcdGet32(PcdCPUCoreSecMonStackSize) * GET_CORE_POS(MpId))));

    //Write the monitor mode vector table address
    ArmWriteVMBar((UINT32) &monitor_vector_table);

    //-------------------- Monitor Mode ---------------------
    // Setup the Trustzone Chipsets
    if (IS_PRIMARY_CORE(MpId)) {
      ArmPlatformTrustzoneInit();

      // Wake up the secondary cores by sending a interrupt to everyone else
      // NOTE 1: The Software Generated Interrupts are always enabled on Cortex-A9
      //         MPcore test chip on Versatile Express board, So the Software doesn't have to
      //         enable SGI's explicitly.
      //      2: As no other Interrupts are enabled,  doesn't have to worry about the priority.
      //      3: As all the cores are in secure state, use secure SGI's
      //

      ArmGicEnableDistributor (PcdGet32(PcdGicDistributorBase));
      ArmGicEnableInterruptInterface (PcdGet32(PcdGicInterruptInterfaceBase));

      // Send SGI to all Secondary core to wake them up from WFI state.
      ArmGicSendSgiTo (PcdGet32(PcdGicDistributorBase), ARM_GIC_ICDSGIR_FILTER_EVERYONEELSE, 0x0E);
    } else {
      // The secondary cores need to wait until the Trustzone chipsets configuration is done
      // before switching to Non Secure World

      // Enabled GIC CPU Interface
      ArmGicEnableInterruptInterface (PcdGet32(PcdGicInterruptInterfaceBase));

      // Waiting for the SGI from the primary core
      ArmCallWFI();

      // Acknowledge the interrupt and send End of Interrupt signal.
      ArmGicAcknowledgeSgiFrom (PcdGet32(PcdGicInterruptInterfaceBase), PRIMARY_CORE_ID);
    }

    // Transfer the interrupt to Non-secure World
    ArmGicSetupNonSecure (PcdGet32(PcdGicDistributorBase),PcdGet32(PcdGicInterruptInterfaceBase));

    // Write to CP15 Non-secure Access Control Register :
    //   - Enable CP10 and CP11 accesses in NS World
    //   - Enable Access to Preload Engine in NS World
    //   - Enable lockable TLB entries allocation in NS world
    //   - Enable R/W access to SMP bit of Auxiliary Control Register in NS world
    ArmWriteNsacr(NSACR_NS_SMP | NSACR_TL | NSACR_PLE | NSACR_CP(10) | NSACR_CP(11));

    // CP15 Secure Configuration Register with Non Secure bit (SCR_NS), CPSR.A modified in any
    // security state (SCR_AW), CPSR.F modified in any security state (SCR_FW)
    ArmWriteScr(SCR_NS | SCR_FW | SCR_AW);
  } else {
    if (IS_PRIMARY_CORE(MpId)) {
      SerialPrint ("Trust Zone Configuration is disabled\n\r");
    }

    // Trustzone is not enabled, just enable the Distributor and CPU interface
    if (IS_PRIMARY_CORE(MpId)) {
      ArmGicEnableDistributor (PcdGet32(PcdGicDistributorBase));
    }
    ArmGicEnableInterruptInterface (PcdGet32(PcdGicInterruptInterfaceBase));

    // With Trustzone support the transition from Sec to Normal world is done by return_from_exception().
    // If we want to keep this function call we need to ensure the SVC's SPSR point to the same Program
    // Status Register as the the current one (CPSR).
    copy_cpsr_into_spsr ();
  }

  JumpAddress = PcdGet32 (PcdFvBaseAddress);
  ArmPlatformSecExtraAction (MpId, &JumpAddress);

  return_from_exception (JumpAddress);
  //-------------------- Non Secure Mode ---------------------

  // PEI Core should always load and never return
  ASSERT (FALSE);
}

VOID
SecCommonExceptionEntry (
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
