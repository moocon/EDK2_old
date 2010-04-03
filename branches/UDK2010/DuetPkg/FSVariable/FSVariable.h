/*++

Copyright (c) 2006 - 2010, Intel Corporation                                                         
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution.  The full text of the license may be found at        
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module Name:

  FSVariable.h
  
Abstract:

--*/

#ifndef _FS_VARIABLE_H
#define _FS_VARIABLE_H

//
// Statements that include other header files
//
#include <PiDxe.h>

#include <Library/BaseLib.h>
#include <Library/PcdLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiRuntimeLib.h>
#include <Library/DebugLib.h>
#include <Library/UefiLib.h>
#include <Library/HobLib.h>
#include <Library/DxeServicesTableLib.h>
#include <Library/DevicePathLib.h>

#include <Guid/HobList.h>
#include <Guid/FlashMapHob.h>
#include <Guid/VariableFormat.h>
#include <Guid/GlobalVariable.h>
#include <Protocol/Variable.h>
#include <Protocol/VariableWrite.h>
#include <Protocol/SimpleFileSystem.h>
#include <Protocol/BlockIo.h>


#include "EfiFlashMap.h"
#include "VariableStorage.h"

#define VOLATILE_VARIABLE_STORE_SIZE  FixedPcdGet32(PcdVariableStoreSize)
#define VARIABLE_SCRATCH_SIZE         MAX(FixedPcdGet32(PcdMaxVariableSize), FixedPcdGet32(PcdMaxHardwareErrorVariableSize))
#define VARIABLE_RECLAIM_THRESHOLD    (1024)
///
/// The size of a 3 character ISO639 language code.
///
#define ISO_639_2_ENTRY_SIZE    3

#define GET_VARIABLE_NAME_PTR(a)  (CHAR16 *) ((UINTN) (a) + sizeof (VARIABLE_HEADER))

typedef enum {
  Physical,
  Virtual
} VARIABLE_POINTER_TYPE;

typedef enum {
  NonVolatile,
  Volatile,
  MaxType
} VARIABLE_STORAGE_TYPE;

typedef struct {
  VARIABLE_HEADER         *CurrPtr;
  VARIABLE_HEADER         *EndPtr;
  VARIABLE_HEADER         *StartPtr;
  VARIABLE_STORAGE_TYPE   Type;
} VARIABLE_POINTER_TRACK;

#define VARIABLE_MEMBER_OFFSET(Member, StartOffset) \
        ( sizeof (VARIABLE_STORE_HEADER) + (StartOffset) + \
          (UINTN) ((UINT8 *) &((VARIABLE_HEADER*) 0)->Member - (UINT8 *) &((VARIABLE_HEADER*) 0)->StartId) \
        )


typedef struct {
  EFI_EVENT_NOTIFY   GoVirtualChildEvent[MaxType];
  VARIABLE_STORAGE   *VariableStore[MaxType];       // Instance of VariableStorage
  VOID               *VariableBase[MaxType];        // Start address of variable storage
  UINTN              LastVariableOffset[MaxType];   // The position to write new variable to (index from VariableBase)
  VOID               *Scratch;                      // Buffer used during reclaim
  UINTN              CommonVariableTotalSize;
  UINTN              HwErrVariableTotalSize;
  CHAR8              PlatformLangCodes[256]; //Pre-allocate 256 bytes space to accommodate the PlatformlangCodes.
  CHAR8              LangCodes[256];         //Pre-allocate 256 bytes space to accommodate the langCodes.
  CHAR8              PlatformLang[8];        //Pre-allocate 8 bytes space to accommodate the Platformlang variable.
  CHAR8              Lang[4];                //Pre-allocate 4 bytes space to accommodate the lang variable.
} VARIABLE_GLOBAL;

//
// Functions
//

EFI_STATUS
EFIAPI
VariableServiceInitialize (
  IN EFI_HANDLE         ImageHandle,
  IN EFI_SYSTEM_TABLE   *SystemTable
  );

VOID
EFIAPI
VariableClassAddressChangeEvent (
  IN EFI_EVENT        Event,
  IN VOID             *Context
  );

EFI_STATUS
EFIAPI
DuetGetVariable (
  IN      CHAR16            *VariableName,
  IN      EFI_GUID          *VendorGuid,
  OUT     UINT32            *Attributes OPTIONAL,
  IN OUT  UINTN             *DataSize,
  OUT     VOID              *Data
  );

EFI_STATUS
EFIAPI
GetNextVariableName (
  IN OUT  UINTN             *VariableNameSize,
  IN OUT  CHAR16            *VariableName,
  IN OUT  EFI_GUID          *VendorGuid
  );

EFI_STATUS
EFIAPI
SetVariable (
  IN CHAR16                  *VariableName,
  IN EFI_GUID                *VendorGuid,
  IN UINT32                  Attributes,
  IN UINTN                   DataSize,
  IN VOID                    *Data
  );

EFI_STATUS
EFIAPI
QueryVariableInfo (
  IN  UINT32                 Attributes,
  OUT UINT64                 *MaximumVariableStorageSize,
  OUT UINT64                 *RemainingVariableStorageSize,
  OUT UINT64                 *MaximumVariableSize
  );

#endif
