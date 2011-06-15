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

#include <Library/IoLib.h>
#include <Library/ArmTrustZoneLib.h>
#include <Library/ArmPlatformLib.h>
#include <Library/DebugLib.h>
#include <Library/PcdLib.h>
#include <Library/SerialPortLib.h>

#include <Drivers/PL341Dmc.h>
#include <Drivers/PL301Axi.h>
#include <Drivers/SP804Timer.h>

#define SerialPrint(txt)  SerialPortWrite ((UINT8*)(txt), AsciiStrLen(txt)+1);

// DDR2 timings
PL341_DMC_CONFIG DDRTimings = {
  .base   = ARM_VE_DMC_BASE,
  .phy_ctrl_base  = 0x0,  //There is no DDR2 PHY controller on CTA9 test chip
  .MaxChip   = 1,
  .IsUserCfg = TRUE,
  .User0Cfg = 0x7C924924,
  .User2Cfg = (TC_UIOLHXC_VALUE << TC_UIOLHNC_SHIFT) | (TC_UIOLHXC_VALUE << TC_UIOLHPC_SHIFT) | (0x1 << TC_UIOHOCT_SHIFT) | (0x1 << TC_UIOHSTOP_SHIFT),
  .HasQos    = TRUE,
  .refresh_prd  = 0x3D0,
  .cas_latency  = 0x8,
  .write_latency  = 0x3,
  .t_mrd    = 0x2,
  .t_ras    = 0xA,
  .t_rc   = 0xE,
  .t_rcd    = 0x104,
  .t_rfc    = 0x2f32,
  .t_rp   = 0x14,
  .t_rrd    = 0x2,
  .t_wr   = 0x4,
  .t_wtr    = 0x2,
  .t_xp   = 0x2,
  .t_xsr    = 0xC8,
  .t_esr    = 0x14,
  .MemoryCfg   = DMC_MEMORY_CONFIG_ACTIVE_CHIP_1 | DMC_MEMORY_CONFIG_BURST_4 |
                        DMC_MEMORY_CONFIG_ROW_ADDRESS_15 | DMC_MEMORY_CONFIG_COLUMN_ADDRESS_10,
  .MemoryCfg2  = DMC_MEMORY_CFG2_DQM_INIT | DMC_MEMORY_CFG2_CKE_INIT |
            DMC_MEMORY_CFG2_BANK_BITS_3 | DMC_MEMORY_CFG2_MEM_WIDTH_32,
  .MemoryCfg3  = 0x00000001,
  .ChipCfg0    = 0x00010000,
  .t_faw    = 0x00000A0D,
  .ModeReg = DDR2_MR_BURST_LENGTH_4 | DDR2_MR_CAS_LATENCY_4 | DDR2_MR_WR_CYCLES_4,
  .ExtModeReg = DDR_EMR_RTT_50R | (DDR_EMR_ODS_VAL << DDR_EMR_ODS_MASK),
};

/**
  Return if Trustzone is supported by your platform

  A non-zero value must be returned if you want to support a Secure World on your platform.
  ArmVExpressTrustzoneInit() will later set up the secure regions.
  This function can return 0 even if Trustzone is supported by your processor. In this case,
  the platform will continue to run in Secure World.

  @return   A non-zero value if Trustzone supported.

**/
UINTN
ArmPlatformTrustzoneSupported (
  VOID
  )
{
  return (MmioRead32(ARM_VE_SYS_CFGRW1_REG) & ARM_VE_CFGRW1_TZASC_EN_BIT_MASK);
}

/**
  Return the current Boot Mode

  This function returns the boot reason on the platform

  @return   Return the current Boot Mode of the platform

**/
EFI_BOOT_MODE
ArmPlatformGetBootMode (
  VOID
  )
{
  return BOOT_WITH_FULL_CONFIGURATION;
}

/**
  Remap the memory at 0x0

  Some platform requires or gives the ability to remap the memory at the address 0x0.
  This function can do nothing if this feature is not relevant to your platform.

**/
VOID
ArmPlatformBootRemapping (
  VOID
  )
{
  UINT32 Value;

  if (FeaturePcdGet(PcdNorFlashRemapping)) {
    SerialPrint ("Secure ROM at 0x0\n\r");
  } else {
    Value = MmioRead32(ARM_VE_SYS_CFGRW1_REG); //Scc - CFGRW1
    // Remap the DRAM to 0x0
    MmioWrite32(ARM_VE_SYS_CFGRW1_REG, (Value & 0x0FFFFFFF) | ARM_VE_CFGRW1_REMAP_DRAM);
  }
}

/**
  Initialize controllers that must setup in the normal world

  This function is called by the ArmPlatformPkg/Pei or ArmPlatformPkg/Pei/PlatformPeim
  in the PEI phase.

**/
VOID
ArmPlatformNormalInitialize (
  VOID
  )
{
  // Configure periodic timer (TIMER0) for 1MHz operation
  MmioOr32 (SP810_CTRL_BASE + SP810_SYS_CTRL_REG, SP810_SYS_CTRL_TIMER0_TIMCLK);
  // Configure 1MHz clock
  MmioOr32 (SP810_CTRL_BASE + SP810_SYS_CTRL_REG, SP810_SYS_CTRL_TIMER1_TIMCLK);
  // configure SP810 to use 1MHz clock and disable
  MmioAndThenOr32 (SP810_CTRL_BASE + SP810_SYS_CTRL_REG, ~SP810_SYS_CTRL_TIMER2_EN, SP810_SYS_CTRL_TIMER2_TIMCLK);
  // Configure SP810 to use 1MHz clock and disable
  MmioAndThenOr32 (SP810_CTRL_BASE + SP810_SYS_CTRL_REG, ~SP810_SYS_CTRL_TIMER3_EN, SP810_SYS_CTRL_TIMER3_TIMCLK);
}

/**
  Initialize the system (or sometimes called permanent) memory

  This memory is generally represented by the DRAM.

**/
VOID
ArmPlatformInitializeSystemMemory (
  VOID
  )
{
  PL341DmcInit(&DDRTimings);
  PL301AxiInit(ARM_VE_FAXI_BASE);
}
