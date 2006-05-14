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
#   MultU64x64.asm
#
# Abstract:
#
#   Calculate the product of a 64-bit integer and another 64-bit integer
#
#------------------------------------------------------------------------------



     

.global _MultS64x64
_MultS64x64: 
    #
    # MultS64x32 shares the same implementation with _MultU64x32, and thus no
    # code inside this function.
    #


.global _MultU64x64
    push    %ebx
    movl    8(%esp),%ebx
    movl    16(%esp),%edx
    movl    %ebx,%ecx
    movl    %edx,%eax
    imull   20(%esp),%ebx
    imull   12(%esp),%edx
    addl    %edx,%ebx
    mull    %ecx
    addl    %ebx,%edx
    pop     %ebx
    ret



