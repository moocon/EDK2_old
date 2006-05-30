/*++

Copyright (c)  2005 Intel Corporation. All rights reserved
This software and associated documentation (if any) is furnished
under a license and may only be used or copied in accordance
with the terms of the license. Except as permitted by such
license, no part of this software or documentation may be
reproduced, stored in a retrieval system, or transmitted in any
form or by any means without the express written consent of
Intel Corporation.


Module Name:

    SecApResetVectorFixup.c

Abstract:

    This utility is part of build process for IA32 Fvrecovery.fv whose total size
    is larger than 128kB so that we cannot use GenFvImage utility to put Ap reset 
    vector at the zero vector of Fv header.
    
      PEI FV after using the tool
    
    -------------------------
    |zzz                    |
    |                       |
    |                       |
    |      FFS              |
    |                       |
    |                       |
    |                       |
    |---------------------- |
    |       PAD             |
    |                       |
    |.......................|  --- 
    |                       |   |
    |xxx                    |   | 128K    
    |---------------------- |   | 
    |       VTF (SEC)       |   |
    -------------------------  ---
    
    1. zzz --> Zero vector, which is beyond the 128K limited address space
    2. xxx --> AP reset vector at 4K alignment below 128K and it is in the PAD
       file area.
    3. After the build process ,the PAD guid is changed to a new GUID to avoid 
       the PAD definition confusing. If there is some problem, try to disable
       UpdatePadFileGuid
    
     
    
--*/

#include "SecApResetVectorFixup.h"


EFI_GUID  DefaultFvPadFileNameGuid = { 0x78f54d4, 0xcc22, 0x4048, 0x9e, 0x94, 0x87, 0x9c, 0x21, 0x4d, 0x56, 0x2f };
EFI_GUID  NewFvPadFileNameGuid     = { 0x145372bc, 0x66b9, 0x476d, 0x81, 0xbc, 0x21, 0x27, 0xc3, 0x76, 0xbb, 0x66 };

//
// jmp 0xf000:0xffd0 (0xFFFFFFD0)
//
UINT8 ApResetVector[5] = {0xEA, 0xD0, 0xFF, 0x00, 0xF0};

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
    "%s - Tiano IA32 SEC Ap Reset Vector Fixup Utility."" Version %i.%i\n\n",
    UTILITY_NAME,
    UTILITY_MAJOR_VERSION,
    UTILITY_MINOR_VERSION
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
  printf ("Usage: %s InputFvrecoveryFile OutputFvrecoveryFile\n", UTILITY_NAME);
  printf ("  Where:\n");
  printf ("\tInputFvrecoveryFile   - Name of the IA32 input Fvrecovery.fv file.\n");
  printf ("\tOutputFvrecoveryFile  - Name of the IA32 output Fvrecovery.fv file.\n");
}


VOID 
UpdatePadFileGuid (
  IN     EFI_FIRMWARE_VOLUME_HEADER  *FvHeader,
  IN     EFI_FFS_FILE_HEADER         *FileHeader,
  IN     UINT32                      FileLength,
  IN OUT EFI_GUID                    *Guid
  )
/*++

Routine Description:

  Update the Pad File Guid to change it to other guid and update
  the checksum

Arguments:
  FvHeader   - EFI_FIRMWARE_VOLUME_HEADER 
  FileHeader - The FFS PAD file header.
  FileLength - The FFS PAD file length.
  Guid       - The Guid to compare and if it is PAD Guid, update it to new Guid
Returns:
  VOID
--*/

{
  if ((CompareGuid (Guid, (EFI_GUID *)&DefaultFvPadFileNameGuid)) == 0) {
    //
    // Set new Pad file guid
    // 
    memcpy (Guid, &NewFvPadFileNameGuid, sizeof (EFI_GUID));



    FileHeader->Type       = EFI_FV_FILETYPE_FFS_PAD;
    FileHeader->Attributes = 0;
    //
    // Fill in checksums and state, must be zero during checksum calculation.
    //
    FileHeader->IntegrityCheck.Checksum.Header = 0;
    FileHeader->IntegrityCheck.Checksum.File   = 0;
    FileHeader->State                          = 0;
    FileHeader->IntegrityCheck.Checksum.Header = CalculateChecksum8 ((UINT8 *) FileHeader, sizeof (EFI_FFS_FILE_HEADER));
    if (FileHeader->Attributes & FFS_ATTRIB_CHECKSUM) {
      FileHeader->IntegrityCheck.Checksum.File = CalculateChecksum8 ((UINT8 *) FileHeader, FileLength);
    } else {
      FileHeader->IntegrityCheck.Checksum.File = FFS_FIXED_CHECKSUM;
    }

    FileHeader->State = EFI_FILE_HEADER_CONSTRUCTION | EFI_FILE_HEADER_VALID | EFI_FILE_DATA_VALID;

    if (FvHeader->Attributes & EFI_FVB_ERASE_POLARITY) {
      FileHeader->State = (UINT8)~(FileHeader->State);
    }    
  }
  
}


STATUS
main (
  IN INTN   argc,
  IN CHAR8  **argv
  )
/*++

Routine Description:

  Main function.

Arguments:

  argc - Number of command line parameters.
  argv - Array of pointers to parameter strings.

Returns:
  STATUS_SUCCESS - Utility exits successfully.
  STATUS_ERROR   - Some error occurred during execution.

--*/
{
  FILE                        *FpIn;
  FILE                        *FpOut;
  UINT32                      FvrecoveryFileSize;
  UINT8                       *FileBuffer;
  UINT8                       *FileBufferRaw;
  UINT64                      FvLength;
  UINT32                      Offset;
  UINT32                      FileLength;
  UINT32                      FileOccupiedSize;
  EFI_FIRMWARE_VOLUME_HEADER  *FvHeader;
  EFI_FFS_FILE_HEADER         *FileHeader;
  EFI_GUID                    *TempGuid;
  UINT8                       *FixPoint;
  UINT32                      TempResult;
  UINT32                      Index;
  UINT32                      IpiVector;

  TempGuid = NULL;
  SetUtilityName (UTILITY_NAME);

  //
  // Display utility information
  //
  PrintUtilityInfo ();

  //
  // Verify the correct number of arguments
  //
  if (argc != MAX_ARGS) {
    Error (NULL, 0, 0, "invalid number of input parameters specified", NULL);
    PrintUsage ();
    return STATUS_ERROR;
  }
  //
  // Open the Input Fvrecovery.fv file
  //
  if ((FpIn = fopen (argv[1], "rb")) == NULL) {
    Error (NULL, 0, 0, "Unable to open file", argv[1]);
    return STATUS_ERROR;
  }
  //
  // Get the Input Fvrecovery.fv file size
  //
  fseek (FpIn, 0, SEEK_END);
  FvrecoveryFileSize = ftell (FpIn);
  //
  // Read the contents of input file to memory buffer
  //
  FileBuffer    = NULL;
  FileBufferRaw = NULL;
  FileBufferRaw = (UINT8 *) malloc (FvrecoveryFileSize + 0x10000);
  if (NULL == FileBufferRaw) {
    Error (NULL, 0, 0, "No sufficient memory to allocate!", NULL);
    fclose (FpIn);
    return STATUS_ERROR;
  }
  TempResult = 0x10000 - ((UINT32)FileBufferRaw & 0x0FFFF);
  FileBuffer = (UINT8 *)((UINT32)FileBufferRaw + TempResult);
  fseek (FpIn, 0, SEEK_SET);
  TempResult = fread (FileBuffer, 1, FvrecoveryFileSize, FpIn);
  if (TempResult != FvrecoveryFileSize) {
    Error (NULL, 0, 0, "Read input file error!", NULL);
    free ((VOID *)FileBufferRaw);
    fclose (FpIn);
    return STATUS_ERROR;
  }
  //
  // Close the input Fvrecovery.fv file
  //
  fclose (FpIn);
  //
  // Find the pad FFS file
  //
  FvHeader         = (EFI_FIRMWARE_VOLUME_HEADER *)FileBuffer;
  FvLength         = FvHeader->FvLength;
  FileHeader       = (EFI_FFS_FILE_HEADER *)(FileBuffer + FvHeader->HeaderLength);
  FileLength       = (*(UINT32 *)(FileHeader->Size)) & 0x00FFFFFF;
  FileOccupiedSize = GETOCCUPIEDSIZE(FileLength, 8);
  Offset           = (UINT32)FileHeader - (UINT32)FileBuffer;
  
  while (Offset < FvLength) {
    TempGuid = (EFI_GUID *)&(FileHeader->Name);
    FileLength = (*(UINT32 *)(FileHeader->Size)) & 0x00FFFFFF;
    FileOccupiedSize = GETOCCUPIEDSIZE(FileLength, 8);
    if ((CompareGuid (TempGuid, (EFI_GUID *)&DefaultFvPadFileNameGuid)) == 0) {
      break;
    }
    FileHeader = (EFI_FFS_FILE_HEADER *)((UINT32)FileHeader + FileOccupiedSize);
    Offset = (UINT32)FileHeader - (UINT32)FileBuffer;
  }

  if (Offset >= FvLength) {
    Error (NULL, 0, 0, "No pad file found!", NULL);
    free ((VOID *)FileBufferRaw);
    return STATUS_ERROR;
  }
  //
  // Find the position to place Ap reset vector, the offset
  // between the position and the end of Fvrecovery.fv file
  // should not exceed 128kB to prevent Ap reset vector from
  // outside legacy E and F segment
  //
  FixPoint = (UINT8 *)(FileHeader + sizeof(EFI_FFS_FILE_HEADER));
  TempResult = 0x1000 - ((UINT32)FixPoint & 0x0FFF);
  FixPoint +=TempResult;
  if (((UINT32)FixPoint - (UINT32)FileHeader + 5) > FileOccupiedSize) {
    Error (NULL, 0, 0, "No appropriate space in pad file to add Ap reset vector!", NULL);
    free ((VOID *)FileBufferRaw);
    return STATUS_ERROR;  
  }
  while (((UINT32)FixPoint - (UINT32)FileHeader + 5) <= FileOccupiedSize) {
    FixPoint += 0x1000;
  }
  FixPoint -= 0x1000;
  if ((UINT32)FvHeader + FvLength - (UINT32)FixPoint > 0x20000) {
    Error (NULL, 0, 0, "The position to place Ap reset vector is not in E and F segment!", NULL);
    free ((VOID *)FileBufferRaw);
    return STATUS_ERROR; 
  }  	
  //
  // Fix up Ap reset vector and calculate the IPI vector
  //
  for (Index = 0; Index < 5; Index++) {
    FixPoint[Index] = ApResetVector[Index];
  } 
  TempResult = 0x0FFFFFFFF - ((UINT32)FvHeader + (UINT32)FvLength - 1 - (UINT32)FixPoint);
  TempResult >>= 12;
  IpiVector = TempResult & 0x0FF;
  
  
  UpdatePadFileGuid (FvHeader, FileHeader, FileLength, TempGuid);

  //
  // Open the output Fvrecovery.fv file
  //
  if ((FpOut = fopen (argv[2], "w+b")) == NULL) {
    Error (NULL, 0, 0, "Unable to open file", argv[2]);
    free ((VOID *)FileBufferRaw);
    return STATUS_ERROR;
  }
  //
  // Write the output Fvrecovery.fv file
  //
  if ((fwrite (FileBuffer, 1, FvrecoveryFileSize, FpOut)) != FvrecoveryFileSize) {
    Error (NULL, 0, 0, "Write output file error!", NULL);
    free ((VOID *)FileBufferRaw);
    return STATUS_ERROR;   
  }
  //
  //
  //
  fseek (FpOut, -8, SEEK_END);
  if ((fwrite (&IpiVector, 1, sizeof(UINT32), FpOut)) != sizeof(UINT32)) {
    Error (NULL, 0, 0, "Write output file error!", NULL);
    free ((VOID *)FileBufferRaw);
    return STATUS_ERROR;
  }  
  //
  // Close the output Fvrecovery.fv file
  //
  fclose (FpOut);
  free ((VOID *)FileBufferRaw);
  return STATUS_SUCCESS;
}

