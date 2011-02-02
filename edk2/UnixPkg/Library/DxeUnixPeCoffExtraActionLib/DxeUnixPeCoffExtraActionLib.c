/**@file

Copyright (c) 2006 - 2009, Intel Corporation. All rights reserved.<BR>
Portions copyright (c) 2008 - 2009, Apple Inc. All rights reserved.<BR>
This program and the accompanying materials
are licensed and made available under the terms and conditions of the BSD License
which accompanies this distribution.  The full text of the license may be found at
http://opensource.org/licenses/bsd-license.php

THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

Module Name:

  PeiUnixPeCoffExtraActionLib.c

Abstract:

  Provides services to perform additional actions to relocate and unload
  PE/Coff image for Unix environment specific purpose such as souce level debug.
  This version only works for DXE phase  


**/

#include <PiDxe.h>
#include <UnixDxe.h>
#include <Library/PeCoffLib.h>

#include <Library/BaseLib.h>
#include <Library/DebugLib.h>
#include <Library/HobLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/PeCoffExtraActionLib.h>

//

// Cache of UnixThunk protocol 

//

EFI_UNIX_THUNK_PROTOCOL   *mUnix = NULL;


/**
  The constructor function gets  the pointer of the WinNT thunk functions
  It will ASSERT() if Unix thunk protocol is not installed.

  @retval EFI_SUCCESS   Unix thunk protocol is found and cached.

**/
EFI_STATUS
EFIAPI
DxeUnixPeCoffLibExtraActionConstructor (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
{
   EFI_HOB_GUID_TYPE        *GuidHob;

  //
  // Retrieve UnixThunkProtocol from GUID'ed HOB
  //
  GuidHob = GetFirstGuidHob (&gEfiUnixThunkProtocolGuid);
  ASSERT (GuidHob != NULL);
  mUnix = (EFI_UNIX_THUNK_PROTOCOL *)(*(UINTN *)(GET_GUID_HOB_DATA (GuidHob)));
  ASSERT (mUnix != NULL);

  return EFI_SUCCESS;
}

/**
  Performs additional actions after a PE/COFF image has been loaded and relocated.

  If ImageContext is NULL, then ASSERT().

  @param  ImageContext  Pointer to the image context structure that describes the
                        PE/COFF image that has already been loaded and relocated.

**/
VOID
EFIAPI
PeCoffLoaderRelocateImageExtraAction (
  IN OUT PE_COFF_LOADER_IMAGE_CONTEXT  *ImageContext
  )
{
  if (mUnix != NULL) {
    mUnix->PeCoffRelocateImageExtraAction (ImageContext);
  }
}



/**
  Performs additional actions just before a PE/COFF image is unloaded.  Any resources
  that were allocated by PeCoffLoaderRelocateImageExtraAction() must be freed.
  
  If ImageContext is NULL, then ASSERT().
  
  @param  ImageContext  Pointer to the image context structure that describes the
                        PE/COFF image that is being unloaded.

**/
VOID
EFIAPI
PeCoffLoaderUnloadImageExtraAction (
  IN OUT PE_COFF_LOADER_IMAGE_CONTEXT  *ImageContext
  )
{
  if (mUnix != NULL) {
    mUnix->PeCoffUnloadImageExtraAction (ImageContext);
  }
}
