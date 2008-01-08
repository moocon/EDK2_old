/*++

Copyright (c) 2004 - 2007, Intel Corporation
All rights reserved. This program and the accompanying materials
are licensed and made available under the terms and conditions of the BSD License
which accompanies this distribution.  The full text of the license may be found at
http://opensource.org/licenses/bsd-license.php

THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

Module Name:

  Bds.h

Abstract:

  Head file for BDS Architectural Protocol implementation

Revision History

--*/

#ifndef _BDS_MODULE_H_
#define _BDS_MODULE_H_

#undef EFI_SPECIFICATION_VERSION
#define EFI_SPECIFICATION_VERSION 0x0002000A
#include <PiDxe.h>
#include <MdeModuleHii.h>

#include <Guid/FileSystemVolumeLabelInfo.h>
#include <Protocol/DevicePath.h>
#include <Guid/BootState.h>
#include <Guid/DataHubRecords.h>
#include <Protocol/LoadFile.h>
#include <Protocol/CpuIo.h>
#include <Guid/HobList.h>
#include <Guid/FileInfo.h>
#include <Protocol/HiiConfigRouting.h>
#include <Protocol/Bds.h>
#include <Protocol/DataHub.h>
#include <Protocol/UgaDraw.h>
#include <Protocol/BlockIo.h>
#include <Guid/GlobalVariable.h>
#include <Guid/GenericPlatformVariable.h>
#include <Guid/CapsuleVendor.h>
#include <Protocol/ConsoleControl.h>
#include <Protocol/GenericMemoryTest.h>
#include <Protocol/FormBrowser2.h>
#include <Protocol/HiiConfigAccess.h>
#include <Protocol/GraphicsOutput.h>
#include <Protocol/SimpleFileSystem.h>
#include <Protocol/HiiDatabase.h>
#include <Protocol/HiiString.h>
#include <Protocol/SerialIo.h>
#include <Protocol/LegacyBios.h>
#include <Protocol/SimpleTextInEx.h>
#include <Protocol/Performance.h>

#include <Library/UefiDriverEntryPoint.h>
#include <Library/PrintLib.h>
#include <Library/DebugLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiLib.h>
#include <Library/GraphicsLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/DxeServicesTableLib.h>
#include <Library/PerformanceLib.h>
#include <Library/ReportStatusCodeLib.h>
#include <Library/UefiIfrSupportLib.h>
#include <Library/UefiRuntimeServicesTableLib.h>
#include <Library/HobLib.h>
#include <Library/BaseLib.h>
#include <Library/DevicePathLib.h>
#include <Library/PcdLib.h>
#include <Library/CapsuleLib.h>
#include <Library/HiiLib.h>

#include <Library/GenericBdsLib.h>
#include <Library/PlatformBdsLib.h>

#define EFI_BDS_ARCH_PROTOCOL_INSTANCE_FROM_THIS(_this) \
  CR (_this, \
      EFI_BDS_ARCH_PROTOCOL_INSTANCE, \
      Bds, \
      EFI_BDS_ARCH_PROTOCOL_INSTANCE_SIGNATURE \
      )

EFI_STATUS
PlatformBdsShowProgress (
  IN EFI_GRAPHICS_OUTPUT_BLT_PIXEL TitleForeground,
  IN EFI_GRAPHICS_OUTPUT_BLT_PIXEL TitleBackground,
  IN CHAR16                        *Title,
  IN EFI_GRAPHICS_OUTPUT_BLT_PIXEL ProgressColor,
  IN UINTN                         Progress,
  IN UINTN                         PreviousValue
  );

//
// Prototypes
//
EFI_STATUS
EFIAPI
BdsInitialize (
  IN EFI_HANDLE                     ImageHandle,
  IN EFI_SYSTEM_TABLE               *SystemTable
  );

VOID
EFIAPI
BdsEntry (
  IN  EFI_BDS_ARCH_PROTOCOL *This
  );

#endif
