/** @file
  AsmWriteMm2 function

  Copyright (c) 2006, Intel Corporation<BR>
  All rights reserved. This program and the accompanying materials
  are licensed and made available under the terms and conditions of the BSD License
  which accompanies this distribution.  The full text of the license may be found at
  http://opensource.org/licenses/bsd-license.php

  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

#if _MSC_EXTENSIONS

VOID
EFIAPI
AsmWriteMm2 (
  IN UINT64   Value
  )
{
  _asm {
    movq    mm2, qword ptr [Value]
    emms
  }
}

#endif
