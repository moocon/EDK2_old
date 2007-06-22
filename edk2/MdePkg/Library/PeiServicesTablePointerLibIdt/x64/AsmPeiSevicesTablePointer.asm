;------------------------------------------------------------------------------
;
; Copyright (c) 2006, Intel Corporation
; All rights reserved. This program and the accompanying materials
; are licensed and made available under the terms and conditions of the BSD License
; which accompanies this distribution.  The full text of the license may be found at
; http://opensource.org/licenses/bsd-license.php
;
; THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
; WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.
;
; Module Name:
;
;   ReadIdtr.Asm
;
; Abstract:
;
;   AsmPeiSevicesTablePointer function
;
; Notes:
;
;------------------------------------------------------------------------------

    .code

;------------------------------------------------------------------------------
; EFI_PEI_SERVICES **
; EFIAPI
; AsmPeiSevicesTablePointer (
;   );
;------------------------------------------------------------------------------
AsmPeiSevicesTablePointer   PROC
    sidt    fword ptr [rcx]
    mov     rcx, [rcx - 8]
    ret
AsmPeiSevicesTablePointer   ENDP

    END
