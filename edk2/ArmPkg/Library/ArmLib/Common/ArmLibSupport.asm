//------------------------------------------------------------------------------ 
//
// Copyright (c) 2008 - 2009, Apple Inc. All rights reserved.<BR>
//
// This program and the accompanying materials
// are licensed and made available under the terms and conditions of the BSD License
// which accompanies this distribution.  The full text of the license may be found at
// http://opensource.org/licenses/bsd-license.php
//
// THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
// WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.
//
//------------------------------------------------------------------------------

#include <AsmMacroIoLib.h>
    
    INCLUDE AsmMacroIoLib.inc

    EXPORT  Cp15IdCode
    EXPORT  Cp15CacheInfo
    EXPORT  ArmIsMPCore
    EXPORT  ArmEnableInterrupts
    EXPORT  ArmDisableInterrupts
    EXPORT  ArmGetInterruptState
    EXPORT  ArmEnableFiq
    EXPORT  ArmDisableFiq
    EXPORT  ArmGetFiqState
    EXPORT  ArmInvalidateTlb
    EXPORT  ArmSetTTBR0
    EXPORT  ArmGetTTBR0BaseAddress
    EXPORT  ArmSetDomainAccessControl
    EXPORT  CPSRMaskInsert
    EXPORT  CPSRRead

    AREA ArmLibSupport, CODE, READONLY

Cp15IdCode
  mrc     p15,0,R0,c0,c0,0
  bx      LR

Cp15CacheInfo
  mrc     p15,0,R0,c0,c0,1
  bx      LR

ArmIsMPCore
  mrc     p15,0,R0,c0,c0,5
  // Get Multiprocessing extension (bit31) & U bit (bit30)
  and     R0, R0, #0xC0000000
  // if bit30 == 0 then the processor is part of a multiprocessor system)
  and     R0, R0, #0x80000000
  bx      LR

ArmEnableInterrupts
  mrs     R0,CPSR
  bic     R0,R0,#0x80    ;Enable IRQ interrupts
  msr     CPSR_c,R0
  bx      LR

ArmDisableInterrupts
  mrs     R0,CPSR
  orr     R1,R0,#0x80    ;Disable IRQ interrupts
  msr     CPSR_c,R1
  tst     R0,#0x80
  moveq   R0,#1
  movne   R0,#0
  bx      LR

ArmGetInterruptState
  mrs     R0,CPSR
  tst     R0,#0x80      ;Check if IRQ is enabled.
  moveq   R0,#1
  movne   R0,#0
  bx      LR

ArmEnableFiq
  mrs     R0,CPSR
  bic     R0,R0,#0x40    ;Enable IRQ interrupts
  msr     CPSR_c,R0
  bx      LR

ArmDisableFiq
  mrs     R0,CPSR
  orr     R1,R0,#0x40    ;Disable IRQ interrupts
  msr     CPSR_c,R1
  tst     R0,#0x40
  moveq   R0,#1
  movne   R0,#0
  bx      LR

ArmGetFiqState
  mrs     R0,CPSR
  tst     R0,#0x40      ;Check if IRQ is enabled.
  moveq   R0,#1
  movne   R0,#0
  bx      LR
  
ArmInvalidateTlb
  mov     r0,#0
  mcr     p15,0,r0,c8,c7,0
  bx      lr

ArmSetTTBR0
  mcr     p15,0,r0,c2,c0,0
  bx      lr

ArmGetTTBR0BaseAddress
  mrc     p15,0,r0,c2,c0,0
  LoadConstantToReg(0xFFFFC000,r1)  // and     r0, r0, #0xFFFFC000
  and     r0, r0, r1
  bx      lr

ArmSetDomainAccessControl
  mcr     p15,0,r0,c3,c0,0
  bx      lr

CPSRMaskInsert              ; on entry, r0 is the mask and r1 is the field to insert
  stmfd   sp!, {r4-r12, lr} ; save all the banked registers
  mov     r3, sp            ; copy the stack pointer into a non-banked register
  mrs     r2, cpsr          ; read the cpsr
  bic     r2, r2, r0        ; clear mask in the cpsr
  and     r1, r1, r0        ; clear bits outside the mask in the input
  orr     r2, r2, r1        ; set field
  msr     cpsr_cxsf, r2     ; write back cpsr (may have caused a mode switch)
  mov     sp, r3            ; restore stack pointer
  ldmfd   sp!, {r4-r12, lr} ; restore registers
  bx      lr                ; return (hopefully thumb-safe!)

CPSRRead
  mrs     r0, cpsr
  bx      lr
  
  END


