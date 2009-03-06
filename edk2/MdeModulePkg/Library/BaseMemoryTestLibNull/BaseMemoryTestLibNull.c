/** @file
  Null instance of Memory Test Library.
  
Copyright (c) 2009, Intel Corporation.<BR>
All rights reserved. This program and the accompanying materials
are licensed and made available under the terms and conditions of the BSD License
which accompanies this distribution.  The full text of the license may be found at
http://opensource.org/licenses/bsd-license.php

THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.
**/

#include <Base.h>
#include <Library/BaseMemoryTestLib.h>

/**
  Perform a quick system memory range test.

  This function performs a quick system memory range test. It leads to quick performance
  but least reliability.

  @param  StartAddress           Start address of the memory range to test.
  @param  Length                 Length of the memory range to test.
  @param  ErrorAddress           Address of the memory where error is encountered.

  @retval RETURN_SUCCESS         The memory range passes test.
  @retval RETURN_DEVICE_ERROR    The memory range does not pass test.

**/
RETURN_STATUS
EFIAPI
QuickMemoryTest (
  IN  VOID      *StartAddress,
  IN  UINT64    Length,
  OUT VOID      **ErrorAddress
  )
{
  return RETURN_SUCCESS;
}

/**
  Test a system memory range with sparsely sampled memory units.

  This function tests a system memory range, whose memory units
  are sampled sparsely. It leads to relatively good performance
  and partial reliability.

  @param  StartAddress           Start address of the memory range to test.
  @param  Length                 Length of the memory range to test.
  @param  ErrorAddress           Address of the memory where error is encountered.

  @retval RETURN_SUCCESS         The memory range passes test.
  @retval RETURN_DEVICE_ERROR    The memory range does not pass test.

**/
RETURN_STATUS
EFIAPI
SparseMemoryTest (
  IN  VOID      *StartAddress,
  IN  UINT64    Length,
  OUT VOID      **ErrorAddress
  )
{
  return RETURN_SUCCESS;
}

/**
  Test a system memory range with extensively sampled memory units.

  This function tests a system memory range, whose memory units
  are sampled extensively. Compared with SparseMemoryTest, it achieves
  more reliability and less performance.

  @param  StartAddress           Start address of the memory range to test.
  @param  Length                 Length of the memory range to test.
  @param  ErrorAddress           Address of the memory where error is encountered.

  @retval RETURN_SUCCESS         The memory range passes test.
  @retval RETURN_DEVICE_ERROR    The memory range does not pass test.

**/
RETURN_STATUS
EFIAPI
ExtensiveMemoryTest (
  IN  VOID      *StartAddress,
  IN  UINT64    Length,
  OUT VOID      **ErrorAddress
  )
{
  return RETURN_SUCCESS;
}

/**
  Check if soft ECC initialzation is needed for system

  @retval TRUE         Soft ECC initialzation is needed.
  @retval FALSE        Soft ECC initialzation is not needed.

**/
BOOLEAN
EFIAPI
IsSoftEccInitRequired (
  VOID
  )
{
  return FALSE;
}
