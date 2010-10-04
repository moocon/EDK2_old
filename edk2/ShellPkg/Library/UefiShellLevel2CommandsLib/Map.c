/** @file
  Main file for map shell level 2 command.

  Copyright (c) 2009 - 2010, Intel Corporation. All rights reserved.<BR>
  This program and the accompanying materials
  are licensed and made available under the terms and conditions of the BSD License
  which accompanies this distribution.  The full text of the license may be found at
  http://opensource.org/licenses/bsd-license.php

  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

#include "UefiShellLevel2CommandsLib.h"
#include <Protocol/SimpleFileSystem.h>
#include <Protocol/BlockIo.h>
#include <Library/DevicePathLib.h>
#include <Library/HandleParsingLib.h>
#include <Library/SortLib.h>

BOOLEAN
EFIAPI
SearchList(
  IN CONST CHAR16   *List,
  IN CONST CHAR16   *MetaTarget,
  OUT CHAR16        **FullName OPTIONAL,
  IN CONST BOOLEAN  Meta,
  IN CONST BOOLEAN  SkipTrailingNumbers,
  IN CONST CHAR16   *Target

  )
{
  CHAR16        *TempList;
  CONST CHAR16  *ListWalker;
  BOOLEAN       Result;
  CHAR16        *TempSpot;

  for (ListWalker = List , TempList = NULL
     ; ListWalker != NULL && *ListWalker != CHAR_NULL
     ;
   ) {
    TempList = StrnCatGrow(&TempList, NULL, ListWalker, 0);
    ASSERT(TempList != NULL);
    TempSpot = StrStr(TempList, Target);
    if (TempSpot != NULL) {
      *TempSpot = CHAR_NULL;
    }

    while (SkipTrailingNumbers && (ShellIsDecimalDigitCharacter(TempList[StrLen(TempList)-1]) || TempList[StrLen(TempList)-1] == L':')) {
      TempList[StrLen(TempList)-1] = CHAR_NULL;
    }

    ListWalker = StrStr(ListWalker, Target);
    while(ListWalker != NULL && *ListWalker == *Target) {
      ListWalker++;
    }
    if (Meta) {
      Result = gUnicodeCollation->MetaiMatch(gUnicodeCollation, (CHAR16*)TempList, (CHAR16*)MetaTarget);
    } else {
      Result = (BOOLEAN)(StrCmp(TempList, MetaTarget)==0);
    }
    if (Result) {
      if (FullName != NULL) {
        *FullName = TempList;
      } else {
        FreePool(TempList);
      }
      return (TRUE);
    }
    FreePool(TempList);
    TempList = NULL;
  }

  return (FALSE);
}

EFI_STATUS
EFIAPI
UpdateMapping (
  VOID
  )
{
  EFI_STATUS                Status;
  EFI_HANDLE                *HandleList;
  UINTN                     Count;
  EFI_DEVICE_PATH_PROTOCOL  **DevicePathList;
  CHAR16                    *NewDefaultName;
  CHAR16                    *NewConsistName;
  EFI_DEVICE_PATH_PROTOCOL  **ConsistMappingTable;

  HandleList  = NULL;
  Status      = EFI_SUCCESS;

  //
  // remove mappings that represent removed devices.
  //

  //
  // Find each handle with Simple File System
  //
  HandleList = GetHandleListByProtocol(&gEfiSimpleFileSystemProtocolGuid);
  if (HandleList != NULL) {
    //
    // Do a count of the handles
    //
    for (Count = 0 ; HandleList[Count] != NULL ; Count++);

    //
    // Get all Device Paths
    //
    DevicePathList = AllocatePool(sizeof(EFI_DEVICE_PATH_PROTOCOL*) * Count);
    ASSERT(DevicePathList != NULL);

    for (Count = 0 ; HandleList[Count] != NULL ; Count++) {
      DevicePathList[Count] = DevicePathFromHandle(HandleList[Count]);
    }

    //
    // Sort all DevicePaths
    //
    PerformQuickSort(DevicePathList, Count, sizeof(EFI_DEVICE_PATH_PROTOCOL*), DevicePathCompare);

    ShellCommandConsistMappingInitialize(&ConsistMappingTable);

    //
    // Assign new Mappings to remainders
    //
    for (Count = 0 ; HandleList[Count] != NULL && !EFI_ERROR(Status); Count++) {
      //
      // Skip ones that already have
      //
      if (gEfiShellProtocol->GetMapFromDevicePath(&DevicePathList[Count]) != NULL) {
        continue;
      }
      //
      // Get default name
      //
      NewDefaultName = ShellCommandCreateNewMappingName(MappingTypeFileSystem);
      ASSERT(NewDefaultName != NULL);

      //
      // Call shell protocol SetMap function now...
      //
      Status = gEfiShellProtocol->SetMap(DevicePathList[Count], NewDefaultName);

      if (!EFI_ERROR(Status)) {
        //
        // Now do consistent name
        //
        NewConsistName = ShellCommandConsistMappingGenMappingName(DevicePathList[Count], ConsistMappingTable);
        if (NewConsistName != NULL) {
          Status = gEfiShellProtocol->SetMap(DevicePathList[Count], NewConsistName);
          FreePool(NewConsistName);
        }
      }

      FreePool(NewDefaultName);
    }
    ShellCommandConsistMappingUnInitialize(ConsistMappingTable);
    SHELL_FREE_NON_NULL(HandleList);
    SHELL_FREE_NON_NULL(DevicePathList);

    HandleList = NULL;
  } else {
    Count = (UINTN)-1;
  }
  //
  // Do it all over again for gEfiBlockIoProtocolGuid
  //

  return (Status);
}

CHAR16*
EFIAPI
GetDeviceMediaType (
  IN  EFI_DEVICE_PATH_PROTOCOL     *DevicePath
  )
{
  ACPI_HID_DEVICE_PATH  *Acpi;

  //
  //  Parse the device path:
  //  Devicepath sub type                 mediatype
  //    MEDIA_HANRDDRIVE_DP      ->       Hard Disk
  //    MEDIA_CDROM_DP           ->       CD Rom
  //    Acpi.HID = 0X0604        ->       Floppy
  //
  if (NULL == DevicePath) {
    return HiiGetString(gShellLevel2HiiHandle, STRING_TOKEN(STR_MAP_MEDIA_UNKNOWN), NULL);
  }

  for (;!IsDevicePathEndType (DevicePath) ;DevicePath = NextDevicePathNode (DevicePath)) {
    if (DevicePathType (DevicePath) == MEDIA_DEVICE_PATH) {
      switch (DevicePathSubType (DevicePath)) {
      case MEDIA_HARDDRIVE_DP:
        return HiiGetString(gShellLevel2HiiHandle, STRING_TOKEN(STR_MAP_MEDIA_HARDDISK), NULL);
      case MEDIA_CDROM_DP:
        return HiiGetString(gShellLevel2HiiHandle, STRING_TOKEN(STR_MAP_MEDIA_CDROM), NULL);
      }
    } else if (DevicePathType (DevicePath) == ACPI_DEVICE_PATH) {
      Acpi = (ACPI_HID_DEVICE_PATH *) DevicePath;
      if (EISA_ID_TO_NUM (Acpi->HID) == 0x0604) {
        return HiiGetString(gShellLevel2HiiHandle, STRING_TOKEN(STR_MAP_MEDIA_FLOPPY), NULL);
      }
    }
  }

  return HiiGetString(gShellLevel2HiiHandle, STRING_TOKEN(STR_MAP_MEDIA_UNKNOWN), NULL);
}

/**
  Function to detemine if a handle has removable storage.

  @param[in] DevicePath             DevicePath to test.

  @retval TRUE                      The handle has removable storage.
  @retval FALSE                     The handle does not have removable storage.
**/
BOOLEAN
EFIAPI
IsRemoveableDevice (
  IN EFI_DEVICE_PATH_PROTOCOL      *DevicePath
  )
{
  if (NULL == DevicePath) {
    return FALSE;
  }

  while (!IsDevicePathEndType (DevicePath)) {
    if (DevicePathType (DevicePath) == MESSAGING_DEVICE_PATH) {
      switch (DevicePathSubType (DevicePath)) {
      case MSG_USB_DP:
      case MSG_SCSI_DP:
        return TRUE;
      default:
        return FALSE;
      }
    }
    DevicePath = NextDevicePathNode (DevicePath);
  }
  return FALSE;
}

/**
  Function to detemine if a something on the map list matches.

  @param[in] MapList          The pointer to the list to test.
  @param[in] Specific         The pointer to a specific name to test for.
  @param[in] TypeString       The pointer to the list of types.
  @param[in] Normal           Always show normal mappings.
  @param[in] Consist          Always show consistent mappings.

  @retval TRUE                The map should be displayed.
  @retval FALSE               The map should not be displayed.
**/
BOOLEAN
EFIAPI
MappingListHasType(
  IN CONST CHAR16     *MapList,
  IN CONST CHAR16     *Specific,
  IN CONST CHAR16     *TypeString,
  IN CONST BOOLEAN    Normal,
  IN CONST BOOLEAN    Consist
  )
{
  //
  // specific has priority
  //
  if ( Specific != NULL
    && SearchList(MapList, Specific, NULL, TRUE, FALSE, L";")) {
    return (TRUE);
  }

  if (  Consist
    && (SearchList(MapList, L"HD*",  NULL, TRUE, TRUE, L";")
      ||SearchList(MapList, L"CD*",  NULL, TRUE, TRUE, L";")
      ||SearchList(MapList, L"F*",   NULL, TRUE, TRUE, L";")
      ||SearchList(MapList, L"FP*",  NULL, TRUE, TRUE, L";"))){
    return (TRUE);
  }

  if (  Normal
    && (SearchList(MapList, L"FS",  NULL, FALSE, TRUE, L";")
      ||SearchList(MapList, L"BLK", NULL, FALSE, TRUE, L";"))){
    return (TRUE);
  }

  if (TypeString != NULL && SearchList(MapList, TypeString,  NULL, TRUE, TRUE, L";")) {
    return (TRUE);
  }
  return (FALSE);
}


VOID
EFIAPI
PerformSingleMappingDisplay(
  IN CONST BOOLEAN    Verbose,
  IN CONST BOOLEAN    Consist,
  IN CONST BOOLEAN    Normal,
  IN CONST CHAR16     *TypeString,
  IN CONST BOOLEAN    SFO,
  IN CONST CHAR16     *Specific OPTIONAL,
  IN CONST EFI_HANDLE Handle
  )
{
  EFI_DEVICE_PATH_PROTOCOL  *DevPath;
  EFI_DEVICE_PATH_PROTOCOL  *DevPathCopy;
  CONST CHAR16              *MapList;
  CHAR16                    *CurrentName;
  CHAR16                    *MediaType;
  CHAR16                    *DevPathString;
  CHAR16                    *TempSpot;
  UINTN                     TempLen;
  BOOLEAN                   Removable;
  CONST CHAR16              *TempSpot2;

  DevPath = DevicePathFromHandle(Handle);
  DevPathCopy = DevPath;
  MapList = gEfiShellProtocol->GetMapFromDevicePath(&DevPathCopy);
  if (MapList == NULL) {
    return;
  }

  if (!MappingListHasType(MapList, Specific, TypeString, Normal, Consist)){
    return;
  }

  CurrentName = NULL;
  CurrentName = StrnCatGrow(&CurrentName, 0, MapList, 0);
  TempSpot = StrStr(CurrentName, L";");
  if (TempSpot != NULL) {
    *TempSpot = CHAR_NULL;
  }
  DevPathString = gDevPathToText->ConvertDevicePathToText(DevPath, TRUE, FALSE);
  if (!SFO) {
    TempLen = StrLen(CurrentName);
    ShellPrintHiiEx (
      -1,
      -1,
      NULL,
      STRING_TOKEN (STR_MAP_ENTRY),
      gShellLevel2HiiHandle,
      CurrentName,
      TempLen < StrLen(MapList)?MapList + TempLen+1:L"",
      DevPathString
     );
    if (Verbose) {
      //
      // also print handle, media type, removable (y/n), and current directory
      //
      MediaType = GetDeviceMediaType(DevPath);
      if ((TypeString != NULL &&MediaType != NULL && StrStr(TypeString, MediaType) != NULL) || TypeString == NULL) {
        Removable = IsRemoveableDevice(DevPath);
        TempSpot2 = ShellGetCurrentDir(CurrentName);
        ShellPrintHiiEx (
          -1,
          -1,
          NULL,
          STRING_TOKEN (STR_MAP_ENTRY_VERBOSE),
          gShellLevel2HiiHandle,
          ConvertHandleToHandleIndex(Handle),
          MediaType,
          Removable?L"Yes":L"No",
          TempSpot2
         );
      }
      FreePool(MediaType);
    }
  } else {
    TempLen = StrLen(CurrentName);
    ShellPrintHiiEx (
      -1,
      -1,
      NULL,
      STRING_TOKEN (STR_MAP_SFO_MAPPINGS),
      gShellLevel2HiiHandle,
      CurrentName,
      DevPathString,
      TempLen < StrLen(MapList)?MapList + TempLen+1:L""
     );
  }
  FreePool(DevPathString);
  FreePool(CurrentName);
  return;
}

EFI_STATUS
EFIAPI
PerformSingleMappingDelete(
  IN CONST CHAR16     *Specific,
  IN CONST EFI_HANDLE Handle
  )
{
  EFI_DEVICE_PATH_PROTOCOL  *DevPath;
  EFI_DEVICE_PATH_PROTOCOL  *DevPathCopy;
  CONST CHAR16              *MapList;
  CHAR16                    *CurrentName;

  DevPath     = DevicePathFromHandle(Handle);
  DevPathCopy = DevPath;
  MapList     = gEfiShellProtocol->GetMapFromDevicePath(&DevPathCopy);
  CurrentName = NULL;

  if (MapList == NULL) {
    return (EFI_NOT_FOUND);
  }
  //
  // if there is a specific and its not on the list...
  //
  if (!SearchList(MapList, Specific, &CurrentName, TRUE, FALSE, L";")) {
    return (EFI_NOT_FOUND);
  }
  return (gEfiShellProtocol->SetMap(NULL, CurrentName));
}

/**
  Function to display mapping information to the user.

  if Specific is specified then Consist and Normal will be ignored since information will
  be printed for the specific item only.

  @param[in] Verbose                TRUE to display (extra) verbose information
  @param[in] Consist                TRUE to display consistent mappings
  @param[in] Normal                 TRUE to display normal (not consist) mappings
  @param[in] TypeString             pointer to string of filter types
  @param[in] SFO                    TRUE to display output in Standard Output Format
  @param[in] Specific               pointer to string for specific map to display

  @retval SHELL_SUCCESS               the display was printed
  @retval SHELL_INVALID_PARAMETER     one of Consist or Normal must be TRUE if no Specific

**/
CONST CHAR16 Cd[] = L"cd*";
CONST CHAR16 Hd[] = L"hd*";
CONST CHAR16 Fp[] = L"fp*";
CONST CHAR16 F[] = L"F*";
SHELL_STATUS
EFIAPI
PerformMappingDisplay(
  IN CONST BOOLEAN Verbose,
  IN CONST BOOLEAN Consist,
  IN CONST BOOLEAN Normal,
  IN CONST CHAR16  *TypeString,
  IN CONST BOOLEAN SFO,
  IN CONST CHAR16  *Specific OPTIONAL,
  IN CONST BOOLEAN Header
  )
{
  EFI_STATUS                Status;
  EFI_HANDLE                *HandleBuffer;
  UINTN                     BufferSize;
  UINTN                     LoopVar;
  CHAR16                    *Test;

  if (!Consist && !Normal && Specific == NULL && TypeString == NULL) {
    ShellPrintHiiEx(-1, -1, NULL, STRING_TOKEN (STR_GEN_TOO_FEW), gShellLevel2HiiHandle);
    return (SHELL_INVALID_PARAMETER);
  }

  if (TypeString != NULL) {
    Test = (CHAR16*)Cd;
    if (StrnCmp(TypeString, Test, StrLen(Test)-1) != 0) {
      Test = (CHAR16*)Hd;
      if (StrnCmp(TypeString, Test, StrLen(Test)-1) != 0) {
        Test = (CHAR16*)Fp;
        if (StrnCmp(TypeString, Test, StrLen(Test)-1) != 0) {
          ShellPrintHiiEx(-1, -1, NULL, STRING_TOKEN (STR_GEN_PROBLEM), gShellLevel2HiiHandle, TypeString);
          return (SHELL_INVALID_PARAMETER);
        }
      } else if (Test == NULL) {
        Test = (CHAR16*)F;
      }
    }
  } else {
    Test = NULL;
  }

  if (Header) {
    //
    // Print the header
    //
    if (!SFO) {
      ShellPrintHiiEx(-1, -1, NULL, STRING_TOKEN (STR_MAP_HEADER), gShellLevel2HiiHandle);
    } else {
      ShellPrintHiiEx(-1, -1, NULL, STRING_TOKEN (STR_GEN_SFO_HEADER), gShellLevel2HiiHandle, L"map");
    }
  }

  BufferSize    = 0;
  HandleBuffer  = NULL;

  //
  // Look up all SimpleFileSystems in the platform
  //
  Status = gBS->LocateHandle(
    ByProtocol,
    &gEfiDevicePathProtocolGuid,
    NULL,
    &BufferSize,
    HandleBuffer);
  if (Status == EFI_BUFFER_TOO_SMALL) {
    HandleBuffer = AllocatePool(BufferSize);
    if (HandleBuffer == NULL) {
      return (SHELL_OUT_OF_RESOURCES);
    }
    Status = gBS->LocateHandle(
      ByProtocol,
      &gEfiDevicePathProtocolGuid,
      NULL,
      &BufferSize,
      HandleBuffer);
  }
  ASSERT_EFI_ERROR(Status);
  ASSERT(HandleBuffer != NULL);

  //
  // Get the map name(s) for each one.
  //
  for ( LoopVar = 0
      ; LoopVar < (BufferSize / sizeof(EFI_HANDLE))
      ; LoopVar ++
     ){
    PerformSingleMappingDisplay(
      Verbose,
      Consist,
      Normal,
      Test,
      SFO,
      Specific,
      HandleBuffer[LoopVar]);
  }

  //
  // Look up all BlockIo in the platform
  //
  Status = gBS->LocateHandle(
    ByProtocol,
    &gEfiBlockIoProtocolGuid,
    NULL,
    &BufferSize,
    HandleBuffer);
  if (Status == EFI_BUFFER_TOO_SMALL) {
    FreePool(HandleBuffer);
    HandleBuffer = AllocatePool(BufferSize);
    if (HandleBuffer == NULL) {
      return (SHELL_OUT_OF_RESOURCES);
    }
    Status = gBS->LocateHandle(
      ByProtocol,
      &gEfiBlockIoProtocolGuid,
      NULL,
      &BufferSize,
      HandleBuffer);
  }
  if (!EFI_ERROR(Status)) {
    //
    // Get the map name(s) for each one.
    //
    for ( LoopVar = 0
        ; LoopVar < BufferSize / sizeof(EFI_HANDLE)
        ; LoopVar ++
       ){
      //
      // Skip any that were already done...
      //
      if (gBS->OpenProtocol(
        HandleBuffer[LoopVar],
        &gEfiDevicePathProtocolGuid,
        NULL,
        gImageHandle,
        NULL,
        EFI_OPEN_PROTOCOL_TEST_PROTOCOL) == EFI_SUCCESS) {
          continue;
      }
      PerformSingleMappingDisplay(
        Verbose,
        Consist,
        Normal,
        Test,
        SFO,
        Specific,
        HandleBuffer[LoopVar]);
    }
    FreePool(HandleBuffer);
  }
  return (SHELL_SUCCESS);
}

SHELL_STATUS
EFIAPI
PerformMappingDisplay2(
  IN CONST BOOLEAN Verbose,
  IN CONST BOOLEAN Consist,
  IN CONST BOOLEAN Normal,
  IN CONST CHAR16  *TypeString,
  IN CONST BOOLEAN SFO,
  IN CONST CHAR16  *Specific OPTIONAL
  )
{
  CONST CHAR16  *TypeWalker;
  SHELL_STATUS  ShellStatus;
  CHAR16        *Comma;


  if (TypeString == NULL) {
    return (PerformMappingDisplay(Verbose, Consist, Normal, NULL, SFO, Specific, TRUE));
  }
  ShellStatus = SHELL_SUCCESS;
  for (TypeWalker = TypeString ; TypeWalker != NULL && *TypeWalker != CHAR_NULL ;) {
    Comma = StrStr(TypeWalker, L",");
    if (Comma == NULL) {
      if (ShellStatus == SHELL_SUCCESS) {
        ShellStatus = PerformMappingDisplay(Verbose, Consist, Normal, TypeWalker, SFO, Specific, (BOOLEAN)(TypeWalker == TypeString));
      } else {
        PerformMappingDisplay(Verbose, Consist, Normal, TypeWalker, SFO, Specific, (BOOLEAN)(TypeWalker == TypeString));
      }
      break;
    } else {
      *Comma = CHAR_NULL;
      if (ShellStatus == SHELL_SUCCESS) {
        ShellStatus = PerformMappingDisplay(Verbose, Consist, Normal, TypeWalker, SFO, Specific, (BOOLEAN)(TypeWalker == TypeString));
      } else {
        PerformMappingDisplay(Verbose, Consist, Normal, TypeWalker, SFO, Specific, (BOOLEAN)(TypeWalker == TypeString));
      }
      *Comma = L',';
      TypeWalker = Comma + 1;
    }
  }

  return (ShellStatus);
}

EFI_STATUS
EFIAPI
PerformMappingDelete(
  IN CONST CHAR16  *Specific
  )
{
  EFI_STATUS                Status;
  EFI_HANDLE                *HandleBuffer;
  UINTN                     BufferSize;
  UINTN                     LoopVar;
  BOOLEAN                   Deleted;

  if (Specific == NULL) {
    return (EFI_INVALID_PARAMETER);
  }

  BufferSize    = 0;
  HandleBuffer  = NULL;
  Deleted       = FALSE;

  //
  // Look up all SimpleFileSystems in the platform
  //
  Status = gBS->LocateHandle(
    ByProtocol,
    &gEfiDevicePathProtocolGuid,
    NULL,
    &BufferSize,
    HandleBuffer);
  if (Status == EFI_BUFFER_TOO_SMALL) {
    HandleBuffer = AllocatePool(BufferSize);
    Status = gBS->LocateHandle(
      ByProtocol,
      &gEfiDevicePathProtocolGuid,
      NULL,
      &BufferSize,
      HandleBuffer);
  }
  ASSERT_EFI_ERROR(Status);
  ASSERT(HandleBuffer != NULL);

  //
  // Get the map name(s) for each one.
  //
  for ( LoopVar = 0
      ; LoopVar < BufferSize / sizeof(EFI_HANDLE)
      ; LoopVar ++
     ){
    if (PerformSingleMappingDelete(Specific,HandleBuffer[LoopVar]) == SHELL_SUCCESS) {
        Deleted = TRUE;
    }
  }

  //
  // Look up all BlockIo in the platform
  //
  Status = gBS->LocateHandle(
    ByProtocol,
    &gEfiBlockIoProtocolGuid,
    NULL,
    &BufferSize,
    HandleBuffer);
  if (Status == EFI_BUFFER_TOO_SMALL) {
    FreePool(HandleBuffer);
    HandleBuffer = AllocatePool(BufferSize);
    Status = gBS->LocateHandle(
      ByProtocol,
      &gEfiBlockIoProtocolGuid,
      NULL,
      &BufferSize,
      HandleBuffer);
  }
  ASSERT_EFI_ERROR(Status);

  //
  // Get the map name(s) for each one.
  //
  for ( LoopVar = 0
      ; LoopVar < BufferSize / sizeof(EFI_HANDLE)
      ; LoopVar ++
     ){
    //
    // Skip any that were already done...
    //
    if (gBS->OpenProtocol(
      HandleBuffer[LoopVar],
      &gEfiDevicePathProtocolGuid,
      NULL,
      gImageHandle,
      NULL,
      EFI_OPEN_PROTOCOL_TEST_PROTOCOL) == EFI_SUCCESS) {
        continue;
    }
    if (PerformSingleMappingDelete(Specific,HandleBuffer[LoopVar]) == SHELL_SUCCESS) {
        Deleted = TRUE;
    }
  }
  FreePool(HandleBuffer);
  if (!Deleted) {
    return (EFI_NOT_FOUND);
  }
  return (EFI_SUCCESS);
}

/**
  function to add a mapping from mapping.

  This function will get the device path associated with the mapping and call SetMap.

  @param[in] Map         The Map to add a mapping for
  @param[in] SName       The name of the new mapping

  @retval SHELL_SUCCESS             the mapping was added
  @retval SHELL_INVALID_PARAMETER   the device path for Map could not be retrieved.
  @return                           Shell version of a return value from EfiShellProtocol->SetMap

**/
SHELL_STATUS
EFIAPI
AddMappingFromMapping(
  IN CONST CHAR16     *Map,
  IN CONST CHAR16     *SName
  )
{
  CONST EFI_DEVICE_PATH_PROTOCOL  *DevPath;
  EFI_STATUS                      Status;

  if (StrStr(SName, L"*") != NULL
    ||StrStr(SName, L"?") != NULL
    ||StrStr(SName, L"[") != NULL
    ||StrStr(SName, L"]") != NULL) {
    return (SHELL_INVALID_PARAMETER);
  }

  DevPath = gEfiShellProtocol->GetDevicePathFromMap(Map);
  if (DevPath == NULL) {
    return (SHELL_INVALID_PARAMETER);
  }

  Status = gEfiShellProtocol->SetMap(DevPath, SName);
  if (EFI_ERROR(Status)) {
    return (SHELL_DEVICE_ERROR);
  }
  return (SHELL_SUCCESS);
}

/**
  function to determine if a string has only numbers and letters

  This is useful for such things as Map names which can only be letters and numbers

  @param[in] String       pointer to the string to analyze

  @retval TRUE            String has only numbers and letters
  @retval FALSE           String has at least one other character.
**/
BOOLEAN
EFIAPI
IsNumberLetterOnly(
  IN CONST CHAR16 *String
  )
{
  while(String != NULL && *String != CHAR_NULL) {
    if (! ((*String >= L'a' && *String <= L'z') ||
           (*String >= L'A' && *String <= L'Z') ||
           (*String >= L'0' && *String <= L'9'))
        ){
      return (FALSE);
    }
    String++;
  }
  return (TRUE);
}

/**
  function to add a mapping from an EFI_HANDLE.

  This function will get the device path associated with the Handle and call SetMap.

  @param[in] Handle       The handle to add a mapping for
  @param[in] SName        The name of the new mapping

  @retval SHELL_SUCCESS           the mapping was added
  @retval SHELL_INVALID_PARAMETER SName was not valid for a map name.
  @return                         Shell version of a return value from either
                                  gBS->OpenProtocol or EfiShellProtocol->SetMap

**/
SHELL_STATUS
EFIAPI
AddMappingFromHandle(
  IN CONST EFI_HANDLE Handle,
  IN CONST CHAR16     *SName
  )
{
  EFI_DEVICE_PATH_PROTOCOL  *DevPath;
  EFI_STATUS                Status;

  if (!IsNumberLetterOnly(SName)) {
    return (SHELL_INVALID_PARAMETER);
  }

  Status = gBS->OpenProtocol(
    Handle,
    &gEfiDevicePathProtocolGuid,
    (VOID**)&DevPath,
    gImageHandle,
    NULL,
    EFI_OPEN_PROTOCOL_GET_PROTOCOL
   );
  if (EFI_ERROR(Status)) {
    return (SHELL_DEVICE_ERROR);
  }
  Status = gEfiShellProtocol->SetMap(DevPath, SName);
  if (EFI_ERROR(Status)) {
    return (SHELL_DEVICE_ERROR);
  }
  return (SHELL_SUCCESS);
}

STATIC CONST SHELL_PARAM_ITEM MapParamList[] = {
  {L"-d", TypeValue},
  {L"-r", TypeFlag},
  {L"-v", TypeFlag},
  {L"-c", TypeFlag},
  {L"-f", TypeFlag},
  {L"-u", TypeFlag},
  {L"-t", TypeValue},
  {L"-sfo", TypeValue},
  {NULL, TypeMax}
  };

/**
  Function for 'map' command.

  @param[in] ImageHandle  Handle to the Image (NULL if Internal).
  @param[in] SystemTable  Pointer to the System Table (NULL if Internal).
**/
SHELL_STATUS
EFIAPI
ShellCommandRunMap (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
{
  EFI_STATUS    Status;
  LIST_ENTRY    *Package;
  CHAR16        *ProblemParam;
  CONST CHAR16  *SName;
  CONST CHAR16  *Mapping;
  EFI_HANDLE    MappingAsHandle;
  CONST EFI_DEVICE_PATH_PROTOCOL *DevPath;
  SHELL_STATUS  ShellStatus;
  BOOLEAN       SfoMode;
  BOOLEAN       ConstMode;
  BOOLEAN       NormlMode;
  CONST CHAR16  *Param1;
  CONST CHAR16  *TypeString;

  ProblemParam  = NULL;
  Mapping       = NULL;
  SName         = NULL;
  DevPath       = NULL;
  ShellStatus   = SHELL_SUCCESS;
  MappingAsHandle = NULL;

  //
  // initialize the shell lib (we must be in non-auto-init...)
  //
  Status = ShellInitialize();
  ASSERT_EFI_ERROR(Status);

  Status = CommandInit();
  ASSERT_EFI_ERROR(Status);

  //
  // parse the command line
  //
  Status = ShellCommandLineParse (MapParamList, &Package, &ProblemParam, TRUE);
  if (EFI_ERROR(Status)) {
    if (Status == EFI_VOLUME_CORRUPTED && ProblemParam != NULL) {
      ShellPrintHiiEx(-1, -1, NULL, STRING_TOKEN (STR_GEN_PROBLEM), gShellLevel2HiiHandle, ProblemParam);
      FreePool(ProblemParam);
      ShellStatus = SHELL_INVALID_PARAMETER;
    } else {
      ASSERT(FALSE);
    }
  } else {
    //
    // check for "-?"
    //
    SfoMode   = ShellCommandLineGetFlag(Package, L"-sfo");
    ConstMode = ShellCommandLineGetFlag(Package, L"-c");
    NormlMode = ShellCommandLineGetFlag(Package, L"-f");
    if (ShellCommandLineGetFlag(Package, L"-?")) {
      ASSERT(FALSE);
    } else if (ShellCommandLineGetRawValue(Package, 3) != NULL) {
      ShellPrintHiiEx(-1, -1, NULL, STRING_TOKEN (STR_GEN_TOO_MANY), gShellLevel2HiiHandle);
      ShellStatus = SHELL_INVALID_PARAMETER;
    } else {
      //
      // Deleting a map name...
      //
      if (ShellCommandLineGetFlag(Package, L"-d")) {
        if ( ShellCommandLineGetFlag(Package, L"-r")
          || ShellCommandLineGetFlag(Package, L"-v")
          || ConstMode
          || NormlMode
          || ShellCommandLineGetFlag(Package, L"-u")
          || ShellCommandLineGetFlag(Package, L"-t")
         ){
          ShellPrintHiiEx(-1, -1, NULL, STRING_TOKEN (STR_GEN_PARAM_CON), gShellLevel2HiiHandle);
          ShellStatus = SHELL_INVALID_PARAMETER;
        } else {
          SName = ShellCommandLineGetValue(Package, L"-d");
          if (SName != NULL) {
            Status = PerformMappingDelete(SName);
            if (EFI_ERROR(Status)) {
              switch (Status) {
                case EFI_ACCESS_DENIED:
                  ShellPrintHiiEx(-1, -1, NULL, STRING_TOKEN (STR_GEN_ERR_AD), gShellLevel2HiiHandle);
                  ShellStatus = SHELL_ACCESS_DENIED;
                  break;
                case EFI_NOT_FOUND:
                  ShellPrintHiiEx(-1, -1, NULL, STRING_TOKEN (STR_MAP_NF), gShellLevel2HiiHandle, SName);
                  ShellStatus = SHELL_INVALID_PARAMETER;
                  break;
                default:
                  ShellPrintHiiEx(-1, -1, NULL, STRING_TOKEN (STR_GEN_ERR_UK), gShellLevel2HiiHandle, Status);
                  ShellStatus = SHELL_UNSUPPORTED;
              }
            }
          } else {
            ShellPrintHiiEx(-1, -1, NULL, STRING_TOKEN (STR_GEN_TOO_FEW), gShellLevel2HiiHandle);
            ShellStatus = SHELL_INVALID_PARAMETER;
          }
        }
      } else if ( ShellCommandLineGetFlag(Package, L"-r")
//               || ShellCommandLineGetFlag(Package, L"-v")
               || ConstMode
               || NormlMode
               || ShellCommandLineGetFlag(Package, L"-u")
               || ShellCommandLineGetFlag(Package, L"-t")
              ){
        if ( ShellCommandLineGetFlag(Package, L"-r")) {
          //
          // Do the reset
          //
          Status = ShellCommandCreateInitialMappingsAndPaths();
          if (EFI_ERROR(Status)) {
            ShellPrintHiiEx(-1, -1, NULL, STRING_TOKEN (STR_GEN_ERR_UK), gShellLevel2HiiHandle, Status);
            ShellStatus = SHELL_UNSUPPORTED;
          }
        }
        if ( ShellStatus == SHELL_SUCCESS && ShellCommandLineGetFlag(Package, L"-u")) {
          //
          // Do the Update
          //
          Status = UpdateMapping();
          if (EFI_ERROR(Status)) {
            ShellPrintHiiEx(-1, -1, NULL, STRING_TOKEN (STR_GEN_ERR_UK), gShellLevel2HiiHandle, Status);
            ShellStatus = SHELL_UNSUPPORTED;
          }
        }
        if (ShellStatus == SHELL_SUCCESS) {
          Param1 = ShellCommandLineGetRawValue(Package, 1);
          TypeString = ShellCommandLineGetValue(Package, L"-t");
          if (!ConstMode
            &&!NormlMode
            &&TypeString  == NULL
            ) {
            //
            // now do the display...
            //
            ShellStatus = PerformMappingDisplay(
              ShellCommandLineGetFlag(Package, L"-v"),
              TRUE,
              TRUE,
              NULL,
              SfoMode,
              Param1,
              TRUE
             );
          } else {
            //
            // now do the display...
            //
            ShellStatus = PerformMappingDisplay2(
              ShellCommandLineGetFlag(Package, L"-v"),
              ConstMode,
              NormlMode,
              TypeString,
              SfoMode,
              Param1
             );
          }
        }
      } else {
        //
        // adding or displaying (there were no flags)
        //
        SName = ShellCommandLineGetRawValue(Package, 1);
        Mapping = ShellCommandLineGetRawValue(Package, 2);
        if ( SName == NULL
          && Mapping == NULL
         ){
          //
          // display only since no flags
          //
          ShellStatus = PerformMappingDisplay(
            ShellCommandLineGetFlag(Package, L"-v"),
            TRUE,
            TRUE,
            NULL,
            SfoMode,
            NULL,
            TRUE
           );
        } else if ( SName == NULL
          || Mapping == NULL
         ){
            //
            // Display only the one specified
            //
          ShellStatus = PerformMappingDisplay(
            FALSE,
            FALSE,
            FALSE,
            NULL,
            SfoMode,
            SName, // note the variable here...
            TRUE
           );
        } else {
          if (ShellIsHexOrDecimalNumber(Mapping, TRUE, FALSE)) {
            MappingAsHandle = ConvertHandleIndexToHandle(StrHexToUintn(Mapping));
          } else {
            MappingAsHandle = NULL;
          }
          if (MappingAsHandle == NULL && Mapping[StrLen(Mapping)-1] != L':') {
            ShellPrintHiiEx(-1, -1, NULL, STRING_TOKEN (STR_GEN_PROBLEM), gShellLevel2HiiHandle, Mapping);
            ShellStatus = SHELL_INVALID_PARAMETER;
          } else {
            if (MappingAsHandle != NULL) {
              ShellStatus = AddMappingFromHandle(MappingAsHandle, SName);
            } else {
              ShellStatus = AddMappingFromMapping(Mapping, SName);
            }
            if (ShellStatus != SHELL_SUCCESS) {
              switch (ShellStatus) {
                case SHELL_ACCESS_DENIED:
                  ShellPrintHiiEx(-1, -1, NULL, STRING_TOKEN (STR_GEN_ERR_AD), gShellLevel2HiiHandle);
                  break;
                case SHELL_INVALID_PARAMETER:
                  ShellPrintHiiEx(-1, -1, NULL, STRING_TOKEN (STR_GEN_PARAM_INV), gShellLevel2HiiHandle);
                  break;
                default:
                  ShellPrintHiiEx(-1, -1, NULL, STRING_TOKEN (STR_GEN_ERR_UK), gShellLevel2HiiHandle, ShellStatus|MAX_BIT);
              }
            } else {
              //
              // now do the display...
              //
              ShellStatus = PerformMappingDisplay(
                FALSE,
                FALSE,
                FALSE,
                NULL,
                SfoMode,
                SName,
                TRUE
               );
            } // we were sucessful so do an output
          } // got a valid map target
        } // got 2 variables
      } // we are adding a mapping
    } // got valid parameters
  }

  //
  // free the command line package
  //
  ShellCommandLineFreeVarList (Package);

  return (ShellStatus);
}

