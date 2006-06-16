/*++

Copyright (c)  1999 - 2002 Intel Corporation. All rights reserved
This software and associated documentation (if any) is furnished
under a license and may only be used or copied in accordance
with the terms of the license. Except as permitted by such
license, no part of this software or documentation may be
reproduced, stored in a retrieval system, or transmitted in any
form or by any means without the express written consent of
Intel Corporation.


Module Name:
  
  PeiRebaseExe.h

Abstract:

  Definitions for the PeiRebase exe utility.

--*/

#ifndef _EFI_PEIM_FIXUP_EXE_H
#define _EFI_PEIM_FIXUP_EXE_H

#include <Common/FirmwareVolumeImageFormat.h>
#include <Common/FirmwareFileSystem.h>
#include <Common/FirmwareVolumeHeader.h>
#include <Common/MultiPhase.h>

//
// Utility Name
//
#define UTILITY_NAME  "PeiRebase"

//
// Utility version information
//
#define UTILITY_MAJOR_VERSION 0
#define UTILITY_MINOR_VERSION 1
#define UTILITY_DATE          __DATE__

//
// The maximum number of arguments accepted from the command line.
//
#define MAX_ARGS  7

//
// The file copy buffer size
//
#define FILE_COPY_BUFFER_SIZE 512

//
// The function that displays general utility information
//
VOID
PrintUtilityInfo (
  VOID
  )
/*++

Routine Description:

  GC_TODO: Add function description

Arguments:

  None

Returns:

  GC_TODO: add return values

--*/
;

//
// The function that displays the utility usage message.
//
VOID
PrintUsage (
  VOID
  )
/*++

Routine Description:

  GC_TODO: Add function description

Arguments:

  None

Returns:

  GC_TODO: add return values

--*/
;

//
// Internal function declarations
//
EFI_STATUS
FfsRebaseImageRead (
  IN     VOID    *FileHandle,
  IN     UINTN   FileOffset,
  IN OUT UINT32  *ReadSize,
  OUT    VOID    *Buffer
  )
/*++

Routine Description:

  GC_TODO: Add function description

Arguments:

  FileHandle  - GC_TODO: add argument description
  FileOffset  - GC_TODO: add argument description
  ReadSize    - GC_TODO: add argument description
  Buffer      - GC_TODO: add argument description

Returns:

  GC_TODO: add return values

--*/
;

EFI_STATUS
FfsRebase (
  IN OUT EFI_FFS_FILE_HEADER    *FfsFile,
  IN EFI_PHYSICAL_ADDRESS       BaseAddress
  )
/*++

Routine Description:

  GC_TODO: Add function description

Arguments:

  FfsFile     - GC_TODO: add argument description
  BaseAddress - GC_TODO: add argument description

Returns:

  GC_TODO: add return values

--*/
;

#endif
