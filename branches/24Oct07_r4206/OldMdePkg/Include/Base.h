/** @file

  Root include file for Mde Package Base type modules

  This is the include file for any module of type base. Base modules only use 
  types defined via this include file and can be ported easily to any 
  environment. There are a set of base libraries in the Mde Package that can
  be used to implement base modules.

Copyright (c) 2006, Intel Corporation
All rights reserved. This program and the accompanying materials
are licensed and made available under the terms and conditions of the BSD License
which accompanies this distribution.  The full text of the license may be found at
http://opensource.org/licenses/bsd-license.php

THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/


#ifndef __BASE_H__
#define __BASE_H__

#include <Common/BaseTypes.h>
#include <Common/EfiImage.h>

#if defined(MDE_CPU_IPF)
#include <PalApi.h>
#endif

#endif
