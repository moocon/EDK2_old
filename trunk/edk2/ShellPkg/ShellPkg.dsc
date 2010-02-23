## @file
# Shell Package
#
# Copyright (c) 2007 - 2010, Intel Corporation
#
#  All rights reserved. This program and the accompanying materials
#    are licensed and made available under the terms and conditions of the BSD License
#    which accompanies this distribution. The full text of the license may be found at
#    http://opensource.org/licenses/bsd-license.php
#
#    THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
#    WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.
#
##

[Defines]
  PLATFORM_NAME                  = Shell
  PLATFORM_GUID                  = E1DC9BF8-7013-4c99-9437-795DAA45F3BD
  PLATFORM_VERSION               = 0.2
  DSC_SPECIFICATION              = 0x00010006
  OUTPUT_DIRECTORY               = Build/Shell
  SUPPORTED_ARCHITECTURES        = IA32|IPF|X64|EBC
  BUILD_TARGETS                  = DEBUG|RELEASE
  SKUID_IDENTIFIER               = DEFAULT

[LibraryClasses]
  UefiApplicationEntryPoint|MdePkg/Library/UefiApplicationEntryPoint/UefiApplicationEntryPoint.inf
  UefiBootServicesTableLib|MdePkg/Library/UefiBootServicesTableLib/UefiBootServicesTableLib.inf
  DevicePathLib|MdePkg/Library/UefiDevicePathLib/UefiDevicePathLib.inf
  DebugLib|MdePkg/Library/BaseDebugLibNull/BaseDebugLibNull.inf
  PcdLib|MdePkg/Library/BasePcdLibNull/BasePcdLibNull.inf
  MemoryAllocationLib|MdePkg/Library/UefiMemoryAllocationLib/UefiMemoryAllocationLib.inf
  UefiLib|MdePkg/Library/UefiLib/UefiLib.inf
  BaseLib|MdePkg/Library/BaseLib/BaseLib.inf
  BaseMemoryLib|MdePkg/Library/BaseMemoryLib/BaseMemoryLib.inf
  PrintLib|MdeModulePkg/Library/DxePrintLibPrint2Protocol/DxePrintLibPrint2Protocol.inf
  UefiRuntimeServicesTableLib|MdePkg/Library/UefiRuntimeServicesTableLib/UefiRuntimeServicesTableLib.inf
  UefiHiiServicesLib|MdeModulePkg/Library/UefiHiiServicesLib/UefiHiiServicesLib.inf
  HiiLib|MdeModulePkg/Library/UefiHiiLib/UefiHiiLib.inf
  
  ShellLib|ShellPkg/Library/UefiShellLib/UefiShellLib.inf
  FileHandleLib|ShellPkg/Library/BaseFileHandleLib/BaseFileHandleLib.inf
  ShellCEntryLib|ShellPkg/Library/UefiShellCEntryLib/UefiShellCEntryLib.inf
  SortLib|ShellPkg/Library/BaseSortLib/BaseSortLib.inf

[PcdsFixedAtBuild]

[Components]
  ShellPkg/Application/ShellExecTestApp/SA.inf
  ShellPkg/Application/ShellLibTestApp/SA3.inf
  ShellPkg/Library/BaseFileHandleLib/BaseFileHandleLib.inf
  ShellPkg/Library/UefiShellLib/UefiShellLib.inf
  ShellPkg/Library/UefiShellCEntryLib/UefiShellCEntryLib.inf
  ShellPkg/Library/BaseSortLib/BaseSortLib.inf
  ShellPkg/Application/ShellCTestApp/ShellCTestApp.inf
  ShellPkg/Application/ShellSortTestApp/ShellSortTestApp.inf