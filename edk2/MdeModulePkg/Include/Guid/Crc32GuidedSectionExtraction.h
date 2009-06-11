/** @file

  This file defines a group of guids to specify the different 
  encapsulation scheme for the GUIDed section.

  Only one GUID is defined for the CRC32 encapsulation scheme.

  Copyright (c) 2006 - 2009, Intel Corporation
  All rights reserved. This program and the accompanying materials
  are licensed and made available under the terms and conditions of the BSD License
  which accompanies this distribution.  The full text of the license may be found at
  http://opensource.org/licenses/bsd-license.php

  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

#ifndef __CRC32_GUIDED_SECTION_EXTRACTION_H__
#define __CRC32_GUIDED_SECTION_EXTRACTION_H__

//
// GUID definition. All GUIDed section extraction protocols share the
// same interface, but each has a different GUID. All the GUIDs are defined here.
//
#define EFI_CRC32_GUIDED_SECTION_EXTRACTION_GUID \
  { 0xFC1BCDB0, 0x7D31, 0x49aa, {0x93, 0x6A, 0xA4, 0x60, 0x0D, 0x9D, 0xD0, 0x83 } }

extern EFI_GUID gEfiCrc32GuidedSectionExtractionGuid;

#endif
