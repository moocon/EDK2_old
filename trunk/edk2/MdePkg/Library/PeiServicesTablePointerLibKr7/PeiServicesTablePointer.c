/** @file
  PEI Services Table Pointer Library implementation for IPF that uses Kernel
  Register 7 to store the pointer.

  Copyright (c) 2006 - 2008, Intel Corporation.<BR>
  All rights reserved. This program and the accompanying materials
  are licensed and made available under the terms and conditions of the BSD License
  which accompanies this distribution.  The full text of the license may be found at
  http://opensource.org/licenses/bsd-license.php

  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

#include <PiPei.h>
#include <Library/BaseLib.h>
#include <Library/DebugLib.h>

/**
  The function returns the pointer to PeiServices.

  The function returns the pointer to PeiServices.
  It will ASSERT() if the pointer to PeiServices is NULL.

  @return  The pointer to PeiServices.

**/
CONST EFI_PEI_SERVICES **
EFIAPI
GetPeiServicesTablePointer (
  VOID
  )
{
  CONST EFI_PEI_SERVICES  **PeiServices;

  PeiServices = (CONST EFI_PEI_SERVICES **)(UINTN)AsmReadKr7 ();
  ASSERT (PeiServices != NULL);
  return PeiServices;
}

/**
  The constructor function caches the pointer to PEI services.

  The constructor function caches the pointer to PEI services.
  It will always return EFI_SUCCESS.

  @param  FfsHeader   Pointer to FFS header the loaded driver.
  @param  PeiServices Pointer to the PEI services.

  @retval EFI_SUCCESS   The constructor always returns EFI_SUCCESS.

**/
EFI_STATUS
EFIAPI
PeiServicesTablePointerLibConstructor (
  IN EFI_PEI_FILE_HANDLE  *FfsHeader,
  IN EFI_PEI_SERVICES     **PeiServices
  )
{
  AsmWriteKr7 ((UINT64)(UINTN)PeiServices);
  return EFI_SUCCESS;
}

/**
  The function set the pointer of PEI services in KR7 register 
  according to PI specification.
  
  @param    PeiServicesTablePointer   The address of PeiServices pointer.
**/
VOID
EFIAPI
SetPeiServicesTablePointer (
  IN EFI_PEI_SERVICES ** PeiServicesTablePointer
  )
{
  AsmWriteKr7 ((UINT64)(UINTN)PeiServicesTablePointer);
}
  



