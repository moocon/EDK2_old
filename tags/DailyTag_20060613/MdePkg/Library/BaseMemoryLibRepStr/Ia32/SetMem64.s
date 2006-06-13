#------------------------------------------------------------------------------
#
# Copyright (c) 2006, Intel Corporation
# All rights reserved. This program and the accompanying materials
# are licensed and made available under the terms and conditions of the BSD License
# which accompanies this distribution.  The full text of the license may be found at
# http://opensource.org/licenses/bsd-license.php
#
# THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
# WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.
#
# Module Name:
#
#   SetMem64.Asm
#
# Abstract:
#
#   SetMem64 function
#
# Notes:
#
#------------------------------------------------------------------------------

    .386: 
    .code: 

.global InternalMemSetMem64
InternalMemSetMem64:
    push    %edi
    movl    12(%esp),%ecx
    movl    16(%esp),%eax
    movl    20(%esp),%edx
    movl    8(%esp),%edi
L0: 
    mov     %eax,-8(%edi,%ecx,8)
    mov     %edx,-4(%edi,%ecx,8)
    loop    L0
    movl    %edi,%eax
    pop     %edi
    ret
