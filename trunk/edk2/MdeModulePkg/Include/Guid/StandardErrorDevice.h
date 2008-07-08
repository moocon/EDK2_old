/** @file
  This guid is used to specifiy the device is the StdErr device.
  If the device is the StdErr device, this guid as the protocol guid
  will be installed into this device handle.

Copyright (c) 2006 - 2008, Intel Corporation
All rights reserved. This program and the accompanying materials
are licensed and made available under the terms and conditions of the BSD License
which accompanies this distribution.  The full text of the license may be found at
http://opensource.org/licenses/bsd-license.php

THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

#ifndef __STANDARD_ERROR_DEVICE_H__
#define __STANDARD_ERROR_DEVICE_H__

#define EFI_STANDARD_ERROR_DEVICE_GUID    \
    { 0xd3b36f2d, 0xd551, 0x11d4, {0x9a, 0x46, 0x0, 0x90, 0x27, 0x3f, 0xc1, 0x4d } }

extern EFI_GUID gEfiStandardErrorDeviceGuid;

#endif
