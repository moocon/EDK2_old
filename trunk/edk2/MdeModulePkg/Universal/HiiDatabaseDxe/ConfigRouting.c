/** @file
Implementation of interfaces function for EFI_HII_CONFIG_ROUTING_PROTOCOL.

Copyright (c) 2007 - 2009, Intel Corporation
All rights reserved. This program and the accompanying materials
are licensed and made available under the terms and conditions of the BSD License
which accompanies this distribution.  The full text of the license may be found at
http://opensource.org/licenses/bsd-license.php

THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/


#include "HiiDatabase.h"

/**
  Calculate the number of Unicode characters of the incoming Configuration string,
  not including NULL terminator.

  This is a internal function.

  @param  String                 String in <MultiConfigRequest> or
                                 <MultiConfigResp> format.

  @return The number of Unicode characters.

**/
UINTN
CalculateConfigStringLen (
  IN EFI_STRING                    String
  )
{
  UINTN Length;

  //
  // "GUID=" should be the first element of incoming string.
  //
  ASSERT (String != NULL);
  ASSERT (StrnCmp (String, L"GUID=", StrLen (L"GUID=")) == 0);

  Length  = StrLen (L"GUID=");
  String += Length;

  //
  // The beginning of next <ConfigRequest>/<ConfigResp> should be "&GUID=".
  // Will meet '\0' if there is only one <ConfigRequest>/<ConfigResp>.
  //
  while (*String != 0 && StrnCmp (String, L"&GUID=", StrLen (L"&GUID=")) != 0) {
    Length++;
    String++;
  }

  return Length;
}


/**
  Convert the hex UNICODE %02x encoding of a UEFI device path to binary
  from <PathHdr> of <ConfigHdr>.

  This is a internal function.

  @param  String                 UEFI configuration string
  @param  DevicePath             binary of a UEFI device path.

  @retval EFI_INVALID_PARAMETER  Any incoming parameter is invalid.
  @retval EFI_OUT_OF_RESOURCES   Lake of resources to store neccesary structures.
  @retval EFI_SUCCESS            The device path is retrieved and translated to
                                 binary format.

**/
EFI_STATUS
GetDevicePath (
  IN  EFI_STRING                   String,
  OUT UINT8                        **DevicePath
  )
{
  UINTN      Length;
  EFI_STRING PathHdr;
  EFI_STRING DevicePathString;
  UINT8      *DevicePathBuffer;
  CHAR16     TemStr[2];
  UINTN      Index;
  UINT8      DigitUint8;

  if (String == NULL || DevicePath == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  //
  // Find the 'PATH=' of <PathHdr> and skip it.
  //
  for (; (*String != 0 && StrnCmp (String, L"PATH=", StrLen (L"PATH=")) != 0); String++);
  if (*String == 0) {
    return EFI_INVALID_PARAMETER;
  }

  String += StrLen (L"PATH=");
  PathHdr = String;

  //
  // The content between 'PATH=' of <ConfigHdr> and '&' of next element
  // or '\0' (end of configuration string) is the UNICODE %02x bytes encoding
  // of UEFI device path.
  //
  for (Length = 0; *String != 0 && *String != L'&'; String++, Length++);
  DevicePathString = (EFI_STRING) AllocateZeroPool ((Length + 1) * sizeof (CHAR16));
  if (DevicePathString == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }
  StrnCpy (DevicePathString, PathHdr, Length);
  *(DevicePathString + Length) = 0;

  //
  // The data in <PathHdr> is encoded as hex UNICODE %02x bytes in the same order
  // as the device path resides in RAM memory.
  // Translate the data into binary.
  //
  DevicePathBuffer = (UINT8 *) AllocateZeroPool ((Length + 1) / 2);
  if (DevicePathBuffer == NULL) {
    FreePool (DevicePathString);
    return EFI_OUT_OF_RESOURCES;
  }

  ZeroMem (TemStr, sizeof (TemStr));
  for (Index = 0; DevicePathString[Index] != L'\0'; Index ++) {
    TemStr[0] = DevicePathString[Index];
    DigitUint8 = (UINT8) StrHexToUint64 (TemStr);
    if ((Index & 1) == 0) {
      DevicePathBuffer [Index/2] = DigitUint8;
    } else {
      DevicePathBuffer [Index/2] = (UINT8) ((DevicePathBuffer [Index/2] << 4) + DigitUint8);
    }
  }

  FreePool (DevicePathString);
  
  *DevicePath = DevicePathBuffer;

  return EFI_SUCCESS;

}

/**
  Converts the unicode character of the string from uppercase to lowercase.
  This is a internal function.

  @param Str     String to be converted

**/
VOID
EFIAPI
HiiToLower (
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
  Generate a sub string then output it.

  This is a internal function.

  @param  String                 A constant string which is the prefix of the to be
                                 generated string, e.g. GUID=
  @param  BufferLen              The length of the Buffer in bytes.
  @param  Buffer                 Points to a buffer which will be converted to be the 
                                          content of the generated string.
  @param  Flag           If 1, the buffer contains data for the value of GUID or PATH stored in 
                                UINT8 *; if 2, the buffer contains unicode string for the value of NAME;
                                if 3, the buffer contains other data.
  @param  SubStr                 Points to the output string. It's caller's
                                 responsibility to free this buffer.


**/
VOID
GenerateSubStr (
  IN CONST EFI_STRING              String,
  IN  UINTN                        BufferLen,
  IN  VOID                         *Buffer,
  IN  UINT8                        Flag,
  OUT EFI_STRING                   *SubStr
  )
{
  UINTN       Length;
  EFI_STRING  Str;
  EFI_STRING  StringHeader;
  CHAR16      *TemString;
  CHAR16      *TemName;
  UINT8       *TemBuffer;
  UINTN       Index;

  ASSERT (String != NULL && SubStr != NULL);

  if (Buffer == NULL) {
    *SubStr = AllocateCopyPool (StrSize (String), String);
    ASSERT (*SubStr != NULL);
    return ;
  }

  Length = StrLen (String) + BufferLen * 2 + 1 + 1;
  Str    = AllocateZeroPool (Length * sizeof (CHAR16));
  ASSERT (Str != NULL);

  StrCpy (Str, String);
  Length = (BufferLen * 2 + 1) * sizeof (CHAR16);

  StringHeader = Str + StrLen (String);
  TemString    = (CHAR16 *) StringHeader;

  switch (Flag) {
  case 1:
    //
    // Convert Buffer to Hex String in reverse order
    //
    TemBuffer = ((UINT8 *) Buffer);
    for (Index = 0; Index < BufferLen; Index ++, TemBuffer ++) {
      TemString += UnicodeValueToString (TemString, PREFIX_ZERO | RADIX_HEX, *TemBuffer, 2);
    }
    break;
  case 2:
    //
    // Check buffer is enough
    //
    TemName = (CHAR16 *) Buffer;
    ASSERT (Length < ((StrLen (TemName) * 4 + 1) * sizeof (CHAR16)));
    //
    // Convert Unicode String to Config String, e.g. "ABCD" => "0041004200430044"
    //
    for (; *TemName != L'\0'; TemName++) {
      TemString += UnicodeValueToString (TemString, PREFIX_ZERO | RADIX_HEX, *TemName, 4);
    }
    break;
  case 3:
    //
    // Convert Buffer to Hex String
    //
    TemBuffer = ((UINT8 *) Buffer) + BufferLen - 1;
    for (Index = 0; Index < BufferLen; Index ++, TemBuffer --) {
      TemString += UnicodeValueToString (TemString, PREFIX_ZERO | RADIX_HEX, *TemBuffer, 2);
    }
    break;
  default:
    break;
  }

  //
  // Convert the uppercase to lowercase since <HexAf> is defined in lowercase format.
  //
  HiiToLower (StringHeader);
  StrCat (Str, L"&");

  *SubStr = Str;
}


/**
  Retrieve the <ConfigBody> from String then output it.

  This is a internal function.

  @param  String                 A sub string of a configuration string in
                                 <MultiConfigAltResp> format.
  @param  ConfigBody             Points to the output string. It's caller's
                                 responsibility to free this buffer.

  @retval EFI_INVALID_PARAMETER  There is no form package in current hii database.
  @retval EFI_OUT_OF_RESOURCES   Not enough memory to finish this operation.
  @retval EFI_SUCCESS            All existing storage is exported.

**/
EFI_STATUS
OutputConfigBody (
  IN  EFI_STRING                   String,
  OUT EFI_STRING                   *ConfigBody
  )
{
  EFI_STRING  TmpPtr;
  EFI_STRING  Result;
  UINTN       Length;

  if (String == NULL || ConfigBody == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  TmpPtr = StrStr (String, L"GUID=");
  if (TmpPtr == NULL) {
    //
    // It is the last <ConfigResp> of the incoming configuration string.
    //
    Result = AllocateCopyPool (StrSize (String), String);
    if (Result == NULL) {
      return EFI_OUT_OF_RESOURCES;
    } else {
      *ConfigBody = Result;
      return EFI_SUCCESS;
    }
  }

  Length = TmpPtr - String;
  Result = AllocateCopyPool (Length * sizeof (CHAR16), String);
  if (Result == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }

  *(Result + Length - 1) = 0;
  *ConfigBody = Result;
  return EFI_SUCCESS;

}

/**
  Append a string to a multi-string format.

  This is a internal function.

  @param  MultiString            String in <MultiConfigRequest>,
                                 <MultiConfigAltResp>, or <MultiConfigResp>. On
                                 input, the buffer length of  this string is
                                 MAX_STRING_LENGTH. On output, the  buffer length
                                 might be updated.
  @param  AppendString           NULL-terminated Unicode string.

  @retval EFI_INVALID_PARAMETER  Any incoming parameter is invalid.
  @retval EFI_SUCCESS            AppendString is append to the end of MultiString

**/
EFI_STATUS
AppendToMultiString (
  IN OUT EFI_STRING                *MultiString,
  IN EFI_STRING                    AppendString
  )
{
  UINTN AppendStringSize;
  UINTN MultiStringSize;

  if (MultiString == NULL || *MultiString == NULL || AppendString == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  AppendStringSize = StrSize (AppendString);
  MultiStringSize  = StrSize (*MultiString);

  //
  // Enlarge the buffer each time when length exceeds MAX_STRING_LENGTH.
  //
  if (MultiStringSize + AppendStringSize > MAX_STRING_LENGTH ||
      MultiStringSize > MAX_STRING_LENGTH) {
    *MultiString = (EFI_STRING) ReallocatePool (
                                  MultiStringSize,
                                  MultiStringSize + AppendStringSize,
                                  (VOID *) (*MultiString)
                                  );
    ASSERT (*MultiString != NULL);
  }
  //
  // Append the incoming string
  //
  StrCat (*MultiString, AppendString);

  return EFI_SUCCESS;
}


/**
  Get the value of <Number> in <BlockConfig> format, i.e. the value of OFFSET
  or WIDTH or VALUE.
  <BlockConfig> ::= 'OFFSET='<Number>&'WIDTH='<Number>&'VALUE'=<Number>

  This is a internal function.

  @param  StringPtr              String in <BlockConfig> format and points to the
                                 first character of <Number>.
  @param  Number                 The output value. Caller takes the responsibility
                                 to free memory.
  @param  Len                    Length of the <Number>, in characters.

  @retval EFI_OUT_OF_RESOURCES   Insufficient resources to store neccessary
                                 structures.
  @retval EFI_SUCCESS            Value of <Number> is outputted in Number
                                 successfully.

**/
EFI_STATUS
GetValueOfNumber (
  IN EFI_STRING                    StringPtr,
  OUT UINT8                        **Number,
  OUT UINTN                        *Len
  )
{
  EFI_STRING               TmpPtr;
  UINTN                    Length;
  EFI_STRING               Str;
  UINT8                    *Buf;
  EFI_STATUS               Status;
  UINT8                    DigitUint8;
  UINTN                    Index;
  CHAR16                   TemStr[2];

  ASSERT (StringPtr != NULL && Number != NULL && Len != NULL);
  ASSERT (*StringPtr != L'\0');

  Buf = NULL;

  TmpPtr = StringPtr;
  while (*StringPtr != L'\0' && *StringPtr != L'&') {
    StringPtr++;
  }
  *Len   = StringPtr - TmpPtr;
  Length = *Len + 1;

  Str = (EFI_STRING) AllocateZeroPool (Length * sizeof (EFI_STRING));
  if (Str == NULL) {
    Status = EFI_OUT_OF_RESOURCES;
    goto Exit;
  }
  CopyMem (Str, TmpPtr, *Len * sizeof (CHAR16));
  *(Str + *Len) = L'\0';

  Length = (Length + 1) / 2;
  Buf = (UINT8 *) AllocateZeroPool (Length);
  if (Buf == NULL) {
    Status = EFI_OUT_OF_RESOURCES;
    goto Exit;
  }
  
  Length = *Len;
  ZeroMem (TemStr, sizeof (TemStr));
  for (Index = 0; Index < Length; Index ++) {
    TemStr[0] = Str[Length - Index - 1];
    DigitUint8 = (UINT8) StrHexToUint64 (TemStr);
    if ((Index & 1) == 0) {
      Buf [Index/2] = DigitUint8;
    } else {
      Buf [Index/2] = (UINT8) ((DigitUint8 << 4) + Buf [Index/2]);
    }
  }

  *Number = Buf;
  Status  = EFI_SUCCESS;

Exit:
  if (Str != NULL) {
    FreePool (Str);
  }

  return Status;
}


/**
  This function allows a caller to extract the current configuration
  for one or more named elements from one or more drivers.

  @param  This                   A pointer to the EFI_HII_CONFIG_ROUTING_PROTOCOL
                                 instance.
  @param  Request                A null-terminated Unicode string in
                                 <MultiConfigRequest> format.
  @param  Progress               On return, points to a character in the Request
                                 string. Points to the string's null terminator if
                                 request was successful. Points to the most recent
                                 & before the first failing name / value pair (or
                                 the beginning of the string if the failure is in
                                 the first name / value pair) if the request was
                                 not successful.
  @param  Results                Null-terminated Unicode string in
                                 <MultiConfigAltResp> format which has all values
                                 filled in for the names in the Request string.
                                 String to be allocated by the called function.

  @retval EFI_SUCCESS            The Results string is filled with the values
                                 corresponding to all requested names.
  @retval EFI_OUT_OF_RESOURCES   Not enough memory to store the parts of the
                                 results that must be stored awaiting possible
                                 future        protocols.
  @retval EFI_NOT_FOUND          Routing data doesn't match any known driver.
                                   Progress set to the "G" in "GUID" of the routing
                                  header that doesn't match. Note: There is no
                                    requirement that all routing data be validated
                                 before any configuration extraction.
  @retval EFI_INVALID_PARAMETER  For example, passing in a NULL for the Request
                                 parameter would result in this type of error. The
                                 Progress parameter is set to NULL.
  @retval EFI_INVALID_PARAMETER  Illegal syntax. Progress set to most recent &
                                 before the error or the beginning of the string.
  @retval EFI_INVALID_PARAMETER  Unknown name. Progress points to the & before the
                                 name in question.

**/
EFI_STATUS
EFIAPI
HiiConfigRoutingExtractConfig (
  IN  CONST EFI_HII_CONFIG_ROUTING_PROTOCOL  *This,
  IN  CONST EFI_STRING                       Request,
  OUT EFI_STRING                             *Progress,
  OUT EFI_STRING                             *Results
  )
{
  EFI_STRING                          StringPtr;
  EFI_STRING                          ConfigRequest;
  UINTN                               Length;
  EFI_DEVICE_PATH_PROTOCOL            *DevicePath;
  EFI_DEVICE_PATH_PROTOCOL            *TempDevicePath;
  EFI_STATUS                          Status;
  EFI_HANDLE                          DriverHandle;
  EFI_HII_CONFIG_ACCESS_PROTOCOL      *ConfigAccess;
  EFI_STRING                          AccessProgress;
  EFI_STRING                          AccessResults;
  BOOLEAN                             FirstElement;

  if (This == NULL || Progress == NULL || Results == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  if (Request == NULL) {
    *Progress = NULL;
    return EFI_INVALID_PARAMETER;
  }

  StringPtr = Request;
  *Progress = StringPtr;

  //
  // The first element of <MultiConfigRequest> should be
  // <GuidHdr>, which is in 'GUID='<Guid> syntax.
  //
  if (StrnCmp (StringPtr, L"GUID=", StrLen (L"GUID=")) != 0) {
    return EFI_INVALID_PARAMETER;
  }

  FirstElement = TRUE;

  //
  // Allocate a fix length of memory to store Results. Reallocate memory for
  // Results if this fix length is insufficient.
  //
  *Results = (EFI_STRING) AllocateZeroPool (MAX_STRING_LENGTH);
  if (*Results == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }

  while (*StringPtr != 0 && StrnCmp (StringPtr, L"GUID=", StrLen (L"GUID=")) == 0) {
    //
    // If parsing error, set Progress to the beginning of the <MultiConfigRequest>
    // or most recent & before the error.
    //
    if (StringPtr == Request) {
      *Progress = StringPtr;
    } else {
      *Progress = StringPtr - 1;
    }

    //
    // Process each <ConfigRequest> of <MultiConfigRequest>
    //
    Length = CalculateConfigStringLen (StringPtr);
    ConfigRequest = AllocateCopyPool ((Length + 1) * sizeof (CHAR16), StringPtr);
    if (ConfigRequest == NULL) {
      return EFI_OUT_OF_RESOURCES;
    }
    *(ConfigRequest + Length) = 0;

    //
    // Get the UEFI device path
    //
    Status = GetDevicePath (ConfigRequest, (UINT8 **) &DevicePath);
    if (EFI_ERROR (Status)) {
      FreePool (ConfigRequest);
      return Status;
    }

    //
    // Find driver handle by device path
    //
    DriverHandle = NULL;
    TempDevicePath = DevicePath;
    Status = gBS->LocateDevicePath (
                    &gEfiDevicePathProtocolGuid,
                    &TempDevicePath,
                    &DriverHandle
                    );
    FreePool (DevicePath);

    if (EFI_ERROR (Status) || (DriverHandle == NULL)) {
      //
      // Cannot find any known driver.
      // Set Progress to the 'G' in "GUID" of the routing header.
      //
      *Progress = StringPtr;
      FreePool (ConfigRequest);
      return EFI_NOT_FOUND;
    }

    //
    // Call corresponding ConfigAccess protocol to extract settings
    //
    Status = gBS->HandleProtocol (
                    DriverHandle,
                    &gEfiHiiConfigAccessProtocolGuid,
                    (VOID **) &ConfigAccess
                    );
    ASSERT_EFI_ERROR (Status);

    Status = ConfigAccess->ExtractConfig (
                             ConfigAccess,
                             ConfigRequest,
                             &AccessProgress,
                             &AccessResults
                             );
    if (EFI_ERROR (Status)) {
      //
      // AccessProgress indicates the parsing progress on <ConfigRequest>.
      // Map it to the progress on <MultiConfigRequest> then return it.
      //
      *Progress = StrStr (StringPtr, AccessProgress);
      FreePool (ConfigRequest);
      return Status;
    }

    //
    // Attach this <ConfigAltResp> to a <MultiConfigAltResp>. There is a '&'
    // which seperates the first <ConfigAltResp> and the following ones.
    //
    ASSERT (*AccessProgress == 0);

    if (!FirstElement) {
      Status = AppendToMultiString (Results, L"&");
      ASSERT_EFI_ERROR (Status);
    }
    
    Status = AppendToMultiString (Results, AccessResults);
    ASSERT_EFI_ERROR (Status);

    FirstElement = FALSE;
    
    FreePool (AccessResults);
    AccessResults = NULL;
    FreePool (ConfigRequest);
    ConfigRequest = NULL;

    //
    // Go to next <ConfigRequest> (skip '&').
    //
    StringPtr += Length;
    if (*StringPtr == 0) {
      *Progress = StringPtr;
      break;
    }

    StringPtr++;

  }

  return EFI_SUCCESS;

}


/**
  This function allows the caller to request the current configuration for the
  entirety of the current HII database and returns the data in a
  null-terminated Unicode string.

  @param  This                   A pointer to the EFI_HII_CONFIG_ROUTING_PROTOCOL
                                 instance.
  @param  Results                Null-terminated Unicode string in
                                 <MultiConfigAltResp> format which has all values
                                 filled in for the names in the Request string.
                                 String to be allocated by the  called function.
                                 De-allocation is up to the caller.

  @retval EFI_SUCCESS            The Results string is filled with the values
                                 corresponding to all requested names.
  @retval EFI_OUT_OF_RESOURCES   Not enough memory to store the parts of the
                                 results that must be stored awaiting possible
                                 future        protocols.
  @retval EFI_INVALID_PARAMETER  For example, passing in a NULL for the Results
                                 parameter would result in this type of error.

**/
EFI_STATUS
EFIAPI
HiiConfigRoutingExportConfig (
  IN  CONST EFI_HII_CONFIG_ROUTING_PROTOCOL  *This,
  OUT EFI_STRING                             *Results
  )
{
  EFI_STATUS                          Status;
  EFI_HII_CONFIG_ACCESS_PROTOCOL      *ConfigAccess;
  EFI_STRING                          AccessResults;   
  UINTN                               Index;
  EFI_HANDLE                          *ConfigAccessHandles;
  UINTN                               NumberConfigAccessHandles;
  BOOLEAN                             FirstElement;

  if (This == NULL || Results == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  //
  // Allocate a fix length of memory to store Results. Reallocate memory for
  // Results if this fix length is insufficient.
  //
  *Results = (EFI_STRING) AllocateZeroPool (MAX_STRING_LENGTH);
  if (*Results == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }

  NumberConfigAccessHandles = 0;
  Status = gBS->LocateHandleBuffer (
             ByProtocol,
             &gEfiHiiConfigAccessProtocolGuid,
             NULL,
             &NumberConfigAccessHandles,
             &ConfigAccessHandles
             );
  if (EFI_ERROR (Status)) {
    return Status;
  }

  FirstElement = TRUE;

  for (Index = 0; Index < NumberConfigAccessHandles; Index++) {
    Status = gBS->HandleProtocol (
                    ConfigAccessHandles[Index],
                    &gEfiHiiConfigAccessProtocolGuid,
                    (VOID **) &ConfigAccess
                    );
    if (EFI_ERROR (Status)) {
      continue;
    }

    Status = ConfigAccess->ExtractConfig (
                             ConfigAccess,
                             NULL,
                             NULL,
                             &AccessResults
                             );
    if (!EFI_ERROR (Status)) {
      //
      // Attach this <ConfigAltResp> to a <MultiConfigAltResp>. There is a '&'
      // which seperates the first <ConfigAltResp> and the following ones.      
      //
      if (!FirstElement) {
        Status = AppendToMultiString (Results, L"&");
        ASSERT_EFI_ERROR (Status);
      }
      
      Status = AppendToMultiString (Results, AccessResults);
      ASSERT_EFI_ERROR (Status);

      FirstElement = FALSE;
      
      FreePool (AccessResults);
      AccessResults = NULL;
    }
  }
  FreePool (ConfigAccessHandles);

  return EFI_SUCCESS;  
}


/**
  This function processes the results of processing forms and routes it to the
  appropriate handlers or storage.

  @param  This                   A pointer to the EFI_HII_CONFIG_ROUTING_PROTOCOL
                                 instance.
  @param  Configuration          A null-terminated Unicode string in
                                 <MulltiConfigResp> format.
  @param  Progress               A pointer to a string filled in with the offset of
                                 the most recent & before the first failing name /
                                 value pair (or the beginning of the string if the
                                 failure is in the first name / value pair) or the
                                 terminating NULL if all was successful.

  @retval EFI_SUCCESS            The results have been distributed or are awaiting
                                 distribution.
  @retval EFI_OUT_OF_RESOURCES   Not enough memory to store the parts of the
                                 results that must be stored awaiting possible
                                 future        protocols.
  @retval EFI_INVALID_PARAMETER  Passing in a NULL for the Configuration parameter
                                 would result in this type of error.
  @retval EFI_NOT_FOUND          Target for the specified routing data was not
                                 found.

**/
EFI_STATUS
EFIAPI
HiiConfigRoutingRouteConfig (
  IN  CONST EFI_HII_CONFIG_ROUTING_PROTOCOL  *This,
  IN  CONST EFI_STRING                       Configuration,
  OUT EFI_STRING                             *Progress
  )
{
  EFI_STRING                          StringPtr;
  EFI_STRING                          ConfigResp;
  UINTN                               Length;
  EFI_STATUS                          Status;
  EFI_DEVICE_PATH_PROTOCOL            *DevicePath;
  EFI_DEVICE_PATH_PROTOCOL            *TempDevicePath;
  EFI_HANDLE                          DriverHandle;
  EFI_HII_CONFIG_ACCESS_PROTOCOL      *ConfigAccess;
  EFI_STRING                          AccessProgress;

  if (This == NULL || Progress == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  if (Configuration == NULL) {
    *Progress = NULL;
    return EFI_INVALID_PARAMETER;
  }

  StringPtr = Configuration;
  *Progress = StringPtr;

  //
  // The first element of <MultiConfigResp> should be
  // <GuidHdr>, which is in 'GUID='<Guid> syntax.
  //
  if (StrnCmp (StringPtr, L"GUID=", StrLen (L"GUID=")) != 0) {
    return EFI_INVALID_PARAMETER;
  }

  while (*StringPtr != 0 && StrnCmp (StringPtr, L"GUID=", StrLen (L"GUID=")) == 0) {
    //
    // If parsing error, set Progress to the beginning of the <MultiConfigResp>
    // or most recent & before the error.
    //
    if (StringPtr == Configuration) {
      *Progress = StringPtr;
    } else {
      *Progress = StringPtr - 1;
    }

    //
    // Process each <ConfigResp> of <MultiConfigResp>
    //
    Length = CalculateConfigStringLen (StringPtr);
    ConfigResp = AllocateCopyPool ((Length + 1) * sizeof (CHAR16), StringPtr);
    if (ConfigResp == NULL) {
      return EFI_OUT_OF_RESOURCES;
    }
    //
    // Append '\0' to the end of ConfigRequest
    //
    *(ConfigResp + Length) = 0;

    //
    // Get the UEFI device path
    //
    Status = GetDevicePath (ConfigResp, (UINT8 **) &DevicePath);
    if (EFI_ERROR (Status)) {
      FreePool (ConfigResp);
      return Status;
    }

    //
    // Find driver handle by device path
    //
    DriverHandle = NULL;
    TempDevicePath = DevicePath;
    Status = gBS->LocateDevicePath (
                    &gEfiDevicePathProtocolGuid,
                    &TempDevicePath,
                    &DriverHandle
                    );
    FreePool (DevicePath);

    if (EFI_ERROR (Status) || (DriverHandle == NULL)) {
      //
      // Cannot find any known driver.
      // Set Progress to the 'G' in "GUID" of the routing header.
      //
      *Progress = StringPtr;
      FreePool (ConfigResp);
      return EFI_NOT_FOUND;
    }

    //
    // Call corresponding ConfigAccess protocol to route settings
    //
    Status = gBS->HandleProtocol (
                    DriverHandle,
                    &gEfiHiiConfigAccessProtocolGuid,
                    (VOID **)  &ConfigAccess
                    );
    ASSERT_EFI_ERROR (Status);

    Status = ConfigAccess->RouteConfig (
                             ConfigAccess,
                             ConfigResp,
                             &AccessProgress
                             );

    if (EFI_ERROR (Status)) {
      //
      // AccessProgress indicates the parsing progress on <ConfigResp>.
      // Map it to the progress on <MultiConfigResp> then return it.
      //
      *Progress = StrStr (StringPtr, AccessProgress);

      FreePool (ConfigResp);
      return Status;
    }

    FreePool (ConfigResp);
    ConfigResp = NULL;

    //
    // Go to next <ConfigResp> (skip '&').
    //
    StringPtr += Length;
    if (*StringPtr == 0) {
      *Progress = StringPtr;
      break;
    }

    StringPtr++;

  }

  return EFI_SUCCESS;
}


/**
  This helper function is to be called by drivers to map configuration data
  stored in byte array ("block") formats such as UEFI Variables into current
  configuration strings.

  @param  This                   A pointer to the EFI_HII_CONFIG_ROUTING_PROTOCOL
                                 instance.
  @param  ConfigRequest          A null-terminated Unicode string in
                                 <ConfigRequest> format.
  @param  Block                  Array of bytes defining the block's configuration.
  @param  BlockSize              Length in bytes of Block.
  @param  Config                 Filled-in configuration string. String allocated
                                 by  the function. Returned only if call is
                                 successful.
  @param  Progress               A pointer to a string filled in with the offset of
                                  the most recent & before the first failing
                                 name/value pair (or the beginning of the string if
                                 the failure is in the first name / value pair) or
                                 the terminating NULL if all was successful.

  @retval EFI_SUCCESS            The request succeeded. Progress points to the null
                                 terminator at the end of the ConfigRequest
                                 string.
  @retval EFI_OUT_OF_RESOURCES   Not enough memory to allocate Config.     Progress
                                 points to the first character of ConfigRequest.
  @retval EFI_INVALID_PARAMETER  Passing in a NULL for the ConfigRequest or
                                 Block parameter would result in this type of
                                 error. Progress points to the first character of
                                 ConfigRequest.
  @retval EFI_DEVICE_ERROR       Block not large enough. Progress undefined.
  @retval EFI_INVALID_PARAMETER  Encountered non <BlockName> formatted string.
                                     Block is left updated and Progress points at
                                 the "&" preceding the first non-<BlockName>.

**/
EFI_STATUS
EFIAPI
HiiBlockToConfig (
  IN  CONST EFI_HII_CONFIG_ROUTING_PROTOCOL  *This,
  IN  CONST EFI_STRING                       ConfigRequest,
  IN  CONST UINT8                            *Block,
  IN  CONST UINTN                            BlockSize,
  OUT EFI_STRING                             *Config,
  OUT EFI_STRING                             *Progress
  )
{
  EFI_STRING                          StringPtr;
  UINTN                               Length;
  EFI_STATUS                          Status;
  EFI_STRING                          TmpPtr;
  UINT8                               *TmpBuffer;
  UINTN                               Offset;
  UINTN                               Width;
  UINT8                               *Value;
  EFI_STRING                          ValueStr;
  EFI_STRING                          ConfigElement;
  UINTN                               Index;
  UINT8                               *TemBuffer;
  CHAR16                              *TemString;

  if (This == NULL || Progress == NULL || Config == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  if (Block == NULL || ConfigRequest == NULL) {
    *Progress = ConfigRequest;
    return EFI_INVALID_PARAMETER;
  }

  StringPtr     = ConfigRequest;
  ValueStr      = NULL;
  Value         = NULL;
  ConfigElement = NULL;

  //
  // Allocate a fix length of memory to store Results. Reallocate memory for
  // Results if this fix length is insufficient.
  //
  *Config = (EFI_STRING) AllocateZeroPool (MAX_STRING_LENGTH);
  if (*Config == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }

  //
  // Jump <ConfigHdr>
  //
  if (StrnCmp (StringPtr, L"GUID=", StrLen (L"GUID=")) != 0) {
    *Progress = StringPtr;
    Status = EFI_INVALID_PARAMETER;
    goto Exit;
  }
  while (*StringPtr != 0 && StrnCmp (StringPtr, L"PATH=", StrLen (L"PATH=")) != 0) {
    StringPtr++;
  }
  if (*StringPtr == 0) {
    *Progress = StringPtr;
    Status = EFI_INVALID_PARAMETER;
    goto Exit;
  }

  while (*StringPtr != L'&' && *StringPtr != 0) {
    StringPtr++;
  }
  if (*StringPtr == 0) {
    *Progress = StringPtr;
    Status = EFI_INVALID_PARAMETER;
    goto Exit;
  }
  //
  // Skip '&'
  //
  StringPtr++;

  //
  // Copy <ConfigHdr> and an additional '&' to <ConfigResp>
  //
  Length = StringPtr - ConfigRequest;
  CopyMem (*Config, ConfigRequest, Length * sizeof (CHAR16));

  //
  // Parse each <RequestElement> if exists
  // Only <BlockName> format is supported by this help function.
  // <BlockName> ::= 'OFFSET='<Number>&'WIDTH='<Number>
  //
  while (*StringPtr != 0 && StrnCmp (StringPtr, L"OFFSET=", StrLen (L"OFFSET=")) == 0) {
    //
    // Back up the header of one <BlockName>
    //
    TmpPtr = StringPtr;

    StringPtr += StrLen (L"OFFSET=");
    //
    // Get Offset
    //
    Status = GetValueOfNumber (StringPtr, &TmpBuffer, &Length);
    if (Status == EFI_OUT_OF_RESOURCES) {
      *Progress = ConfigRequest;
      goto Exit;
    }
    Offset = 0;
    CopyMem (
      &Offset,
      TmpBuffer,
      (((Length + 1) / 2) < sizeof (UINTN)) ? ((Length + 1) / 2) : sizeof (UINTN)
      );
    FreePool (TmpBuffer);

    StringPtr += Length;
    if (StrnCmp (StringPtr, L"&WIDTH=", StrLen (L"&WIDTH=")) != 0) {
      *Progress = StringPtr - Length - StrLen (L"OFFSET=") - 1;
      Status = EFI_INVALID_PARAMETER;
      goto Exit;
    }
    StringPtr += StrLen (L"&WIDTH=");

    //
    // Get Width
    //
    Status = GetValueOfNumber (StringPtr, &TmpBuffer, &Length);
    if (Status == EFI_OUT_OF_RESOURCES) {
      *Progress = ConfigRequest;
      goto Exit;
    }
    Width = 0;
    CopyMem (
      &Width,
      TmpBuffer,
      (((Length + 1) / 2) < sizeof (UINTN)) ? ((Length + 1) / 2) : sizeof (UINTN)
      );
    FreePool (TmpBuffer);

    StringPtr += Length;
    if (*StringPtr != 0 && *StringPtr != L'&') {
      *Progress = StringPtr - Length - StrLen (L"&WIDTH=");
      Status = EFI_INVALID_PARAMETER;
      goto Exit;
    }

    //
    // Calculate Value and convert it to hex string.
    //
    if (Offset + Width > BlockSize) {
      *Progress = StringPtr;
      Status = EFI_DEVICE_ERROR;
      goto Exit;
    }

    Value = (UINT8 *) AllocateZeroPool (Width);
    if (Value == NULL) {
      *Progress = ConfigRequest;
      Status = EFI_OUT_OF_RESOURCES;
      goto Exit;
    }

    CopyMem (Value, (UINT8 *) Block + Offset, Width);

    Length = Width * 2 + 1;
    ValueStr = (EFI_STRING) AllocateZeroPool (Length  * sizeof (CHAR16));
    if (ValueStr == NULL) {
      *Progress = ConfigRequest;
      Status = EFI_OUT_OF_RESOURCES;
      goto Exit;
    }
    
    TemString = ValueStr;
    TemBuffer = Value + Width - 1;
    for (Index = 0; Index < Width; Index ++, TemBuffer --) {
      TemString += UnicodeValueToString (TemString, PREFIX_ZERO | RADIX_HEX, *TemBuffer, 2);
    }
    HiiToLower (ValueStr);

    FreePool (Value);
    Value = NULL;

    //
    // Build a ConfigElement
    //
    Length += StringPtr - TmpPtr + 1 + StrLen (L"VALUE=");
    ConfigElement = (EFI_STRING) AllocateZeroPool (Length * sizeof (CHAR16));
    if (ConfigElement == NULL) {
      Status = EFI_OUT_OF_RESOURCES;
      goto Exit;
    }
    CopyMem (ConfigElement, TmpPtr, (StringPtr - TmpPtr + 1) * sizeof (CHAR16));
    if (*StringPtr == 0) {
      *(ConfigElement + (StringPtr - TmpPtr)) = L'&';
    }
    *(ConfigElement + (StringPtr - TmpPtr) + 1) = 0;
    StrCat (ConfigElement, L"VALUE=");
    StrCat (ConfigElement, ValueStr);

    AppendToMultiString (Config, ConfigElement);

    FreePool (ConfigElement);
    FreePool (ValueStr);
    ConfigElement = NULL;
    ValueStr = NULL;

    //
    // If '\0', parsing is finished. Otherwise skip '&' to continue
    //
    if (*StringPtr == 0) {
      break;
    }
    AppendToMultiString (Config, L"&");
    StringPtr++;

  }

  if (*StringPtr != 0) {
    *Progress = StringPtr - 1;
    Status = EFI_INVALID_PARAMETER;
    goto Exit;
  }

  *Progress = StringPtr;
  return EFI_SUCCESS;

Exit:
  FreePool (*Config);
  if (ValueStr != NULL) {
    FreePool (ValueStr);
  }
  if (Value != NULL) {
    FreePool (Value);
  }
  if (ConfigElement != NULL) {
    FreePool (ConfigElement);
  }

  return Status;

}


/**
  This helper function is to be called by drivers to map configuration strings
  to configurations stored in byte array ("block") formats such as UEFI Variables.

  @param  This                   A pointer to the EFI_HII_CONFIG_ROUTING_PROTOCOL
                                 instance.
  @param  ConfigResp             A null-terminated Unicode string in <ConfigResp>
                                 format.
  @param  Block                  A possibly null array of bytes representing the
                                 current  block. Only bytes referenced in the
                                 ConfigResp string  in the block are modified. If
                                 this parameter is null or if the *BlockSize
                                 parameter is (on input) shorter than required by
                                 the Configuration string, only the BlockSize
                                 parameter is updated and an appropriate status
                                 (see below)  is returned.
  @param  BlockSize              The length of the Block in units of UINT8.  On
                                 input, this is the size of the Block. On output,
                                 if successful, contains the index of the  last
                                 modified byte in the Block.
  @param  Progress               On return, points to an element of the ConfigResp
                                 string filled in with the offset of the most
                                 recent '&' before the first failing name / value
                                 pair (or  the beginning of the string if the
                                 failure is in the  first name / value pair) or the
                                 terminating NULL if all was successful.

  @retval EFI_SUCCESS            The request succeeded. Progress points to the null
                                 terminator at the end of the ConfigResp string.
  @retval EFI_OUT_OF_RESOURCES   Not enough memory to allocate Config.     Progress
                                 points to the first character of ConfigResp.
  @retval EFI_INVALID_PARAMETER  Passing in a NULL for the ConfigResp or
                                 Block parameter would result in this type of
                                 error. Progress points to the first character of
                                         ConfigResp.
  @retval EFI_INVALID_PARAMETER  Encountered non <BlockName> formatted name /
                                 value pair. Block is left updated and
                                 Progress points at the '&' preceding the first
                                 non-<BlockName>.

**/
EFI_STATUS
EFIAPI
HiiConfigToBlock (
  IN     CONST EFI_HII_CONFIG_ROUTING_PROTOCOL *This,
  IN     CONST EFI_STRING                      ConfigResp,
  IN OUT UINT8                                 *Block,
  IN OUT UINTN                                 *BlockSize,
  OUT    EFI_STRING                            *Progress
  )
{
  HII_DATABASE_PRIVATE_DATA           *Private;
  EFI_STRING                          StringPtr;
  UINTN                               Length;
  EFI_STATUS                          Status;
  UINT8                               *TmpBuffer;
  UINTN                               Offset;
  UINTN                               Width;
  UINT8                               *Value;
  UINTN                               BufferSize;

  if (This == NULL || BlockSize == NULL || Progress == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  if (ConfigResp == NULL || Block == NULL) {
    *Progress = ConfigResp;
    return EFI_INVALID_PARAMETER;
  }

  Private = CONFIG_ROUTING_DATABASE_PRIVATE_DATA_FROM_THIS (This);
  ASSERT (Private != NULL);

  StringPtr  = ConfigResp;
  BufferSize = *BlockSize;
  Value      = NULL;

  //
  // Jump <ConfigHdr>
  //
  if (StrnCmp (StringPtr, L"GUID=", StrLen (L"GUID=")) != 0) {
    *Progress = StringPtr;
    Status = EFI_INVALID_PARAMETER;
    goto Exit;
  }
  while (*StringPtr != 0 && StrnCmp (StringPtr, L"PATH=", StrLen (L"PATH=")) != 0) {
    StringPtr++;
  }
  if (*StringPtr == 0) {
    *Progress = StringPtr;
    Status = EFI_INVALID_PARAMETER;
    goto Exit;
  }

  while (*StringPtr != L'&' && *StringPtr != 0) {
    StringPtr++;
  }
  if (*StringPtr == 0) {
    *Progress = StringPtr;
    Status = EFI_INVALID_PARAMETER;
    goto Exit;
  }
  //
  // Skip '&'
  //
  StringPtr++;

  //
  // Parse each <ConfigElement> if exists
  // Only <BlockConfig> format is supported by this help function.
  // <BlockConfig> ::= 'OFFSET='<Number>&'WIDTH='<Number>&'VALUE='<Number>
  //
  while (*StringPtr != 0 && StrnCmp (StringPtr, L"OFFSET=", StrLen (L"OFFSET=")) == 0) {
    StringPtr += StrLen (L"OFFSET=");
    //
    // Get Offset
    //
    Status = GetValueOfNumber (StringPtr, &TmpBuffer, &Length);
    if (EFI_ERROR (Status)) {
      *Progress = ConfigResp;
      goto Exit;
    }
    Offset = 0;
    CopyMem (
      &Offset,
      TmpBuffer,
      (((Length + 1) / 2) < sizeof (UINTN)) ? ((Length + 1) / 2) : sizeof (UINTN)
      );
    FreePool (TmpBuffer);

    StringPtr += Length;
    if (StrnCmp (StringPtr, L"&WIDTH=", StrLen (L"&WIDTH=")) != 0) {
      *Progress = StringPtr - Length - StrLen (L"OFFSET=") - 1;
      Status = EFI_INVALID_PARAMETER;
      goto Exit;
    }
    StringPtr += StrLen (L"&WIDTH=");

    //
    // Get Width
    //
    Status = GetValueOfNumber (StringPtr, &TmpBuffer, &Length);
    if (Status == EFI_OUT_OF_RESOURCES) {
      *Progress = ConfigResp;
      goto Exit;
    }
    Width = 0;
    CopyMem (
      &Width,
      TmpBuffer,
      (((Length + 1) / 2) < sizeof (UINTN)) ? ((Length + 1) / 2) : sizeof (UINTN)
      );
    FreePool (TmpBuffer);

    StringPtr += Length;
    if (StrnCmp (StringPtr, L"&VALUE=", StrLen (L"&VALUE=")) != 0) {
      *Progress = StringPtr - Length - StrLen (L"&WIDTH=");
      Status = EFI_INVALID_PARAMETER;
      goto Exit;
    }
    StringPtr += StrLen (L"&VALUE=");

    //
    // Get Value
    //
    Status = GetValueOfNumber (StringPtr, &Value, &Length);
    if (EFI_ERROR (Status)) {
      *Progress = ConfigResp;
      goto Exit;
    }

    StringPtr += Length;
    if (*StringPtr != 0 && *StringPtr != L'&') {
      *Progress = StringPtr - Length - 7;
      Status = EFI_INVALID_PARAMETER;
      goto Exit;
    }

    //
    // Update the Block with configuration info
    //

    if (Offset + Width > BufferSize) {
      return EFI_DEVICE_ERROR;
    }

    CopyMem (Block + Offset, Value, Width);
    *BlockSize = Offset + Width - 1;

    FreePool (Value);
    Value = NULL;

    //
    // If '\0', parsing is finished. Otherwise skip '&' to continue
    //
    if (*StringPtr == 0) {
      break;
    }

    StringPtr++;
  }

  if (*StringPtr != 0) {
    *Progress = StringPtr - 1;
    Status = EFI_INVALID_PARAMETER;
    goto Exit;
  }

  *Progress = StringPtr;
  return EFI_SUCCESS;

Exit:

  if (Value != NULL) {
    FreePool (Value);
  }
  return Status;
}


/**
  This helper function is to be called by drivers to extract portions of
  a larger configuration string.

  @param  This                   A pointer to the EFI_HII_CONFIG_ROUTING_PROTOCOL
                                 instance.
  @param  Configuration          A null-terminated Unicode string in
                                 <MultiConfigAltResp> format.
  @param  Guid                   A pointer to the GUID value to search for in the
                                 routing portion of the ConfigResp string when
                                 retrieving  the requested data. If Guid is NULL,
                                 then all GUID  values will be searched for.
  @param  Name                   A pointer to the NAME value to search for in the
                                 routing portion of the ConfigResp string when
                                 retrieving  the requested data. If Name is NULL,
                                 then all Name  values will be searched for.
  @param  DevicePath             A pointer to the PATH value to search for in the
                                 routing portion of the ConfigResp string when
                                 retrieving  the requested data. If DevicePath is
                                 NULL, then all  DevicePath values will be searched
                                 for.
  @param  AltCfgId               A pointer to the ALTCFG value to search for in the
                                  routing portion of the ConfigResp string when
                                 retrieving  the requested data.  If this parameter
                                 is NULL,  then the current setting will be
                                 retrieved.
  @param  AltCfgResp             A pointer to a buffer which will be allocated by
                                 the  function which contains the retrieved string
                                 as requested.   This buffer is only allocated if
                                 the call was successful.

  @retval EFI_SUCCESS            The request succeeded. The requested data was
                                 extracted  and placed in the newly allocated
                                 AltCfgResp buffer.
  @retval EFI_OUT_OF_RESOURCES   Not enough memory to allocate AltCfgResp.
  @retval EFI_INVALID_PARAMETER  Any parameter is invalid.
  @retval EFI_NOT_FOUND          Target for the specified routing data was not
                                 found.

**/
EFI_STATUS
EFIAPI
HiiGetAltCfg (
  IN  CONST EFI_HII_CONFIG_ROUTING_PROTOCOL    *This,
  IN  CONST EFI_STRING                         Configuration,
  IN  CONST EFI_GUID                           *Guid,
  IN  CONST EFI_STRING                         Name,
  IN  CONST EFI_DEVICE_PATH_PROTOCOL           *DevicePath,
  IN  CONST UINT16                             *AltCfgId,
  OUT EFI_STRING                               *AltCfgResp
  )
{
  EFI_STATUS                          Status;
  EFI_STRING                          StringPtr;
  EFI_STRING                          HdrStart;
  EFI_STRING                          HdrEnd;
  EFI_STRING                          TmpPtr;
  UINTN                               Length;
  EFI_STRING                          GuidStr;
  EFI_STRING                          NameStr;
  EFI_STRING                          PathStr;
  EFI_STRING                          AltIdStr;
  EFI_STRING                          Result;
  BOOLEAN                             GuidFlag;
  BOOLEAN                             NameFlag;
  BOOLEAN                             PathFlag;

  HdrStart = NULL;
  HdrEnd   = NULL;
  GuidStr  = NULL;
  NameStr  = NULL;
  PathStr  = NULL;
  AltIdStr = NULL;
  Result   = NULL;
  GuidFlag = FALSE;
  NameFlag = FALSE;
  PathFlag = FALSE;

  if (This == NULL || Configuration == NULL || AltCfgResp == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  StringPtr = Configuration;
  if (StrnCmp (StringPtr, L"GUID=", StrLen (L"GUID=")) != 0) {
    return EFI_INVALID_PARAMETER;
  }

  //
  // Generate the sub string for later matching.
  //
  GenerateSubStr (L"GUID=", sizeof (EFI_GUID), (VOID *) Guid, 1, &GuidStr);
  GenerateSubStr (
    L"PATH=",
    GetDevicePathSize ((EFI_DEVICE_PATH_PROTOCOL *) DevicePath),
    (VOID *) DevicePath,
    1,
    &PathStr
    );
  if (AltCfgId != NULL) {
    GenerateSubStr (L"ALTCFG=", sizeof (UINT16), (VOID *) AltCfgId, 3, &AltIdStr);  
  }
  if (Name != NULL) {
    GenerateSubStr (L"NAME=", StrLen (Name) * sizeof (CHAR16), (VOID *) Name, 2, &NameStr);    
  } else {
    GenerateSubStr (L"NAME=", 0, NULL, 2, &NameStr);
  }

  while (*StringPtr != 0) {
    //
    // Try to match the GUID
    //
    if (!GuidFlag) {
      TmpPtr = StrStr (StringPtr, GuidStr);
      if (TmpPtr == NULL) {
        Status = EFI_NOT_FOUND;
        goto Exit;
      }
      HdrStart = TmpPtr;

      //
      // Jump to <NameHdr>
      //
      if (Guid != NULL) {
        StringPtr = TmpPtr + StrLen (GuidStr);
      } else {
        StringPtr = StrStr (TmpPtr, L"NAME=");
        if (StringPtr == NULL) {
          Status = EFI_NOT_FOUND;
          goto Exit;
        }
      }
      GuidFlag = TRUE;
    }

    //
    // Try to match the NAME
    //
    if (GuidFlag && !NameFlag) {
      if (StrnCmp (StringPtr, NameStr, StrLen (NameStr)) != 0) {
        GuidFlag = FALSE;
      } else {
        //
        // Jump to <PathHdr>
        //
        if (Name != NULL) {
          StringPtr += StrLen (NameStr);
        } else {
          StringPtr = StrStr (StringPtr, L"PATH=");
          if (StringPtr == NULL) {
            Status = EFI_NOT_FOUND;
            goto Exit;
          }
        }
        NameFlag = TRUE;
      }
    }

    //
    // Try to match the DevicePath
    //
    if (GuidFlag && NameFlag && !PathFlag) {
      if (StrnCmp (StringPtr, PathStr, StrLen (PathStr)) != 0) {
        GuidFlag = FALSE;
        NameFlag = FALSE;
      } else {
        //
        // Jump to '&' before <DescHdr> or <ConfigBody>
        //
        if (DevicePath != NULL) {
          StringPtr += StrLen (PathStr);
        } else {
          StringPtr = StrStr (StringPtr, L"&");
          if (StringPtr == NULL) {
            Status = EFI_NOT_FOUND;
            goto Exit;
          }
        }
        PathFlag = TRUE;
        HdrEnd   = ++StringPtr;
      }
    }

    //
    // Try to match the AltCfgId
    //
    if (GuidFlag && NameFlag && PathFlag) {
      if (AltCfgId == NULL) {
        //
        // Return Current Setting when AltCfgId is NULL.
        //
        Status = OutputConfigBody (StringPtr, &Result);
        goto Exit;
      }
      //
      // Search the <ConfigAltResp> to get the <AltResp> with AltCfgId.
      //
      if (StrnCmp (StringPtr, AltIdStr, StrLen (AltIdStr)) != 0) {
        GuidFlag = FALSE;
        NameFlag = FALSE;
        PathFlag = FALSE;
      } else {
        Status = OutputConfigBody (StringPtr, &Result);
        goto Exit;
      }
    }
  }

  Status = EFI_NOT_FOUND;

Exit:

  if (!EFI_ERROR (Status) && (Result != NULL)) {
    //
    // Copy the <ConfigHdr> and <ConfigBody>
    //
    Length = HdrEnd - HdrStart + StrLen (Result);
    *AltCfgResp = AllocateZeroPool (Length * sizeof (CHAR16));
    if (*AltCfgResp == NULL) {
      Status = EFI_OUT_OF_RESOURCES;
    } else {
      StrnCpy (*AltCfgResp, HdrStart, HdrEnd - HdrStart);
      StrCat (*AltCfgResp, Result);
      Status = EFI_SUCCESS;
    }
  }

  if (GuidStr != NULL) {
    FreePool (GuidStr);
  }
  if (NameStr != NULL) {
    FreePool (NameStr);
  }
  if (PathStr != NULL) {
    FreePool (PathStr);
  }
  if (AltIdStr != NULL) {
    FreePool (AltIdStr);
  }
  if (Result != NULL) {
    FreePool (Result);
  }

  return Status;

}


