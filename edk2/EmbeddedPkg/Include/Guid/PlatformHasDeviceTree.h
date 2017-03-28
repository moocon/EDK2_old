/** @file
  EDKII Platform Has Device Tree GUID

  A NULL protocol instance with this GUID in the DXE protocol database, and/or
  a NULL PPI with this GUID in the PPI database, implies that the platform
  provides the operating system with a Device Tree-based hardware description.
  Note that this is not necessarily exclusive with different kinds of hardware
  description (for example, an ACPI-based one). A platform driver and/or PEIM
  is supposed to produce a single instance of the protocol and/or PPI (with
  NULL contents), if appropriate.

  Copyright (C) 2017, Red Hat, Inc.

  This program and the accompanying materials are licensed and made available
  under the terms and conditions of the BSD License that accompanies this
  distribution. The full text of the license may be found at
  http://opensource.org/licenses/bsd-license.php.

  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS, WITHOUT
  WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.
**/


#ifndef __EDKII_PLATFORM_HAS_DEVICE_TREE_H__
#define __EDKII_PLATFORM_HAS_DEVICE_TREE_H__

#define EDKII_PLATFORM_HAS_DEVICE_TREE_GUID \
  { \
    0x7ebb920d, 0x1aaf, 0x46d9, \
    { 0xb2, 0xaf, 0x54, 0x1e, 0x1d, 0xce, 0x14, 0x8b } \
  }

extern EFI_GUID gEdkiiPlatformHasDeviceTreeGuid;

#endif
