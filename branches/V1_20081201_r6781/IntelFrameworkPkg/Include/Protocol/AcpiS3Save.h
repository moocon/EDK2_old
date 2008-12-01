/** @file
  This protocol is used to prepare all information that is needed for the S3 resume boot path. This
  protocol is not required for all platforms.
  This protocol is defined in framework S3Resume v0.9, page 21.
  
Copyright (c) 2006, Intel Corporation
All rights reserved. This program and the accompanying materials
are licensed and made available under the terms and conditions of the BSD License
which accompanies this distribution.  The full text of the license may be found at
http://opensource.org/licenses/bsd-license.php

THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

#ifndef _ACPI_S3_SAVE_PROTOCOL_H
#define _ACPI_S3_SAVE_PROTOCOL_H

#include <PiDxe.h>

//
// Forward reference for pure ANSI compatability
//
typedef struct _EFI_ACPI_S3_SAVE_PROTOCOL EFI_ACPI_S3_SAVE_PROTOCOL;

//
// S3 Save Protocol GUID
//
#define EFI_ACPI_S3_SAVE_GUID \
  { \
    0x125f2de1, 0xfb85, 0x440c, {0xa5, 0x4c, 0x4d, 0x99, 0x35, 0x8a, 0x8d, 0x38 } \
  }

//
// Protocol Data Structures
//
typedef
EFI_STATUS
(EFIAPI *EFI_ACPI_S3_SAVE)(
  IN EFI_ACPI_S3_SAVE_PROTOCOL      * This,
  IN VOID                           * LegacyMemoryAddress
  );

typedef
EFI_STATUS
(EFIAPI *EFI_ACPI_GET_LEGACY_MEMORY_SIZE)(
  IN  EFI_ACPI_S3_SAVE_PROTOCOL     * This,
  OUT UINTN                         * Size
);

struct _EFI_ACPI_S3_SAVE_PROTOCOL {
  EFI_ACPI_GET_LEGACY_MEMORY_SIZE   GetLegacyMemorySize;
  EFI_ACPI_S3_SAVE                  S3Save;
};

extern EFI_GUID gEfiAcpiS3SaveProtocolGuid;

#endif
