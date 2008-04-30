/** @file
  Function and Macro defintions for to extract default values from UEFI Form package.

  Copyright (c) 2008, Intel Corporation
  All rights reserved. This program and the accompanying materials
  are licensed and made available under the terms and conditions of the BSD License
  which accompanies this distribution.  The full text of the license may be found at
  http://opensource.org/licenses/bsd-license.php

  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/


#include <FrameworkDxe.h>

#include <Protocol/FrameworkHii.h>
#include <Protocol/HiiDatabase.h>

#include <Library/BaseLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/DebugLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/UefiBootServicesTableLib.h>

#include "UefiIfrParser.h"
#include "UefiIfrDefault.h"

//
// Extern Variables
//
extern CONST EFI_HII_DATABASE_PROTOCOL            *mHiiDatabase;
extern CONST EFI_HII_FONT_PROTOCOL                *mHiiFontProtocol;
extern CONST EFI_HII_IMAGE_PROTOCOL               *mHiiImageProtocol;
extern CONST EFI_HII_STRING_PROTOCOL              *mHiiStringProtocol;
extern CONST EFI_HII_CONFIG_ROUTING_PROTOCOL      *mHiiConfigRoutingProtocol;

extern EFI_GUID          gZeroGuid;

/**
  Fetch the Ifr binary data of a FormSet.

  @param  Handle                 PackageList Handle
  @param  FormSetGuid            GUID of a formset. If not specified (NULL or zero
                                 GUID), take the first FormSet found in package
                                 list.
  @param  BinaryLength           The length of the FormSet IFR binary.
  @param  BinaryData             The buffer designed to receive the FormSet.

  @retval EFI_SUCCESS            Buffer filled with the requested FormSet.
                                 BufferLength was updated.
  @retval EFI_INVALID_PARAMETER  The handle is unknown.
  @retval EFI_NOT_FOUND          A form or FormSet on the requested handle cannot
                                 be found with the requested FormId.

**/
EFI_STATUS
GetIfrBinaryData (
  IN  EFI_HII_HANDLE   Handle,
  IN OUT EFI_GUID      *FormSetGuid,
  OUT UINTN            *BinaryLength,
  OUT UINT8            **BinaryData
  )
{
  EFI_STATUS                   Status;
  EFI_HII_PACKAGE_LIST_HEADER  *HiiPackageList;
  UINTN                        BufferSize;
  UINT8                        *Package;
  UINT8                        *OpCodeData;
  UINT32                       Offset;
  UINT32                       Offset2;
  BOOLEAN                      ReturnDefault;
  UINT32                       PackageListLength;
  EFI_HII_PACKAGE_HEADER       PackageHeader;

  OpCodeData = NULL;
  Package = NULL;
  ZeroMem (&PackageHeader, sizeof (EFI_HII_PACKAGE_HEADER));;

  //
  // if FormSetGuid is NULL or zero GUID, return first FormSet in the package list
  //
  if (FormSetGuid == NULL || CompareGuid (FormSetGuid, &gZeroGuid)) {
    ReturnDefault = TRUE;
  } else {
    ReturnDefault = FALSE;
  }

  //
  // Get HII PackageList
  //
  BufferSize = 0;
  HiiPackageList = NULL;
  Status = mHiiDatabase->ExportPackageLists (mHiiDatabase, Handle, &BufferSize, HiiPackageList);
  if (Status == EFI_BUFFER_TOO_SMALL) {
    HiiPackageList = AllocatePool (BufferSize);
    ASSERT (HiiPackageList != NULL);

    Status = mHiiDatabase->ExportPackageLists (mHiiDatabase, Handle, &BufferSize, HiiPackageList);
  }
  if (EFI_ERROR (Status)) {
    return Status;
  }

  //
  // Get Form package from this HII package List
  //
  Offset = sizeof (EFI_HII_PACKAGE_LIST_HEADER);
  Offset2 = 0;
  CopyMem (&PackageListLength, &HiiPackageList->PackageLength, sizeof (UINT32));

  while (Offset < PackageListLength) {
    Package = ((UINT8 *) HiiPackageList) + Offset;
    CopyMem (&PackageHeader, Package, sizeof (EFI_HII_PACKAGE_HEADER));

    if (PackageHeader.Type == EFI_HII_PACKAGE_FORM) {
      //
      // Search FormSet in this Form Package
      //
      Offset2 = sizeof (EFI_HII_PACKAGE_HEADER);
      while (Offset2 < PackageHeader.Length) {
        OpCodeData = Package + Offset2;

        if (((EFI_IFR_OP_HEADER *) OpCodeData)->OpCode == EFI_IFR_FORM_SET_OP) {
          //
          // Check whether return default FormSet
          //
          if (ReturnDefault) {
            break;
          }

          //
          // FormSet GUID is specified, check it
          //
          if (CompareGuid (FormSetGuid, (EFI_GUID *)(OpCodeData + sizeof (EFI_IFR_OP_HEADER)))) {
            break;
          }
        }

        Offset2 += ((EFI_IFR_OP_HEADER *) OpCodeData)->Length;
      }

      if (Offset2 < PackageHeader.Length) {
        //
        // Target formset found
        //
        break;
      }
    }

    Offset += PackageHeader.Length;
  }

  if (Offset >= PackageListLength) {
    //
    // Form package not found in this Package List
    //
    gBS->FreePool (HiiPackageList);
    return EFI_NOT_FOUND;
  }

  if (ReturnDefault && FormSetGuid != NULL) {
    //
    // Return the default FormSet GUID
    //
    CopyMem (FormSetGuid, &((EFI_IFR_FORM_SET *) OpCodeData)->Guid, sizeof (EFI_GUID));
  }

  //
  // To determine the length of a whole FormSet IFR binary, one have to parse all the Opcodes
  // in this FormSet; So, here just simply copy the data from start of a FormSet to the end
  // of the Form Package.
  //
  *BinaryLength = PackageHeader.Length - Offset2;
  *BinaryData = AllocateCopyPool (*BinaryLength, OpCodeData);

  gBS->FreePool (HiiPackageList);

  if (*BinaryData == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }

  return EFI_SUCCESS;
}

/**
  Initialize the internal data structure of a FormSet.

  @param  Handle                 PackageList Handle
  @param  FormSetGuid            GUID of a formset. If not specified (NULL or zero
                                 GUID), take the first FormSet found in package
                                 list.
  @param  FormSet                FormSet data structure.

  @retval EFI_SUCCESS            The function completed successfully.
  @retval EFI_NOT_FOUND          The specified FormSet could not be found.

**/
EFI_STATUS
InitializeFormSet (
  IN  EFI_HII_HANDLE                   Handle,
  IN OUT EFI_GUID                      *FormSetGuid,
  OUT FORM_BROWSER_FORMSET             *FormSet
  )
{
  EFI_STATUS                Status;
  EFI_HANDLE                DriverHandle;

  Status = GetIfrBinaryData (Handle, FormSetGuid, &FormSet->IfrBinaryLength, &FormSet->IfrBinaryData);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  FormSet->HiiHandle = Handle;
  CopyMem (&FormSet->Guid, FormSetGuid, sizeof (EFI_GUID));

  //
  // Retrieve ConfigAccess Protocol associated with this HiiPackageList
  //
  Status = mHiiDatabase->GetPackageListHandle (mHiiDatabase, Handle, &DriverHandle);
  if (EFI_ERROR (Status)) {
    return Status;
  }
  FormSet->DriverHandle = DriverHandle;
  Status = gBS->HandleProtocol (
                  DriverHandle,
                  &gEfiHiiConfigAccessProtocolGuid,
                  (VOID **) &FormSet->ConfigAccess
                  );
  if (EFI_ERROR (Status)) {
    //
    // Configuration Driver don't attach ConfigAccess protocol to its HII package
    // list, then there will be no configuration action required
    //
    FormSet->ConfigAccess = NULL;
  }

  //
  // Parse the IFR binary OpCodes
  //
  Status = ParseOpCodes (FormSet);
  if (EFI_ERROR (Status)) {
    return Status;
  }
  return Status;
}

/**
  Set the data position at Offset with Width in Node->Buffer based 
  the value passed in.

  @param  Node                    The Buffer Storage Node.
  @param Value                    The input value.
  @param Offset                   The offset in Node->Buffer for the update.
  @param Width                    The length of the Value.
  
  @retval VOID

**/
VOID
SetNodeBuffer (
  OUT UEFI_IFR_BUFFER_STORAGE_NODE        *Node,
  IN  CONST   EFI_HII_VALUE               *Value,
  IN  UINTN                               Offset,
  IN  UINTN                               Width
  )
{
  ASSERT (Node->Signature == UEFI_IFR_BUFFER_STORAGE_NODE_SIGNATURE);
  ASSERT (Offset + Width <= Node->Size);

  CopyMem (Node->Buffer + Offset, &Value->Value.u8, Width);
}


/**
  Reset Question to its default value.

  @param  FormSet                FormSet data structure.
  @param  DefaultId              The Class of the default.

  @retval EFI_SUCCESS            Question is reset to default value.

**/
EFI_STATUS
GetQuestionDefault (
  IN FORM_BROWSER_FORMSET             *FormSet,
  IN FORM_BROWSER_FORM                *Form,
  IN FORM_BROWSER_STATEMENT           *Question,
  IN UINT16                           DefaultId,
  IN UINT16                           VarStoreId,
  OUT UEFI_IFR_BUFFER_STORAGE_NODE        *Node
  )
{
  EFI_STATUS              Status;
  LIST_ENTRY              *Link;
  QUESTION_DEFAULT        *Default;
  QUESTION_OPTION         *Option;
  EFI_HII_VALUE           *HiiValue;

  Status = EFI_SUCCESS;

  //
  // Statement don't have storage, skip them
  //
  if (Question->QuestionId == 0) {
    return Status;
  }

  if (Question->VarStoreId != VarStoreId) {
    return Status;
  }

  ASSERT (Question->Storage->Type == EFI_HII_VARSTORE_BUFFER);

  //
  // There are three ways to specify default value for a Question:
  //  1, use nested EFI_IFR_DEFAULT (highest priority)
  //  2, set flags of EFI_ONE_OF_OPTION (provide Standard and Manufacturing default)
  //  3, set flags of EFI_IFR_CHECKBOX (provide Standard and Manufacturing default) (lowest priority)
  //
  HiiValue = &Question->HiiValue;

  //
  // EFI_IFR_DEFAULT has highest priority
  //
  if (!IsListEmpty (&Question->DefaultListHead)) {
    Link = GetFirstNode (&Question->DefaultListHead);
    while (!IsNull (&Question->DefaultListHead, Link)) {
      Default = QUESTION_DEFAULT_FROM_LINK (Link);

      if (Default->DefaultId == DefaultId) {
        if (Default->ValueExpression != NULL) {
          //
          // Default is provided by an Expression, evaluate it
          //
          Status = EvaluateExpression (FormSet, Form, Default->ValueExpression);
          if (EFI_ERROR (Status)) {
            return Status;
          }

          CopyMem (HiiValue, &Default->ValueExpression->Result, sizeof (EFI_HII_VALUE));
        } else {
          //
          // Default value is embedded in EFI_IFR_DEFAULT
          //
          CopyMem (HiiValue, &Default->Value, sizeof (EFI_HII_VALUE));
        }
       
        SetNodeBuffer (Node, HiiValue, Question->VarStoreInfo.VarOffset, Question->StorageWidth);
        return EFI_SUCCESS;
      }

      Link = GetNextNode (&Question->DefaultListHead, Link);
    }
  }

  //
  // EFI_ONE_OF_OPTION
  //
  if ((Question->Operand == EFI_IFR_ONE_OF_OP) && !IsListEmpty (&Question->OptionListHead)) {
    if (DefaultId <= EFI_HII_DEFAULT_CLASS_MANUFACTURING)  {
      //
      // OneOfOption could only provide Standard and Manufacturing default
      //
      Link = GetFirstNode (&Question->OptionListHead);
      while (!IsNull (&Question->OptionListHead, Link)) {
        Option = QUESTION_OPTION_FROM_LINK (Link);

        if (((DefaultId == EFI_HII_DEFAULT_CLASS_STANDARD) && (Option->Flags & EFI_IFR_OPTION_DEFAULT)) ||
            ((DefaultId == EFI_HII_DEFAULT_CLASS_MANUFACTURING) && (Option->Flags & EFI_IFR_OPTION_DEFAULT_MFG))
           ) {
          CopyMem (HiiValue, &Option->Value, sizeof (EFI_HII_VALUE));

          SetNodeBuffer (Node, HiiValue, Question->VarStoreInfo.VarOffset, Question->StorageWidth);
          return EFI_SUCCESS;
        }

        Link = GetNextNode (&Question->OptionListHead, Link);
      }
    }
  }

  //
  // EFI_IFR_CHECKBOX - lowest priority
  //
  if (Question->Operand == EFI_IFR_CHECKBOX_OP) {
    if (DefaultId <= EFI_HII_DEFAULT_CLASS_MANUFACTURING)  {
      //
      // Checkbox could only provide Standard and Manufacturing default
      //
      if (((DefaultId == EFI_HII_DEFAULT_CLASS_STANDARD) && (Question->Flags & EFI_IFR_CHECKBOX_DEFAULT)) ||
          ((DefaultId == EFI_HII_DEFAULT_CLASS_MANUFACTURING) && (Question->Flags & EFI_IFR_CHECKBOX_DEFAULT_MFG))
         ) {
        HiiValue->Value.b = TRUE;
      } else {
        HiiValue->Value.b = FALSE;
      }

      SetNodeBuffer (Node, HiiValue, Question->VarStoreInfo.VarOffset, Question->StorageWidth);
      return EFI_SUCCESS;
    }
  }

  return Status;
}


/**
  Reset Questions in a Form to their default value.

  @param  FormSet                FormSet data structure.
  @param  Form                   The Form which to be reset.
  @param  DefaultId              The Class of the default.

  @retval EFI_SUCCESS            The function completed successfully.

**/
EFI_STATUS
ExtractFormDefault (
  IN FORM_BROWSER_FORMSET             *FormSet,
  IN FORM_BROWSER_FORM                *Form,
  IN UINT16                           DefaultId,
  IN UINT16                           VarStoreId,
  OUT UEFI_IFR_BUFFER_STORAGE_NODE        *Node
  )
{
  EFI_STATUS              Status;
  LIST_ENTRY              *Link;
  FORM_BROWSER_STATEMENT  *Question;

  Link = GetFirstNode (&Form->StatementListHead);
  while (!IsNull (&Form->StatementListHead, Link)) {
    Question = FORM_BROWSER_STATEMENT_FROM_LINK (Link);
    Link = GetNextNode (&Form->StatementListHead, Link);

    //
    // Reset Question to its default value
    //
    Status = GetQuestionDefault (FormSet, Form, Question, DefaultId, VarStoreId, Node);
    if (EFI_ERROR (Status)) {
      continue;
    }

  }
  return EFI_SUCCESS;
}


/**
  Destroy all the buffer allocated for the fileds of
  UEFI_IFR_BUFFER_STORAGE_NODE. The Node itself
  will be freed too.

  @param  FormSet                FormSet data structure.
  @param  DefaultId              The Class of the default.

  @retval   VOID

**/
VOID
DestroyDefaultNode (
  IN UEFI_IFR_BUFFER_STORAGE_NODE        *Node
  )
{
  SafeFreePool (Node->Buffer);
  SafeFreePool (Node->Name);
  SafeFreePool (Node);
}


/**
  Get the default value for Buffer Type storage named by
  a Default Store and a Storage Store from a FormSet.
  The result is in the a instance of UEFI_IFR_BUFFER_STORAGE_NODE
  allocated by this function. It is inserted to the link list.
  
  @param  DefaultStore            The Default Store.
  @param  Storage                   The Storage.
  @param  FormSet                  The Form Set.
  @param  UefiDefaultsListHead The head of link list for the output.

  @retval   EFI_SUCCESS          Successful.
  
**/
EFI_STATUS
GetBufferTypeDefaultIdAndStorageId (
  IN        FORMSET_DEFAULTSTORE        *DefaultStore,
  IN        FORMSET_STORAGE             *Storage,
  IN        FORM_BROWSER_FORMSET        *FormSet,
  OUT       LIST_ENTRY                  *UefiDefaultsListHead
 )
{
  UEFI_IFR_BUFFER_STORAGE_NODE        *Node;
  LIST_ENTRY              *Link;
  FORM_BROWSER_FORM       *Form;
  EFI_STATUS              Status;

  Node = AllocateZeroPool (sizeof (UEFI_IFR_BUFFER_STORAGE_NODE));
  ASSERT (Node != NULL);

  Node->Signature = UEFI_IFR_BUFFER_STORAGE_NODE_SIGNATURE;
  Node->Name      = AllocateCopyPool (StrSize (Storage->Name), Storage->Name);
  Node->DefaultId = DefaultStore->DefaultId;
  CopyGuid (&Node->Guid, &Storage->Guid);
  Node->Size      = Storage->Size;
  Node->Buffer    = AllocateZeroPool (Node->Size);
  //
  // Extract default from IFR binary
  //
  Link = GetFirstNode (&FormSet->FormListHead);
  while (!IsNull (&FormSet->FormListHead, Link)) {
    Form = FORM_BROWSER_FORM_FROM_LINK (Link);

    Status = ExtractFormDefault (FormSet, Form, DefaultStore->DefaultId, Storage->VarStoreId, Node);
    ASSERT_EFI_ERROR (Status);

    Link = GetNextNode (&FormSet->FormListHead, Link);
  }

  InsertTailList (UefiDefaultsListHead, &Node->List);
  
  return EFI_SUCCESS;
}


/**
  Get the default value for Buffer Type storage named by
  a Default Store from a FormSet.
  The result is in the a instance of UEFI_IFR_BUFFER_STORAGE_NODE
  allocated by this function. The output can be multiple instances
  of UEFI_IFR_BUFFER_STORAGE_NODE. It is inserted to the link list.
  
  @param  DefaultStore            The Default Store.
  @param  FormSet                  The Form Set.
  @param  UefiDefaultsListHead The head of link list for the output.

  @retval   EFI_SUCCESS          Successful.
  
**/
EFI_STATUS
GetBufferTypeDefaultId (
  IN  FORMSET_DEFAULTSTORE  *DefaultStore,
  IN  FORM_BROWSER_FORMSET  *FormSet,
  OUT       LIST_ENTRY      *UefiDefaultsListHead
  )
{
  LIST_ENTRY                  *StorageListEntry;
  FORMSET_STORAGE             *Storage;
  EFI_STATUS                  Status;

  StorageListEntry = GetFirstNode (&FormSet->StorageListHead);

  while (!IsNull (&FormSet->StorageListHead, StorageListEntry)) {
    Storage = FORMSET_STORAGE_FROM_LINK(StorageListEntry);

    if (Storage->Type == EFI_HII_VARSTORE_BUFFER) {
      Status = GetBufferTypeDefaultIdAndStorageId (DefaultStore, Storage, FormSet, UefiDefaultsListHead);
    }

    StorageListEntry = GetNextNode (&FormSet->StorageListHead, StorageListEntry);
  }
  
  return EFI_SUCCESS;
}


/**
  Get the default value for Buffer Type storage from the first FormSet
  in the Package List specified by a EFI_HII_HANDLE.
  
  The results can be multiple instances of UEFI_IFR_BUFFER_STORAGE_NODE. 
  They are inserted to the link list.
  
  @param  UefiHiiHandle           The handle for the package list.
  @param  UefiDefaultsListHead The head of link list for the output.

  @retval   EFI_SUCCESS          Successful.
  
**/
EFI_STATUS
UefiIfrGetBufferTypeDefaults (
  IN  EFI_HII_HANDLE      UefiHiiHandle,
  OUT LIST_ENTRY          **UefiDefaults
  )
{
  FORM_BROWSER_FORMSET *FormSet;
  EFI_GUID              FormSetGuid;
  LIST_ENTRY            *DefaultListEntry;
  FORMSET_DEFAULTSTORE  *DefaultStore;
  EFI_STATUS            Status;

  ASSERT (UefiDefaults != NULL);

  FormSet = AllocateZeroPool (sizeof (FORM_BROWSER_FORMSET));    
  ASSERT (FormSet != NULL);

  CopyGuid (&FormSetGuid, &gZeroGuid);
  Status = InitializeFormSet (UefiHiiHandle, &FormSetGuid, FormSet);
  ASSERT_EFI_ERROR (Status);

  *UefiDefaults = AllocateZeroPool (sizeof (LIST_ENTRY));
  ASSERT (UefiDefaults != NULL);
  InitializeListHead (*UefiDefaults);

  DefaultListEntry = GetFirstNode (&FormSet->DefaultStoreListHead);
  while (!IsNull (&FormSet->DefaultStoreListHead, DefaultListEntry)) {
    DefaultStore = FORMSET_DEFAULTSTORE_FROM_LINK(DefaultListEntry);

    Status = GetBufferTypeDefaultId (DefaultStore, FormSet, *UefiDefaults);
    ASSERT_EFI_ERROR (Status);

    DefaultListEntry = GetNextNode (&FormSet->DefaultStoreListHead, DefaultListEntry);    
  }

  DestroyFormSet (FormSet);
  
  return EFI_SUCCESS;
}


/**
  Convert the UEFI Buffer Type default values to a Framework HII default
  values specified by a EFI_HII_VARIABLE_PACK_LIST structure.
  
  @param  ListHead                  The link list of UEFI_IFR_BUFFER_STORAGE_NODE
                                              which contains the default values retrived from
                                              a UEFI form set.
  @param  DefaultMask            The default mask.
                                             The valid values are FRAMEWORK_EFI_IFR_FLAG_DEFAULT
                                             and FRAMEWORK_EFI_IFR_FLAG_MANUFACTURING.
                                            UEFI spec only map FRAMEWORK_EFI_IFR_FLAG_DEFAULT and FRAMEWORK_EFI_IFR_FLAG_MANUFACTURING 
                                            from specification to valid default class.
  @param  VariablePackList     The output default value in a format defined in Framework.
                                             

  @retval   EFI_SUCCESS                       Successful.
  @retval   EFI_INVALID_PARAMETER      The default mask is not FRAMEWORK_EFI_IFR_FLAG_DEFAULT or 
                                                           FRAMEWORK_EFI_IFR_FLAG_MANUFACTURING.
**/
EFI_STATUS
UefiDefaultsToFrameworkDefaults (
  IN     LIST_ENTRY                  *ListHead,
  IN     UINTN                       DefaultMask,
  OUT    EFI_HII_VARIABLE_PACK_LIST  **VariablePackList
  )
{
  LIST_ENTRY                        *List;
  UEFI_IFR_BUFFER_STORAGE_NODE      *Node;
  UINTN                             Size;
  UINTN                             Count;
  UINT16                            DefaultId;
  EFI_HII_VARIABLE_PACK             *Pack;
  EFI_HII_VARIABLE_PACK_LIST        *PackList;

  if (DefaultMask == FRAMEWORK_EFI_IFR_FLAG_DEFAULT) {
    DefaultId = EFI_HII_DEFAULT_CLASS_STANDARD;
  } else if (DefaultMask == FRAMEWORK_EFI_IFR_FLAG_MANUFACTURING) {
    DefaultId = EFI_HII_DEFAULT_CLASS_MANUFACTURING;
  } else {
    //
    // UEFI spec only map FRAMEWORK_EFI_IFR_FLAG_DEFAULT and FRAMEWORK_EFI_IFR_FLAG_MANUFACTURING 
    // from specification to valid default class.
    //
    ASSERT (FALSE);
    return EFI_INVALID_PARAMETER;
  }
  
  //
  // Calculate the size of the output EFI_HII_VARIABLE_PACK_LIST structure
  //
  Size = 0;
  Count = 0;
  List = GetFirstNode (ListHead);
  while (!IsNull (ListHead, List)) {
    Node = UEFI_IFR_BUFFER_STORAGE_NODE_FROM_LIST(List);

    if (Node->DefaultId == DefaultId) {
      Size += Node->Size;
      Size += StrSize (Node->Name);

      Count++;
    }
    
    List = GetNextNode (ListHead, List);  
  }

  Size = Size + Count * (sizeof (EFI_HII_VARIABLE_PACK_LIST) + sizeof (EFI_HII_VARIABLE_PACK));
  
  *VariablePackList = AllocateZeroPool (Size);
  ASSERT (*VariablePackList != NULL);

  List = GetFirstNode (ListHead);

  PackList = (EFI_HII_VARIABLE_PACK_LIST *) *VariablePackList;
  Pack     = (EFI_HII_VARIABLE_PACK *) (PackList + 1);
  while (!IsNull (ListHead, List)) {
    Node = UEFI_IFR_BUFFER_STORAGE_NODE_FROM_LIST(List);

    Size = 0;    
    if (Node->DefaultId == DefaultId) {
      Size += Node->Size;
      Size += StrSize (Node->Name);
      Size += sizeof (EFI_HII_VARIABLE_PACK);      

      //
      // In UEFI, 0 is defined to be invalid for EFI_IFR_VARSTORE.VarStoreId.
      // So the default storage of Var Store in VFR from a Framework module 
      // should be translated to 0xFFEE.
      //
      if (Node->StoreId == RESERVED_VARSTORE_ID) {
        Pack->VariableId = 0;
      }
      //
      // Initialize EFI_HII_VARIABLE_PACK
      //
      Pack->Header.Type   = 0;
      Pack->Header.Length = Size;
      Pack->VariableId = Node->StoreId;
      Pack->VariableNameLength = StrSize (Node->Name);
      CopyMem (&Pack->VariableGuid, &Node->Guid, sizeof (EFI_GUID));
      
      CopyMem ((UINT8 *) Pack + sizeof (EFI_HII_VARIABLE_PACK), Node->Name, StrSize (Node->Name));
      CopyMem ((UINT8 *) Pack + sizeof (EFI_HII_VARIABLE_PACK) + Pack->VariableNameLength, Node->Buffer, Node->Size);

      Size += sizeof (EFI_HII_VARIABLE_PACK_LIST);

      //
      // initialize EFI_HII_VARIABLE_PACK_LIST
      //
      PackList->VariablePack = Pack;
      PackList->NextVariablePack = (EFI_HII_VARIABLE_PACK_LIST *)((UINT8 *) PackList + Size);
            
    }
    
    List = GetNextNode (ListHead, List);  
  }
  
  
  return EFI_SUCCESS;
}


/**
  Free up all buffer allocated for the link list of UEFI_IFR_BUFFER_STORAGE_NODE.
    
  @param  ListHead                  The link list of UEFI_IFR_BUFFER_STORAGE_NODE
                                              which contains the default values retrived from
                                              a UEFI form set.
                                             

  @retval   EFI_SUCCESS                       Successful.
  @retval   EFI_INVALID_PARAMETER      The default mask is not FRAMEWORK_EFI_IFR_FLAG_DEFAULT or 
                                                           FRAMEWORK_EFI_IFR_FLAG_MANUFACTURING.
**/
VOID
FreeDefaultList (
  IN     LIST_ENTRY                  *ListHead
  )
{
  LIST_ENTRY *Node;
  UEFI_IFR_BUFFER_STORAGE_NODE *Default;

  Node = GetFirstNode (ListHead);
  
  while (!IsNull (ListHead, Node)) {
    Default = UEFI_IFR_BUFFER_STORAGE_NODE_FROM_LIST(Node);

    RemoveEntryList (Node);
   
    DestroyDefaultNode (Default);
    
    Node = GetFirstNode (ListHead);
  }

  FreePool (ListHead);
}

