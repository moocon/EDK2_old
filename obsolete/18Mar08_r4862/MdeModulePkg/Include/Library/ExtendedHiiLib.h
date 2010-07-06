/** @file
  This library includes the APIs that is specific to EDK II's implementation.
  It extended the UEFI Specification to define GUIDed device path.

  Copyright (c) 2008, Intel Corporation                                                         
  All rights reserved. This program and the accompanying materials                          
  are licensed and made available under the terms and conditions of the BSD License         
  which accompanies this distribution.  The full text of the license may be found at        
  http://opensource.org/licenses/bsd-license.php                                            

  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

**/

#ifndef __EXTENDED_HII_LIB_H__
#define __EXTENDED_HII_LIB_H__

/**
  The HII driver handle passed in for HiiDatabase.NewPackageList() requires
  that there should be DevicePath Protocol installed on it.
  This routine create a virtual Driver Handle by installing a vendor device
  path on it, so as to use it to invoke HiiDatabase.NewPackageList().
  The Device Path created is a Vendor Device Path specific to Intel's implemenation.

  @param  DriverHandle           Handle to be returned

  @retval EFI_SUCCESS            Handle destroy success.
  @retval EFI_OUT_OF_RESOURCES   Not enough memory.

**/
EFI_STATUS
EFIAPI
HiiLibCreateHiiDriverHandle (
  OUT EFI_HANDLE               *DriverHandle
  )
;

/**
  Destroy the Driver Handle created by CreateHiiDriverHandle().

  If no Device Path protocol is installed on the DriverHandle, then ASSERT.
  If this Device Path protocol is failed to be uninstalled, then ASSERT.

  @param  DriverHandle           Handle returned by CreateHiiDriverHandle()


**/
VOID
EFIAPI
HiiLibDestroyHiiDriverHandle (
  IN EFI_HANDLE                 DriverHandle
  )
;


#endif
