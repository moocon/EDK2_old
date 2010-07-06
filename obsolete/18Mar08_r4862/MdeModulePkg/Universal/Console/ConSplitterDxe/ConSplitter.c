/**@file
  Console Splitter Driver. Any Handle that attatched
  EFI_CONSOLE_IDENTIFIER_PROTOCOL can be bound by this driver.

  So far it works like any other driver by opening a SimpleTextIn and/or
  SimpleTextOut protocol with EFI_OPEN_PROTOCOL_BY_DRIVER attributes. The big
  difference is this driver does not layer a protocol on the passed in
  handle, or construct a child handle like a standard device or bus driver.
  This driver produces three virtual handles as children, one for console input
  splitter, one for console output splitter and one for error output splitter.
  EFI_CONSOLE_SPLIT_PROTOCOL will be attatched onto each virtual handle to
  identify the splitter type.

  Each virtual handle, that supports both the EFI_CONSOLE_SPLIT_PROTOCOL
  and Console I/O protocol, will be produced in the driver entry point.
  The virtual handle are added on driver entry and never removed.
  Such design ensures sytem function well during none console device situation.

Copyright (c) 2006 - 2007 Intel Corporation. <BR>
All rights reserved. This program and the accompanying materials
are licensed and made available under the terms and conditions of the BSD License
which accompanies this distribution.  The full text of the license may be found at
http://opensource.org/licenses/bsd-license.php

THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

#include "ConSplitter.h"

//
// Global Variables
//
STATIC TEXT_IN_SPLITTER_PRIVATE_DATA  mConIn = {
  TEXT_IN_SPLITTER_PRIVATE_DATA_SIGNATURE,
  (EFI_HANDLE) NULL,
  {
    ConSplitterTextInReset,
    ConSplitterTextInReadKeyStroke,
    (EFI_EVENT) NULL
  },
  0,
  (EFI_SIMPLE_TEXT_INPUT_PROTOCOL **) NULL,
  0,
  {
    ConSplitterTextInResetEx,
    ConSplitterTextInReadKeyStrokeEx,
    (EFI_EVENT) NULL,
    ConSplitterTextInSetState,
    ConSplitterTextInRegisterKeyNotify,
    ConSplitterTextInUnregisterKeyNotify
  },
  0,
  (EFI_SIMPLE_TEXT_INPUT_EX_PROTOCOL **) NULL,
  0,
  {
    (struct _LIST_ENTRY     *) NULL,
    (struct _LIST_ENTRY     *) NULL
  },

  {
    ConSplitterSimplePointerReset,
    ConSplitterSimplePointerGetState,
    (EFI_EVENT) NULL,
    (EFI_SIMPLE_POINTER_MODE *) NULL
  },
  {
    0x10000,
    0x10000,
    0x10000,
    TRUE,
    TRUE
  },
  0,
  (EFI_SIMPLE_POINTER_PROTOCOL **) NULL,
  0,

  {
    ConSplitterAbsolutePointerReset,
    ConSplitterAbsolutePointerGetState,
    (EFI_EVENT) NULL,
    (EFI_ABSOLUTE_POINTER_MODE *) NULL
  },

  {
    0,       //AbsoluteMinX
    0,       //AbsoluteMinY
    0,       //AbsoluteMinZ
    0x10000, //AbsoluteMaxX
    0x10000, //AbsoluteMaxY
    0x10000, //AbsoluteMaxZ
    0        //Attributes
  },
  0,
  (EFI_ABSOLUTE_POINTER_PROTOCOL **) NULL,
  0,
  FALSE,

  FALSE,
  {
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
  },
  0,
  {
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
  },
  (EFI_EVENT) NULL,

  FALSE,
  FALSE
};

GLOBAL_REMOVE_IF_UNREFERENCED EFI_UGA_DRAW_PROTOCOL gUgaDrawProtocolTemplate = {
  ConSpliterUgaDrawGetMode,
  ConSpliterUgaDrawSetMode,
  ConSpliterUgaDrawBlt
};

GLOBAL_REMOVE_IF_UNREFERENCED EFI_GRAPHICS_OUTPUT_PROTOCOL gGraphicsOutputProtocolTemplate = {
  ConSpliterGraphicsOutputQueryMode,
  ConSpliterGraphicsOutputSetMode,
  ConSpliterGraphicsOutputBlt,
  NULL
};

STATIC TEXT_OUT_SPLITTER_PRIVATE_DATA mConOut = {
  TEXT_OUT_SPLITTER_PRIVATE_DATA_SIGNATURE,
  (EFI_HANDLE) NULL,
  {
    ConSplitterTextOutReset,
    ConSplitterTextOutOutputString,
    ConSplitterTextOutTestString,
    ConSplitterTextOutQueryMode,
    ConSplitterTextOutSetMode,
    ConSplitterTextOutSetAttribute,
    ConSplitterTextOutClearScreen,
    ConSplitterTextOutSetCursorPosition,
    ConSplitterTextOutEnableCursor,
    (EFI_SIMPLE_TEXT_OUTPUT_MODE *) NULL
  },
  {
    1,
    0,
    0,
    0,
    0,
    FALSE,
  },
  {
    NULL,
    NULL,
    NULL
  },
  0,
  0,
  0,
  0,
  (EFI_UGA_PIXEL *) NULL,
  {
    NULL,
    NULL,
    NULL,
    NULL
  },
  (EFI_GRAPHICS_OUTPUT_BLT_PIXEL *) NULL,
  (EFI_GRAPHICS_OUTPUT_MODE_INFORMATION *) NULL,
  0,
  0,
  TRUE,
  {
    ConSpliterConsoleControlGetMode,
    ConSpliterConsoleControlSetMode,
    ConSpliterConsoleControlLockStdIn
  },

  0,
  (TEXT_OUT_AND_GOP_DATA *) NULL,
  0,
  (TEXT_OUT_SPLITTER_QUERY_DATA *) NULL,
  0,
  (INT32 *) NULL,

  EfiConsoleControlScreenText,
  0,
  0,
  (CHAR16 *) NULL,
  (INT32 *) NULL
};

STATIC TEXT_OUT_SPLITTER_PRIVATE_DATA mStdErr = {
  TEXT_OUT_SPLITTER_PRIVATE_DATA_SIGNATURE,
  (EFI_HANDLE) NULL,
  {
    ConSplitterTextOutReset,
    ConSplitterTextOutOutputString,
    ConSplitterTextOutTestString,
    ConSplitterTextOutQueryMode,
    ConSplitterTextOutSetMode,
    ConSplitterTextOutSetAttribute,
    ConSplitterTextOutClearScreen,
    ConSplitterTextOutSetCursorPosition,
    ConSplitterTextOutEnableCursor,
    (EFI_SIMPLE_TEXT_OUTPUT_MODE *) NULL
  },
  {
    1,
    0,
    0,
    0,
    0,
    FALSE,
  },
  {
    NULL,
    NULL,
    NULL
  },
  0,
  0,
  0,
  0,
  (EFI_UGA_PIXEL *) NULL,
  {
    NULL,
    NULL,
    NULL,
    NULL
  },
  (EFI_GRAPHICS_OUTPUT_BLT_PIXEL *) NULL,
  (EFI_GRAPHICS_OUTPUT_MODE_INFORMATION *) NULL,
  0,
  0,
  TRUE,
  {
    ConSpliterConsoleControlGetMode,
    ConSpliterConsoleControlSetMode,
    ConSpliterConsoleControlLockStdIn
  },

  0,
  (TEXT_OUT_AND_GOP_DATA *) NULL,
  0,
  (TEXT_OUT_SPLITTER_QUERY_DATA *) NULL,
  0,
  (INT32 *) NULL,

  EfiConsoleControlScreenText,
  0,
  0,
  (CHAR16 *) NULL,
  (INT32 *) NULL
};

EFI_DRIVER_BINDING_PROTOCOL           gConSplitterConInDriverBinding = {
  ConSplitterConInDriverBindingSupported,
  ConSplitterConInDriverBindingStart,
  ConSplitterConInDriverBindingStop,
  0xa,
  NULL,
  NULL
};

EFI_DRIVER_BINDING_PROTOCOL           gConSplitterSimplePointerDriverBinding = {
  ConSplitterSimplePointerDriverBindingSupported,
  ConSplitterSimplePointerDriverBindingStart,
  ConSplitterSimplePointerDriverBindingStop,
  0xa,
  NULL,
  NULL
};

//
// Driver binding instance for Absolute Pointer protocol
//
EFI_DRIVER_BINDING_PROTOCOL           gConSplitterAbsolutePointerDriverBinding = {
  ConSplitterAbsolutePointerDriverBindingSupported,
  ConSplitterAbsolutePointerDriverBindingStart,
  ConSplitterAbsolutePointerDriverBindingStop,
  0xa,
  NULL,
  NULL
};

EFI_DRIVER_BINDING_PROTOCOL           gConSplitterConOutDriverBinding = {
  ConSplitterConOutDriverBindingSupported,
  ConSplitterConOutDriverBindingStart,
  ConSplitterConOutDriverBindingStop,
  0xa,
  NULL,
  NULL
};

EFI_DRIVER_BINDING_PROTOCOL           gConSplitterStdErrDriverBinding = {
  ConSplitterStdErrDriverBindingSupported,
  ConSplitterStdErrDriverBindingStart,
  ConSplitterStdErrDriverBindingStop,
  0xa,
  NULL,
  NULL
};

/**
  The user Entry Point for module ConSplitter. The user code starts with this function.

  @param[in] ImageHandle    The firmware allocated handle for the EFI image.
  @param[in] SystemTable    A pointer to the EFI System Table.

  @retval EFI_SUCCESS       The entry point is executed successfully.
  @retval other             Some error occurs when executing this entry point.

**/
EFI_STATUS
EFIAPI
InitializeConSplitter(
  IN EFI_HANDLE           ImageHandle,
  IN EFI_SYSTEM_TABLE     *SystemTable
  )
{
  EFI_STATUS              Status;

  //
  // Install driver model protocol(s).
  //
  Status = EfiLibInstallDriverBindingComponentName2 (
             ImageHandle,
             SystemTable,
             &gConSplitterConInDriverBinding,
             ImageHandle,
             &gConSplitterConInComponentName,
             &gConSplitterConInComponentName2
             );
  ASSERT_EFI_ERROR (Status);

  Status = EfiLibInstallDriverBindingComponentName2 (
             ImageHandle,
             SystemTable,
             &gConSplitterSimplePointerDriverBinding,
             NULL,
             &gConSplitterSimplePointerComponentName,
             &gConSplitterSimplePointerComponentName2
             );
  ASSERT_EFI_ERROR (Status);

  Status = EfiLibInstallDriverBindingComponentName2 (
             ImageHandle,
             SystemTable,
             &gConSplitterAbsolutePointerDriverBinding,
             NULL,
             &gConSplitterAbsolutePointerComponentName,
             &gConSplitterAbsolutePointerComponentName2
             );
  ASSERT_EFI_ERROR (Status);

  Status = EfiLibInstallDriverBindingComponentName2 (
             ImageHandle,
             SystemTable,
             &gConSplitterConOutDriverBinding,
             NULL,
             &gConSplitterConOutComponentName,
             &gConSplitterConOutComponentName2
             );
  ASSERT_EFI_ERROR (Status);

  Status = EfiLibInstallDriverBindingComponentName2 (
             ImageHandle,
             SystemTable,
             &gConSplitterStdErrDriverBinding,
             NULL,
             &gConSplitterStdErrComponentName,
             &gConSplitterStdErrComponentName2
             );
  ASSERT_EFI_ERROR (Status);


  //
  // Call the original Entry Point
  //
  Status = ConSplitterDriverEntry (ImageHandle, SystemTable);

  return Status;
}


EFI_STATUS
EFIAPI
ConSplitterDriverEntry (
  IN EFI_HANDLE                       ImageHandle,
  IN EFI_SYSTEM_TABLE                 *SystemTable
  )
/*++

Routine Description:
  Intialize a virtual console device to act as an agrigator of physical console
  devices.

Arguments:
  ImageHandle - (Standard EFI Image entry - EFI_IMAGE_ENTRY_POINT)
  SystemTable - (Standard EFI Image entry - EFI_IMAGE_ENTRY_POINT)
Returns:
  EFI_SUCCESS

--*/
{
  EFI_STATUS  Status;

  ASSERT (FeaturePcdGet (PcdConOutGopSupport) ||
          FeaturePcdGet (PcdConOutUgaSupport));
  //
  // The driver creates virtual handles for ConIn, ConOut, and StdErr.
  // The virtual handles will always exist even if no console exist in the
  // system. This is need to support hotplug devices like USB.
  //
  //
  // Create virtual device handle for StdErr Splitter
  //
  Status = ConSplitterTextOutConstructor (&mStdErr);
  if (!EFI_ERROR (Status)) {
    Status = gBS->InstallMultipleProtocolInterfaces (
                    &mStdErr.VirtualHandle,
                    &gEfiSimpleTextOutProtocolGuid,
                    &mStdErr.TextOut,
                    &gEfiPrimaryStandardErrorDeviceGuid,
                    NULL,
                    NULL
                    );
  }
  //
  // Create virtual device handle for ConIn Splitter
  //
  Status = ConSplitterTextInConstructor (&mConIn);
  if (!EFI_ERROR (Status)) {
    Status = gBS->InstallMultipleProtocolInterfaces (
                    &mConIn.VirtualHandle,
                    &gEfiSimpleTextInProtocolGuid,
                    &mConIn.TextIn,
                    &gEfiSimpleTextInputExProtocolGuid,
                    &mConIn.TextInEx,
                    &gEfiSimplePointerProtocolGuid,
                    &mConIn.SimplePointer,
                    &gEfiAbsolutePointerProtocolGuid,
                    &mConIn.AbsolutePointer,
                    &gEfiPrimaryConsoleInDeviceGuid,
                    NULL,
                    NULL
                    );
    if (!EFI_ERROR (Status)) {
      //
      // Update the EFI System Table with new virtual console
      //
      gST->ConsoleInHandle  = mConIn.VirtualHandle;
      gST->ConIn            = &mConIn.TextIn;
    }
  }
  //
  // Create virtual device handle for ConOut Splitter
  //
  Status = ConSplitterTextOutConstructor (&mConOut);
  if (!EFI_ERROR (Status)) {
    if (!FeaturePcdGet (PcdConOutGopSupport)) {
      //
      // In EFI mode, UGA Draw protocol is installed
      //
      Status = gBS->InstallMultipleProtocolInterfaces (
                      &mConOut.VirtualHandle,
                      &gEfiSimpleTextOutProtocolGuid,
                      &mConOut.TextOut,
                      &gEfiUgaDrawProtocolGuid,
                      &mConOut.UgaDraw,
                      &gEfiConsoleControlProtocolGuid,
                      &mConOut.ConsoleControl,
                      &gEfiPrimaryConsoleOutDeviceGuid,
                      NULL,
                      NULL
                      );
    } else if (!FeaturePcdGet (PcdConOutUgaSupport)) {
      //
      // In UEFI mode, Graphics Output Protocol is installed on virtual handle.
      //
      Status = gBS->InstallMultipleProtocolInterfaces (
                      &mConOut.VirtualHandle,
                      &gEfiSimpleTextOutProtocolGuid,
                      &mConOut.TextOut,
                      &gEfiGraphicsOutputProtocolGuid,
                      &mConOut.GraphicsOutput,
                      &gEfiConsoleControlProtocolGuid,
                      &mConOut.ConsoleControl,
                      &gEfiPrimaryConsoleOutDeviceGuid,
                      NULL,
                      NULL
                      );
    } else {
      //
      // In EFI and UEFI comptible mode, Graphics Output Protocol and UGA are
      // installed on virtual handle.
      //
      Status = gBS->InstallMultipleProtocolInterfaces (
                      &mConOut.VirtualHandle,
                      &gEfiSimpleTextOutProtocolGuid,
                      &mConOut.TextOut,
                      &gEfiGraphicsOutputProtocolGuid,
                      &mConOut.GraphicsOutput,
                      &gEfiUgaDrawProtocolGuid,
                      &mConOut.UgaDraw,
                      &gEfiConsoleControlProtocolGuid,
                      &mConOut.ConsoleControl,
                      &gEfiPrimaryConsoleOutDeviceGuid,
                      NULL,
                      NULL
                      );
    }

    if (!EFI_ERROR (Status)) {
      //
      // Update the EFI System Table with new virtual console
      //
      gST->ConsoleOutHandle = mConOut.VirtualHandle;
      gST->ConOut           = &mConOut.TextOut;
    }

  }
  //
  // Update the CRC32 in the EFI System Table header
  //
  gST->Hdr.CRC32 = 0;
  gBS->CalculateCrc32 (
        (UINT8 *) &gST->Hdr,
        gST->Hdr.HeaderSize,
        &gST->Hdr.CRC32
        );

  return EFI_SUCCESS;
}

EFI_STATUS
ConSplitterTextInConstructor (
  TEXT_IN_SPLITTER_PRIVATE_DATA       *ConInPrivate
  )
/*++

Routine Description:

  Construct the ConSplitter.

Arguments:

  ConInPrivate    - A pointer to the TEXT_IN_SPLITTER_PRIVATE_DATA structure.

Returns:
  EFI_OUT_OF_RESOURCES - Out of resources.

--*/
{
  EFI_STATUS  Status;

  //
  // Initilize console input splitter's private data.
  //
  Status = ConSplitterGrowBuffer (
            sizeof (EFI_SIMPLE_TEXT_INPUT_PROTOCOL *),
            &ConInPrivate->TextInListCount,
            (VOID **) &ConInPrivate->TextInList
            );
  if (EFI_ERROR (Status)) {
    return EFI_OUT_OF_RESOURCES;
  }
  //
  // Create Event to support locking StdIn Device
  //
  Status = gBS->CreateEvent (
                  EVT_TIMER | EVT_NOTIFY_SIGNAL,
                  TPL_CALLBACK,
                  ConSpliterConsoleControlLockStdInEvent,
                  NULL,
                  &ConInPrivate->LockEvent
                  );
  ASSERT_EFI_ERROR (Status);

  Status = gBS->CreateEvent (
                  EVT_NOTIFY_WAIT,
                  TPL_NOTIFY,
                  ConSplitterTextInWaitForKey,
                  ConInPrivate,
                  &ConInPrivate->TextIn.WaitForKey
                  );
  ASSERT_EFI_ERROR (Status);

  //
  // Buffer for Simple Text Input Ex Protocol
  //
  Status = ConSplitterGrowBuffer (
             sizeof (EFI_SIMPLE_TEXT_INPUT_EX_PROTOCOL *),
             &ConInPrivate->TextInExListCount,
             (VOID **) &ConInPrivate->TextInExList
             );
  if (EFI_ERROR (Status)) {
    return EFI_OUT_OF_RESOURCES;
  }

  Status = gBS->CreateEvent (
                  EVT_NOTIFY_WAIT,
                  TPL_NOTIFY,
                  ConSplitterTextInWaitForKey,
                  ConInPrivate,
                  &ConInPrivate->TextInEx.WaitForKeyEx
                  );
  ASSERT_EFI_ERROR (Status);

  InitializeListHead (&ConInPrivate->NotifyList);

  //
  // Allocate Buffer and Create Event for Absolute Pointer and Simple Pointer Protocols
  //
  ConInPrivate->AbsolutePointer.Mode = &ConInPrivate->AbsolutePointerMode;

  Status = ConSplitterGrowBuffer (
            sizeof (EFI_ABSOLUTE_POINTER_PROTOCOL *),
            &ConInPrivate->AbsolutePointerListCount,
            (VOID **) &ConInPrivate->AbsolutePointerList
            );
  if (EFI_ERROR (Status)) {
    return EFI_OUT_OF_RESOURCES;
  }

  Status = gBS->CreateEvent (
            EVT_NOTIFY_WAIT,
            TPL_NOTIFY,
            ConSplitterAbsolutePointerWaitForInput,
            ConInPrivate,
            &ConInPrivate->AbsolutePointer.WaitForInput
        );
  ASSERT_EFI_ERROR (Status);

  ConInPrivate->SimplePointer.Mode = &ConInPrivate->SimplePointerMode;

  Status = ConSplitterGrowBuffer (
            sizeof (EFI_SIMPLE_POINTER_PROTOCOL *),
            &ConInPrivate->PointerListCount,
            (VOID **) &ConInPrivate->PointerList
            );
  if (EFI_ERROR (Status)) {
    return EFI_OUT_OF_RESOURCES;
  }

  Status = gBS->CreateEvent (
                  EVT_NOTIFY_WAIT,
                  TPL_NOTIFY,
                  ConSplitterSimplePointerWaitForInput,
                  ConInPrivate,
                  &ConInPrivate->SimplePointer.WaitForInput
                  );

  return Status;
}

EFI_STATUS
ConSplitterTextOutConstructor (
  TEXT_OUT_SPLITTER_PRIVATE_DATA      *ConOutPrivate
  )
{
  EFI_STATUS  Status;
  EFI_GRAPHICS_OUTPUT_MODE_INFORMATION  *Info;

  //
  // Copy protocols template
  //
  if (FeaturePcdGet (PcdConOutUgaSupport)) {
    CopyMem (&ConOutPrivate->UgaDraw, &gUgaDrawProtocolTemplate, sizeof (EFI_UGA_DRAW_PROTOCOL));
  }

  if (FeaturePcdGet (PcdConOutGopSupport)) {
    CopyMem (&ConOutPrivate->GraphicsOutput, &gGraphicsOutputProtocolTemplate, sizeof (EFI_GRAPHICS_OUTPUT_PROTOCOL));
  }

  //
  // Initilize console output splitter's private data.
  //
  ConOutPrivate->TextOut.Mode = &ConOutPrivate->TextOutMode;

  //
  // When new console device is added, the new mode will be set later,
  // so put current mode back to init state.
  //
  ConOutPrivate->TextOutMode.Mode = 0xFF;

  Status = ConSplitterGrowBuffer (
            sizeof (TEXT_OUT_AND_GOP_DATA),
            &ConOutPrivate->TextOutListCount,
            (VOID **) &ConOutPrivate->TextOutList
            );
  if (EFI_ERROR (Status)) {
    return EFI_OUT_OF_RESOURCES;
  }

  Status = ConSplitterGrowBuffer (
            sizeof (TEXT_OUT_SPLITTER_QUERY_DATA),
            &ConOutPrivate->TextOutQueryDataCount,
            (VOID **) &ConOutPrivate->TextOutQueryData
            );
  if (EFI_ERROR (Status)) {
    return EFI_OUT_OF_RESOURCES;
  }
  //
  // Setup the DevNullTextOut console to 80 x 25
  //
  ConOutPrivate->TextOutQueryData[0].Columns  = 80;
  ConOutPrivate->TextOutQueryData[0].Rows     = 25;
  DevNullTextOutSetMode (ConOutPrivate, 0);

  if (FeaturePcdGet (PcdConOutUgaSupport)) {
    //
    // Setup the DevNullUgaDraw to 800 x 600 x 32 bits per pixel
    //
    ConSpliterUgaDrawSetMode (&ConOutPrivate->UgaDraw, 800, 600, 32, 60);
  }
  if (FeaturePcdGet (PcdConOutGopSupport)) {
    //
    // Setup resource for mode information in Graphics Output Protocol interface
    //
    if ((ConOutPrivate->GraphicsOutput.Mode = AllocateZeroPool (sizeof (EFI_GRAPHICS_OUTPUT_PROTOCOL_MODE))) == NULL) {
      return EFI_OUT_OF_RESOURCES;
    }
    if ((ConOutPrivate->GraphicsOutput.Mode->Info = AllocateZeroPool (sizeof (EFI_GRAPHICS_OUTPUT_MODE_INFORMATION))) == NULL) {
      return EFI_OUT_OF_RESOURCES;
    }
    //
    // Setup the DevNullGraphicsOutput to 800 x 600 x 32 bits per pixel
    // DevNull will be updated to user-defined mode after driver has started.
    //
    if ((ConOutPrivate->GraphicsOutputModeBuffer = AllocateZeroPool (sizeof (EFI_GRAPHICS_OUTPUT_MODE_INFORMATION))) == NULL) {
      return EFI_OUT_OF_RESOURCES;
    }
    Info = &ConOutPrivate->GraphicsOutputModeBuffer[0];
    Info->Version = 0;
    Info->HorizontalResolution = 800;
    Info->VerticalResolution = 600;
    Info->PixelFormat = PixelBltOnly;
    Info->PixelsPerScanLine = 800;
    CopyMem (ConOutPrivate->GraphicsOutput.Mode->Info, Info, sizeof (EFI_GRAPHICS_OUTPUT_MODE_INFORMATION));
    ConOutPrivate->GraphicsOutput.Mode->SizeOfInfo = sizeof (EFI_GRAPHICS_OUTPUT_MODE_INFORMATION);

    //
    // Initialize the following items, theset items remain unchanged in GraphicsOutput->SetMode()
    // GraphicsOutputMode->FrameBufferBase, GraphicsOutputMode->FrameBufferSize
    //
    ConOutPrivate->GraphicsOutput.Mode->FrameBufferBase = (EFI_PHYSICAL_ADDRESS) NULL;
    ConOutPrivate->GraphicsOutput.Mode->FrameBufferSize = 0;

    ConOutPrivate->GraphicsOutput.Mode->MaxMode = 1;
    //
    // Initial current mode to unknow state, and then set to mode 0
    //
    ConOutPrivate->GraphicsOutput.Mode->Mode = 0xffff;
    ConOutPrivate->GraphicsOutput.SetMode (&ConOutPrivate->GraphicsOutput, 0);
  }

  return Status;
}

STATIC
EFI_STATUS
ConSplitterSupported (
  IN  EFI_DRIVER_BINDING_PROTOCOL     *This,
  IN  EFI_HANDLE                      ControllerHandle,
  IN  EFI_GUID                        *Guid
  )
/*++

Routine Description:
  Generic Supported Check

Arguments:
  This              - Pointer to protocol.
  ControllerHandle  - Controller Handle.
  Guid              - Guid.

Returns:

  EFI_UNSUPPORTED - unsupported.
  EFI_SUCCESS     - operation is OK.

--*/
{
  EFI_STATUS  Status;
  VOID        *Instance;

  //
  // Make sure the Console Splitter does not attempt to attach to itself
  //
  if (ControllerHandle == mConIn.VirtualHandle) {
    return EFI_UNSUPPORTED;
  }

  if (ControllerHandle == mConOut.VirtualHandle) {
    return EFI_UNSUPPORTED;
  }

  if (ControllerHandle == mStdErr.VirtualHandle) {
    return EFI_UNSUPPORTED;
  }
  //
  // Check to see whether the handle has the ConsoleInDevice GUID on it
  //
  Status = gBS->OpenProtocol (
                  ControllerHandle,
                  Guid,
                  &Instance,
                  This->DriverBindingHandle,
                  ControllerHandle,
                  EFI_OPEN_PROTOCOL_BY_DRIVER
                  );

  if (EFI_ERROR (Status)) {
    return Status;
  }

  gBS->CloseProtocol (
        ControllerHandle,
        Guid,
        This->DriverBindingHandle,
        ControllerHandle
        );

  return EFI_SUCCESS;
}

EFI_STATUS
EFIAPI
ConSplitterConInDriverBindingSupported (
  IN  EFI_DRIVER_BINDING_PROTOCOL     *This,
  IN  EFI_HANDLE                      ControllerHandle,
  IN  EFI_DEVICE_PATH_PROTOCOL        *RemainingDevicePath
  )
/*++

Routine Description:
  Console In Supported Check

Arguments:
  This              - Pointer to protocol.
  ControllerHandle  - Controller handle.
  RemainingDevicePath  - Remaining device path.

Returns:

  EFI_STATUS

--*/
{
  return ConSplitterSupported (
          This,
          ControllerHandle,
          &gEfiConsoleInDeviceGuid
          );
}

EFI_STATUS
EFIAPI
ConSplitterSimplePointerDriverBindingSupported (
  IN  EFI_DRIVER_BINDING_PROTOCOL     *This,
  IN  EFI_HANDLE                      ControllerHandle,
  IN  EFI_DEVICE_PATH_PROTOCOL        *RemainingDevicePath
  )
/*++

Routine Description:
  Standard Error Supported Check

Arguments:
  This              - Pointer to protocol.
  ControllerHandle  - Controller handle.
  RemainingDevicePath  - Remaining device path.

Returns:

  EFI_STATUS

--*/
{
  return ConSplitterSupported (
          This,
          ControllerHandle,
          &gEfiSimplePointerProtocolGuid
          );
}

EFI_STATUS
EFIAPI
ConSplitterAbsolutePointerDriverBindingSupported (
  IN  EFI_DRIVER_BINDING_PROTOCOL     *This,
  IN  EFI_HANDLE                      ControllerHandle,
  IN  EFI_DEVICE_PATH_PROTOCOL        *RemainingDevicePath
  )
/*++

Routine Description:
  Absolute Pointer Supported Check

Arguments:
  This              - Pointer to protocol.
  ControllerHandle  - Controller handle.
  RemainingDevicePath  - Remaining device path.

Returns:

  EFI_STATUS

--*/
{
  return ConSplitterSupported (
          This,
          ControllerHandle,
          &gEfiAbsolutePointerProtocolGuid
          );
}

EFI_STATUS
EFIAPI
ConSplitterConOutDriverBindingSupported (
  IN  EFI_DRIVER_BINDING_PROTOCOL     *This,
  IN  EFI_HANDLE                      ControllerHandle,
  IN  EFI_DEVICE_PATH_PROTOCOL        *RemainingDevicePath
  )
/*++

Routine Description:
  Console Out Supported Check

Arguments:
  This              - Pointer to protocol.
  ControllerHandle  - Controller handle.
  RemainingDevicePath  - Remaining device path.

Returns:

  EFI_STATUS

--*/
{
  return ConSplitterSupported (
          This,
          ControllerHandle,
          &gEfiConsoleOutDeviceGuid
          );
}

EFI_STATUS
EFIAPI
ConSplitterStdErrDriverBindingSupported (
  IN  EFI_DRIVER_BINDING_PROTOCOL     *This,
  IN  EFI_HANDLE                      ControllerHandle,
  IN  EFI_DEVICE_PATH_PROTOCOL        *RemainingDevicePath
  )
/*++

Routine Description:
  Standard Error Supported Check

Arguments:
  This              - Pointer to protocol.
  ControllerHandle  - Controller handle.
  RemainingDevicePath  - Remaining device path.

Returns:

  EFI_STATUS

--*/
{
  return ConSplitterSupported (
          This,
          ControllerHandle,
          &gEfiStandardErrorDeviceGuid
          );
}

STATIC
EFI_STATUS
EFIAPI
ConSplitterStart (
  IN  EFI_DRIVER_BINDING_PROTOCOL     *This,
  IN  EFI_HANDLE                      ControllerHandle,
  IN  EFI_HANDLE                      ConSplitterVirtualHandle,
  IN  EFI_GUID                        *DeviceGuid,
  IN  EFI_GUID                        *InterfaceGuid,
  IN  VOID                            **Interface
  )
/*++

Routine Description:
  Start ConSplitter on ControllerHandle, and create the virtual
  agrogated console device on first call Start for a SimpleTextIn handle.

Arguments:
  (Standard DriverBinding Protocol Start() function)

Returns:
  EFI_ERROR if a SimpleTextIn protocol is not started.

--*/
{
  EFI_STATUS  Status;
  VOID        *Instance;

  //
  // Check to see whether the handle has the ConsoleInDevice GUID on it
  //
  Status = gBS->OpenProtocol (
                  ControllerHandle,
                  DeviceGuid,
                  &Instance,
                  This->DriverBindingHandle,
                  ControllerHandle,
                  EFI_OPEN_PROTOCOL_BY_DRIVER
                  );
  if (EFI_ERROR (Status)) {
    return Status;
  }

  Status = gBS->OpenProtocol (
                  ControllerHandle,
                  DeviceGuid,
                  &Instance,
                  This->DriverBindingHandle,
                  ConSplitterVirtualHandle,
                  EFI_OPEN_PROTOCOL_BY_CHILD_CONTROLLER
                  );
  if (EFI_ERROR (Status)) {
    return Status;
  }

  return gBS->OpenProtocol (
                ControllerHandle,
                InterfaceGuid,
                Interface,
                This->DriverBindingHandle,
                ConSplitterVirtualHandle,
                EFI_OPEN_PROTOCOL_GET_PROTOCOL
                );
}

EFI_STATUS
EFIAPI
ConSplitterConInDriverBindingStart (
  IN  EFI_DRIVER_BINDING_PROTOCOL     *This,
  IN  EFI_HANDLE                      ControllerHandle,
  IN  EFI_DEVICE_PATH_PROTOCOL        *RemainingDevicePath
  )
/*++

Routine Description:
  Start ConSplitter on ControllerHandle, and create the virtual
  agrogated console device on first call Start for a SimpleTextIn handle.

Arguments:
  This              - Pointer to protocol.
  ControllerHandle  - Controller handle.
  RemainingDevicePath  - Remaining device path.

Returns:

  EFI_STATUS
  EFI_ERROR if a SimpleTextIn protocol is not started.

--*/
{
  EFI_STATUS                     Status;
  EFI_SIMPLE_TEXT_INPUT_PROTOCOL *TextIn;
  EFI_SIMPLE_TEXT_INPUT_EX_PROTOCOL *TextInEx;

  //
  // Start ConSplitter on ControllerHandle, and create the virtual
  // agrogated console device on first call Start for a SimpleTextIn handle.
  //
  Status = ConSplitterStart (
            This,
            ControllerHandle,
            mConIn.VirtualHandle,
            &gEfiConsoleInDeviceGuid,
            &gEfiSimpleTextInProtocolGuid,
            (VOID **) &TextIn
            );
  if (EFI_ERROR (Status)) {
    return Status;
  }

  Status = ConSplitterTextInAddDevice (&mConIn, TextIn);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  Status = gBS->OpenProtocol (
                  ControllerHandle,
                  &gEfiSimpleTextInputExProtocolGuid,
                  (VOID **) &TextInEx,
                  This->DriverBindingHandle,
                  mConIn.VirtualHandle,
                  EFI_OPEN_PROTOCOL_GET_PROTOCOL
                  );
  if (EFI_ERROR (Status)) {
    return Status;
  }

  Status = ConSplitterTextInExAddDevice (&mConIn, TextInEx);

  return Status;
}

EFI_STATUS
EFIAPI
ConSplitterSimplePointerDriverBindingStart (
  IN  EFI_DRIVER_BINDING_PROTOCOL     *This,
  IN  EFI_HANDLE                      ControllerHandle,
  IN  EFI_DEVICE_PATH_PROTOCOL        *RemainingDevicePath
  )
/*++

Routine Description:
  Start ConSplitter on ControllerHandle, and create the virtual
  agrogated console device on first call Start for a SimpleTextIn handle.

Arguments:
  This              - Pointer to protocol.
  ControllerHandle  - Controller handle.
  RemainingDevicePath  - Remaining device path.

Returns:

  EFI_ERROR if a SimpleTextIn protocol is not started.

--*/
{
  EFI_STATUS                  Status;
  EFI_SIMPLE_POINTER_PROTOCOL *SimplePointer;

  Status = ConSplitterStart (
            This,
            ControllerHandle,
            mConIn.VirtualHandle,
            &gEfiSimplePointerProtocolGuid,
            &gEfiSimplePointerProtocolGuid,
            (VOID **) &SimplePointer
            );
  if (EFI_ERROR (Status)) {
    return Status;
  }

  return ConSplitterSimplePointerAddDevice (&mConIn, SimplePointer);
}

EFI_STATUS
EFIAPI
ConSplitterAbsolutePointerDriverBindingStart (
  IN  EFI_DRIVER_BINDING_PROTOCOL     *This,
  IN  EFI_HANDLE                      ControllerHandle,
  IN  EFI_DEVICE_PATH_PROTOCOL        *RemainingDevicePath
  )
/*++

Routine Description:
  Start ConSplitter on ControllerHandle, and create the virtual
  agrogated console device on first call Start for a ConIn handle.

Arguments:
  This                 - Pointer to protocol.
  ControllerHandle     - Controller handle.
  RemainingDevicePath  - Remaining device path.

Returns:

  EFI_ERROR if a AbsolutePointer protocol is not started.

--*/
{
  EFI_STATUS                        Status;
  EFI_ABSOLUTE_POINTER_PROTOCOL     *AbsolutePointer;

  Status = ConSplitterStart (
             This,
             ControllerHandle,
             mConIn.VirtualHandle,
             &gEfiAbsolutePointerProtocolGuid,
             &gEfiAbsolutePointerProtocolGuid,
             (VOID **) &AbsolutePointer
             );

  if (EFI_ERROR (Status)) {
    return Status;
  }

  return ConSplitterAbsolutePointerAddDevice (&mConIn, AbsolutePointer);
}

EFI_STATUS
EFIAPI
ConSplitterConOutDriverBindingStart (
  IN  EFI_DRIVER_BINDING_PROTOCOL     *This,
  IN  EFI_HANDLE                      ControllerHandle,
  IN  EFI_DEVICE_PATH_PROTOCOL        *RemainingDevicePath
  )
/*++

Routine Description:
  Start ConSplitter on ControllerHandle, and create the virtual
  agrogated console device on first call Start for a SimpleTextIn handle.

Arguments:
  This              - Pointer to protocol.
  ControllerHandle  - Controller handle.
  RemainingDevicePath  - Remaining device path.

Returns:
  EFI_ERROR if a SimpleTextIn protocol is not started.

--*/
{
  EFI_STATUS                       Status;
  EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL  *TextOut;
  EFI_GRAPHICS_OUTPUT_PROTOCOL     *GraphicsOutput;
  EFI_UGA_DRAW_PROTOCOL            *UgaDraw;

  Status = ConSplitterStart (
            This,
            ControllerHandle,
            mConOut.VirtualHandle,
            &gEfiConsoleOutDeviceGuid,
            &gEfiSimpleTextOutProtocolGuid,
            (VOID **) &TextOut
            );
  if (EFI_ERROR (Status)) {
    return Status;
  }

  GraphicsOutput = NULL;
  UgaDraw        = NULL;
  //
  // Try to Open Graphics Output protocol
  //
  Status = gBS->OpenProtocol (
                  ControllerHandle,
                  &gEfiGraphicsOutputProtocolGuid,
                  (VOID **) &GraphicsOutput,
                  This->DriverBindingHandle,
                  mConOut.VirtualHandle,
                  EFI_OPEN_PROTOCOL_GET_PROTOCOL
                  );

  if (EFI_ERROR (Status) && FeaturePcdGet (PcdUgaConsumeSupport)) {
    //
    // Open UGA_DRAW protocol
    //
    Status = gBS->OpenProtocol (
                    ControllerHandle,
                    &gEfiUgaDrawProtocolGuid,
                    (VOID **) &UgaDraw,
                    This->DriverBindingHandle,
                    mConOut.VirtualHandle,
                    EFI_OPEN_PROTOCOL_GET_PROTOCOL
                    );
  }

  //
  // When new console device is added, the new mode will be set later,
  // so put current mode back to init state.
  //
  mConOut.TextOutMode.Mode = 0xFF;

  //
  // If both ConOut and StdErr incorporate the same Text Out device,
  // their MaxMode and QueryData should be the intersection of both.
  //
  Status = ConSplitterTextOutAddDevice (&mConOut, TextOut, GraphicsOutput, UgaDraw);
  ConSplitterTextOutSetAttribute (&mConOut.TextOut, EFI_TEXT_ATTR (EFI_LIGHTGRAY, EFI_BLACK));

  if (FeaturePcdGet (PcdConOutUgaSupport) && FeaturePcdGet (PcdUgaConsumeSupport)) {
    //
    // Match the UGA mode data of ConOut with the current mode
    //
    if (UgaDraw != NULL) {
      UgaDraw->GetMode (
                 UgaDraw,
                 &mConOut.UgaHorizontalResolution,
                 &mConOut.UgaVerticalResolution,
                 &mConOut.UgaColorDepth,
                 &mConOut.UgaRefreshRate
                 );
    }
  }
  return Status;
}

EFI_STATUS
EFIAPI
ConSplitterStdErrDriverBindingStart (
  IN  EFI_DRIVER_BINDING_PROTOCOL     *This,
  IN  EFI_HANDLE                      ControllerHandle,
  IN  EFI_DEVICE_PATH_PROTOCOL        *RemainingDevicePath
  )
/*++

Routine Description:
  Start ConSplitter on ControllerHandle, and create the virtual
  agrogated console device on first call Start for a SimpleTextIn handle.

Arguments:
  This              - Pointer to protocol.
  ControllerHandle  - Controller handle.
  RemainingDevicePath  - Remaining device path.

Returns:
  EFI_ERROR if a SimpleTextIn protocol is not started.

--*/
{
  EFI_STATUS                       Status;
  EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL  *TextOut;

  Status = ConSplitterStart (
            This,
            ControllerHandle,
            mStdErr.VirtualHandle,
            &gEfiStandardErrorDeviceGuid,
            &gEfiSimpleTextOutProtocolGuid,
            (VOID **) &TextOut
            );
  if (EFI_ERROR (Status)) {
    return Status;
  }

  //
  // When new console device is added, the new mode will be set later,
  // so put current mode back to init state.
  //
  mStdErr.TextOutMode.Mode = 0xFF;

  //
  // If both ConOut and StdErr incorporate the same Text Out device,
  // their MaxMode and QueryData should be the intersection of both.
  //
  Status = ConSplitterTextOutAddDevice (&mStdErr, TextOut, NULL, NULL);
  ConSplitterTextOutSetAttribute (&mStdErr.TextOut, EFI_TEXT_ATTR (EFI_MAGENTA, EFI_BLACK));
  if (EFI_ERROR (Status)) {
    return Status;
  }

  if (mStdErr.CurrentNumberOfConsoles == 1) {
    gST->StandardErrorHandle  = mStdErr.VirtualHandle;
    gST->StdErr               = &mStdErr.TextOut;
    //
    // Update the CRC32 in the EFI System Table header
    //
    gST->Hdr.CRC32 = 0;
    gBS->CalculateCrc32 (
          (UINT8 *) &gST->Hdr,
          gST->Hdr.HeaderSize,
          &gST->Hdr.CRC32
          );
  }

  return Status;
}

STATIC
EFI_STATUS
EFIAPI
ConSplitterStop (
  IN  EFI_DRIVER_BINDING_PROTOCOL     *This,
  IN  EFI_HANDLE                      ControllerHandle,
  IN  EFI_HANDLE                      ConSplitterVirtualHandle,
  IN  EFI_GUID                        *DeviceGuid,
  IN  EFI_GUID                        *InterfaceGuid,
  IN  VOID                            **Interface
  )
/*++

Routine Description:

Arguments:
  (Standard DriverBinding Protocol Stop() function)

Returns:

  None

--*/
{
  EFI_STATUS  Status;

  Status = gBS->OpenProtocol (
                  ControllerHandle,
                  InterfaceGuid,
                  Interface,
                  This->DriverBindingHandle,
                  ControllerHandle,
                  EFI_OPEN_PROTOCOL_GET_PROTOCOL
                  );
  if (EFI_ERROR (Status)) {
    return Status;
  }
  //
  // close the protocol refered.
  //
  gBS->CloseProtocol (
        ControllerHandle,
        DeviceGuid,
        This->DriverBindingHandle,
        ConSplitterVirtualHandle
        );
  gBS->CloseProtocol (
        ControllerHandle,
        DeviceGuid,
        This->DriverBindingHandle,
        ControllerHandle
        );

  return EFI_SUCCESS;
}

EFI_STATUS
EFIAPI
ConSplitterConInDriverBindingStop (
  IN  EFI_DRIVER_BINDING_PROTOCOL     *This,
  IN  EFI_HANDLE                      ControllerHandle,
  IN  UINTN                           NumberOfChildren,
  IN  EFI_HANDLE                      *ChildHandleBuffer
  )
/*++

Routine Description:

Arguments:
  (Standard DriverBinding Protocol Stop() function)

Returns:

  None

--*/
{
  EFI_STATUS                     Status;
  EFI_SIMPLE_TEXT_INPUT_PROTOCOL *TextIn;

  EFI_SIMPLE_TEXT_INPUT_EX_PROTOCOL *TextInEx;
  if (NumberOfChildren == 0) {
    return EFI_SUCCESS;
  }

  Status = gBS->OpenProtocol (
                  ControllerHandle,
                  &gEfiSimpleTextInputExProtocolGuid,
                  (VOID **) &TextInEx,
                  This->DriverBindingHandle,
                  ControllerHandle,
                  EFI_OPEN_PROTOCOL_GET_PROTOCOL
                  );
  if (EFI_ERROR (Status)) {
    return Status;
  }

  Status = ConSplitterTextInExDeleteDevice (&mConIn, TextInEx);
  if (EFI_ERROR (Status)) {
    return Status;
  }


  Status = ConSplitterStop (
            This,
            ControllerHandle,
            mConIn.VirtualHandle,
            &gEfiConsoleInDeviceGuid,
            &gEfiSimpleTextInProtocolGuid,
            (VOID **) &TextIn
            );
  if (EFI_ERROR (Status)) {
    return Status;
  }
  //
  // Delete this console input device's data structures.
  //
  return ConSplitterTextInDeleteDevice (&mConIn, TextIn);
}

EFI_STATUS
EFIAPI
ConSplitterSimplePointerDriverBindingStop (
  IN  EFI_DRIVER_BINDING_PROTOCOL     *This,
  IN  EFI_HANDLE                      ControllerHandle,
  IN  UINTN                           NumberOfChildren,
  IN  EFI_HANDLE                      *ChildHandleBuffer
  )
/*++

Routine Description:

Arguments:
  (Standard DriverBinding Protocol Stop() function)

Returns:

  None

--*/
{
  EFI_STATUS                  Status;
  EFI_SIMPLE_POINTER_PROTOCOL *SimplePointer;

  if (NumberOfChildren == 0) {
    return EFI_SUCCESS;
  }

  Status = ConSplitterStop (
            This,
            ControllerHandle,
            mConIn.VirtualHandle,
            &gEfiSimplePointerProtocolGuid,
            &gEfiSimplePointerProtocolGuid,
            (VOID **) &SimplePointer
            );
  if (EFI_ERROR (Status)) {
    return Status;
  }
  //
  // Delete this console input device's data structures.
  //
  return ConSplitterSimplePointerDeleteDevice (&mConIn, SimplePointer);
}

EFI_STATUS
EFIAPI
ConSplitterAbsolutePointerDriverBindingStop (
  IN  EFI_DRIVER_BINDING_PROTOCOL     *This,
  IN  EFI_HANDLE                      ControllerHandle,
  IN  UINTN                           NumberOfChildren,
  IN  EFI_HANDLE                      *ChildHandleBuffer
  )
/*++

Routine Description:

Arguments:
  (Standard DriverBinding Protocol Stop() function)

Returns:

  None

--*/
{
  EFI_STATUS                        Status;
  EFI_ABSOLUTE_POINTER_PROTOCOL     *AbsolutePointer;

  if (NumberOfChildren == 0) {
    return EFI_SUCCESS;
  }

  Status = ConSplitterStop (
             This,
             ControllerHandle,
             mConIn.VirtualHandle,
             &gEfiAbsolutePointerProtocolGuid,
             &gEfiAbsolutePointerProtocolGuid,
             (VOID **) &AbsolutePointer
             );
  if (EFI_ERROR (Status)) {
    return Status;
  }
  //
  // Delete this console input device's data structures.
  //
  return ConSplitterAbsolutePointerDeleteDevice (&mConIn, AbsolutePointer);
}

EFI_STATUS
EFIAPI
ConSplitterConOutDriverBindingStop (
  IN  EFI_DRIVER_BINDING_PROTOCOL     *This,
  IN  EFI_HANDLE                      ControllerHandle,
  IN  UINTN                           NumberOfChildren,
  IN  EFI_HANDLE                      *ChildHandleBuffer
  )
/*++

Routine Description:

Arguments:
  (Standard DriverBinding Protocol Stop() function)

Returns:

  None

--*/
{
  EFI_STATUS                       Status;
  EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL  *TextOut;

  if (NumberOfChildren == 0) {
    return EFI_SUCCESS;
  }

  Status = ConSplitterStop (
            This,
            ControllerHandle,
            mConOut.VirtualHandle,
            &gEfiConsoleOutDeviceGuid,
            &gEfiSimpleTextOutProtocolGuid,
            (VOID **) &TextOut
            );
  if (EFI_ERROR (Status)) {
    return Status;
  }

  //
  // Delete this console output device's data structures.
  //
  return ConSplitterTextOutDeleteDevice (&mConOut, TextOut);
}

EFI_STATUS
EFIAPI
ConSplitterStdErrDriverBindingStop (
  IN  EFI_DRIVER_BINDING_PROTOCOL     *This,
  IN  EFI_HANDLE                      ControllerHandle,
  IN  UINTN                           NumberOfChildren,
  IN  EFI_HANDLE                      *ChildHandleBuffer
  )
/*++

Routine Description:

Arguments:
  (Standard DriverBinding Protocol Stop() function)

Returns:

  EFI_SUCCESS - Complete successfully.

--*/
{
  EFI_STATUS                       Status;
  EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL  *TextOut;

  if (NumberOfChildren == 0) {
    return EFI_SUCCESS;
  }

  Status = ConSplitterStop (
            This,
            ControllerHandle,
            mStdErr.VirtualHandle,
            &gEfiStandardErrorDeviceGuid,
            &gEfiSimpleTextOutProtocolGuid,
            (VOID **) &TextOut
            );
  if (EFI_ERROR (Status)) {
    return Status;
  }
  //
  // Delete this console error out device's data structures.
  //
  Status = ConSplitterTextOutDeleteDevice (&mStdErr, TextOut);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  if (mStdErr.CurrentNumberOfConsoles == 0) {
    gST->StandardErrorHandle  = NULL;
    gST->StdErr               = NULL;
    //
    // Update the CRC32 in the EFI System Table header
    //
    gST->Hdr.CRC32 = 0;
    gBS->CalculateCrc32 (
          (UINT8 *) &gST->Hdr,
          gST->Hdr.HeaderSize,
          &gST->Hdr.CRC32
          );
  }

  return Status;
}

EFI_STATUS
ConSplitterGrowBuffer (
  IN  UINTN                           SizeOfCount,
  IN  UINTN                           *Count,
  IN OUT  VOID                        **Buffer
  )
/*++

Routine Description:
  Take the passed in Buffer of size SizeOfCount and grow the buffer
  by MAX (CONSOLE_SPLITTER_CONSOLES_ALLOC_UNIT, MaxGrow) * SizeOfCount
  bytes. Copy the current data in Buffer to the new version of Buffer
  and free the old version of buffer.


Arguments:
  SizeOfCount - Size of element in array
  Count       - Current number of elements in array
  Buffer      - Bigger version of passed in Buffer with all the data

Returns:
  EFI_SUCCESS - Buffer size has grown
  EFI_OUT_OF_RESOURCES - Could not grow the buffer size

  None

--*/
{
  UINTN NewSize;
  UINTN OldSize;
  VOID  *Ptr;

  //
  // grow the buffer to new buffer size,
  // copy the old buffer's content to the new-size buffer,
  // then free the old buffer.
  //
  OldSize = *Count * SizeOfCount;
  *Count += CONSOLE_SPLITTER_CONSOLES_ALLOC_UNIT;
  NewSize = *Count * SizeOfCount;

  Ptr     = AllocateZeroPool (NewSize);
  if (Ptr == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }

  CopyMem (Ptr, *Buffer, OldSize);

  if (*Buffer != NULL) {
    FreePool (*Buffer);
  }

  *Buffer = Ptr;

  return EFI_SUCCESS;
}

EFI_STATUS
ConSplitterTextInAddDevice (
  IN  TEXT_IN_SPLITTER_PRIVATE_DATA   *Private,
  IN  EFI_SIMPLE_TEXT_INPUT_PROTOCOL  *TextIn
  )
/*++

Routine Description:

Arguments:

Returns:

  EFI_SUCCESS
  EFI_OUT_OF_RESOURCES

--*/
{
  EFI_STATUS  Status;

  //
  // If the Text In List is full, enlarge it by calling growbuffer().
  //
  if (Private->CurrentNumberOfConsoles >= Private->TextInListCount) {
    Status = ConSplitterGrowBuffer (
              sizeof (EFI_SIMPLE_TEXT_INPUT_PROTOCOL *),
              &Private->TextInListCount,
              (VOID **) &Private->TextInList
              );
    if (EFI_ERROR (Status)) {
      return EFI_OUT_OF_RESOURCES;
    }
  }
  //
  // Add the new text-in device data structure into the Text In List.
  //
  Private->TextInList[Private->CurrentNumberOfConsoles] = TextIn;
  Private->CurrentNumberOfConsoles++;

  //
  // Extra CheckEvent added to reduce the double CheckEvent() in UI.c
  //
  gBS->CheckEvent (TextIn->WaitForKey);

  return EFI_SUCCESS;
}

EFI_STATUS
ConSplitterTextInDeleteDevice (
  IN  TEXT_IN_SPLITTER_PRIVATE_DATA   *Private,
  IN  EFI_SIMPLE_TEXT_INPUT_PROTOCOL  *TextIn
  )
/*++

Routine Description:

Arguments:

Returns:

  EFI_SUCCESS
  EFI_NOT_FOUND

--*/
{
  UINTN Index;
  //
  // Remove the specified text-in device data structure from the Text In List,
  // and rearrange the remaining data structures in the Text In List.
  //
  for (Index = 0; Index < Private->CurrentNumberOfConsoles; Index++) {
    if (Private->TextInList[Index] == TextIn) {
      for (Index = Index; Index < Private->CurrentNumberOfConsoles - 1; Index++) {
        Private->TextInList[Index] = Private->TextInList[Index + 1];
      }

      Private->CurrentNumberOfConsoles--;
      return EFI_SUCCESS;
    }
  }

  return EFI_NOT_FOUND;
}

EFI_STATUS
ConSplitterTextInExAddDevice (
  IN  TEXT_IN_SPLITTER_PRIVATE_DATA         *Private,
  IN  EFI_SIMPLE_TEXT_INPUT_EX_PROTOCOL     *TextInEx
  )
{
  EFI_STATUS  Status;

  //
  // If the TextInEx List is full, enlarge it by calling growbuffer().
  //
  if (Private->CurrentNumberOfExConsoles >= Private->TextInExListCount) {
    Status = ConSplitterGrowBuffer (
              sizeof (EFI_SIMPLE_TEXT_INPUT_EX_PROTOCOL *),
              &Private->TextInExListCount,
              (VOID **) &Private->TextInExList
              );
    if (EFI_ERROR (Status)) {
      return EFI_OUT_OF_RESOURCES;
    }
  }
  //
  // Add the new text-in device data structure into the Text In List.
  //
  Private->TextInExList[Private->CurrentNumberOfExConsoles] = TextInEx;
  Private->CurrentNumberOfExConsoles++;

  //
  // Extra CheckEvent added to reduce the double CheckEvent() in UI.c
  //
  gBS->CheckEvent (TextInEx->WaitForKeyEx);

  return EFI_SUCCESS;
}

EFI_STATUS
ConSplitterTextInExDeleteDevice (
  IN  TEXT_IN_SPLITTER_PRIVATE_DATA         *Private,
  IN  EFI_SIMPLE_TEXT_INPUT_EX_PROTOCOL     *TextInEx
  )
{
  UINTN Index;
  //
  // Remove the specified text-in device data structure from the Text In List,
  // and rearrange the remaining data structures in the Text In List.
  //
  for (Index = 0; Index < Private->CurrentNumberOfExConsoles; Index++) {
    if (Private->TextInExList[Index] == TextInEx) {
      for (Index = Index; Index < Private->CurrentNumberOfExConsoles - 1; Index++) {
        Private->TextInExList[Index] = Private->TextInExList[Index + 1];
      }

      Private->CurrentNumberOfExConsoles--;
      return EFI_SUCCESS;
    }
  }

  return EFI_NOT_FOUND;
}

EFI_STATUS
ConSplitterSimplePointerAddDevice (
  IN  TEXT_IN_SPLITTER_PRIVATE_DATA   *Private,
  IN  EFI_SIMPLE_POINTER_PROTOCOL     *SimplePointer
  )
/*++

Routine Description:

Arguments:

Returns:

  EFI_OUT_OF_RESOURCES
  EFI_SUCCESS

--*/
{
  EFI_STATUS  Status;

  //
  // If the Text In List is full, enlarge it by calling growbuffer().
  //
  if (Private->CurrentNumberOfPointers >= Private->PointerListCount) {
    Status = ConSplitterGrowBuffer (
              sizeof (EFI_SIMPLE_POINTER_PROTOCOL *),
              &Private->PointerListCount,
              (VOID **) &Private->PointerList
              );
    if (EFI_ERROR (Status)) {
      return EFI_OUT_OF_RESOURCES;
    }
  }
  //
  // Add the new text-in device data structure into the Text In List.
  //
  Private->PointerList[Private->CurrentNumberOfPointers] = SimplePointer;
  Private->CurrentNumberOfPointers++;
  return EFI_SUCCESS;
}

EFI_STATUS
ConSplitterSimplePointerDeleteDevice (
  IN  TEXT_IN_SPLITTER_PRIVATE_DATA   *Private,
  IN  EFI_SIMPLE_POINTER_PROTOCOL     *SimplePointer
  )
/*++

Routine Description:

Arguments:

Returns:

  None

--*/
{
  UINTN Index;
  //
  // Remove the specified text-in device data structure from the Text In List,
  // and rearrange the remaining data structures in the Text In List.
  //
  for (Index = 0; Index < Private->CurrentNumberOfPointers; Index++) {
    if (Private->PointerList[Index] == SimplePointer) {
      for (Index = Index; Index < Private->CurrentNumberOfPointers - 1; Index++) {
        Private->PointerList[Index] = Private->PointerList[Index + 1];
      }

      Private->CurrentNumberOfPointers--;
      return EFI_SUCCESS;
    }
  }

  return EFI_NOT_FOUND;
}

EFI_STATUS
ConSplitterAbsolutePointerAddDevice (
  IN  TEXT_IN_SPLITTER_PRIVATE_DATA     *Private,
  IN  EFI_ABSOLUTE_POINTER_PROTOCOL     *AbsolutePointer
  )
/*++

Routine Description:

Arguments:

Returns:

  EFI_OUT_OF_RESOURCES
  EFI_SUCCESS

--*/
{
  EFI_STATUS  Status;

  //
  // If the Absolute Pointer List is full, enlarge it by calling growbuffer().
  //
  if (Private->CurrentNumberOfAbsolutePointers >= Private->AbsolutePointerListCount) {
    Status = ConSplitterGrowBuffer (
              sizeof (EFI_ABSOLUTE_POINTER_PROTOCOL *),
              &Private->AbsolutePointerListCount,
              (VOID **) &Private->AbsolutePointerList
              );
    if (EFI_ERROR (Status)) {
      return EFI_OUT_OF_RESOURCES;
    }
  }
  //
  // Add the new text-in device data structure into the Text In List.
  //
  Private->AbsolutePointerList[Private->CurrentNumberOfAbsolutePointers] = AbsolutePointer;
  Private->CurrentNumberOfAbsolutePointers++;
  return EFI_SUCCESS;
}

EFI_STATUS
ConSplitterAbsolutePointerDeleteDevice (
  IN  TEXT_IN_SPLITTER_PRIVATE_DATA     *Private,
  IN  EFI_ABSOLUTE_POINTER_PROTOCOL     *AbsolutePointer
  )
/*++

Routine Description:

Arguments:

Returns:

  None

--*/
{
  UINTN Index;
  //
  // Remove the specified text-in device data structure from the Text In List,
  // and rearrange the remaining data structures in the Text In List.
  //
  for (Index = 0; Index < Private->CurrentNumberOfAbsolutePointers; Index++) {
    if (Private->AbsolutePointerList[Index] == AbsolutePointer) {
      for (Index = Index; Index < Private->CurrentNumberOfAbsolutePointers - 1; Index++) {
        Private->AbsolutePointerList[Index] = Private->AbsolutePointerList[Index + 1];
      }

      Private->CurrentNumberOfAbsolutePointers--;
      return EFI_SUCCESS;
    }
  }

  return EFI_NOT_FOUND;
}

STATIC
EFI_STATUS
ConSplitterGrowMapTable (
  IN  TEXT_OUT_SPLITTER_PRIVATE_DATA  *Private
  )
/*++

Routine Description:

Arguments:

Returns:

  None

--*/
{
  UINTN Size;
  UINTN NewSize;
  UINTN TotalSize;
  INT32 *TextOutModeMap;
  INT32 *OldTextOutModeMap;
  INT32 *SrcAddress;
  INT32 Index;

  NewSize           = Private->TextOutListCount * sizeof (INT32);
  OldTextOutModeMap = Private->TextOutModeMap;
  TotalSize         = NewSize * Private->TextOutQueryDataCount;

  TextOutModeMap    = AllocateZeroPool (TotalSize);
  if (TextOutModeMap == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }

  SetMem (TextOutModeMap, TotalSize, 0xFF);
  Private->TextOutModeMap = TextOutModeMap;

  //
  // If TextOutList has been enlarged, need to realloc the mode map table
  // The mode map table is regarded as a two dimension array.
  //
  //                         Old                    New
  //  0   ---------> TextOutListCount ----> TextOutListCount
  //  |   -------------------------------------------
  //  |  |                    |                      |
  //  |  |                    |                      |
  //  |  |                    |                      |
  //  |  |                    |                      |
  //  |  |                    |                      |
  // \/  |                    |                      |
  //      -------------------------------------------
  // QueryDataCount
  //
  if (OldTextOutModeMap != NULL) {

    Size        = Private->CurrentNumberOfConsoles * sizeof (INT32);
    Index       = 0;
    SrcAddress  = OldTextOutModeMap;

    //
    // Copy the old data to the new one
    //
    while (Index < Private->TextOutMode.MaxMode) {
      CopyMem (TextOutModeMap, SrcAddress, Size);
      TextOutModeMap += NewSize;
      SrcAddress += Size;
      Index++;
    }
    //
    // Free the old buffer
    //
    FreePool (OldTextOutModeMap);
  }

  return EFI_SUCCESS;
}

STATIC
EFI_STATUS
ConSplitterAddOutputMode (
  IN  TEXT_OUT_SPLITTER_PRIVATE_DATA     *Private,
  IN  EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL    *TextOut
  )
/*++

Routine Description:

Arguments:

Returns:

  None

--*/
{
  EFI_STATUS  Status;
  INT32       MaxMode;
  INT32       Mode;
  UINTN       Index;

  MaxMode                       = TextOut->Mode->MaxMode;
  Private->TextOutMode.MaxMode  = MaxMode;

  //
  // Grow the buffer if query data buffer is not large enough to
  // hold all the mode supported by the first console.
  //
  while (MaxMode > (INT32) Private->TextOutQueryDataCount) {
    Status = ConSplitterGrowBuffer (
              sizeof (TEXT_OUT_SPLITTER_QUERY_DATA),
              &Private->TextOutQueryDataCount,
              (VOID **) &Private->TextOutQueryData
              );
    if (EFI_ERROR (Status)) {
      return EFI_OUT_OF_RESOURCES;
    }
  }
  //
  // Allocate buffer for the output mode map
  //
  Status = ConSplitterGrowMapTable (Private);
  if (EFI_ERROR (Status)) {
    return EFI_OUT_OF_RESOURCES;
  }
  //
  // As the first textout device, directly add the mode in to QueryData
  // and at the same time record the mapping between QueryData and TextOut.
  //
  Mode  = 0;
  Index = 0;
  while (Mode < MaxMode) {
    Status = TextOut->QueryMode (
                  TextOut,
                  Mode,
                  &Private->TextOutQueryData[Mode].Columns,
                  &Private->TextOutQueryData[Mode].Rows
                  );
    //
    // If mode 1 (80x50) is not supported, make sure mode 1 in TextOutQueryData
    // is clear to 0x0.
    //
    if ((EFI_ERROR(Status)) && (Mode == 1)) {
      Private->TextOutQueryData[Mode].Columns = 0;
      Private->TextOutQueryData[Mode].Rows = 0;
    }
    Private->TextOutModeMap[Index] = Mode;
    Mode++;
    Index += Private->TextOutListCount;
  }

  return EFI_SUCCESS;
}

/**
  Reconstruct TextOutModeMap to get intersection of modes

  This routine reconstruct TextOutModeMap to get the intersection
  of modes for all console out devices. Because EFI/UEFI spec require
  mode 0 is 80x25, mode 1 is 80x50, this routine will not check the
  intersection for mode 0 and mode 1.

  @parm TextOutModeMap  Current text out mode map, begin with the mode 80x25
  @parm NewlyAddedMap   New text out mode map, begin with the mode 80x25
  @parm MapStepSize     Mode step size for one console device
  @parm NewMapStepSize  Mode step size for one console device
  @parm MaxMode         Current max text mode
  @parm CurrentMode     Current text mode

  @retval None

**/
STATIC
VOID
ConSplitterGetIntersection (
  IN  INT32                           *TextOutModeMap,
  IN  INT32                           *NewlyAddedMap,
  IN  UINTN                           MapStepSize,
  IN  UINTN                           NewMapStepSize,
  OUT INT32                           *MaxMode,
  OUT INT32                           *CurrentMode
  )
{
  INT32 Index;
  INT32 *CurrentMapEntry;
  INT32 *NextMapEntry;
  INT32 CurrentMaxMode;
  INT32 Mode;

  //
  // According to EFI/UEFI spec, mode 0 and mode 1 have been reserved
  // for 80x25 and 80x50 in Simple Text Out protocol, so don't make intersection
  // for mode 0 and mode 1, mode number starts from 2.
  //
  Index           = 2;
  CurrentMapEntry = &TextOutModeMap[MapStepSize * 2];
  NextMapEntry    = &TextOutModeMap[MapStepSize * 2];
  NewlyAddedMap   = &NewlyAddedMap[NewMapStepSize * 2];

  CurrentMaxMode  = *MaxMode;
  Mode            = *CurrentMode;

  while (Index < CurrentMaxMode) {
    if (*NewlyAddedMap == -1) {
      //
      // This mode is not supported any more. Remove it. Special care
      // must be taken as this remove will also affect current mode;
      //
      if (Index == *CurrentMode) {
        Mode = -1;
      } else if (Index < *CurrentMode) {
        Mode--;
      }
      (*MaxMode)--;
    } else {
      if (CurrentMapEntry != NextMapEntry) {
        CopyMem (NextMapEntry, CurrentMapEntry, MapStepSize * sizeof (INT32));
      }

      NextMapEntry += MapStepSize;
    }

    CurrentMapEntry += MapStepSize;
    NewlyAddedMap += NewMapStepSize;
    Index++;
  }

  *CurrentMode = Mode;

  return ;
}

STATIC
VOID
ConSplitterSyncOutputMode (
  IN  TEXT_OUT_SPLITTER_PRIVATE_DATA     *Private,
  IN  EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL    *TextOut
  )
/*++

Routine Description:

Arguments:
  Private - Private data structure.
  TextOut - Text Out Protocol.
Returns:

  None

--*/
{
  INT32                         CurrentMaxMode;
  INT32                         Mode;
  INT32                         Index;
  INT32                         *TextOutModeMap;
  INT32                         *MapTable;
  INT32                         QueryMode;
  TEXT_OUT_SPLITTER_QUERY_DATA  *TextOutQueryData;
  UINTN                         Rows;
  UINTN                         Columns;
  UINTN                         StepSize;
  EFI_STATUS                    Status;

  //
  // Must make sure that current mode won't change even if mode number changes
  //
  CurrentMaxMode    = Private->TextOutMode.MaxMode;
  TextOutModeMap    = Private->TextOutModeMap;
  StepSize          = Private->TextOutListCount;
  TextOutQueryData  = Private->TextOutQueryData;

  //
  // Query all the mode that the newly added TextOut supports
  //
  Mode      = 0;
  MapTable  = TextOutModeMap + Private->CurrentNumberOfConsoles;
  while (Mode < TextOut->Mode->MaxMode) {
    Status = TextOut->QueryMode (TextOut, Mode, &Columns, &Rows);
    if (EFI_ERROR(Status)) {
      if (Mode == 1) {
        MapTable[StepSize] = Mode;
        TextOutQueryData[Mode].Columns = 0;
        TextOutQueryData[Mode].Rows = 0;
      }
      Mode++;
      continue;
    }
    //
    // Search the intersection map and QueryData database to see if they intersects
    //
    Index = 0;
    while (Index < CurrentMaxMode) {
      QueryMode = *(TextOutModeMap + Index * StepSize);
      if ((TextOutQueryData[QueryMode].Rows == Rows) && (TextOutQueryData[QueryMode].Columns == Columns)) {
        MapTable[Index * StepSize] = Mode;
        break;
      }

      Index++;
    }

    Mode++;
  }
  //
  // Now search the TextOutModeMap table to find the intersection of supported
  // mode between ConSplitter and the newly added device.
  //
  ConSplitterGetIntersection (
    TextOutModeMap,
    MapTable,
    StepSize,
    StepSize,
    &Private->TextOutMode.MaxMode,
    &Private->TextOutMode.Mode
    );

  return ;
}

STATIC
EFI_STATUS
ConSplitterGetIntersectionBetweenConOutAndStrErr (
  VOID
  )
/*++

Routine Description:

Arguments:

Returns:

  EFI_SUCCESS
  EFI_OUT_OF_RESOURCES

--*/
{
  UINTN                         ConOutNumOfConsoles;
  UINTN                         StdErrNumOfConsoles;
  TEXT_OUT_AND_GOP_DATA         *ConOutTextOutList;
  TEXT_OUT_AND_GOP_DATA         *StdErrTextOutList;
  UINTN                         Indexi;
  UINTN                         Indexj;
  UINTN                         ConOutRows;
  UINTN                         ConOutColumns;
  UINTN                         StdErrRows;
  UINTN                         StdErrColumns;
  INT32                         ConOutMaxMode;
  INT32                         StdErrMaxMode;
  INT32                         ConOutMode;
  INT32                         StdErrMode;
  INT32                         Mode;
  INT32                         Index;
  INT32                         *ConOutModeMap;
  INT32                         *StdErrModeMap;
  INT32                         *ConOutMapTable;
  INT32                         *StdErrMapTable;
  TEXT_OUT_SPLITTER_QUERY_DATA  *ConOutQueryData;
  TEXT_OUT_SPLITTER_QUERY_DATA  *StdErrQueryData;
  UINTN                         ConOutStepSize;
  UINTN                         StdErrStepSize;
  BOOLEAN                       FoundTheSameTextOut;
  UINTN                         ConOutMapTableSize;
  UINTN                         StdErrMapTableSize;

  ConOutNumOfConsoles = mConOut.CurrentNumberOfConsoles;
  StdErrNumOfConsoles = mStdErr.CurrentNumberOfConsoles;
  ConOutTextOutList   = mConOut.TextOutList;
  StdErrTextOutList   = mStdErr.TextOutList;

  Indexi              = 0;
  FoundTheSameTextOut = FALSE;
  while ((Indexi < ConOutNumOfConsoles) && (!FoundTheSameTextOut)) {
    Indexj = 0;
    while (Indexj < StdErrNumOfConsoles) {
      if (ConOutTextOutList->TextOut == StdErrTextOutList->TextOut) {
        FoundTheSameTextOut = TRUE;
        break;
      }

      Indexj++;
      StdErrTextOutList++;
    }

    Indexi++;
    ConOutTextOutList++;
  }

  if (!FoundTheSameTextOut) {
    return EFI_SUCCESS;
  }
  //
  // Must make sure that current mode won't change even if mode number changes
  //
  ConOutMaxMode     = mConOut.TextOutMode.MaxMode;
  ConOutModeMap     = mConOut.TextOutModeMap;
  ConOutStepSize    = mConOut.TextOutListCount;
  ConOutQueryData   = mConOut.TextOutQueryData;

  StdErrMaxMode     = mStdErr.TextOutMode.MaxMode;
  StdErrModeMap     = mStdErr.TextOutModeMap;
  StdErrStepSize    = mStdErr.TextOutListCount;
  StdErrQueryData   = mStdErr.TextOutQueryData;

  //
  // Allocate the map table and set the map table's index to -1.
  //
  ConOutMapTableSize  = ConOutMaxMode * sizeof (INT32);
  ConOutMapTable      = AllocateZeroPool (ConOutMapTableSize);
  if (ConOutMapTable == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }

  SetMem (ConOutMapTable, ConOutMapTableSize, 0xFF);

  StdErrMapTableSize  = StdErrMaxMode * sizeof (INT32);
  StdErrMapTable      = AllocateZeroPool (StdErrMapTableSize);
  if (StdErrMapTable == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }

  SetMem (StdErrMapTable, StdErrMapTableSize, 0xFF);

  //
  // Find the intersection of the two set of modes. If they actually intersect, the
  // correponding entry in the map table is set to 1.
  //
  Mode = 0;
  while (Mode < ConOutMaxMode) {
    //
    // Search the intersection map and QueryData database to see if they intersect
    //
    Index = 0;
    ConOutMode    = *(ConOutModeMap + Mode * ConOutStepSize);
    ConOutRows    = ConOutQueryData[ConOutMode].Rows;
    ConOutColumns = ConOutQueryData[ConOutMode].Columns;
    while (Index < StdErrMaxMode) {
      StdErrMode    = *(StdErrModeMap + Index * StdErrStepSize);
      StdErrRows    = StdErrQueryData[StdErrMode].Rows;
      StdErrColumns = StdErrQueryData[StdErrMode].Columns;
      if ((StdErrRows == ConOutRows) && (StdErrColumns == ConOutColumns)) {
        ConOutMapTable[Mode]  = 1;
        StdErrMapTable[Index] = 1;
        break;
      }

      Index++;
    }

    Mode++;
  }
  //
  // Now search the TextOutModeMap table to find the intersection of supported
  // mode between ConSplitter and the newly added device.
  //
  ConSplitterGetIntersection (
    ConOutModeMap,
    ConOutMapTable,
    mConOut.TextOutListCount,
    1,
    &(mConOut.TextOutMode.MaxMode),
    &(mConOut.TextOutMode.Mode)
    );
  if (mConOut.TextOutMode.Mode < 0) {
    mConOut.TextOut.SetMode (&(mConOut.TextOut), 0);
  }

  ConSplitterGetIntersection (
    StdErrModeMap,
    StdErrMapTable,
    mStdErr.TextOutListCount,
    1,
    &(mStdErr.TextOutMode.MaxMode),
    &(mStdErr.TextOutMode.Mode)
    );
  if (mStdErr.TextOutMode.Mode < 0) {
    mStdErr.TextOut.SetMode (&(mStdErr.TextOut), 0);
  }

  FreePool (ConOutMapTable);
  FreePool (StdErrMapTable);

  return EFI_SUCCESS;
}

STATIC
EFI_STATUS
ConSplitterAddGraphicsOutputMode (
  IN  TEXT_OUT_SPLITTER_PRIVATE_DATA  *Private,
  IN  EFI_GRAPHICS_OUTPUT_PROTOCOL    *GraphicsOutput,
  IN  EFI_UGA_DRAW_PROTOCOL           *UgaDraw
  )
/*++

Routine Description:

Arguments:

Returns:

  None

--*/
{
  EFI_STATUS                           Status;
  UINTN                                Index;
  UINTN                                CurrentIndex;
  EFI_GRAPHICS_OUTPUT_MODE_INFORMATION *Mode;
  UINTN                                SizeOfInfo;
  EFI_GRAPHICS_OUTPUT_MODE_INFORMATION *Info;
  EFI_GRAPHICS_OUTPUT_PROTOCOL_MODE    *CurrentGraphicsOutputMode;
  EFI_GRAPHICS_OUTPUT_MODE_INFORMATION *ModeBuffer;
  EFI_GRAPHICS_OUTPUT_MODE_INFORMATION *MatchedMode;
  UINTN                                NumberIndex;
  BOOLEAN                              Match;
  BOOLEAN                              AlreadyExist;
  UINT32                               UgaHorizontalResolution;
  UINT32                               UgaVerticalResolution;
  UINT32                               UgaColorDepth;
  UINT32                               UgaRefreshRate;

  if ((GraphicsOutput == NULL) && (UgaDraw == NULL)) {
    return EFI_UNSUPPORTED;
  }

  CurrentGraphicsOutputMode = Private->GraphicsOutput.Mode;

  Index        = 0;
  CurrentIndex = 0;

  if (Private->CurrentNumberOfUgaDraw != 0) {
    //
    // If any UGA device has already been added, then there is no need to
    // calculate intersection of display mode of different GOP/UGA device,
    // since only one display mode will be exported (i.e. user-defined mode)
    //
    goto Done;
  }

  if (GraphicsOutput != NULL) {
    if (Private->CurrentNumberOfGraphicsOutput == 0) {
        //
        // This is the first Graphics Output device added
        //
        CurrentGraphicsOutputMode->MaxMode = GraphicsOutput->Mode->MaxMode;
        CurrentGraphicsOutputMode->Mode = GraphicsOutput->Mode->Mode;
        CopyMem (CurrentGraphicsOutputMode->Info, GraphicsOutput->Mode->Info, GraphicsOutput->Mode->SizeOfInfo);
        CurrentGraphicsOutputMode->SizeOfInfo = GraphicsOutput->Mode->SizeOfInfo;
        CurrentGraphicsOutputMode->FrameBufferBase = GraphicsOutput->Mode->FrameBufferBase;
        CurrentGraphicsOutputMode->FrameBufferSize = GraphicsOutput->Mode->FrameBufferSize;

        //
        // Allocate resource for the private mode buffer
        //
        ModeBuffer = AllocatePool (GraphicsOutput->Mode->SizeOfInfo * GraphicsOutput->Mode->MaxMode);
        if (ModeBuffer == NULL) {
          return EFI_OUT_OF_RESOURCES;
        }
        FreePool (Private->GraphicsOutputModeBuffer);
        Private->GraphicsOutputModeBuffer = ModeBuffer;

        //
        // Store all supported display modes to the private mode buffer
        //
        Mode = ModeBuffer;
        for (Index = 0; Index < GraphicsOutput->Mode->MaxMode; Index++) {
          Status = GraphicsOutput->QueryMode (GraphicsOutput, (UINT32) Index, &SizeOfInfo, &Info);
          if (EFI_ERROR (Status)) {
            return Status;
          }
          CopyMem (Mode, Info, SizeOfInfo);
          Mode++;
          FreePool (Info);
        }
    } else {
      //
      // Check intersection of display mode
      //
      ModeBuffer = AllocatePool (sizeof (EFI_GRAPHICS_OUTPUT_MODE_INFORMATION) * CurrentGraphicsOutputMode->MaxMode);
      if (ModeBuffer == NULL) {
        return EFI_OUT_OF_RESOURCES;
      }

      MatchedMode = ModeBuffer;
      Mode = &Private->GraphicsOutputModeBuffer[0];
      for (Index = 0; Index < CurrentGraphicsOutputMode->MaxMode; Index++) {
        Match = FALSE;

        for (NumberIndex = 0; NumberIndex < GraphicsOutput->Mode->MaxMode; NumberIndex++) {
          Status = GraphicsOutput->QueryMode (GraphicsOutput, (UINT32) NumberIndex, &SizeOfInfo, &Info);
          if (EFI_ERROR (Status)) {
            return Status;
          }
          if ((Info->HorizontalResolution == Mode->HorizontalResolution) &&
              (Info->VerticalResolution == Mode->VerticalResolution)) {
            Match = TRUE;
            FreePool (Info);
            break;
          }
          FreePool (Info);
        }

        if (Match) {
          AlreadyExist = FALSE;

          for (Info = ModeBuffer; Info < MatchedMode; Info++) {
            if ((Info->HorizontalResolution == Mode->HorizontalResolution) &&
                (Info->VerticalResolution == Mode->VerticalResolution)) {
              AlreadyExist = TRUE;
              break;
            }
          }

          if (!AlreadyExist) {
            CopyMem (MatchedMode, Mode, sizeof (EFI_GRAPHICS_OUTPUT_MODE_INFORMATION));

            //
            // Physical frame buffer is no longer available, change PixelFormat to PixelBltOnly
            //
            MatchedMode->Version = 0;
            MatchedMode->PixelFormat = PixelBltOnly;
            ZeroMem (&MatchedMode->PixelInformation, sizeof (EFI_PIXEL_BITMASK));

            MatchedMode++;
          }
        }

        Mode++;
      }

      //
      // Drop the old mode buffer, assign it to a new one
      //
      FreePool (Private->GraphicsOutputModeBuffer);
      Private->GraphicsOutputModeBuffer = ModeBuffer;

      //
      // Physical frame buffer is no longer available when there are more than one physical GOP devices
      //
      CurrentGraphicsOutputMode->MaxMode = (UINT32) (((UINTN) MatchedMode - (UINTN) ModeBuffer) / sizeof (EFI_GRAPHICS_OUTPUT_MODE_INFORMATION));
      CurrentGraphicsOutputMode->Info->PixelFormat = PixelBltOnly;
      ZeroMem (&CurrentGraphicsOutputMode->Info->PixelInformation, sizeof (EFI_PIXEL_BITMASK));
      CurrentGraphicsOutputMode->SizeOfInfo = sizeof (EFI_GRAPHICS_OUTPUT_MODE_INFORMATION);
      CurrentGraphicsOutputMode->FrameBufferBase = (EFI_PHYSICAL_ADDRESS) NULL;
      CurrentGraphicsOutputMode->FrameBufferSize = 0;
    }

    //
    // Graphics console driver can ensure the same mode for all GOP devices
    //
    for (Index = 0; Index < CurrentGraphicsOutputMode->MaxMode; Index++) {
      Mode = &Private->GraphicsOutputModeBuffer[Index];
      if ((Mode->HorizontalResolution == GraphicsOutput->Mode->Info->HorizontalResolution) &&
         (Mode->VerticalResolution == GraphicsOutput->Mode->Info->VerticalResolution)) {
        CurrentIndex = Index;
        break;
      }
    }
    if (Index >= CurrentGraphicsOutputMode->MaxMode) {
      //
      // if user defined mode is not found, set to default mode 800x600
      //
      for (Index = 0; Index < CurrentGraphicsOutputMode->MaxMode; Index++) {
        Mode = &Private->GraphicsOutputModeBuffer[Index];
        if ((Mode->HorizontalResolution == 800) && (Mode->VerticalResolution == 600)) {
          CurrentIndex = Index;
          break;
        }
      }
    }
  }
  if (UgaDraw != NULL && FeaturePcdGet (PcdUgaConsumeSupport)) {
    //
    // Graphics console driver can ensure the same mode for all GOP devices
    // so we can get the current mode from this video device
    //
    UgaDraw->GetMode (
               UgaDraw,
               &UgaHorizontalResolution,
               &UgaVerticalResolution,
               &UgaColorDepth,
               &UgaRefreshRate
               );

    CurrentGraphicsOutputMode->MaxMode = 1;
    Info = CurrentGraphicsOutputMode->Info;
    Info->Version = 0;
    Info->HorizontalResolution = UgaHorizontalResolution;
    Info->VerticalResolution = UgaVerticalResolution;
    Info->PixelFormat = PixelBltOnly;
    Info->PixelsPerScanLine = UgaHorizontalResolution;
    CurrentGraphicsOutputMode->SizeOfInfo = sizeof (EFI_GRAPHICS_OUTPUT_MODE_INFORMATION);
    CurrentGraphicsOutputMode->FrameBufferBase = (EFI_PHYSICAL_ADDRESS) NULL;
    CurrentGraphicsOutputMode->FrameBufferSize = 0;

    //
    // Update the private mode buffer
    //
    CopyMem (&Private->GraphicsOutputModeBuffer[0], Info, sizeof (EFI_GRAPHICS_OUTPUT_MODE_INFORMATION));

    //
    // Only mode 0 is available to be set
    //
    CurrentIndex = 0;
  }

Done:

  if (GraphicsOutput != NULL) {
    Private->CurrentNumberOfGraphicsOutput++;
  }
  if (UgaDraw != NULL && FeaturePcdGet (PcdUgaConsumeSupport)) {
    Private->CurrentNumberOfUgaDraw++;
  }

  //
  // Force GraphicsOutput mode to be set,
  // regardless whether the console is in EfiConsoleControlScreenGraphics or EfiConsoleControlScreenText mode
  //
  Private->HardwareNeedsStarting = TRUE;
  //
  // Current mode number may need update now, so set it to an invalid mode number
  //
  CurrentGraphicsOutputMode->Mode = 0xffff;
  //
  // Graphics console can ensure all GOP devices have the same mode which can be taken as current mode.
  //
  Status = Private->GraphicsOutput.SetMode (&Private->GraphicsOutput, (UINT32) CurrentIndex);

  //
  // If user defined mode is not valid for UGA, set to the default mode 800x600.
  //
  if (EFI_ERROR(Status)) {
    (Private->GraphicsOutputModeBuffer[0]).HorizontalResolution = 800;
    (Private->GraphicsOutputModeBuffer[0]).VerticalResolution   = 600;
    Status = Private->GraphicsOutput.SetMode (&Private->GraphicsOutput, 0);
  }

  return Status;
}

VOID
ConsplitterSetConsoleOutMode (
  IN  TEXT_OUT_SPLITTER_PRIVATE_DATA  *Private
  )
/*++

Routine Description:

  This routine will get the current console mode information (column, row)
  from ConsoleOutMode variable and set it; if the variable does not exist,
  set to user defined console mode.

Arguments:

  None

Returns:

  None

--*/
{
  UINTN                         Col;
  UINTN                         Row;
  UINTN                         Mode;
  UINTN                         PreferMode;
  UINTN                         BaseMode;
  UINTN                         ModeInfoSize;
  UINTN                         MaxMode;
  EFI_STATUS                    Status;
  CONSOLE_OUT_MODE              *ModeInfo;
  EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL  *TextOut;

  PreferMode   = 0xFF;
  BaseMode     = 0xFF;
  TextOut      = &Private->TextOut;
  MaxMode      = (UINTN) (TextOut->Mode->MaxMode);
  ModeInfoSize = sizeof (CONSOLE_OUT_MODE);

  ModeInfo = AllocateZeroPool (sizeof(CONSOLE_OUT_MODE));
  ASSERT(ModeInfo != NULL);

  Status = gRT->GetVariable (
                   VarConOutMode,
                   &gEfiGenericPlatformVariableGuid,
                   NULL,
                   &ModeInfoSize,
                   ModeInfo
                   );

  //
  // Set to the default mode 80 x 25 required by EFI/UEFI spec;
  // user can also define other valid default console mode here.
  //
  if (EFI_ERROR(Status)) {
    ModeInfo->Column = 80;
    ModeInfo->Row    = 25;
    Status = gRT->SetVariable (
                    VarConOutMode,
                    &gEfiGenericPlatformVariableGuid,
                    EFI_VARIABLE_BOOTSERVICE_ACCESS | EFI_VARIABLE_NON_VOLATILE,
                    sizeof (CONSOLE_OUT_MODE),
                    ModeInfo
                    );
  }

  for (Mode = 0; Mode < MaxMode; Mode++) {
    Status = TextOut->QueryMode (TextOut, Mode, &Col, &Row);
    if (!EFI_ERROR(Status)) {
      if (Col == ModeInfo->Column && Row == ModeInfo->Row) {
        PreferMode = Mode;
      }
      if (Col == 80 && Row == 25) {
        BaseMode = Mode;
      }
    }
  }

  Status = TextOut->SetMode (TextOut, PreferMode);

  //
  // if current mode setting is failed, default 80x25 mode will be set.
  //
  if (EFI_ERROR(Status)) {
    Status = TextOut->SetMode (TextOut, BaseMode);
    ASSERT(!EFI_ERROR(Status));

    ModeInfo->Column = 80;
    ModeInfo->Row    = 25;

    //
    // Update ConOutMode variable
    //
    Status = gRT->SetVariable (
                    VarConOutMode,
                    &gEfiGenericPlatformVariableGuid,
                    EFI_VARIABLE_BOOTSERVICE_ACCESS | EFI_VARIABLE_NON_VOLATILE,
                    sizeof (CONSOLE_OUT_MODE),
                    ModeInfo
                    );
  }

  gBS->FreePool (ModeInfo);
}


EFI_STATUS
ConSplitterTextOutAddDevice (
  IN  TEXT_OUT_SPLITTER_PRIVATE_DATA     *Private,
  IN  EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL    *TextOut,
  IN  EFI_GRAPHICS_OUTPUT_PROTOCOL       *GraphicsOutput,
  IN  EFI_UGA_DRAW_PROTOCOL              *UgaDraw
  )
/*++

Routine Description:

Arguments:

Returns:

  None

--*/
{
  EFI_STATUS            Status;
  UINTN                 CurrentNumOfConsoles;
  INT32                 CurrentMode;
  INT32                 MaxMode;
  UINT32                UgaHorizontalResolution;
  UINT32                UgaVerticalResolution;
  UINT32                UgaColorDepth;
  UINT32                UgaRefreshRate;
  TEXT_OUT_AND_GOP_DATA *TextAndGop;

  Status                = EFI_SUCCESS;
  CurrentNumOfConsoles  = Private->CurrentNumberOfConsoles;

  //
  // If the Text Out List is full, enlarge it by calling growbuffer().
  //
  while (CurrentNumOfConsoles >= Private->TextOutListCount) {
    Status = ConSplitterGrowBuffer (
              sizeof (TEXT_OUT_AND_GOP_DATA),
              &Private->TextOutListCount,
              (VOID **) &Private->TextOutList
              );
    if (EFI_ERROR (Status)) {
      return EFI_OUT_OF_RESOURCES;
    }
    //
    // Also need to reallocate the TextOutModeMap table
    //
    Status = ConSplitterGrowMapTable (Private);
    if (EFI_ERROR (Status)) {
      return EFI_OUT_OF_RESOURCES;
    }
  }

  TextAndGop          = &Private->TextOutList[CurrentNumOfConsoles];

  TextAndGop->TextOut = TextOut;
  TextAndGop->GraphicsOutput = GraphicsOutput;
  TextAndGop->UgaDraw = UgaDraw;

  if ((GraphicsOutput == NULL) && (UgaDraw == NULL)) {
    //
    // If No GOP/UGA device then use the ConOut device
    //
    TextAndGop->TextOutEnabled = TRUE;
  } else {
    //
    // If GOP/UGA device use ConOut device only used if screen is in Text mode
    //
    TextAndGop->TextOutEnabled = (BOOLEAN) (Private->ConsoleOutputMode == EfiConsoleControlScreenText);
  }

  if (CurrentNumOfConsoles == 0) {
    //
    // Add the first device's output mode to console splitter's mode list
    //
    Status = ConSplitterAddOutputMode (Private, TextOut);
  } else {
    ConSplitterSyncOutputMode (Private, TextOut);
  }

  Private->CurrentNumberOfConsoles++;

  //
  // Scan both TextOutList, for the intersection TextOut device
  // maybe both ConOut and StdErr incorporate the same Text Out
  // device in them, thus the output of both should be synced.
  //
  ConSplitterGetIntersectionBetweenConOutAndStrErr ();

  CurrentMode = Private->TextOutMode.Mode;
  MaxMode     = Private->TextOutMode.MaxMode;
  ASSERT (MaxMode >= 1);

  //
  // Update DevNull mode according to current video device
  //
  if (FeaturePcdGet (PcdConOutGopSupport)) {
    if ((GraphicsOutput != NULL) || (UgaDraw != NULL)) {
      ConSplitterAddGraphicsOutputMode (Private, GraphicsOutput, UgaDraw);
    }
  }
  if (FeaturePcdGet (PcdConOutUgaSupport)) {
    if (UgaDraw != NULL && FeaturePcdGet (PcdUgaConsumeSupport)) {
      Status = UgaDraw->GetMode (
                    UgaDraw,
                    &UgaHorizontalResolution,
                    &UgaVerticalResolution,
                    &UgaColorDepth,
                    &UgaRefreshRate
                    );
      if (!EFI_ERROR (Status)) {
        Status = ConSpliterUgaDrawSetMode (
                    &Private->UgaDraw,
                    UgaHorizontalResolution,
                    UgaVerticalResolution,
                    UgaColorDepth,
                    UgaRefreshRate
                    );
      }
      //
      // If GetMode/SetMode is failed, set to 800x600 mode
      //
      if(EFI_ERROR (Status)) {
        Status = ConSpliterUgaDrawSetMode (
                    &Private->UgaDraw,
                    800,
                    600,
                    32,
                    60
                    );
      }
    }
  }

  if (Private->ConsoleOutputMode == EfiConsoleControlScreenGraphics && GraphicsOutput != NULL) {
    //
    // We just added a new GOP or UGA device in graphics mode
    //
    if (FeaturePcdGet (PcdConOutGopSupport)) {
      DevNullGopSync (Private, TextAndGop->GraphicsOutput, TextAndGop->UgaDraw);
    } else if (FeaturePcdGet (PcdConOutUgaSupport)) {
      DevNullUgaSync (Private, TextAndGop->GraphicsOutput, TextAndGop->UgaDraw);
    }
  } else if ((CurrentMode >= 0) && ((GraphicsOutput != NULL) || (UgaDraw != NULL)) && (CurrentMode < Private->TextOutMode.MaxMode)) {
    //
    // The new console supports the same mode of the current console so sync up
    //
    DevNullSyncStdOut (Private);
  } else {
    //
    // If ConOut, then set the mode to Mode #0 which us 80 x 25
    //
    Private->TextOut.SetMode (&Private->TextOut, 0);
  }

  //
  // After adding new console device, all existing console devices should be
  // synced to the current shared mode.
  //
  ConsplitterSetConsoleOutMode (Private);

  return Status;
}

EFI_STATUS
ConSplitterTextOutDeleteDevice (
  IN  TEXT_OUT_SPLITTER_PRIVATE_DATA     *Private,
  IN  EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL    *TextOut
  )
/*++

Routine Description:

Arguments:

Returns:

  None

--*/
{
  INT32                 Index;
  UINTN                 CurrentNumOfConsoles;
  TEXT_OUT_AND_GOP_DATA *TextOutList;
  EFI_STATUS            Status;

  //
  // Remove the specified text-out device data structure from the Text out List,
  // and rearrange the remaining data structures in the Text out List.
  //
  CurrentNumOfConsoles  = Private->CurrentNumberOfConsoles;
  Index                 = (INT32) CurrentNumOfConsoles - 1;
  TextOutList           = Private->TextOutList;
  while (Index >= 0) {
    if (TextOutList->TextOut == TextOut) {
      CopyMem (TextOutList, TextOutList + 1, sizeof (TEXT_OUT_AND_GOP_DATA) * Index);
      CurrentNumOfConsoles--;
      if (TextOutList->UgaDraw != NULL && FeaturePcdGet (PcdUgaConsumeSupport)) {
        Private->CurrentNumberOfUgaDraw--;
      }
      if (TextOutList->GraphicsOutput != NULL) {
        Private->CurrentNumberOfGraphicsOutput--;
      }
      break;
    }

    Index--;
    TextOutList++;
  }
  //
  // The specified TextOut is not managed by the ConSplitter driver
  //
  if (Index < 0) {
    return EFI_NOT_FOUND;
  }

  if (CurrentNumOfConsoles == 0) {
    //
    // If the number of consoles is zero clear the Dev NULL device
    //
    Private->CurrentNumberOfConsoles      = 0;
    Private->TextOutMode.MaxMode          = 1;
    Private->TextOutQueryData[0].Columns  = 80;
    Private->TextOutQueryData[0].Rows     = 25;
    DevNullTextOutSetMode (Private, 0);

    return EFI_SUCCESS;
  }
  //
  // Max Mode is realy an intersection of the QueryMode command to all
  // devices. So we must copy the QueryMode of the first device to
  // QueryData.
  //
  ZeroMem (
    Private->TextOutQueryData,
    Private->TextOutQueryDataCount * sizeof (TEXT_OUT_SPLITTER_QUERY_DATA)
    );

  FreePool (Private->TextOutModeMap);
  Private->TextOutModeMap = NULL;
  TextOutList             = Private->TextOutList;

  //
  // Add the first TextOut to the QueryData array and ModeMap table
  //
  Status = ConSplitterAddOutputMode (Private, TextOutList->TextOut);

  //
  // Now add one by one
  //
  Index = 1;
  Private->CurrentNumberOfConsoles = 1;
  TextOutList++;
  while ((UINTN) Index < CurrentNumOfConsoles) {
    ConSplitterSyncOutputMode (Private, TextOutList->TextOut);
    Index++;
    Private->CurrentNumberOfConsoles++;
    TextOutList++;
  }

  ConSplitterGetIntersectionBetweenConOutAndStrErr ();

  return Status;
}
//
// ConSplitter TextIn member functions
//
EFI_STATUS
EFIAPI
ConSplitterTextInReset (
  IN  EFI_SIMPLE_TEXT_INPUT_PROTOCOL  *This,
  IN  BOOLEAN                         ExtendedVerification
  )
/*++

  Routine Description:
    Reset the input device and optionaly run diagnostics

  Arguments:
    This                 - Protocol instance pointer.
    ExtendedVerification - Driver may perform diagnostics on reset.

  Returns:
    EFI_SUCCESS           - The device was reset.
    EFI_DEVICE_ERROR      - The device is not functioning properly and could
                            not be reset.

--*/
{
  EFI_STATUS                    Status;
  EFI_STATUS                    ReturnStatus;
  TEXT_IN_SPLITTER_PRIVATE_DATA *Private;
  UINTN                         Index;

  Private                       = TEXT_IN_SPLITTER_PRIVATE_DATA_FROM_THIS (This);

  Private->KeyEventSignalState  = FALSE;

  //
  // return the worst status met
  //
  for (Index = 0, ReturnStatus = EFI_SUCCESS; Index < Private->CurrentNumberOfConsoles; Index++) {
    Status = Private->TextInList[Index]->Reset (
                                          Private->TextInList[Index],
                                          ExtendedVerification
                                          );
    if (EFI_ERROR (Status)) {
      ReturnStatus = Status;
    }
  }

  return ReturnStatus;
}

EFI_STATUS
EFIAPI
ConSplitterTextInPrivateReadKeyStroke (
  IN  TEXT_IN_SPLITTER_PRIVATE_DATA   *Private,
  OUT EFI_INPUT_KEY                   *Key
  )
/*++

  Routine Description:
    Reads the next keystroke from the input device. The WaitForKey Event can
    be used to test for existance of a keystroke via WaitForEvent () call.

  Arguments:
    This   - Protocol instance pointer.
    Key    - Driver may perform diagnostics on reset.

  Returns:
    EFI_SUCCESS       - The keystroke information was returned.
    EFI_NOT_READY     - There was no keystroke data availiable.
    EFI_DEVICE_ERROR  - The keydtroke information was not returned due to
                        hardware errors.

--*/
{
  EFI_STATUS    Status;
  UINTN         Index;
  EFI_INPUT_KEY CurrentKey;

  Key->UnicodeChar  = 0;
  Key->ScanCode     = SCAN_NULL;

  //
  // if no physical console input device exists, return EFI_NOT_READY;
  // if any physical console input device has key input,
  // return the key and EFI_SUCCESS.
  //
  for (Index = 0; Index < Private->CurrentNumberOfConsoles; Index++) {
    Status = Private->TextInList[Index]->ReadKeyStroke (
                                          Private->TextInList[Index],
                                          &CurrentKey
                                          );
    if (!EFI_ERROR (Status)) {
      *Key = CurrentKey;
      return Status;
    }
  }

  return EFI_NOT_READY;
}

BOOLEAN
ConSpliterConssoleControlStdInLocked (
  VOID
  )
/*++

Routine Description:
  Return TRUE if StdIn is locked. The ConIn device on the virtual handle is
  the only device locked.

Arguments:
  NONE

Returns:
  TRUE  - StdIn locked
  FALSE - StdIn working normally

--*/
{
  return mConIn.PasswordEnabled;
}

VOID
EFIAPI
ConSpliterConsoleControlLockStdInEvent (
  IN  EFI_EVENT                       Event,
  IN  VOID                            *Context
  )
/*++

Routine Description:
  This timer event will fire when StdIn is locked. It will check the key
  sequence on StdIn to see if it matches the password. Any error in the
  password will cause the check to reset. As long a mConIn.PasswordEnabled is
  TRUE the StdIn splitter will not report any input.

Arguments:
  (Standard EFI_EVENT_NOTIFY)

Returns:
  None

--*/
{
  EFI_STATUS    Status;
  EFI_INPUT_KEY Key;
  CHAR16        BackSpaceString[2];
  CHAR16        SpaceString[2];

  do {
    Status = ConSplitterTextInPrivateReadKeyStroke (&mConIn, &Key);
    if (!EFI_ERROR (Status)) {
      //
      // if it's an ENTER, match password
      //
      if ((Key.UnicodeChar == CHAR_CARRIAGE_RETURN) && (Key.ScanCode == SCAN_NULL)) {
        mConIn.PwdAttempt[mConIn.PwdIndex] = CHAR_NULL;
        if (StrCmp (mConIn.Password, mConIn.PwdAttempt)) {
          //
          // Password not match
          //
          ConSplitterTextOutOutputString (&mConOut.TextOut, (CHAR16 *) L"\n\rPassword not correct\n\r");
          mConIn.PwdIndex = 0;
        } else {
          //
          // Key matches password sequence
          //
          gBS->SetTimer (mConIn.LockEvent, TimerPeriodic, 0);
          mConIn.PasswordEnabled  = FALSE;
          Status                  = EFI_NOT_READY;
        }
      } else if ((Key.UnicodeChar == CHAR_BACKSPACE) && (Key.ScanCode == SCAN_NULL)) {
        //
        // BackSpace met
        //
        if (mConIn.PwdIndex > 0) {
          BackSpaceString[0]  = CHAR_BACKSPACE;
          BackSpaceString[1]  = 0;

          SpaceString[0]      = ' ';
          SpaceString[1]      = 0;

          ConSplitterTextOutOutputString (&mConOut.TextOut, BackSpaceString);
          ConSplitterTextOutOutputString (&mConOut.TextOut, SpaceString);
          ConSplitterTextOutOutputString (&mConOut.TextOut, BackSpaceString);

          mConIn.PwdIndex--;
        }
      } else if ((Key.ScanCode == SCAN_NULL) && (Key.UnicodeChar >= 32)) {
        //
        // If it's not an ENTER, neigher a function key, nor a CTRL-X or ALT-X, record the input
        //
        if (mConIn.PwdIndex < (MAX_STD_IN_PASSWORD - 1)) {
          if (mConIn.PwdIndex == 0) {
            ConSplitterTextOutOutputString (&mConOut.TextOut, (CHAR16 *) L"\n\r");
          }

          ConSplitterTextOutOutputString (&mConOut.TextOut, (CHAR16 *) L"*");
          mConIn.PwdAttempt[mConIn.PwdIndex] = Key.UnicodeChar;
          mConIn.PwdIndex++;
        }
      }
    }
  } while (!EFI_ERROR (Status));
}

EFI_STATUS
EFIAPI
ConSpliterConsoleControlLockStdIn (
  IN  EFI_CONSOLE_CONTROL_PROTOCOL    *This,
  IN  CHAR16                          *Password
  )
/*++

Routine Description:
  If Password is NULL unlock the password state variable and set the event
  timer. If the Password is too big return an error. If the Password is valid
  Copy the Password and enable state variable and then arm the periodic timer

Arguments:

Returns:
  EFI_SUCCESS           - Lock the StdIn device
  EFI_INVALID_PARAMETER - Password is NULL
  EFI_OUT_OF_RESOURCES  - Buffer allocation to store the password fails

--*/
{
  if (Password == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  if (StrLen (Password) >= MAX_STD_IN_PASSWORD) {
    //
    // Currently have a max password size
    //
    return EFI_OUT_OF_RESOURCES;
  }
  //
  // Save the password, initialize state variables and arm event timer
  //
  StrCpy (mConIn.Password, Password);
  mConIn.PasswordEnabled  = TRUE;
  mConIn.PwdIndex         = 0;
  gBS->SetTimer (mConIn.LockEvent, TimerPeriodic, (10000 * 25));

  return EFI_SUCCESS;
}

EFI_STATUS
EFIAPI
ConSplitterTextInReadKeyStroke (
  IN  EFI_SIMPLE_TEXT_INPUT_PROTOCOL  *This,
  OUT EFI_INPUT_KEY                   *Key
  )
/*++

  Routine Description:
    Reads the next keystroke from the input device. The WaitForKey Event can
    be used to test for existance of a keystroke via WaitForEvent () call.
    If the ConIn is password locked make it look like no keystroke is availible

  Arguments:
    This   - Protocol instance pointer.
    Key    - Driver may perform diagnostics on reset.

  Returns:
    EFI_SUCCESS       - The keystroke information was returned.
    EFI_NOT_READY     - There was no keystroke data availiable.
    EFI_DEVICE_ERROR  - The keydtroke information was not returned due to
                        hardware errors.

--*/
{
  TEXT_IN_SPLITTER_PRIVATE_DATA *Private;

  Private = TEXT_IN_SPLITTER_PRIVATE_DATA_FROM_THIS (This);
  if (Private->PasswordEnabled) {
    //
    // If StdIn Locked return not ready
    //
    return EFI_NOT_READY;
  }

  Private->KeyEventSignalState = FALSE;

  return ConSplitterTextInPrivateReadKeyStroke (Private, Key);
}

VOID
EFIAPI
ConSplitterTextInWaitForKey (
  IN  EFI_EVENT                       Event,
  IN  VOID                            *Context
  )
/*++

Routine Description:
  This event agregates all the events of the ConIn devices in the spliter.
  If the ConIn is password locked then return.
  If any events of physical ConIn devices are signaled, signal the ConIn
  spliter event. This will cause the calling code to call
  ConSplitterTextInReadKeyStroke ().

Arguments:
  Event   - The Event assoicated with callback.
  Context - Context registered when Event was created.

Returns:
  None

--*/
{
  EFI_STATUS                    Status;
  TEXT_IN_SPLITTER_PRIVATE_DATA *Private;
  UINTN                         Index;

  Private = (TEXT_IN_SPLITTER_PRIVATE_DATA *) Context;
  if (Private->PasswordEnabled) {
    //
    // If StdIn Locked return not ready
    //
    return ;
  }

  //
  // if KeyEventSignalState is flagged before, and not cleared by Reset() or ReadKeyStroke()
  //
  if (Private->KeyEventSignalState) {
    gBS->SignalEvent (Event);
    return ;
  }
  //
  // if any physical console input device has key input, signal the event.
  //
  for (Index = 0; Index < Private->CurrentNumberOfConsoles; Index++) {
    Status = gBS->CheckEvent (Private->TextInList[Index]->WaitForKey);
    if (!EFI_ERROR (Status)) {
      gBS->SignalEvent (Event);
      Private->KeyEventSignalState = TRUE;
    }
  }
}


STATIC
BOOLEAN
IsKeyRegistered (
  IN EFI_KEY_DATA  *RegsiteredData,
  IN EFI_KEY_DATA  *InputData
  )
/*++

Routine Description:

Arguments:

  RegsiteredData    - A pointer to a buffer that is filled in with the keystroke
                      state data for the key that was registered.
  InputData         - A pointer to a buffer that is filled in with the keystroke
                      state data for the key that was pressed.

Returns:
  TRUE              - Key be pressed matches a registered key.
  FLASE             - Match failed.

--*/
{
  ASSERT (RegsiteredData != NULL && InputData != NULL);

  if ((RegsiteredData->Key.ScanCode    != InputData->Key.ScanCode) ||
      (RegsiteredData->Key.UnicodeChar != InputData->Key.UnicodeChar)) {
    return FALSE;
  }

  //
  // Assume KeyShiftState/KeyToggleState = 0 in Registered key data means these state could be ignored.
  //
  if (RegsiteredData->KeyState.KeyShiftState != 0 &&
      RegsiteredData->KeyState.KeyShiftState != InputData->KeyState.KeyShiftState) {
    return FALSE;
  }
  if (RegsiteredData->KeyState.KeyToggleState != 0 &&
      RegsiteredData->KeyState.KeyToggleState != InputData->KeyState.KeyToggleState) {
    return FALSE;
  }

  return TRUE;

}

//
// Simple Text Input Ex protocol functions
//

EFI_STATUS
EFIAPI
ConSplitterTextInResetEx (
  IN EFI_SIMPLE_TEXT_INPUT_EX_PROTOCOL  *This,
  IN BOOLEAN                            ExtendedVerification
  )
/*++

  Routine Description:
    Reset the input device and optionaly run diagnostics

  Arguments:
    This                 - Protocol instance pointer.
    ExtendedVerification - Driver may perform diagnostics on reset.

  Returns:
    EFI_SUCCESS           - The device was reset.
    EFI_DEVICE_ERROR      - The device is not functioning properly and could
                            not be reset.

--*/
{
  EFI_STATUS                    Status;
  EFI_STATUS                    ReturnStatus;
  TEXT_IN_SPLITTER_PRIVATE_DATA *Private;
  UINTN                         Index;

  Private                       = TEXT_IN_EX_SPLITTER_PRIVATE_DATA_FROM_THIS (This);

  Private->KeyEventSignalState  = FALSE;

  //
  // return the worst status met
  //
  for (Index = 0, ReturnStatus = EFI_SUCCESS; Index < Private->CurrentNumberOfExConsoles; Index++) {
    Status = Private->TextInExList[Index]->Reset (
                                             Private->TextInExList[Index],
                                             ExtendedVerification
                                             );
    if (EFI_ERROR (Status)) {
      ReturnStatus = Status;
    }
  }

  return ReturnStatus;

}

EFI_STATUS
EFIAPI
ConSplitterTextInReadKeyStrokeEx (
  IN  EFI_SIMPLE_TEXT_INPUT_EX_PROTOCOL *This,
  OUT EFI_KEY_DATA                      *KeyData
  )
/*++

  Routine Description:
    Reads the next keystroke from the input device. The WaitForKey Event can
    be used to test for existance of a keystroke via WaitForEvent () call.

  Arguments:
    This       - Protocol instance pointer.
    KeyData    - A pointer to a buffer that is filled in with the keystroke
                 state data for the key that was pressed.

  Returns:
    EFI_SUCCESS           - The keystroke information was returned.
    EFI_NOT_READY         - There was no keystroke data availiable.
    EFI_DEVICE_ERROR      - The keystroke information was not returned due to
                            hardware errors.
    EFI_INVALID_PARAMETER - KeyData is NULL.

--*/
{
  TEXT_IN_SPLITTER_PRIVATE_DATA *Private;
  EFI_STATUS                    Status;
  UINTN                         Index;
  EFI_KEY_DATA                  CurrentKeyData;


  if (KeyData == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  Private = TEXT_IN_EX_SPLITTER_PRIVATE_DATA_FROM_THIS (This);
  if (Private->PasswordEnabled) {
    //
    // If StdIn Locked return not ready
    //
    return EFI_NOT_READY;
  }

  Private->KeyEventSignalState = FALSE;

  KeyData->Key.UnicodeChar  = 0;
  KeyData->Key.ScanCode     = SCAN_NULL;

  //
  // if no physical console input device exists, return EFI_NOT_READY;
  // if any physical console input device has key input,
  // return the key and EFI_SUCCESS.
  //
  for (Index = 0; Index < Private->CurrentNumberOfExConsoles; Index++) {
    Status = Private->TextInExList[Index]->ReadKeyStrokeEx (
                                          Private->TextInExList[Index],
                                          &CurrentKeyData
                                          );
    if (!EFI_ERROR (Status)) {
      CopyMem (KeyData, &CurrentKeyData, sizeof (CurrentKeyData));
      return Status;
    }
  }

  return EFI_NOT_READY;
}

EFI_STATUS
EFIAPI
ConSplitterTextInSetState (
  IN EFI_SIMPLE_TEXT_INPUT_EX_PROTOCOL  *This,
  IN EFI_KEY_TOGGLE_STATE               *KeyToggleState
  )
/*++

  Routine Description:
    Set certain state for the input device.

  Arguments:
    This                  - Protocol instance pointer.
    KeyToggleState        - A pointer to the EFI_KEY_TOGGLE_STATE to set the
                            state for the input device.

  Returns:
    EFI_SUCCESS           - The device state was set successfully.
    EFI_DEVICE_ERROR      - The device is not functioning correctly and could
                            not have the setting adjusted.
    EFI_UNSUPPORTED       - The device does not have the ability to set its state.
    EFI_INVALID_PARAMETER - KeyToggleState is NULL.

--*/
{
  TEXT_IN_SPLITTER_PRIVATE_DATA *Private;
  EFI_STATUS                    Status;
  UINTN                         Index;

  if (KeyToggleState == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  Private = TEXT_IN_EX_SPLITTER_PRIVATE_DATA_FROM_THIS (This);

  //
  // if no physical console input device exists, return EFI_SUCCESS;
  // otherwise return the status of setting state of physical console input device
  //
  for (Index = 0; Index < Private->CurrentNumberOfExConsoles; Index++) {
    Status = Private->TextInExList[Index]->SetState (
                                             Private->TextInExList[Index],
                                             KeyToggleState
                                             );
    if (EFI_ERROR (Status)) {
      return Status;
    }
  }

  return EFI_SUCCESS;

}

EFI_STATUS
EFIAPI
ConSplitterTextInRegisterKeyNotify (
  IN EFI_SIMPLE_TEXT_INPUT_EX_PROTOCOL  *This,
  IN EFI_KEY_DATA                       *KeyData,
  IN EFI_KEY_NOTIFY_FUNCTION            KeyNotificationFunction,
  OUT EFI_HANDLE                        *NotifyHandle
  )
/*++

  Routine Description:
    Register a notification function for a particular keystroke for the input device.

  Arguments:
    This                    - Protocol instance pointer.
    KeyData                 - A pointer to a buffer that is filled in with the keystroke
                              information data for the key that was pressed.
    KeyNotificationFunction - Points to the function to be called when the key
                              sequence is typed specified by KeyData.
    NotifyHandle            - Points to the unique handle assigned to the registered notification.

  Returns:
    EFI_SUCCESS             - The notification function was registered successfully.
    EFI_OUT_OF_RESOURCES    - Unable to allocate resources for necesssary data structures.
    EFI_INVALID_PARAMETER   - KeyData or NotifyHandle is NULL.

--*/
{
  TEXT_IN_SPLITTER_PRIVATE_DATA *Private;
  EFI_STATUS                    Status;
  UINTN                         Index;
  TEXT_IN_EX_SPLITTER_NOTIFY    *NewNotify;
  LIST_ENTRY                    *Link;
  TEXT_IN_EX_SPLITTER_NOTIFY    *CurrentNotify;


  if (KeyData == NULL || NotifyHandle == NULL || KeyNotificationFunction == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  Private = TEXT_IN_EX_SPLITTER_PRIVATE_DATA_FROM_THIS (This);

  //
  // if no physical console input device exists,
  // return EFI_SUCCESS directly.
  //
  if (Private->CurrentNumberOfExConsoles <= 0) {
    return EFI_SUCCESS;
  }

  //
  // Return EFI_SUCCESS if the (KeyData, NotificationFunction) is already registered.
  //
  for (Link = Private->NotifyList.ForwardLink; Link != &Private->NotifyList; Link = Link->ForwardLink) {
    CurrentNotify = CR (
                      Link,
                      TEXT_IN_EX_SPLITTER_NOTIFY,
                      NotifyEntry,
                      TEXT_IN_EX_SPLITTER_NOTIFY_SIGNATURE
                      );
    if (IsKeyRegistered (&CurrentNotify->KeyData, KeyData)) {
      if (CurrentNotify->KeyNotificationFn == KeyNotificationFunction) {
        *NotifyHandle = CurrentNotify->NotifyHandle;
        return EFI_SUCCESS;
      }
    }
  }

  //
  // Allocate resource to save the notification function
  //
  NewNotify = (TEXT_IN_EX_SPLITTER_NOTIFY *) AllocateZeroPool (sizeof (TEXT_IN_EX_SPLITTER_NOTIFY));
  if (NewNotify == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }
  NewNotify->NotifyHandleList = (EFI_HANDLE *) AllocateZeroPool (sizeof (EFI_HANDLE) * Private->CurrentNumberOfExConsoles);
  if (NewNotify->NotifyHandleList == NULL) {
    gBS->FreePool (NewNotify);
    return EFI_OUT_OF_RESOURCES;
  }
  NewNotify->Signature         = TEXT_IN_EX_SPLITTER_NOTIFY_SIGNATURE;
  NewNotify->KeyNotificationFn = KeyNotificationFunction;
  CopyMem (&NewNotify->KeyData, KeyData, sizeof (KeyData));

  //
  // Return the wrong status of registering key notify of
  // physical console input device if meet problems
  //
  for (Index = 0; Index < Private->CurrentNumberOfExConsoles; Index++) {
    Status = Private->TextInExList[Index]->RegisterKeyNotify (
                                             Private->TextInExList[Index],
                                             KeyData,
                                             KeyNotificationFunction,
                                             &NewNotify->NotifyHandleList[Index]
                                             );
    if (EFI_ERROR (Status)) {
      gBS->FreePool (NewNotify->NotifyHandleList);
      gBS->FreePool (NewNotify);
      return Status;
    }
  }

  //
  // Use gSimpleTextInExNotifyGuid to get a valid EFI_HANDLE
  //
  Status = gBS->InstallMultipleProtocolInterfaces (
                  &NewNotify->NotifyHandle,
                  &gSimpleTextInExNotifyGuid,
                  NULL,
                  NULL
                  );
  ASSERT_EFI_ERROR (Status);

  InsertTailList (&mConIn.NotifyList, &NewNotify->NotifyEntry);

  *NotifyHandle                = NewNotify->NotifyHandle;

  return EFI_SUCCESS;

}

EFI_STATUS
EFIAPI
ConSplitterTextInUnregisterKeyNotify (
  IN EFI_SIMPLE_TEXT_INPUT_EX_PROTOCOL  *This,
  IN EFI_HANDLE                         NotificationHandle
  )
/*++

  Routine Description:
    Remove a registered notification function from a particular keystroke.

  Arguments:
    This                    - Protocol instance pointer.
    NotificationHandle      - The handle of the notification function being unregistered.

  Returns:
    EFI_SUCCESS             - The notification function was unregistered successfully.
    EFI_INVALID_PARAMETER   - The NotificationHandle is invalid.
    EFI_NOT_FOUND           - Can not find the matching entry in database.

--*/
{
  TEXT_IN_SPLITTER_PRIVATE_DATA *Private;
  EFI_STATUS                    Status;
  UINTN                         Index;
  TEXT_IN_EX_SPLITTER_NOTIFY    *CurrentNotify;
  LIST_ENTRY                    *Link;

  if (NotificationHandle == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  Status = gBS->OpenProtocol (
                  NotificationHandle,
                  &gSimpleTextInExNotifyGuid,
                  NULL,
                  NULL,
                  NULL,
                  EFI_OPEN_PROTOCOL_TEST_PROTOCOL
                  );
  if (EFI_ERROR (Status)) {
    return EFI_INVALID_PARAMETER;
  }

  Private = TEXT_IN_EX_SPLITTER_PRIVATE_DATA_FROM_THIS (This);

  //
  // if no physical console input device exists,
  // return EFI_SUCCESS directly.
  //
  if (Private->CurrentNumberOfExConsoles <= 0) {
    return EFI_SUCCESS;
  }

  for (Link = Private->NotifyList.ForwardLink; Link != &Private->NotifyList; Link = Link->ForwardLink) {
    CurrentNotify = CR (Link, TEXT_IN_EX_SPLITTER_NOTIFY, NotifyEntry, TEXT_IN_EX_SPLITTER_NOTIFY_SIGNATURE);
    if (CurrentNotify->NotifyHandle == NotificationHandle) {
      for (Index = 0; Index < Private->CurrentNumberOfExConsoles; Index++) {
        Status = Private->TextInExList[Index]->UnregisterKeyNotify (
                                                 Private->TextInExList[Index],
                                                 CurrentNotify->NotifyHandleList[Index]
                                                 );
        if (EFI_ERROR (Status)) {
          return Status;
        }
      }
      RemoveEntryList (&CurrentNotify->NotifyEntry);
      Status = gBS->UninstallMultipleProtocolInterfaces (
                      CurrentNotify->NotifyHandle,
                      &gSimpleTextInExNotifyGuid,
                      NULL,
                      NULL
                      );
      ASSERT_EFI_ERROR (Status);
      gBS->FreePool (CurrentNotify->NotifyHandleList);
      gBS->FreePool (CurrentNotify);
      return EFI_SUCCESS;
    }
  }

  return EFI_NOT_FOUND;

}

EFI_STATUS
EFIAPI
ConSplitterSimplePointerReset (
  IN  EFI_SIMPLE_POINTER_PROTOCOL     *This,
  IN  BOOLEAN                         ExtendedVerification
  )
/*++

  Routine Description:
    Reset the input device and optionaly run diagnostics

  Arguments:
    This                 - Protocol instance pointer.
    ExtendedVerification - Driver may perform diagnostics on reset.

  Returns:
    EFI_SUCCESS           - The device was reset.
    EFI_DEVICE_ERROR      - The device is not functioning properly and could
                            not be reset.

--*/
{
  EFI_STATUS                    Status;
  EFI_STATUS                    ReturnStatus;
  TEXT_IN_SPLITTER_PRIVATE_DATA *Private;
  UINTN                         Index;

  Private                         = TEXT_IN_SPLITTER_PRIVATE_DATA_FROM_SIMPLE_POINTER_THIS (This);

  Private->InputEventSignalState  = FALSE;

  if (Private->CurrentNumberOfPointers == 0) {
    return EFI_SUCCESS;
  }
  //
  // return the worst status met
  //
  for (Index = 0, ReturnStatus = EFI_SUCCESS; Index < Private->CurrentNumberOfPointers; Index++) {
    Status = Private->PointerList[Index]->Reset (
                                            Private->PointerList[Index],
                                            ExtendedVerification
                                            );
    if (EFI_ERROR (Status)) {
      ReturnStatus = Status;
    }
  }

  return ReturnStatus;
}

STATIC
EFI_STATUS
EFIAPI
ConSplitterSimplePointerPrivateGetState (
  IN  TEXT_IN_SPLITTER_PRIVATE_DATA   *Private,
  IN OUT EFI_SIMPLE_POINTER_STATE     *State
  )
/*++

  Routine Description:
    Reads the next keystroke from the input device. The WaitForKey Event can
    be used to test for existance of a keystroke via WaitForEvent () call.

  Arguments:
    This   - Protocol instance pointer.
    State  -

  Returns:
    EFI_SUCCESS       - The keystroke information was returned.
    EFI_NOT_READY     - There was no keystroke data availiable.
    EFI_DEVICE_ERROR  - The keydtroke information was not returned due to
                        hardware errors.

--*/
{
  EFI_STATUS                Status;
  EFI_STATUS                ReturnStatus;
  UINTN                     Index;
  EFI_SIMPLE_POINTER_STATE  CurrentState;

  State->RelativeMovementX  = 0;
  State->RelativeMovementY  = 0;
  State->RelativeMovementZ  = 0;
  State->LeftButton         = FALSE;
  State->RightButton        = FALSE;

  //
  // if no physical console input device exists, return EFI_NOT_READY;
  // if any physical console input device has key input,
  // return the key and EFI_SUCCESS.
  //
  ReturnStatus = EFI_NOT_READY;
  for (Index = 0; Index < Private->CurrentNumberOfPointers; Index++) {

    Status = Private->PointerList[Index]->GetState (
                                            Private->PointerList[Index],
                                            &CurrentState
                                            );
    if (!EFI_ERROR (Status)) {
      if (ReturnStatus == EFI_NOT_READY) {
        ReturnStatus = EFI_SUCCESS;
      }

      if (CurrentState.LeftButton) {
        State->LeftButton = TRUE;
      }

      if (CurrentState.RightButton) {
        State->RightButton = TRUE;
      }

      if (CurrentState.RelativeMovementX != 0 && Private->PointerList[Index]->Mode->ResolutionX != 0) {
        State->RelativeMovementX += (CurrentState.RelativeMovementX * (INT32) Private->SimplePointerMode.ResolutionX) / (INT32) Private->PointerList[Index]->Mode->ResolutionX;
      }

      if (CurrentState.RelativeMovementY != 0 && Private->PointerList[Index]->Mode->ResolutionY != 0) {
        State->RelativeMovementY += (CurrentState.RelativeMovementY * (INT32) Private->SimplePointerMode.ResolutionY) / (INT32) Private->PointerList[Index]->Mode->ResolutionY;
      }

      if (CurrentState.RelativeMovementZ != 0 && Private->PointerList[Index]->Mode->ResolutionZ != 0) {
        State->RelativeMovementZ += (CurrentState.RelativeMovementZ * (INT32) Private->SimplePointerMode.ResolutionZ) / (INT32) Private->PointerList[Index]->Mode->ResolutionZ;
      }
    } else if (Status == EFI_DEVICE_ERROR) {
      ReturnStatus = EFI_DEVICE_ERROR;
    }
  }

  return ReturnStatus;
}

EFI_STATUS
EFIAPI
ConSplitterSimplePointerGetState (
  IN  EFI_SIMPLE_POINTER_PROTOCOL     *This,
  IN OUT EFI_SIMPLE_POINTER_STATE     *State
  )
/*++

  Routine Description:
    Reads the next keystroke from the input device. The WaitForKey Event can
    be used to test for existance of a keystroke via WaitForEvent () call.
    If the ConIn is password locked make it look like no keystroke is availible

  Arguments:
    This   - Protocol instance pointer.
    State  -

  Returns:
    EFI_SUCCESS       - The keystroke information was returned.
    EFI_NOT_READY     - There was no keystroke data availiable.
    EFI_DEVICE_ERROR  - The keydtroke information was not returned due to
                        hardware errors.

--*/
{
  TEXT_IN_SPLITTER_PRIVATE_DATA *Private;

  Private = TEXT_IN_SPLITTER_PRIVATE_DATA_FROM_SIMPLE_POINTER_THIS (This);
  if (Private->PasswordEnabled) {
    //
    // If StdIn Locked return not ready
    //
    return EFI_NOT_READY;
  }

  Private->InputEventSignalState = FALSE;

  return ConSplitterSimplePointerPrivateGetState (Private, State);
}

VOID
EFIAPI
ConSplitterSimplePointerWaitForInput (
  IN  EFI_EVENT                       Event,
  IN  VOID                            *Context
  )
/*++

Routine Description:
  This event agregates all the events of the ConIn devices in the spliter.
  If the ConIn is password locked then return.
  If any events of physical ConIn devices are signaled, signal the ConIn
  spliter event. This will cause the calling code to call
  ConSplitterTextInReadKeyStroke ().

Arguments:
  Event   - The Event assoicated with callback.
  Context - Context registered when Event was created.

Returns:
  None

--*/
{
  EFI_STATUS                    Status;
  TEXT_IN_SPLITTER_PRIVATE_DATA *Private;
  UINTN                         Index;

  Private = (TEXT_IN_SPLITTER_PRIVATE_DATA *) Context;
  if (Private->PasswordEnabled) {
    //
    // If StdIn Locked return not ready
    //
    return ;
  }

  //
  // if InputEventSignalState is flagged before, and not cleared by Reset() or ReadKeyStroke()
  //
  if (Private->InputEventSignalState) {
    gBS->SignalEvent (Event);
    return ;
  }
  //
  // if any physical console input device has key input, signal the event.
  //
  for (Index = 0; Index < Private->CurrentNumberOfPointers; Index++) {
    Status = gBS->CheckEvent (Private->PointerList[Index]->WaitForInput);
    if (!EFI_ERROR (Status)) {
      gBS->SignalEvent (Event);
      Private->InputEventSignalState = TRUE;
    }
  }
}

//
// Absolute Pointer Protocol functions
//

EFI_STATUS
EFIAPI
ConSplitterAbsolutePointerReset (
  IN EFI_ABSOLUTE_POINTER_PROTOCOL   *This,
  IN BOOLEAN                         ExtendedVerification
  )
/*++

  Routine Description:
    Resets the pointer device hardware.

  Arguments:
    This                  - Protocol instance pointer.
    ExtendedVerification  - Driver may perform diagnostics on reset.

  Returns:
    EFI_SUCCESS           - The device was reset.
    EFI_DEVICE_ERROR      - The device is not functioning correctly and could
                            not be reset.

--*/
{
  EFI_STATUS                    Status;
  EFI_STATUS                    ReturnStatus;
  TEXT_IN_SPLITTER_PRIVATE_DATA *Private;
  UINTN                         Index;

  Private = TEXT_IN_SPLITTER_PRIVATE_DATA_FROM_ABSOLUTE_POINTER_THIS (This);

  Private->AbsoluteInputEventSignalState = FALSE;

  if (Private->CurrentNumberOfAbsolutePointers == 0) {
    return EFI_SUCCESS;
  }
  //
  // return the worst status met
  //
  for (Index = 0, ReturnStatus = EFI_SUCCESS; Index < Private->CurrentNumberOfAbsolutePointers; Index++) {
    Status = Private->AbsolutePointerList[Index]->Reset (
                                                    Private->AbsolutePointerList[Index],
                                                    ExtendedVerification
                                                    );
    if (EFI_ERROR (Status)) {
      ReturnStatus = Status;
    }
  }

  return ReturnStatus;
}

EFI_STATUS
EFIAPI
ConSplitterAbsolutePointerGetState (
  IN EFI_ABSOLUTE_POINTER_PROTOCOL   *This,
  IN OUT EFI_ABSOLUTE_POINTER_STATE  *State
  )
/*++

  Routine Description:
    Retrieves the current state of a pointer device.

  Arguments:
    This                  - Protocol instance pointer.
    State                 - A pointer to the state information on the pointer device.

  Returns:
    EFI_SUCCESS           - The state of the pointer device was returned in State..
    EFI_NOT_READY         - The state of the pointer device has not changed since the last call to
                            GetState().
    EFI_DEVICE_ERROR      - A device error occurred while attempting to retrieve the pointer
                            device's current state.
--*/
{
  TEXT_IN_SPLITTER_PRIVATE_DATA *Private;
  EFI_STATUS                    Status;
  EFI_STATUS                    ReturnStatus;
  UINTN                         Index;
  EFI_ABSOLUTE_POINTER_STATE    CurrentState;


  Private = TEXT_IN_SPLITTER_PRIVATE_DATA_FROM_ABSOLUTE_POINTER_THIS (This);
  if (Private->PasswordEnabled) {
    //
    // If StdIn Locked return not ready
    //
    return EFI_NOT_READY;
  }

  Private->AbsoluteInputEventSignalState = FALSE;

  State->CurrentX                        = 0;
  State->CurrentY                        = 0;
  State->CurrentZ                        = 0;
  State->ActiveButtons                   = 0;

  //
  // if no physical pointer device exists, return EFI_NOT_READY;
  // if any physical pointer device has changed state,
  // return the state and EFI_SUCCESS.
  //
  ReturnStatus = EFI_NOT_READY;
  for (Index = 0; Index < Private->CurrentNumberOfAbsolutePointers; Index++) {

    Status = Private->AbsolutePointerList[Index]->GetState (
                                                    Private->AbsolutePointerList[Index],
                                                    &CurrentState
                                                    );
    if (!EFI_ERROR (Status)) {
      if (ReturnStatus == EFI_NOT_READY) {
        ReturnStatus = EFI_SUCCESS;
      }

      State->ActiveButtons = CurrentState.ActiveButtons;

      if (!(Private->AbsolutePointerMode.AbsoluteMinX == 0 && Private->AbsolutePointerMode.AbsoluteMaxX == 0)) {
        State->CurrentX = CurrentState.CurrentX;
      }
      if (!(Private->AbsolutePointerMode.AbsoluteMinY == 0 && Private->AbsolutePointerMode.AbsoluteMaxY == 0)) {
        State->CurrentY = CurrentState.CurrentY;
      }
      if (!(Private->AbsolutePointerMode.AbsoluteMinZ == 0 && Private->AbsolutePointerMode.AbsoluteMaxZ == 0)) {
        State->CurrentZ = CurrentState.CurrentZ;
      }

    } else if (Status == EFI_DEVICE_ERROR) {
      ReturnStatus = EFI_DEVICE_ERROR;
    }
  }

  return ReturnStatus;
}

VOID
EFIAPI
ConSplitterAbsolutePointerWaitForInput (
  IN  EFI_EVENT                       Event,
  IN  VOID                            *Context
  )
/*++

Routine Description:
  This event agregates all the events of the pointer devices in the splitter.
  If the ConIn is password locked then return.
  If any events of physical pointer devices are signaled, signal the pointer
  splitter event. This will cause the calling code to call
  ConSplitterAbsolutePointerGetState ().

Arguments:
  Event   - The Event assoicated with callback.
  Context - Context registered when Event was created.

Returns:
  None

--*/
{
  EFI_STATUS                    Status;
  TEXT_IN_SPLITTER_PRIVATE_DATA *Private;
  UINTN                         Index;

  Private = (TEXT_IN_SPLITTER_PRIVATE_DATA *) Context;
  if (Private->PasswordEnabled) {
    //
    // If StdIn Locked return not ready
    //
    return ;
  }

  //
  // if AbsoluteInputEventSignalState is flagged before,
  // and not cleared by Reset() or GetState(), signal it
  //
  if (Private->AbsoluteInputEventSignalState) {
    gBS->SignalEvent (Event);
    return ;
  }
  //
  // if any physical console input device has key input, signal the event.
  //
  for (Index = 0; Index < Private->CurrentNumberOfAbsolutePointers; Index++) {
    Status = gBS->CheckEvent (Private->AbsolutePointerList[Index]->WaitForInput);
    if (!EFI_ERROR (Status)) {
      gBS->SignalEvent (Event);
      Private->AbsoluteInputEventSignalState = TRUE;
    }
  }
}

EFI_STATUS
EFIAPI
ConSplitterTextOutReset (
  IN  EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL    *This,
  IN  BOOLEAN                            ExtendedVerification
  )
/*++

  Routine Description:
    Reset the text output device hardware and optionaly run diagnostics

  Arguments:
    This                 - Protocol instance pointer.
    ExtendedVerification - Driver may perform more exhaustive verfication
                           operation of the device during reset.

  Returns:
    EFI_SUCCESS       - The text output device was reset.
    EFI_DEVICE_ERROR  - The text output device is not functioning correctly and
                        could not be reset.

--*/
{
  EFI_STATUS                      Status;
  TEXT_OUT_SPLITTER_PRIVATE_DATA  *Private;
  UINTN                           Index;
  EFI_STATUS                      ReturnStatus;

  Private = TEXT_OUT_SPLITTER_PRIVATE_DATA_FROM_THIS (This);

  //
  // return the worst status met
  //
  for (Index = 0, ReturnStatus = EFI_SUCCESS; Index < Private->CurrentNumberOfConsoles; Index++) {

    if (Private->TextOutList[Index].TextOutEnabled) {

      Status = Private->TextOutList[Index].TextOut->Reset (
                                                      Private->TextOutList[Index].TextOut,
                                                      ExtendedVerification
                                                      );
      if (EFI_ERROR (Status)) {
        ReturnStatus = Status;
      }
    }
  }

  This->SetAttribute (This, EFI_TEXT_ATTR (This->Mode->Attribute & 0x0F, EFI_BACKGROUND_BLACK));

  Status = DevNullTextOutSetMode (Private, 0);
  if (EFI_ERROR (Status)) {
    ReturnStatus = Status;
  }

  return ReturnStatus;
}

EFI_STATUS
EFIAPI
ConSplitterTextOutOutputString (
  IN  EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL    *This,
  IN  CHAR16                             *WString
  )
/*++

  Routine Description:
    Write a Unicode string to the output device.

  Arguments:
    This    - Protocol instance pointer.
    String  - The NULL-terminated Unicode string to be displayed on the output
              device(s). All output devices must also support the Unicode
              drawing defined in this file.

  Returns:
    EFI_SUCCESS       - The string was output to the device.
    EFI_DEVICE_ERROR  - The device reported an error while attempting to output
                         the text.
    EFI_UNSUPPORTED        - The output device's mode is not currently in a
                              defined text mode.
    EFI_WARN_UNKNOWN_GLYPH - This warning code indicates that some of the
                              characters in the Unicode string could not be
                              rendered and were skipped.

--*/
{
  EFI_STATUS                      Status;
  TEXT_OUT_SPLITTER_PRIVATE_DATA  *Private;
  UINTN                           Index;
  UINTN                           BackSpaceCount;
  EFI_STATUS                      ReturnStatus;
  CHAR16                          *TargetString;

  This->SetAttribute (This, This->Mode->Attribute);

  Private         = TEXT_OUT_SPLITTER_PRIVATE_DATA_FROM_THIS (This);

  BackSpaceCount  = 0;
  for (TargetString = WString; *TargetString; TargetString++) {
    if (*TargetString == CHAR_BACKSPACE) {
      BackSpaceCount++;
    }

  }

  if (BackSpaceCount == 0) {
    TargetString = WString;
  } else {
    TargetString = AllocatePool (sizeof (CHAR16) * (StrLen (WString) + BackSpaceCount + 1));
    StrCpy (TargetString, WString);
  }
  //
  // return the worst status met
  //
  Status = DevNullTextOutOutputString (Private, TargetString);
  if (EFI_ERROR (Status)) {
    ReturnStatus = Status;
  }

  for (Index = 0, ReturnStatus = EFI_SUCCESS; Index < Private->CurrentNumberOfConsoles; Index++) {

    if (Private->TextOutList[Index].TextOutEnabled) {
      Status = Private->TextOutList[Index].TextOut->OutputString (
                                                      Private->TextOutList[Index].TextOut,
                                                      TargetString
                                                      );
      if (EFI_ERROR (Status)) {
        ReturnStatus = Status;
      }
    }
  }

  if (BackSpaceCount) {
    FreePool (TargetString);
  }

  return ReturnStatus;
}

EFI_STATUS
EFIAPI
ConSplitterTextOutTestString (
  IN  EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL    *This,
  IN  CHAR16                             *WString
  )
/*++

  Routine Description:
    Verifies that all characters in a Unicode string can be output to the
    target device.

  Arguments:
    This    - Protocol instance pointer.
    String  - The NULL-terminated Unicode string to be examined for the output
               device(s).

  Returns:
    EFI_SUCCESS     - The device(s) are capable of rendering the output string.
    EFI_UNSUPPORTED - Some of the characters in the Unicode string cannot be
                       rendered by one or more of the output devices mapped
                       by the EFI handle.

--*/
{
  EFI_STATUS                      Status;
  TEXT_OUT_SPLITTER_PRIVATE_DATA  *Private;
  UINTN                           Index;
  EFI_STATUS                      ReturnStatus;

  Private = TEXT_OUT_SPLITTER_PRIVATE_DATA_FROM_THIS (This);

  //
  // return the worst status met
  //
  for (Index = 0, ReturnStatus = EFI_SUCCESS; Index < Private->CurrentNumberOfConsoles; Index++) {
    if (Private->TextOutList[Index].TextOutEnabled) {
      Status = Private->TextOutList[Index].TextOut->TestString (
                                                      Private->TextOutList[Index].TextOut,
                                                      WString
                                                      );
      if (EFI_ERROR (Status)) {
        ReturnStatus = Status;
      }
    }
  }
  //
  // There is no DevNullTextOutTestString () since a Unicode buffer would
  // always return EFI_SUCCESS.
  // ReturnStatus will be EFI_SUCCESS if no consoles are present
  //
  return ReturnStatus;
}

EFI_STATUS
EFIAPI
ConSplitterTextOutQueryMode (
  IN  EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL    *This,
  IN  UINTN                              ModeNumber,
  OUT UINTN                              *Columns,
  OUT UINTN                              *Rows
  )
/*++

  Routine Description:
    Returns information for an available text mode that the output device(s)
    supports.

  Arguments:
    This       - Protocol instance pointer.
    ModeNumber - The mode number to return information on.
    Columns, Rows - Returns the geometry of the text output device for the
                    requested ModeNumber.

  Returns:
    EFI_SUCCESS      - The requested mode information was returned.
    EFI_DEVICE_ERROR - The device had an error and could not
                       complete the request.
    EFI_UNSUPPORTED - The mode number was not valid.

--*/
{
  TEXT_OUT_SPLITTER_PRIVATE_DATA  *Private;
  UINTN                           CurrentMode;
  INT32                           *TextOutModeMap;

  Private = TEXT_OUT_SPLITTER_PRIVATE_DATA_FROM_THIS (This);

  //
  // Check whether param ModeNumber is valid.
  // ModeNumber should be within range 0 ~ MaxMode - 1.
  //
  if ( (ModeNumber > (UINTN)(((UINT32)-1)>>1)) ) {
    return EFI_UNSUPPORTED;
  }

  if ((INT32) ModeNumber >= This->Mode->MaxMode) {
    return EFI_UNSUPPORTED;
  }

  //
  // We get the available mode from mode intersection map if it's available
  //
  if (Private->TextOutModeMap != NULL) {
    TextOutModeMap = Private->TextOutModeMap + Private->TextOutListCount * ModeNumber;
    CurrentMode    = (UINTN)(*TextOutModeMap);
    *Columns       = Private->TextOutQueryData[CurrentMode].Columns;
    *Rows          = Private->TextOutQueryData[CurrentMode].Rows;
  } else {
    *Columns  = Private->TextOutQueryData[ModeNumber].Columns;
    *Rows     = Private->TextOutQueryData[ModeNumber].Rows;
  }

  if (*Columns <= 0 && *Rows <= 0) {
    return EFI_UNSUPPORTED;

  }

  return EFI_SUCCESS;
}

EFI_STATUS
EFIAPI
ConSplitterTextOutSetMode (
  IN  EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL    *This,
  IN  UINTN                              ModeNumber
  )
/*++

  Routine Description:
    Sets the output device(s) to a specified mode.

  Arguments:
    This       - Protocol instance pointer.
    ModeNumber - The mode number to set.

  Returns:
    EFI_SUCCESS      - The requested text mode was set.
    EFI_DEVICE_ERROR - The device had an error and
                       could not complete the request.
    EFI_UNSUPPORTED - The mode number was not valid.

--*/
{
  EFI_STATUS                      Status;
  TEXT_OUT_SPLITTER_PRIVATE_DATA  *Private;
  UINTN                           Index;
  INT32                           *TextOutModeMap;
  EFI_STATUS                      ReturnStatus;

  Private = TEXT_OUT_SPLITTER_PRIVATE_DATA_FROM_THIS (This);

  //
  // Check whether param ModeNumber is valid.
  // ModeNumber should be within range 0 ~ MaxMode - 1.
  //
  if ( (ModeNumber > (UINTN)(((UINT32)-1)>>1)) ) {
    return EFI_UNSUPPORTED;
  }

  if ((INT32) ModeNumber >= This->Mode->MaxMode) {
    return EFI_UNSUPPORTED;
  }
  //
  // If the mode is being set to the curent mode, then just clear the screen and return.
  //
  if (Private->TextOutMode.Mode == (INT32) ModeNumber) {
    return ConSplitterTextOutClearScreen (This);
  }
  //
  // return the worst status met
  //
  TextOutModeMap = Private->TextOutModeMap + Private->TextOutListCount * ModeNumber;
  for (Index = 0, ReturnStatus = EFI_SUCCESS; Index < Private->CurrentNumberOfConsoles; Index++) {

    if (Private->TextOutList[Index].TextOutEnabled) {
      Status = Private->TextOutList[Index].TextOut->SetMode (
                                                      Private->TextOutList[Index].TextOut,
                                                      TextOutModeMap[Index]
                                                      );
      //
      // If this console device is based on a GOP or UGA device, then sync up the bitmap from
      // the GOP/UGA splitter and reclear the text portion of the display in the new mode.
      //
      if ((Private->TextOutList[Index].GraphicsOutput != NULL) || (Private->TextOutList[Index].UgaDraw != NULL)) {
        Private->TextOutList[Index].TextOut->ClearScreen (Private->TextOutList[Index].TextOut);
      }

      if (EFI_ERROR (Status)) {
        ReturnStatus = Status;
      }
    }
  }
  //
  // The DevNull Console will support any possible mode as it allocates memory
  //
  Status = DevNullTextOutSetMode (Private, ModeNumber);
  if (EFI_ERROR (Status)) {
    ReturnStatus = Status;
  }

  return ReturnStatus;
}

EFI_STATUS
EFIAPI
ConSplitterTextOutSetAttribute (
  IN  EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL    *This,
  IN  UINTN                              Attribute
  )
/*++

  Routine Description:
    Sets the background and foreground colors for the OutputString () and
    ClearScreen () functions.

  Arguments:
    This      - Protocol instance pointer.
    Attribute - The attribute to set. Bits 0..3 are the foreground color, and
                bits 4..6 are the background color. All other bits are undefined
                and must be zero. The valid Attributes are defined in this file.

  Returns:
    EFI_SUCCESS      - The attribute was set.
    EFI_DEVICE_ERROR - The device had an error and
                       could not complete the request.
    EFI_UNSUPPORTED - The attribute requested is not defined.

--*/
{
  EFI_STATUS                      Status;
  TEXT_OUT_SPLITTER_PRIVATE_DATA  *Private;
  UINTN                           Index;
  EFI_STATUS                      ReturnStatus;

  Private = TEXT_OUT_SPLITTER_PRIVATE_DATA_FROM_THIS (This);

  //
  // Check whether param Attribute is valid.
  //
  if ( (Attribute > (UINTN)(((UINT32)-1)>>1)) ) {
    return EFI_UNSUPPORTED;
  }

  //
  // return the worst status met
  //
  for (Index = 0, ReturnStatus = EFI_SUCCESS; Index < Private->CurrentNumberOfConsoles; Index++) {

    if (Private->TextOutList[Index].TextOutEnabled) {
      Status = Private->TextOutList[Index].TextOut->SetAttribute (
                                                      Private->TextOutList[Index].TextOut,
                                                      Attribute
                                                      );
      if (EFI_ERROR (Status)) {
        ReturnStatus = Status;
      }
    }
  }

  Private->TextOutMode.Attribute = (INT32) Attribute;

  return ReturnStatus;
}

EFI_STATUS
EFIAPI
ConSplitterTextOutClearScreen (
  IN  EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL    *This
  )
/*++

  Routine Description:
    Clears the output device(s) display to the currently selected background
    color.

  Arguments:
    This      - Protocol instance pointer.

  Returns:
    EFI_SUCCESS      - The operation completed successfully.
    EFI_DEVICE_ERROR - The device had an error and
                       could not complete the request.
    EFI_UNSUPPORTED - The output device is not in a valid text mode.

--*/
{
  EFI_STATUS                      Status;
  TEXT_OUT_SPLITTER_PRIVATE_DATA  *Private;
  UINTN                           Index;
  EFI_STATUS                      ReturnStatus;

  Private = TEXT_OUT_SPLITTER_PRIVATE_DATA_FROM_THIS (This);

  //
  // return the worst status met
  //
  for (Index = 0, ReturnStatus = EFI_SUCCESS; Index < Private->CurrentNumberOfConsoles; Index++) {

    if (Private->TextOutList[Index].TextOutEnabled) {
      Status = Private->TextOutList[Index].TextOut->ClearScreen (Private->TextOutList[Index].TextOut);
      if (EFI_ERROR (Status)) {
        ReturnStatus = Status;
      }
    }
  }

  Status = DevNullTextOutClearScreen (Private);
  if (EFI_ERROR (Status)) {
    ReturnStatus = Status;
  }

  return ReturnStatus;
}

EFI_STATUS
EFIAPI
ConSplitterTextOutSetCursorPosition (
  IN  EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL    *This,
  IN  UINTN                              Column,
  IN  UINTN                              Row
  )
/*++

  Routine Description:
    Sets the current coordinates of the cursor position

  Arguments:
    This        - Protocol instance pointer.
    Column, Row - the position to set the cursor to. Must be greater than or
                  equal to zero and less than the number of columns and rows
                  by QueryMode ().

  Returns:
    EFI_SUCCESS      - The operation completed successfully.
    EFI_DEVICE_ERROR - The device had an error and
                       could not complete the request.
    EFI_UNSUPPORTED - The output device is not in a valid text mode, or the
                       cursor position is invalid for the current mode.

--*/
{
  EFI_STATUS                      Status;
  TEXT_OUT_SPLITTER_PRIVATE_DATA  *Private;
  UINTN                           Index;
  EFI_STATUS                      ReturnStatus;
  UINTN                           MaxColumn;
  UINTN                           MaxRow;
  INT32                           *TextOutModeMap;
  INT32                           ModeNumber;
  INT32                           CurrentMode;

  Private   = TEXT_OUT_SPLITTER_PRIVATE_DATA_FROM_THIS (This);
  TextOutModeMap  = NULL;
  ModeNumber      = Private->TextOutMode.Mode;

  //
  // Get current MaxColumn and MaxRow from intersection map
  //
  if (Private->TextOutModeMap != NULL) {
    TextOutModeMap = Private->TextOutModeMap + Private->TextOutListCount * ModeNumber;
    CurrentMode    = *TextOutModeMap;
  } else {
    CurrentMode = ModeNumber;
  }

  MaxColumn = Private->TextOutQueryData[CurrentMode].Columns;
  MaxRow    = Private->TextOutQueryData[CurrentMode].Rows;

  if (Column >= MaxColumn || Row >= MaxRow) {
    return EFI_UNSUPPORTED;
  }
  //
  // return the worst status met
  //
  for (Index = 0, ReturnStatus = EFI_SUCCESS; Index < Private->CurrentNumberOfConsoles; Index++) {

    if (Private->TextOutList[Index].TextOutEnabled) {
      Status = Private->TextOutList[Index].TextOut->SetCursorPosition (
                                                      Private->TextOutList[Index].TextOut,
                                                      Column,
                                                      Row
                                                      );
      if (EFI_ERROR (Status)) {
        ReturnStatus = Status;
      }
    }
  }

  DevNullTextOutSetCursorPosition (Private, Column, Row);

  return ReturnStatus;
}

EFI_STATUS
EFIAPI
ConSplitterTextOutEnableCursor (
  IN  EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL    *This,
  IN  BOOLEAN                            Visible
  )
/*++

  Routine Description:
    Makes the cursor visible or invisible

  Arguments:
    This    - Protocol instance pointer.
    Visible - If TRUE, the cursor is set to be visible. If FALSE, the cursor is
              set to be invisible.

  Returns:
    EFI_SUCCESS      - The operation completed successfully.
    EFI_DEVICE_ERROR - The device had an error and could not complete the
                        request, or the device does not support changing
                        the cursor mode.
    EFI_UNSUPPORTED - The output device is not in a valid text mode.

--*/
{
  EFI_STATUS                      Status;
  TEXT_OUT_SPLITTER_PRIVATE_DATA  *Private;
  UINTN                           Index;
  EFI_STATUS                      ReturnStatus;

  Private = TEXT_OUT_SPLITTER_PRIVATE_DATA_FROM_THIS (This);

  //
  // return the worst status met
  //
  for (Index = 0, ReturnStatus = EFI_SUCCESS; Index < Private->CurrentNumberOfConsoles; Index++) {

    if (Private->TextOutList[Index].TextOutEnabled) {
      Status = Private->TextOutList[Index].TextOut->EnableCursor (
                                                      Private->TextOutList[Index].TextOut,
                                                      Visible
                                                      );
      if (EFI_ERROR (Status)) {
        ReturnStatus = Status;
      }
    }
  }

  DevNullTextOutEnableCursor (Private, Visible);

  return ReturnStatus;
}

