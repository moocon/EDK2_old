/*++

Copyright (c) 2006, Intel Corporation                                                         
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution.  The full text of the license may be found at        
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module Name:

  BdsMisc.c

Abstract:

  Misc BDS library function

--*/

extern UINT16 gPlatformBootTimeOutDefault;

UINT16
BdsLibGetTimeout (
  VOID
  )
/*++

Routine Description:
  
  Return the default value for system Timeout variable.

Arguments:

  None

Returns:
  
  Timeout value.

--*/
{
  UINT16      Timeout;
  UINTN       Size;
  EFI_STATUS  Status;

  //
  // Return Timeout variable or 0xffff if no valid
  // Timeout variable exists.
  //
  Size    = sizeof (UINT16);
  Status  = gRT->GetVariable (L"Timeout", &gEfiGlobalVariableGuid, NULL, &Size, &Timeout);
  if (!EFI_ERROR (Status)) {
    return Timeout;
  }
  //
  // To make the current EFI Automatic-Test activity possible, just add
  // following code to make AutoBoot enabled when this variable is not
  // present.
  // This code should be removed later.
  //
  Timeout = gPlatformBootTimeOutDefault;

  //
  // Notes: Platform should set default variable if non exists on all error cases!!!
  //
  Status = gRT->SetVariable (
                  L"Timeout",
                  &gEfiGlobalVariableGuid,
                  EFI_VARIABLE_BOOTSERVICE_ACCESS | EFI_VARIABLE_RUNTIME_ACCESS | EFI_VARIABLE_NON_VOLATILE,
                  sizeof (UINT16),
                  &Timeout
                  );
  return Timeout;
}

VOID
BdsLibLoadDrivers (
  IN LIST_ENTRY               *BdsDriverLists
  )
/*++

Routine Description:
  
  The function will go through the driver optoin link list, load and start
  every driver the driver optoin device path point to.

Arguments:

  BdsDriverLists   - The header of the current driver option link list

Returns:
  
  None

--*/
{
  EFI_STATUS                Status;
  LIST_ENTRY                *Link;
  BDS_COMMON_OPTION         *Option;
  EFI_HANDLE                ImageHandle;
  EFI_LOADED_IMAGE_PROTOCOL *ImageInfo;
  UINTN                     ExitDataSize;
  CHAR16                    *ExitData;
  BOOLEAN                   ReconnectAll;

  ReconnectAll = FALSE;

  //
  // Process the driver option
  //
  for (Link = BdsDriverLists->ForwardLink; Link != BdsDriverLists; Link = Link->ForwardLink) {
    Option = CR (Link, BDS_COMMON_OPTION, Link, BDS_LOAD_OPTION_SIGNATURE);
    //
    // If a load option is not marked as LOAD_OPTION_ACTIVE,
    // the boot manager will not automatically load the option.
    //
    if (!IS_LOAD_OPTION_TYPE (Option->Attribute, LOAD_OPTION_ACTIVE)) {
      continue;
    }
    //
    // If a driver load option is marked as LOAD_OPTION_FORCE_RECONNECT,
    // then all of the EFI drivers in the system will be disconnected and
    // reconnected after the last driver load option is processed.
    //
    if (IS_LOAD_OPTION_TYPE (Option->Attribute, LOAD_OPTION_FORCE_RECONNECT)) {
      ReconnectAll = TRUE;
    }
    //
    // Make sure the driver path is connected.
    //
    BdsLibConnectDevicePath (Option->DevicePath);

    //
    // Load and start the image that Driver#### describes
    //
    Status = gBS->LoadImage (
                    FALSE,
                    mBdsImageHandle,
                    Option->DevicePath,
                    NULL,
                    0,
                    &ImageHandle
                    );

    if (!EFI_ERROR (Status)) {
      gBS->HandleProtocol (ImageHandle, &gEfiLoadedImageProtocolGuid, &ImageInfo);

      //
      // Verify whether this image is a driver, if not,
      // exit it and continue to parse next load option
      //
      if (ImageInfo->ImageCodeType != EfiBootServicesCode && ImageInfo->ImageCodeType != EfiRuntimeServicesCode) {
        gBS->Exit (ImageHandle, EFI_INVALID_PARAMETER, 0, NULL);
        continue;
      }

      if (Option->LoadOptionsSize != 0) {
        ImageInfo->LoadOptionsSize  = Option->LoadOptionsSize;
        ImageInfo->LoadOptions      = Option->LoadOptions;
      }
      //
      // Before calling the image, enable the Watchdog Timer for
      // the 5 Minute period
      //
      gBS->SetWatchdogTimer (5 * 60, 0x0000, 0x00, NULL);

      Status = gBS->StartImage (ImageHandle, &ExitDataSize, &ExitData);
      DEBUG ((EFI_D_INFO | EFI_D_LOAD, "Driver Return Status = %r\n", Status));

      //
      // Clear the Watchdog Timer after the image returns
      //
      gBS->SetWatchdogTimer (0x0000, 0x0000, 0x0000, NULL);
    }
  }
  //
  // Process the LOAD_OPTION_FORCE_RECONNECT driver option
  //
  if (ReconnectAll) {
    BdsLibDisconnectAllEfi ();
    BdsLibConnectAll ();
  }

}

EFI_STATUS
BdsLibRegisterNewOption (
  IN  LIST_ENTRY                     *BdsOptionList,
  IN  EFI_DEVICE_PATH_PROTOCOL       *DevicePath,
  IN  CHAR16                         *String,
  IN  CHAR16                         *VariableName
  )
/*++

Routine Description:
  
  This function will register the new boot#### or driver#### option base on
  the VariableName. The new registered boot#### or driver#### will be linked
  to BdsOptionList and also update to the VariableName. After the boot#### or
  driver#### updated, the BootOrder or DriverOrder will also be updated.

Arguments:

  BdsOptionList    - The header of the boot#### or driver#### link list
  
  DevicePath       - The device path which the boot####
                     or driver#### option present
                     
  String           - The description of the boot#### or driver####
  
  VariableName     - Indicate if the boot#### or driver#### option

Returns:
  
  EFI_SUCCESS      - The boot#### or driver#### have been success registered
  
  EFI_STATUS       - Return the status of gRT->SetVariable ().

--*/
{
  EFI_STATUS                Status;
  UINTN                     Index;
  UINT16                    MaxOptionNumber;
  UINT16                    RegisterOptionNumber;
  UINT16                    *TempOptionPtr;
  UINTN                     TempOptionSize;
  UINT16                    *OptionOrderPtr;
  VOID                      *OptionPtr;
  UINTN                     OptionSize;
  UINT8                     *TempPtr;
  EFI_DEVICE_PATH_PROTOCOL  *OptionDevicePath;
  CHAR16                    *Description;
  CHAR16                    OptionName[10];
  BOOLEAN                   UpdateBootDevicePath;

  OptionPtr             = NULL;
  OptionSize            = 0;
  TempPtr               = NULL;
  OptionDevicePath      = NULL;
  Description           = NULL;
  MaxOptionNumber       = 0;
  OptionOrderPtr        = NULL;
  UpdateBootDevicePath  = FALSE;
  ZeroMem (OptionName, sizeof (OptionName));

  TempOptionSize = 0;
  TempOptionPtr = BdsLibGetVariableAndSize (
                    VariableName,
                    &gEfiGlobalVariableGuid,
                    &TempOptionSize
                    );
  //
  // Compare with current option variable
  //
  for (Index = 0; Index < TempOptionSize / sizeof (UINT16); Index++) {
    //
    // Got the max option#### number
    //
    if (MaxOptionNumber < TempOptionPtr[Index]) {
      MaxOptionNumber = TempOptionPtr[Index];
    }

    if (*VariableName == 'B') {
      UnicodeSPrint (OptionName, sizeof (OptionName), L"Boot%04x", TempOptionPtr[Index]);
    } else {
      UnicodeSPrint (OptionName, sizeof (OptionName), L"Driver%04x", TempOptionPtr[Index]);
    }

    OptionPtr = BdsLibGetVariableAndSize (
                  OptionName,
                  &gEfiGlobalVariableGuid,
                  &OptionSize
                  );
    TempPtr = OptionPtr;
    TempPtr += sizeof (UINT32) + sizeof (UINT16);
    Description = (CHAR16 *) TempPtr;
    TempPtr += StrSize ((CHAR16 *) TempPtr);
    OptionDevicePath = (EFI_DEVICE_PATH_PROTOCOL *) TempPtr;

    //
    // Notes: the description may will change base on the GetStringToken
    //
    if (CompareMem (Description, String, StrSize (Description)) == 0) {
      if (CompareMem (OptionDevicePath, DevicePath, GetDevicePathSize (OptionDevicePath)) == 0) {
        //
        // Got the option, so just return
        //
        gBS->FreePool (OptionPtr);
        gBS->FreePool (TempOptionPtr);
        return EFI_SUCCESS;
      } else {
        //
        // Boot device path changed, need update.
        //
        UpdateBootDevicePath = TRUE;
        break;
      }
    }

    gBS->FreePool (OptionPtr);
  }

  OptionSize          = sizeof (UINT32) + sizeof (UINT16) + StrSize (String) + GetDevicePathSize (DevicePath);
  OptionPtr           = AllocateZeroPool (OptionSize);
  TempPtr             = OptionPtr;
  *(UINT32 *) TempPtr = LOAD_OPTION_ACTIVE;
  TempPtr += sizeof (UINT32);
  *(UINT16 *) TempPtr = (UINT16) GetDevicePathSize (DevicePath);
  TempPtr += sizeof (UINT16);
  CopyMem (TempPtr, String, StrSize (String));
  TempPtr += StrSize (String);
  CopyMem (TempPtr, DevicePath, GetDevicePathSize (DevicePath));

  if (UpdateBootDevicePath) {
    //
    // The number in option#### to be updated
    //
    RegisterOptionNumber = TempOptionPtr[Index];
  } else {
    //
    // The new option#### number
    //
    RegisterOptionNumber = MaxOptionNumber + 1;
  }

  if (*VariableName == 'B') {
    UnicodeSPrint (OptionName, sizeof (OptionName), L"Boot%04x", RegisterOptionNumber);
  } else {
    UnicodeSPrint (OptionName, sizeof (OptionName), L"Driver%04x", RegisterOptionNumber);
  }

  Status = gRT->SetVariable (
                  OptionName,
                  &gEfiGlobalVariableGuid,
                  EFI_VARIABLE_BOOTSERVICE_ACCESS | EFI_VARIABLE_RUNTIME_ACCESS | EFI_VARIABLE_NON_VOLATILE,
                  OptionSize,
                  OptionPtr
                  );
  if (EFI_ERROR (Status) || UpdateBootDevicePath) {
    gBS->FreePool (OptionPtr);
    gBS->FreePool (TempOptionPtr);
    return Status;
  }

  gBS->FreePool (OptionPtr);

  //
  // Update the option order variable
  //
  OptionOrderPtr = AllocateZeroPool ((Index + 1) * sizeof (UINT16));
  CopyMem (OptionOrderPtr, TempOptionPtr, Index * sizeof (UINT16));
  OptionOrderPtr[Index] = RegisterOptionNumber;
  Status = gRT->SetVariable (
                  VariableName,
                  &gEfiGlobalVariableGuid,
                  EFI_VARIABLE_BOOTSERVICE_ACCESS | EFI_VARIABLE_RUNTIME_ACCESS | EFI_VARIABLE_NON_VOLATILE,
                  (Index + 1) * sizeof (UINT16),
                  OptionOrderPtr
                  );
  if (EFI_ERROR (Status)) {
    gBS->FreePool (TempOptionPtr);
    gBS->FreePool (OptionOrderPtr);
    return Status;
  }

  gBS->FreePool (TempOptionPtr);
  gBS->FreePool (OptionOrderPtr);

  return EFI_SUCCESS;
}

BDS_COMMON_OPTION *
BdsLibVariableToOption (
  IN OUT LIST_ENTRY               *BdsCommonOptionList,
  IN  CHAR16                      *VariableName
  )
/*++

Routine Description:

  Build the boot#### or driver#### option from the VariableName, the 
  build boot#### or driver#### will also be linked to BdsCommonOptionList
  
Arguments:

  BdsCommonOptionList - The header of the boot#### or driver#### option link list

  VariableName - EFI Variable name indicate if it is boot#### or driver####

Returns:

  BDS_COMMON_OPTION    - Get the option just been created

  NULL                 - Failed to get the new option

--*/
{
  UINT32                    Attribute;
  UINT16                    FilePathSize;
  UINT8                     *Variable;
  UINT8                     *TempPtr;
  UINTN                     VariableSize;
  EFI_DEVICE_PATH_PROTOCOL  *DevicePath;
  BDS_COMMON_OPTION         *Option;
  VOID                      *LoadOptions;
  UINT32                    LoadOptionsSize;
  CHAR16                    *Description;

  //
  // Read the variable. We will never free this data.
  //
  Variable = BdsLibGetVariableAndSize (
              VariableName,
              &gEfiGlobalVariableGuid,
              &VariableSize
              );
  if (Variable == NULL) {
    return NULL;
  }
  //
  // Notes: careful defined the variable of Boot#### or
  // Driver####, consider use some macro to abstract the code
  //
  //
  // Get the option attribute
  //
  TempPtr   = Variable;
  Attribute = *(UINT32 *) Variable;
  TempPtr += sizeof (UINT32);

  //
  // Get the option's device path size
  //
  FilePathSize = *(UINT16 *) TempPtr;
  TempPtr += sizeof (UINT16);

  //
  // Get the option's description string
  //
  Description = (CHAR16 *) TempPtr;

  //
  // Get the option's description string size
  //
  TempPtr += StrSize ((CHAR16 *) TempPtr);

  //
  // Get the option's device path
  //
  DevicePath = (EFI_DEVICE_PATH_PROTOCOL *) TempPtr;
  TempPtr += FilePathSize;

  LoadOptions     = TempPtr;
  LoadOptionsSize = (UINT32) (VariableSize - (UINTN) (TempPtr - Variable));

  //
  // The Console variables may have multiple device paths, so make
  // an Entry for each one.
  //
  Option = AllocateZeroPool (sizeof (BDS_COMMON_OPTION));
  if (Option == NULL) {
    return NULL;
  }

  Option->Signature   = BDS_LOAD_OPTION_SIGNATURE;
  Option->DevicePath  = AllocateZeroPool (GetDevicePathSize (DevicePath));
  CopyMem (Option->DevicePath, DevicePath, GetDevicePathSize (DevicePath));
  Option->Attribute   = Attribute;
  Option->Description = AllocateZeroPool (StrSize (Description));
  CopyMem (Option->Description, Description, StrSize (Description));
  Option->LoadOptions = AllocateZeroPool (LoadOptionsSize);
  CopyMem (Option->LoadOptions, LoadOptions, LoadOptionsSize);
  Option->LoadOptionsSize = LoadOptionsSize;

  //
  // Insert active entry to BdsDeviceList
  //
  if ((Option->Attribute & LOAD_OPTION_ACTIVE) == LOAD_OPTION_ACTIVE) {
    InsertTailList (BdsCommonOptionList, &Option->Link);
    gBS->FreePool (Variable);
    return Option;
  }

  gBS->FreePool (Variable);
  gBS->FreePool (Option);
  return NULL;

}

EFI_STATUS
BdsLibBuildOptionFromVar (
  IN  LIST_ENTRY                  *BdsCommonOptionList,
  IN  CHAR16                      *VariableName
  )
/*++

Routine Description:

  Process BootOrder, or DriverOrder variables, by calling
  BdsLibVariableToOption () for each UINT16 in the variables.

Arguments:

  BdsCommonOptionList - The header of the option list base on variable
                        VariableName

  VariableName - EFI Variable name indicate the BootOrder or DriverOrder

Returns:

  EFI_SUCCESS - Success create the boot option or driver option list

  EFI_OUT_OF_RESOURCES - Failed to get the boot option or driver option list

--*/
{
  UINT16            *OptionOrder;
  UINTN             OptionOrderSize;
  UINTN             Index;
  BDS_COMMON_OPTION *Option;
  CHAR16            OptionName[20];

  //
  // Zero Buffer in order to get all BOOT#### variables
  //
  ZeroMem (OptionName, sizeof (OptionName));

  //
  // Read the BootOrder, or DriverOrder variable.
  //
  OptionOrder = BdsLibGetVariableAndSize (
                  VariableName,
                  &gEfiGlobalVariableGuid,
                  &OptionOrderSize
                  );
  if (OptionOrder == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }

  for (Index = 0; Index < OptionOrderSize / sizeof (UINT16); Index++) {
    if (*VariableName == 'B') {
      UnicodeSPrint (OptionName, sizeof (OptionName), L"Boot%04x", OptionOrder[Index]);
    } else {
      UnicodeSPrint (OptionName, sizeof (OptionName), L"Driver%04x", OptionOrder[Index]);
    }
    Option              = BdsLibVariableToOption (BdsCommonOptionList, OptionName);
    Option->BootCurrent = OptionOrder[Index];

  }

  gBS->FreePool (OptionOrder);

  return EFI_SUCCESS;
}

EFI_STATUS
BdsLibGetBootMode (
  OUT EFI_BOOT_MODE       *BootMode
  )
/*++

Routine Description:

  Get boot mode by looking up configuration table and parsing HOB list

Arguments:

  BootMode - Boot mode from PEI handoff HOB.

Returns:

  EFI_SUCCESS - Successfully get boot mode
  
  EFI_NOT_FOUND - Can not find the current system boot mode

--*/
{
  EFI_HOB_HANDOFF_INFO_TABLE        *HobList;

  HobList = GetHobList ();
  ASSERT (HobList->Header.HobType == EFI_HOB_TYPE_HANDOFF);
  *BootMode = HobList->BootMode;

  return EFI_SUCCESS;
}

VOID *
BdsLibGetVariableAndSize (
  IN  CHAR16              *Name,
  IN  EFI_GUID            *VendorGuid,
  OUT UINTN               *VariableSize
  )
/*++

Routine Description:

  Read the EFI variable (VendorGuid/Name) and return a dynamically allocated
  buffer, and the size of the buffer. If failure return NULL.

Arguments:

  Name       - String part of EFI variable name

  VendorGuid - GUID part of EFI variable name

  VariableSize - Returns the size of the EFI variable that was read

Returns:

  Dynamically allocated memory that contains a copy of the EFI variable.
  Caller is responsible freeing the buffer.

  NULL - Variable was not read

--*/
{
  EFI_STATUS  Status;
  UINTN       BufferSize;
  VOID        *Buffer;

  Buffer = NULL;

  //
  // Pass in a zero size buffer to find the required buffer size.
  //
  BufferSize  = 0;
  Status      = gRT->GetVariable (Name, VendorGuid, NULL, &BufferSize, Buffer);
  if (Status == EFI_BUFFER_TOO_SMALL) {
    //
    // Allocate the buffer to return
    //
    Buffer = AllocateZeroPool (BufferSize);
    if (Buffer == NULL) {
      return NULL;
    }
    //
    // Read variable into the allocated buffer.
    //
    Status = gRT->GetVariable (Name, VendorGuid, NULL, &BufferSize, Buffer);
    if (EFI_ERROR (Status)) {
      BufferSize = 0;
    }
  }

  *VariableSize = BufferSize;
  return Buffer;
}

BOOLEAN
BdsLibMatchDevicePaths (
  IN  EFI_DEVICE_PATH_PROTOCOL  *Multi,
  IN  EFI_DEVICE_PATH_PROTOCOL  *Single
  )
/*++

Routine Description:

  Function compares a device path data structure to that of all the nodes of a
  second device path instance.

Arguments:

  Multi        - A pointer to a multi-instance device path data structure.

  Single       - A pointer to a single-instance device path data structure.

Returns:

  TRUE   - If the Single is contained within Multi
  
  FALSE  - The Single is not match within Multi
  

--*/
{
  EFI_DEVICE_PATH_PROTOCOL  *DevicePath;
  EFI_DEVICE_PATH_PROTOCOL  *DevicePathInst;
  UINTN                     Size;

  if (!Multi || !Single) {
    return FALSE;
  }

  DevicePath      = Multi;
  DevicePathInst  = GetNextDevicePathInstance (&DevicePath, &Size);
  Size -= sizeof (EFI_DEVICE_PATH_PROTOCOL);

  //
  // Search for the match of 'Single' in 'Multi'
  //
  while (DevicePathInst != NULL) {
    //
    // If the single device path is found in multiple device paths,
    // return success
    //
    if (Size == 0) {
      return FALSE;
    }

    if (CompareMem (Single, DevicePathInst, Size) == 0) {
      return TRUE;
    }

    gBS->FreePool (DevicePathInst);
    DevicePathInst = GetNextDevicePathInstance (&DevicePath, &Size);
    Size -= sizeof (EFI_DEVICE_PATH_PROTOCOL);
  }

  return FALSE;
}

EFI_STATUS
BdsLibOutputStrings (
  IN EFI_SIMPLE_TEXT_OUT_PROTOCOL   *ConOut,
  ...
  )
/*++

Routine Description:

  This function prints a series of strings.

Arguments:

  ConOut               - Pointer to EFI_SIMPLE_TEXT_OUT_PROTOCOL

  ...                  - A variable argument list containing series of strings,
                         the last string must be NULL.

Returns:

  EFI_SUCCESS          - Success print out the string using ConOut.
  
  EFI_STATUS           - Return the status of the ConOut->OutputString ().

--*/
{
  VA_LIST     args;
  EFI_STATUS  Status;
  CHAR16      *String;

  Status = EFI_SUCCESS;
  VA_START (args, ConOut);

  while (!EFI_ERROR (Status)) {
    //
    // If String is NULL, then it's the end of the list
    //
    String = VA_ARG (args, CHAR16 *);
    if (!String) {
      break;
    }

    Status = ConOut->OutputString (ConOut, String);

    if (EFI_ERROR (Status)) {
      break;
    }
  }

  return Status;
}
