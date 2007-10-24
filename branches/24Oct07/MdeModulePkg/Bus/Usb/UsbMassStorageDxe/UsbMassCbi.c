/** @file

Copyright (c) 2007, Intel Corporation
All rights reserved. This program and the accompanying materials
are licensed and made available under the terms and conditions of the BSD License
which accompanies this distribution.  The full text of the license may be found at
http://opensource.org/licenses/bsd-license.php

THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

Module Name:

  UsbMassCbi.c

Abstract:

  Implementation of the USB mass storage Control/Bulk/Interrupt transpor.
  Notice: it is being obseleted by the standard body in favor of the BOT
  (Bulk-Only Transport).

Revision History


**/

#include "UsbMass.h"
#include "UsbMassCbi.h"

UINTN mUsbCbiInfo  = DEBUG_INFO;
UINTN mUsbCbiError = DEBUG_ERROR;

STATIC
EFI_STATUS
UsbCbiResetDevice (
  IN  VOID                    *Context,
  IN  BOOLEAN                  ExtendedVerification
  );


/**
  Initialize the USB mass storage class CBI transport protocol.
  If Context isn't NULL, it will save its context in it.

  @param  UsbIo                 The USB IO to use
  @param  Controller            The device controller
  @param  Context               The variable to save context in

  @retval EFI_OUT_OF_RESOURCES  Failed to allocate memory
  @retval EFI_UNSUPPORTED       The device isn't supported
  @retval EFI_SUCCESS           The CBI protocol is initialized.

**/
STATIC
EFI_STATUS
UsbCbiInit (
  IN  EFI_USB_IO_PROTOCOL   *UsbIo,
  IN  EFI_HANDLE            Controller,
  OUT VOID                  **Context       OPTIONAL
  )
{
  USB_CBI_PROTOCOL              *UsbCbi;
  EFI_USB_INTERFACE_DESCRIPTOR  *Interface;
  EFI_USB_ENDPOINT_DESCRIPTOR   EndPoint;
  EFI_STATUS                    Status;
  UINT8                         Index;

  //
  // Allocate the CBI context
  //
  UsbCbi = AllocateZeroPool (
             sizeof (USB_CBI_PROTOCOL) + 3 * sizeof (EFI_USB_ENDPOINT_DESCRIPTOR)
             );

  if (UsbCbi == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }

  UsbCbi->UsbIo = UsbIo;

  //
  // Get the interface descriptor and validate that it is a USB mass
  // storage class CBI interface.
  //
  Status = UsbIo->UsbGetInterfaceDescriptor (UsbIo, &UsbCbi->Interface);
  if (EFI_ERROR (Status)) {
    goto ON_ERROR;
  }

  Interface = &UsbCbi->Interface;
  if ((Interface->InterfaceProtocol != USB_MASS_STORE_CBI0)
      && (Interface->InterfaceProtocol != USB_MASS_STORE_CBI1)) {
    Status = EFI_UNSUPPORTED;
    goto ON_ERROR;
  }

  //
  // Locate and save the bulk-in, bulk-out, and interrupt endpoint
  //
  for (Index = 0; Index < Interface->NumEndpoints; Index++) {
    Status = UsbIo->UsbGetEndpointDescriptor (UsbIo, Index, &EndPoint);
    if (EFI_ERROR (Status)) {
      continue;
    }

    if (USB_IS_BULK_ENDPOINT (EndPoint.Attributes)) {
      //
      // Use the first Bulk-In and Bulk-Out endpoints
      //
      if (USB_IS_IN_ENDPOINT (EndPoint.EndpointAddress) &&
         (UsbCbi->BulkInEndpoint == NULL)) {

        UsbCbi->BulkInEndpoint  = (EFI_USB_ENDPOINT_DESCRIPTOR *) (UsbCbi + 1);
        CopyMem(UsbCbi->BulkInEndpoint, &EndPoint, sizeof (EndPoint));;
      }

      if (USB_IS_OUT_ENDPOINT (EndPoint.EndpointAddress) &&
         (UsbCbi->BulkOutEndpoint == NULL)) {

        UsbCbi->BulkOutEndpoint   = (EFI_USB_ENDPOINT_DESCRIPTOR *) (UsbCbi + 1) + 1;
        CopyMem(UsbCbi->BulkOutEndpoint, &EndPoint, sizeof (EndPoint));
      }

    } else if (USB_IS_INTERRUPT_ENDPOINT (EndPoint.Attributes)) {
      //
      // Use the first interrupt endpoint if it is CBI0
      //
      if ((Interface->InterfaceProtocol == USB_MASS_STORE_CBI0) &&
          (UsbCbi->InterruptEndpoint == NULL)) {

        UsbCbi->InterruptEndpoint   = (EFI_USB_ENDPOINT_DESCRIPTOR *) (UsbCbi + 1) + 2;
        CopyMem(UsbCbi->InterruptEndpoint, &EndPoint, sizeof (EndPoint));
      }
    }
  }

  if ((UsbCbi->BulkInEndpoint == NULL)
      || (UsbCbi->BulkOutEndpoint == NULL)
      || ((Interface->InterfaceProtocol == USB_MASS_STORE_CBI0)
          && (UsbCbi->InterruptEndpoint == NULL))) {
    Status = EFI_UNSUPPORTED;
    goto ON_ERROR;
  }

  if (Context != NULL) {
    *Context = UsbCbi;
  } else {
    gBS->FreePool (UsbCbi);
  }
  return EFI_SUCCESS;

ON_ERROR:
  gBS->FreePool (UsbCbi);
  return Status;
}



/**
  Send the command to the device using class specific control transfer.

  @param  UsbCbi                The USB CBI protocol
  @param  Cmd                   The high level command to transfer to device
  @param  CmdLen                The length of the command
  @param  Timeout               The time to wait the command to finish

  @retval EFI_SUCCESS           The command is transferred to device
  @retval Others                The command failed to transfer to device

**/
STATIC
EFI_STATUS
UsbCbiSendCommand (
  IN USB_CBI_PROTOCOL       *UsbCbi,
  IN UINT8                  *Cmd,
  IN UINT8                  CmdLen,
  IN UINT32                 Timeout
  )
{
  EFI_USB_DEVICE_REQUEST  Request;
  EFI_STATUS              Status;
  UINT32                  TransStatus;
  UINTN                   DataLen;
  INTN                    Retry;

  //
  // Fill in the device request, CBI use the "Accept Device-Specific
  // Cmd" (ADSC) class specific request to send commands
  //
  Request.RequestType = 0x21;
  Request.Request     = 0;
  Request.Value       = 0;
  Request.Index       = UsbCbi->Interface.InterfaceNumber;
  Request.Length      = CmdLen;

  Status              = EFI_SUCCESS;
  Timeout             = Timeout / USB_MASS_1_MILLISECOND;

  for (Retry = 0; Retry < USB_CBI_MAX_RETRY; Retry++) {
    //
    // Use the UsbIo to send the command to the device
    //
    TransStatus = 0;
    DataLen     = CmdLen;

    Status = UsbCbi->UsbIo->UsbControlTransfer (
                              UsbCbi->UsbIo,
                              &Request,
                              EfiUsbDataOut,
                              Timeout,
                              Cmd,
                              DataLen,
                              &TransStatus
                              );
    //
    // The device can fail the command by STALL the control endpoint.
    // It can delay the command by NAK the data or status stage, this
    // is a "class-specific exemption to the USB specification". Retry
    // if the command is NAKed.
    //
    if (EFI_ERROR (Status) && (TransStatus == EFI_USB_ERR_NAK)) {
      continue;
    }

    break;
  }

  return Status;
}


/**
  Transfer data between the device and host. The CBI contains three phase,
  command, data, and status. This is data phase.

  @param  UsbCbi                The USB CBI device
  @param  DataDir               The direction of the data transfer
  @param  Data                  The buffer to hold the data
  @param  TransLen              The expected transfer length
  @param  Timeout               The time to wait the command to execute

  @retval EFI_SUCCESS           The data transfer succeeded
  @retval Others                Failed to transfer all the data

**/
STATIC
EFI_STATUS
UsbCbiDataTransfer (
  IN USB_CBI_PROTOCOL         *UsbCbi,
  IN EFI_USB_DATA_DIRECTION   DataDir,
  IN OUT UINT8                *Data,
  IN OUT UINTN                *TransLen,
  IN UINT32                   Timeout
  )
{
  EFI_USB_ENDPOINT_DESCRIPTOR *Endpoint;
  EFI_STATUS                  Status;
  UINT32                      TransStatus;
  UINTN                       Remain;
  UINTN                       Increment;
  UINT8                       *Next;
  UINTN                       Retry;

  //
  // It's OK if no data to transfer
  //
  if ((DataDir == EfiUsbNoData) || (*TransLen == 0)) {
    return EFI_SUCCESS;
  }

  //
  // Select the endpoint then issue the transfer
  //
  if (DataDir == EfiUsbDataIn) {
    Endpoint = UsbCbi->BulkInEndpoint;
  } else {
    Endpoint = UsbCbi->BulkOutEndpoint;
  }

  Next    = Data;
  Remain  = *TransLen;
  Retry   = 0;
  Status  = EFI_SUCCESS;
  Timeout = Timeout / USB_MASS_1_MILLISECOND;

  //
  // Transfer the data, if the device returns NAK, retry it.
  //
  while (Remain > 0) {
    TransStatus = 0;

    if (Remain > (UINTN) USB_CBI_MAX_PACKET_NUM * Endpoint->MaxPacketSize) {
      Increment = USB_CBI_MAX_PACKET_NUM * Endpoint->MaxPacketSize;
    } else {
      Increment = Remain;
    }

    Status = UsbCbi->UsbIo->UsbBulkTransfer (
                              UsbCbi->UsbIo,
                              Endpoint->EndpointAddress,
                              Next,
                              &Increment,
                              Timeout,
                              &TransStatus
                              );
    if (EFI_ERROR (Status)) {
      if (TransStatus == EFI_USB_ERR_NAK) {
        //
        // The device can NAK the host if either the data/buffer isn't
        // aviable or the command is in-progress. The data can be partly
        // transferred. The transfer is aborted if several succssive data
        // transfer commands are NAKed.
        //
        if (Increment == 0) {
          if (++Retry > USB_CBI_MAX_RETRY) {
            goto ON_EXIT;
          }

        } else {
          Next   += Increment;
          Remain -= Increment;
          Retry   = 0;
        }

        continue;
      }

      //
      // The device can fail the command by STALL the bulk endpoint.
      // Clear the stall if that is the case.
      //
      if (TransStatus == EFI_USB_ERR_STALL) {
        UsbClearEndpointStall (UsbCbi->UsbIo, Endpoint->EndpointAddress);
      }

      goto ON_EXIT;
    }

    Next += Increment;
    Remain -= Increment;
  }

ON_EXIT:
  *TransLen -= Remain;
  return Status;
}


/**
  Get the result of high level command execution from interrupt
  endpoint. This function returns the USB transfer status, and
  put the high level command execution result in Result.

  @param  UsbCbi                The USB CBI protocol
  @param  Timeout               The time to wait the command to execute
  @param  Result                GC_TODO: add argument description

  @retval EFI_SUCCESS           The high level command execution result is
                                retrieved in Result.
  @retval Others                Failed to retrieve the result.

**/
STATIC
EFI_STATUS
UsbCbiGetStatus (
  IN  USB_CBI_PROTOCOL        *UsbCbi,
  IN  UINT32                  Timeout,
  OUT USB_CBI_STATUS          *Result
  )
{
  UINTN                     Len;
  UINT8                     Endpoint;
  EFI_STATUS                Status;
  UINT32                    TransStatus;
  INTN                      Retry;

  Endpoint  = UsbCbi->InterruptEndpoint->EndpointAddress;
  Status    = EFI_SUCCESS;
  Timeout   = Timeout / USB_MASS_1_MILLISECOND;

  //
  // Attemp to the read the result from interrupt endpoint
  //
  for (Retry = 0; Retry < USB_CBI_MAX_RETRY; Retry++) {
    TransStatus = 0;
    Len         = sizeof (USB_CBI_STATUS);

    Status = UsbCbi->UsbIo->UsbSyncInterruptTransfer (
                              UsbCbi->UsbIo,
                              Endpoint,
                              Result,
                              &Len,
                              Timeout,
                              &TransStatus
                              );
    //
    // The CBI can NAK the interrupt endpoint if the command is in-progress.
    //
    if (EFI_ERROR (Status) && (TransStatus == EFI_USB_ERR_NAK)) {
      continue;
    }

    break;
  }

  return Status;
}


/**
  Execute USB mass storage command through the CBI0/CBI1 transport protocol

  @param  Context               The USB CBI device
  @param  Cmd                   The command to transfer to device
  @param  CmdLen                The length of the command
  @param  DataDir               The direction of data transfer
  @param  Data                  The buffer to hold the data
  @param  DataLen               The length of the buffer
  @param  Timeout               The time to wait
  @param  CmdStatus             The result of the command execution

  @retval EFI_SUCCESS           The command is executed OK and result in CmdStatus.
  @retval EFI_DEVICE_ERROR      Failed to execute the command

**/
STATIC
EFI_STATUS
UsbCbiExecCommand (
  IN  VOID                    *Context,
  IN  VOID                    *Cmd,
  IN  UINT8                   CmdLen,
  IN  EFI_USB_DATA_DIRECTION  DataDir,
  IN  VOID                    *Data,
  IN  UINT32                  DataLen,
  IN  UINT32                  Timeout,
  OUT UINT32                  *CmdStatus
  )
{
  USB_CBI_PROTOCOL          *UsbCbi;
  USB_CBI_STATUS            Result;
  EFI_STATUS                Status;
  UINTN                     TransLen;

  *CmdStatus  = USB_MASS_CMD_SUCCESS;
  UsbCbi      = (USB_CBI_PROTOCOL *) Context;

  //
  // Send the command to the device. Return immediately if device
  // rejects the command.
  //
  Status = UsbCbiSendCommand (UsbCbi, Cmd, CmdLen, Timeout);
  if (EFI_ERROR (Status)) {
    DEBUG ((mUsbCbiError, "UsbCbiExecCommand: UsbCbiSendCommand (%r)\n",Status));
    return Status;
  }

  //
  // Transfer the data, return this status if no interrupt endpoint
  // is used to report the transfer status.
  //
  TransLen = (UINTN) DataLen;

  Status   = UsbCbiDataTransfer (UsbCbi, DataDir, Data, &TransLen, Timeout);
  if (UsbCbi->InterruptEndpoint == NULL) {
    DEBUG ((mUsbCbiError, "UsbCbiExecCommand: UsbCbiDataTransfer (%r)\n",Status));
    return Status;
  }

  //
  // Get the status, if that succeeds, interpret the result
  //
  Status = UsbCbiGetStatus (UsbCbi, Timeout, &Result);
  if (EFI_ERROR (Status)) {
    DEBUG ((mUsbCbiError, "UsbCbiExecCommand: UsbCbiGetStatus (%r)\n",Status));
    return EFI_DEVICE_ERROR;
  }

  if (UsbCbi->Interface.InterfaceSubClass == USB_MASS_STORE_UFI) {
    //
    // For UFI device, ASC and ASCQ are returned.
    //
    if (Result.Type != 0) {
      *CmdStatus = USB_MASS_CMD_FAIL;
    }

  } else {
    //
    // Check page 27, CBI spec 1.1 for vaious reture status.
    //
    switch (Result.Value & 0x03) {
    case 0x00:
      //
      // Pass
      //
      *CmdStatus = USB_MASS_CMD_SUCCESS;
      break;

    case 0x02:
      //
      // Phase Error, response with reset. Fall through to Fail.
      //
      UsbCbiResetDevice (UsbCbi, FALSE);

    case 0x01:
      //
      // Fail
      //
      *CmdStatus = USB_MASS_CMD_FAIL;
      break;

    case 0x03:
      //
      // Persistent Fail, need to send REQUEST SENSE.
      //
      *CmdStatus = USB_MASS_CMD_PERSISTENT;
      break;
    }
  }

  return EFI_SUCCESS;
}


/**
  Call the Usb mass storage class transport protocol to
  reset the device. The reset is defined as a Non-Data
  command. Don't use UsbCbiExecCommand to send the command
  to device because that may introduce recursive loop.

  @param  Context               The USB CBI device protocol

  @retval EFI_SUCCESS           the device is reset
  @retval Others                Failed to reset the device

**/
STATIC
EFI_STATUS
UsbCbiResetDevice (
  IN  VOID                    *Context,
  IN  BOOLEAN                  ExtendedVerification
  )
{
  UINT8                     ResetCmd[USB_CBI_RESET_CMD_LEN];
  USB_CBI_PROTOCOL          *UsbCbi;
  USB_CBI_STATUS            Result;
  EFI_STATUS                Status;
  UINT32                    Timeout;

  UsbCbi = (USB_CBI_PROTOCOL *) Context;

  //
  // Fill in the reset command.
  //
  SetMem (ResetCmd, USB_CBI_RESET_CMD_LEN, 0xFF);

  ResetCmd[0] = 0x1D;
  ResetCmd[1] = 0x04;
  Timeout     = USB_CBI_RESET_DEVICE_TIMEOUT / USB_MASS_1_MILLISECOND;

  //
  // Send the command to the device. Don't use UsbCbiExecCommand here.
  //
  Status = UsbCbiSendCommand (UsbCbi, ResetCmd, USB_CBI_RESET_CMD_LEN, Timeout);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  //
  // Just retrieve the status and ignore that. Then stall
  // 50ms to wait it complete
  //
  UsbCbiGetStatus (UsbCbi, Timeout, &Result);
  gBS->Stall (USB_CBI_RESET_DEVICE_STALL);

  //
  // Clear the Bulk-In and Bulk-Out stall condition and
  // init data toggle.
  //
  UsbClearEndpointStall (UsbCbi->UsbIo, UsbCbi->BulkInEndpoint->EndpointAddress);
  UsbClearEndpointStall (UsbCbi->UsbIo, UsbCbi->BulkOutEndpoint->EndpointAddress);
  return Status;
}


/**
  Clean up the CBI protocol's resource

  @param  Context               The CBI protocol

  @retval EFI_SUCCESS           The resource is cleaned up.

**/
STATIC
EFI_STATUS
UsbCbiFini (
  IN  VOID                   *Context
  )
{
  gBS->FreePool (Context);
  return EFI_SUCCESS;
}

USB_MASS_TRANSPORT
mUsbCbi0Transport = {
  USB_MASS_STORE_CBI0,
  UsbCbiInit,
  UsbCbiExecCommand,
  UsbCbiResetDevice,
  UsbCbiFini
};

USB_MASS_TRANSPORT
mUsbCbi1Transport = {
  USB_MASS_STORE_CBI1,
  UsbCbiInit,
  UsbCbiExecCommand,
  UsbCbiResetDevice,
  UsbCbiFini
};
