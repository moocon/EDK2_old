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
#   WriteMm3.Asm
#
# Abstract:
#
#   AsmWriteMm3 function
#
# Notes:
#
#------------------------------------------------------------------------------



     
     

#------------------------------------------------------------------------------
# UINT64
# EFIAPI
# AsmWriteMm3 (
#   IN UINT64   Value
#   );
#------------------------------------------------------------------------------
.global _AsmWriteMm3
_AsmWriteMm3: 
    movq    4(%esp),%mm3
    ret



