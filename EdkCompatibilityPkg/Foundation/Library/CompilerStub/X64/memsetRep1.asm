;------------------------------------------------------------------------------
;
; Copyright (c) 2007, Intel Corporation
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
;   memsetRep1.asm
;
; Abstract:
;
;   SetMem function
;
; Notes:
;
;------------------------------------------------------------------------------

    .code

;------------------------------------------------------------------------------
; VOID *
; memset (
;   OUT     VOID                      *Buffer,
;   IN      UINTN                     Size,
;   IN      UINT8                     Value
;   );
;------------------------------------------------------------------------------
memset   PROC    USES    rdi
    cmp     rdx, 0                      ; if Size == 0, do nothing
    mov     r9,  rcx
    je      @SetDone
    mov     rax, r8
    mov     rdi, rcx
    mov     rcx, rdx
    rep     stosb
@SetDone:
    mov     rax, r9
    ret
memset   ENDP

    END

