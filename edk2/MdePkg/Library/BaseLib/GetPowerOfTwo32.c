/** @file
  Math worker functions.

  Copyright (c) 2006, Intel Corporation<BR>
  All rights reserved. This program and the accompanying materials
  are licensed and made available under the terms and conditions of the BSD License
  which accompanies this distribution.  The full text of the license may be found at
  http://opensource.org/licenses/bsd-license.php

  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

#include "BaseLibInternals.h"

/**
  Returns the value of the highest bit set in a 32-bit value. Equivalent to
  1 << HighBitSet32(x).

  This function computes the value of the highest bit set in the 32-bit value
  specified by Operand. If Operand is zero, then zero is returned.

  @param  Operand The 32-bit operand to evaluate.

  @return 1 << HighBitSet32(Operand)
  @retval 0 Operand is zero.

**/
UINT32
EFIAPI
GetPowerOfTwo32 (
  IN      UINT32                    Operand
  )
{
  INTN                              BitPos;
  
  BitPos = HighBitSet32 (Operand);
  return BitPos >= 0 ? 1ul << BitPos : 0;
}
