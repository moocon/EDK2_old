/** @file
  AsmWriteMm6 function

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
  Writes the current value of 64-bit MMX Register #6 (MM6).

  Writes the current value of MM6. This function is only available on IA32 and
  X64.

  @param  Value The 64-bit value to write to MM6.

**/
VOID
EFIAPI
AsmWriteMm6 (
  IN UINT64   Value
  )
{
  _asm {
    movq    mm6, qword ptr [Value]
    emms
  }
}
