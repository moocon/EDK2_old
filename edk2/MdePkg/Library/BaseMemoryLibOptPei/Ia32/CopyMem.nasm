;------------------------------------------------------------------------------
;
; Copyright (c) 2006, Intel Corporation. All rights reserved.<BR>
; SPDX-License-Identifier: BSD-2-Clause-Patent
;
; Module Name:
;
;   CopyMem.Asm
;
; Abstract:
;
;   CopyMem function
;
; Notes:
;
;------------------------------------------------------------------------------

    SECTION .text

;------------------------------------------------------------------------------
;  VOID *
;  InternalMemCopyMem (
;    IN VOID   *Destination,
;    IN VOID   *Source,
;    IN UINTN  Count
;    )
;------------------------------------------------------------------------------
global ASM_PFX(InternalMemCopyMem)
ASM_PFX(InternalMemCopyMem):
    push    esi
    push    edi
    mov     esi, [esp+16]                ; esi <- Source
    mov     edi, [esp+12]                ; edi <- Destination
    mov     edx, [esp+20]                ; edx <- Count
    cmp     esi, edi
    je      @CopyDone
    cmp     edx, 0
    je      @CopyDone
    lea     eax, [esi + edx - 1]         ; eax <- End of Source
    cmp     esi, edi
    jae     @CopyBytes
    cmp     eax, edi
    jb      @CopyBytes                   ; Copy backward if overlapped
    mov     esi, eax                     ; esi <- End of Source
    lea     edi, [edi + edx - 1]         ; edi <- End of Destination
    std
@CopyBytes:
    mov     ecx, edx
    rep     movsb                        ; Copy bytes backward
    cld
@CopyDone:
    mov     eax, [esp + 12]             ; eax <- Destination as return value
    pop     edi
    pop     esi
    ret

