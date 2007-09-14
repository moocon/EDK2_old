/**@file
  This driver produces a VGA console.

Copyright (c) 2006, Intel Corporation                                                         
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution.  The full text of the license may be found at        
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

**/

#include "VgaClass.h"

//
// EFI Driver Binding Protocol for the VGA Class Driver
//
EFI_DRIVER_BINDING_PROTOCOL gVgaClassDriverBinding = {
  VgaClassDriverBindingSupported,
  VgaClassDriverBindingStart,
  VgaClassDriverBindingStop,
  0xa,
  NULL,
  NULL
};

/**
  The user Entry Point for module VgaClass. The user code starts with this function.

  @param[in] ImageHandle    The firmware allocated handle for the EFI image.  
  @param[in] SystemTable    A pointer to the EFI System Table.
  
  @retval EFI_SUCCESS       The entry point is executed successfully.
  @retval other             Some error occurs when executing this entry point.

**/
EFI_STATUS
EFIAPI
InitializeVgaClass(
  IN EFI_HANDLE           ImageHandle,
  IN EFI_SYSTEM_TABLE     *SystemTable
  )
{
  EFI_STATUS              Status;

  //
  // Install driver model protocol(s).
  //
  Status = EfiLibInstallAllDriverProtocols (
             ImageHandle,
             SystemTable,
             &gVgaClassDriverBinding,
             ImageHandle,
             &gVgaClassComponentName,
             NULL,
             NULL
             );
  ASSERT_EFI_ERROR (Status);


  return Status;
}

//
// Local variables
//
static CHAR16               CrLfString[3] = { CHAR_CARRIAGE_RETURN, CHAR_LINEFEED, CHAR_NULL };

typedef struct {
  CHAR16  Unicode;
  CHAR8   PcAnsi;
  CHAR8   Ascii;
} UNICODE_TO_CHAR;

//
// This list is used to define the valid extend chars.
// It also provides a mapping from Unicode to PCANSI or
// ASCII. The ASCII mapping we just made up.
//
//
STATIC UNICODE_TO_CHAR  UnicodeToPcAnsiOrAscii[] = {
  {
    BOXDRAW_HORIZONTAL,
    0xc4,
    L'-'
  },
  {
    BOXDRAW_VERTICAL,
    0xb3,
    L'|'
  },
  {
    BOXDRAW_DOWN_RIGHT,
    0xda,
    L'/'
  },
  {
    BOXDRAW_DOWN_LEFT,
    0xbf,
    L'\\'
  },
  {
    BOXDRAW_UP_RIGHT,
    0xc0,
    L'\\'
  },
  {
    BOXDRAW_UP_LEFT,
    0xd9,
    L'/'
  },
  {
    BOXDRAW_VERTICAL_RIGHT,
    0xc3,
    L'|'
  },
  {
    BOXDRAW_VERTICAL_LEFT,
    0xb4,
    L'|'
  },
  {
    BOXDRAW_DOWN_HORIZONTAL,
    0xc2,
    L'+'
  },
  {
    BOXDRAW_UP_HORIZONTAL,
    0xc1,
    L'+'
  },
  {
    BOXDRAW_VERTICAL_HORIZONTAL,
    0xc5,
    L'+'
  },
  {
    BOXDRAW_DOUBLE_HORIZONTAL,
    0xcd,
    L'-'
  },
  {
    BOXDRAW_DOUBLE_VERTICAL,
    0xba,
    L'|'
  },
  {
    BOXDRAW_DOWN_RIGHT_DOUBLE,
    0xd5,
    L'/'
  },
  {
    BOXDRAW_DOWN_DOUBLE_RIGHT,
    0xd6,
    L'/'
  },
  {
    BOXDRAW_DOUBLE_DOWN_RIGHT,
    0xc9,
    L'/'
  },
  {
    BOXDRAW_DOWN_LEFT_DOUBLE,
    0xb8,
    L'\\'
  },
  {
    BOXDRAW_DOWN_DOUBLE_LEFT,
    0xb7,
    L'\\'
  },
  {
    BOXDRAW_DOUBLE_DOWN_LEFT,
    0xbb,
    L'\\'
  },
  {
    BOXDRAW_UP_RIGHT_DOUBLE,
    0xd4,
    L'\\'
  },
  {
    BOXDRAW_UP_DOUBLE_RIGHT,
    0xd3,
    L'\\'
  },
  {
    BOXDRAW_DOUBLE_UP_RIGHT,
    0xc8,
    L'\\'
  },
  {
    BOXDRAW_UP_LEFT_DOUBLE,
    0xbe,
    L'/'
  },
  {
    BOXDRAW_UP_DOUBLE_LEFT,
    0xbd,
    L'/'
  },
  {
    BOXDRAW_DOUBLE_UP_LEFT,
    0xbc,
    L'/'
  },
  {
    BOXDRAW_VERTICAL_RIGHT_DOUBLE,
    0xc6,
    L'|'
  },
  {
    BOXDRAW_VERTICAL_DOUBLE_RIGHT,
    0xc7,
    L'|'
  },
  {
    BOXDRAW_DOUBLE_VERTICAL_RIGHT,
    0xcc,
    L'|'
  },
  {
    BOXDRAW_VERTICAL_LEFT_DOUBLE,
    0xb5,
    L'|'
  },
  {
    BOXDRAW_VERTICAL_DOUBLE_LEFT,
    0xb6,
    L'|'
  },
  {
    BOXDRAW_DOUBLE_VERTICAL_LEFT,
    0xb9,
    L'|'
  },
  {
    BOXDRAW_DOWN_HORIZONTAL_DOUBLE,
    0xd1,
    L'+'
  },
  {
    BOXDRAW_DOWN_DOUBLE_HORIZONTAL,
    0xd2,
    L'+'
  },
  {
    BOXDRAW_DOUBLE_DOWN_HORIZONTAL,
    0xcb,
    L'+'
  },
  {
    BOXDRAW_UP_HORIZONTAL_DOUBLE,
    0xcf,
    L'+'
  },
  {
    BOXDRAW_UP_DOUBLE_HORIZONTAL,
    0xd0,
    L'+'
  },
  {
    BOXDRAW_DOUBLE_UP_HORIZONTAL,
    0xca,
    L'+'
  },
  {
    BOXDRAW_VERTICAL_HORIZONTAL_DOUBLE,
    0xd8,
    L'+'
  },
  {
    BOXDRAW_VERTICAL_DOUBLE_HORIZONTAL,
    0xd7,
    L'+'
  },
  {
    BOXDRAW_DOUBLE_VERTICAL_HORIZONTAL,
    0xce,
    L'+'
  },

  {
    BLOCKELEMENT_FULL_BLOCK,
    0xdb,
    L'*'
  },
  {
    BLOCKELEMENT_LIGHT_SHADE,
    0xb0,
    L'+'
  },

  {
    GEOMETRICSHAPE_UP_TRIANGLE,
    0x1e,
    L'^'
  },
  {
    GEOMETRICSHAPE_RIGHT_TRIANGLE,
    0x10,
    L'>'
  },
  {
    GEOMETRICSHAPE_DOWN_TRIANGLE,
    0x1f,
    L'v'
  },
  {
    GEOMETRICSHAPE_LEFT_TRIANGLE,
    0x11,
    L'<'
  },

  {
    ARROW_LEFT,
    0x3c,
    L'<'
  },

  {
    ARROW_UP,
    0x18,
    L'^'
  },

  {
    ARROW_RIGHT,
    0x3e,
    L'>'
  },

  {
    ARROW_DOWN,
    0x19,
    L'v'
  },

  {
    0x0000,
    0x00,
    0x00
  }
};

//
// Private worker functions
//
STATIC
VOID
SetVideoCursorPosition (
  IN  VGA_CLASS_DEV  *VgaClassDev,
  IN  UINTN          Column,
  IN  UINTN          Row,
  IN  UINTN          MaxColumn
  );

STATIC
VOID
WriteCrtc (
  IN  VGA_CLASS_DEV  *VgaClassDev,
  IN  UINT16         Address,
  IN  UINT8          Data
  );

STATIC
BOOLEAN
LibIsValidTextGraphics (
  IN  CHAR16  Graphic,
  OUT CHAR8   *PcAnsi, OPTIONAL
  OUT CHAR8   *Ascii OPTIONAL
  );

STATIC
BOOLEAN
IsValidAscii (
  IN  CHAR16  Ascii
  );

STATIC
BOOLEAN
IsValidEfiCntlChar (
  IN  CHAR16  c
  );

EFI_STATUS
EFIAPI
VgaClassDriverBindingSupported (
  IN EFI_DRIVER_BINDING_PROTOCOL  *This,
  IN EFI_HANDLE                   Controller,
  IN EFI_DEVICE_PATH_PROTOCOL     *RemainingDevicePath
  )
/*++
  
  Routine Description:
  
    Supported.
    
  Arguments:
  
    (Standard DriverBinding Protocol Supported() function)
    
  Returns:
  
    EFI_STATUS
  
--*/
// GC_TODO:    This - add argument and description to function comment
// GC_TODO:    Controller - add argument and description to function comment
// GC_TODO:    RemainingDevicePath - add argument and description to function comment
{
  EFI_STATUS                  Status;
  EFI_VGA_MINI_PORT_PROTOCOL  *VgaMiniPort;

  //
  // Open the IO Abstraction(s) needed to perform the supported test
  //
  Status = gBS->OpenProtocol (
                  Controller,
                  &gEfiVgaMiniPortProtocolGuid,
                  (VOID **) &VgaMiniPort,
                  This->DriverBindingHandle,
                  Controller,
                  EFI_OPEN_PROTOCOL_TEST_PROTOCOL
                  );
  if (EFI_ERROR (Status)) {
    return Status;
  }
  //
  // Open the IO Abstraction(s) needed to perform the supported test
  //
  Status = gBS->OpenProtocol (
                  Controller,
                  &gEfiPciIoProtocolGuid,
                  NULL,
                  This->DriverBindingHandle,
                  Controller,
                  EFI_OPEN_PROTOCOL_TEST_PROTOCOL
                  );
  if (EFI_ERROR (Status)) {
    return Status;
  }

  return Status;
}

EFI_STATUS
EFIAPI
VgaClassDriverBindingStart (
  IN EFI_DRIVER_BINDING_PROTOCOL  *This,
  IN EFI_HANDLE                   Controller,
  IN EFI_DEVICE_PATH_PROTOCOL     *RemainingDevicePath
  )
/*++
  
  Routine Description:
  
    Layers the Simple Text Output Protocol on top of the 
    VGA Mini Port Protocol
  
  Arguments:
  
    (Standard DriverBinding Protocol Start() function)
    
  Returns:
  
    EFI_STATUS
    
--*/
// GC_TODO:    This - add argument and description to function comment
// GC_TODO:    Controller - add argument and description to function comment
// GC_TODO:    RemainingDevicePath - add argument and description to function comment
// GC_TODO:    EFI_OUT_OF_RESOURCES - add return value to function comment
{
  EFI_STATUS                  Status;
  EFI_VGA_MINI_PORT_PROTOCOL  *VgaMiniPort;
  EFI_PCI_IO_PROTOCOL         *PciIo;
  VGA_CLASS_DEV               *VgaClassPrivate;
  EFI_DEVICE_PATH_PROTOCOL    *DevicePath;

  Status = gBS->HandleProtocol (
                  Controller,
                  &gEfiDevicePathProtocolGuid,
                  (VOID **) &DevicePath
                  );
  if (EFI_ERROR (Status)) {
    return Status;
  }
  //
  // Report that VGA Class driver is being enabled
  //
  REPORT_STATUS_CODE_WITH_DEVICE_PATH (
    EFI_PROGRESS_CODE,
    EFI_PERIPHERAL_LOCAL_CONSOLE | EFI_P_PC_ENABLE,
    DevicePath
    );

  //
  // Open the IO Abstraction(s) needed
  //
  Status = gBS->OpenProtocol (
                  Controller,
                  &gEfiPciIoProtocolGuid,
                  (VOID **) &PciIo,
                  This->DriverBindingHandle,
                  Controller,
                  EFI_OPEN_PROTOCOL_GET_PROTOCOL
                  );
  if (EFI_ERROR (Status)) {
    return Status;
  }

  Status = gBS->OpenProtocol (
                  Controller,
                  &gEfiVgaMiniPortProtocolGuid,
                  (VOID **) &VgaMiniPort,
                  This->DriverBindingHandle,
                  Controller,
                  EFI_OPEN_PROTOCOL_BY_DRIVER
                  );
  if (EFI_ERROR (Status)) {
    return Status;
  }
  //
  // Allocate the private device structure
  //
  Status = gBS->AllocatePool (
                  EfiBootServicesData,
                  sizeof (VGA_CLASS_DEV),
                  (VOID **) &VgaClassPrivate
                  );
  if (EFI_ERROR (Status)) {
    gBS->CloseProtocol (
          Controller,
          &gEfiVgaMiniPortProtocolGuid,
          This->DriverBindingHandle,
          Controller
          );
    return EFI_OUT_OF_RESOURCES;
  }

  ZeroMem (VgaClassPrivate, sizeof (VGA_CLASS_DEV));

  //
  // Initialize the private device structure
  //
  VgaClassPrivate->Signature                        = VGA_CLASS_DEV_SIGNATURE;
  VgaClassPrivate->Handle = Controller;
  VgaClassPrivate->VgaMiniPort = VgaMiniPort;
  VgaClassPrivate->PciIo = PciIo;

  VgaClassPrivate->SimpleTextOut.Reset = VgaClassReset;
  VgaClassPrivate->SimpleTextOut.OutputString = VgaClassOutputString;
  VgaClassPrivate->SimpleTextOut.TestString = VgaClassTestString;
  VgaClassPrivate->SimpleTextOut.ClearScreen = VgaClassClearScreen;
  VgaClassPrivate->SimpleTextOut.SetAttribute = VgaClassSetAttribute;
  VgaClassPrivate->SimpleTextOut.SetCursorPosition = VgaClassSetCursorPosition;
  VgaClassPrivate->SimpleTextOut.EnableCursor = VgaClassEnableCursor;
  VgaClassPrivate->SimpleTextOut.QueryMode = VgaClassQueryMode;
  VgaClassPrivate->SimpleTextOut.SetMode = VgaClassSetMode;

  VgaClassPrivate->SimpleTextOut.Mode = &VgaClassPrivate->SimpleTextOutputMode;
  VgaClassPrivate->SimpleTextOutputMode.MaxMode = VgaMiniPort->MaxMode;
  VgaClassPrivate->DevicePath = DevicePath;

  Status = VgaClassPrivate->SimpleTextOut.SetAttribute (
                                            &VgaClassPrivate->SimpleTextOut,
                                            EFI_TEXT_ATTR (EFI_WHITE,
    EFI_BLACK)
                                            );
  if (EFI_ERROR (Status)) {
    goto ErrorExit;
  }

  Status = VgaClassPrivate->SimpleTextOut.Reset (
                                            &VgaClassPrivate->SimpleTextOut,
                                            FALSE
                                            );
  if (EFI_ERROR (Status)) {
    goto ErrorExit;
  }

  Status = VgaClassPrivate->SimpleTextOut.EnableCursor (
                                            &VgaClassPrivate->SimpleTextOut,
                                            TRUE
                                            );
  if (EFI_ERROR (Status)) {
    goto ErrorExit;
  }

  Status = gBS->InstallMultipleProtocolInterfaces (
                  &Controller,
                  &gEfiSimpleTextOutProtocolGuid,
                  &VgaClassPrivate->SimpleTextOut,
                  NULL
                  );

  return Status;

ErrorExit:
  REPORT_STATUS_CODE_WITH_DEVICE_PATH (
    EFI_ERROR_CODE | EFI_ERROR_MINOR,
    EFI_PERIPHERAL_LOCAL_CONSOLE | EFI_P_EC_CONTROLLER_ERROR,
    DevicePath
    );

  return Status;

}

EFI_STATUS
EFIAPI
VgaClassDriverBindingStop (
  IN  EFI_DRIVER_BINDING_PROTOCOL     *This,
  IN  EFI_HANDLE                      Controller,
  IN  UINTN                           NumberOfChildren,
  IN  EFI_HANDLE                      *ChildHandleBuffer
  )
/*++
  
  Routine Description:
  
    Stop.
  
  Arguments:
  
    (Standard DriverBinding Protocol Stop() function)
  
  Returns:
  
    EFI_STATUS
  
--*/
// GC_TODO:    This - add argument and description to function comment
// GC_TODO:    Controller - add argument and description to function comment
// GC_TODO:    NumberOfChildren - add argument and description to function comment
// GC_TODO:    ChildHandleBuffer - add argument and description to function comment
// GC_TODO:    EFI_SUCCESS - add return value to function comment
{
  EFI_STATUS                    Status;
  EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL  *SimpleTextOut;
  VGA_CLASS_DEV                 *VgaClassPrivate;

  Status = gBS->OpenProtocol (
                  Controller,
                  &gEfiSimpleTextOutProtocolGuid,
                  (VOID **) &SimpleTextOut,
                  This->DriverBindingHandle,
                  Controller,
                  EFI_OPEN_PROTOCOL_GET_PROTOCOL
                  );
  if (EFI_ERROR (Status)) {
    return Status;
  }

  VgaClassPrivate = VGA_CLASS_DEV_FROM_THIS (SimpleTextOut);

  //
  // Report that VGA Class driver is being disabled
  //
  REPORT_STATUS_CODE_WITH_DEVICE_PATH (
    EFI_PROGRESS_CODE,
    EFI_PERIPHERAL_LOCAL_CONSOLE | EFI_P_PC_DISABLE,
    VgaClassPrivate->DevicePath
    );

  Status = gBS->UninstallProtocolInterface (
                  Controller,
                  &gEfiSimpleTextOutProtocolGuid,
                  &VgaClassPrivate->SimpleTextOut
                  );
  if (EFI_ERROR (Status)) {
    return Status;
  }
  //
  // Release PCI I/O and VGA Mini Port Protocols on the controller handle.
  //
  gBS->CloseProtocol (
        Controller,
        &gEfiPciIoProtocolGuid,
        This->DriverBindingHandle,
        Controller
        );

  gBS->CloseProtocol (
        Controller,
        &gEfiVgaMiniPortProtocolGuid,
        This->DriverBindingHandle,
        Controller
        );

  gBS->FreePool (VgaClassPrivate);

  return EFI_SUCCESS;
}
//
// Simple Text Output Protocol Functions
//
EFI_STATUS
EFIAPI
VgaClassReset (
  IN  EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL        *This,
  IN  BOOLEAN                             ExtendedVerification
  )
/*++

Routine Description:

  GC_TODO: Add function description

Arguments:

  This                  - GC_TODO: add argument description
  ExtendedVerification  - GC_TODO: add argument description

Returns:

  GC_TODO: add return values

--*/
{
  EFI_STATUS    Status;
  VGA_CLASS_DEV *VgaClassPrivate;

  VgaClassPrivate = VGA_CLASS_DEV_FROM_THIS (This);

  REPORT_STATUS_CODE_WITH_DEVICE_PATH (
    EFI_PROGRESS_CODE,
    EFI_PERIPHERAL_LOCAL_CONSOLE | EFI_P_PC_RESET,
    VgaClassPrivate->DevicePath
    );

  This->SetAttribute (This, EFI_TEXT_ATTR (This->Mode->Attribute & 0x0F, EFI_BACKGROUND_BLACK));

  Status = This->SetMode (This, 0);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  return This->ClearScreen (This);
}

EFI_STATUS
EFIAPI
VgaClassOutputString (
  IN  EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL    *This,
  IN  CHAR16                          *WString
  )
/*++

Routine Description:

  GC_TODO: Add function description

Arguments:

  This    - GC_TODO: add argument description
  WString - GC_TODO: add argument description

Returns:

  EFI_SUCCESS - GC_TODO: Add description for return value

--*/
{
  EFI_STATUS                  Status;
  VGA_CLASS_DEV               *VgaClassDev;
  EFI_SIMPLE_TEXT_OUTPUT_MODE *Mode;
  UINTN                       MaxColumn;
  UINTN                       MaxRow;
  UINT32                      VideoChar;
  CHAR8                       GraphicChar;

  VgaClassDev = VGA_CLASS_DEV_FROM_THIS (This);

  Mode        = This->Mode;

  Status = This->QueryMode (
                  This,
                  Mode->Mode,
                  &MaxColumn,
                  &MaxRow
                  );
  if (EFI_ERROR (Status)) {
    return Status;
  }

  for (; *WString != CHAR_NULL; WString++) {

    switch (*WString) {
    case CHAR_BACKSPACE:
      if (Mode->CursorColumn > 0) {
        Mode->CursorColumn--;
      }
      break;

    case CHAR_LINEFEED:
      if (Mode->CursorRow == (INT32) (MaxRow - 1)) {
        //
        // Scroll the screen by copying the contents
        // of the VGA display up one line
        //
        VgaClassDev->PciIo->CopyMem (
                              VgaClassDev->PciIo,
                              EfiPciIoWidthUint32,
                              VgaClassDev->VgaMiniPort->VgaMemoryBar,
                              VgaClassDev->VgaMiniPort->VgaMemoryOffset,
                              VgaClassDev->VgaMiniPort->VgaMemoryBar,
                              VgaClassDev->VgaMiniPort->VgaMemoryOffset + MaxColumn * 2,
                              ((MaxRow - 1) * MaxColumn) >> 1
                              );

        //
        // Print Blank Line of spaces with the current color attributes
        //
        VideoChar = (Mode->Attribute << 8) | ' ';
        VideoChar = (VideoChar << 16) | VideoChar;
        VgaClassDev->PciIo->Mem.Write (
                                  VgaClassDev->PciIo,
                                  EfiPciIoWidthFillUint32,
                                  VgaClassDev->VgaMiniPort->VgaMemoryBar,
                                  VgaClassDev->VgaMiniPort->VgaMemoryOffset + (MaxRow - 1) * MaxColumn * 2,
                                  MaxColumn >> 1,
                                  &VideoChar
                                  );
      }

      if (Mode->CursorRow < (INT32) (MaxRow - 1)) {
        Mode->CursorRow++;
      }
      break;

    case CHAR_CARRIAGE_RETURN:
      Mode->CursorColumn = 0;
      break;

    default:
      if (!LibIsValidTextGraphics (*WString, &GraphicChar, NULL)) {
        //
        // Just convert to ASCII
        //
        GraphicChar = (CHAR8) *WString;
        if (!IsValidAscii (GraphicChar)) {
          //
          // Keep the API from supporting PCANSI Graphics chars
          //
          GraphicChar = '?';
        }
      }

      VideoChar = (Mode->Attribute << 8) | GraphicChar;
      VgaClassDev->PciIo->Mem.Write (
                                VgaClassDev->PciIo,
                                EfiPciIoWidthUint16,
                                VgaClassDev->VgaMiniPort->VgaMemoryBar,
                                VgaClassDev->VgaMiniPort->VgaMemoryOffset + ((Mode->CursorRow * MaxColumn + Mode->CursorColumn) * 2),
                                1,
                                &VideoChar
                                );

      if (Mode->CursorColumn >= (INT32) (MaxColumn - 1)) {
        This->OutputString (This, CrLfString);
      } else {
        Mode->CursorColumn++;
      }
      break;
    }
  }

  SetVideoCursorPosition (
    VgaClassDev,
    (UINTN) Mode->CursorColumn,
    (UINTN) Mode->CursorRow,
    MaxColumn
    );

  return EFI_SUCCESS;
}

EFI_STATUS
EFIAPI
VgaClassTestString (
  IN  EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL    *This,
  IN  CHAR16                          *WString
  )
/*++

Routine Description:

  GC_TODO: Add function description

Arguments:

  This    - GC_TODO: add argument description
  WString - GC_TODO: add argument description

Returns:

  EFI_UNSUPPORTED - GC_TODO: Add description for return value
  EFI_SUCCESS - GC_TODO: Add description for return value

--*/
{
  while (*WString != 0x0000) {
    if (!(IsValidAscii (*WString) || IsValidEfiCntlChar (*WString) || LibIsValidTextGraphics (*WString, NULL, NULL))) {
      return EFI_UNSUPPORTED;
    }

    WString++;
  }

  return EFI_SUCCESS;
}

EFI_STATUS
EFIAPI
VgaClassClearScreen (
  IN  EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL    *This
  )
/*++

Routine Description:

  GC_TODO: Add function description

Arguments:

  This  - GC_TODO: add argument description

Returns:

  EFI_SUCCESS - GC_TODO: Add description for return value

--*/
{
  EFI_STATUS    Status;
  VGA_CLASS_DEV *VgaClassDev;
  UINTN         MaxRow;
  UINTN         MaxColumn;
  UINT32        VideoChar;

  VgaClassDev = VGA_CLASS_DEV_FROM_THIS (This);

  Status = This->QueryMode (
                  This,
                  This->Mode->Mode,
                  &MaxColumn,
                  &MaxRow
                  );
  if (EFI_ERROR (Status)) {
    return Status;
  }

  VideoChar = (This->Mode->Attribute << 8) | ' ';
  VideoChar = (VideoChar << 16) | VideoChar;
  VgaClassDev->PciIo->Mem.Write (
                            VgaClassDev->PciIo,
                            EfiPciIoWidthFillUint32,
                            VgaClassDev->VgaMiniPort->VgaMemoryBar,
                            VgaClassDev->VgaMiniPort->VgaMemoryOffset,
                            (MaxRow * MaxColumn) >> 1,
                            &VideoChar
                            );

  This->SetCursorPosition (This, 0, 0);

  return EFI_SUCCESS;
}

EFI_STATUS
EFIAPI
VgaClassSetAttribute (
  IN  EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL    *This,
  IN  UINTN                           Attribute
  )
/*++

Routine Description:

  GC_TODO: Add function description

Arguments:

  This      - GC_TODO: add argument description
  Attribute - GC_TODO: add argument description

Returns:

  EFI_SUCCESS - GC_TODO: Add description for return value
  EFI_UNSUPPORTED - GC_TODO: Add description for return value

--*/
{
  if (Attribute <= EFI_MAX_ATTRIBUTE) {
    This->Mode->Attribute = (INT32) Attribute;
    return EFI_SUCCESS;
  }

  return EFI_UNSUPPORTED;
}

EFI_STATUS
EFIAPI
VgaClassSetCursorPosition (
  IN  EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL    *This,
  IN  UINTN                           Column,
  IN  UINTN                           Row
  )
/*++

Routine Description:

  GC_TODO: Add function description

Arguments:

  This    - GC_TODO: add argument description
  Column  - GC_TODO: add argument description
  Row     - GC_TODO: add argument description

Returns:

  EFI_UNSUPPORTED - GC_TODO: Add description for return value
  EFI_SUCCESS - GC_TODO: Add description for return value

--*/
{
  EFI_STATUS    Status;
  VGA_CLASS_DEV *VgaClassDev;
  UINTN         MaxColumn;
  UINTN         MaxRow;

  VgaClassDev = VGA_CLASS_DEV_FROM_THIS (This);

  Status = This->QueryMode (
                  This,
                  This->Mode->Mode,
                  &MaxColumn,
                  &MaxRow
                  );
  if (EFI_ERROR (Status)) {
    return Status;
  }

  if (Column >= MaxColumn || Row >= MaxRow) {
    return EFI_UNSUPPORTED;
  }

  SetVideoCursorPosition (VgaClassDev, Column, Row, MaxColumn);

  This->Mode->CursorColumn  = (INT32) Column;
  This->Mode->CursorRow     = (INT32) Row;

  return EFI_SUCCESS;
}

EFI_STATUS
EFIAPI
VgaClassEnableCursor (
  IN  EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL    *This,
  IN  BOOLEAN                         Visible
  )
/*++

Routine Description:

  GC_TODO: Add function description

Arguments:

  This    - GC_TODO: add argument description
  Visible - GC_TODO: add argument description

Returns:

  EFI_SUCCESS - GC_TODO: Add description for return value

--*/
{
  VGA_CLASS_DEV *VgaClassDev;

  VgaClassDev = VGA_CLASS_DEV_FROM_THIS (This);
  if (Visible) {
    switch (This->Mode->Mode) {
    case 1:
      WriteCrtc (VgaClassDev, CRTC_CURSOR_START, 0x06);
      WriteCrtc (VgaClassDev, CRTC_CURSOR_END, 0x07);
      break;

    default:
      WriteCrtc (VgaClassDev, CRTC_CURSOR_START, 0x0e);
      WriteCrtc (VgaClassDev, CRTC_CURSOR_END, 0x0f);
      break;
    }
  } else {
    WriteCrtc (VgaClassDev, CRTC_CURSOR_START, 0x20);
  }

  This->Mode->CursorVisible = Visible;
  return EFI_SUCCESS;
}

EFI_STATUS
EFIAPI
VgaClassQueryMode (
  IN  EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL    *This,
  IN  UINTN                           ModeNumber,
  OUT UINTN                           *Columns,
  OUT UINTN                           *Rows
  )
/*++

Routine Description:

  GC_TODO: Add function description

Arguments:

  This        - GC_TODO: add argument description
  ModeNumber  - GC_TODO: add argument description
  Columns     - GC_TODO: add argument description
  Rows        - GC_TODO: add argument description

Returns:

  EFI_UNSUPPORTED - GC_TODO: Add description for return value
  EFI_UNSUPPORTED - GC_TODO: Add description for return value
  EFI_SUCCESS - GC_TODO: Add description for return value

--*/
{
  if ((INT32) ModeNumber >= This->Mode->MaxMode) {
    *Columns  = 0;
    *Rows     = 0;
    return EFI_UNSUPPORTED;
  }

  switch (ModeNumber) {
  case 0:
    *Columns  = 80;
    *Rows     = 25;
    break;

  case 1:
    *Columns  = 80;
    *Rows     = 50;
    break;

  default:
    *Columns  = 0;
    *Rows     = 0;
    return EFI_UNSUPPORTED;
  }

  return EFI_SUCCESS;
}

EFI_STATUS
EFIAPI
VgaClassSetMode (
  IN  EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL    *This,
  IN  UINTN                           ModeNumber
  )
/*++

Routine Description:

  GC_TODO: Add function description

Arguments:

  This        - GC_TODO: add argument description
  ModeNumber  - GC_TODO: add argument description

Returns:

  EFI_UNSUPPORTED - GC_TODO: Add description for return value

--*/
{
  EFI_STATUS    Status;
  VGA_CLASS_DEV *VgaClassDev;

  VgaClassDev = VGA_CLASS_DEV_FROM_THIS (This);

  if ((INT32) ModeNumber >= This->Mode->MaxMode) {
    return EFI_UNSUPPORTED;
  }

  This->ClearScreen (This);

  This->Mode->Mode  = (INT32) ModeNumber;

  Status            = VgaClassDev->VgaMiniPort->SetMode (VgaClassDev->VgaMiniPort, ModeNumber);

  return Status;
}
//
// Private Worker Functions
//
STATIC
VOID
SetVideoCursorPosition (
  IN  VGA_CLASS_DEV  *VgaClassDev,
  IN  UINTN          Column,
  IN  UINTN          Row,
  IN  UINTN          MaxColumn
  )
/*++

Routine Description:

  GC_TODO: Add function description

Arguments:

  VgaClassDev - GC_TODO: add argument description
  Column      - GC_TODO: add argument description
  Row         - GC_TODO: add argument description
  MaxColumn   - GC_TODO: add argument description

Returns:

  GC_TODO: add return values

--*/
{
  Column    = Column & 0xff;
  Row       = Row & 0xff;
  MaxColumn = MaxColumn & 0xff;
  WriteCrtc (
    VgaClassDev,
    CRTC_CURSOR_LOCATION_HIGH,
    (UINT8) ((Row * MaxColumn + Column) >> 8)
    );
  WriteCrtc (
    VgaClassDev,
    CRTC_CURSOR_LOCATION_LOW,
    (UINT8) ((Row * MaxColumn + Column) & 0xff)
    );
}

STATIC
VOID
WriteCrtc (
  IN  VGA_CLASS_DEV  *VgaClassDev,
  IN  UINT16         Address,
  IN  UINT8          Data
  )
/*++

Routine Description:

  GC_TODO: Add function description

Arguments:

  VgaClassDev - GC_TODO: add argument description
  Address     - GC_TODO: add argument description
  Data        - GC_TODO: add argument description

Returns:

  GC_TODO: add return values

--*/
{
  VgaClassDev->PciIo->Io.Write (
                          VgaClassDev->PciIo,
                          EfiPciIoWidthUint8,
                          VgaClassDev->VgaMiniPort->CrtcAddressRegisterBar,
                          VgaClassDev->VgaMiniPort->CrtcAddressRegisterOffset,
                          1,
                          &Address
                          );

  VgaClassDev->PciIo->Io.Write (
                          VgaClassDev->PciIo,
                          EfiPciIoWidthUint8,
                          VgaClassDev->VgaMiniPort->CrtcDataRegisterBar,
                          VgaClassDev->VgaMiniPort->CrtcDataRegisterOffset,
                          1,
                          &Data
                          );
}

STATIC
BOOLEAN
LibIsValidTextGraphics (
  IN  CHAR16  Graphic,
  OUT CHAR8   *PcAnsi, OPTIONAL
  OUT CHAR8   *Ascii OPTIONAL
  )
/*++

Routine Description:

    Detects if a Unicode char is for Box Drawing text graphics.

Arguments:

    Grphic  - Unicode char to test.

    PcAnsi  - Optional pointer to return PCANSI equivalent of Graphic.

    Asci    - Optional pointer to return Ascii equivalent of Graphic.

Returns:

    TRUE if Gpaphic is a supported Unicode Box Drawing character.

--*/
// GC_TODO:    Graphic - add argument and description to function comment
{
  UNICODE_TO_CHAR *Table;

  if ((((Graphic & 0xff00) != 0x2500) && ((Graphic & 0xff00) != 0x2100))) {
    //
    // Unicode drawing code charts are all in the 0x25xx range,
    //  arrows are 0x21xx
    //
    return FALSE;
  }

  for (Table = UnicodeToPcAnsiOrAscii; Table->Unicode != 0x0000; Table++) {
    if (Graphic == Table->Unicode) {
      if (PcAnsi) {
        *PcAnsi = Table->PcAnsi;
      }

      if (Ascii) {
        *Ascii = Table->Ascii;
      }

      return TRUE;
    }
  }

  return FALSE;
}

STATIC
BOOLEAN
IsValidAscii (
  IN  CHAR16  Ascii
  )
/*++

Routine Description:

  GC_TODO: Add function description

Arguments:

  Ascii - GC_TODO: add argument description

Returns:

  GC_TODO: add return values

--*/
{
  if ((Ascii >= 0x20) && (Ascii <= 0x7f)) {
    return TRUE;
  }

  return FALSE;
}

STATIC
BOOLEAN
IsValidEfiCntlChar (
  IN  CHAR16  c
  )
/*++

Routine Description:

  GC_TODO: Add function description

Arguments:

  c - GC_TODO: add argument description

Returns:

  GC_TODO: add return values

--*/
{
  if (c == CHAR_NULL || c == CHAR_BACKSPACE || c == CHAR_LINEFEED || c == CHAR_CARRIAGE_RETURN) {
    return TRUE;
  }

  return FALSE;
}
