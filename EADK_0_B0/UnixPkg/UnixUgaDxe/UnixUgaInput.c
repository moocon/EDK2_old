/*++

Copyright (c) 2006, Intel Corporation. All rights reserved.<BR>
Portions copyright (c) 2010, Apple, Inc. All rights reserved.<BR>
This program and the accompanying materials
are licensed and made available under the terms and conditions of the BSD License
which accompanies this distribution.  The full text of the license may be found at
http://opensource.org/licenses/bsd-license.php

THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

Module Name:

  UnixUgaInput.c

Abstract:

  This file produces the Simple Text In for an Uga window.

  This stuff is linked at the hip to the Window, since the window
  processing is done in a thread kicked off in UnixUgaImplementation.c

  Since the window information is processed in an other thread we need
  a keyboard Queue to pass data about. The Simple Text In code just
  takes data off the Queue. The WinProc message loop takes keyboard input
  and places it in the Queue.

--*/

#include "UnixUga.h"

//
// Simple Text In implementation.
//

EFI_STATUS
EFIAPI
UnixUgaSimpleTextInReset (
  IN EFI_SIMPLE_TEXT_INPUT_PROTOCOL          *This,
  IN BOOLEAN                              ExtendedVerification
  )
/*++

Routine Description:

  TODO: Add function description

Arguments:

  This                  - TODO: add argument description
  ExtendedVerification  - TODO: add argument description

Returns:

  EFI_SUCCESS - TODO: Add description for return value

--*/
{
  UGA_PRIVATE_DATA  *Private;
  EFI_KEY_DATA      Key;
  EFI_TPL           OldTpl;

  Private = UGA_PRIVATE_DATA_FROM_TEXT_IN_THIS (This);
  if (Private->UgaIo == NULL) {
    return EFI_SUCCESS;
  }

  //
  // Enter critical section
  //
  OldTpl = gBS->RaiseTPL (TPL_NOTIFY);

  //
  // A reset is draining the Queue
  //
  while (Private->UgaIo->UgaGetKey (Private->UgaIo, &Key) == EFI_SUCCESS)
    ;

  //
  // Leave critical section and return
  //
  gBS->RestoreTPL (OldTpl);
  return EFI_SUCCESS;
}

EFI_STATUS
EFIAPI
UnixUgaSimpleTextInReadKeyStroke (
  IN EFI_SIMPLE_TEXT_INPUT_PROTOCOL          *This,
  OUT EFI_INPUT_KEY                          *Key
  )
/*++

Routine Description:

  TODO: Add function description

Arguments:

  This  - TODO: add argument description
  Key   - TODO: add argument description

Returns:

  TODO: add return values

--*/
{
  UGA_PRIVATE_DATA  *Private;
  EFI_STATUS        Status;
  EFI_TPL           OldTpl;
  EFI_KEY_DATA      KeyData;
  
  Private = UGA_PRIVATE_DATA_FROM_TEXT_IN_THIS (This);
  if (Private->UgaIo == NULL) {
    return EFI_NOT_READY;
  }

  //
  // Enter critical section
  //
  OldTpl  = gBS->RaiseTPL (TPL_NOTIFY);

  Status  = Private->UgaIo->UgaGetKey(Private->UgaIo, &KeyData);
  CopyMem (Key, &KeyData, sizeof (EFI_INPUT_KEY));
  
  //
  // Leave critical section and return
  //
  gBS->RestoreTPL (OldTpl);

  return Status;
}

VOID
EFIAPI
UnixUgaSimpleTextInWaitForKey (
  IN EFI_EVENT          Event,
  IN VOID               *Context
  )
/*++

Routine Description:

  TODO: Add function description

Arguments:

  Event   - TODO: add argument description
  Context - TODO: add argument description

Returns:

  TODO: add return values

--*/
{
  UGA_PRIVATE_DATA  *Private;
  EFI_STATUS        Status;
  EFI_TPL           OldTpl;

  Private = (UGA_PRIVATE_DATA *) Context;
  if (Private->UgaIo == NULL) {
    return;
  }

  //
  // Enter critical section
  //
  OldTpl  = gBS->RaiseTPL (TPL_NOTIFY);

  Status  = Private->UgaIo->UgaCheckKey(Private->UgaIo);
  if (!EFI_ERROR (Status)) {
    //
    // If a there is a key in the queue signal our event.
    //
    gBS->SignalEvent (Event);
  }
  //
  // Leave critical section and return
  //
  gBS->RestoreTPL (OldTpl);
}

//
// Simple Pointer implementation.
//

EFI_STATUS
EFIAPI
UnixUgaSimplePointerReset (
  IN EFI_SIMPLE_POINTER_PROTOCOL          *This,
  IN BOOLEAN                              ExtendedVerification
  )
/*++

Routine Description:

  TODO: Add function description

Arguments:

  This                  - TODO: add argument description
  ExtendedVerification  - TODO: add argument description

Returns:

  EFI_SUCCESS - TODO: Add description for return value

--*/
{
  UGA_PRIVATE_DATA             *Private;
  EFI_SIMPLE_POINTER_STATE     State;
  EFI_TPL                      OldTpl;

  Private = UGA_PRIVATE_DATA_FROM_POINTER_THIS (This);
  if (Private->UgaIo == NULL) {
    return EFI_SUCCESS;
  }

  //
  // Enter critical section
  //
  OldTpl = gBS->RaiseTPL (TPL_NOTIFY);

  //
  // A reset is draining the Queue
  //
  while (Private->UgaIo->UgaGetPointerState(Private->UgaIo, &State) == EFI_SUCCESS)
    ;

  //
  // Leave critical section and return
  //
  gBS->RestoreTPL (OldTpl);
  return EFI_SUCCESS;
}

EFI_STATUS
EFIAPI
UnixUgaSimplePointerGetState (
  IN EFI_SIMPLE_POINTER_PROTOCOL          *This,
  IN OUT EFI_SIMPLE_POINTER_STATE         *State
  )
/*++

Routine Description:

  TODO: Add function description

Arguments:

  This  - TODO: add argument description
  Key   - TODO: add argument description

Returns:

  TODO: add return values

--*/
{
  UGA_PRIVATE_DATA  *Private;
  EFI_STATUS        Status;
  EFI_TPL           OldTpl;

  Private = UGA_PRIVATE_DATA_FROM_POINTER_THIS (This);
  if (Private->UgaIo == NULL) {
    return EFI_NOT_READY;
  }

  //
  // Enter critical section
  //
  OldTpl  = gBS->RaiseTPL (TPL_NOTIFY);

  Status  = Private->UgaIo->UgaGetPointerState(Private->UgaIo, State);
  //
  // Leave critical section and return
  //
  gBS->RestoreTPL (OldTpl);

  return Status;
}

VOID
EFIAPI
UnixUgaSimplePointerWaitForInput (
  IN EFI_EVENT          Event,
  IN VOID               *Context
  )
/*++

Routine Description:

  TODO: Add function description

Arguments:

  Event   - TODO: add argument description
  Context - TODO: add argument description

Returns:

  TODO: add return values

--*/
{
  UGA_PRIVATE_DATA  *Private;
  EFI_STATUS        Status;
  EFI_TPL           OldTpl;

  Private = (UGA_PRIVATE_DATA *) Context;
  if (Private->UgaIo == NULL) {
    return;
  }

  //
  // Enter critical section
  //
  OldTpl  = gBS->RaiseTPL (TPL_NOTIFY);

  Status  = Private->UgaIo->UgaCheckPointer(Private->UgaIo);
  if (!EFI_ERROR (Status)) {
    //
    // If the pointer state has changed, signal our event.
    //
    gBS->SignalEvent (Event);
  }
  //
  // Leave critical section and return
  //
  gBS->RestoreTPL (OldTpl);
}

EFI_STATUS
UnixUgaInitializeSimpleTextInForWindow (
  IN  UGA_PRIVATE_DATA    *Private
  )
/*++

Routine Description:

  TODO: Add function description

Arguments:

  Private - TODO: add argument description

Returns:

  TODO: add return values

--*/
{
  EFI_STATUS  Status;

  //
  // Initialize Simple Text In protoocol
  //
  Private->SimpleTextIn.Reset         = UnixUgaSimpleTextInReset;
  Private->SimpleTextIn.ReadKeyStroke = UnixUgaSimpleTextInReadKeyStroke;

  Status = gBS->CreateEvent (
                  EVT_NOTIFY_WAIT,
                  TPL_NOTIFY,
                  UnixUgaSimpleTextInWaitForKey,
                  Private,
                  &Private->SimpleTextIn.WaitForKey
                  );

  return Status;
}

EFI_STATUS
UnixUgaInitializeSimplePointerForWindow (
  IN  UGA_PRIVATE_DATA    *Private
  )
/*++

Routine Description:

  TODO: Add function description

Arguments:

  Private - TODO: add argument description

Returns:

  TODO: add return values

--*/
{
  EFI_STATUS  Status;

  //
  // Initialize Simple Pointer protoocol
  //
  Private->PointerMode.ResolutionX = 1;
  Private->PointerMode.ResolutionY = 1;
  Private->PointerMode.LeftButton  = TRUE;
  Private->PointerMode.RightButton = TRUE;

  Private->SimplePointer.Reset     = UnixUgaSimplePointerReset;
  Private->SimplePointer.GetState  = UnixUgaSimplePointerGetState;
  Private->SimplePointer.Mode      = &Private->PointerMode;

  Status = gBS->CreateEvent (
                  EVT_NOTIFY_WAIT,
                  TPL_NOTIFY,
                  UnixUgaSimplePointerWaitForInput,
                  Private,
                  &Private->SimplePointer.WaitForInput
                  );

  return Status;
}
