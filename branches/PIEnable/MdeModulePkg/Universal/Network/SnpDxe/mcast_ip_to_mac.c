/** @file
Copyright (c) 2004 - 2007, Intel Corporation
All rights reserved. This program and the accompanying materials
are licensed and made available under the terms and conditions of the BSD License
which accompanies this distribution.  The full text of the license may be found at
http://opensource.org/licenses/bsd-license.php

THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

Module name:
  mcast_ip_to_mac.c

Abstract:

Revision history:
  2000-Feb-17 M(f)J   Genesis.

**/

#include "Snp.h"

/**
  this routine calls undi to convert an multicast IP address to a MAC address

  @param  snp   pointer to snp driver structure
  @param  IPv6  flag to indicate if this is an ipv6 address
  @param  IP    multicast IP address
  @param  MAC   pointer to hold the return MAC address


**/
STATIC
EFI_STATUS
pxe_ip2mac (
  IN SNP_DRIVER          *snp,
  IN BOOLEAN             IPv6,
  IN EFI_IP_ADDRESS      *IP,
  IN OUT EFI_MAC_ADDRESS *MAC
  )
{
  PXE_CPB_MCAST_IP_TO_MAC *cpb;
  PXE_DB_MCAST_IP_TO_MAC  *db;

  cpb                 = snp->cpb;
  db                  = snp->db;
  snp->cdb.OpCode     = PXE_OPCODE_MCAST_IP_TO_MAC;
  snp->cdb.OpFlags    = (UINT16) (IPv6 ? PXE_OPFLAGS_MCAST_IPV6_TO_MAC : PXE_OPFLAGS_MCAST_IPV4_TO_MAC);
  snp->cdb.CPBsize    = sizeof (PXE_CPB_MCAST_IP_TO_MAC);
  snp->cdb.DBsize     = sizeof (PXE_DB_MCAST_IP_TO_MAC);

  snp->cdb.CPBaddr    = (UINT64)(UINTN) cpb;
  snp->cdb.DBaddr     = (UINT64)(UINTN) db;

  snp->cdb.StatCode   = PXE_STATCODE_INITIALIZE;
  snp->cdb.StatFlags  = PXE_STATFLAGS_INITIALIZE;
  snp->cdb.IFnum      = snp->if_num;
  snp->cdb.Control    = PXE_CONTROL_LAST_CDB_IN_LIST;

  CopyMem (&cpb->IP, IP, sizeof (PXE_IP_ADDR));

  //
  // Issue UNDI command and check result.
  //
  DEBUG ((EFI_D_NET, "\nsnp->undi.mcast_ip_to_mac()  "));

  (*snp->issue_undi32_command) ((UINT64)(UINTN) &snp->cdb);

  switch (snp->cdb.StatCode) {
  case PXE_STATCODE_SUCCESS:
    break;

  case PXE_STATCODE_INVALID_CPB:
    return EFI_INVALID_PARAMETER;

  case PXE_STATCODE_UNSUPPORTED:
    DEBUG (
      (EFI_D_NET,
      "\nsnp->undi.mcast_ip_to_mac()  %xh:%xh\n",
      snp->cdb.StatFlags,
      snp->cdb.StatCode)
      );
    return EFI_UNSUPPORTED;

  default:
    //
    // UNDI command failed.  Return EFI_DEVICE_ERROR
    // to caller.
    //
    DEBUG (
      (EFI_D_NET,
      "\nsnp->undi.mcast_ip_to_mac()  %xh:%xh\n",
      snp->cdb.StatFlags,
      snp->cdb.StatCode)
      );

    return EFI_DEVICE_ERROR;
  }

  CopyMem (MAC, &db->MAC, sizeof (PXE_MAC_ADDR));
  return EFI_SUCCESS;
}


/**
  This is the SNP interface routine for converting a multicast IP address to
  a MAC address.
  This routine basically retrieves snp structure, checks the SNP state and
  calls the pxe_ip2mac routine to actually do the conversion

  @param  this  context pointer
  @param  IPv6  flag to indicate if this is an ipv6 address
  @param  IP    multicast IP address
  @param  MAC   pointer to hold the return MAC address


**/
EFI_STATUS
EFIAPI
snp_undi32_mcast_ip_to_mac (
  IN EFI_SIMPLE_NETWORK_PROTOCOL *this,
  IN BOOLEAN                     IPv6,
  IN EFI_IP_ADDRESS              *IP,
  OUT EFI_MAC_ADDRESS            *MAC
  )
{
  SNP_DRIVER  *snp;
  EFI_TPL     OldTpl;
  EFI_STATUS  Status;

  //
  // Get pointer to SNP driver instance for *this.
  //
  if (this == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  if (IP == NULL || MAC == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  snp = EFI_SIMPLE_NETWORK_DEV_FROM_THIS (this);

  OldTpl = gBS->RaiseTPL (TPL_CALLBACK);

  switch (snp->mode.State) {
  case EfiSimpleNetworkInitialized:
    break;

  case EfiSimpleNetworkStopped:
    Status = EFI_NOT_STARTED;
    goto ON_EXIT;

  default:
    Status = EFI_DEVICE_ERROR;
    goto ON_EXIT;
  }

  Status = pxe_ip2mac (snp, IPv6, IP, MAC);

ON_EXIT:
  gBS->RestoreTPL (OldTpl);

  return Status;
}
