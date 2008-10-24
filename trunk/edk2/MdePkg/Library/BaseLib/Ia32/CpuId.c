/** @file
  AsmCpuid function.

  Copyright (c) 2006 - 2008, Intel Corporation<BR>
  All rights reserved. This program and the accompanying materials
  are licensed and made available under the terms and conditions of the BSD License
  which accompanies this distribution.  The full text of the license may be found at
  http://opensource.org/licenses/bsd-license.php

  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

/**
  Retrieves CPUID information.

  Executes the CPUID instruction with EAX set to the value specified by Index.
  This function always returns Index.
  If Eax is not NULL, then the value of EAX after CPUID is returned in Eax.
  If Ebx is not NULL, then the value of EBX after CPUID is returned in Ebx.
  If Ecx is not NULL, then the value of ECX after CPUID is returned in Ecx.
  If Edx is not NULL, then the value of EDX after CPUID is returned in Edx.
  This function is only available on IA-32 and X64.

  @param  Index         The 32-bit value to load into EAX prior to invoking the CPUID
                        instruction.
  @param  Eax           Pointer to the 32-bit EAX value returned by the CPUID
                        instruction. This is an optional parameter that may be NULL.
  @param  Ebx           Pointer to the 32-bit EBX value returned by the CPUID
                        instruction. This is an optional parameter that may be NULL.
  @param  Ecx           Pointer to the 32-bit ECX value returned by the CPUID
                        instruction. This is an optional parameter that may be NULL.
  @param  Edx           Pointer to the 32-bit EDX value returned by the CPUID
                        instruction. This is an optional parameter that may be NULL.

  @return Index

**/
UINT32
EFIAPI
AsmCpuid (
  IN      UINT32                    Index,
  OUT     UINT32                    *Eax,  OPTIONAL
  OUT     UINT32                    *Ebx,  OPTIONAL
  OUT     UINT32                    *Ecx,  OPTIONAL
  OUT     UINT32                    *Edx   OPTIONAL
  )
{
  _asm {
    mov     eax, Index
    cpuid
    push    ecx
    mov     ecx, Eax
    jecxz   SkipEax
    mov     [ecx], eax
SkipEax:
    mov     ecx, Ebx
    jecxz   SkipEbx
    mov     [ecx], ebx
SkipEbx:
    pop     eax
    mov     ecx, Ecx
    jecxz   SkipEcx
    mov     [ecx], eax
SkipEcx:
    mov     ecx, Edx
    jecxz   SkipEdx
    mov     [ecx], edx
SkipEdx:
    mov     eax, Index
  }
}

