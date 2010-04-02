/** @file
  EFI Shell Interface protocol from EDK shell (no spec).

  Shell Interface - additional information (over image_info) provided
  to an application started by the shell.

  ConIo - provides a file style interface to the console.  Note that the
  ConOut & ConIn interfaces in the system table will work as well, and both
  all will be redirected to a file if needed on a command line

  The shell interface's and data (including ConIo) are only valid during
  the applications Entry Point.  Once the application returns from it's
  entry point the data is freed by the invoking shell.

  Copyright (c) 2006 - 2010, Intel Corporation. All rights reserved.<BR>
  This program and the accompanying materials
  are licensed and made available under the terms and conditions of the BSD License
  which accompanies this distribution.  The full text of the license may be found at
  http://opensource.org/licenses/bsd-license.php

  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

#if !defined(_SHELLINTERFACE_H_)
#define _SHELLINTERFACE_H_

#define SHELL_INTERFACE_PROTOCOL_GUID \
  { \
    0x47c7b223, 0xc42a, 0x11d2, {0x8e, 0x57, 0x0, 0xa0, 0xc9, 0x69, 0x72, 0x3b} \
  }

///
/// bit definitions for EFI_SHELL_ARG_INFO
///
typedef enum {
  ARG_NO_ATTRIB         = 0x0,
  ARG_IS_QUOTED         = BIT0,
  ARG_PARTIALLY_QUOTED  = BIT1,
  ARG_FIRST_HALF_QUOTED = BIT2,
  ARG_FIRST_CHAR_IS_ESC = BIT3
} EFI_SHELL_ARG_INFO_TYPES;

///
/// attributes for an argument.
///
typedef struct _EFI_SHELL_ARG_INFO {
  UINT32  Attributes;
} EFI_SHELL_ARG_INFO;

///
/// This protocol provides access to additional information about a shell app.
///
typedef struct {
  ///
  /// Handle back to original image handle & image info
  ///
  EFI_HANDLE                ImageHandle;
  EFI_LOADED_IMAGE_PROTOCOL *Info;

  ///
  /// Parsed arg list converted more C like format
  ///
  CHAR16                    **Argv;
  UINTN                     Argc;

  ///
  /// Storage for file redirection args after parsing
  ///
  CHAR16                    **RedirArgv;
  UINTN                     RedirArgc;

  ///
  /// A file style handle for console io
  ///
  EFI_FILE_HANDLE           StdIn;
  EFI_FILE_HANDLE           StdOut;
  EFI_FILE_HANDLE           StdErr;

  ///
  /// list of attributes for each argument
  ///
  EFI_SHELL_ARG_INFO        *ArgInfo;

  ///
  /// whether we are echoing
  ///
  BOOLEAN                   EchoOn;
} EFI_SHELL_INTERFACE;

extern EFI_GUID gEfiShellInterfaceGuid;

#endif
