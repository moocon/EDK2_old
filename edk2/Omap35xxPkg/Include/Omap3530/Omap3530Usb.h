/** @file

  Copyright (c) 2008-2009 Apple Inc. All rights reserved.<BR>

  All rights reserved. This program and the accompanying materials
  are licensed and made available under the terms and conditions of the BSD License
  which accompanies this distribution.  The full text of the license may be found at
  http://opensource.org/licenses/bsd-license.php

  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

#ifndef __OMAP3530USB_H__
#define __OMAP3530USB_H__

#define USB_BASE            (0x48060000)

#define UHH_SYSCONFIG       (USB_BASE + 0x4010)
#define UHH_HOSTCONFIG      (USB_BASE + 0x4040)

#define USB_EHCI_HCCAPBASE  (USB_BASE + 0x4800)

#define UHH_SYSCONFIG_MIDLEMODE_NO_STANDBY  (1UL << 12)
#define UHH_SYSCONFIG_CLOCKACTIVITY_ON      (1UL <<  8)
#define UHH_SYSCONFIG_SIDLEMODE_NO_STANDBY  (1UL <<  3)
#define UHH_SYSCONFIG_ENAWAKEUP_ENABLE      (1UL <<  2)
#define UHH_SYSCONFIG_SOFTRESET             (1UL <<  1)
#define UHH_SYSCONFIG_AUTOIDLE_ALWAYS_RUN   (0UL <<  0)

#define UHH_HOSTCONFIG_P3_CONNECT_STATUS_DISCONNECT (0UL << 10)
#define UHH_HOSTCONFIG_P2_CONNECT_STATUS_DISCONNECT (0UL <<  9)
#define UHH_HOSTCONFIG_P1_CONNECT_STATUS_DISCONNECT (0UL <<  8)
#define UHH_HOSTCONFIG_ENA_INCR_ALIGN_DISABLE       (0UL <<  5)
#define UHH_HOSTCONFIG_ENA_INCR16_ENABLE            (1UL <<  4)
#define UHH_HOSTCONFIG_ENA_INCR8_ENABLE             (1UL <<  3)
#define UHH_HOSTCONFIG_ENA_INCR4_ENABLE             (1UL <<  2)
#define UHH_HOSTCONFIG_AUTOPPD_ON_OVERCUR_EN_ON     (0UL <<  1)
#define UHH_HOSTCONFIG_P1_ULPI_BYPASS_ULPI_MODE     (0UL <<  0)

#endif // __OMAP3530USB_H__

