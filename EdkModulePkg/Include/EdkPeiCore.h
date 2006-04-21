/*++

Copyright (c) 2006, Intel Corporation                                                         
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution.  The full text of the license may be found at        
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module Name:
  EdkDxe.h

Abstract:
  This file defines the base package surface area for writting a PEIM
  
  Things defined in the Tiano specification go in DxeCis.h. 

  Dxe.h contains build environment and library information needed to build
  a basic Dxe driver. This file must match the "base package" definition of
  how to write a Dxe driver.

--*/

#ifndef __EDK_PEI_CORE_H__
#define __EDK_PEI_CORE_H__

//
// BUGBUG: We must include this lib here due to ordering issues
//
#include <Library/PeCoffLib.h>

#include <Guid/PeiPeCoffLoader.h>

//
// BUGBUG: Performance related Guid.
// It is Tiano-private, but is required for PeiCore
//
#include <Guid/PeiPerformanceHob.h>

#endif
