/** @file
PCD Library Class Interface Declarations

Copyright (c) 2006, Intel Corporation                                                         
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution.  The full text of the license may be found at        
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             


Module Name: PcdLib.h

**/

#ifndef __PCD_LIB_H__
#define __PCD_LIB_H__

#include <Common/Pcd.h>


#define PcdToken(TokenName)  _PCD_TOKEN_##TokenName


//
// Feature Flag is in the form of a global constant
//
#define FeaturePcdGet(TokenName)     _gPcd_FixedAtBuild_##TokenName


//
// Fixed is fixed at build time
//
#define FixedPcdGet8(TokenName)     _gPcd_FixedAtBuild_##TokenName
#define FixedPcdGet16(TokenName)    _gPcd_FixedAtBuild_##TokenName
#define FixedPcdGet32(TokenName)    _gPcd_FixedAtBuild_##TokenName
#define FixedPcdGet64(TokenName)    _gPcd_FixedAtBuild_##TokenName
#define FixedPcdGetBool(TokenName)  _gPcd_FixedAtBuild_##TokenName


//
// BugBug: This works for strings, but not constants.
//
#define FixedPcdGetPtr(TokenName)    ((VOID *)_gPcd_FixedAtBuild_##TokenName)


//
// (Binary) Patch is in the form of a global variable
//
#define PatchPcdGet8(TokenName)     _gPcd_BinaryPatch_##TokenName
#define PatchPcdGet16(TokenName)    _gPcd_BinaryPatch_##TokenName
#define PatchPcdGet32(TokenName)    _gPcd_BinaryPatch_##TokenName
#define PatchPcdGet64(TokenName)    _gPcd_BinaryPatch_##TokenName
#define PatchPcdGetBool(TokenName)  _gPcd_BinaryPatch_##TokenName
#define PatchPcdGetPtr(TokenName)   ((VOID *)_gPcd_BinaryPatch_##TokenName)

#define PatchPcdSet8(TokenName, Value)     (_gPcd_BinaryPatch_##TokenName = Value)
#define PatchPcdSet16(TokenName, Value)    (_gPcd_BinaryPatch_##TokenName = Value)
#define PatchPcdSet32(TokenName, Value)    (_gPcd_BinaryPatch_##TokenName = Value)
#define PatchPcdSet64(TokenName, Value)    (_gPcd_BinaryPatch_##TokenName = Value)
#define PatchPcdSetBool(TokenName, Value)  (_gPcd_BinaryPatch_##TokenName = Value)
#define PatchPcdSetPtr(TokenName, Value, Size)   CopyMem (_gPcd_BinaryPatch_##TokenName, Value, Size)

//
// Dynamic is via the protocol with only the TokenNumber as argument
//  It can also be Patch or Fixed type based on a build option
//
#define PcdGet8(TokenName)          _PCD_MODE_8_##TokenName
#define PcdGet16(TokenName)         _PCD_MODE_16_##TokenName
#define PcdGet32(TokenName)         _PCD_MODE_32_##TokenName
#define PcdGet64(TokenName)         _PCD_MODE_64_##TokenName
#define PcdGetPtr(TokenName)        _PCD_MODE_PTR_##TokenName
#define PcdGetBool(TokenName)       _PCD_MODE_BOOL_##TokenName


//
// Dynamic Ex is to support binary distribution
//
#define PcdGetEx8(Guid, TokenName)     LibPcdGetEx8 (Guid, _PCD_TOKEN_##TokenName)
#define PcdGetEx16(Guid, TokenName)    LibPcdGetEx16 (Guid, _PCD_TOKEN_##TokenName)
#define PcdGetEx32(Guid, TokenName)    LibPcdGetEx32 (Guid, _PCD_TOKEN_##TokenName)
#define PcdGetEx64(Guid, TokenName)    LibPcdGetEx64 (Guid, _PCD_TOKEN_##TokenName)
#define PcdGetExPtr(Guid, TokenName)   LibPcdGetExPtr (Guid, _PCD_TOKEN_##TokenName)
#define PcdGetExBool(Guid, TokenName)  LibPcdGetExBool (Guid, _PCD_TOKEN_##TokenName)


//
// Dynamic Set
//
#define PcdSet8(TokenName, Value)       LibPcdSet8   (_PCD_TOKEN_##TokenName, Value)
#define PcdSet16(TokenName, Value)      LibPcdSet16  (_PCD_TOKEN_##TokenName, Value)
#define PcdSet32(TokenName, Value)      LibPcdSet32  (_PCD_TOKEN_##TokenName, Value)
#define PcdSet64(TokenName, Value)      LibPcdSet64  (_PCD_TOKEN_##TokenName, Value)
#define PcdSetPtr(TokenName, Value)     LibPcdSetPtr (_PCD_TOKEN_##TokenName, Value)
#define PcdSetBool(TokenName, Value)    LibPcdSetBool(_PCD_TOKEN_##TokenName, Value)


//
// Dynamic Set Ex
//
#define PcdSetEx8(Guid, TokenName, Value)      LibPcdSetEx8   (Guid, _PCD_TOKEN_##TokenName, Value)
#define PcdSetEx16(Guid, TokenName, Value)     LibPcdSetEx16  (Guid, _PCD_TOKEN_##TokenName, Value)
#define PcdSetEx32(Guid, TokenName, Value)     LibPcdSetEx32  (Guid, _PCD_TOKEN_##TokenName, Value)
#define PcdSetEx64(Guid, TokenName, Value)     LibPcdSetEx64  (Guid, _PCD_TOKEN_##TokenName, Value)
#define PcdSetExPtr(Guid, TokenName, Value)    LibPcdSetExPtr (Guid, _PCD_TOKEN_##TokenName, Value)
#define PcdSetExBool(Guid, TokenName, Value)   LibPcdSetExBool(Guid, _PCD_TOKEN_##TokenName, Value)


/**
  Sets the current SKU in the PCD database to the value specified by SkuId.  SkuId is returned.

  @param[in]  SkuId The SKU value that will be used when the PCD service will retrieve and 
              set values associated with a PCD token.

  @retval SKU_ID Return the SKU ID that just be set.

**/
SKU_ID
EFIAPI
LibPcdSetSku (
  IN SKU_ID   SkuId
  );


/**
  Returns the 8-bit value for the token specified by TokenNumber. 

  @param[in]  The PCD token number to retrieve a current value for.

  @retval UINT8 Returns the 8-bit value for the token specified by TokenNumber. 

**/
UINT8
EFIAPI
LibPcdGet8 (
  IN PCD_TOKEN_NUMBER  TokenNumber
  );


/**
  Returns the 16-bit value for the token specified by TokenNumber. 

  @param[in]  The PCD token number to retrieve a current value for.

  @retval UINT16 Returns the 16-bit value for the token specified by TokenNumber. 

**/
UINT16
EFIAPI
LibPcdGet16 (
  IN PCD_TOKEN_NUMBER  TokenNumber
  );


/**
  Returns the 32-bit value for the token specified by TokenNumber. 

  @param[in]  TokenNumber The PCD token number to retrieve a current value for.

  @retval UINT32 Returns the 32-bit value for the token specified by TokenNumber.

**/
UINT32
EFIAPI
LibPcdGet32 (
  IN PCD_TOKEN_NUMBER  TokenNumber
  );


/**
  Returns the 64-bit value for the token specified by TokenNumber.

  @param[in]  TokenNumber The PCD token number to retrieve a current value for.

  @retval UINT64 Returns the 64-bit value for the token specified by TokenNumber.

**/
UINT64
EFIAPI
LibPcdGet64 (
  IN PCD_TOKEN_NUMBER  TokenNumber
  );


/**
  Returns the pointer to the buffer of the token specified by TokenNumber.

  @param[in]  TokenNumber The PCD token number to retrieve a current value for.

  @retval VOID* Returns the pointer to the token specified by TokenNumber.

**/
VOID *
EFIAPI
LibPcdGetPtr (
  IN PCD_TOKEN_NUMBER  TokenNumber
  );


/**
  Returns the Boolean value of the token specified by TokenNumber. 

  @param[in]  TokenNumber The PCD token number to retrieve a current value for.

  @retval BOOLEAN Returns the Boolean value of the token specified by TokenNumber. 

**/
BOOLEAN 
EFIAPI
LibPcdGetBool (
  IN PCD_TOKEN_NUMBER  TokenNumber
  );


/**
  Returns the size of the token specified by TokenNumber. 

  @param[in]  TokenNumber The PCD token number to retrieve a current value for.

  @retval UINTN Returns the size of the token specified by TokenNumber. 

**/
UINTN
EFIAPI
LibPcdGetSize (
  IN PCD_TOKEN_NUMBER  TokenNumber
  );


/**
  Returns the 8-bit value for the token specified by TokenNumber and Guid.
  If Guid is NULL, then ASSERT(). 

  @param[in]  Guid Pointer to a 128-bit unique value that designates 
              which namespace to retrieve a value from.
  @param[in]  TokenNumber The PCD token number to retrieve a current value for.

  @retval UINT8 Return the UINT8.

**/
UINT8
EFIAPI
LibPcdGetEx8 (
  IN CONST GUID        *Guid,
  IN PCD_TOKEN_NUMBER  TokenNumber
  );


/**
  Returns the 16-bit value for the token specified by TokenNumber and Guid.
  If Guid is NULL, then ASSERT(). 

  @param[in]  Guid Pointer to a 128-bit unique value that designates 
              which namespace to retrieve a value from.
  @param[in]  TokenNumber The PCD token number to retrieve a current value for.

  @retval UINT16 Return the UINT16.

**/
UINT16
EFIAPI
LibPcdGetEx16 (
  IN CONST GUID        *Guid,
  IN PCD_TOKEN_NUMBER  TokenNumber
  );


/**
  Returns the 32-bit value for the token specified by TokenNumber and Guid.
  If Guid is NULL, then ASSERT(). 

  @param[in]  Guid Pointer to a 128-bit unique value that designates 
              which namespace to retrieve a value from.
  @param[in]  TokenNumber The PCD token number to retrieve a current value for.

  @retval UINT32 Return the UINT32.

**/
UINT32
EFIAPI
LibPcdGetEx32 (
  IN CONST GUID        *Guid,
  IN PCD_TOKEN_NUMBER  TokenNumber
  );


/**
  Returns the 64-bit value for the token specified by TokenNumber and Guid.
  If Guid is NULL, then ASSERT(). 

  @param[in]  Guid Pointer to a 128-bit unique value that designates 
              which namespace to retrieve a value from.
  @param[in]  TokenNumber The PCD token number to retrieve a current value for.

  @retval UINT64 Return the UINT64.

**/
UINT64
EFIAPI
LibPcdGetEx64 (
  IN CONST GUID        *Guid,
  IN PCD_TOKEN_NUMBER  TokenNumber
  );


/**
  Returns the pointer to the buffer of token specified by TokenNumber and Guid.
  If Guid is NULL, then ASSERT(). 

  @param[in]  Guid Pointer to a 128-bit unique value that designates 
              which namespace to retrieve a value from.
  @param[in]  TokenNumber The PCD token number to retrieve a current value for.

  @retval VOID* Return the VOID* pointer.

**/
VOID *
EFIAPI
LibPcdGetExPtr (
  IN CONST GUID        *Guid,
  IN PCD_TOKEN_NUMBER  TokenNumber
  );


/**
  Returns the Boolean value of the token specified by TokenNumber and Guid. 
  If Guid is NULL, then ASSERT(). 

  @param[in]  Guid Pointer to a 128-bit unique value that designates 
              which namespace to retrieve a value from.
  @param[in]  TokenNumber The PCD token number to retrieve a current value for.

  @retval BOOLEAN Return the BOOLEAN.

**/
BOOLEAN
EFIAPI
LibPcdGetExBool (
  IN CONST GUID        *Guid,
  IN PCD_TOKEN_NUMBER  TokenNumber
  );


/**
  Returns the size of the token specified by TokenNumber and Guid. 
  If Guid is NULL, then ASSERT(). 

  @param[in]  Guid Pointer to a 128-bit unique value that designates 
              which namespace to retrieve a value from.
  @param[in]  TokenNumber The PCD token number to retrieve a current value for.

  @retval UINTN Return the size.

**/
UINTN
EFIAPI
LibPcdGetExSize (
  IN CONST GUID        *Guid,
  IN PCD_TOKEN_NUMBER  TokenNumber
  );


/**
  Sets the 8-bit value for the token specified by TokenNumber 
  to the value specified by Value.  Value is returned.
  
  @param[in]  TokenNumber The PCD token number to set a current value for.
  @param[in]  Value The 8-bit value to set.

  @retval UINT8 Return the value been set.

**/
UINT8
EFIAPI
LibPcdSet8 (
  IN PCD_TOKEN_NUMBER  TokenNumber,
  IN UINT8             Value
  );


/**
  Sets the 16-bit value for the token specified by TokenNumber 
  to the value specified by Value.  Value is returned.
  
  @param[in]  TokenNumber The PCD token number to set a current value for.
  @param[in]  Value The 16-bit value to set.

  @retval UINT16 Return the value been set.

**/
UINT16
EFIAPI
LibPcdSet16 (
  IN PCD_TOKEN_NUMBER  TokenNumber,
  IN UINT16            Value
  );


/**
  Sets the 32-bit value for the token specified by TokenNumber 
  to the value specified by Value.  Value is returned.
  
  @param[in]  TokenNumber The PCD token number to set a current value for.
  @param[in]  Value The 32-bit value to set.

  @retval UINT32 Return the value been set.

**/
UINT32
EFIAPI
LibPcdSet32 (
  IN PCD_TOKEN_NUMBER  TokenNumber,
  IN UINT32            Value
  );


/**
  Sets the 64-bit value for the token specified by TokenNumber 
  to the value specified by Value.  Value is returned.
  
  @param[in]  TokenNumber The PCD token number to set a current value for.
  @param[in]  Value The 64-bit value to set.

  @retval UINT64 Return the value been set.

**/
UINT64
EFIAPI
LibPcdSet64 (
  IN PCD_TOKEN_NUMBER  TokenNumber,
  IN UINT64            Value
  );


/**
  Sets a buffer for the token specified by TokenNumber to 
  the value specified by Value.     Value is returned.
  If Value is NULL, then ASSERT().
  
  @param[in]  TokenNumber The PCD token number to set a current value for.
  @param[in]  Value A pointer to the buffer to set.

  @retval VOID* Return the pointer for the buffer been set.

**/
VOID*
EFIAPI
LibPcdSetPtr (
  IN PCD_TOKEN_NUMBER  TokenNumber,
  IN UINTN             SizeOfBuffer,
  IN CONST VOID        *Value
  );


/**
  Sets the Boolean value for the token specified by TokenNumber 
  to the value specified by Value.  Value is returned.
  
  @param[in]  TokenNumber The PCD token number to set a current value for.
  @param[in]  Value The boolean value to set.

  @retval BOOLEAN Return the value been set.

**/
BOOLEAN
EFIAPI
LibPcdSetBool (
  IN PCD_TOKEN_NUMBER  TokenNumber,
  IN BOOLEAN           Value
  );


/**
  Sets the 8-bit value for the token specified by TokenNumber and 
  Guid to the value specified by Value. Value is returned.
  If Guid is NULL, then ASSERT().
  
  @param[in]  Guid Pointer to a 128-bit unique value that 
              designates which namespace to set a value from.
  @param[in]  TokenNumber The PCD token number to set a current value for.
  @param[in]  Value The 8-bit value to set.

  @retval UINT8 Return the value been set.

**/
UINT8
EFIAPI
LibPcdSetEx8 (
  IN CONST GUID        *Guid,
  IN PCD_TOKEN_NUMBER  TokenNumber,
  IN UINT8             Value
  );


/**
  Sets the 16-bit value for the token specified by TokenNumber and 
  Guid to the value specified by Value. Value is returned.
  If Guid is NULL, then ASSERT().
  
  @param[in]  Guid Pointer to a 128-bit unique value that 
              designates which namespace to set a value from.
  @param[in]  TokenNumber The PCD token number to set a current value for.
  @param[in]  Value The 16-bit value to set.

  @retval UINT8 Return the value been set.

**/
UINT16
EFIAPI
LibPcdSetEx16 (
  IN CONST GUID        *Guid,
  IN PCD_TOKEN_NUMBER  TokenNumber,
  IN UINT16            Value
  );


/**
  Sets the 32-bit value for the token specified by TokenNumber and 
  Guid to the value specified by Value. Value is returned.
  If Guid is NULL, then ASSERT().
  
  @param[in]  Guid Pointer to a 128-bit unique value that 
              designates which namespace to set a value from.
  @param[in]  TokenNumber The PCD token number to set a current value for.
  @param[in]  Value The 32-bit value to set.

  @retval UINT32 Return the value been set.

**/
UINT32
EFIAPI
LibPcdSetEx32 (
  IN CONST GUID        *Guid,
  IN PCD_TOKEN_NUMBER  TokenNumber,
  IN UINT32            Value
  );


/**
  Sets the 64-bit value for the token specified by TokenNumber and 
  Guid to the value specified by Value. Value is returned.
  If Guid is NULL, then ASSERT().
  
  @param[in]  Guid Pointer to a 128-bit unique value that 
              designates which namespace to set a value from.
  @param[in]  TokenNumber The PCD token number to set a current value for.
  @param[in]  Value The 64-bit value to set.

  @retval UINT64 Return the value been set.

**/
UINT64
EFIAPI
LibPcdSetEx64 (
  IN CONST GUID        *Guid,
  IN PCD_TOKEN_NUMBER  TokenNumber,
  IN UINT64            Value
  );


/**
  Sets a buffer for the token specified by TokenNumber and 
  Guid to the value specified by Value. Value is returned.
  If Guid is NULL, then ASSERT().
  If Value is NULL, then ASSERT().
  
  @param[in]  Guid Pointer to a 128-bit unique value that 
              designates which namespace to set a value from.
  @param[in]  TokenNumber The PCD token number to set a current value for.
  @param[in]  Value The 8-bit value to set.

  @retval VOID * Return the value been set.

**/
VOID *
EFIAPI
LibPcdSetExPtr (
  IN CONST GUID        *Guid,
  IN PCD_TOKEN_NUMBER  TokenNumber,
  IN UINTN             SizeOfBuffer,
  IN CONST VOID        *Value
  );


/**
  Sets the Boolean value for the token specified by TokenNumber and 
  Guid to the value specified by Value. Value is returned.
  If Guid is NULL, then ASSERT().
  
  @param[in]  Guid Pointer to a 128-bit unique value that 
              designates which namespace to set a value from.
  @param[in]  TokenNumber The PCD token number to set a current value for.
  @param[in]  Value The Boolean value to set.

  @retval Boolean Return the value been set.

**/
BOOLEAN
EFIAPI
LibPcdSetExBool (
  IN CONST GUID        *Guid,
  IN PCD_TOKEN_NUMBER  TokenNumber,
  IN BOOLEAN           Value
  );


/**
  When the token specified by TokenNumber and Guid is set, 
  then notification function specified by NotificationFunction is called.  
  If Guid is NULL, then the default token space is used. 
  If NotificationFunction is NULL, then ASSERT().

  @param[in]  CallBackGuid The PCD token GUID being set.
  @param[in]  CallBackToken The PCD token number being set.
  @param[in]  TokenData A pointer to the token data being set.
  @param[in]  TokenDataSize The size, in bytes, of the data being set.

  @retval VOID

**/
typedef
VOID
(EFIAPI *PCD_CALLBACK) (
  IN        CONST GUID        *CallBackGuid, OPTIONAL
  IN        PCD_TOKEN_NUMBER  CallBackToken,
  IN  OUT   VOID              *TokenData,
  IN        UINTN             TokenDataSize
  );


/**
  When the token specified by TokenNumber and Guid is set, 
  then notification function specified by NotificationFunction is called.  
  If Guid is NULL, then the default token space is used. 
  If NotificationFunction is NULL, then ASSERT().

  @param[in]  Guid Pointer to a 128-bit unique value that designates which 
              namespace to set a value from.  If NULL, then the default 
              token space is used.
  @param[in]  TokenNumber The PCD token number to monitor.
  @param[in]  NotificationFunction The function to call when the token 
              specified by Guid and TokenNumber is set.

  @retval VOID

**/
VOID
EFIAPI
LibPcdCallbackOnSet (
  IN CONST GUID               *Guid,       OPTIONAL
  IN PCD_TOKEN_NUMBER         TokenNumber,
  IN PCD_CALLBACK             NotificationFunction
  );


/**
  Disable a notification function that was established with LibPcdCallbackonSet().

  @param[in]  Guid Specify the GUID token space.
  @param[in]  TokenNumber Specify the token number.
  @param[in]  NotificationFunction The callback function to be unregistered.

  @retval VOID

**/
VOID
EFIAPI
LibPcdCancelCallback (
  IN CONST GUID               *Guid,       OPTIONAL
  IN PCD_TOKEN_NUMBER         TokenNumber,
  IN PCD_CALLBACK             NotificationFunction
  );


/**
  Retrieves the next PCD token number from the token space specified by Guid.  
  If Guid is NULL, then the default token space is used.  If TokenNumber is 0, 
  then the first token number is returned.  Otherwise, the token number that 
  follows TokenNumber in the token space is returned.  If TokenNumber is the last 
  token number in the token space, then 0 is returned.  If TokenNumber is not 0 and 
  is not in the token space specified by Guid, then ASSERT().

  @param[in]  Pointer to a 128-bit unique value that designates which namespace 
              to set a value from.  If NULL, then the default token space is used.
  @param[in]  The previous PCD token number.  If 0, then retrieves the first PCD 
              token number.

  @retval PCD_TOKEN_NUMBER The next valid token number.

**/
PCD_TOKEN_NUMBER
EFIAPI
LibPcdGetNextToken (
  IN CONST GUID               *Guid,       OPTIONAL
  IN PCD_TOKEN_NUMBER         TokenNumber
  );

#endif
