/** @file
  AsmWriteCr4 function

  Copyright (c) 2006 - 2007, Intel Corporation<BR>
  All rights reserved. This program and the accompanying materials
  are licensed and made available under the terms and conditions of the BSD License
  which accompanies this distribution.  The full text of the license may be found at
  http://opensource.org/licenses/bsd-license.php

  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

//
// Include common header file for this module.
//


/**
  Writes a value to Control Register 4 (CR4).

  Writes and returns a new value to CR4. This function is only available on
  IA-32 and X64. This writes a 32-bit value on IA-32 and a 64-bit value on X64.

  @param  Cr4 The value to write to CR4.

  @return The value written to CR4.

**/
UINTN
EFIAPI
AsmWriteCr4 (
  UINTN  Value
  )
{
  _asm {
    mov     eax, Value
    _emit  0x0f  // mov  cr4, eax
    _emit  0x22
    _emit  0xE0
  }
}

