/** @file
*
*  Copyright (c) 2011, ARM Limited. All rights reserved.
*  
*  This program and the accompanying materials                          
*  are licensed and made available under the terms and conditions of the BSD License         
*  which accompanies this distribution.  The full text of the license may be found at        
*  http://opensource.org/licenses/bsd-license.php                                            
*
*  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
*  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             
*
**/

#include "BdsInternal.h"

EFI_STATUS
GetEnvironmentVariable (
  IN     CONST CHAR16*   VariableName,
  IN     VOID*           DefaultValue,
  IN OUT UINTN*          Size,
  OUT    VOID**          Value
  )
{
  EFI_STATUS  Status;
  UINTN       VariableSize;

  // Try to get the variable size.
  *Value = NULL;
  VariableSize = 0;
  Status = gRT->GetVariable ((CHAR16 *) VariableName, &gEfiGlobalVariableGuid, NULL, &VariableSize, *Value);
  if (Status == EFI_NOT_FOUND) {
    if ((DefaultValue != NULL) && (Size != NULL) && (*Size != 0)) {
      // If the environment variable does not exist yet then set it with the default value
      Status = gRT->SetVariable (
                    (CHAR16*)VariableName,
                    &gEfiGlobalVariableGuid,
                    EFI_VARIABLE_NON_VOLATILE | EFI_VARIABLE_BOOTSERVICE_ACCESS | EFI_VARIABLE_RUNTIME_ACCESS,
                    *Size,
                    DefaultValue
                    );
      *Value = DefaultValue;
    } else {
      return EFI_NOT_FOUND;
    }
  } else if (Status == EFI_BUFFER_TOO_SMALL) {
    // Get the environment variable value
    *Value = AllocatePool (VariableSize);
    if (*Value == NULL) {
      return EFI_OUT_OF_RESOURCES;
    }

    Status = gRT->GetVariable ((CHAR16 *)VariableName, &gEfiGlobalVariableGuid, NULL, &VariableSize, *Value);
    if (EFI_ERROR (Status)) {
      FreePool(*Value);
      return EFI_INVALID_PARAMETER;
    }

    if (Size) {
      *Size = VariableSize;
    }
  } else {
    *Value = DefaultValue;
    return Status;
  }

  return EFI_SUCCESS;
}

EFI_STATUS
EditHIInputAscii (
  IN OUT CHAR8   *CmdLine,
  IN     UINTN   MaxCmdLine
  )
{
  UINTN           CmdLineIndex;
  UINTN           WaitIndex;
  CHAR8           Char;
  EFI_INPUT_KEY   Key;
  EFI_STATUS      Status;

  AsciiPrint (CmdLine);

  for (CmdLineIndex = AsciiStrLen(CmdLine); CmdLineIndex < MaxCmdLine; ) {
    Status = gBS->WaitForEvent (1, &gST->ConIn->WaitForKey, &WaitIndex);
    ASSERT_EFI_ERROR (Status);

    Status = gST->ConIn->ReadKeyStroke (gST->ConIn, &Key);
    ASSERT_EFI_ERROR (Status);

    // Unicode character is valid when Scancode is NUll
    if (Key.ScanCode == SCAN_NULL) {
      // Scan code is NUll, hence read Unicode character
      Char = (CHAR8)Key.UnicodeChar;
    } else {
      Char = CHAR_NULL;
    }

    if ((Char == CHAR_LINEFEED) || (Char == CHAR_CARRIAGE_RETURN) || (Char == 0x7f)) {
      CmdLine[CmdLineIndex] = '\0';
      AsciiPrint ("\n\r");

      return EFI_SUCCESS;
    } else if ((Char == '\b') || (Key.ScanCode == SCAN_LEFT) || (Key.ScanCode == SCAN_DELETE)){
      if (CmdLineIndex != 0) {
        CmdLineIndex--;
        AsciiPrint ("\b \b");
      }
    } else if ((Key.ScanCode == SCAN_ESC) || (Char == 0x1B) || (Char == 0x0)) {
      return EFI_INVALID_PARAMETER;
    } else {
      CmdLine[CmdLineIndex++] = Char;
      AsciiPrint ("%c", Char);
    }
  }

  return EFI_SUCCESS;
}

EFI_STATUS
GetHIInputAscii (
  IN OUT CHAR8   *CmdLine,
  IN     UINTN   MaxCmdLine
  )
{
  // For a new input just passed an empty string
  CmdLine[0] = '\0';

  return EditHIInputAscii (CmdLine,MaxCmdLine);
}

EFI_STATUS
GetHIInputInteger (
  OUT UINTN   *Integer
  )
{
  CHAR8  CmdLine[255];
  EFI_STATUS  Status;

  CmdLine[0] = '\0';
  Status = EditHIInputAscii (CmdLine,255);
  if (!EFI_ERROR(Status)) {
    *Integer = AsciiStrDecimalToUintn (CmdLine);
  }

  return Status;
}

EFI_STATUS
GetHIInputIP (
  OUT EFI_IP_ADDRESS   *Ip
  )
{
  CHAR8  CmdLine[255];
  CHAR8  *Str;
  EFI_STATUS  Status;

  CmdLine[0] = '\0';
  Status = EditHIInputAscii (CmdLine,255);
  if (!EFI_ERROR(Status)) {
    Str = CmdLine;
    Ip->v4.Addr[0] = (UINT8)AsciiStrDecimalToUintn (Str);

    Str = AsciiStrStr (Str, ".");
    if (Str == NULL) {
      return EFI_INVALID_PARAMETER;
    }

    Ip->v4.Addr[1] = (UINT8)AsciiStrDecimalToUintn (++Str);

    Str = AsciiStrStr (Str, ".");
    if (Str == NULL) {
      return EFI_INVALID_PARAMETER;
    }

    Ip->v4.Addr[2] = (UINT8)AsciiStrDecimalToUintn (++Str);

    Str = AsciiStrStr (Str, ".");
    if (Str == NULL) {
      return EFI_INVALID_PARAMETER;
    }

    Ip->v4.Addr[3] = (UINT8)AsciiStrDecimalToUintn (++Str);
  }

  return Status;
}

EFI_STATUS
GetHIInputBoolean (
  OUT BOOLEAN *Value
  )
{
  CHAR8       CmdBoolean[2];
  EFI_STATUS  Status;

  while(1) {
    Print (L"[y/n] ");
    Status = GetHIInputAscii (CmdBoolean,2);
    if (EFI_ERROR(Status)) {
      return Status;
    } else if ((CmdBoolean[0] == 'y') || (CmdBoolean[0] == 'Y')) {
      if (Value) *Value = TRUE;
      return EFI_SUCCESS;
    } else if ((CmdBoolean[0] == 'n') || (CmdBoolean[0] == 'N')) {
      if (Value) *Value = FALSE;
      return EFI_SUCCESS;
    }
  }
}

BOOLEAN
HasFilePathEfiExtension (
  IN CHAR16* FilePath
  )
{
  return (StrCmp (FilePath + (StrSize(FilePath)/sizeof(CHAR16)) - 5, L".efi") == 0);
}

// Return the last non end-type Device Path Node from a Device Path
EFI_DEVICE_PATH*
GetLastDevicePathNode (
  IN EFI_DEVICE_PATH*  DevicePath
  )
{
  EFI_DEVICE_PATH*     PrevDevicePathNode;

  PrevDevicePathNode = DevicePath;
  while (!IsDevicePathEndType (DevicePath)) {
    PrevDevicePathNode = DevicePath;
    DevicePath = NextDevicePathNode (DevicePath);
  }

  return PrevDevicePathNode;
}

EFI_STATUS
GenerateDeviceDescriptionName (
  IN  EFI_HANDLE  Handle,
  IN OUT CHAR16*  Description
  )
{
  EFI_STATUS                        Status;
  EFI_COMPONENT_NAME_PROTOCOL*      ComponentName2Protocol;
  EFI_DEVICE_PATH_TO_TEXT_PROTOCOL* DevicePathToTextProtocol;
  EFI_DEVICE_PATH_PROTOCOL*         DevicePathProtocol;
  CHAR16*                           DriverName;
  CHAR16*                           DevicePathTxt;
  EFI_DEVICE_PATH*                  DevicePathNode;

  ComponentName2Protocol = NULL;
  Status = gBS->HandleProtocol (Handle, &gEfiComponentName2ProtocolGuid, (VOID **)&ComponentName2Protocol);
  if (!EFI_ERROR(Status)) {
    //TODO: Fixme. we must find the best langague
    Status = ComponentName2Protocol->GetDriverName (ComponentName2Protocol,"en",&DriverName);
    if (!EFI_ERROR(Status)) {
      StrnCpy (Description,DriverName,BOOT_DEVICE_DESCRIPTION_MAX);
    }
  }

  if (EFI_ERROR(Status)) {
    // Use the lastest non null entry of the Device path as a description
    Status = gBS->HandleProtocol (Handle, &gEfiDevicePathProtocolGuid, (VOID **)&DevicePathProtocol);
    if (EFI_ERROR(Status)) {
      return Status;
    }

    // Convert the last non end-type Device Path Node in text for the description
    DevicePathNode = GetLastDevicePathNode (DevicePathProtocol);
    Status = gBS->LocateProtocol (&gEfiDevicePathToTextProtocolGuid, NULL, (VOID **)&DevicePathToTextProtocol);
    ASSERT_EFI_ERROR(Status);
    DevicePathTxt = DevicePathToTextProtocol->ConvertDevicePathToText(DevicePathNode,TRUE,TRUE);
    StrnCpy (Description, DevicePathTxt, BOOT_DEVICE_DESCRIPTION_MAX);
    FreePool (DevicePathTxt);
  }

  return EFI_SUCCESS;
}

EFI_STATUS
BdsStartBootOption (
  IN CHAR16* BootOption
  )
{
  EFI_STATUS          Status;
  EFI_LOAD_OPTION     EfiLoadOption;
  UINTN               EfiLoadOptionSize;
  BDS_LOAD_OPTION     *BdsLoadOption;

  Status = GetEnvironmentVariable (BootOption, NULL, &EfiLoadOptionSize, (VOID**)&EfiLoadOption);
  if (!EFI_ERROR(Status)) {
    Status = BootOptionParseLoadOption (EfiLoadOption, EfiLoadOptionSize, &BdsLoadOption);
    if (!EFI_ERROR(Status)) {
      Status = BootOptionStart (BdsLoadOption);
      FreePool (BdsLoadOption);
    }

    if (!EFI_ERROR(Status)) {
      Status = EFI_SUCCESS;
    } else {
      Status = EFI_NOT_STARTED;
    }
  } else {
    Status = EFI_NOT_FOUND;
  }
  return Status;
}
