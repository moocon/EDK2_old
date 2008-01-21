/*++

Copyright (c) 2007, Intel Corporation
All rights reserved. This program and the accompanying materials
are licensed and made available under the terms and conditions of the BSD License
which accompanies this distribution.  The full text of the license may be found at
http://opensource.org/licenses/bsd-license.php

THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

Module Name:

  HwErrRecSupport.h

Abstract:

  Set the level of support for Hardware Error Record Persistence that is
  implemented by the platform.

Revision History

--*/

#ifndef _HW_ERR_REC_SUPPORT_H
#define _HW_ERR_REC_SUPPORT_H

#include "Bds.h"

VOID
InitializeHwErrRecSupport (
  IN UINT16       HwErrRecSupportLevel
  )
/*++

  Routine Description:
    Set the HwErrRecSupport variable contains a binary UINT16 that supplies the
    level of support for Hardware Error Record Persistence that is implemented
    by the platform.

  Arguments:
    HwErrRecSupportLevel
      zero value      - Indicates that the platform implements no support for
                        Hardware Error Record Persistence.
      non-zero value  - Indicates that the platform implements Hardware Error
                        Record Persistence.

  Returns:

--*/
;

#endif
