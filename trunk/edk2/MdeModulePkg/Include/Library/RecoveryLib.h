/** @file
  Recovery library class defines a set of methods related recovery boot mode.

Copyright (c) 2005 - 2008, Intel Corporation. <BR>
All rights reserved. This program and the accompanying materials
are licensed and made available under the terms and conditions of the BSD License
which accompanies this distribution.  The full text of the license may be found at
http://opensource.org/licenses/bsd-license.php

THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

#ifndef __RECOVERY_LIB_H__
#define __RECOVERY_LIB_H__

/**
  Calling this function causes the system to carry out a recovery boot path.
  
  @retval EFI_SUCESS   Sucess.
  @retval Others       Failure.

**/
EFI_STATUS
EFIAPI
PeiRecoverFirmware (
  VOID
  );

#endif


