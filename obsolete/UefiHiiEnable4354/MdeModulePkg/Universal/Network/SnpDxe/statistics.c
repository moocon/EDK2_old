/** @file
Copyright (c) 2004 - 2007, Intel Corporation
All rights reserved. This program and the accompanying materials
are licensed and made available under the terms and conditions of the BSD License
which accompanies this distribution.  The full text of the license may be found at
http://opensource.org/licenses/bsd-license.php

THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

Module name:
  statistics.c

Abstract:

Revision history:
  2000-Feb-17 M(f)J   Genesis.

**/


#include "Snp.h"


/**
  This is the SNP interface routine for getting the NIC's statistics.
  This routine basically retrieves snp structure, checks the SNP state and
  calls the pxe_ routine to actually do the

  @param  this              context pointer
  @param  ResetFlag         true to reset the NIC's statistics counters to zero.
  @param  StatTableSizePtr  pointer to the statistics table size
  @param  StatTablePtr      pointer to the statistics table


**/
EFI_STATUS
EFIAPI
snp_undi32_statistics (
  IN EFI_SIMPLE_NETWORK_PROTOCOL * this,
  IN BOOLEAN                     ResetFlag,
  IN OUT UINTN                   *StatTableSizePtr OPTIONAL,
  IN OUT EFI_NETWORK_STATISTICS  * StatTablePtr OPTIONAL
  )
{
  SNP_DRIVER        *snp;
  PXE_DB_STATISTICS *db;
  UINT64            *stp;
  UINT64            mask;
  UINTN             size;
  UINTN             n;
  EFI_TPL           OldTpl;
  EFI_STATUS        Status;

  //
  // Get pointer to SNP driver instance for *this.
  //
  if (this == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  snp = EFI_SIMPLE_NETWORK_DEV_FROM_THIS (this);

  OldTpl = gBS->RaiseTPL (TPL_CALLBACK);

  //
  // Return error if the SNP is not initialized.
  //
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
  //
  // if we are not resetting the counters, we have to have a valid stat table
  // with >0 size. if no reset, no table and no size, return success.
  //
  if (!ResetFlag && StatTableSizePtr == NULL) {
    Status = StatTablePtr ? EFI_INVALID_PARAMETER : EFI_SUCCESS;
    goto ON_EXIT;
  }
  //
  // Initialize UNDI Statistics CDB
  //
  snp->cdb.OpCode     = PXE_OPCODE_STATISTICS;
  snp->cdb.CPBsize    = PXE_CPBSIZE_NOT_USED;
  snp->cdb.CPBaddr    = PXE_CPBADDR_NOT_USED;
  snp->cdb.StatCode   = PXE_STATCODE_INITIALIZE;
  snp->cdb.StatFlags  = PXE_STATFLAGS_INITIALIZE;
  snp->cdb.IFnum      = snp->if_num;
  snp->cdb.Control    = PXE_CONTROL_LAST_CDB_IN_LIST;

  if (ResetFlag) {
    snp->cdb.OpFlags  = PXE_OPFLAGS_STATISTICS_RESET;
    snp->cdb.DBsize   = PXE_DBSIZE_NOT_USED;
    snp->cdb.DBaddr   = PXE_DBADDR_NOT_USED;
    db                = snp->db;
  } else {
    snp->cdb.OpFlags                = PXE_OPFLAGS_STATISTICS_READ;
    snp->cdb.DBsize                 = sizeof (PXE_DB_STATISTICS);
    snp->cdb.DBaddr                 = (UINT64)(UINTN) (db = snp->db);
  }
  //
  // Issue UNDI command and check result.
  //
  DEBUG ((EFI_D_NET, "\nsnp->undi.statistics()  "));

  (*snp->issue_undi32_command) ((UINT64)(UINTN) &snp->cdb);

  switch (snp->cdb.StatCode) {
  case PXE_STATCODE_SUCCESS:
    break;

  case PXE_STATCODE_UNSUPPORTED:
    DEBUG (
      (EFI_D_ERROR,
      "\nsnp->undi.statistics()  %xh:%xh\n",
      snp->cdb.StatFlags,
      snp->cdb.StatCode)
      );

    Status = EFI_UNSUPPORTED;
    goto ON_EXIT;

  default:
    DEBUG (
      (EFI_D_ERROR,
      "\nsnp->undi.statistics()  %xh:%xh\n",
      snp->cdb.StatFlags,
      snp->cdb.StatCode)
      );

    Status = EFI_DEVICE_ERROR;
    goto ON_EXIT;
  }

  if (ResetFlag) {
    Status = EFI_SUCCESS;
    goto ON_EXIT;
  }

  if (StatTablePtr == NULL) {
    *StatTableSizePtr = sizeof (EFI_NETWORK_STATISTICS);
    Status = EFI_BUFFER_TOO_SMALL;
    goto ON_EXIT;
  }
  //
  // Convert the UNDI statistics information to SNP statistics
  // information.
  //
  ZeroMem (StatTablePtr, *StatTableSizePtr);
  stp   = (UINT64 *) StatTablePtr;
  size  = 0;

  for (n = 0, mask = 1; n < 64; n++, mask = LShiftU64 (mask, 1), stp++) {
    //
    // There must be room for a full UINT64.  Partial
    // numbers will not be stored.
    //
    if ((n + 1) * sizeof (UINT64) > *StatTableSizePtr) {
      break;
    }

    if (db->Supported & mask) {
      *stp  = db->Data[n];
      size  = n + 1;
    } else {
      SetMem (stp, sizeof (UINT64), 0xFF);
    }
  }
  //
  // Compute size up to last supported statistic.
  //
  while (++n < 64) {
    if (db->Supported & (mask = LShiftU64 (mask, 1))) {
      size = n;
    }
  }

  size *= sizeof (UINT64);

  if (*StatTableSizePtr >= size) {
    *StatTableSizePtr = size;
    Status = EFI_SUCCESS;
  } else {
    *StatTableSizePtr = size;
    Status = EFI_BUFFER_TOO_SMALL;
  }

ON_EXIT:
  gBS->RestoreTPL (OldTpl);

  return Status;
}
