/** @file
  Provides library functions for common SMBIOS operations. Only available to DXE
  and UEFI module types.


Copyright (c) 2012, Apple Inc. All rights reserved.
Portitions Copyright (c) 2006 - 2011, Intel Corporation. All rights reserved.<BR>
This program and the accompanying materials are licensed and made available under 
the terms and conditions of the BSD License that accompanies this distribution.  
The full text of the license may be found at
http://opensource.org/licenses/bsd-license.php.

THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/
  
#include <PiDxe.h>
#include <Library/BaseLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/DebugLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiLib.h>
#include <Library/SmbiosLib.h>


EFI_SMBIOS_PROTOCOL *gSmbios = NULL;


EFI_STATUS
EFIAPI
InitializeSmbiosTableFromTemplate (
  IN  SMBIOS_TEMPLATE_ENTRY   *Template
  )
{
  EFI_STATUS    Status;
  UINTN         Index;

  if (Template == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  for (Index = 0; Template[Index].Entry != NULL; Index++) {
    Status = CreateSmbiosEntry (Template[Index].Entry, Template[Index].StringArray);
  }
  
  return Status;
}



/**
  Create SMBIOS record.

  Converts a fixed SMBIOS structure and an array of pointers to strings into
  an SMBIOS record where the strings are cat'ed on the end of the fixed record
  and terminated via a double NULL and add to SMBIOS table.

  SMBIOS_TABLE_TYPE32 gSmbiosType12 = {
    { EFI_SMBIOS_TYPE_SYSTEM_CONFIGURATION_OPTIONS, sizeof (SMBIOS_TABLE_TYPE12), 0 },
    1 // StringCount
  };
  CHAR8 *gSmbiosType12Strings[] = {
    "Not Found",
    NULL
  };
  
  ...
  CreateSmbiosEntry (
    (EFI_SMBIOS_TABLE_HEADER*)&gSmbiosType12, 
    gSmbiosType12Strings
    );

  @param  SmbiosEntry   Fixed SMBIOS structure
  @param  StringArray   Array of strings to convert to an SMBIOS string pack. 
                        NULL is OK.

**/
EFI_STATUS
EFIAPI
CreateSmbiosEntry (
  IN  SMBIOS_STRUCTURE *SmbiosEntry,
  IN  CHAR8            **StringArray 
  )
{
  EFI_STATUS                Status;
  EFI_SMBIOS_HANDLE         SmbiosHandle;
  EFI_SMBIOS_TABLE_HEADER   *Record;
  UINTN                     Index;
  UINTN                     StringSize;
  UINTN                     Size;
  CHAR8                     *Str;

  // Calculate the size of the fixed record and optional string pack
  Size = SmbiosEntry->Length;
  if (StringArray == NULL) { 
    Size += 2; // Min string section is double null
  } else if (StringArray[0] == NULL) {
    Size += 2; // Min string section is double null
  } else {
    for (Index = 0; StringArray[Index] != NULL; Index++) {
      StringSize = AsciiStrSize (StringArray[Index]);
      Size += StringSize;
    }
    // Don't forget the terminating double null
    Size += 1;
  }

  // Copy over Template
  Record = (EFI_SMBIOS_TABLE_HEADER *)AllocateZeroPool (Size);
  if (Record == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }
  CopyMem (Record, SmbiosEntry, SmbiosEntry->Length);

  if (StringArray != NULL) {
    // Append string pack
    Str = ((CHAR8 *)Record) + Record->Length;
    for (Index = 0; StringArray[Index] != NULL; Index++) {
      StringSize = AsciiStrSize (StringArray[Index]);
      CopyMem (Str, StringArray[Index], StringSize);
      Str += StringSize;
    }
    *Str = 0;
  }  

  SmbiosHandle = SMBIOS_HANDLE_PI_RESERVED;
  Status = gSmbios->Add (
                     gSmbios,
                     gImageHandle,
                     &SmbiosHandle,
                     Record
                     );
  
  FreePool (Record);
  return Status;
}



/**
  Update the string associated with an existing SMBIOS record.
  
  This function allows the update of specific SMBIOS strings. The number of valid strings for any
  SMBIOS record is defined by how many strings were present when Add() was called.
  
  @param[in]    SmbiosHandle    SMBIOS Handle of structure that will have its string updated.
  @param[in]    StringNumber    The non-zero string number of the string to update.
  @param[in]    String          Update the StringNumber string with String.
  
  @retval EFI_SUCCESS           SmbiosHandle had its StringNumber String updated.
  @retval EFI_INVALID_PARAMETER SmbiosHandle does not exist. Or String is invalid.
  @retval EFI_UNSUPPORTED       String was not added because it is longer than the SMBIOS Table supports.
  @retval EFI_NOT_FOUND         The StringNumber.is not valid for this SMBIOS record.    
**/
EFI_STATUS
EFIAPI
SmbiosUpdateString (
  IN  EFI_SMBIOS_HANDLE     SmbiosHandle,
  IN  SMBIOS_TABLE_STRING   StringNumber,
  IN  CHAR8                 *String
  )
{
  UINTN StringIndex;

  if (String == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  if (*String == '\0') {
    // A string with no data is not legal in SMBIOS
    return EFI_INVALID_PARAMETER;
  }

  StringIndex = StringNumber;
  return gSmbios->UpdateString (gSmbios, &SmbiosHandle, &StringIndex, String);
}


/**
  Update the string associated with an existing SMBIOS record.
  
  This function allows the update of specific SMBIOS strings. The number of valid strings for any
  SMBIOS record is defined by how many strings were present when Add() was called.
  
  @param[in]    SmbiosHandle    SMBIOS Handle of structure that will have its string updated.
  @param[in]    StringNumber    The non-zero string number of the string to update.
  @param[in]    String          Update the StringNumber string with String.
  
  @retval EFI_SUCCESS           SmbiosHandle had its StringNumber String updated.
  @retval EFI_INVALID_PARAMETER SmbiosHandle does not exist. Or String is invalid.
  @retval EFI_UNSUPPORTED       String was not added because it is longer than the SMBIOS Table supports.
  @retval EFI_NOT_FOUND         The StringNumber.is not valid for this SMBIOS record.    
**/
EFI_STATUS
EFIAPI
SmbiosUpdateUnicodeString (
  IN  EFI_SMBIOS_HANDLE     SmbiosHandle,
  IN  SMBIOS_TABLE_STRING   StringNumber,
  IN  CHAR16                *String
  )
{
  EFI_STATUS  Status;
  UINTN       StringIndex;
  CHAR8       *Ascii;

  if (String == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  if (*String == '\0') {
    // A string with no data is not legal in SMBIOS
    return EFI_INVALID_PARAMETER;
  }

  Ascii = AllocateZeroPool (StrSize (String));
  if (Ascii == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }
  UnicodeStrToAsciiStr (String, Ascii);

  StringIndex = StringNumber;
  Status = gSmbios->UpdateString (gSmbios, &SmbiosHandle, &StringIndex, Ascii);
  
  FreePool (Ascii);
  return Status;
}


/**
  Allow caller to read a specific SMBIOS string
  
  @param[in]    Header          SMBIOS record that contains the string. 
  @param[in[    Intance         Instance of SMBIOS string 0 - N-1. 

  @retval NULL                  Instance of Type SMBIOS string was not found. 
  @retval Other                 Pointer to matching SMBIOS string. 
**/
CHAR8 *
SmbiosReadString (
  IN SMBIOS_STRUCTURE *Header,
  IN UINTN            Instance
  )
{
  CHAR8       *Data;
  UINTN       NullCount;
  
  Data = (CHAR8 *)Header + Header->Length;
  for (NullCount = 0;!(*Data == 0 && *(Data+1) == 0); ) {
    if (Instance == NullCount) {
      return Data;
    }
    Data++;
    if (*(Data - 1) == '\0') {
      NullCount++;
    }
  } 

  return NULL;
}


/**
  Allow the caller to discover a specific SMBIOS entry, and patch it if necissary. 
  
  @param[in]    Type            Type of the next SMBIOS record to return. 
  @param[in[    Instance        Instance of SMBIOS record 0 - N-1. 
  @param[out]   SmbiosHandle    Returns SMBIOS handle for the matching record. 

  @retval NULL                  Instance of Type SMBIOS record was not found. 
  @retval Other                 Pointer to matching SMBIOS record. 
**/
SMBIOS_STRUCTURE *
EFIAPI
SmbiosGetRecord (
  IN  EFI_SMBIOS_TYPE   Type,
  IN  UINTN             Instance,
  OUT EFI_SMBIOS_HANDLE *SmbiosHandle
  )
{
  EFI_STATUS              Status;
  EFI_SMBIOS_TABLE_HEADER *Record;
  UINTN                   Match;

  Match         = 0;
  *SmbiosHandle = SMBIOS_HANDLE_PI_RESERVED;
  do {
    Status = gSmbios->GetNext (gSmbios, SmbiosHandle, &Type, &Record, NULL);
    if (!EFI_ERROR (Status)) {
      if (Match == Instance) {
        return (SMBIOS_STRUCTURE *)Record;
      }
      Match++;
    }
  } while (!EFI_ERROR (Status));

  return NULL;
}


/**
  Remove an SMBIOS record.
  
  This function removes an SMBIOS record using the handle specified by SmbiosHandle.
  
  @param[in]    SmbiosHandle        The handle of the SMBIOS record to remove.
  
  @retval EFI_SUCCESS               SMBIOS record was removed.
  @retval EFI_INVALID_PARAMETER     SmbiosHandle does not specify a valid SMBIOS record.
**/
EFI_STATUS
EFIAPI
SmbiosRemove (
  OUT EFI_SMBIOS_HANDLE SmbiosHandle
  )
{
  return gSmbios->Remove (gSmbios, SmbiosHandle);
}


EFI_STATUS
EFIAPI
SmbiosGetVersion (
  OUT UINT8   *SmbiosMajorVersion,
  OUT UINT8   *SmbiosMinorVersion
  )
{
  if (SmbiosMajorVersion != NULL) {
    *SmbiosMajorVersion = gSmbios->MajorVersion;
  }
  if (SmbiosMinorVersion != NULL) {
    *SmbiosMinorVersion = gSmbios->MinorVersion;
  }
  return EFI_SUCCESS;
}  


/**
  

  @param  ImageHandle  ImageHandle of the loaded driver.
  @param  SystemTable  Pointer to the EFI System Table.

  @retval  EFI_SUCCESS            Register successfully.
  @retval  EFI_OUT_OF_RESOURCES   No enough memory to register this handler.
**/
EFI_STATUS
EFIAPI
SmbiosLibConstructor (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
{
  return gBS->LocateProtocol (&gEfiSmbiosProtocolGuid, NULL, (VOID **)&gSmbios);
}

