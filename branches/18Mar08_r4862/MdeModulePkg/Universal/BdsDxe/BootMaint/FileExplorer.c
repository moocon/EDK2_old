/*++

Copyright (c) 2004 - 2008, Intel Corporation
All rights reserved. This program and the accompanying materials
are licensed and made available under the terms and conditions of the BSD License
which accompanies this distribution.  The full text of the license may be found at
http://opensource.org/licenses/bsd-license.php

THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

Module Name:

  FileExplorer.c

Abstract:

  File explorer related functions.

--*/

#include "BootMaint.h"

VOID
UpdateFileExplorePage (
  IN BMM_CALLBACK_DATA            *CallbackData,
  BM_MENU_OPTION                  *MenuOption
  )
/*++
Routine Description:
  Update the File Explore page.

Arguments:
  MenuOption      - Pointer to menu options to display.

Returns:
  None.

--*/
{
  UINTN           Index;
  BM_MENU_ENTRY   *NewMenuEntry;
  BM_FILE_CONTEXT *NewFileContext;
  EFI_FORM_ID     FormId;

  NewMenuEntry    = NULL;
  NewFileContext  = NULL;
  FormId          = 0;

  RefreshUpdateData ();

  for (Index = 0; Index < MenuOption->MenuNumber; Index++) {
    NewMenuEntry    = BOpt_GetMenuEntry (MenuOption, Index);
    NewFileContext  = (BM_FILE_CONTEXT *) NewMenuEntry->VariableContext;

    if (NewFileContext->IsBootLegacy) {
      continue;
    }

    if ((NewFileContext->IsDir) || (BOOT_FROM_FILE_STATE == CallbackData->FeCurrentState)) {
      //
      // Create Text opcode for directory, also create Text opcode for file in BOOT_FROM_FILE_STATE.
      //
      CreateActionOpCode (
        (UINT16) (FILE_OPTION_OFFSET + Index),
        NewMenuEntry->DisplayStringToken,
        STRING_TOKEN (STR_NULL_STRING),
        EFI_IFR_FLAG_CALLBACK,
        0,
        &gUpdateData
        );
    } else {
      //
      // Create Goto opcode for file in ADD_BOOT_OPTION_STATE or ADD_DRIVER_OPTION_STATE.
      //
      if (ADD_BOOT_OPTION_STATE == CallbackData->FeCurrentState) {
        FormId = FORM_BOOT_ADD_DESCRIPTION_ID;
      } else if (ADD_DRIVER_OPTION_STATE == CallbackData->FeCurrentState) {
        FormId = FORM_DRIVER_ADD_FILE_DESCRIPTION_ID;
      }

      CreateGotoOpCode (
        FormId,
        NewMenuEntry->DisplayStringToken,
        STRING_TOKEN (STR_NULL_STRING),
        EFI_IFR_FLAG_CALLBACK,
        (UINT16) (FILE_OPTION_OFFSET + Index),
        &gUpdateData
        );
    }
  }

  IfrLibUpdateForm (
    CallbackData->FeHiiHandle,
    &mFileExplorerGuid,
    FORM_FILE_EXPLORER_ID,
    FORM_FILE_EXPLORER_ID,
    FALSE,
    &gUpdateData
    );
}

BOOLEAN
UpdateFileExplorer (
  IN BMM_CALLBACK_DATA            *CallbackData,
  IN UINT16                       KeyValue
  )
/*++

Routine Description:
  Update the file explower page with the refershed file system.

Arguments:
  CallbackData  -   BMM context data
  KeyValue        - Key value to identify the type of data to expect.

Returns:
  TRUE          - Inform the caller to create a callback packet to exit file explorer.
  FALSE         - Indicate that there is no need to exit file explorer.

--*/
{
  UINT16          FileOptionMask;
  BM_MENU_ENTRY   *NewMenuEntry;
  BM_FILE_CONTEXT *NewFileContext;
  EFI_FORM_ID     FormId;
  BOOLEAN         ExitFileExplorer;
  EFI_STATUS      Status;

  NewMenuEntry      = NULL;
  NewFileContext    = NULL;
  ExitFileExplorer  = FALSE;

  FileOptionMask    = (UINT16) (FILE_OPTION_MASK & KeyValue);

  if (UNKNOWN_CONTEXT == CallbackData->FeDisplayContext) {
    //
    // First in, display file system.
    //
    BOpt_FreeMenu (&FsOptionMenu);
    BOpt_FindFileSystem (CallbackData);
    CreateMenuStringToken (CallbackData, CallbackData->FeHiiHandle, &FsOptionMenu);

    UpdateFileExplorePage (CallbackData, &FsOptionMenu);

    CallbackData->FeDisplayContext = FILE_SYSTEM;
  } else {
    if (FILE_SYSTEM == CallbackData->FeDisplayContext) {
      NewMenuEntry = BOpt_GetMenuEntry (&FsOptionMenu, FileOptionMask);
    } else if (DIRECTORY == CallbackData->FeDisplayContext) {
      NewMenuEntry = BOpt_GetMenuEntry (&DirectoryMenu, FileOptionMask);
    }

    CallbackData->FeDisplayContext  = DIRECTORY;

    NewFileContext                  = (BM_FILE_CONTEXT *) NewMenuEntry->VariableContext;

    if (NewFileContext->IsDir ) {
      RemoveEntryList (&NewMenuEntry->Link);
      BOpt_FreeMenu (&DirectoryMenu);
      Status = BOpt_FindFiles (CallbackData, NewMenuEntry);
       if (EFI_ERROR (Status)) {
         ExitFileExplorer = TRUE;
         goto exit;
       }
      CreateMenuStringToken (CallbackData, CallbackData->FeHiiHandle, &DirectoryMenu);
      BOpt_DestroyMenuEntry (NewMenuEntry);

      UpdateFileExplorePage (CallbackData, &DirectoryMenu);

    } else {
      switch (CallbackData->FeCurrentState) {
      case BOOT_FROM_FILE_STATE:
        //
        // Here boot from file
        //
        BootThisFile (NewFileContext);
        ExitFileExplorer = TRUE;
        break;

      case ADD_BOOT_OPTION_STATE:
      case ADD_DRIVER_OPTION_STATE:
        if (ADD_BOOT_OPTION_STATE == CallbackData->FeCurrentState) {
          FormId = FORM_BOOT_ADD_DESCRIPTION_ID;
        } else {
          FormId = FORM_DRIVER_ADD_FILE_DESCRIPTION_ID;
        }

        CallbackData->MenuEntry = NewMenuEntry;
        CallbackData->LoadContext->FilePathList = ((BM_FILE_CONTEXT *) (CallbackData->MenuEntry->VariableContext))->DevicePath;

        //
        // Create Subtitle op-code for the display string of the option.
        //
        RefreshUpdateData ();

        CreateSubTitleOpCode (
          NewMenuEntry->DisplayStringToken,
          0,
          0,
          0,
          &gUpdateData
          );

        IfrLibUpdateForm (
          CallbackData->FeHiiHandle,
          &mFileExplorerGuid,
          FormId,
          FormId,
          FALSE,
          &gUpdateData
          );
        break;

      default:
        break;
      }
    }
  }
  exit:
  return ExitFileExplorer;
}

EFI_STATUS
EFIAPI
FileExplorerCallback (
  IN  CONST EFI_HII_CONFIG_ACCESS_PROTOCOL   *This,
  IN  EFI_BROWSER_ACTION                     Action,
  IN  EFI_QUESTION_ID                        QuestionId,
  IN  UINT8                                  Type,
  IN  EFI_IFR_TYPE_VALUE                     *Value,
  OUT EFI_BROWSER_ACTION_REQUEST             *ActionRequest
  )
/*++

  Routine Description:
    This function processes the results of changes in configuration.

  Arguments:
    This          - Points to the EFI_HII_CONFIG_ACCESS_PROTOCOL.
    Action        - Specifies the type of action taken by the browser.
    QuestionId    - A unique value which is sent to the original exporting driver
                    so that it can identify the type of data to expect.
    Type          - The type of value for the question.
    Value         - A pointer to the data being sent to the original exporting driver.
    ActionRequest - On return, points to the action requested by the callback function.

  Returns:
    EFI_SUCCESS          - The callback successfully handled the action.
    EFI_OUT_OF_RESOURCES - Not enough storage is available to hold the variable and its data.
    EFI_DEVICE_ERROR     - The variable could not be saved.
    EFI_UNSUPPORTED      - The specified Action is not supported by the callback.

--*/
{
  BMM_CALLBACK_DATA     *Private;
  FILE_EXPLORER_NV_DATA *NvRamMap;
  EFI_STATUS            Status;
  UINTN                 BufferSize;

  if ((Value == NULL) || (ActionRequest == NULL)) {
    return EFI_INVALID_PARAMETER;
  }

  Status         = EFI_SUCCESS;
  Private        = FE_CALLBACK_DATA_FROM_THIS (This);
  *ActionRequest = EFI_BROWSER_ACTION_REQUEST_NONE;

  //
  // Retrive uncommitted data from Form Browser
  //
  NvRamMap = &Private->FeFakeNvData;
  BufferSize = sizeof (FILE_EXPLORER_NV_DATA);
  Status = GetBrowserData (NULL, NULL, &BufferSize, (UINT8 *) NvRamMap);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  if (QuestionId == KEY_VALUE_SAVE_AND_EXIT_BOOT || QuestionId == KEY_VALUE_SAVE_AND_EXIT_DRIVER) {
    //
    // Apply changes and exit formset
    //
    if (ADD_BOOT_OPTION_STATE == Private->FeCurrentState) {
      Status = Var_UpdateBootOption (Private, NvRamMap);
      if (EFI_ERROR (Status)) {
        return Status;
      }

      BOpt_GetBootOptions (Private);
      CreateMenuStringToken (Private, Private->FeHiiHandle, &BootOptionMenu);
    } else if (ADD_DRIVER_OPTION_STATE == Private->FeCurrentState) {
      Status = Var_UpdateDriverOption (
                Private,
                Private->FeHiiHandle,
                NvRamMap->DescriptionData,
                NvRamMap->OptionalData,
                NvRamMap->ForceReconnect
                );
      if (EFI_ERROR (Status)) {
        return Status;
      }

      BOpt_GetDriverOptions (Private);
      CreateMenuStringToken (Private, Private->FeHiiHandle, &DriverOptionMenu);
    }

    *ActionRequest = EFI_BROWSER_ACTION_REQUEST_EXIT;
  } else if (QuestionId == KEY_VALUE_NO_SAVE_AND_EXIT_BOOT || QuestionId == KEY_VALUE_NO_SAVE_AND_EXIT_DRIVER) {
    //
    // Discard changes and exit formset
    //
    NvRamMap->OptionalData[0]     = 0x0000;
    NvRamMap->DescriptionData[0]  = 0x0000;
    *ActionRequest = EFI_BROWSER_ACTION_REQUEST_EXIT;
  } else if (QuestionId < FILE_OPTION_OFFSET) {
    //
    // Exit File Explorer formset
    //
    *ActionRequest = EFI_BROWSER_ACTION_REQUEST_EXIT;
  } else {
    if (UpdateFileExplorer (Private, QuestionId)) {
      *ActionRequest = EFI_BROWSER_ACTION_REQUEST_EXIT;
    }
  }

  return Status;
}
