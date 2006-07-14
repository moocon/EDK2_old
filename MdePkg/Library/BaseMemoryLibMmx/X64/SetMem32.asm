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
;   SetMem32.asm
;
; Abstract:
;
;   SetMem32 function
;
; Notes:
;
;------------------------------------------------------------------------------

    .code

InternalMemSetMem32 PROC    USES    rdi
    DB      49h, 0fh, 6eh, 0c0h        ;  movq    mm0, r8       ; mm0 <- Value
    mov     rax, rcx                                            ; rax <- Buffer
    mov     rdi, rax
    mov     rcx, rdx
    shr     rcx, 1                                      
    jz      @SetDwords
    DB      0fh, 70h, 0C0h, 44h        ;  pshufw  mm0, mm0, 44h
@@:
    DB      48h, 0fh, 0e7h, 07h        ;  movntq  [rdi], mm0
    add     rdi, 8
    loop    @B
    mfence
@SetDwords:
    test    dl, 1
    jz      @F
    DB      0fh, 7eh, 07h              ;  movd    [rdi], mm0
@@:
    ret
InternalMemSetMem32 ENDP

    END
