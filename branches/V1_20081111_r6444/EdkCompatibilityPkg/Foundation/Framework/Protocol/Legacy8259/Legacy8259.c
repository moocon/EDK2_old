/*++

Copyright (c) 1999 - 2006, Intel Corporation
All rights reserved. This program and the accompanying materials
are licensed and made available under the terms and conditions of the BSD License
which accompanies this distribution.  The full text of the license may be found at
http://opensource.org/licenses/bsd-license.php

THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.


Module Name:
  
    Legacy8259.c
    
Abstract:

  EFI Legacy Region Protocol

Revision History

--*/

#include "Tiano.h"

#include EFI_PROTOCOL_DEFINITION (Legacy8259)

EFI_GUID  gEfiLegacy8259ProtocolGuid = EFI_LEGACY_8259_PROTOCOL_GUID;

EFI_GUID_STRING(&gEfiLegacy8259ProtocolGuid, "Legacy 8259 Protocol", "Legacy 8259 Protocol");
