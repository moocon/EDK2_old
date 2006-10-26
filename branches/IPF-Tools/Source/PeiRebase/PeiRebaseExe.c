/*++

Copyright (c)  1999-2006 Intel Corporation. All rights reserved
This program and the accompanying materials are licensed and made available 
under the terms and conditions of the BSD License which accompanies this 
distribution.  The full text of the license may be found at
http://opensource.org/licenses/bsd-license.php

THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.


Module Name:

  PeiRebaseExe.c

Abstract:

  This contains all code necessary to build the PeiRebase.exe utility.
  This utility relies heavily on the PeiRebase DLL.  Definitions for both
  can be found in the PEI Rebase Utility Specification, review draft.

--*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <Common/UefiBaseTypes.h>
#include <Common/FirmwareVolumeImageFormat.h>
#include <Common/FirmwareFileSystem.h>
#include <Library/PeCoffLib.h>

#include "CommonLib.h"
#include "ParseInf.h"
#include "FvLib.h"
#include "EfiUtilityMsgs.h"
#include "PeiRebaseExe.h"

EFI_STATUS
ReadHeader (
  IN FILE       *InputFile,
  OUT UINT32    *FvSize,
  OUT BOOLEAN   *ErasePolarity
  );

int
main (
  int  argc,
  char **argv
  )
/*++

Routine Description:

  This utility relocates PEI XIP PE32s in a FV.

Arguments:

  argc          - Number of command line arguments
  argv[]:
  BaseAddress     The base address to use for rebasing the FV.  The correct 
                  format is a hex number preceded by 0x.
  InputFileName   The name of the input FV file.
  OutputFileName  The name of the output FV file.
  MapFileName     The name of the map file of relocation info.

  Arguments come in pair in any order.
    -I InputFileName 
    -O OutputFileName
    -B BaseAddress 
    -M MapFileName 

Returns:

  0   No error conditions detected.
  1   One or more of the input parameters is invalid.
  2   A resource required by the utility was unavailable.  
      Most commonly this will be memory allocation or file creation.
  3   PeiRebase.dll could not be loaded.
  4   Error executing the PEI rebase.

--*/
{
  UINT8                       Index;
  CHAR8                       InputFileName[_MAX_PATH];
  CHAR8                       OutputFileName[_MAX_PATH];
  CHAR8                       MapFileName[_MAX_PATH];
  EFI_PHYSICAL_ADDRESS        BaseAddress;
  BOOLEAN                     BaseAddressSet;
  EFI_STATUS                  Status;
  FILE                        *InputFile;
  FILE                        *OutputFile;
  FILE                        *MapFile;
  UINT64                      FvOffset;
  UINT32                      FileCount;
  int                         BytesRead;
  EFI_FIRMWARE_VOLUME_HEADER  *FvImage;
  UINT32                      FvSize;
  EFI_FFS_FILE_HEADER         *CurrentFile;
  BOOLEAN                     ErasePolarity;
  EFI_PHYSICAL_ADDRESS        CurrentFileBaseAddress;

  ErasePolarity = FALSE;
  //
  // Set utility name for error/warning reporting purposes.
  //
  SetUtilityName (UTILITY_NAME);
  //
  // Verify the correct number of arguments
  //
  if (argc != MAX_ARGS) {
    PrintUsage ();
    return STATUS_ERROR;
  }

  //
  // Initialize variables
  //
  InputFileName[0]  = 0;
  OutputFileName[0] = 0;
  MapFileName[0]    = 0;
  BaseAddress       = 0;
  BaseAddressSet    = FALSE;
  FvOffset          = 0;
  FileCount         = 0;
  ErasePolarity     = FALSE;
  InputFile         = NULL;
  OutputFile        = NULL;
  MapFile           = NULL;
  FvImage           = NULL;

  //
  // Parse the command line arguments
  //
  for (Index = 1; Index < MAX_ARGS; Index += 2) {
    //
    // Make sure argument pair begin with - or /
    //
    if (argv[Index][0] != '-' && argv[Index][0] != '/') {
      PrintUsage ();
      Error (NULL, 0, 0, argv[Index], "unrecognized option");
      return STATUS_ERROR;
    }
    //
    // Make sure argument specifier is only one letter
    //
    if (argv[Index][2] != 0) {
      PrintUsage ();
      Error (NULL, 0, 0, argv[Index], "unrecognized option");
      return STATUS_ERROR;
    }    
    //
    // Determine argument to read
    //
    switch (argv[Index][1]) {
    case 'I':
    case 'i':
      if (strlen (InputFileName) == 0) {
        strcpy (InputFileName, argv[Index + 1]);
      } else {
        PrintUsage ();
        Error (NULL, 0, 0, argv[Index + 1], "only one -i InputFileName may be specified");
        return STATUS_ERROR;
      }
      break;

    case 'O':
    case 'o':
      if (strlen (OutputFileName) == 0) {
        strcpy (OutputFileName, argv[Index + 1]);
      } else {
        PrintUsage ();
        Error (NULL, 0, 0, argv[Index + 1], "only one -o OutputFileName may be specified");
        return STATUS_ERROR;
      }
      break;

    case 'B':
    case 'b':
      if (!BaseAddressSet) {
        Status = AsciiStringToUint64 (argv[Index + 1], FALSE, &BaseAddress);
        if (EFI_ERROR (Status)) {
          PrintUsage ();
          Error (NULL, 0, 0, argv[Index + 1], "invalid hex digit given for the base address");
          return STATUS_ERROR;
        }

        BaseAddressSet = TRUE;
      } else {
        PrintUsage ();
        Error (NULL, 0, 0, argv[Index + 1], "-b BaseAddress may only be specified once");
        return STATUS_ERROR;
      }
      break;

    case 'M':
    case 'm':
      if (strlen (MapFileName) == 0) {
        strcpy (MapFileName, argv[Index + 1]);
      } else {
        PrintUsage ();
        Error (NULL, 0, 0, argv[Index + 1], "only one -m MapFileName may be specified");
        return STATUS_ERROR;
      }
      break;

    default:
      PrintUsage ();
      Error (NULL, 0, 0, argv[Index], "unrecognized argument");
      return STATUS_ERROR;
      break;
    }
  }

  //
  // Create the Map file if we need it
  //
  if (strlen (MapFileName) != 0) {
    MapFile = fopen (MapFileName, "w");
    if (MapFile == NULL) {
      Error (NULL, 0, 0, MapFileName, "failed to open map file");
      goto Finish;
    }
  } 

  //
  // Open the file containing the FV
  //
  InputFile = fopen (InputFileName, "rb");
  if (InputFile == NULL) {
    Error (NULL, 0, 0, InputFileName, "could not open input file for reading");
    return STATUS_ERROR;
  }
  //
  // Determine size of FV
  //
  Status = ReadHeader (InputFile, &FvSize, &ErasePolarity);
  if (EFI_ERROR (Status)) {
    Error (NULL, 0, 0, "could not parse the FV header", NULL);
    goto Finish;
  }
  //
  // Allocate a buffer for the FV image
  //
  FvImage = malloc (FvSize);
  if (FvImage == NULL) {
    Error (NULL, 0, 0, "application error", "memory allocation failed");
    goto Finish;
  }
  //
  // Read the entire FV to the buffer
  //
  BytesRead = fread (FvImage, 1, FvSize, InputFile);
  fclose (InputFile);
  InputFile = NULL;
  if ((unsigned int) BytesRead != FvSize) {
    Error (NULL, 0, 0, InputFileName, "failed to read from file");
    goto Finish;
  }
  //
  // Prepare to walk the FV image
  //
  InitializeFvLib (FvImage, FvSize);
  //
  // Get the first file
  //
  Status = GetNextFile (NULL, &CurrentFile);
  if (EFI_ERROR (Status)) {
    Error (NULL, 0, 0, "cannot find the first file in the FV image", NULL);
    goto Finish;
  }
  //
  // Check if each file should be rebased
  //
  while (CurrentFile != NULL) {
    //
    // Rebase this file
    //
    CurrentFileBaseAddress  = BaseAddress + ((UINTN) CurrentFile - (UINTN) FvImage);
    Status                  = FfsRebase (CurrentFile, CurrentFileBaseAddress, MapFile);

    if (EFI_ERROR (Status)) {
      switch (Status) {

      case EFI_INVALID_PARAMETER:
        Error (NULL, 0, 0, "invalid parameter passed to FfsRebase", NULL);
        break;

      case EFI_ABORTED:
        Error (NULL, 0, 0, "error detected while rebasing -- aborted", NULL);
        break;

      case EFI_OUT_OF_RESOURCES:
        Error (NULL, 0, 0, "FfsRebase could not allocate required resources", NULL);
        break;

      case EFI_NOT_FOUND:
        Error (NULL, 0, 0, "FfsRebase could not locate a PE32 section", NULL);
        break;

      default:
        Error (NULL, 0, 0, "FfsRebase returned unknown status", "status=0x%08X", Status);
        break;
      }

      goto Finish;
    }

    //
    // Get the next file
    //
    Status = GetNextFile (CurrentFile, &CurrentFile);
    if (EFI_ERROR (Status)) {
      Error (NULL, 0, 0, "cannot find the next file in the FV image", NULL);
      goto Finish;
    }
  }
  //
  // Open the output file
  //
  OutputFile = fopen (OutputFileName, "wb");
  if (OutputFile == NULL) {
    Error (NULL, 0, 0, OutputFileName, "failed to open output file");
    goto Finish;
  }

  if (fwrite (FvImage, 1, FvSize, OutputFile) != FvSize) {
    Error (NULL, 0, 0, "failed to write to output file", 0);
    goto Finish;
  }

Finish:
  if (InputFile != NULL) {
    fclose (InputFile);
  }
  //
  // If we created an output file, and there was an error, remove it so
  // subsequent builds will rebuild it.
  //
  if (OutputFile != NULL) {
    if (GetUtilityStatus () == STATUS_ERROR) {
      remove (OutputFileName);
    }

    fclose (OutputFile);
  }

  if (MapFile != NULL) {
    fclose (MapFile);
  }

  if (FvImage != NULL) {
    free (FvImage);
  }

  return GetUtilityStatus ();
}

EFI_STATUS
ReadHeader (
  IN FILE       *InputFile,
  OUT UINT32    *FvSize,
  OUT BOOLEAN   *ErasePolarity
  )
/*++

Routine Description:

  This function determines the size of the FV and the erase polarity.  The 
  erase polarity is the FALSE value for file state.

Arguments:

  InputFile       The file that contains the FV image.
  FvSize          The size of the FV.
  ErasePolarity   The FV erase polarity.
    
Returns:
 
  EFI_SUCCESS             Function completed successfully.
  EFI_INVALID_PARAMETER   A required parameter was NULL or is out of range.
  EFI_ABORTED             The function encountered an error.

--*/
{
  EFI_FIRMWARE_VOLUME_HEADER  VolumeHeader;
  EFI_FV_BLOCK_MAP_ENTRY      BlockMap;
  UINTN                       Signature[2];
  UINTN                       BytesRead;
  UINT32                      Size;

  BytesRead = 0;
  Size      = 0;
  //
  // Check input parameters
  //
  if ((InputFile == NULL) || (FvSize == NULL) || (ErasePolarity == NULL)) {
    Error (NULL, 0, 0, "ReadHeader()", "invalid input parameter");
    return EFI_INVALID_PARAMETER;
  }
  //
  // Read the header
  //
  fread (&VolumeHeader, sizeof (EFI_FIRMWARE_VOLUME_HEADER) - sizeof (EFI_FV_BLOCK_MAP_ENTRY), 1, InputFile);
  BytesRead     = sizeof (EFI_FIRMWARE_VOLUME_HEADER) - sizeof (EFI_FV_BLOCK_MAP_ENTRY);
  Signature[0]  = VolumeHeader.Signature;
  Signature[1]  = 0;

  //
  // Get erase polarity
  //
  if (VolumeHeader.Attributes & EFI_FVB_ERASE_POLARITY) {
    *ErasePolarity = TRUE;
  }

  do {
    fread (&BlockMap, sizeof (EFI_FV_BLOCK_MAP_ENTRY), 1, InputFile);
    BytesRead += sizeof (EFI_FV_BLOCK_MAP_ENTRY);

    if (BlockMap.NumBlocks != 0) {
      Size += BlockMap.NumBlocks * BlockMap.BlockLength;
    }

  } while (!(BlockMap.NumBlocks == 0 && BlockMap.BlockLength == 0));

  if (VolumeHeader.FvLength != Size) {
    Error (NULL, 0, 0, "volume size not consistant with block maps", NULL);
    return EFI_ABORTED;
  }

  *FvSize = Size;

  rewind (InputFile);

  return EFI_SUCCESS;
}

VOID
PrintUtilityInfo (
  VOID
  )
/*++

Routine Description:

  Displays the standard utility information to SDTOUT

Arguments:

  None

Returns:

  None

--*/
{
  printf (
    "%s, PEI Rebase Utility. Version %i.%i, %s.\n\n",
    UTILITY_NAME,
    UTILITY_MAJOR_VERSION,
    UTILITY_MINOR_VERSION,
    UTILITY_DATE
    );
}

VOID
PrintUsage (
  VOID
  )
/*++

Routine Description:

  Displays the utility usage syntax to STDOUT

Arguments:

  None

Returns:

  None

--*/
{
  printf (
    "Usage: %s -I InputFileName -O OutputFileName -B BaseAddress [-M MapFile]\n",
    UTILITY_NAME
    );
  printf ("  Where:\n");
  printf ("    InputFileName is the name of the EFI FV file to rebase.\n");
  printf ("    OutputFileName is the desired output file name.\n");
  printf ("    BaseAddress is the FV base address to rebase agains.\n");
  printf ("    MapFileName is an optional map file of the relocations\n");
  printf ("  Argument pair may be in any order.\n\n");
}

EFI_STATUS
FfsRebase (
  IN OUT EFI_FFS_FILE_HEADER    *FfsFile,
  IN EFI_PHYSICAL_ADDRESS       BaseAddress,
  IN FILE                       *MapFile      OPTIONAL
  )
/*++

Routine Description:

  This function determines if a file is XIP and should be rebased.  It will 
  rebase any PE32 sections found in the file using the base address.
  
Arguments:

  FfsFile           A pointer to Ffs file image.
  BaseAddress       The base address to use for rebasing the file image.
  MapFile           Optional file to dump relocation information into

Returns:

  EFI_SUCCESS             The image was properly rebased.
  EFI_INVALID_PARAMETER   An input parameter is invalid.
  EFI_ABORTED             An error occurred while rebasing the input file image.
  EFI_OUT_OF_RESOURCES    Could not allocate a required resource.
  EFI_NOT_FOUND           No compressed sections could be found.

--*/
{
  EFI_STATUS                            Status;
  PE_COFF_LOADER_IMAGE_CONTEXT          ImageContext;
  UINTN                                 MemoryImagePointer;
  UINTN                                 MemoryImagePointerAligned;
  EFI_PHYSICAL_ADDRESS                  ImageAddress;
  UINT64                                ImageSize;
  EFI_PHYSICAL_ADDRESS                  EntryPoint;
  UINT32                                Pe32ImageSize;
  UINT32                                NewPe32BaseAddress;
  UINTN                                 Index;
  EFI_FILE_SECTION_POINTER              CurrentPe32Section;
  EFI_FFS_FILE_STATE                    SavedState;
  EFI_IMAGE_NT_HEADERS                  *PeHdr;
  UINT32                                *PeHdrSizeOfImage;
  UINT32                                *PeHdrChecksum;
  UINT32                                FoundCount;
  EFI_TE_IMAGE_HEADER                   *TEImageHeader;
  UINT8                                 *TEBuffer;
  EFI_IMAGE_DOS_HEADER                  *DosHeader;
  UINT8                                 FileGuidString[80];
  UINT32                                TailSize;
  EFI_FFS_FILE_TAIL                     TailValue;

  //
  // Verify input parameters
  //
  if (FfsFile == NULL) {
    return EFI_INVALID_PARAMETER;
  }
  
  //
  // Convert the GUID to a string so we can at least report which file
  // if we find an error.
  //
  PrintGuidToBuffer (&FfsFile->Name, FileGuidString, sizeof (FileGuidString), TRUE);
  if (FfsFile->Attributes & FFS_ATTRIB_TAIL_PRESENT) {
    TailSize = sizeof (EFI_FFS_FILE_TAIL);
  } else {
    TailSize = 0;
  }
  
  //
  // Do some cursory checks on the FFS file contents
  //
  Status = VerifyFfsFile (FfsFile);
  if (EFI_ERROR (Status)) {
    Error (NULL, 0, 0, "file does not appear to be a valid FFS file, cannot be rebased", FileGuidString);
    return EFI_INVALID_PARAMETER;
  }

  memset (&ImageContext, 0, sizeof (ImageContext));

  //
  // Check if XIP file type. If not XIP, don't rebase.
  //
  if (FfsFile->Type != EFI_FV_FILETYPE_PEI_CORE &&
      FfsFile->Type != EFI_FV_FILETYPE_PEIM &&
      FfsFile->Type != EFI_FV_FILETYPE_SECURITY_CORE &&
      FfsFile->Type != EFI_FV_FILETYPE_COMBINED_PEIM_DRIVER
      ) {
    return EFI_SUCCESS;
  }

  //
  // Rebase each PE32 section
  //
  Status      = EFI_SUCCESS;
  FoundCount  = 0;
  for (Index = 1;; Index++) {
    Status = GetSectionByType (FfsFile, EFI_SECTION_PE32, Index, &CurrentPe32Section);
    if (EFI_ERROR (Status)) {
      break;
    }

    FoundCount++;

    //
    // Calculate the PE32 base address, the FFS file base plus the offset of the PE32 section
    //
    NewPe32BaseAddress = ((UINT32) BaseAddress) + ((UINTN) CurrentPe32Section.Pe32Section + sizeof (EFI_COMMON_SECTION_HEADER) - (UINTN) FfsFile);

    //
    // Initialize context
    //
    memset (&ImageContext, 0, sizeof (ImageContext));
    ImageContext.Handle     = (VOID *) ((UINTN) CurrentPe32Section.Pe32Section + sizeof (EFI_PE32_SECTION));
    ImageContext.ImageRead  = (PE_COFF_LOADER_READ_FILE) FfsRebaseImageRead;

    Status                  = PeCoffLoaderGetImageInfo (&ImageContext);

    if (EFI_ERROR (Status)) {
      Error (NULL, 0, 0, "GetImageInfo() call failed on rebase", FileGuidString);
      return Status;
    }
    //
    // Allocate a buffer for the image to be loaded into.
    //
    Pe32ImageSize       = GetLength (CurrentPe32Section.Pe32Section->CommonHeader.Size) - sizeof (EFI_PE32_SECTION);
    MemoryImagePointer  = (UINTN) (malloc (Pe32ImageSize + 0x1000));
    if (MemoryImagePointer == 0) {
      Error (NULL, 0, 0, "memory allocation failure", NULL);
      return EFI_OUT_OF_RESOURCES;
    }
    memset ((void *) MemoryImagePointer, 0, Pe32ImageSize + 0x1000);
    MemoryImagePointerAligned = (MemoryImagePointer + 0x0FFF) & (-1 << 12);
    

    ImageContext.ImageAddress = MemoryImagePointerAligned;

    Status                    = PeCoffLoaderLoadImage (&ImageContext);
    if (EFI_ERROR (Status)) {
      Error (NULL, 0, 0, "LoadImage() call failed on rebase", FileGuidString);
      free ((VOID *) MemoryImagePointer);
      return Status;
    }

    ImageContext.DestinationAddress = NewPe32BaseAddress;
    Status                          = PeCoffLoaderRelocateImage (&ImageContext);
    if (EFI_ERROR (Status)) {
      Error (NULL, 0, 0, "RelocateImage() call failed on rebase", FileGuidString);
      free ((VOID *) MemoryImagePointer);
      return Status;
    }

    ImageAddress  = ImageContext.ImageAddress;
    ImageSize     = ImageContext.ImageSize;
    EntryPoint    = ImageContext.EntryPoint;

    if (ImageSize > Pe32ImageSize) {
      Error (
        NULL,
        0,
        0,
        "rebased image is larger than original PE32 image",
        "0x%X > 0x%X, file %s",
        ImageSize,
        Pe32ImageSize,
        FileGuidString
        );
      free ((VOID *) MemoryImagePointer);
      return EFI_ABORTED;
    }
    //
    // Since we may have updated the Codeview RVA, we need to insure the PE
    // header indicates the image is large enough to contain the Codeview data
    // so it will be loaded properly later if the PEIM is reloaded into memory...
    //
    PeHdr = (VOID *) ((UINTN) ImageAddress + ImageContext.PeCoffHeaderOffset);
    if (PeHdr->FileHeader.Machine == EFI_IMAGE_MACHINE_IA32) {
      PeHdrSizeOfImage     = (UINT32 *) (&(*(EFI_IMAGE_OPTIONAL_HEADER32 *) &PeHdr->OptionalHeader).SizeOfImage);
      PeHdrChecksum        = (UINT32 *) (&(*(EFI_IMAGE_OPTIONAL_HEADER32 *) &PeHdr->OptionalHeader).CheckSum);
    } else if (PeHdr->FileHeader.Machine == EFI_IMAGE_MACHINE_IA64) {
      PeHdrSizeOfImage     = (UINT32 *) (&(*(EFI_IMAGE_OPTIONAL_HEADER64 *) &PeHdr->OptionalHeader).SizeOfImage);
      PeHdrChecksum        = (UINT32 *) (&(*(EFI_IMAGE_OPTIONAL_HEADER64 *) &PeHdr->OptionalHeader).CheckSum);
    } else if (PeHdr->FileHeader.Machine == EFI_IMAGE_MACHINE_X64) {
      PeHdrSizeOfImage     = (UINT32 *) (&(*(EFI_IMAGE_OPTIONAL_HEADER64 *) &PeHdr->OptionalHeader).SizeOfImage);
      PeHdrChecksum        = (UINT32 *) (&(*(EFI_IMAGE_OPTIONAL_HEADER64 *) &PeHdr->OptionalHeader).CheckSum);
    } else {
      Error (
        NULL,
        0,
        0,
        "unknown machine type in PE32 image",
        "machine type=0x%X, file=%s",
        (UINT32) PeHdr->FileHeader.Machine,
        FileGuidString
        );
      free ((VOID *) MemoryImagePointer);
      return EFI_ABORTED;
    }

    if (*PeHdrSizeOfImage != ImageContext.ImageSize) {
      *PeHdrSizeOfImage = (UINT32) ImageContext.ImageSize;
      if (*PeHdrChecksum) {
        *PeHdrChecksum = 0;
      }
    }

    memcpy (CurrentPe32Section.Pe32Section + 1, (VOID *) MemoryImagePointerAligned, (UINT32) ImageSize);
    
    //
    // Get EntryPoint in Flash Region.
    //
    EntryPoint = NewPe32BaseAddress + EntryPoint - ImageAddress;

    //
    // If a map file was selected output mapping information for any file that
    // was rebased.
    //
    if (MapFile != NULL) {
      fprintf (MapFile, "PE32 File: %s Base:%08lx", FileGuidString, BaseAddress);
      fprintf (MapFile, " EntryPoint:%08lx", EntryPoint);
      if (ImageContext.PdbPointer != NULL) {
        fprintf (MapFile, " FileName: %s", ImageContext.PdbPointer);
      }
      fprintf (MapFile, "\n");
    }

    free ((VOID *) MemoryImagePointer);

    //
    // Now update file checksum
    //
    if (FfsFile->Attributes & FFS_ATTRIB_TAIL_PRESENT) {
      TailSize = sizeof (EFI_FFS_FILE_TAIL);
    } else {
      TailSize = 0;
    }

    if (FfsFile->Attributes & FFS_ATTRIB_CHECKSUM) {
      SavedState  = FfsFile->State;
      FfsFile->IntegrityCheck.Checksum.File = 0;
      FfsFile->State                        = 0;
      if (FfsFile->Attributes & FFS_ATTRIB_CHECKSUM) {
        FfsFile->IntegrityCheck.Checksum.File = CalculateChecksum8 (
                                                  (UINT8 *) FfsFile,
                                                  GetLength (FfsFile->Size) - TailSize
                                                  );
      } else {
        FfsFile->IntegrityCheck.Checksum.File = FFS_FIXED_CHECKSUM;
      }

      FfsFile->State = SavedState;
    }
    //
    // Update tail if present
    //
    if (FfsFile->Attributes & FFS_ATTRIB_TAIL_PRESENT) {
      TailValue = (EFI_FFS_FILE_TAIL) (~(FfsFile->IntegrityCheck.TailReference));
      *(EFI_FFS_FILE_TAIL *) (((UINTN) FfsFile + GetLength (FfsFile->Size) - sizeof (EFI_FFS_FILE_TAIL))) = TailValue;
    }
  }
  //
  // Now process TE sections
  //
  for (Index = 1;; Index++) {
    Status = GetSectionByType (FfsFile, EFI_SECTION_TE, Index, &CurrentPe32Section);
    if (EFI_ERROR (Status)) {
      break;
    }

    FoundCount++;

    //
    // Calculate the TE base address, the FFS file base plus the offset of the TE section less the size stripped off
    // by GenTEImage
    //
    TEImageHeader = (EFI_TE_IMAGE_HEADER *) ((UINT8 *) CurrentPe32Section.Pe32Section + sizeof (EFI_COMMON_SECTION_HEADER));

    NewPe32BaseAddress = ((UINT32) BaseAddress) +
      (
        (UINTN) CurrentPe32Section.Pe32Section +
        sizeof (EFI_COMMON_SECTION_HEADER) +
        sizeof (EFI_TE_IMAGE_HEADER) -
        TEImageHeader->StrippedSize -
        (UINTN) FfsFile
      );

    //
    // Allocate a buffer to unshrink the image into.
    //
    Pe32ImageSize = GetLength (CurrentPe32Section.Pe32Section->CommonHeader.Size) - sizeof (EFI_PE32_SECTION) -
    sizeof (EFI_TE_IMAGE_HEADER);
    Pe32ImageSize += TEImageHeader->StrippedSize;
    TEBuffer = (UINT8 *) malloc (Pe32ImageSize);
    if (TEBuffer == NULL) {
      Error (NULL, 0, 0, "failed to allocate memory", NULL);
      return EFI_OUT_OF_RESOURCES;
    }
    //
    // Expand the image into our buffer and fill in critical fields in the DOS header
    // Fill in fields required by the loader.
    // At offset 0x3C is the offset to the PE signature. We'll put it immediately following the offset value
    // itself.
    //
    memset (TEBuffer, 0, Pe32ImageSize);
    DosHeader = (EFI_IMAGE_DOS_HEADER *) TEBuffer;
    DosHeader->e_magic = EFI_IMAGE_DOS_SIGNATURE;
    *(UINT32 *) (TEBuffer + 0x3C) = 0x40;
    PeHdr = (EFI_IMAGE_NT_HEADERS *) (TEBuffer + 0x40);
    PeHdr->Signature = EFI_IMAGE_NT_SIGNATURE;
    PeHdr->FileHeader.Machine = TEImageHeader->Machine;
    PeHdr->FileHeader.NumberOfSections = TEImageHeader->NumberOfSections;

    //
    // Say the size of the optional header is the total we stripped off less the size of a PE file header and PE signature and
    // the 0x40 bytes for our DOS header.
    //
    PeHdr->FileHeader.SizeOfOptionalHeader = (UINT16) (TEImageHeader->StrippedSize - 0x40 - sizeof (UINT32) - sizeof (EFI_IMAGE_FILE_HEADER));
    PeHdr->OptionalHeader.ImageBase = (UINTN) (TEImageHeader->ImageBase - TEImageHeader->StrippedSize + sizeof (EFI_TE_IMAGE_HEADER));
    PeHdr->OptionalHeader.AddressOfEntryPoint = TEImageHeader->AddressOfEntryPoint;
    PeHdr->OptionalHeader.BaseOfCode  = TEImageHeader->BaseOfCode;
    PeHdr->OptionalHeader.SizeOfImage = Pe32ImageSize;
    PeHdr->OptionalHeader.Subsystem   = TEImageHeader->Subsystem;
    PeHdr->OptionalHeader.SizeOfImage = Pe32ImageSize;
    PeHdr->OptionalHeader.SizeOfHeaders = TEImageHeader->StrippedSize + TEImageHeader->NumberOfSections *
    sizeof (EFI_IMAGE_SECTION_HEADER) - 12;

    //
    // Set NumberOfRvaAndSizes in the optional header to what we had available in the original image
    //
    if ((TEImageHeader->DataDirectory[EFI_TE_IMAGE_DIRECTORY_ENTRY_BASERELOC].VirtualAddress != 0) ||
        (TEImageHeader->DataDirectory[EFI_TE_IMAGE_DIRECTORY_ENTRY_BASERELOC].Size != 0)
        ) {
      PeHdr->OptionalHeader.NumberOfRvaAndSizes = EFI_IMAGE_DIRECTORY_ENTRY_BASERELOC + 1;
      PeHdr->OptionalHeader.DataDirectory[EFI_IMAGE_DIRECTORY_ENTRY_BASERELOC].VirtualAddress = TEImageHeader->DataDirectory[EFI_TE_IMAGE_DIRECTORY_ENTRY_BASERELOC].VirtualAddress;
      PeHdr->OptionalHeader.DataDirectory[EFI_IMAGE_DIRECTORY_ENTRY_BASERELOC].Size = TEImageHeader->DataDirectory[EFI_TE_IMAGE_DIRECTORY_ENTRY_BASERELOC].Size;
    }

    if ((TEImageHeader->DataDirectory[EFI_TE_IMAGE_DIRECTORY_ENTRY_DEBUG].VirtualAddress != 0) ||
        (TEImageHeader->DataDirectory[EFI_TE_IMAGE_DIRECTORY_ENTRY_DEBUG].Size != 0)
        ) {
      PeHdr->OptionalHeader.DataDirectory[EFI_IMAGE_DIRECTORY_ENTRY_DEBUG].VirtualAddress = TEImageHeader->DataDirectory[EFI_TE_IMAGE_DIRECTORY_ENTRY_DEBUG].VirtualAddress;
      PeHdr->OptionalHeader.DataDirectory[EFI_IMAGE_DIRECTORY_ENTRY_DEBUG].Size = TEImageHeader->DataDirectory[EFI_TE_IMAGE_DIRECTORY_ENTRY_DEBUG].Size;
      if (PeHdr->OptionalHeader.NumberOfRvaAndSizes < EFI_IMAGE_DIRECTORY_ENTRY_DEBUG + 1) {
        PeHdr->OptionalHeader.NumberOfRvaAndSizes = EFI_IMAGE_DIRECTORY_ENTRY_DEBUG + 1;
      }
    }
    //
    // NOTE: These values are defaults, and should be verified to be correct in the GenTE utility
    //
    PeHdr->OptionalHeader.SectionAlignment = 0x10;

    //
    // Copy the rest of the image to its original offset
    //
    memcpy (
      TEBuffer + TEImageHeader->StrippedSize,
      (UINT8 *) CurrentPe32Section.Pe32Section + sizeof (EFI_PE32_SECTION) + sizeof (EFI_TE_IMAGE_HEADER),
      GetLength (CurrentPe32Section.Pe32Section->CommonHeader.Size) - sizeof (EFI_PE32_SECTION) -
      sizeof (EFI_TE_IMAGE_HEADER)
      );

    //
    // Initialize context
    //
    memset (&ImageContext, 0, sizeof (ImageContext));
    ImageContext.Handle     = (VOID *) TEBuffer;
    ImageContext.ImageRead  = (PE_COFF_LOADER_READ_FILE) FfsRebaseImageRead;

    Status                  = PeCoffLoaderGetImageInfo (&ImageContext);

    if (EFI_ERROR (Status)) {
      Error (NULL, 0, 0, "GetImageInfo() call failed on rebase of TE image", FileGuidString);
      free (TEBuffer);
      return Status;
    }
    //
    // Allocate a buffer for the image to be loaded into.
    //
    MemoryImagePointer = (UINTN) (malloc (Pe32ImageSize + 0x1000));
    if (MemoryImagePointer == 0) {
      Error (NULL, 0, 0, "memory allocation error on rebase of TE image", FileGuidString);
      free (TEBuffer);
      return EFI_OUT_OF_RESOURCES;
    }
    memset ((void *) MemoryImagePointer, 0, Pe32ImageSize + 0x1000);
    MemoryImagePointerAligned = (MemoryImagePointer + 0x0FFF) & (-1 << 12);
    

    ImageContext.ImageAddress = MemoryImagePointerAligned;
    Status                    = PeCoffLoaderLoadImage (&ImageContext);
    if (EFI_ERROR (Status)) {
      Error (NULL, 0, 0, "LoadImage() call failed on rebase of TE image", FileGuidString);
      free (TEBuffer);
      free ((VOID *) MemoryImagePointer);
      return Status;
    }

    ImageContext.DestinationAddress = NewPe32BaseAddress;
    Status                          = PeCoffLoaderRelocateImage (&ImageContext);
    if (EFI_ERROR (Status)) {
      Error (NULL, 0, 0, "RelocateImage() call failed on rebase of TE image", FileGuidString);
      free ((VOID *) MemoryImagePointer);
      free (TEBuffer);
      return Status;
    }

    ImageAddress  = ImageContext.ImageAddress;
    ImageSize     = ImageContext.ImageSize;
    EntryPoint    = ImageContext.EntryPoint;

    //
    // Since we may have updated the Codeview RVA, we need to insure the PE
    // header indicates the image is large enough to contain the Codeview data
    // so it will be loaded properly later if the PEIM is reloaded into memory...
    //
    PeHdr = (VOID *) ((UINTN) ImageAddress + ImageContext.PeCoffHeaderOffset);
    if (PeHdr->FileHeader.Machine == EFI_IMAGE_MACHINE_IA32) {
      PeHdrSizeOfImage     = (UINT32 *) (&(*(EFI_IMAGE_OPTIONAL_HEADER32 *) &PeHdr->OptionalHeader).SizeOfImage);
      PeHdrChecksum        = (UINT32 *) (&(*(EFI_IMAGE_OPTIONAL_HEADER32 *) &PeHdr->OptionalHeader).CheckSum);
    } else if (PeHdr->FileHeader.Machine == EFI_IMAGE_MACHINE_IA64) {
      PeHdrSizeOfImage     = (UINT32 *) (&(*(EFI_IMAGE_OPTIONAL_HEADER64 *) &PeHdr->OptionalHeader).SizeOfImage);
      PeHdrChecksum        = (UINT32 *) (&(*(EFI_IMAGE_OPTIONAL_HEADER64 *) &PeHdr->OptionalHeader).CheckSum);
    } else {
      Error (
        NULL,
        0,
        0,
        "unknown machine type in TE image",
        "machine type=0x%X, file=%s",
        (UINT32) PeHdr->FileHeader.Machine,
        FileGuidString
        );
      free ((VOID *) MemoryImagePointer);
      free (TEBuffer);
      return EFI_ABORTED;
    }

    if (*PeHdrSizeOfImage != ImageContext.ImageSize) {
      *PeHdrSizeOfImage = (UINT32) ImageContext.ImageSize;
      if (*PeHdrChecksum) {
        *PeHdrChecksum = 0;
      }
    }

    TEImageHeader->ImageBase = (UINT64) (NewPe32BaseAddress + TEImageHeader->StrippedSize - sizeof (EFI_TE_IMAGE_HEADER));
    memcpy (
      (UINT8 *) (CurrentPe32Section.Pe32Section + 1) + sizeof (EFI_TE_IMAGE_HEADER),
      (VOID *) ((UINT8 *) MemoryImagePointerAligned + TEImageHeader->StrippedSize),
      GetLength (CurrentPe32Section.Pe32Section->CommonHeader.Size) - sizeof (EFI_PE32_SECTION) -
      sizeof (EFI_TE_IMAGE_HEADER)
      );
    
    //
    // Get EntryPoint in Flash Region.
    //
    EntryPoint = NewPe32BaseAddress + EntryPoint - ImageAddress;

    //
    // If a map file was selected output mapping information for any file that
    // was rebased.
    //
    if (MapFile != NULL) {
      fprintf (MapFile, "TE   File: %s Base:%08lx", FileGuidString, BaseAddress);
      fprintf (MapFile, " EntryPoint:%08lx", EntryPoint);
      if (ImageContext.PdbPointer != NULL) {
        fprintf (MapFile, " FileName: %s", ImageContext.PdbPointer);
      }
      fprintf (MapFile, "\n");
    }

    free ((VOID *) MemoryImagePointer);
    free (TEBuffer);
    if (FfsFile->Attributes & FFS_ATTRIB_TAIL_PRESENT) {
      TailSize = sizeof (EFI_FFS_FILE_TAIL);
    } else {
      TailSize = 0;
    }
    //
    // Now update file checksum
    //
    if (FfsFile->Attributes & FFS_ATTRIB_CHECKSUM) {
      SavedState  = FfsFile->State;
      FfsFile->IntegrityCheck.Checksum.File = 0;
      FfsFile->State                        = 0;
      if (FfsFile->Attributes & FFS_ATTRIB_CHECKSUM) {
        FfsFile->IntegrityCheck.Checksum.File = CalculateChecksum8 (
                                                  (UINT8 *) FfsFile,
                                                  GetLength (FfsFile->Size) - TailSize
                                                  );
      } else {
        FfsFile->IntegrityCheck.Checksum.File = FFS_FIXED_CHECKSUM;
      }

      FfsFile->State = SavedState;
    }
    //
    // Update tail if present
    //
    if (FfsFile->Attributes & FFS_ATTRIB_TAIL_PRESENT) {
      TailValue = (EFI_FFS_FILE_TAIL) (~(FfsFile->IntegrityCheck.TailReference));
      *(EFI_FFS_FILE_TAIL *) (((UINTN) FfsFile + GetLength (FfsFile->Size) - sizeof (EFI_FFS_FILE_TAIL))) = TailValue;
    }
  }
  //
  // If we found no files, then emit an error if no compressed sections either
  //
  if (FoundCount == 0) {
    Status = GetSectionByType (FfsFile, EFI_SECTION_COMPRESSION, Index, &CurrentPe32Section);
    if (EFI_ERROR (Status)) {
      Error (NULL, 0, 0, "no PE32, TE, nor compressed section found in FV file", FileGuidString);
      return EFI_NOT_FOUND;
    }
  }
  
  return EFI_SUCCESS;
}

EFI_STATUS
FfsRebaseImageRead (
  IN     VOID    *FileHandle,
  IN     UINTN   FileOffset,
  IN OUT UINT32  *ReadSize,
  OUT    VOID    *Buffer
  )
/*++

Routine Description:

  Support routine for the PE/COFF Loader that reads a buffer from a PE/COFF file

Arguments:

  FileHandle - The handle to the PE/COFF file

  FileOffset - The offset, in bytes, into the file to read

  ReadSize   - The number of bytes to read from the file starting at FileOffset

  Buffer     - A pointer to the buffer to read the data into.

Returns:

  EFI_SUCCESS - ReadSize bytes of data were read into Buffer from the PE/COFF file starting at FileOffset

--*/
{
  CHAR8   *Destination8;
  CHAR8   *Source8;
  UINT32  Length;

  Destination8  = Buffer;
  Source8       = (CHAR8 *) ((UINTN) FileHandle + FileOffset);
  Length        = *ReadSize;
  while (Length--) {
    *(Destination8++) = *(Source8++);
  }

  return EFI_SUCCESS;
}
