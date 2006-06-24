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
#   WriteMsr32.Asm
#
# Abstract:
#
#   AsmWriteMsr32 function
#
# Notes:
#
#------------------------------------------------------------------------------



     

#------------------------------------------------------------------------------
# UINT32
# EFIAPI
# AsmWriteMsr32 (
#   IN UINT32  Index,
#   IN UINT32  Value
#   );
#------------------------------------------------------------------------------
.global _AsmWriteMsr32
_AsmWriteMsr32: 
    movl    8(%esp),%eax
    xorl    %edx,%edx
    movl    4(%esp),%ecx
    wrmsr
    ret



