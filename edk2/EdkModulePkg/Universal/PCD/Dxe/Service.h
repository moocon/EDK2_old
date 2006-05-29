/** @file
Private functions used by PCD DXE driver.

Copyright (c) 2006, Intel Corporation                                                         
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution.  The full text of the license may be found at        
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             


Module Name: Service.h

**/

#ifndef _SERVICE_H
#define _SERVICE_H

//
// Please make sure the PCD Serivce PEIM Version is consistent with
// the version of PCD Database generation tool
//
#define PCD_DXE_SERVICE_DRIVER_VERSION      1

//
// PCD_DXE_DATABASE_GENTOOL_VERSION is defined in Autogen.h
// and generated by PCD Database generation tool.
//
#if (PCD_DXE_SERVICE_PEIM_VERSION != PCD_DXE_DATABASE_GENTOOL_VERSION)
  #error "Please make sure the version of PCD Service DXE Driver and PCD DXE Database Generation Tool matches"
#endif


typedef struct {
  LIST_ENTRY              Node;
  PCD_PROTOCOL_CALLBACK   CallbackFn;
} CALLBACK_FN_ENTRY;

#define CR_FNENTRY_FROM_LISTNODE(Record, Type, Field) _CR(Record, Type, Field)

//
// Internal Functions
//

EFI_STATUS
SetWorker (
  IN PCD_TOKEN_NUMBER          TokenNumber,
  IN VOID                      *Data,
  IN UINTN                     Size,
  IN BOOLEAN                   PtrType
  )
;

EFI_STATUS
ExSetWorker (
  IN PCD_TOKEN_NUMBER     ExTokenNumber,
  IN CONST EFI_GUID       *Guid,
  VOID                    *Data,
  UINTN                   Size,
  BOOLEAN                 PtrType
  )
;


VOID *
GetWorker (
  PCD_TOKEN_NUMBER  TokenNumber
  )
;

VOID *
ExGetWorker (
  IN CONST EFI_GUID         *Guid,
  IN PCD_TOKEN_NUMBER       ExTokenNumber,
  IN UINTN                  GetSize
  ) 
;

UINT32
GetSkuEnabledTokenNumber (
  UINT32 LocalTokenNumber,
  UINTN  Size,
  BOOLEAN IsPeiDb
  ) 
;

EFI_STATUS
GetHiiVariable (
  IN  EFI_GUID      *VariableGuid,
  IN  UINT16        *VariableName,
  OUT VOID          **VariableData,
  OUT UINTN         *VariableSize
  )
;

EFI_STATUS
DxeRegisterCallBackWorker (
  IN  PCD_TOKEN_NUMBER        TokenNumber,
  IN  CONST EFI_GUID          *Guid, OPTIONAL
  IN  PCD_PROTOCOL_CALLBACK   CallBackFunction
);

EFI_STATUS
DxeUnRegisterCallBackWorker (
  IN  PCD_TOKEN_NUMBER        TokenNumber,
  IN  CONST EFI_GUID          *Guid, OPTIONAL
  IN  PCD_PROTOCOL_CALLBACK   CallBackFunction
);

VOID
BuildPcdDxeDataBase (
  VOID
);


typedef struct {
  UINTN   TokenNumber;
  UINTN   Size;
  UINT32  LocalTokenNumberAlias;
  BOOLEAN IsPeiDb;
} EX_PCD_ENTRY_ATTRIBUTE;

VOID
GetExPcdTokenAttributes (
  IN CONST EFI_GUID             *Guid,
  IN PCD_TOKEN_NUMBER           ExTokenNumber,
  OUT EX_PCD_ENTRY_ATTRIBUTE    *ExAttr
  )
;

//
// Protocol Interface function declaration.
//
VOID
EFIAPI
DxePcdSetSku (
  IN  SKU_ID                  SkuId
  )
;


UINT8
EFIAPI
DxePcdGet8 (
  IN PCD_TOKEN_NUMBER  TokenNumber
  )
;


UINT16
EFIAPI
DxePcdGet16 (
  IN PCD_TOKEN_NUMBER  TokenNumber
  )
;


UINT32
EFIAPI
DxePcdGet32 (
  IN PCD_TOKEN_NUMBER  TokenNumber
  )
;


UINT64
EFIAPI
DxePcdGet64 (
  IN PCD_TOKEN_NUMBER  TokenNumber
  )
;


VOID *
EFIAPI
DxePcdGetPtr (
  IN PCD_TOKEN_NUMBER  TokenNumber
  )
;


BOOLEAN
EFIAPI
DxePcdGetBool (
  IN PCD_TOKEN_NUMBER  TokenNumber
  )
;


UINTN
EFIAPI
DxePcdGetSize (
  IN PCD_TOKEN_NUMBER  TokenNumber
  )
;


UINT8
EFIAPI
DxePcdGet8Ex (
  IN CONST EFI_GUID        *Guid,
  IN PCD_TOKEN_NUMBER  TokenNumber
  )
;


UINT16
EFIAPI
DxePcdGet16Ex (
  IN CONST EFI_GUID        *Guid,
  IN PCD_TOKEN_NUMBER  TokenNumber
  )
;


UINT32
EFIAPI
DxePcdGet32Ex (
  IN CONST EFI_GUID        *Guid,
  IN PCD_TOKEN_NUMBER  TokenNumber
  )
;



UINT64
EFIAPI
DxePcdGet64Ex (
  IN CONST EFI_GUID        *Guid,
  IN PCD_TOKEN_NUMBER  TokenNumber
  )
;



VOID *
EFIAPI
DxePcdGetPtrEx (
  IN CONST EFI_GUID        *Guid,
  IN PCD_TOKEN_NUMBER  TokenNumber
  )
;


BOOLEAN
EFIAPI
DxePcdGetBoolEx (
  IN CONST EFI_GUID        *Guid,
  IN PCD_TOKEN_NUMBER  TokenNumber
  )
;


UINTN
EFIAPI
DxePcdGetSizeEx (
  IN CONST EFI_GUID        *Guid,
  IN PCD_TOKEN_NUMBER  TokenNumber
  )
;


EFI_STATUS
EFIAPI
DxePcdSet8 (
  IN PCD_TOKEN_NUMBER  TokenNumber,
  IN UINT8             Value
  )
;


EFI_STATUS
EFIAPI
DxePcdSet16 (
  IN PCD_TOKEN_NUMBER  TokenNumber,
  IN UINT16             Value
  )
;


EFI_STATUS
EFIAPI
DxePcdSet32 (
  IN PCD_TOKEN_NUMBER  TokenNumber,
  IN UINT32             Value
  )
;


EFI_STATUS
EFIAPI
DxePcdSet64 (
  IN PCD_TOKEN_NUMBER  TokenNumber,
  IN UINT64            Value
  )
;


EFI_STATUS
EFIAPI
DxePcdSetPtr (
  IN PCD_TOKEN_NUMBER  TokenNumber,
  IN UINTN             SizeOfBuffer,
  IN VOID              *Buffer
  )
;


EFI_STATUS
EFIAPI
DxePcdSetBool (
  IN PCD_TOKEN_NUMBER  TokenNumber,
  IN BOOLEAN           Value
  )
;


EFI_STATUS
EFIAPI
DxePcdSet8Ex (
  IN CONST EFI_GUID        *Guid,
  IN PCD_TOKEN_NUMBER  TokenNumber,
  IN UINT8             Value
  )
;


EFI_STATUS
EFIAPI
DxePcdSet16Ex (
  IN CONST EFI_GUID        *Guid,
  IN PCD_TOKEN_NUMBER  TokenNumber,
  IN UINT16            Value
  )
;


EFI_STATUS
EFIAPI
DxePcdSet32Ex (
  IN CONST EFI_GUID        *Guid,
  IN PCD_TOKEN_NUMBER  TokenNumber,
  IN UINT32             Value
  )
;


EFI_STATUS
EFIAPI
DxePcdSet64Ex (
  IN CONST EFI_GUID        *Guid,
  IN PCD_TOKEN_NUMBER  TokenNumber,
  IN UINT64            Value
  )
;


EFI_STATUS
EFIAPI
DxePcdSetPtrEx (
  IN CONST EFI_GUID    *Guid,
  IN PCD_TOKEN_NUMBER  TokenNumber,
  IN UINTN             SizeOfBuffer,
  IN VOID              *Buffer
  )
;


EFI_STATUS
EFIAPI
DxePcdSetBoolEx (
  IN CONST EFI_GUID    *Guid,
  IN PCD_TOKEN_NUMBER  TokenNumber,
  IN BOOLEAN           Value
  )
;



EFI_STATUS
EFIAPI
DxeRegisterCallBackOnSet (
  IN  PCD_TOKEN_NUMBER        TokenNumber,
  IN  CONST EFI_GUID          *Guid, OPTIONAL
  IN  PCD_PROTOCOL_CALLBACK   CallBackFunction
  )
;


EFI_STATUS
EFIAPI
DxeUnRegisterCallBackOnSet (
  IN  PCD_TOKEN_NUMBER        TokenNumber,
  IN  CONST EFI_GUID          *Guid, OPTIONAL
  IN  PCD_PROTOCOL_CALLBACK   CallBackFunction
  )
;


EFI_STATUS
EFIAPI
DxePcdGetNextToken (
  IN CONST EFI_GUID               *Guid, OPTIONAL
  IN OUT   PCD_TOKEN_NUMBER       *TokenNumber
  )
;

EFI_STATUS
SetWorkerByLocalTokenNumber (
  UINT32        LocalTokenNumber,
  VOID          *Data,
  UINTN         Size,
  BOOLEAN       PtrType,
  BOOLEAN       IsPeiDb
  )
;

PCD_TOKEN_NUMBER
ExGetNextTokeNumber (
  IN CONST EFI_GUID    *Guid,
  IN PCD_TOKEN_NUMBER  TokenNumber,
  IN EFI_GUID          *GuidTable,
  IN UINTN             SizeOfGuidTable,
  IN DYNAMICEX_MAPPING *ExMapTable,
  IN UINTN             SizeOfExMapTable
  )
;

extern EFI_GUID gPcdDataBaseHobGuid;

extern PCD_DATABASE * mPcdDatabase;

extern DXE_PCD_DATABASE_INIT gDXEPcdDbInit;

#endif
