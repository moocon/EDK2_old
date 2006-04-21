/** @file
	Library that provides a global pointer to the UEFI Boot Services Tables

	Copyright (c) 2006, Intel Corporation                                                         
	All rights reserved. This program and the accompanying materials                          
	are licensed and made available under the terms and conditions of the BSD License         
	which accompanies this distribution.  The full text of the license may be found at        
	http://opensource.org/licenses/bsd-license.php                                            

	THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
	WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

	Module Name:	UefiBootServicesTableLib.h

**/

#ifndef __UEFI_BOOT_SERVICES_TABLE_LIB_H__
#define __UEFI_BOOT_SERVICES_TABLE_LIB_H__

//
//
//
extern EFI_HANDLE         gImageHandle;
extern EFI_SYSTEM_TABLE   *gST;
extern EFI_BOOT_SERVICES  *gBS;

#endif
