#/** @file
# Beagle board package.
#
# Copyright (c) 2009, Apple Inc. All rights reserved.
#
#  All rights reserved. This program and the accompanying materials
#    are licensed and made available under the terms and conditions of the BSD License
#    which accompanies this distribution. The full text of the license may be found at
#    http://opensource.org/licenses/bsd-license.php
#
#    THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
#    WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.
#
#**/

################################################################################
#
# Defines Section - statements that will be processed to create a Makefile.
#
################################################################################
[Defines]
  PLATFORM_NAME                  = BeagleBoardPkg
  PLATFORM_GUID                  = 91fa6c28-33df-46ac-aee6-292d6811ea31
  PLATFORM_VERSION               = 0.1
  DSC_SPECIFICATION              = 0x00010005
  OUTPUT_DIRECTORY               = Build/BeagleBoard
  SUPPORTED_ARCHITECTURES        = ARM
  BUILD_TARGETS                  = DEBUG|RELEASE
  SKUID_IDENTIFIER               = DEFAULT
  FLASH_DEFINITION               = BeagleBoardPkg/BeagleBoardPkg.fdf
  DEFINE TARGET_HACK             = DEBUG


[LibraryClasses.common]
!if $(BUILD_TARGETS) == RELEASE
  DebugLib|MdePkg/Library/BaseDebugLibNull/BaseDebugLibNull.inf
  UncachedMemoryAllocationLib|ArmPkg/Library/UncachedMemoryAllocationLib/UncachedMemoryAllocationLib.inf
!else
  DebugLib|MdePkg/Library/BaseDebugLibSerialPort/BaseDebugLibSerialPort.inf
  UncachedMemoryAllocationLib|ArmPkg/Library/DebugUncachedMemoryAllocationLib/DebugUncachedMemoryAllocationLib.inf
!endif

  ArmLib|ArmPkg/Library/ArmLib/ArmV7/ArmV7Lib.inf
  MemoryAllocationLib|MdePkg/Library/UefiMemoryAllocationLib/UefiMemoryAllocationLib.inf
  
  BaseLib|MdePkg/Library/BaseLib/BaseLib.inf
  BaseMemoryLib|MdePkg/Library/BaseMemoryLib/BaseMemoryLib.inf

  BeagleBoardSystemLib|BeagleBoardPkg/Library/BeagleBoardSystemLib/BeagleBoardSystemLib.inf
  EfiResetSystemLib|BeagleBoardPkg/Library/ResetSystemLib/ResetSystemLib.inf
  
  PciLib|MdePkg/Library/BasePciLibCf8/BasePciLibCf8.inf
  PerformanceLib|MdePkg/Library/BasePerformanceLibNull/BasePerformanceLibNull.inf
  PrintLib|MdePkg/Library/BasePrintLib/BasePrintLib.inf
  
  EblCmdLib|BeagleBoardPkg/Library/EblCmdLib/EblCmdLib.inf
  
  EfiFileLib|EmbeddedPkg/Library/EfiFileLib/EfiFileLib.inf
  
  
  PeCoffGetEntryPointLib|MdePkg/Library/BasePeCoffGetEntryPointLib/BasePeCoffGetEntryPointLib.inf
  
  #
  # Uncomment (and comment out the next line) For RealView Debugger. The Standard IO window 
  # in the debugger will show load and unload commands for symbols. You can cut and paste this
  # into the command window to load symbols. We should be able to use a script to do this, but
  # the version of RVD I have does not support scipts accessing system memory.
  #
#  PeCoffExtraActionLib|ArmPkg/Library/RvdPeCoffExtraActionLib/RvdPeCoffExtraActionLib.inf
  PeCoffExtraActionLib|MdePkg/Library/BasePeCoffExtraActionLibNull/BasePeCoffExtraActionLibNull.inf

  
  CacheMaintenanceLib|ArmPkg/Library/ArmCacheMaintenanceLib/ArmCacheMaintenanceLib.inf
  DefaultExceptioHandlerLib|ArmPkg/Library/DefaultExceptionHandlerLib/DefaultExceptionHandlerLib.inf
  PrePiLib|EmbeddedPkg/Library/PrePiLib/PrePiLib.inf
  
  SerialPortLib|Omap35xxPkg/Library/SerialPortLib/SerialPortLib.inf
  SemihostLib|ArmPkg/Library/SemihostLib/SemihostLib.inf
  
  RealTimeClockLib|EmbeddedPkg/Library/TemplateRealTimeClockLib/TemplateRealTimeClockLib.inf

  IoLib|MdePkg/Library/BaseIoLibIntrinsic/BaseIoLibIntrinsic.inf
  
  MemoryAllocationLib|MdePkg/Library/UefiMemoryAllocationLib/UefiMemoryAllocationLib.inf
  UefiLib|MdePkg/Library/UefiLib/UefiLib.inf
  HobLib|MdePkg/Library/DxeHobLib/DxeHobLib.inf
  UefiRuntimeServicesTableLib|MdePkg/Library/UefiRuntimeServicesTableLib/UefiRuntimeServicesTableLib.inf
  DevicePathLib|MdePkg/Library/UefiDevicePathLib/UefiDevicePathLib.inf
  UefiBootServicesTableLib|MdePkg/Library/UefiBootServicesTableLib/UefiBootServicesTableLib.inf

  DxeServicesTableLib|MdePkg/Library/DxeServicesTableLib/DxeServicesTableLib.inf
  UefiDriverEntryPoint|MdePkg/Library/UefiDriverEntryPoint/UefiDriverEntryPoint.inf
  UefiApplicationEntryPoint|MdePkg/Library/UefiApplicationEntryPoint/UefiApplicationEntryPoint.inf

#
# Assume everything is fixed at build
#
  PcdLib|MdePkg/Library/BasePcdLibNull/BasePcdLibNull.inf

  UefiRuntimeLib|MdePkg/Library/UefiRuntimeLib/UefiRuntimeLib.inf
  
  UefiUsbLib|MdePkg/Library/UefiUsbLib/UefiUsbLib.inf
  EblAddExternalCommandLib|EmbeddedPkg/Library/EblAddExternalCommandLib/EblAddExternalCommandLib.inf
  

  CpuLib|MdePkg/Library/BaseCpuLib/BaseCpuLib.inf

  TimerLib|Omap35xxPkg/Library/BeagleBoardTimerLib/BeagleBoardTimerLib.inf  
  OmapLib|Omap35xxPkg/Library/OmapLib/OmapLib.inf
  EblNetworkLib|EmbeddedPkg/Library/EblNetworkLib/EblNetworkLib.inf
  
  GdbSerialLib|Omap35xxPkg/Library/GdbSerialLib/GdbSerialLib.inf
  ArmDisassemblerLib|ArmPkg/Library/ArmDisassemblerLib/ArmDisassemblerLib.inf
  DebugAgentLib|MdeModulePkg/Library/DebugAgentLibNull/DebugAgentLibNull.inf


[LibraryClasses.common.SEC]
  ArmLib|ArmPkg/Library/ArmLib/ArmV7/ArmV7LibPrePi.inf
  PcdLib|MdePkg/Library/BasePcdLibNull/BasePcdLibNull.inf
  ReportStatusCodeLib|IntelFrameworkModulePkg/Library/PeiDxeDebugLibReportStatusCode/PeiDxeDebugLibReportStatusCode.inf
  UefiDecompressLib|MdePkg/Library/BaseUefiDecompressLib/BaseUefiDecompressLib.inf
  ExtractGuidedSectionLib|EmbeddedPkg/Library/PrePiExtractGuidedSectionLib/PrePiExtractGuidedSectionLib.inf
  LzmaDecompressLib|IntelFrameworkModulePkg/Library/LzmaCustomDecompressLib/LzmaCustomDecompressLib.inf
  PeCoffLib|MdePkg/Library/BasePeCoffLib/BasePeCoffLib.inf

[LibraryClasses.common.PEI_CORE]
  PcdLib|MdePkg/Library/BasePcdLibNull/BasePcdLibNull.inf
  ReportStatusCodeLib|IntelFrameworkModulePkg/Library/PeiDxeDebugLibReportStatusCode/PeiDxeDebugLibReportStatusCode.inf

[LibraryClasses.common.DXE_CORE]
  HobLib|MdePkg/Library/DxeCoreHobLib/DxeCoreHobLib.inf
  MemoryAllocationLib|MdeModulePkg/Library/DxeCoreMemoryAllocationLib/DxeCoreMemoryAllocationLib.inf
  DxeCoreEntryPoint|MdePkg/Library/DxeCoreEntryPoint/DxeCoreEntryPoint.inf
  ReportStatusCodeLib|IntelFrameworkModulePkg/Library/DxeReportStatusCodeLibFramework/DxeReportStatusCodeLib.inf
  ExtractGuidedSectionLib|MdePkg/Library/DxeExtractGuidedSectionLib/DxeExtractGuidedSectionLib.inf
  UefiDecompressLib|MdePkg/Library/BaseUefiDecompressLib/BaseUefiDecompressLib.inf
  DxeServicesLib|MdePkg/Library/DxeServicesLib/DxeServicesLib.inf
#  PeCoffLib|MdePkg/Library/BasePeCoffLib/BasePeCoffLib.inf
  PeCoffLib|EmbeddedPkg/Library/DxeHobPeCoffLib/DxeHobPeCoffLib.inf

[LibraryClasses.common.DXE_DRIVER]
  ReportStatusCodeLib|IntelFrameworkModulePkg/Library/DxeReportStatusCodeLibFramework/DxeReportStatusCodeLib.inf
  DxeServicesLib|MdePkg/Library/DxeServicesLib/DxeServicesLib.inf
  SecurityManagementLib|MdeModulePkg/Library/DxeSecurityManagementLib/DxeSecurityManagementLib.inf


[LibraryClasses.common.UEFI_APPLICATION]
  ReportStatusCodeLib|IntelFrameworkModulePkg/Library/DxeReportStatusCodeLibFramework/DxeReportStatusCodeLib.inf
  UefiDecompressLib|IntelFrameworkModulePkg/Library/BaseUefiTianoCustomDecompressLib/BaseUefiTianoCustomDecompressLib.inf

[LibraryClasses.common.UEFI_DRIVER]
  ReportStatusCodeLib|IntelFrameworkModulePkg/Library/DxeReportStatusCodeLibFramework/DxeReportStatusCodeLib.inf
  UefiDecompressLib|IntelFrameworkModulePkg/Library/BaseUefiTianoCustomDecompressLib/BaseUefiTianoCustomDecompressLib.inf
  ExtractGuidedSectionLib|MdePkg/Library/DxeExtractGuidedSectionLib/DxeExtractGuidedSectionLib.inf

[LibraryClasses.common.DXE_RUNTIME_DRIVER]
  HobLib|MdePkg/Library/DxeHobLib/DxeHobLib.inf
  MemoryAllocationLib|MdePkg/Library/UefiMemoryAllocationLib/UefiMemoryAllocationLib.inf
  ReportStatusCodeLib|IntelFrameworkModulePkg/Library/DxeReportStatusCodeLibFramework/DxeReportStatusCodeLib.inf
  CapsuleLib|MdeModulePkg/Library/DxeCapsuleLibNull/DxeCapsuleLibNull.inf
#  PeCoffLib|MdePkg/Library/BasePeCoffLib/BasePeCoffLib.inf
  PeCoffLib|EmbeddedPkg/Library/DxeHobPeCoffLib/DxeHobPeCoffLib.inf


[LibraryClasses.ARM]
  #
  # It is not possible to prevent the ARM compiler for generic intrinsic functions.
  # This library provides the instrinsic functions generate by a given compiler.
  # [LibraryClasses.ARM] and NULL mean link this library into all ARM images.
  #
  NULL|ArmPkg/Library/CompilerIntrinsicsLib/CompilerIntrinsicsLib.inf


[BuildOptions]
  XCODE:*_*_ARM_ARCHCC_FLAGS     == -arch armv7 -march=armv7
  XCODE:*_*_ARM_ARCHASM_FLAGS    == -arch armv7
  XCODE:*_*_ARM_ARCHDLINK_FLAGS  == -arch armv7
  XCODE:RELEASE_*_*_CC_FLAGS     = -DMDEPKG_NDEBUG 

  RVCT:*_*_ARM_ARCHCC_FLAGS == --cpu 7-A --thumb
  RVCT:RELEASE_*_*_CC_FLAGS = -DMDEPKG_NDEBUG 

################################################################################
#
# Pcd Section - list of all EDK II PCD Entries defined by this Platform
#
################################################################################

[PcdsFeatureFlag.common]
  gEfiMdePkgTokenSpaceGuid.PcdComponentNameDisable|TRUE
  gEfiMdePkgTokenSpaceGuid.PcdDriverDiagnosticsDisable|TRUE
  gEfiMdePkgTokenSpaceGuid.PcdComponentName2Disable|TRUE
  gEfiMdePkgTokenSpaceGuid.PcdDriverDiagnostics2Disable|TRUE
  
  #
  # Control what commands are supported from the UI
  # Turn these on and off to add features or save size
  #  
  gEmbeddedTokenSpaceGuid.PcdEmbeddedMacBoot|TRUE
  gEmbeddedTokenSpaceGuid.PcdEmbeddedDirCmd|TRUE
  gEmbeddedTokenSpaceGuid.PcdEmbeddedHobCmd|TRUE
  gEmbeddedTokenSpaceGuid.PcdEmbeddedHwDebugCmd|TRUE
  gEmbeddedTokenSpaceGuid.PcdEmbeddedPciDebugCmd|TRUE
  gEmbeddedTokenSpaceGuid.PcdEmbeddedIoEnable|FALSE
  gEmbeddedTokenSpaceGuid.PcdEmbeddedScriptCmd|FALSE

  gEmbeddedTokenSpaceGuid.PcdCacheEnable|TRUE
  
  gEmbeddedTokenSpaceGuid.PcdPrePiProduceMemoryTypeInformationHob|TRUE
  gArmTokenSpaceGuid.PcdCpuDxeProduceDebugSupport|FALSE

  gEfiMdeModulePkgTokenSpaceGuid.PcdTurnOffUsbLegacySupport|TRUE
  
[PcdsFixedAtBuild.common]
  gEmbeddedTokenSpaceGuid.PcdEmbeddedPrompt|"BeagleEdk2"
  gEmbeddedTokenSpaceGuid.PcdPrePiCpuMemorySize|32
  gEmbeddedTokenSpaceGuid.PcdPrePiCpuIoSize|0
  gEfiMdePkgTokenSpaceGuid.PcdMaximumUnicodeStringLength|1000000
  gEfiMdePkgTokenSpaceGuid.PcdMaximumAsciiStringLength|1000000
  gEfiMdePkgTokenSpaceGuid.PcdMaximumLinkedListLength|1000000
  gEfiMdePkgTokenSpaceGuid.PcdSpinLockTimeout|10000000
  gEfiMdePkgTokenSpaceGuid.PcdDebugClearMemoryValue|0xAF
  gEfiMdePkgTokenSpaceGuid.PcdPerformanceLibraryPropertyMask|0
  gEfiMdePkgTokenSpaceGuid.PcdPostCodePropertyMask|0
  gEfiMdePkgTokenSpaceGuid.PcdUefiLibMaxPrintBufferSize|320

# DEBUG_ASSERT_ENABLED       0x01
# DEBUG_PRINT_ENABLED        0x02
# DEBUG_CODE_ENABLED         0x04
# CLEAR_MEMORY_ENABLED       0x08
# ASSERT_BREAKPOINT_ENABLED  0x10
# ASSERT_DEADLOOP_ENABLED    0x20

  gEfiMdePkgTokenSpaceGuid.PcdDebugPropertyMask|0x2f

#  DEBUG_INIT      0x00000001  // Initialization
#  DEBUG_WARN      0x00000002  // Warnings
#  DEBUG_LOAD      0x00000004  // Load events
#  DEBUG_FS        0x00000008  // EFI File system
#  DEBUG_POOL      0x00000010  // Alloc & Free's
#  DEBUG_PAGE      0x00000020  // Alloc & Free's
#  DEBUG_INFO      0x00000040  // Verbose
#  DEBUG_DISPATCH  0x00000080  // PEI/DXE Dispatchers
#  DEBUG_VARIABLE  0x00000100  // Variable
#  DEBUG_BM        0x00000400  // Boot Manager
#  DEBUG_BLKIO     0x00001000  // BlkIo Driver
#  DEBUG_NET       0x00004000  // SNI Driver
#  DEBUG_UNDI      0x00010000  // UNDI Driver
#  DEBUG_LOADFILE  0x00020000  // UNDI Driver
#  DEBUG_EVENT     0x00080000  // Event messages
#  DEBUG_ERROR     0x80000000  // Error

  gEfiMdePkgTokenSpaceGuid.PcdDebugPrintErrorLevel|0x80000004

  gEfiMdePkgTokenSpaceGuid.PcdReportStatusCodePropertyMask|0x07

  gEmbeddedTokenSpaceGuid.PcdEmbeddedAutomaticBootCommand|""
  gEmbeddedTokenSpaceGuid.PcdEmbeddedDefaultTextColor|0x07
  gEmbeddedTokenSpaceGuid.PcdEmbeddedMemVariableStoreSize|0x10000
  
  gEmbeddedTokenSpaceGuid.PcdPrePiTempMemorySize|0
  gEmbeddedTokenSpaceGuid.PcdPrePiBfvBaseAddress|0
  gEmbeddedTokenSpaceGuid.PcdPrePiBfvSize|0
  gEmbeddedTokenSpaceGuid.PcdFlashFvMainBase|0
  gEmbeddedTokenSpaceGuid.PcdFlashFvMainSize|0

#
# Optional feature to help prevent EFI memory map fragments
# Turned on and off via: PcdPrePiProduceMemoryTypeInformationHob
# Values are in EFI Pages (4K). DXE Core will make sure that 
# at least this much of each type of memory can be allocated 
# from a single memory range. This way you only end up with
# maximum of two fragements for each type in the memory map
# (the memory used, and the free memory that was prereserved
# but not used).
#
  gEmbeddedTokenSpaceGuid.PcdMemoryTypeEfiACPIReclaimMemory|0
  gEmbeddedTokenSpaceGuid.PcdMemoryTypeEfiACPIMemoryNVS|0
  gEmbeddedTokenSpaceGuid.PcdMemoryTypeEfiReservedMemoryType|0
  gEmbeddedTokenSpaceGuid.PcdMemoryTypeEfiRuntimeServicesData|80
  gEmbeddedTokenSpaceGuid.PcdMemoryTypeEfiRuntimeServicesCode|40
  gEmbeddedTokenSpaceGuid.PcdMemoryTypeEfiBootServicesCode|400
  gEmbeddedTokenSpaceGuid.PcdMemoryTypeEfiBootServicesData|3000
  gEmbeddedTokenSpaceGuid.PcdMemoryTypeEfiLoaderCode|10
  gEmbeddedTokenSpaceGuid.PcdMemoryTypeEfiLoaderData|0


#
# Beagle board Specific PCDs
#
  gEmbeddedTokenSpaceGuid.PcdPrePiHobBase|0x80001000
  gEmbeddedTokenSpaceGuid.PcdPrePiStackBase|0x87FE0000 # stack at top of memory
  gEmbeddedTokenSpaceGuid.PcdPrePiStackSize|0x20000  # 128K stack
  
  gEmbeddedTokenSpaceGuid.PcdMemoryBase|0x80000000
  gEmbeddedTokenSpaceGuid.PcdMemorySize|0x10000000


  gArmTokenSpaceGuid.PcdCpuVectorBaseAddress|0x80000000
  gArmTokenSpaceGuid.PcdCpuResetAddress|0x80008000
  
  gBeagleBoardTokenSpaceGuid.PcdBeagleGpmcOffset|0x6E000000
  gBeagleBoardTokenSpaceGuid.PcdBeagleMMCHS1Base|0x4809C000

  # Console  
  gBeagleBoardTokenSpaceGuid.PcdBeagleConsoleUart|3
  
  # Timers
  gBeagleBoardTokenSpaceGuid.PcdBeagleArchTimer|3
  gBeagleBoardTokenSpaceGuid.PcdBeagleFreeTimer|4
  gEmbeddedTokenSpaceGuid.PcdTimerPeriod|100000
  gEmbeddedTokenSpaceGuid.PcdEmbeddedFdPerformanceCounterPeriodInNanoseconds|77
  gEmbeddedTokenSpaceGuid.PcdEmbeddedFdPerformanceCounterFrequencyInHz|13000000
  
  #
  # ARM Pcds
  #
  gArmTokenSpaceGuid.PcdArmUncachedMemoryMask|0x0000000040000000

################################################################################
#
# Components Section - list of all EDK II Modules needed by this Platform
#
################################################################################
[Components.common]

#
# SEC
#
  BeagleBoardPkg/Sec/Sec.inf
  
#
# DXE
#
  MdeModulePkg/Core/Dxe/DxeMain.inf {
    <LibraryClasses>
      PcdLib|MdePkg/Library/BasePcdLibNull/BasePcdLibNull.inf
      NULL|MdeModulePkg/Library/DxeCrc32GuidedSectionExtractLib/DxeCrc32GuidedSectionExtractLib.inf
#      NULL|IntelFrameworkModulePkg/Library/LzmaCustomDecompressLib/LzmaCustomDecompressLib.inf
      NULL|EmbeddedPkg/Library/LzmaHobCustomDecompressLib/LzmaHobCustomDecompressLib.inf    
  }

  ArmPkg/Drivers/CpuDxe/CpuDxe.inf
  
  MdeModulePkg/Core/RuntimeDxe/RuntimeDxe.inf
  MdeModulePkg/Universal/SecurityStubDxe/SecurityStubDxe.inf
  MdeModulePkg/Universal/WatchdogTimerDxe/WatchdogTimer.inf
  MdeModulePkg/Universal/CapsuleRuntimeDxe/CapsuleRuntimeDxe.inf
  MdeModulePkg/Universal/Variable/EmuRuntimeDxe/EmuVariableRuntimeDxe.inf
  EmbeddedPkg/EmbeddedMonotonicCounter/EmbeddedMonotonicCounter.inf
  EmbeddedPkg/SimpleTextInOutSerial/SimpleTextInOutSerial.inf
  
  EmbeddedPkg/ResetRuntimeDxe/ResetRuntimeDxe.inf
  EmbeddedPkg/RealTimeClockRuntimeDxe/RealTimeClockRuntimeDxe.inf
  EmbeddedPkg/MetronomeDxe/MetronomeDxe.inf

  #
  # Semi-hosting filesystem
  #
  ArmPkg/Filesystem/SemihostFs/SemihostFs.inf
  
  #
  # FAT filesystem + GPT/MBR partitioning
  #
  MdeModulePkg/Universal/Disk/DiskIoDxe/DiskIoDxe.inf
  MdeModulePkg/Universal/Disk/PartitionDxe/PartitionDxe.inf
  FatPkg/EnhancedFatDxe/Fat.inf
  MdeModulePkg/Universal/Disk/UnicodeCollation/EnglishDxe/EnglishDxe.inf

  #
  # USB
  #
  Omap35xxPkg/PciEmulation/PciEmulation.inf

  #NOTE: Open source EHCI stack doesn't work on Beagleboard.
  #NOTE: UsbBus and UsbMassStorage don't work using iPhone SDK tool chain.
  MdeModulePkg/Bus/Pci/EhciDxe/EhciDxe.inf {
    <PcdsFixedAtBuild>
      gEfiMdePkgTokenSpaceGuid.PcdDebugPrintErrorLevel|0x800fffff
  }
  MdeModulePkg/Bus/Usb/UsbBusDxe/UsbBusDxe.inf
  MdeModulePkg/Bus/Usb/UsbMassStorageDxe/UsbMassStorageDxe.inf

  #
  # Nand Flash
  #
  Omap35xxPkg/Flash/Flash.inf

  #
  # MMC/SD
  #
  Omap35xxPkg/MMCHSDxe/MMCHS.inf
  
  #
  # I2C
  #
  Omap35xxPkg/SmbusDxe/Smbus.inf
  
  #
  # SoC Drivers
  #
  Omap35xxPkg/Gpio/Gpio.inf
  Omap35xxPkg/InterruptDxe/InterruptDxe.inf
  Omap35xxPkg/TimerDxe/TimerDxe.inf 
  
  #
  # Power IC
  #
  Omap35xxPkg/TPS65950Dxe/TPS65950.inf
  
  #
  # Application
  #  
  EmbeddedPkg/Ebl/Ebl.inf
  
  #
  # Bds
  #
  BeagleBoardPkg/Bds/Bds.inf  
  
  #
  # Gdb Stub
  #
  EmbeddedPkg/GdbStub/GdbStub.inf
  ArmPkg/Drivers/DebugSupportDxe/DebugSupportDxe.inf

