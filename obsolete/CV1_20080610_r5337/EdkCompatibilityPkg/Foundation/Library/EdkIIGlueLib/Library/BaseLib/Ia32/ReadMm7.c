/*++

Copyright (c) 2004 - 2006, Intel Corporation                                                         
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution.  The full text of the license may be found at        
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.  


Module Name:

  ReadMm7.c
  
Abstract: 



--*/

#include "BaseLibInternals.h"

UINT64
EFIAPI
AsmReadMm7 (
  VOID
  )
{
  _asm {
    push    eax
    push    eax
    movq    [esp], mm7
    pop     eax
    pop     edx
    emms
  }
}
