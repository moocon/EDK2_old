/** @file
  AsmWriteDr0 function

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
  Writes a value to Debug Register 0 (DR0).

  Writes and returns a new value to DR0. This function is only available on
  IA-32 and X64. This writes a 32-bit value on IA-32 and a 64-bit value on X64.

  @param  Dr0 The value to write to Dr0.

  @return The value written to Debug Register 0 (DR0).

**/
UINTN
EFIAPI
AsmWriteDr0 (
  IN UINTN Value
  )
{
  _asm {
    mov     eax, Value
    mov     dr0, eax
  }
}

