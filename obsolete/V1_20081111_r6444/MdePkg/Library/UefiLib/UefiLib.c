/** @file
  The UEFI Library provides functions and macros that simplify the development of 
  UEFI Drivers and UEFI Applications.  These functions and macros help manage EFI 
  events, build simple locks utilizing EFI Task Priority Levels (TPLs), install 
  EFI Driver Model related protocols, manage Unicode string tables for UEFI Drivers, 
  and print messages on the console output and standard error devices.

  Copyright (c) 2006 - 2007, Intel Corporation<BR>
  All rights reserved. This program and the accompanying materials
  are licensed and made available under the terms and conditions of the BSD License
  which accompanies this distribution.  The full text of the license may be found at
  http://opensource.org/licenses/bsd-license.php

  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/


#include "UefiLibInternal.h"

/**
  Compare whether two names of languages are identical.

  @param  Language1 Name of language 1.
  @param  Language2 Name of language 2.

  @retval TRUE      Language 1 and language 2 are the same.
  @retval FALSE     Language 1 and language 2 are not the same.

**/
BOOLEAN
CompareIso639LanguageCode (
  IN CONST CHAR8  *Language1,
  IN CONST CHAR8  *Language2
  )
{
  UINT32  Name1;
  UINT32  Name2;

  Name1 = ReadUnaligned24 ((CONST UINT32 *) Language1);
  Name2 = ReadUnaligned24 ((CONST UINT32 *) Language2);

  return (BOOLEAN) (Name1 == Name2);
}

/**
  This function searches the list of configuration tables stored in the EFI System
  Table for a table with a GUID that matches TableGuid.  If a match is found,
  then a pointer to the configuration table is returned in Table, and EFI_SUCCESS
  is returned.  If a matching GUID is not found, then EFI_NOT_FOUND is returned.
  If TableGuid is NULL, then ASSERT().
  If Table is NULL, then ASSERT().

  @param  TableGuid       Pointer to table's GUID type..
  @param  Table           Pointer to the table associated with TableGuid in the EFI System Table.

  @retval EFI_SUCCESS     A configuration table matching TableGuid was found.
  @retval EFI_NOT_FOUND   A configuration table matching TableGuid could not be found.

**/
EFI_STATUS
EFIAPI
EfiGetSystemConfigurationTable (
  IN  EFI_GUID  *TableGuid,
  OUT VOID      **Table
  )
{
  EFI_SYSTEM_TABLE  *SystemTable;
  UINTN             Index;

  ASSERT (TableGuid != NULL);
  ASSERT (Table != NULL);

  SystemTable = gST;
  *Table = NULL;
  for (Index = 0; Index < SystemTable->NumberOfTableEntries; Index++) {
    if (CompareGuid (TableGuid, &(SystemTable->ConfigurationTable[Index].VendorGuid))) {
      *Table = SystemTable->ConfigurationTable[Index].VendorTable;
      return EFI_SUCCESS;
    }
  }

  return EFI_NOT_FOUND;
}

/**
  This function causes the notification function to be executed for every protocol
  of type ProtocolGuid instance that exists in the system when this function is
  invoked.  In addition, every time a protocol of type ProtocolGuid instance is
  installed or reinstalled, the notification function is also executed.

  @param  ProtocolGuid    Supplies GUID of the protocol upon whose installation the event is fired.
  @param  NotifyTpl       Supplies the task priority level of the event notifications.
  @param  NotifyFunction  Supplies the function to notify when the event is signaled.
  @param  NotifyContext   The context parameter to pass to NotifyFunction.
  @param  Registration    A pointer to a memory location to receive the registration value.

  @return The notification event that was created.

**/
EFI_EVENT
EFIAPI
EfiCreateProtocolNotifyEvent(
  IN  EFI_GUID          *ProtocolGuid,
  IN  EFI_TPL           NotifyTpl,
  IN  EFI_EVENT_NOTIFY  NotifyFunction,
  IN  VOID              *NotifyContext,  OPTIONAL
  OUT VOID              **Registration
  )
{
  EFI_STATUS  Status;
  EFI_EVENT   Event;

  //
  // Create the event
  //

  Status = gBS->CreateEvent (
                  EVT_NOTIFY_SIGNAL,
                  NotifyTpl,
                  NotifyFunction,
                  NotifyContext,
                  &Event
                  );
  ASSERT_EFI_ERROR (Status);

  //
  // Register for protocol notifactions on this event
  //

  Status = gBS->RegisterProtocolNotify (
                  ProtocolGuid,
                  Event,
                  Registration
                  );

  ASSERT_EFI_ERROR (Status);

  //
  // Kick the event so we will perform an initial pass of
  // current installed drivers
  //

  gBS->SignalEvent (Event);
  return Event;
}

/**
  This function creates an event using NotifyTpl, NoifyFunction, and NotifyContext.
  This event is signaled with EfiNamedEventSignal().  This provide the ability for
  one or more listeners on the same event named by the GUID specified by Name.
  If Name is NULL, then ASSERT().
  If NotifyTpl is not a legal TPL value, then ASSERT().
  If NotifyFunction is NULL, then ASSERT().

  @param  Name                  Supplies GUID name of the event.
  @param  NotifyTpl             Supplies the task priority level of the event notifications.
  @param  NotifyFunction        Supplies the function to notify when the event is signaled.
  @param  NotifyContext         The context parameter to pass to NotifyFunction.
  @param  Registration          A pointer to a memory location to receive the registration value.

  @retval EFI_SUCCESS           A named event was created.
  @retval EFI_OUT_OF_RESOURCES  There are not enough resource to create the named event.

**/
EFI_STATUS
EFIAPI
EfiNamedEventListen (
  IN CONST EFI_GUID    *Name,
  IN EFI_TPL           NotifyTpl,
  IN EFI_EVENT_NOTIFY  NotifyFunction,
  IN CONST VOID        *NotifyContext,  OPTIONAL
  OUT VOID             *Registration OPTIONAL
  )
{
  EFI_STATUS  Status;
  EFI_EVENT   Event;
  VOID        *RegistrationLocal;

  ASSERT (Name != NULL);
  ASSERT (NotifyFunction != NULL);
  ASSERT (NotifyTpl <= TPL_HIGH_LEVEL);
  
  //
  // Create event
  //
  Status = gBS->CreateEvent (
                  EVT_NOTIFY_SIGNAL,
                  NotifyTpl,
                  NotifyFunction,
                  (VOID *) NotifyContext,
                  &Event
                  );
  ASSERT_EFI_ERROR (Status);

  //
  // The Registration is not optional to RegisterProtocolNotify().
  // To make it optional to EfiNamedEventListen(), may need to substitute with a local.
  //
  if (Registration != NULL) {
    RegistrationLocal = Registration;
  } else {
    RegistrationLocal = &RegistrationLocal;
  }

  //
  // Register for an installation of protocol interface
  //

  Status = gBS->RegisterProtocolNotify (
                  (EFI_GUID *) Name,
                  Event,
                  RegistrationLocal
                  );
  ASSERT_EFI_ERROR (Status);

  return Status;
}

/**
  This function signals the named event specified by Name.  The named event must
  have been created with EfiNamedEventListen().

  @param  Name                  Supplies GUID name of the event.

  @retval EFI_SUCCESS           A named event was signaled.
  @retval EFI_OUT_OF_RESOURCES  There are not enough resource to signal the named event.

**/
EFI_STATUS
EFIAPI
EfiNamedEventSignal (
  IN CONST EFI_GUID  *Name
  )
{
  EFI_STATUS  Status;
  EFI_HANDLE  Handle;

  Handle = NULL;
  Status = gBS->InstallProtocolInterface (
                  &Handle,
                  (EFI_GUID *) Name,
                  EFI_NATIVE_INTERFACE,
                  NULL
                  );
  ASSERT_EFI_ERROR (Status);

  Status = gBS->UninstallProtocolInterface (
                  Handle,
                  (EFI_GUID *) Name,
                  NULL
                  );
  ASSERT_EFI_ERROR (Status);

  return Status;
}

/**
  Returns the current TPL.

  This function returns the current TPL.  There is no EFI service to directly
  retrieve the current TPL. Instead, the RaiseTPL() function is used to raise
  the TPL to TPL_HIGH_LEVEL.  This will return the current TPL.  The TPL level
  can then immediately be restored back to the current TPL level with a call
  to RestoreTPL().

  @param  VOID

  @retval EFI_TPL              The current TPL.

**/
EFI_TPL
EFIAPI
EfiGetCurrentTpl (
  VOID
  )
{
  EFI_TPL Tpl;

  Tpl = gBS->RaiseTPL (TPL_HIGH_LEVEL);
  gBS->RestoreTPL (Tpl);

  return Tpl;
}


/**
  This function initializes a basic mutual exclusion lock to the released state
  and returns the lock.  Each lock provides mutual exclusion access at its task
  priority level.  Since there is no preemption or multiprocessor support in EFI,
  acquiring the lock only consists of raising to the locks TPL.
  If Lock is NULL, then ASSERT().
  If Priority is not a valid TPL value, then ASSERT().

  @param  Lock       A pointer to the lock data structure to initialize.
  @param  Priority   EFI TPL associated with the lock.

  @return The lock.

**/
EFI_LOCK *
EFIAPI
EfiInitializeLock (
  IN OUT EFI_LOCK  *Lock,
  IN EFI_TPL        Priority
  )
{
  ASSERT (Lock != NULL);
  ASSERT (Priority <= TPL_HIGH_LEVEL);

  Lock->Tpl       = Priority;
  Lock->OwnerTpl  = TPL_APPLICATION;
  Lock->Lock      = EfiLockReleased ;
  return Lock;
}

/**
  This function raises the system's current task priority level to the task
  priority level of the mutual exclusion lock.  Then, it places the lock in the
  acquired state.
  If Lock is NULL, then ASSERT().
  If Lock is not initialized, then ASSERT().
  If Lock is already in the acquired state, then ASSERT().

  @param  Lock   The task lock with priority level.

**/
VOID
EFIAPI
EfiAcquireLock (
  IN EFI_LOCK  *Lock
  )
{
  ASSERT (Lock != NULL);
  ASSERT (Lock->Lock == EfiLockReleased);

  Lock->OwnerTpl = gBS->RaiseTPL (Lock->Tpl);
  Lock->Lock     = EfiLockAcquired;
}

/**
  This function raises the system's current task priority level to the task
  priority level of the mutual exclusion lock.  Then, it attempts to place the
  lock in the acquired state.

  @param  Lock              A pointer to the lock to acquire.

  @retval EFI_SUCCESS       The lock was acquired.
  @retval EFI_ACCESS_DENIED The lock could not be acquired because it is already owned.

**/
EFI_STATUS
EFIAPI
EfiAcquireLockOrFail (
  IN EFI_LOCK  *Lock
  )
{

  ASSERT (Lock != NULL);
  ASSERT (Lock->Lock != EfiLockUninitialized);

  if (Lock->Lock == EfiLockAcquired) {
    //
    // Lock is already owned, so bail out
    //
    return EFI_ACCESS_DENIED;
  }

  Lock->OwnerTpl = gBS->RaiseTPL (Lock->Tpl);

  Lock->Lock = EfiLockAcquired;

  return EFI_SUCCESS;
}

/**
  This function transitions a mutual exclusion lock from the acquired state to
  the released state, and restores the system's task priority level to its
  previous level.

  @param  Lock  A pointer to the lock to release.

**/
VOID
EFIAPI
EfiReleaseLock (
  IN EFI_LOCK  *Lock
  )
{
  EFI_TPL Tpl;

  ASSERT (Lock != NULL);
  ASSERT (Lock->Lock == EfiLockAcquired);

  Tpl = Lock->OwnerTpl;

  Lock->Lock = EfiLockReleased;

  gBS->RestoreTPL (Tpl);
}

/**
  Tests whether a controller handle is being managed by a specific driver.

  This function tests whether the driver specified by DriverBindingHandle is
  currently managing the controller specified by ControllerHandle.  This test
  is performed by evaluating if the the protocol specified by ProtocolGuid is
  present on ControllerHandle and is was opened by DriverBindingHandle with an
  attribute of EFI_OPEN_PROTOCOL_BY_DRIVER.
  If ProtocolGuid is NULL, then ASSERT().

  @param  ControllerHandle     A handle for a controller to test.
  @param  DriverBindingHandle  Specifies the driver binding handle for the
                               driver.
  @param  ProtocolGuid         Specifies the protocol that the driver specified
                               by DriverBindingHandle opens in its Start()
                               function.

  @retval EFI_SUCCESS          ControllerHandle is managed by the driver
                               specifed by DriverBindingHandle.
  @retval EFI_UNSUPPORTED      ControllerHandle is not managed by the driver
                               specifed by DriverBindingHandle.

**/
EFI_STATUS
EFIAPI
EfiTestManagedDevice (
  IN CONST EFI_HANDLE       ControllerHandle,
  IN CONST EFI_HANDLE       DriverBindingHandle,
  IN CONST EFI_GUID         *ProtocolGuid
  )
{
  EFI_STATUS     Status;
  VOID           *ManagedInterface;

  ASSERT (ProtocolGuid != NULL);

  Status = gBS->OpenProtocol (
                  ControllerHandle,
                  (EFI_GUID *) ProtocolGuid,
                  &ManagedInterface,
                  DriverBindingHandle,
                  ControllerHandle,
                  EFI_OPEN_PROTOCOL_BY_DRIVER
                  );
  if (!EFI_ERROR (Status)) {
    gBS->CloseProtocol (
           ControllerHandle,
           (EFI_GUID *) ProtocolGuid,
           DriverBindingHandle,
           ControllerHandle
           );
    return EFI_UNSUPPORTED;
  }

  if (Status != EFI_ALREADY_STARTED) {
    return EFI_UNSUPPORTED;
  }

  return EFI_SUCCESS;
}

/**
  Tests whether a child handle is a child device of the controller.

  This function tests whether ChildHandle is one of the children of
  ControllerHandle.  This test is performed by checking to see if the protocol
  specified by ProtocolGuid is present on ControllerHandle and opened by
  ChildHandle with an attribute of EFI_OPEN_PROTOCOL_BY_CHILD_CONTROLLER.
  If ProtocolGuid is NULL, then ASSERT().

  @param  ControllerHandle     A handle for a (parent) controller to test.
  @param  ChildHandle          A child handle to test.
  @param  ProtocolGuid         Supplies the protocol that the child controller
                               opens on its parent controller.

  @retval EFI_SUCCESS          ChildHandle is a child of the ControllerHandle.
  @retval EFI_UNSUPPORTED      ChildHandle is not a child of the
                               ControllerHandle.

**/
EFI_STATUS
EFIAPI
EfiTestChildHandle (
  IN CONST EFI_HANDLE       ControllerHandle,
  IN CONST EFI_HANDLE       ChildHandle,
  IN CONST EFI_GUID         *ProtocolGuid
  )
{
  EFI_STATUS                            Status;
  EFI_OPEN_PROTOCOL_INFORMATION_ENTRY   *OpenInfoBuffer;
  UINTN                                 EntryCount;
  UINTN                                 Index;

  ASSERT (ProtocolGuid != NULL);

  //
  // Retrieve the list of agents that are consuming the specific protocol
  // on ControllerHandle.
  //
  Status = gBS->OpenProtocolInformation (
                  ControllerHandle,
                  (EFI_GUID *) ProtocolGuid,
                  &OpenInfoBuffer,
                  &EntryCount
                  );
  if (EFI_ERROR (Status)) {
    return EFI_UNSUPPORTED;
  }

  //
  // Inspect if ChildHandle is one of the agents.
  //
  Status = EFI_UNSUPPORTED;
  for (Index = 0; Index < EntryCount; Index++) {
    if ((OpenInfoBuffer[Index].ControllerHandle == ChildHandle) &&
        (OpenInfoBuffer[Index].Attributes & EFI_OPEN_PROTOCOL_BY_CHILD_CONTROLLER) != 0) {
      Status = EFI_SUCCESS;
      break;
    }
  }

  FreePool (OpenInfoBuffer);
  return Status;
}

/**
  This function looks up a Unicode string in UnicodeStringTable.
  If Language is a member of SupportedLanguages and a Unicode
  string is found in UnicodeStringTable that matches the
  language code specified by Language, then it is returned in
  UnicodeString.

  @param  Language                A pointer to the ISO 639-2
                                  language code for the Unicode
                                  string to look up and return.
  
  @param  SupportedLanguages      A pointer to the set of ISO
                                  639-2language
                                  codes that the Unicode string
                                  table supports. Language must
                                  be a member of this set.
  
  @param  UnicodeStringTable      A pointer to the table of
                                  Unicode strings.
  
  @param  UnicodeString           A pointer to the Unicode
                                  string from UnicodeStringTable
                                  that matches the language
                                  specified by Language.

  @retval  EFI_SUCCESS            The Unicode string that
                                  matches the language specified
                                  by Language was found in the
                                  table of Unicoide strings
                                  UnicodeStringTable, and it was
                                  returned in UnicodeString.
  
  @retval  EFI_INVALID_PARAMETER  Language is NULL.
  
  @retval  EFI_INVALID_PARAMETER  UnicodeString is NULL.
  @retval  EFI_UNSUPPORTED        SupportedLanguages is NULL.
  
  @retval  EFI_UNSUPPORTED        UnicodeStringTable is NULL.
  
  @retval  EFI_UNSUPPORTED        The language specified by
                                  Language is not a member
                                  ofSupportedLanguages.
  
  @retval EFI_UNSUPPORTED         The language specified by
                                  Language is not supported by
                                  UnicodeStringTable.

**/
EFI_STATUS
EFIAPI
LookupUnicodeString (
  IN CONST CHAR8                     *Language,
  IN CONST CHAR8                     *SupportedLanguages,
  IN CONST EFI_UNICODE_STRING_TABLE  *UnicodeStringTable,
  OUT CHAR16                         **UnicodeString
  )
{
  //
  // Make sure the parameters are valid
  //
  if (Language == NULL || UnicodeString == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  //
  // If there are no supported languages, or the Unicode String Table is empty, then the
  // Unicode String specified by Language is not supported by this Unicode String Table
  //
  if (SupportedLanguages == NULL || UnicodeStringTable == NULL) {
    return EFI_UNSUPPORTED;
  }

  //
  // Make sure Language is in the set of Supported Languages
  //
  while (*SupportedLanguages != 0) {
    if (CompareIso639LanguageCode (Language, SupportedLanguages)) {

      //
      // Search the Unicode String Table for the matching Language specifier
      //
      while (UnicodeStringTable->Language != NULL) {
        if (CompareIso639LanguageCode (Language, UnicodeStringTable->Language)) {

          //
          // A matching string was found, so return it
          //
          *UnicodeString = UnicodeStringTable->UnicodeString;
          return EFI_SUCCESS;
        }

        UnicodeStringTable++;
      }

      return EFI_UNSUPPORTED;
    }

    SupportedLanguages += 3;
  }

  return EFI_UNSUPPORTED;
}



/**
  This function looks up a Unicode string in UnicodeStringTable.
  If Language is a member of SupportedLanguages and a Unicode
  string is found in UnicodeStringTable that matches the
  language code specified by Language, then it is returned in
  UnicodeString.

  @param  Language                A pointer to the ISO 639-2 or
                                  RFC 3066 language code for the
                                  Unicode string to look up and
                                  return.
  
  @param  SupportedLanguages      A pointer to the set of ISO
                                  639-2 or RFC 3066 language
                                  codes that the Unicode string
                                  table supports. Language must
                                  be a member of this set.
  
  @param  UnicodeStringTable      A pointer to the table of
                                  Unicode strings.
  
  @param  UnicodeString           A pointer to the Unicode
                                  string from UnicodeStringTable
                                  that matches the language
                                  specified by Language.

  @param  Iso639Language          Specify the language code
                                  format supported. If true,
                                  then the format follow ISO
                                  639-2. If false, then it
                                  follows RFC3066.

  @retval  EFI_SUCCESS            The Unicode string that
                                  matches the language specified
                                  by Language was found in the
                                  table of Unicoide strings
                                  UnicodeStringTable, and it was
                                  returned in UnicodeString.
  
  @retval  EFI_INVALID_PARAMETER  Language is NULL.
  
  @retval  EFI_INVALID_PARAMETER  UnicodeString is NULL.
  
  @retval  EFI_UNSUPPORTED        SupportedLanguages is NULL.
  
  @retval  EFI_UNSUPPORTED        UnicodeStringTable is NULL.
  
  @retval  EFI_UNSUPPORTED        The language specified by
                                  Language is not a member
                                  ofSupportedLanguages.
  
  @retval EFI_UNSUPPORTED         The language specified by
                                  Language is not supported by
                                  UnicodeStringTable.

**/
EFI_STATUS

EFIAPI
LookupUnicodeString2 (
  IN CONST CHAR8                     *Language,
  IN CONST CHAR8                     *SupportedLanguages,
  IN CONST EFI_UNICODE_STRING_TABLE  *UnicodeStringTable,
  OUT CHAR16                         **UnicodeString,
  IN BOOLEAN                         Iso639Language
  )
{
  BOOLEAN   Found;
  UINTN     Index;
  CHAR8     *LanguageString;

  //
  // Make sure the parameters are valid
  //
  if (Language == NULL || UnicodeString == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  //
  // If there are no supported languages, or the Unicode String Table is empty, then the
  // Unicode String specified by Language is not supported by this Unicode String Table
  //
  if (SupportedLanguages == NULL || UnicodeStringTable == NULL) {
    return EFI_UNSUPPORTED;
  }

  //
  // Make sure Language is in the set of Supported Languages
  //
  Found = FALSE;
  while (*SupportedLanguages != 0) {
    if (Iso639Language) {
      if (CompareIso639LanguageCode (Language, SupportedLanguages)) {
        Found = TRUE;
        break;
      }
      SupportedLanguages += 3;
    } else {
      for (Index = 0; SupportedLanguages[Index] != 0 && SupportedLanguages[Index] != ';'; Index++);
      if ((AsciiStrnCmp(SupportedLanguages, Language, Index) == 0) && (Language[Index] == 0)) {
        Found = TRUE;
        break;
      }
      SupportedLanguages += Index;
      for (; *SupportedLanguages != 0 && *SupportedLanguages == ';'; SupportedLanguages++);
    }
  }

  //
  // If Language is not a member of SupportedLanguages, then return EFI_UNSUPPORTED
  //
  if (!Found) {
    return EFI_UNSUPPORTED;
  }

  //
  // Search the Unicode String Table for the matching Language specifier
  //
  while (UnicodeStringTable->Language != NULL) {
    LanguageString = UnicodeStringTable->Language;
    while (0 != *LanguageString) {
      for (Index = 0 ;LanguageString[Index] != 0 && LanguageString[Index] != ';'; Index++);
      if (AsciiStrnCmp(LanguageString, Language, Index) == 0) {
        *UnicodeString = UnicodeStringTable->UnicodeString;
        return EFI_SUCCESS;
      }
      LanguageString += Index;
      for (Index = 0 ;LanguageString[Index] != 0 && LanguageString[Index] == ';'; Index++);
    }
    UnicodeStringTable++;
  }

  return EFI_UNSUPPORTED;
}


/**
  
  This function adds a Unicode string to UnicodeStringTable.
  If Language is a member of SupportedLanguages then
  UnicodeString is added to UnicodeStringTable.  New buffers are
  allocated for both Language and UnicodeString.  The contents
  of Language and UnicodeString are copied into these new
  buffers.  These buffers are automatically freed when
  FreeUnicodeStringTable() is called.

  @param  Language                A pointer to the ISO 639-2
                                  language code for the Unicode
                                  string to add.
  
  @param  SupportedLanguages      A pointer to the set of ISO
                                  639-2 language codes that the
                                  Unicode string table supports.
                                  Language must be a member of
                                  this set.
  
  @param  UnicodeStringTable      A pointer to the table of
                                  Unicode strings.
  
  @param  UnicodeString           A pointer to the Unicode
                                  string to add.

  @retval EFI_SUCCESS             The Unicode string that
                                  matches the language specified
                                  by Language was found in the
                                  table of Unicode strings
                                  UnicodeStringTable, and it was
                                  returned in UnicodeString.
  
  @retval EFI_INVALID_PARAMETER   Language is NULL.
  
  @retval EFI_INVALID_PARAMETER   UnicodeString is NULL.
  
  @retval EFI_INVALID_PARAMETER   UnicodeString is an empty string.
  
  @retval EFI_UNSUPPORTED         SupportedLanguages is NULL.
  
  @retval EFI_ALREADY_STARTED     A Unicode string with language
                                  Language is already present in
                                  UnicodeStringTable.
  
  @retval EFI_OUT_OF_RESOURCES    There is not enough memory to
                                  add another Unicode string to
                                  UnicodeStringTable.
  
  @retval EFI_UNSUPPORTED         The language specified by
                                  Language is not a member of
                                  SupportedLanguages.

**/
EFI_STATUS
EFIAPI
AddUnicodeString (
  IN CONST CHAR8               *Language,
  IN CONST CHAR8               *SupportedLanguages,
  IN EFI_UNICODE_STRING_TABLE  **UnicodeStringTable,
  IN CONST CHAR16              *UnicodeString
  )
{
  UINTN                     NumberOfEntries;
  EFI_UNICODE_STRING_TABLE  *OldUnicodeStringTable;
  EFI_UNICODE_STRING_TABLE  *NewUnicodeStringTable;
  UINTN                     UnicodeStringLength;

  //
  // Make sure the parameter are valid
  //
  if (Language == NULL || UnicodeString == NULL || UnicodeStringTable == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  //
  // If there are no supported languages, then a Unicode String can not be added
  //
  if (SupportedLanguages == NULL) {
    return EFI_UNSUPPORTED;
  }

  //
  // If the Unicode String is empty, then a Unicode String can not be added
  //
  if (UnicodeString[0] == 0) {
    return EFI_INVALID_PARAMETER;
  }

  //
  // Make sure Language is a member of SupportedLanguages
  //
  while (*SupportedLanguages != 0) {
    if (CompareIso639LanguageCode (Language, SupportedLanguages)) {

      //
      // Determine the size of the Unicode String Table by looking for a NULL Language entry
      //
      NumberOfEntries = 0;
      if (*UnicodeStringTable != NULL) {
        OldUnicodeStringTable = *UnicodeStringTable;
        while (OldUnicodeStringTable->Language != NULL) {
          if (CompareIso639LanguageCode (Language, OldUnicodeStringTable->Language)) {
            return EFI_ALREADY_STARTED;
          }

          OldUnicodeStringTable++;
          NumberOfEntries++;
        }
      }

      //
      // Allocate space for a new Unicode String Table.  It must hold the current number of
      // entries, plus 1 entry for the new Unicode String, plus 1 entry for the end of table
      // marker
      //
      NewUnicodeStringTable = AllocatePool ((NumberOfEntries + 2) * sizeof (EFI_UNICODE_STRING_TABLE));
      if (NewUnicodeStringTable == NULL) {
        return EFI_OUT_OF_RESOURCES;
      }

      //
      // If the current Unicode String Table contains any entries, then copy them to the
      // newly allocated Unicode String Table.
      //
      if (*UnicodeStringTable != NULL) {
        CopyMem (
           NewUnicodeStringTable,
           *UnicodeStringTable,
           NumberOfEntries * sizeof (EFI_UNICODE_STRING_TABLE)
           );
      }

      //
      // Allocate space for a copy of the Language specifier
      //
      NewUnicodeStringTable[NumberOfEntries].Language = AllocateCopyPool (3, Language);
      if (NewUnicodeStringTable[NumberOfEntries].Language == NULL) {
        gBS->FreePool (NewUnicodeStringTable);
        return EFI_OUT_OF_RESOURCES;
      }

      //
      // Compute the length of the Unicode String
      //
      for (UnicodeStringLength = 0; UnicodeString[UnicodeStringLength] != 0; UnicodeStringLength++)
        ;

      //
      // Allocate space for a copy of the Unicode String
      //
      NewUnicodeStringTable[NumberOfEntries].UnicodeString = AllocateCopyPool (
                                                              (UnicodeStringLength + 1) * sizeof (CHAR16),
                                                              UnicodeString
                                                              );
      if (NewUnicodeStringTable[NumberOfEntries].UnicodeString == NULL) {
        gBS->FreePool (NewUnicodeStringTable[NumberOfEntries].Language);
        gBS->FreePool (NewUnicodeStringTable);
        return EFI_OUT_OF_RESOURCES;
      }

      //
      // Mark the end of the Unicode String Table
      //
      NewUnicodeStringTable[NumberOfEntries + 1].Language       = NULL;
      NewUnicodeStringTable[NumberOfEntries + 1].UnicodeString  = NULL;

      //
      // Free the old Unicode String Table
      //
      if (*UnicodeStringTable != NULL) {
        gBS->FreePool (*UnicodeStringTable);
      }

      //
      // Point UnicodeStringTable at the newly allocated Unicode String Table
      //
      *UnicodeStringTable = NewUnicodeStringTable;

      return EFI_SUCCESS;
    }

    SupportedLanguages += 3;
  }

  return EFI_UNSUPPORTED;
}


/**
  
  This function adds a Unicode string to UnicodeStringTable.
  If Language is a member of SupportedLanguages then
  UnicodeString is added to UnicodeStringTable.  New buffers are
  allocated for both Language and UnicodeString.  The contents
  of Language and UnicodeString are copied into these new
  buffers.  These buffers are automatically freed when
  FreeUnicodeStringTable() is called.

  @param  Language                A pointer to the ISO 639-2 or
                                  RFC 3066 language code for the
                                  Unicode string to add.
  
  @param  SupportedLanguages      A pointer to the set of ISO
                                  639-2 or RFC 3.66 language
                                  codes that the Unicode string
                                  table supports. Language must
                                  be a member of this set.
  
  @param  UnicodeStringTable      A pointer to the table of
                                  Unicode strings.
  
  @param  UnicodeString           A pointer to the Unicode
                                  string to add.
  
  @param  Iso639Language          Specify the language code
                                  format supported. If true,
                                  then the format follow ISO
                                  639-2. If false, then it
                                  follows RFC3066.

  @retval EFI_SUCCESS             The Unicode string that
                                  matches the language specified
                                  by Language was found in the
                                  table of Unicode strings
                                  UnicodeStringTable, and it was
                                  returned in UnicodeString.
  
  @retval EFI_INVALID_PARAMETER   Language is NULL.
  
  @retval EFI_INVALID_PARAMETER   UnicodeString is NULL.
  
  @retval EFI_INVALID_PARAMETER   UnicodeString is an empty string.
  
  @retval EFI_UNSUPPORTED         SupportedLanguages is NULL.
  
  @retval EFI_ALREADY_STARTED     A Unicode string with language
                                  Language is already present in
                                  UnicodeStringTable.
  
  @retval EFI_OUT_OF_RESOURCES    There is not enough memory to
                                  add another Unicode string to
                                  UnicodeStringTable.
  
  @retval EFI_UNSUPPORTED         The language specified by
                                  Language is not a member of
                                  SupportedLanguages.

**/
EFI_STATUS
EFIAPI
AddUnicodeString2 (
  IN CONST CHAR8               *Language,
  IN CONST CHAR8               *SupportedLanguages,
  IN EFI_UNICODE_STRING_TABLE  **UnicodeStringTable,
  IN CONST CHAR16              *UnicodeString,
  IN BOOLEAN                   Iso639Language
  )
{
  UINTN                     NumberOfEntries;
  EFI_UNICODE_STRING_TABLE  *OldUnicodeStringTable;
  EFI_UNICODE_STRING_TABLE  *NewUnicodeStringTable;
  UINTN                     UnicodeStringLength;
  BOOLEAN                   Found;
  UINTN                     Index;
  CHAR8                     *LanguageString;

  //
  // Make sure the parameter are valid
  //
  if (Language == NULL || UnicodeString == NULL || UnicodeStringTable == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  //
  // If there are no supported languages, then a Unicode String can not be added
  //
  if (SupportedLanguages == NULL) {
    return EFI_UNSUPPORTED;
  }

  //
  // If the Unicode String is empty, then a Unicode String can not be added
  //
  if (UnicodeString[0] == 0) {
    return EFI_INVALID_PARAMETER;
  }

  //
  // Make sure Language is a member of SupportedLanguages
  //
  Found = FALSE;
  while (*SupportedLanguages != 0) {
    if (Iso639Language) {
      if (CompareIso639LanguageCode (Language, SupportedLanguages)) {
        Found = TRUE;
        break;
      }
      SupportedLanguages += 3;
    } else {
      for (Index = 0; SupportedLanguages[Index] != 0 && SupportedLanguages[Index] != ';'; Index++);
      if (AsciiStrnCmp(SupportedLanguages, Language, Index) == 0) {
        Found = TRUE;
        break;
      }
      SupportedLanguages += Index;
      for (; *SupportedLanguages != 0 && *SupportedLanguages == ';'; SupportedLanguages++);
    }
  }

  //
  // If Language is not a member of SupportedLanguages, then return EFI_UNSUPPORTED
  //
  if (!Found) {
    return EFI_UNSUPPORTED;
  }

  //
  // Determine the size of the Unicode String Table by looking for a NULL Language entry
  //
  NumberOfEntries = 0;
  if (*UnicodeStringTable != NULL) {
    OldUnicodeStringTable = *UnicodeStringTable;
    while (OldUnicodeStringTable->Language != NULL) {
      LanguageString = OldUnicodeStringTable->Language;

      while (*LanguageString != 0) {
        for (Index = 0; LanguageString[Index] != 0 && LanguageString[Index] != ';'; Index++);

        if (AsciiStrnCmp (Language, LanguageString, Index) == 0) { 
          return EFI_ALREADY_STARTED;
        }
        LanguageString += Index;
        for (; *LanguageString != 0 && *LanguageString == ';'; LanguageString++);
      }
      OldUnicodeStringTable++;
      NumberOfEntries++;
    }
  }

  //
  // Allocate space for a new Unicode String Table.  It must hold the current number of
  // entries, plus 1 entry for the new Unicode String, plus 1 entry for the end of table
  // marker
  //
  NewUnicodeStringTable = AllocatePool ((NumberOfEntries + 2) * sizeof (EFI_UNICODE_STRING_TABLE));
  if (NewUnicodeStringTable == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }

  //
  // If the current Unicode String Table contains any entries, then copy them to the
  // newly allocated Unicode String Table.
  //
  if (*UnicodeStringTable != NULL) {
    CopyMem (
      NewUnicodeStringTable,
      *UnicodeStringTable,
      NumberOfEntries * sizeof (EFI_UNICODE_STRING_TABLE)
      );
  }

  //
  // Allocate space for a copy of the Language specifier
  //
  NewUnicodeStringTable[NumberOfEntries].Language = AllocateCopyPool (AsciiStrSize(Language), Language);
  if (NewUnicodeStringTable[NumberOfEntries].Language == NULL) {
    gBS->FreePool (NewUnicodeStringTable);
    return EFI_OUT_OF_RESOURCES;
  }

  //
  // Compute the length of the Unicode String
  //
  for (UnicodeStringLength = 0; UnicodeString[UnicodeStringLength] != 0; UnicodeStringLength++);

  //
  // Allocate space for a copy of the Unicode String
  //
  NewUnicodeStringTable[NumberOfEntries].UnicodeString = AllocateCopyPool (StrSize (UnicodeString), UnicodeString);
  if (NewUnicodeStringTable[NumberOfEntries].UnicodeString == NULL) {
    gBS->FreePool (NewUnicodeStringTable[NumberOfEntries].Language);
    gBS->FreePool (NewUnicodeStringTable);
    return EFI_OUT_OF_RESOURCES;
  }

  //
  // Mark the end of the Unicode String Table
  //
  NewUnicodeStringTable[NumberOfEntries + 1].Language       = NULL;
  NewUnicodeStringTable[NumberOfEntries + 1].UnicodeString  = NULL;

  //
  // Free the old Unicode String Table
  //
  if (*UnicodeStringTable != NULL) {
    gBS->FreePool (*UnicodeStringTable);
  }

  //
  // Point UnicodeStringTable at the newly allocated Unicode String Table
  //
  *UnicodeStringTable = NewUnicodeStringTable;

  return EFI_SUCCESS;
}

/**
  This function frees the table of Unicode strings in UnicodeStringTable.
  If UnicodeStringTable is NULL, then EFI_SUCCESS is returned.
  Otherwise, each language code, and each Unicode string in the Unicode string 
  table are freed, and EFI_SUCCESS is returned.

  @param  UnicodeStringTable  A pointer to the table of Unicode strings.

  @retval EFI_SUCCESS         The Unicode string table was freed.

**/
EFI_STATUS
EFIAPI
FreeUnicodeStringTable (
  IN EFI_UNICODE_STRING_TABLE  *UnicodeStringTable
  )
{
  UINTN Index;

  //
  // If the Unicode String Table is NULL, then it is already freed
  //
  if (UnicodeStringTable == NULL) {
    return EFI_SUCCESS;
  }

  //
  // Loop through the Unicode String Table until we reach the end of table marker
  //
  for (Index = 0; UnicodeStringTable[Index].Language != NULL; Index++) {

    //
    // Free the Language string from the Unicode String Table
    //
    gBS->FreePool (UnicodeStringTable[Index].Language);

    //
    // Free the Unicode String from the Unicode String Table
    //
    if (UnicodeStringTable[Index].UnicodeString != NULL) {
      gBS->FreePool (UnicodeStringTable[Index].UnicodeString);
    }
  }

  //
  // Free the Unicode String Table itself
  //
  gBS->FreePool (UnicodeStringTable);

  return EFI_SUCCESS;
}

/**
  Determine what is the current language setting. The space reserved for Lang
  must be at least RFC_3066_ENTRY_SIZE bytes;

  If Lang is NULL, then ASSERT.

  @param  Lang                   Pointer of system language. Lang will always be filled with 
                                         a valid RFC 3066 language string. If "PlatformLang" is not
                                         set in the system, the default language specifed by PcdUefiVariableDefaultPlatformLang
                                         is returned.

  @return  EFI_SUCCESS     If the EFI Variable with "PlatformLang" is set and return in Lang.
  @return  EFI_NOT_FOUND If the EFI Variable with "PlatformLang" is not set, but a valid default language is return in Lang.

**/
EFI_STATUS
EFIAPI
GetCurrentLanguage (
  OUT     CHAR8               *Lang
  )
{
  EFI_STATUS  Status;
  UINTN       Size;

  ASSERT (Lang != NULL);

  //
  // Get current language setting
  //
  Size = RFC_3066_ENTRY_SIZE;
  Status = gRT->GetVariable (
                  L"PlatformLang",
                  &gEfiGlobalVariableGuid,
                  NULL,
                  &Size,
                  Lang
                  );

  if (EFI_ERROR (Status)) {
    AsciiStrCpy (Lang, (CHAR8 *) PcdGetPtr (PcdUefiVariableDefaultPlatformLang));
  }

  return Status;
}


