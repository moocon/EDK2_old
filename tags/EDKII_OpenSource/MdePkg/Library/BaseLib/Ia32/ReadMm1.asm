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
;   ReadMm1.Asm
;
; Abstract:
;
;   AsmReadMm1 function
;
; Notes:
;
;------------------------------------------------------------------------------

    .586P
    .model  flat
    .xmm
    .code

;------------------------------------------------------------------------------
; UINTN
; EFIAPI
; AsmReadMm1 (
;   VOID
;   );
;------------------------------------------------------------------------------
_AsmReadMm1 PROC
    push    eax
    push    eax
    movq    [esp], mm1
    pop     eax
    pop     edx
    ret
_AsmReadMm1 ENDP

    END