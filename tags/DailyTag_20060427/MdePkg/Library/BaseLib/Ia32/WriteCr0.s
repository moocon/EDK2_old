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
#   WriteCr0.Asm
#
# Abstract:
#
#   AsmWriteCr0 function
#
# Notes:
#
#------------------------------------------------------------------------------



     

#------------------------------------------------------------------------------
# UINTN
# EFIAPI
# AsmWriteCr0 (
#   VOID
#   );
#------------------------------------------------------------------------------
.global _AsmWriteCr0
_AsmWriteCr0: 
    movl    4(%esp),%eax
    movl    %eax, %cr0
    ret



