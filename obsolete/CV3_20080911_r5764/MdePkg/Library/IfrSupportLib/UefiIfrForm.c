/** @file
Utility functions which helps in opcode creation, HII configuration string manipulations, 
pop up window creations, setup browser persistence data set and get.

Copyright (c) 2007- 2008, Intel Corporation
All rights reserved. This program and the accompanying materials
are licensed and made available under the terms and conditions of the BSD License
which accompanies this distribution.  The full text of the license may be found at
http://opensource.org/licenses/bsd-license.php

THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

#include "UefiIfrLibraryInternal.h"

STATIC CONST EFI_FORM_BROWSER2_PROTOCOL      *mFormBrowser2     = NULL;
STATIC CONST EFI_HII_CONFIG_ROUTING_PROTOCOL *mHiiConfigRouting = NULL;

/**
  This function locate FormBrowser2 protocols for later usage.

  @return Status the status to locate protocol.
**/
EFI_STATUS
LocateFormBrowser2Protocols (
  VOID
  )
{
  EFI_STATUS Status;
  //
  // Locate protocols for later usage
  //
  if (mFormBrowser2 == NULL) {
    Status = gBS->LocateProtocol (&gEfiFormBrowser2ProtocolGuid, NULL, (VOID **) &mFormBrowser2);
    if (EFI_ERROR (Status)) {
      return Status;
    }
  }
  
  if (mHiiConfigRouting == NULL) {
    Status = gBS->LocateProtocol (&gEfiHiiConfigRoutingProtocolGuid, NULL, (VOID **) &mHiiConfigRouting);
    if (EFI_ERROR (Status)) {
      return Status;
    }
  }

  return EFI_SUCCESS;
}

//
// Fake <ConfigHdr>
//
GLOBAL_REMOVE_IF_UNREFERENCED CONST UINT16 mFakeConfigHdr[] = L"GUID=00000000000000000000000000000000&NAME=0000&PATH=0";

/**
  Draw a dialog and return the selected key.

  @param  NumberOfLines          The number of lines for the dialog box
  @param  KeyValue               The EFI_KEY value returned if HotKey is TRUE..
  @param  String                 Pointer to the first string in the list
  @param  ...                    A series of (quantity == NumberOfLines - 1) text
                                 strings which will be used to construct the dialog
                                 box

  @retval EFI_SUCCESS            Displayed dialog and received user interaction
  @retval EFI_INVALID_PARAMETER  One of the parameters was invalid.
  @retval EFI_OUT_OF_RESOURCES   There is no enough available memory space.

**/
EFI_STATUS
EFIAPI
IfrLibCreatePopUp2 (
  IN  UINTN                       NumberOfLines,
  OUT EFI_INPUT_KEY               *KeyValue,
  IN  VA_LIST                     Marker
  )
{
  UINTN                         Index;
  UINTN                         Count;
  UINTN                         Start;
  UINTN                         Top;
  CHAR16                        *StringPtr;
  UINTN                         LeftColumn;
  UINTN                         RightColumn;
  UINTN                         TopRow;
  UINTN                         BottomRow;
  UINTN                         DimensionsWidth;
  UINTN                         DimensionsHeight;
  EFI_INPUT_KEY                 Key;
  UINTN                         LargestString;
  CHAR16                        *StackString;
  EFI_STATUS                    Status;
  UINTN                         StringLen;
  CHAR16                        *LineBuffer;
  CHAR16                        **StringArray;
  EFI_EVENT                     TimerEvent;
  EFI_EVENT                     WaitList[2];
  UINTN                         CurrentAttribute;
  EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL  *ConOut;
  CHAR16                        *String;

  String = VA_ARG (Marker, CHAR16 *);
  
  if ((KeyValue == NULL) || (String == NULL)) {
    return EFI_INVALID_PARAMETER;
  }

  TopRow      = 0;
  BottomRow   = 0;
  LeftColumn  = 0;
  RightColumn = 0;

  ConOut = gST->ConOut;
  ConOut->QueryMode (ConOut, ConOut->Mode->Mode, &RightColumn, &BottomRow);

  DimensionsWidth  = RightColumn - LeftColumn;
  DimensionsHeight = BottomRow - TopRow;

  CurrentAttribute = ConOut->Mode->Attribute;

  LineBuffer = AllocateZeroPool (DimensionsWidth * sizeof (CHAR16));
  if (LineBuffer == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }

  //
  // Determine the largest string in the dialog box
  // Notice we are starting with 1 since String is the first string
  //
  StringArray = AllocateZeroPool (NumberOfLines * sizeof (CHAR16 *));
  if (StringArray == NULL) {
    FreePool (LineBuffer);
    return EFI_OUT_OF_RESOURCES;
  }
  LargestString = StrLen (String);
  StringArray[0] = String;

  for (Index = 1; Index < NumberOfLines; Index++) {
    StackString = VA_ARG (Marker, CHAR16 *);

    if (StackString == NULL) {
      FreePool (LineBuffer);
      FreePool (StringArray);
      return EFI_INVALID_PARAMETER;
    }

    StringArray[Index] = StackString;
    StringLen = StrLen (StackString);
    if (StringLen > LargestString) {
      LargestString = StringLen;
    }
  }

  if ((LargestString + 2) > DimensionsWidth) {
    LargestString = DimensionsWidth - 2;
  }

  //
  // Subtract the PopUp width from total Columns, allow for one space extra on
  // each end plus a border.
  //
  Start     = (DimensionsWidth - LargestString - 2) / 2 + LeftColumn + 1;

  Top       = ((DimensionsHeight - NumberOfLines - 2) / 2) + TopRow - 1;

  //
  // Disable cursor
  //
  ConOut->EnableCursor (ConOut, FALSE);
  ConOut->SetAttribute (ConOut, EFI_LIGHTGRAY | EFI_BACKGROUND_BLUE);

  StringPtr = &LineBuffer[0];
  *StringPtr++ = BOXDRAW_DOWN_RIGHT;
  for (Index = 0; Index < LargestString; Index++) {
    *StringPtr++ = BOXDRAW_HORIZONTAL;
  }
  *StringPtr++ = BOXDRAW_DOWN_LEFT;
  *StringPtr = L'\0';

  ConOut->SetCursorPosition (ConOut, Start, Top);
  ConOut->OutputString (ConOut, LineBuffer);

  for (Index = 0; Index < NumberOfLines; Index++) {
    StringPtr = &LineBuffer[0];
    *StringPtr++ = BOXDRAW_VERTICAL;

    for (Count = 0; Count < LargestString; Count++) {
      StringPtr[Count] = L' ';
    }

    StringLen = StrLen (StringArray[Index]);
    if (StringLen > LargestString) {
      StringLen = LargestString;
    }
    CopyMem (
      StringPtr + ((LargestString - StringLen) / 2),
      StringArray[Index],
      StringLen * sizeof (CHAR16)
      );
    StringPtr += LargestString;

    *StringPtr++ = BOXDRAW_VERTICAL;
    *StringPtr = L'\0';

    ConOut->SetCursorPosition (ConOut, Start, Top + 1 + Index);
    ConOut->OutputString (ConOut, LineBuffer);
  }

  StringPtr = &LineBuffer[0];
  *StringPtr++ = BOXDRAW_UP_RIGHT;
  for (Index = 0; Index < LargestString; Index++) {
    *StringPtr++ = BOXDRAW_HORIZONTAL;
  }
  *StringPtr++ = BOXDRAW_UP_LEFT;
  *StringPtr = L'\0';

  ConOut->SetCursorPosition (ConOut, Start, Top + NumberOfLines + 1);
  ConOut->OutputString (ConOut, LineBuffer);

  do {
    Status = gBS->CreateEvent (EVT_TIMER, 0, NULL, NULL, &TimerEvent);

    //
    // Set a timer event of 1 second expiration
    //
    gBS->SetTimer (
          TimerEvent,
          TimerRelative,
          10000000
          );

    //
    // Wait for the keystroke event or the timer
    //
    WaitList[0] = gST->ConIn->WaitForKey;
    WaitList[1] = TimerEvent;
    Status      = gBS->WaitForEvent (2, WaitList, &Index);

    //
    // Check for the timer expiration
    //
    if (!EFI_ERROR (Status) && Index == 1) {
      Status = EFI_TIMEOUT;
    }

    gBS->CloseEvent (TimerEvent);
  } while (Status == EFI_TIMEOUT);

  Status = gST->ConIn->ReadKeyStroke (gST->ConIn, &Key);
  CopyMem (KeyValue, &Key, sizeof (EFI_INPUT_KEY));

  ConOut->SetAttribute (ConOut, CurrentAttribute);
  ConOut->EnableCursor (ConOut, TRUE);

  FreePool (LineBuffer);
  FreePool (StringArray);

  return Status;
}


/**
  Draw a dialog and return the selected key.

  @param  NumberOfLines          The number of lines for the dialog box
  @param  KeyValue               The EFI_KEY value returned if HotKey is TRUE..
  @param  String                 Pointer to the first string in the list
  @param  ...                    A series of (quantity == NumberOfLines - 1) text
                                 strings which will be used to construct the dialog
                                 box

  @retval EFI_SUCCESS            Displayed dialog and received user interaction
  @retval EFI_INVALID_PARAMETER  One of the parameters was invalid.

**/
EFI_STATUS
EFIAPI
IfrLibCreatePopUp (
  IN  UINTN                       NumberOfLines,
  OUT EFI_INPUT_KEY               *KeyValue,
  IN  CHAR16                      *String,
  ...
  )
{
  EFI_STATUS                      Status;
  VA_LIST                         Marker;

  VA_START (Marker, KeyValue);

  Status = IfrLibCreatePopUp2 (NumberOfLines, KeyValue, Marker);

  VA_END (Marker);

  return Status;
}

/**
  Swap bytes in the buffer. This is a internal function.

  @param  Buffer                 Binary buffer.
  @param  BufferSize             Size of the buffer in bytes.

  @return None.

**/
VOID
SwapBuffer (
  IN OUT UINT8     *Buffer,
  IN UINTN         BufferSize
  )
{
  UINTN  Index;
  UINT8  Temp;
  UINTN  SwapCount;

  SwapCount = BufferSize / 2;
  for (Index = 0; Index < SwapCount; Index++) {
    Temp = Buffer[Index];
    Buffer[Index] = Buffer[BufferSize - 1 - Index];
    Buffer[BufferSize - 1 - Index] = Temp;
  }
}

/**
  Converts the unicode character of the string from uppercase to lowercase.
  This is a internal function.

  @param Str     String to be converted

**/
VOID
EFIAPI
ToLower (
  IN OUT CHAR16    *Str
  )
{
  CHAR16      *Ptr;
  
  for (Ptr = Str; *Ptr != L'\0'; Ptr++) {
    if (*Ptr >= L'A' && *Ptr <= L'Z') {
      *Ptr = (CHAR16) (*Ptr - L'A' + L'a');
    }
  }
}


/**
  Converts binary buffer to Unicode string in reversed byte order from BufToHexString().

  @param  Str                    String for output
  @param  Buffer                 Binary buffer.
  @param  BufferSize             Size of the buffer in bytes.

  @retval EFI_SUCCESS            The function completed successfully.
  @retval EFI_OUT_OF_RESOURCES   There is no enough available memory space.

**/
EFI_STATUS
EFIAPI
BufferToHexString (
  IN OUT CHAR16    *Str,
  IN UINT8         *Buffer,
  IN UINTN         BufferSize
  )
{
  EFI_STATUS  Status;
  UINT8       *NewBuffer;
  UINTN       StrBufferLen;

  NewBuffer = AllocateCopyPool (BufferSize, Buffer);
  if (NewBuffer == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }
  SwapBuffer (NewBuffer, BufferSize);

  StrBufferLen = BufferSize * sizeof (CHAR16) + 1;
  Status = BufToHexString (Str, &StrBufferLen, NewBuffer, BufferSize);

  FreePool (NewBuffer);
  //
  // Convert the uppercase to lowercase since <HexAf> is defined in lowercase format.
  //
  ToLower (Str);

  return Status;
}


/**
  Converts Hex String to binary buffer in reversed byte order from HexStringToBuf().

  @param  Buffer                 Pointer to buffer that receives the data.
  @param  BufferSize             Length in bytes of the buffer to hold converted
                                 data. If routine return with EFI_SUCCESS,
                                 containing length of converted data. If routine
                                 return with EFI_BUFFER_TOO_SMALL, containg length
                                 of buffer desired.
  @param  Str                    String to be converted from.

  @retval EFI_SUCCESS            The function completed successfully.

**/
EFI_STATUS
EFIAPI
HexStringToBuffer (
  IN OUT UINT8         *Buffer,
  IN OUT UINTN         *BufferSize,
  IN CHAR16            *Str
  )
{
  EFI_STATUS  Status;
  UINTN       ConvertedStrLen;

  ConvertedStrLen = 0;
  Status = HexStringToBuf (Buffer, BufferSize, Str, &ConvertedStrLen);
  if (!EFI_ERROR (Status)) {
    SwapBuffer (Buffer, ConvertedStrLen);
  }

  return Status;
}

/**
  Convert binary representation Config string (e.g. "0041004200430044") to the
  original string (e.g. "ABCD"). Config string appears in <ConfigHdr> (i.e.
  "&NAME=<string>"), or Name/Value pair in <ConfigBody> (i.e. "label=<string>").

  @param UnicodeString  Original Unicode string.
  @param StrBufferLen   On input: Length in bytes of buffer to hold the Unicode string.
                                    Includes tailing '\0' character.
                                    On output:
                                      If return EFI_SUCCESS, containing length of Unicode string buffer.
                                      If return EFI_BUFFER_TOO_SMALL, containg length of string buffer desired.
  @param ConfigString   Binary representation of Unicode String, <string> := (<HexCh>4)+

  @retval EFI_SUCCESS          Routine success.
  @retval EFI_BUFFER_TOO_SMALL The string buffer is too small.

**/
EFI_STATUS
EFIAPI
ConfigStringToUnicode (
  IN OUT CHAR16                *UnicodeString,
  IN OUT UINTN                 *StrBufferLen,
  IN CHAR16                    *ConfigString
  )
{
  UINTN       Index;
  UINTN       Len;
  UINTN       BufferSize;
  CHAR16      BackupChar;

  Len = StrLen (ConfigString) / 4;
  BufferSize = (Len + 1) * sizeof (CHAR16);

  if (*StrBufferLen < BufferSize) {
    *StrBufferLen = BufferSize;
    return EFI_BUFFER_TOO_SMALL;
  }

  *StrBufferLen = BufferSize;

  for (Index = 0; Index < Len; Index++) {
    BackupChar = ConfigString[4];
    ConfigString[4] = L'\0';

    HexStringToBuf ((UINT8 *) UnicodeString, &BufferSize, ConfigString, NULL);

    ConfigString[4] = BackupChar;

    ConfigString += 4;
    UnicodeString += 1;
  }

  //
  // Add tailing '\0' character
  //
  *UnicodeString = L'\0';

  return EFI_SUCCESS;
}

/**
  Convert Unicode string to binary representation Config string, e.g.
  "ABCD" => "0041004200430044". Config string appears in <ConfigHdr> (i.e.
  "&NAME=<string>"), or Name/Value pair in <ConfigBody> (i.e. "label=<string>").

  @param ConfigString   Binary representation of Unicode String, <string> := (<HexCh>4)+
  @param  StrBufferLen  On input: Length in bytes of buffer to hold the Unicode string.
                                    Includes tailing '\0' character.
                                    On output:
                                      If return EFI_SUCCESS, containing length of Unicode string buffer.
                                      If return EFI_BUFFER_TOO_SMALL, containg length of string buffer desired.
  @param  UnicodeString  Original Unicode string.

  @retval EFI_SUCCESS           Routine success.
  @retval EFI_BUFFER_TOO_SMALL  The string buffer is too small.

**/
EFI_STATUS
EFIAPI
UnicodeToConfigString (
  IN OUT CHAR16                *ConfigString,
  IN OUT UINTN                 *StrBufferLen,
  IN CHAR16                    *UnicodeString
  )
{
  UINTN       Index;
  UINTN       Len;
  UINTN       BufferSize;
  CHAR16      *String;

  Len = StrLen (UnicodeString);
  BufferSize = (Len * 4 + 1) * sizeof (CHAR16);

  if (*StrBufferLen < BufferSize) {
    *StrBufferLen = BufferSize;
    return EFI_BUFFER_TOO_SMALL;
  }

  *StrBufferLen = BufferSize;
  String        = ConfigString;

  for (Index = 0; Index < Len; Index++) {
    BufToHexString (ConfigString, &BufferSize, (UINT8 *) UnicodeString, 2);

    ConfigString += 4;
    UnicodeString += 1;
  }

  //
  // Add tailing '\0' character
  //
  *ConfigString = L'\0';

  //
  // Convert the uppercase to lowercase since <HexAf> is defined in lowercase format.
  //
  ToLower (String);  
  return EFI_SUCCESS;
}

/**
  Construct <ConfigHdr> using routing information GUID/NAME/PATH.

  @param  ConfigHdr              Pointer to the ConfigHdr string.
  @param  StrBufferLen           On input: Length in bytes of buffer to hold the
                                 ConfigHdr string. Includes tailing '\0' character.
                                 On output: If return EFI_SUCCESS, containing
                                 length of ConfigHdr string buffer. If return
                                 EFI_BUFFER_TOO_SMALL, containg length of string
                                 buffer desired.
  @param  Guid                   Routing information: GUID.
  @param  Name                   Routing information: NAME.
  @param  DriverHandle           Driver handle which contains the routing
                                 information: PATH.

  @retval EFI_SUCCESS            Routine success.
  @retval EFI_BUFFER_TOO_SMALL   The ConfigHdr string buffer is too small.

**/
EFI_STATUS
EFIAPI
ConstructConfigHdr (
  IN OUT CHAR16                *ConfigHdr,
  IN OUT UINTN                 *StrBufferLen,
  IN EFI_GUID                  *Guid,
  IN CHAR16                    *Name, OPTIONAL
  IN EFI_HANDLE                *DriverHandle
  )
{
  EFI_STATUS                Status;
  UINTN                     NameStrLen;
  UINTN                     DevicePathSize;
  UINTN                     BufferSize;
  CHAR16                    *StrPtr;
  EFI_DEVICE_PATH_PROTOCOL  *DevicePath;

  if (Name == NULL) {
    //
    // There will be no "NAME" in <ConfigHdr> for  Name/Value storage
    //
    NameStrLen = 0;
  } else {
    //
    // For buffer storage
    //
    NameStrLen = StrLen (Name);
  }

  //
  // Retrieve DevicePath Protocol associated with this HiiPackageList
  //
  Status = gBS->HandleProtocol (
                  DriverHandle,
                  &gEfiDevicePathProtocolGuid,
                  (VOID **) &DevicePath
                  );
  if (EFI_ERROR (Status)) {
    return Status;
  }

  DevicePathSize = GetDevicePathSize (DevicePath);

  //
  // GUID=<HexCh>32&NAME=<Char>NameStrLen&PATH=<HexChar>DevicePathStrLen <NULL>
  // | 5  |   32   |  6  |  NameStrLen*4 |  6  |    DevicePathStrLen    | 1 |
  //
  BufferSize = (5 + 32 + 6 + NameStrLen * 4 + 6 + DevicePathSize * 2 + 1) * sizeof (CHAR16);
  if (*StrBufferLen < BufferSize) {
    *StrBufferLen = BufferSize;
    return EFI_BUFFER_TOO_SMALL;
  }

  *StrBufferLen = BufferSize;

  StrPtr = ConfigHdr;

  StrCpy (StrPtr, L"GUID=");
  StrPtr += 5;
  BufferToHexString (StrPtr, (UINT8 *) Guid, sizeof (EFI_GUID));
  StrPtr += 32;

  //
  // Convert name string, e.g. name "ABCD" => "&NAME=0041004200430044"
  //
  StrCpy (StrPtr, L"&NAME=");
  StrPtr += 6;
  if (Name != NULL) {
    BufferSize = (NameStrLen * 4 + 1) * sizeof (CHAR16);
    UnicodeToConfigString (StrPtr, &BufferSize, Name);
    StrPtr += (NameStrLen * 4);
  }

  StrCpy (StrPtr, L"&PATH=");
  StrPtr += 6;
  BufferToHexString (StrPtr, (UINT8 *) DevicePath, DevicePathSize);

  return EFI_SUCCESS;
}


/**
  Search BlockName "&OFFSET=Offset&WIDTH=Width" in a string.

  @param  String                 The string to be searched in.
  @param  Offset                 Offset in BlockName.
  @param  Width                  Width in BlockName.

  @retval TRUE                   Block name found.
  @retval FALSE                  Block name not found.

**/
BOOLEAN
EFIAPI
FindBlockName (
  IN OUT CHAR16                *String,
  UINTN                        Offset,
  UINTN                        Width
  )
{
  EFI_STATUS  Status;
  UINTN       Data;
  UINTN       BufferSize;
  UINTN       ConvertedStrLen;

  while ((String = StrStr (String, L"&OFFSET=")) != NULL) {
    //
    // Skip '&OFFSET='
    //
    String = String + 8;

    Data = 0;
    BufferSize = sizeof (UINTN);
    Status = HexStringToBuf ((UINT8 *) &Data, &BufferSize, String, &ConvertedStrLen);
    if (EFI_ERROR (Status)) {
      return FALSE;
    }
    String = String + ConvertedStrLen;

    if (Data != Offset) {
      continue;
    }

    if (StrnCmp (String, L"&WIDTH=", 7) != 0) {
      return FALSE;
    }
    String = String + 7;

    Data = 0;
    BufferSize = sizeof (UINTN);
    Status = HexStringToBuf ((UINT8 *) &Data, &BufferSize, String, &ConvertedStrLen);
    if (EFI_ERROR (Status)) {
      return FALSE;
    }
    if (Data == Width) {
      return TRUE;
    }

    String = String + ConvertedStrLen;
  }

  return FALSE;
}


/**
  This routine is invoked by ConfigAccess.Callback() to retrived uncommitted data from Form Browser.

  @param  VariableGuid           An optional field to indicate the target variable
                                 GUID name to use.
  @param  VariableName           An optional field to indicate the target
                                 human-readable variable name.
  @param  BufferSize             On input: Length in bytes of buffer to hold
                                 retrived data. On output: If return
                                 EFI_BUFFER_TOO_SMALL, containg length of buffer
                                 desired.
  @param  Buffer                 Buffer to hold retrived data.

  @retval EFI_SUCCESS            Routine success.
  @retval EFI_BUFFER_TOO_SMALL   The intput buffer is too small.
  @retval EFI_OUT_OF_RESOURCES   There is no enough available memory space.

**/
EFI_STATUS
EFIAPI
GetBrowserData (
  EFI_GUID                   *VariableGuid, OPTIONAL
  CHAR16                     *VariableName, OPTIONAL
  UINTN                      *BufferSize,
  UINT8                      *Buffer
  )
{
  EFI_STATUS                      Status;
  CONST CHAR16                    *ConfigHdr;
  CHAR16                          *ConfigResp;
  CHAR16                          *StringPtr;
  UINTN                           HeaderLen;
  UINTN                           BufferLen;
  CHAR16                          *Progress;

  //
  // Locate protocols for use
  //
  Status = LocateFormBrowser2Protocols ();
  if (EFI_ERROR (Status)) {
    return Status;
  }

  //
  // Retrive formset storage data from Form Browser
  //
  ConfigHdr = mFakeConfigHdr;
  HeaderLen = StrLen (ConfigHdr);
  
  //
  // First try allocate 0x4000 buffer for the formet storage data.
  //
  BufferLen = 0x4000;
  ConfigResp = AllocateZeroPool (BufferLen + HeaderLen);
  if (ConfigResp == NULL) {
    BufferLen = 0;
  }

  StringPtr = ConfigResp + HeaderLen;
  *StringPtr = L'&';
  StringPtr++;

  Status = mFormBrowser2->BrowserCallback (
                           mFormBrowser2,
                           &BufferLen,
                           StringPtr,
                           TRUE,
                           VariableGuid,
                           VariableName
                           );
  if (Status == EFI_BUFFER_TOO_SMALL) {
    if (ConfigResp != NULL) {
      FreePool (ConfigResp);
    }

    ConfigResp = AllocateZeroPool (BufferLen + HeaderLen);
    if (ConfigResp == NULL) {
      return EFI_OUT_OF_RESOURCES;
    }

    StringPtr = ConfigResp + HeaderLen;
    *StringPtr = L'&';
    StringPtr++;

    Status = mFormBrowser2->BrowserCallback (
                             mFormBrowser2,
                             &BufferLen,
                             StringPtr,
                             TRUE,
                             VariableGuid,
                             VariableName
                             );
  }
  if (EFI_ERROR (Status)) {
    FreePool (ConfigResp);
    return Status;
  }
  CopyMem (ConfigResp, ConfigHdr, HeaderLen * sizeof (UINT16));

  //
  // Convert <ConfigResp> to buffer data
  //
  Status = mHiiConfigRouting->ConfigToBlock (
                               mHiiConfigRouting,
                               ConfigResp,
                               Buffer,
                               BufferSize,
                               &Progress
                               );
  FreePool (ConfigResp);

  return Status;
}


/**
  This routine is invoked by ConfigAccess.Callback() to update uncommitted data of Form Browser.

  @param  VariableGuid           An optional field to indicate the target variable
                                 GUID name to use.
  @param  VariableName           An optional field to indicate the target
                                 human-readable variable name.
  @param  BufferSize             Length in bytes of buffer to hold retrived data.
  @param  Buffer                 Buffer to hold retrived data.
  @param  RequestElement         An optional field to specify which part of the
                                 buffer data will be send back to Browser. If NULL,
                                 the whole buffer of data will be committed to
                                 Browser. <RequestElement> ::=
                                 &OFFSET=<Number>&WIDTH=<Number>*

  @retval EFI_SUCCESS            Routine success.
  @retval EFI_OUT_OF_RESOURCES   There is no enough available memory space.
  @retval Other                  Updating Browser uncommitted data failed.

**/
EFI_STATUS
EFIAPI
SetBrowserData (
  EFI_GUID                   *VariableGuid, OPTIONAL
  CHAR16                     *VariableName, OPTIONAL
  UINTN                      BufferSize,
  UINT8                      *Buffer,
  CHAR16                     *RequestElement  OPTIONAL
  )
{
  EFI_STATUS                      Status;
  CONST CHAR16                    *ConfigHdr;
  CHAR16                          *ConfigResp;
  CHAR16                          *StringPtr;
  UINTN                           HeaderLen;
  UINTN                           BufferLen;
  CHAR16                          *Progress;
  CHAR16                          BlockName[33];
  CHAR16                          *ConfigRequest;
  CHAR16                          *Request;

  //
  // Locate protocols for use
  //
  Status = LocateFormBrowser2Protocols ();
  if (EFI_ERROR (Status)) {
    return Status;
  }

  //
  // Prepare <ConfigRequest>
  //
  ConfigHdr = mFakeConfigHdr;
  HeaderLen = StrLen (ConfigHdr);

  if (RequestElement == NULL) {
    //
    // RequestElement not specified, use "&OFFSET=0&WIDTH=<BufferSize>" as <BlockName>
    //
    BlockName[0] = L'\0';
    StrCpy (BlockName, L"&OFFSET=0&WIDTH=");

    //
    // String lenghth of L"&OFFSET=0&WIDTH=" is 16
    //
    StringPtr = BlockName + 16;
    BufferLen = sizeof (BlockName) - (16 * sizeof (CHAR16));
    BufToHexString (StringPtr, &BufferLen, (UINT8 *) &BufferSize, sizeof (UINTN));

    Request = BlockName;
  } else {
    Request = RequestElement;
  }

  BufferLen = HeaderLen * sizeof (CHAR16) + StrSize (Request);
  ConfigRequest = AllocateZeroPool (BufferLen);
  if (ConfigRequest == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }

  CopyMem (ConfigRequest, ConfigHdr, HeaderLen * sizeof (CHAR16));
  StringPtr = ConfigRequest + HeaderLen;
  StrCpy (StringPtr, Request);

  //
  // Convert buffer to <ConfigResp>
  //
  Status = mHiiConfigRouting->BlockToConfig (
                                mHiiConfigRouting,
                                ConfigRequest,
                                Buffer,
                                BufferSize,
                                &ConfigResp,
                                &Progress
                                );
  if (EFI_ERROR (Status)) {
    FreePool (ConfigRequest);
    return Status;
  }

  //
  // Skip <ConfigHdr> and '&'
  //
  StringPtr = ConfigResp + HeaderLen + 1;

  //
  // Change uncommitted data in Browser
  //
  Status = mFormBrowser2->BrowserCallback (
                           mFormBrowser2,
                           &BufferSize,
                           StringPtr,
                           FALSE,
                           NULL,
                           NULL
                           );
  FreePool (ConfigRequest);
  return Status;
}
