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

CONST EFI_FORM_BROWSER2_PROTOCOL      *mFormBrowser2     = NULL;
CONST EFI_HII_CONFIG_ROUTING_PROTOCOL *mIfrSupportLibHiiConfigRouting = NULL;
GLOBAL_REMOVE_IF_UNREFERENCED CONST CHAR8 mIfrSupportLibHexStr[] = {'0','1','2','3','4','5','6','7','8','9','A','B','C','D','E','F'};

//
// Fake <ConfigHdr>
//
GLOBAL_REMOVE_IF_UNREFERENCED CONST UINT16 mFakeConfigHdr[] = L"GUID=00000000000000000000000000000000&NAME=0000&PATH=0";

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
  
  if (mIfrSupportLibHiiConfigRouting == NULL) {
    Status = gBS->LocateProtocol (&gEfiHiiConfigRoutingProtocolGuid, NULL, (VOID **) &mIfrSupportLibHiiConfigRouting);
    if (EFI_ERROR (Status)) {
      return Status;
    }
  }

  return EFI_SUCCESS;
}

/**
  Draw a dialog and return the selected key.

  @param  NumberOfLines          The number of lines for the dialog box
  @param  KeyValue               The EFI_KEY value returned if HotKey is TRUE..
  @param  String                 The first String to be displayed in the Pop-Up.
  @param  Marker                 A series of (quantity == NumberOfLines - 1) text
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
  IN  CHAR16                      *String,
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

  VA_START (Marker, String);

  Status = IfrLibCreatePopUp2 (NumberOfLines, KeyValue, String, Marker);

  VA_END (Marker);

  return Status;
}

/**
    Extract block name from the array generated by VFR compiler. The name of
  this array is "Vfr + <StorageName> + BlockName", e.g. "VfrMyIfrNVDataBlockName".
  Format of this array is:
     Array length | 4-bytes
       Offset     | 2-bytes
       Width      | 2-bytes
       Offset     | 2-bytes
       Width      | 2-bytes
       ... ...

    @param Buffer                 Array generated by VFR compiler.
    @param BlockName              The returned <BlockName>

    @retval EFI_OUT_OF_RESOURCES   Run out of memory resource.
    @retval EFI_INVALID_PARAMETER  Buffer is NULL or BlockName is NULL.
    @retval EFI_SUCCESS            Operation successful.

**/
EFI_STATUS
ExtractBlockName (
  IN UINT8                        *Buffer,
  OUT CHAR16                      **BlockName
  )

{
  UINTN       Index;
  UINT32      Length;
  UINT32      BlockNameNumber;
  UINTN       HexStringBufferLen;
  CHAR16      *StringPtr;

  if ((Buffer == NULL) || (BlockName == NULL)) {
    return EFI_INVALID_PARAMETER;
  }

  //
  // Calculate number of Offset/Width pair
  //
  CopyMem (&Length, Buffer, sizeof (UINT32));
  BlockNameNumber = (Length - sizeof (UINT32)) / (sizeof (UINT16) * 2);

  //
  // <BlockName> ::= &OFFSET=1234&WIDTH=1234
  //                 |   8  | 4 |  7   | 4 |
  //
  StringPtr = AllocateZeroPool ((BlockNameNumber * (8 + 4 + 7 + 4) + 1) * sizeof (CHAR16));
  if (StringPtr == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }
  *BlockName = StringPtr;

  Buffer += sizeof (UINT32);
  for (Index = 0; Index < BlockNameNumber; Index++) {
    StrCpy (StringPtr, L"&OFFSET=");
    StringPtr += 8;

    HexStringBufferLen = 5;
    BufToHexString (StringPtr, &HexStringBufferLen, Buffer, sizeof (UINT16));
    Buffer += sizeof (UINT16);
    StringPtr += 4;

    StrCpy (StringPtr, L"&WIDTH=");
    StringPtr += 7;

    HexStringBufferLen = 5;
    BufToHexString (StringPtr, &HexStringBufferLen, Buffer, sizeof (UINT16));
    Buffer += sizeof (UINT16);
    StringPtr += 4;
  }

  return EFI_SUCCESS;
}

/**

    Extract block config from the array generated by VFR compiler. The name of
  this array is "Vfr + <StorageName> + Default<HexCh>4", e.g. "VfrMyIfrNVDataDefault0000".

    @param Buffer                - Array generated by VFR compiler.
    @param BlockConfig           - The returned <BlockConfig>

    @retval EFI_OUT_OF_RESOURCES  - Run out of memory resource.
    @retval EFI_INVALID_PARAMETER - Buffer is NULL or BlockConfig is NULL.
    @retval EFI_SUCCESS           - Operation successful.

**/
EFI_STATUS
ExtractBlockConfig (
  IN UINT8                        *Buffer,
  OUT CHAR16                      **BlockConfig
  )
{
  UINT32      Length;
  UINT16      Width;
  UINTN       HexStringBufferLen;
  CHAR16      *StringPtr;
  UINT8       *BufferEnd;
  CHAR16      *StringEnd;
  EFI_STATUS  Status;

  if ((Buffer == NULL) || (BlockConfig == NULL)) {
    return EFI_INVALID_PARAMETER;
  }

  //
  // Calculate length of AltResp string
  // Format of Default value array is:
  //  Array length | 4-bytes
  //        Offset | 2-bytes
  //        Width  | 2-bytes
  //        Value  | Variable length
  //        Offset | 2-bytes
  //        Width  | 2-bytes
  //        Value  | Variable length
  //        ... ...
  // When value is 1 byte in length, overhead of AltResp string will be maximum,
  //  BlockConfig ::= <&OFFSET=1234&WIDTH=1234&VALUE=12>+
  //                   |   8   | 4  |  7   | 4 |  7  |2|
  // so the maximum length of BlockConfig could be calculated as:
  // (ArrayLength / 5) * (8 + 4 + 7 + 4 + 7 + 2) = ArrayLength * 6.4 < ArrayLength * 7
  //
  CopyMem (&Length, Buffer, sizeof (UINT32));
  BufferEnd = Buffer + Length;
  StringPtr = AllocatePool (Length * 7 * sizeof (CHAR16));
  *BlockConfig = StringPtr;
  if (StringPtr == NULL) {
      return EFI_OUT_OF_RESOURCES;
  }
  StringEnd = StringPtr + (Length * 7);

  Buffer += sizeof (UINT32);
  while (Buffer < BufferEnd) {
    StrCpy (StringPtr, L"&OFFSET=");
    StringPtr += 8;

    HexStringBufferLen = 5;
    BufToHexString (StringPtr, &HexStringBufferLen, Buffer, sizeof (UINT16));
    Buffer += sizeof (UINT16);
    StringPtr += 4;

    StrCpy (StringPtr, L"&WIDTH=");
    StringPtr += 7;

    HexStringBufferLen = 5;
    BufToHexString (StringPtr, &HexStringBufferLen, Buffer, sizeof (UINT16));
    CopyMem (&Width, Buffer, sizeof (UINT16));
    Buffer += sizeof (UINT16);
    StringPtr += 4;

    StrCpy (StringPtr, L"&VALUE=");
    StringPtr += 7;

    HexStringBufferLen = StringEnd - StringPtr;
    Status = BufToHexString (StringPtr, &HexStringBufferLen, Buffer, Width);
    if (EFI_ERROR (Status)) {
      return Status;
    }
    Buffer += Width;
    StringPtr += (Width * 2);
  }

  return EFI_SUCCESS;
}

/**
    Construct <ConfigAltResp> for a buffer storage.

    @param ConfigRequest           The Config request string. If set to NULL, all the
                                   configurable elements will be extracted from BlockNameArray.
    @param ConfigAltResp           The returned <ConfigAltResp>.
    @param Progress                On return, points to a character in the Request.
    @param Guid                    GUID of the buffer storage.
    @param Name                    Name of the buffer storage.
    @param DriverHandle            The DriverHandle which is used to invoke HiiDatabase
                                   protocol interface NewPackageList().
    @param BufferStorage           Content of the buffer storage.
    @param BufferStorageSize       Length in bytes of the buffer storage.
    @param BlockNameArray          Array generated by VFR compiler.
    @param NumberAltCfg            Number of Default value array generated by VFR compiler.
                                   The sequential input parameters will be number of
                                   AltCfgId and DefaultValueArray pairs. When set to 0,
                                   there will be no <AltResp>.

    retval EFI_OUT_OF_RESOURCES  Run out of memory resource.
    retval EFI_INVALID_PARAMETER ConfigAltResp is NULL.
    retval EFI_SUCCESS           Operation successful.

**/
EFI_STATUS
ConstructConfigAltResp (
  IN  EFI_STRING                  ConfigRequest,  OPTIONAL
  OUT EFI_STRING                  *Progress,
  OUT EFI_STRING                  *ConfigAltResp,
  IN  EFI_GUID                    *Guid,
  IN  CHAR16                      *Name,
  IN  EFI_HANDLE                  *DriverHandle,
  IN  VOID                        *BufferStorage,
  IN  UINTN                       BufferStorageSize,
  IN  VOID                        *BlockNameArray, OPTIONAL
  IN  UINTN                       NumberAltCfg,
  ...
//IN  UINT16                      AltCfgId,
//IN  VOID                        *DefaultValueArray,
  )
{
  EFI_STATUS                      Status;
  CHAR16                          *ConfigHdr;
  CHAR16                          *BlockName;
  CHAR16                          *DescHdr;
  CHAR16                          *StringPtr;
  CHAR16                          **AltCfg;
  UINT16                          AltCfgId;
  VOID                            *DefaultValueArray;
  UINTN                           StrBufferLen;
  EFI_STRING                      ConfigResp;
  EFI_STRING                      TempStr;
  VA_LIST                         Args;
  UINTN                           AltRespLen;
  UINTN                           Index;
  BOOLEAN                         NeedFreeConfigRequest;
  EFI_HII_CONFIG_ROUTING_PROTOCOL *HiiConfigRouting;
  UINTN                           Len;

  if (ConfigAltResp == NULL) {
    return EFI_INVALID_PARAMETER;
  }
  
  DescHdr = NULL;
  StringPtr = NULL;
  AltCfg    = NULL;
  ConfigResp = NULL;
  BlockName = NULL;
  NeedFreeConfigRequest = FALSE;

  //
  // Construct <ConfigHdr> : "GUID=...&NAME=...&PATH=..."
  //
  ConfigHdr = NULL;
  StrBufferLen = 0;
  Status = ConstructConfigHdr (
             ConfigHdr,
             &StrBufferLen,
             Guid,
             Name,
             DriverHandle
             );
  ASSERT (Status == EFI_BUFFER_TOO_SMALL);
  ConfigHdr = AllocateZeroPool (StrBufferLen);
  if (ConfigHdr == NULL) {
    Status = EFI_OUT_OF_RESOURCES;
    goto Exit;
  }
  Status = ConstructConfigHdr (
             ConfigHdr,
             &StrBufferLen,
             Guid,
             Name,
             DriverHandle
             );

  if (EFI_ERROR (Status)) {
    goto Exit;
  }

  //
  // Construct <ConfigResp>
  //
  if (ConfigRequest == NULL) {
    //
    // If ConfigRequest is set to NULL, export all configurable elements in BlockNameArray
    //
    Status = ExtractBlockName (BlockNameArray, &BlockName);
    if (EFI_ERROR (Status)) {
      goto Exit;
    }
    
    Len = StrSize (ConfigHdr);
    ConfigRequest = AllocateZeroPool (Len + StrSize (BlockName) - sizeof (CHAR16));
    if (ConfigRequest == NULL) {
      Status = EFI_OUT_OF_RESOURCES;
      goto Exit;
    }
    
    StrCpy (ConfigRequest, ConfigHdr);
    StrCat (ConfigRequest, BlockName);
    NeedFreeConfigRequest = TRUE;

  }

  Status = gBS->LocateProtocol (&gEfiHiiConfigRoutingProtocolGuid, NULL, (VOID **) &HiiConfigRouting);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  Status = HiiConfigRouting->BlockToConfig (
                               HiiConfigRouting,
                               ConfigRequest,
                               BufferStorage,
                               BufferStorageSize,
                               &ConfigResp,
                               (Progress == NULL) ? &TempStr : Progress
                               );
  if (EFI_ERROR (Status)) {
    return Status;
  }

  AltRespLen = 0;
  //
  // Construct <AltResp>
  //
  if (NumberAltCfg > 0) {
    DescHdr = AllocateZeroPool (NumberAltCfg * 16 * sizeof (CHAR16));
    if (DescHdr == NULL) {
      Status = EFI_OUT_OF_RESOURCES;
      goto Exit;
    }
    
    StringPtr = DescHdr;
    AltCfg = AllocateZeroPool (NumberAltCfg * sizeof (CHAR16 *));
    if (AltCfg == NULL) {
      Status = EFI_OUT_OF_RESOURCES;
      goto Exit;
    }

    VA_START (Args, NumberAltCfg);
    for (Index = 0; Index < NumberAltCfg; Index++) {
      AltCfgId = (UINT16) VA_ARG (Args, UINT16);
      DefaultValueArray = (UINT8 *) VA_ARG (Args, VOID *);
    
      //
      // '&' <ConfigHdr>
      //
      AltRespLen += (StrLen (ConfigHdr) + 1);
    
      StringPtr = DescHdr + Index * 16;
      StrCpy (StringPtr, L"&ALTCFG=");
      AltRespLen += (8 + sizeof (UINT16) * 2);
    
      StrBufferLen = 5;
      BufToHexString (StringPtr + 8, &StrBufferLen, (UINT8 *) &AltCfgId, sizeof (UINT16));
      Status = ExtractBlockConfig (DefaultValueArray, &AltCfg[Index]);
      if (EFI_ERROR (Status)) {
        goto Exit;
      }
      AltRespLen += StrLen (AltCfg[Index]);
    }
    VA_END (Args);
  }

  //
  // Generate the final <ConfigAltResp>
  //
  StrBufferLen = (StrLen ((CHAR16 *) ConfigResp) + AltRespLen + 1) * sizeof (CHAR16);
  TempStr = AllocateZeroPool (StrBufferLen);
  *ConfigAltResp = TempStr;
  if (TempStr == NULL) {
    goto Exit;
  }

  //
  // <ConfigAltResp> ::= <ConfigResp> ['&' <AltResp>]*
  //
  StrCpy (TempStr, ConfigResp);
  for (Index = 0; Index < NumberAltCfg; Index++) {
    StrCat (TempStr, L"&");
    StrCat (TempStr, ConfigHdr);
    StrCat (TempStr, DescHdr + Index * 16);
    StrCat (TempStr, AltCfg[Index]);

    FreePool (AltCfg[Index]);
  }

Exit:
  if (NeedFreeConfigRequest) {
    FreePool (ConfigRequest);
  }
  FreePool (ConfigHdr);
  if (ConfigResp != NULL) {
    FreePool (ConfigResp);
  }

  if (BlockName != NULL) {
    FreePool (BlockName);
  }

  if (NumberAltCfg > 0) {
    FreePool (DescHdr);
    FreePool (AltCfg);
  }

  return EFI_SUCCESS;
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
BufInReverseOrderToHexString (
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
  @retval RETURN_BUFFER_TOO_SMALL   The input BufferSize is too small to hold the output. BufferSize
                                 will be updated to the size required for the converstion.

**/
EFI_STATUS
EFIAPI
HexStringToBufInReverseOrder (
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
    SwapBuffer (Buffer, (ConvertedStrLen + 1) / 2);
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

  @retval EFI_SUCCESS          Operation completes successfully.
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

  @retval EFI_SUCCESS           Operation completes successfully.
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

  @retval EFI_SUCCESS            Operation completes successfully.
  @retval EFI_BUFFER_TOO_SMALL   The ConfigHdr string buffer is too small.

**/
EFI_STATUS
EFIAPI
ConstructConfigHdr (
  IN OUT CHAR16                *ConfigHdr,
  IN OUT UINTN                 *StrBufferLen,
  IN CONST EFI_GUID            *Guid,
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
  BufInReverseOrderToHexString (StrPtr, (UINT8 *) Guid, sizeof (EFI_GUID));
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
  BufInReverseOrderToHexString (StrPtr, (UINT8 *) DevicePath, DevicePathSize);

  return EFI_SUCCESS;
}

/**
  Determines if the Routing data (Guid and Name) is correct in <ConfigHdr>.

  @param ConfigString  Either <ConfigRequest> or <ConfigResp>.
  @param StorageGuid   GUID of the storage.
  @param StorageName   Name of the stoarge.

  @retval TRUE         Routing information is correct in ConfigString.
  @retval FALSE        Routing information is incorrect in ConfigString.

**/
BOOLEAN
IsConfigHdrMatch (
  IN EFI_STRING                ConfigString,
  IN EFI_GUID                  *StorageGuid, OPTIONAL
  IN CHAR16                    *StorageName  OPTIONAL
  )
{
  EFI_STATUS  Status;
  BOOLEAN     Match;
  EFI_GUID    Guid;
  CHAR16      *Name;
  CHAR16      *StrPtr;
  UINTN       BufferSize;

  //
  // <ConfigHdr> ::=
  // GUID=<HexCh>32&NAME=<Char>NameStrLen&PATH=<HexChar>DevicePathStrLen <NULL>
  // | 5  |   32   |  6  |  NameStrLen*4 |  6  |    DevicePathStrLen    | 1 |
  //
  if (StrLen (ConfigString) <= (5 + 32 + 6)) {
    return FALSE;
  }

  //
  // Compare GUID
  //
  if (StorageGuid != NULL) {

    StrPtr = ConfigString + 5 + 32;
    if (*StrPtr != L'&') {
      return FALSE;
    }
    *StrPtr = L'\0';

    BufferSize = sizeof (EFI_GUID);
    Status = HexStringToBufInReverseOrder (
               (UINT8 *) &Guid,
               &BufferSize,
               ConfigString + 5
               );
    *StrPtr = L'&';

    if (EFI_ERROR (Status)) {
      return FALSE;
    }

    if (!CompareGuid (&Guid, StorageGuid)) {
      return FALSE;
    }
  }

  //
  // Compare Name
  //
  Match = TRUE;
  if (StorageName != NULL) {
    StrPtr = ConfigString + 5 + 32 + 6;
    while (*StrPtr != L'\0' && *StrPtr != L'&') {
      StrPtr++;
    }
    if (*StrPtr != L'&') {
      return FALSE;
    }

    *StrPtr = L'\0';
    BufferSize = (((UINTN) StrPtr) - ((UINTN) &ConfigString[5 + 32 + 6])) / 4 + sizeof (CHAR16);
    Name = AllocatePool (BufferSize);
    ASSERT (Name != NULL);
    Status = ConfigStringToUnicode (
               Name,
               &BufferSize,
               ConfigString + 5 + 32 + 6
               );
    *StrPtr = L'&';

    if (EFI_ERROR (Status) || (StrCmp (Name, StorageName) != 0)) {
      Match = FALSE;
    }
    FreePool (Name);
  }

  return Match;
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
  IN UINTN                     Offset,
  IN UINTN                     Width
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

  @retval EFI_SUCCESS            Operation completes successfully.
  @retval EFI_BUFFER_TOO_SMALL   The intput buffer is too small.
  @retval EFI_OUT_OF_RESOURCES   There is no enough available memory space.

**/
EFI_STATUS
EFIAPI
GetBrowserData (
  IN CONST EFI_GUID              *VariableGuid, OPTIONAL
  IN CONST CHAR16                *VariableName, OPTIONAL
  IN OUT UINTN                   *BufferSize,
  IN OUT UINT8                   *Buffer
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
  if (Status == EFI_BUFFER_TOO_SMALL) {
    FreePool (ConfigResp);

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
  Status = mIfrSupportLibHiiConfigRouting->ConfigToBlock (
                               mIfrSupportLibHiiConfigRouting,
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

  @retval EFI_SUCCESS            Operation completes successfully.
  @retval EFI_OUT_OF_RESOURCES   There is no enough available memory space.
  @retval Other                  Updating Browser uncommitted data failed.

**/
EFI_STATUS
EFIAPI
SetBrowserData (
  IN CONST EFI_GUID          *VariableGuid, OPTIONAL
  IN CONST CHAR16            *VariableName, OPTIONAL
  IN UINTN                   BufferSize,
  IN CONST UINT8             *Buffer,
  IN CONST CHAR16            *RequestElement  OPTIONAL
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
  CONST CHAR16                    *Request;

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
  Status = mIfrSupportLibHiiConfigRouting->BlockToConfig (
                                mIfrSupportLibHiiConfigRouting,
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
                           VariableGuid,
                           VariableName
                           );
  FreePool (ConfigRequest);
  return Status;
}

/**
  Test if  a Unicode character is a hexadecimal digit. If true, the input
  Unicode character is converted to a byte. 

  This function tests if a Unicode character is a hexadecimal digit. If true, the input
  Unicode character is converted to a byte. For example, Unicode character
  L'A' will be converted to 0x0A. 

  If Digit is NULL, then ASSERT.

  @param  Digit       The output hexadecimal digit.

  @param  Char        The input Unicode character.

  @retval TRUE        Char is in the range of Hexadecimal number. Digit is updated
                      to the byte value of the number.
  @retval FALSE       Char is not in the range of Hexadecimal number. Digit is keep
                      intact.

**/
BOOLEAN
EFIAPI
IsHexDigit (
  OUT UINT8      *Digit,
  IN  CHAR16      Char
  )
{
  ASSERT (Digit != NULL);
  
  if ((Char >= L'0') && (Char <= L'9')) {
    *Digit = (UINT8) (Char - L'0');
    return TRUE;
  }

  if ((Char >= L'A') && (Char <= L'F')) {
    *Digit = (UINT8) (Char - L'A' + 0x0A);
    return TRUE;
  }

  if ((Char >= L'a') && (Char <= L'f')) {
    *Digit = (UINT8) (Char - L'a' + 0x0A);
    return TRUE;
  }

  return FALSE;
}

/** 
  Convert binary buffer to a Unicode String in a specified sequence. 

  This function converts bytes in the memory block pointed by Buffer to a Unicode String Str. 
  Each byte will be represented by two Unicode characters. For example, byte 0xA1 will 
  be converted into two Unicode character L'A' and L'1'. In the output String, the Unicode Character 
  for the Most Significant Nibble will be put before the Unicode Character for the Least Significant
  Nibble. The output string for the buffer containing a single byte 0xA1 will be L"A1". 
  For a buffer with multiple bytes, the Unicode character produced by the first byte will be put into the 
  the last character in the output string. The one next to first byte will be put into the
  character before the last character. This rules applies to the rest of the bytes. The Unicode
  character by the last byte will be put into the first character in the output string. For example,
  the input buffer for a 64-bits unsigned integer 0x12345678abcdef1234 will be converted to
  a Unicode string equal to L"12345678abcdef1234".

  @param String                        On input, String is pointed to the buffer allocated for the convertion.
  @param StringLen                     The Length of String buffer to hold the output String. The length must include the tailing '\0' character.
                                       The StringLen required to convert a N bytes Buffer will be a least equal to or greater 
                                       than 2*N + 1.
  @param Buffer                        The pointer to a input buffer.
  @param BufferSizeInBytes             Length in bytes of the input buffer.


  @retval  EFI_SUCCESS                 The convertion is successful. All bytes in Buffer has been convert to the corresponding
                                       Unicode character and placed into the right place in String.
  @retval  EFI_BUFFER_TOO_SMALL        StringSizeInBytes is smaller than 2 * N + 1the number of bytes required to
                                       complete the convertion. 
**/
RETURN_STATUS
EFIAPI
BufToHexString (
  IN OUT       CHAR16               *String,
  IN OUT       UINTN                *StringLen,
  IN     CONST UINT8                *Buffer,
  IN           UINTN                BufferSizeInBytes
  )
{
  UINTN       Idx;
  UINT8       Byte;
  UINTN       StrLen;

  //
  // Make sure string is either passed or allocate enough.
  // It takes 2 Unicode characters (4 bytes) to represent 1 byte of the binary buffer.
  // Plus the Unicode termination character.
  //
  StrLen = BufferSizeInBytes * 2;
  if (StrLen > ((*StringLen) - 1)) {
    *StringLen = StrLen + 1;
    return RETURN_BUFFER_TOO_SMALL;
  }

  *StringLen = StrLen + 1;
  //
  // Ends the string.
  //
  String[StrLen] = L'\0'; 

  for (Idx = 0; Idx < BufferSizeInBytes; Idx++) {
    Byte = Buffer[Idx];
    String[StrLen - 1 - Idx * 2] = mIfrSupportLibHexStr [Byte & 0xF];
    String[StrLen - 2 - Idx * 2] = mIfrSupportLibHexStr [Byte >> 4];
  }

  return RETURN_SUCCESS;
}


/**
  Convert a Unicode string consisting of hexadecimal characters to a output byte buffer.

  This function converts a Unicode string consisting of characters in the range of Hexadecimal
  character (L'0' to L'9', L'A' to L'F' and L'a' to L'f') to a output byte buffer. The function will stop
  at the first non-hexadecimal character or the NULL character. The convertion process can be
  simply viewed as the reverse operations defined by BufToHexString. Two Unicode characters will be 
  converted into one byte. The first Unicode character represents the Most Significant Nibble and the
  second Unicode character represents the Least Significant Nibble in the output byte. 
  The first pair of Unicode characters represents the last byte in the output buffer. The second pair of Unicode 
  characters represent the  the byte preceding the last byte. This rule applies to the rest pairs of bytes. 
  The last pair represent the first byte in the output buffer. 

  For example, a Unciode String L"12345678" will be converted into a buffer wil the following bytes 
  (first byte is the byte in the lowest memory address): "0x78, 0x56, 0x34, 0x12".

  If String has N valid hexadecimal characters for conversion,  the caller must make sure Buffer is at least 
  N/2 (if N is even) or (N+1)/2 (if N if odd) bytes. 

  @param Buffer                      The output buffer allocated by the caller.
  @param BufferSizeInBytes           On input, the size in bytes of Buffer. On output, it is updated to 
                                     contain the size of the Buffer which is actually used for the converstion.
                                     For Unicode string with 2*N hexadecimal characters (not including the 
                                     tailing NULL character), N bytes of Buffer will be used for the output.
  @param String                      The input hexadecimal string.
  @param ConvertedStrLen             The number of hexadecimal characters used to produce content in output
                                     buffer Buffer.

  @retval  RETURN_BUFFER_TOO_SMALL   The input BufferSizeInBytes is too small to hold the output. BufferSizeInBytes
                                     will be updated to the size required for the converstion.
  @retval  RETURN_SUCCESS            The convertion is successful or the first Unicode character from String
                                     is hexadecimal. If ConvertedStrLen is not NULL, it is updated
                                     to the number of hexadecimal character used for the converstion.
**/
RETURN_STATUS
EFIAPI
HexStringToBuf (
  OUT          UINT8                    *Buffer,   
  IN OUT       UINTN                    *BufferSizeInBytes,
  IN     CONST CHAR16                   *String,
  OUT          UINTN                    *ConvertedStrLen  OPTIONAL
  )
{
  UINTN       HexCnt;
  UINTN       Idx;
  UINTN       BufferLength;
  UINT8       Digit;
  UINT8       Byte;

  //
  // Find out how many hex characters the string has.
  //
  for (Idx = 0, HexCnt = 0; IsHexDigit (&Digit, String[Idx]); Idx++, HexCnt++);

  if (HexCnt == 0) {
    *ConvertedStrLen = 0;
    return RETURN_SUCCESS;
  }
  //
  // Two Unicode characters make up 1 buffer byte. Round up.
  //
  BufferLength = (HexCnt + 1) / 2; 

  //
  // Test if  buffer is passed enough.
  //
  if (BufferLength > (*BufferSizeInBytes)) {
    *BufferSizeInBytes = BufferLength;
    return RETURN_BUFFER_TOO_SMALL;
  }

  *BufferSizeInBytes = BufferLength;

  for (Idx = 0; Idx < HexCnt; Idx++) {

    IsHexDigit (&Digit, String[HexCnt - 1 - Idx]);

    //
    // For odd charaters, write the lower nibble for each buffer byte,
    // and for even characters, the upper nibble.
    //
    if ((Idx & 1) == 0) {
      Byte = Digit;
    } else {
      Byte = Buffer[Idx / 2];
      Byte &= 0x0F;
      Byte = (UINT8) (Byte | Digit << 4);
    }

    Buffer[Idx / 2] = Byte;
  }

  if (ConvertedStrLen != NULL) {
    *ConvertedStrLen = HexCnt;
  }

  return RETURN_SUCCESS;
}
