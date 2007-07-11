/** @file

Copyright (c) 2007, Intel Corporation
All rights reserved. This program and the accompanying materials
are licensed and made available under the terms and conditions of the BSD License
which accompanies this distribution.  The full text of the license may be found at
http://opensource.org/licenses/bsd-license.php

THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

Module Name:

    EhciSched.c

Abstract:

    EHCI transfer scheduling routines

Revision History

**/

#include "Ehci.h"


/**
  Create helper QTD/QH for the EHCI device

  @param  Ehc                   The EHCI device

  @retval EFI_OUT_OF_RESOURCES  Failed to allocate resource for helper QTD/QH
  @retval EFI_SUCCESS           Helper QH/QTD are created

**/
STATIC
EFI_STATUS
EhcCreateHelpQ (
  IN USB2_HC_DEV          *Ehc
  )
{
  USB_ENDPOINT            Ep;
  EHC_QH                  *Qh;
  QH_HW                   *QhHw;
  EHC_QTD                 *Qtd;

  //
  // Create an inactive Qtd to terminate the short packet read.
  //
  Qtd = EhcCreateQtd (Ehc, NULL, 0, QTD_PID_INPUT, 0, 64);

  if (Qtd == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }

  Qtd->QtdHw.Status   = QTD_STAT_HALTED;
  Ehc->ShortReadStop  = Qtd;

  //
  // Create a QH to act as the EHC reclamation header.
  // Set the header to loopback to itself.
  //
  Ep.DevAddr    = 0;
  Ep.EpAddr     = 1;
  Ep.Direction  = EfiUsbDataIn;
  Ep.DevSpeed   = EFI_USB_SPEED_HIGH;
  Ep.MaxPacket  = 64;
  Ep.HubAddr    = 0;
  Ep.HubPort    = 0;
  Ep.Toggle     = 0;
  Ep.Type       = EHC_BULK_TRANSFER;
  Ep.PollRate   = 1;

  Qh            = EhcCreateQh (Ehc, &Ep);

  if (Qh == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }

  QhHw              = &Qh->QhHw;
  QhHw->HorizonLink = QH_LINK (QhHw, EHC_TYPE_QH, FALSE);
  QhHw->Status      = QTD_STAT_HALTED;
  QhHw->ReclaimHead = 1;
  Ehc->ReclaimHead  = Qh;

  //
  // Create a dummy QH to act as the terminator for periodical schedule
  //
  Ep.EpAddr   = 2;
  Ep.Type     = EHC_INT_TRANSFER_SYNC;

  Qh          = EhcCreateQh (Ehc, &Ep);

  if (Qh == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }

  Qh->QhHw.Status = QTD_STAT_HALTED;
  Ehc->PeriodOne  = Qh;

  return EFI_SUCCESS;
}



/**
  Initialize the schedule data structure such as frame list

  @param  Ehc                   The EHCI device to init schedule data for

  @retval EFI_OUT_OF_RESOURCES  Failed to allocate resource to init schedule data
  @retval EFI_SUCCESS           The schedule data is initialized

**/
EFI_STATUS
EhcInitSched (
  IN USB2_HC_DEV          *Ehc
  )
{
  EFI_PCI_IO_PROTOCOL   *PciIo;
  VOID                  *Buf;
  EFI_PHYSICAL_ADDRESS  PhyAddr;
  VOID                  *Map;
  UINTN                 Pages;
  UINTN                 Bytes;
  UINTN                 Index;
  UINT32                *Desc;
  EFI_STATUS            Status;

  //
  // First initialize the periodical schedule data:
  // 1. Allocate and map the memory for the frame list
  // 2. Create the help QTD/QH
  // 3. Initialize the frame entries
  // 4. Set the frame list register
  //
  PciIo = Ehc->PciIo;

  Bytes = 4096;
  Pages = EFI_SIZE_TO_PAGES (Bytes);

  Status = PciIo->AllocateBuffer (
                    PciIo,
                    AllocateAnyPages,
                    EfiBootServicesData,
                    Pages,
                    &Buf,
                    0
                    );

  if (EFI_ERROR (Status)) {
    return EFI_OUT_OF_RESOURCES;
  }

  Status = PciIo->Map (
                    PciIo,
                    EfiPciIoOperationBusMasterCommonBuffer,
                    Buf,
                    &Bytes,
                    &PhyAddr,
                    &Map
                    );

  if (EFI_ERROR (Status) || (Bytes != 4096)) {
    PciIo->FreeBuffer (PciIo, Pages, Buf);
    return EFI_OUT_OF_RESOURCES;
  }

  Ehc->PeriodFrameHost  = Buf;
  Ehc->PeriodFrame      = (VOID *) ((UINTN) PhyAddr);
  Ehc->PeriodFrameMap   = Map;
  Ehc->High32bitAddr    = EHC_HIGH_32BIT (PhyAddr);

  //
  // Init memory pool management then create the helper
  // QTD/QH. If failed, previously allocated resources
  // will be freed by EhcFreeSched
  //
  Ehc->MemPool = UsbHcInitMemPool (
                   PciIo,
                   EHC_BIT_IS_SET (Ehc->HcCapParams, HCCP_64BIT),
                   Ehc->High32bitAddr
                   );

  if (Ehc->MemPool == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }

  Status = EhcCreateHelpQ (Ehc);

  if (EFI_ERROR (Status)) {
    return Status;
  }

  //
  // Initialize the frame list entries then set the registers
  //
  Desc = (UINT32 *) Ehc->PeriodFrame;

  for (Index = 0; Index < EHC_FRAME_LEN; Index++) {
    Desc[Index] = QH_LINK (Ehc->PeriodOne, EHC_TYPE_QH, FALSE);
  }

  EhcWriteOpReg (Ehc, EHC_FRAME_BASE_OFFSET, EHC_LOW_32BIT (Ehc->PeriodFrame));

  //
  // Second initialize the asynchronous schedule:
  // Only need to set the AsynListAddr register to
  // the reclamation header
  //
  EhcWriteOpReg (Ehc, EHC_ASYNC_HEAD_OFFSET, EHC_LOW_32BIT (Ehc->ReclaimHead));
  return EFI_SUCCESS;
}



/**
  Free the schedule data. It may be partially initialized.

  @param  Ehc                   The EHCI device

  @return None

**/
VOID
EhcFreeSched (
  IN USB2_HC_DEV          *Ehc
  )
{
  EFI_PCI_IO_PROTOCOL     *PciIo;

  EhcWriteOpReg (Ehc, EHC_FRAME_BASE_OFFSET, 0);
  EhcWriteOpReg (Ehc, EHC_ASYNC_HEAD_OFFSET, 0);

  if (Ehc->PeriodOne != NULL) {
    UsbHcFreeMem (Ehc->MemPool, Ehc->PeriodOne, sizeof (EHC_QH));
    Ehc->PeriodOne = NULL;
  }

  if (Ehc->ReclaimHead != NULL) {
    UsbHcFreeMem (Ehc->MemPool, Ehc->ReclaimHead, sizeof (EHC_QH));
    Ehc->ReclaimHead = NULL;
  }

  if (Ehc->ShortReadStop != NULL) {
    UsbHcFreeMem (Ehc->MemPool, Ehc->ShortReadStop, sizeof (EHC_QTD));
    Ehc->ShortReadStop = NULL;
  }

  if (Ehc->MemPool != NULL) {
    UsbHcFreeMemPool (Ehc->MemPool);
    Ehc->MemPool = NULL;
  }

  if (Ehc->PeriodFrame != NULL) {
    PciIo = Ehc->PciIo;
    ASSERT (PciIo != NULL);

    PciIo->Unmap (PciIo, Ehc->PeriodFrameMap);

    PciIo->FreeBuffer (
             PciIo,
             EFI_SIZE_TO_PAGES (EFI_PAGE_SIZE),
             Ehc->PeriodFrameHost
             );

    Ehc->PeriodFrame = NULL;
  }
}



/**
  Link the queue head to the asynchronous schedule list.
  UEFI only supports one CTRL/BULK transfer at a time
  due to its interfaces. This simplifies the AsynList
  management: A reclamation header is always linked to
  the AsyncListAddr, the only active QH is appended to it.

  @param  Ehc                   The EHCI device
  @param  Qh                    The queue head to link

  @return None

**/
VOID
EhcLinkQhToAsync (
  IN USB2_HC_DEV          *Ehc,
  IN EHC_QH               *Qh
  )
{
  EHC_QH                  *Head;

  //
  // Append the queue head after the reclaim header, then
  // fix the hardware visiable parts (EHCI R1.0 page 72).
  // ReclaimHead is always linked to the EHCI's AsynListAddr.
  //
  Head                    = Ehc->ReclaimHead;

  Qh->NextQh              = Head->NextQh;
  Head->NextQh            = Qh;

  Qh->QhHw.HorizonLink    = QH_LINK (Head, EHC_TYPE_QH, FALSE);;
  Head->QhHw.HorizonLink  = QH_LINK (Qh, EHC_TYPE_QH, FALSE);
}


/**
  Unlink a queue head from the asynchronous schedule list.
  Need to synchronize with hardware

  @param  Ehc                   The EHCI device
  @param  Qh                    The queue head to unlink

  @return None

**/
VOID
EhcUnlinkQhFromAsync (
  IN USB2_HC_DEV          *Ehc,
  IN EHC_QH               *Qh
  )
{
  EHC_QH                  *Head;
  EFI_STATUS              Status;

  ASSERT (Ehc->ReclaimHead->NextQh == Qh);

  //
  // Remove the QH from reclamation head, then update the hardware
  // visiable part: Only need to loopback the ReclaimHead. The Qh
  // is pointing to ReclaimHead (which is staill in the list).
  //
  Head                    = Ehc->ReclaimHead;

  Head->NextQh            = Qh->NextQh;
  Qh->NextQh              = NULL;

  Head->QhHw.HorizonLink  = QH_LINK (Head, EHC_TYPE_QH, FALSE);

  //
  // Set and wait the door bell to synchronize with the hardware
  //
  Status = EhcSetAndWaitDoorBell (Ehc, EHC_GENERIC_TIME);

  if (EFI_ERROR (Status)) {
    EHC_ERROR (("EhcUnlinkQhFromAsync: Failed to synchronize with doorbell\n"));
  }
}


/**
  Link a queue head for interrupt transfer to the periodic
  schedule frame list. This code is very much the same as
  that in UHCI.

  @param  Ehc                   The EHCI device
  @param  Qh                    The queue head to link

  @return None

**/
VOID
EhcLinkQhToPeriod (
  IN USB2_HC_DEV          *Ehc,
  IN EHC_QH               *Qh
  )
{
  UINT32                  *Frames;
  UINTN                   Index;
  EHC_QH                  *Prev;
  EHC_QH                  *Next;

  Frames = Ehc->PeriodFrame;

  for (Index = 0; Index < EHC_FRAME_LEN; Index += Qh->Interval) {
    //
    // First QH can't be NULL because we always keep PeriodOne
    // heads on the frame list
    //
    ASSERT (!EHC_LINK_TERMINATED (Frames[Index]));
    Next  = EHC_ADDR (Ehc->High32bitAddr, Frames[Index]);
    Prev  = NULL;

    //
    // Now, insert the queue head (Qh) into this frame:
    // 1. Find a queue head with the same poll interval, just insert
    //    Qh after this queue head, then we are done.
    //
    // 2. Find the position to insert the queue head into:
    //      Previous head's interval is bigger than Qh's
    //      Next head's interval is less than Qh's
    //    Then, insert the Qh between then
    //
    while (Next->Interval > Qh->Interval) {
      Prev  = Next;
      Next  = Next->NextQh;
    }

    ASSERT (Next != NULL);

    //
    // The entry may have been linked into the frame by early insertation.
    // For example: if insert a Qh with Qh.Interval == 4, and there is a Qh
    // with Qh.Interval == 8 on the frame. If so, we are done with this frame.
    // It isn't necessary to compare all the QH with the same interval to
    // Qh. This is because if there is other QH with the same interval, Qh
    // should has been inserted after that at Frames[0] and at Frames[0] it is
    // impossible for (Next == Qh)
    //
    if (Next == Qh) {
      continue;
    }

    if (Next->Interval == Qh->Interval) {
      //
      // If there is a QH with the same interval, it locates at
      // Frames[0], and we can simply insert it after this QH. We
      // are all done.
      //
      ASSERT ((Index == 0) && (Qh->NextQh == NULL));

      Prev                    = Next;
      Next                    = Next->NextQh;

      Qh->NextQh              = Next;
      Prev->NextQh            = Qh;

      Qh->QhHw.HorizonLink    = Prev->QhHw.HorizonLink;
      Prev->QhHw.HorizonLink  = QH_LINK (Qh, EHC_TYPE_QH, FALSE);
      break;
    }

    //
    // OK, find the right position, insert it in. If Qh's next
    // link has already been set, it is in position. This is
    // guarranted by 2^n polling interval.
    //
    if (Qh->NextQh == NULL) {
      Qh->NextQh              = Next;
      Qh->QhHw.HorizonLink    = QH_LINK (Next, EHC_TYPE_QH, FALSE);
    }

    if (Prev == NULL) {
      Frames[Index] = QH_LINK (Qh, EHC_TYPE_QH, FALSE);
    } else {
      Prev->NextQh            = Qh;
      Prev->QhHw.HorizonLink  = QH_LINK (Qh, EHC_TYPE_QH, FALSE);
    }
  }
}


/**
  Unlink an interrupt queue head from the periodic
  schedule frame list

  @param  Ehc                   The EHCI device
  @param  Qh                    The queue head to unlink

  @return None

**/
VOID
EhcUnlinkQhFromPeriod (
  IN USB2_HC_DEV          *Ehc,
  IN EHC_QH               *Qh
  )
{
  UINT32                  *Frames;
  UINTN                   Index;
  EHC_QH                  *Prev;
  EHC_QH                  *This;

  Frames = Ehc->PeriodFrame;

  for (Index = 0; Index < EHC_FRAME_LEN; Index += Qh->Interval) {
    //
    // Frame link can't be NULL because we always keep PeroidOne
    // on the frame list
    //
    ASSERT (!EHC_LINK_TERMINATED (Frames[Index]));
    This  = EHC_ADDR (Ehc->High32bitAddr, Frames[Index]);
    Prev  = NULL;

    //
    // Walk through the frame's QH list to find the
    // queue head to remove
    //
    while ((This != NULL) && (This != Qh)) {
      Prev  = This;
      This  = This->NextQh;
    }

    //
    // Qh may have already been unlinked from this frame
    // by early action. See the comments in EhcLinkQhToPeriod.
    //
    if (This == NULL) {
      continue;
    }

    if (Prev == NULL) {
      //
      // Qh is the first entry in the frame
      //
      Frames[Index] = Qh->QhHw.HorizonLink;
    } else {
      Prev->NextQh            = Qh->NextQh;
      Prev->QhHw.HorizonLink  = Qh->QhHw.HorizonLink;
    }
  }
}



/**
  Check the URB's execution result and update the URB's
  result accordingly.

  @param  Ehc                   The EHCI device
  @param  Urb                   The URB to check result

  @return Whether the result of URB transfer is finialized.

**/
STATIC
BOOLEAN
EhcCheckUrbResult (
  IN  USB2_HC_DEV         *Ehc,
  IN  URB                 *Urb
  )
{
  LIST_ENTRY              *Entry;
  EHC_QTD                 *Qtd;
  QTD_HW                  *QtdHw;
  UINT8                   State;
  BOOLEAN                 Finished;

  ASSERT ((Ehc != NULL) && (Urb != NULL) && (Urb->Qh != NULL));

  Finished        = TRUE;
  Urb->Completed  = 0;

  Urb->Result     = EFI_USB_NOERROR;

  if (EhcIsHalt (Ehc) || EhcIsSysError (Ehc)) {
    Urb->Result |= EFI_USB_ERR_SYSTEM;
    goto ON_EXIT;
  }

  EFI_LIST_FOR_EACH (Entry, &Urb->Qh->Qtds) {
    Qtd   = EFI_LIST_CONTAINER (Entry, EHC_QTD, QtdList);
    QtdHw = &Qtd->QtdHw;
    State = (UINT8) QtdHw->Status;

    if (EHC_BIT_IS_SET (State, QTD_STAT_HALTED)) {
      //
      // EHCI will halt the queue head when met some error.
      // If it is halted, the result of URB is finialized.
      //
      if ((State & QTD_STAT_ERR_MASK) == 0) {
        Urb->Result |= EFI_USB_ERR_STALL;
      }

      if (EHC_BIT_IS_SET (State, QTD_STAT_BABBLE_ERR)) {
        Urb->Result |= EFI_USB_ERR_BABBLE;
      }

      if (EHC_BIT_IS_SET (State, QTD_STAT_BUFF_ERR)) {
        Urb->Result |= EFI_USB_ERR_BUFFER;
      }

      if (EHC_BIT_IS_SET (State, QTD_STAT_TRANS_ERR) && (QtdHw->ErrCnt == 0)) {
        Urb->Result |= EFI_USB_ERR_TIMEOUT;
      }

      Finished = TRUE;
      goto ON_EXIT;

    } else if (EHC_BIT_IS_SET (State, QTD_STAT_ACTIVE)) {
      //
      // The QTD is still active, no need to check furthur.
      //
      Urb->Result |= EFI_USB_ERR_NOTEXECUTE;

      Finished = FALSE;
      goto ON_EXIT;

    } else {
      //
      // This QTD is finished OK or met short packet read. Update the
      // transfer length if it isn't a setup.
      //
      if (QtdHw->Pid != QTD_PID_SETUP) {
        Urb->Completed += Qtd->DataLen - QtdHw->TotalBytes;
      }

      if ((QtdHw->TotalBytes != 0) && (QtdHw->Pid == QTD_PID_INPUT)) {
        EHC_DUMP_QH ((Urb->Qh, "Short packet read", FALSE));

        //
        // Short packet read condition. If it isn't a setup transfer,
        // no need to check furthur: the queue head will halt at the
        // ShortReadStop. If it is a setup transfer, need to check the
        // Status Stage of the setup transfer to get the finial result
        //
        if (QtdHw->AltNext == QTD_LINK (Ehc->ShortReadStop, FALSE)) {
          EHC_DEBUG (("EhcCheckUrbResult: Short packet read, break\n"));

          Finished = TRUE;
          goto ON_EXIT;
        }

        EHC_DEBUG (("EhcCheckUrbResult: Short packet read, continue\n"));
      }
    }
  }

ON_EXIT:
  //
  // Return the data toggle set by EHCI hardware, bulk and interrupt
  // transfer will use this to initialize the next transaction. For
  // Control transfer, it always start a new data toggle sequence for
  // new transfer.
  //
  // NOTICE: don't move DT update before the loop, otherwise there is
  // a race condition that DT is wrong.
  //
  Urb->DataToggle = (UINT8) Urb->Qh->QhHw.DataToggle;

  return Finished;
}


/**
  Execute the transfer by polling the URB. This is a synchronous operation.

  @param  Ehc                   The EHCI device
  @param  Urb                   The URB to execute
  @param  TimeOut               The time to wait before abort, in millisecond.

  @return EFI_DEVICE_ERROR : The transfer failed due to transfer error
  @return EFI_TIMEOUT      : The transfer failed due to time out
  @return EFI_SUCCESS      : The transfer finished OK

**/
EFI_STATUS
EhcExecTransfer (
  IN  USB2_HC_DEV         *Ehc,
  IN  URB                 *Urb,
  IN  UINTN               TimeOut
  )
{
  EFI_STATUS              Status;
  UINTN                   Index;
  UINTN                   Loop;
  BOOLEAN                 Finished;

  Status    = EFI_SUCCESS;
  Loop      = (TimeOut * EHC_STALL_1_MILLISECOND / EHC_SYNC_POLL_TIME) + 1;
  Finished  = FALSE;

  for (Index = 0; Index < Loop; Index++) {
    Finished = EhcCheckUrbResult (Ehc, Urb);

    if (Finished) {
      break;
    }

    gBS->Stall (EHC_SYNC_POLL_TIME);
  }

  if (!Finished) {
    EHC_ERROR (("EhcExecTransfer: transfer not finished in %dms\n", TimeOut));
    EHC_DUMP_QH ((Urb->Qh, NULL, FALSE));

    Status = EFI_TIMEOUT;

  } else if (Urb->Result != EFI_USB_NOERROR) {
    EHC_ERROR (("EhcExecTransfer: transfer failed with %x\n", Urb->Result));
    EHC_DUMP_QH ((Urb->Qh, NULL, FALSE));

    Status = EFI_DEVICE_ERROR;
  }

  return Status;
}


/**
  Delete a single asynchronous interrupt transfer for
  the device and endpoint

  @param  Ehc                   The EHCI device
  @param  DevAddr               The address of the target device
  @param  EpNum                 The endpoint of the target
  @param  DataToggle            Return the next data toggle to use

  @retval EFI_SUCCESS           An asynchronous transfer is removed
  @retval EFI_NOT_FOUND         No transfer for the device is found

**/
EFI_STATUS
EhciDelAsyncIntTransfer (
  IN  USB2_HC_DEV         *Ehc,
  IN  UINT8               DevAddr,
  IN  UINT8               EpNum,
  OUT UINT8               *DataToggle
  )
{
  LIST_ENTRY              *Entry;
  LIST_ENTRY              *Next;
  URB                     *Urb;
  EFI_USB_DATA_DIRECTION  Direction;

  Direction = ((EpNum & 0x80) ? EfiUsbDataIn : EfiUsbDataOut);
  EpNum    &= 0x0F;

  EFI_LIST_FOR_EACH_SAFE (Entry, Next, &Ehc->AsyncIntTransfers) {
    Urb = EFI_LIST_CONTAINER (Entry, URB, UrbList);

    if ((Urb->Ep.DevAddr == DevAddr) && (Urb->Ep.EpAddr == EpNum) &&
        (Urb->Ep.Direction == Direction)) {
      //
      // Check the URB status to retrieve the next data toggle
      // from the associated queue head.
      //
      EhcCheckUrbResult (Ehc, Urb);
      *DataToggle = Urb->DataToggle;

      EhcUnlinkQhFromPeriod (Ehc, Urb->Qh);
      RemoveEntryList (&Urb->UrbList);

      gBS->FreePool (Urb->Data);
      EhcFreeUrb (Ehc, Urb);
      return EFI_SUCCESS;
    }
  }

  return EFI_NOT_FOUND;
}


/**
  Remove all the asynchronous interrutp transfers

  @param  Ehc                   The EHCI device

  @return None

**/
VOID
EhciDelAllAsyncIntTransfers (
  IN USB2_HC_DEV          *Ehc
  )
{
  LIST_ENTRY              *Entry;
  LIST_ENTRY              *Next;
  URB                     *Urb;

  EFI_LIST_FOR_EACH_SAFE (Entry, Next, &Ehc->AsyncIntTransfers) {
    Urb = EFI_LIST_CONTAINER (Entry, URB, UrbList);

    EhcUnlinkQhFromPeriod (Ehc, Urb->Qh);
    RemoveEntryList (&Urb->UrbList);

    gBS->FreePool (Urb->Data);
    EhcFreeUrb (Ehc, Urb);
  }
}



/**
  Update the queue head for next round of asynchronous transfer

  @param  Urb                   The URB to update

  @return None

**/
STATIC
VOID
EhcUpdateAsyncRequest (
  IN URB                  *Urb
  )
{
  LIST_ENTRY              *Entry;
  EHC_QTD                 *FirstQtd;
  QH_HW                   *QhHw;
  EHC_QTD                 *Qtd;
  QTD_HW                  *QtdHw;
  UINTN                   Index;

  Qtd = NULL;

  if (Urb->Result == EFI_USB_NOERROR) {
    FirstQtd = NULL;

    EFI_LIST_FOR_EACH (Entry, &Urb->Qh->Qtds) {
      Qtd = EFI_LIST_CONTAINER (Entry, EHC_QTD, QtdList);

      if (FirstQtd == NULL) {
        FirstQtd = Qtd;
      }

      //
      // Update the QTD for next round of transfer. Host control
      // may change dt/Total Bytes to Transfer/C_Page/Cerr/Status/
      // Current Offset. These fields need to be updated. DT isn't
      // used by interrupt transfer. It uses DT in queue head.
      // Current Offset is in Page[0], only need to reset Page[0]
      // to initial data buffer.
      //
      QtdHw             = &Qtd->QtdHw;
      QtdHw->Status     = QTD_STAT_ACTIVE;
      QtdHw->ErrCnt     = QTD_MAX_ERR;
      QtdHw->CurPage    = 0;
      QtdHw->TotalBytes = (UINT32) Qtd->DataLen;
      QtdHw->Page[0]    = EHC_LOW_32BIT (Qtd->Data);
    }

    //
    // Update QH for next round of transfer. Host control only
    // touch the fields in transfer overlay area. Only need to
    // zero out the overlay area and set NextQtd to the first
    // QTD. DateToggle bit is left untouched.
    //
    QhHw              = &Urb->Qh->QhHw;
    QhHw->CurQtd      = QTD_LINK (0, TRUE);
    QhHw->AltQtd      = 0;

    QhHw->Status      = 0;
    QhHw->Pid         = 0;
    QhHw->ErrCnt      = 0;
    QhHw->CurPage     = 0;
    QhHw->IOC         = 0;
    QhHw->TotalBytes  = 0;

    for (Index = 0; Index < 5; Index++) {
      QhHw->Page[Index]     = 0;
      QhHw->PageHigh[Index] = 0;
    }

    QhHw->NextQtd = QTD_LINK (FirstQtd, FALSE);
  }

  return ;
}


/**
  Interrupt transfer periodic check handler

  @param  Event                 Interrupt event
  @param  Context               Pointer to USB2_HC_DEV

  @return None

**/
VOID
EhcMoniteAsyncRequests (
  IN EFI_EVENT            Event,
  IN VOID                 *Context
  )
{
  USB2_HC_DEV             *Ehc;
  EFI_TPL                 OldTpl;
  LIST_ENTRY              *Entry;
  LIST_ENTRY              *Next;
  BOOLEAN                 Finished;
  UINT8                   *ProcBuf;
  URB                     *Urb;

  OldTpl  = gBS->RaiseTPL (EHC_TPL);
  Ehc     = (USB2_HC_DEV *) Context;

  EFI_LIST_FOR_EACH_SAFE (Entry, Next, &Ehc->AsyncIntTransfers) {
    Urb = EFI_LIST_CONTAINER (Entry, URB, UrbList);

    //
    // Check the result of URB execution. If it is still
    // active, check the next one.
    //
    Finished = EhcCheckUrbResult (Ehc, Urb);

    if (!Finished) {
      continue;
    }

    //
    // Allocate a buffer then copy the transferred data for user.
    // If failed to allocate the buffer, update the URB for next
    // round of transfer. Ignore the data of this round.
    //
    ProcBuf = NULL;

    if (Urb->Result == EFI_USB_NOERROR) {
      ASSERT (Urb->Completed <= Urb->DataLen);

      ProcBuf = AllocatePool (Urb->Completed);

      if (ProcBuf == NULL) {
        EhcUpdateAsyncRequest (Urb);
        continue;
      }

      CopyMem (ProcBuf, Urb->Data, Urb->Completed);
    }

    EhcUpdateAsyncRequest (Urb);

    //
    // Leave error recovery to its related device driver. A
    // common case of the error recovery is to re-submit the
    // interrupt transfer which is linked to the head of the
    // list. This function scans from head to tail. So the
    // re-submitted interrupt transfer's callback function
    // will not be called again in this round. Don't touch this
    // URB after the callback, it may have been removed by the
    // callback.
    //
    if (Urb->Callback != NULL) {
      //
      // Restore the old TPL, USB bus maybe connect device in
      // his callback. Some drivers may has a lower TPL restriction.
      //
      gBS->RestoreTPL (OldTpl);
      (Urb->Callback) (ProcBuf, Urb->Completed, Urb->Context, Urb->Result);
      OldTpl = gBS->RaiseTPL (EHC_TPL);
    }

    if (ProcBuf != NULL) {
      gBS->FreePool (ProcBuf);
    }
  }

  gBS->RestoreTPL (OldTpl);
}
