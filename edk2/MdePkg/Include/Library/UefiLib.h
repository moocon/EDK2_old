/** @file
  MDE UEFI library functions and macros

  Copyright (c) 2006 - 2007, Intel Corporation                                                         
  All rights reserved. This program and the accompanying materials                          
  are licensed and made available under the terms and conditions of the BSD License         
  which accompanies this distribution.  The full text of the license may be found at        
  http://opensource.org/licenses/bsd-license.php                                            

  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

**/

#ifndef __UEFI_LIB_H__
#define __UEFI_LIB_H__

#include <Protocol/DriverBinding.h>
#include <Protocol/DriverConfiguration.h>
#include <Protocol/ComponentName.h>
#include <Protocol/ComponentName2.h>
#include <Protocol/DriverDiagnostics.h>
#include <Protocol/DriverDiagnostics2.h>

//
// Unicode String Table
//
typedef struct {
  CHAR8   *Language;
  CHAR16  *UnicodeString;
} EFI_UNICODE_STRING_TABLE;

//
// EFI Lock Status
//
typedef enum {
  EfiLockUninitialized = 0,
  EfiLockReleased      = 1,
  EfiLockAcquired      = 2
} EFI_LOCK_STATE;

//
// EFI Lock 
//
typedef struct {
  EFI_TPL         Tpl;
  EFI_TPL         OwnerTpl;
  EFI_LOCK_STATE  Lock;
} EFI_LOCK;


/**
  This function searches the list of configuration tables stored in the EFI System 
  Table for a table with a GUID that matches TableGuid.  If a match is found, 
  then a pointer to the configuration table is returned in Table, and EFI_SUCCESS 
  is returned.  If a matching GUID is not found, then EFI_NOT_FOUND is returned.

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
  );

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
  );

/**
  This function creates an event using NotifyTpl, NoifyFunction, and NotifyContext.
  This event is signaled with EfiNamedEventSignal().  This provide the ability for 
  one or more listeners on the same event named by the GUID specified by Name.

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
  );

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
  );

/** 
  Returns the current TPL.

  This function returns the current TPL.  There is no EFI service to directly 
  retrieve the current TPL. Instead, the RaiseTPL() function is used to raise 
  the TPL to TPL_HIGH_LEVEL.  This will return the current TPL.  The TPL level 
  can then immediately be restored back to the current TPL level with a call 
  to RestoreTPL().

  @param  VOID

  @retvale EFI_TPL              The current TPL.

**/
EFI_TPL
EFIAPI
EfiGetCurrentTpl (
  VOID
  );

/**
  This function initializes a basic mutual exclusion lock to the released state 
  and returns the lock.  Each lock provides mutual exclusion access at its task 
  priority level.  Since there is no preemption or multiprocessor support in EFI,
  acquiring the lock only consists of raising to the locks TPL.

  @param  Lock       A pointer to the lock data structure to initialize.
  @param  Priority   EFI TPL associated with the lock.

  @return The lock.

**/
EFI_LOCK *
EFIAPI
EfiInitializeLock (
  IN OUT EFI_LOCK  *Lock,
  IN EFI_TPL        Priority
  );

/**
  This macro initializes the contents of a basic mutual exclusion lock to the 
  released state.  Each lock provides mutual exclusion access at its task 
  priority level.  Since there is no preemption or multiprocessor support in EFI,
  acquiring the lock only consists of raising to the locks TPL.

  @param  Lock      A pointer to the lock data structure to initialize.
  @param  Priority  The task priority level of the lock.

  @return The lock.

**/
#define EFI_INITIALIZE_LOCK_VARIABLE(Priority) \
  {Priority, EFI_TPL_APPLICATION, EfiLockReleased }


/**
  
  Macro that calls DebugAssert() if an EFI_LOCK structure is not in the locked state.

  If the DEBUG_PROPERTY_DEBUG_ASSERT_ENABLED bit of PcdDebugProperyMask is set, 
  then this macro evaluates the EFI_LOCK structure specified by Lock.  If Lock 
  is not in the locked state, then DebugAssert() is called passing in the source 
  filename, source line number, and Lock.

  If Lock is NULL, then ASSERT().

  @param  LockParameter  A pointer to the lock to acquire.

**/
#define ASSERT_LOCKED(LockParameter)                  \
  do {                                                \
    if (DebugAssertEnabled ()) {                      \
      ASSERT (LockParameter != NULL);                 \
      if ((LockParameter)->Lock != EfiLockAcquired) { \
        _ASSERT (LockParameter not locked);           \
      }                                               \
    }                                                 \
  } while (FALSE)


/**
  This function raises the system's current task priority level to the task 
  priority level of the mutual exclusion lock.  Then, it places the lock in the 
  acquired state.

  @param  Priority  The task priority level of the lock.

**/
VOID
EFIAPI
EfiAcquireLock (
  IN EFI_LOCK  *Lock
  );

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
  );

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
  );

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
  );

/**
  Tests whether a child handle is a child device of the controller.

  This function tests whether ChildHandle is one of the children of
  ControllerHandle.  This test is performed by checking to see if the protocol
  specified by ProtocolGuid is present on ControllerHandle and opened by
  ChildHandle with an attribute of EFI_OPEN_PROTOCOL_BY_CHILD_CONTROLLER.
  If ProtocolGuid is NULL, then ASSERT().

  @param  ControllerHandle     A handle for a (parent) controller to test. 
  @param  ChildHandle          A child handle to test.
  @param  ConsumsedGuid        Supplies the protocol that the child controller
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
  );

/**
  This function looks up a Unicode string in UnicodeStringTable.  If Language is 
  a member of SupportedLanguages and a Unicode string is found in UnicodeStringTable
  that matches the language code specified by Language, then it is returned in 
  UnicodeString.

  @param  Language                A pointer to the ISO 639-2 language code for the 
                                  Unicode string to look up and return.
  @param  SupportedLanguages      A pointer to the set of ISO 639-2 language codes 
                                  that the Unicode string table supports.  Language 
                                  must be a member of this set.
  @param  UnicodeStringTable      A pointer to the table of Unicode strings.
  @param  UnicodeString           A pointer to the Unicode string from UnicodeStringTable
                                  that matches the language specified by Language.

  @retval EFI_SUCCESS             The Unicode string that matches the language 
                                  specified by Language was found
                                  in the table of Unicoide strings UnicodeStringTable, 
                                  and it was returned in UnicodeString.
  @retval EFI_INVALID_PARAMETER   Language is NULL.
  @retval EFI_INVALID_PARAMETER   UnicodeString is NULL.
  @retval EFI_UNSUPPORTED         SupportedLanguages is NULL.
  @retval EFI_UNSUPPORTED         UnicodeStringTable is NULL.
  @retval EFI_UNSUPPORTED         The language specified by Language is not a 
                                  member of SupportedLanguages.
  @retval EFI_UNSUPPORTED         The language specified by Language is not 
                                  supported by UnicodeStringTable.

**/
EFI_STATUS
EFIAPI
LookupUnicodeString (
  IN CONST CHAR8                     *Language,
  IN CONST CHAR8                     *SupportedLanguages,
  IN CONST EFI_UNICODE_STRING_TABLE  *UnicodeStringTable,
  OUT CHAR16                         **UnicodeString
  );

/**
  This function adds a Unicode string to UnicodeStringTable.
  If Language is a member of SupportedLanguages then UnicodeString is added to 
  UnicodeStringTable.  New buffers are allocated for both Language and 
  UnicodeString.  The contents of Language and UnicodeString are copied into 
  these new buffers.  These buffers are automatically freed when 
  FreeUnicodeStringTable() is called.

  @param  Language                A pointer to the ISO 639-2 language code for the Unicode 
                                  string to add.
  @param  SupportedLanguages      A pointer to the set of ISO 639-2 language codes
                                  that the Unicode string table supports.
                                  Language must be a member of this set.
  @param  UnicodeStringTable      A pointer to the table of Unicode strings.
  @param  UnicodeString           A pointer to the Unicode string to add.

  @retval EFI_SUCCESS             The Unicode string that matches the language 
                                  specified by Language was found in the table of 
                                  Unicode strings UnicodeStringTable, and it was 
                                  returned in UnicodeString.
  @retval EFI_INVALID_PARAMETER   Language is NULL.
  @retval EFI_INVALID_PARAMETER   UnicodeString is NULL.
  @retval EFI_INVALID_PARAMETER   UnicodeString is an empty string.
  @retval EFI_UNSUPPORTED         SupportedLanguages is NULL.
  @retval EFI_ALREADY_STARTED     A Unicode string with language Language is 
                                  already present in UnicodeStringTable.
  @retval EFI_OUT_OF_RESOURCES    There is not enough memory to add another 
                                  Unicode string to UnicodeStringTable.
  @retval EFI_UNSUPPORTED         The language specified by Language is not a 
                                  member of SupportedLanguages.

**/
EFI_STATUS
EFIAPI
AddUnicodeString (
  IN CONST CHAR8               *Language,
  IN CONST CHAR8               *SupportedLanguages,
  IN EFI_UNICODE_STRING_TABLE  **UnicodeStringTable,
  IN CONST CHAR16              *UnicodeString
  );

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
  );

/**
  This function computes and returns the width of the Unicode character 
  specified by UnicodeChar.

  @param  UnicodeChar   A Unicode character.

  @retval 0             The width if UnicodeChar could not be determined.
  @retval 1             UnicodeChar is a narrow glyph.
  @retval 2             UnicodeChar is a wide glyph.

**/
UINTN
EFIAPI
GetGlyphWidth (
  IN CHAR16  UnicodeChar
  );

/**
  This function computes and returns the display length of
  the Null-terminated Unicode string specified by String.
  If String is NULL, then 0 is returned.
  If any of the widths of the Unicode characters in String
  can not be determined, then 0 is returned.

  @param  String      A pointer to a Null-terminated Unicode string.

  @return The display length of the Null-terminated Unicode string specified by String.
  
**/
UINTN
EFIAPI
UnicodeStringDisplayLength (
  IN CONST CHAR16  *String
  );

//
// Functions that abstract early Framework contamination of UEFI.
//
/**
  Signal a Ready to Boot Event.  
  
  Create a Ready to Boot Event. Signal it and close it. This causes other 
  events of the same event group to be signaled in other modules. 

**/
VOID
EFIAPI
EfiSignalEventReadyToBoot (
  VOID
  );

/**
  Signal a Legacy Boot Event.  
  
  Create a legacy Boot Event. Signal it and close it. This causes other 
  events of the same event group to be signaled in other modules. 

**/
VOID
EFIAPI
EfiSignalEventLegacyBoot (
  VOID
  );

/**
  Create a Legacy Boot Event.  
  
  Tiano extended the CreateEvent Type enum to add a legacy boot event type. 
  This was bad as Tiano did not own the enum. In UEFI 2.0 CreateEventEx was
  added and now it's possible to not voilate the UEFI specification by 
  declaring a GUID for the legacy boot event class. This library supports
  the EDK/EFI 1.10 form and EDK II/UEFI 2.0 form and allows common code to 
  work both ways.

  @param  LegacyBootEvent   Returns the EFI event returned from gBS->CreateEvent(Ex).

  @retval EFI_SUCCESS       Event was created.
  @retval Other             Event was not created.

**/
EFI_STATUS
EFIAPI
EfiCreateEventLegacyBoot (
  OUT EFI_EVENT  *LegacyBootEvent
  );

/**
  Create an EFI event in the Legacy Boot Event Group and allows
  the caller to specify a notification function.  
  
  This function abstracts the creation of the Legacy Boot Event.
  The Framework moved from a proprietary to UEFI 2.0 based mechanism.
  This library abstracts the caller from how this event is created to prevent
  to code form having to change with the version of the specification supported.
  If LegacyBootEvent is NULL, then ASSERT().

  @param  NotifyTpl         The task priority level of the event.
  @param  NotifyFunction    The notification function to call when the event is signaled.
  @param  NotifyContext     The content to pass to NotifyFunction when the event is signaled.
  @param  LegacyBootEvent   Returns the EFI event returned from gBS->CreateEvent(Ex).

  @retval EFI_SUCCESS       Event was created.
  @retval Other             Event was not created.

**/
EFI_STATUS
EFIAPI
EfiCreateEventLegacyBootEx (
  IN  EFI_TPL           NotifyTpl,
  IN  EFI_EVENT_NOTIFY  NotifyFunction,  OPTIONAL
  IN  VOID              *NotifyContext,  OPTIONAL
  OUT EFI_EVENT         *LegacyBootEvent
  );

/**
  Create a Read to Boot Event.  
  
  Tiano extended the CreateEvent Type enum to add a ready to boot event type. 
  This was bad as Tiano did not own the enum. In UEFI 2.0 CreateEventEx was
  added and now it's possible to not voilate the UEFI specification and use 
  the ready to boot event class defined in UEFI 2.0. This library supports
  the EDK/EFI 1.10 form and EDKII/UEFI 2.0 form and allows common code to 
  work both ways.

  @param  LegacyBootEvent   Returns the EFI event returned from gBS->CreateEvent(Ex).

  @retval EFI_SUCCESS       Event was created.
  @retval Other             Event was not created.

**/
EFI_STATUS
EFIAPI
EfiCreateEventReadyToBoot (
  OUT EFI_EVENT  *ReadyToBootEvent
  );

/**
  Create an EFI event in the Ready To Boot Event Group and allows
  the caller to specify a notification function.  
  
  This function abstracts the creation of the Ready to Boot Event.
  The Framework moved from a proprietary to UEFI 2.0 based mechanism.
  This library abstracts the caller from how this event is created to prevent
  to code form having to change with the version of the specification supported.
  If ReadyToBootEvent is NULL, then ASSERT().

  @param  NotifyTpl         The task priority level of the event.
  @param  NotifyFunction    The notification function to call when the event is signaled.
  @param  NotifyContext     The content to pass to NotifyFunction when the event is signaled.
  @param  LegacyBootEvent   Returns the EFI event returned from gBS->CreateEvent(Ex).

  @retval EFI_SUCCESS       Event was created.
  @retval Other             Event was not created.

**/
EFI_STATUS
EFIAPI
EfiCreateEventReadyToBootEx (
  IN  EFI_TPL           NotifyTpl,
  IN  EFI_EVENT_NOTIFY  NotifyFunction,  OPTIONAL
  IN  VOID              *NotifyContext,  OPTIONAL
  OUT EFI_EVENT         *ReadyToBootEvent
  );

/**
  Initialize a Firmware Volume (FV) Media Device Path node.
  
  Tiano extended the EFI 1.10 device path nodes. Tiano does not own this enum
  so as we move to UEFI 2.0 support we must use a mechanism that conforms with
  the UEFI 2.0 specification to define the FV device path. An UEFI GUIDed 
  device path is defined for Tiano extensions of device path. If the code 
  is compiled to conform with the UEFI 2.0 specification use the new device path
  else use the old form for backwards compatability.

  @param  FvDevicePathNode  Pointer to a FV device path node to initialize
  @param  NameGuid          FV file name to use in FvDevicePathNode

**/
VOID
EFIAPI
EfiInitializeFwVolDevicepathNode (
  IN OUT MEDIA_FW_VOL_FILEPATH_DEVICE_PATH  *FvDevicePathNode,
  IN CONST EFI_GUID                         *NameGuid
  );

/**
  Check to see if the Firmware Volume (FV) Media Device Path is valid 
  
  Tiano extended the EFI 1.10 device path nodes. Tiano does not own this enum
  so as we move to UEFI 2.0 support we must use a mechanism that conforms with
  the UEFI 2.0 specification to define the FV device path. An UEFI GUIDed 
  device path is defined for Tiano extensions of device path. If the code 
  is compiled to conform with the UEFI 2.0 specification use the new device path
  else use the old form for backwards compatability. The return value to this
  function points to a location in FvDevicePathNode and it does not allocate
  new memory for the GUID pointer that is returned.

  @param  FvDevicePathNode  Pointer to FV device path to check.

  @retval NULL              FvDevicePathNode is not valid.
  @retval Other             FvDevicePathNode is valid and pointer to NameGuid was returned.

**/
EFI_GUID *
EFIAPI
EfiGetNameGuidFromFwVolDevicePathNode (
  IN CONST MEDIA_FW_VOL_FILEPATH_DEVICE_PATH  *FvDevicePathNode
  );

/** 
  Prints a formatted Unicode string to the console output device specified by 
  ConOut defined in the EFI_SYSTEM_TABLE.

  This function prints a formatted Unicode string to the console output device 
  specified by ConOut in EFI_SYSTEM_TABLE and returns the number of Unicode 
  characters that printed to ConOut.  If the length of the formatted Unicode 
  string is greater than PcdUefiLibMaxPrintBufferSize, then only the first 
  PcdUefiLibMaxPrintBufferSize characters are sent to ConOut.

  @param Format   Null-terminated Unicode format string.
  @param ...      VARARG list consumed to process Format.
  If Format is NULL, then ASSERT().
  If Format is not aligned on a 16-bit boundary, then ASSERT().

**/
UINTN
EFIAPI
Print (
  IN CONST CHAR16  *Format,
  ...
  );

/** 
  Prints a formatted Unicode string to the console output device specified by 
  StdErr defined in the EFI_SYSTEM_TABLE.

  This function prints a formatted Unicode string to the console output device 
  specified by StdErr in EFI_SYSTEM_TABLE and returns the number of Unicode 
  characters that printed to StdErr.  If the length of the formatted Unicode 
  string is greater than PcdUefiLibMaxPrintBufferSize, then only the first 
  PcdUefiLibMaxPrintBufferSize characters are sent to StdErr.

  @param Format   Null-terminated Unicode format string.
  @param ...      VARARG list consumed to process Format.
  If Format is NULL, then ASSERT().
  If Format is not aligned on a 16-bit boundary, then ASSERT().

**/
UINTN
EFIAPI
ErrorPrint (
  IN CONST CHAR16  *Format,
  ...
  );

/** 
  Prints a formatted ASCII string to the console output device specified by 
  ConOut defined in the EFI_SYSTEM_TABLE.

  This function prints a formatted ASCII string to the console output device 
  specified by ConOut in EFI_SYSTEM_TABLE and returns the number of ASCII 
  characters that printed to ConOut.  If the length of the formatted ASCII 
  string is greater than PcdUefiLibMaxPrintBufferSize, then only the first 
  PcdUefiLibMaxPrintBufferSize characters are sent to ConOut.

  @param Format   Null-terminated ASCII format string.
  @param ...      VARARG list consumed to process Format.
  If Format is NULL, then ASSERT().
  If Format is not aligned on a 16-bit boundary, then ASSERT().

**/
UINTN
EFIAPI
AsciiPrint (
  IN CONST CHAR8  *Format,
  ...
  );

/** 
  Prints a formatted ASCII string to the console output device specified by 
  StdErr defined in the EFI_SYSTEM_TABLE.

  This function prints a formatted ASCII string to the console output device 
  specified by StdErr in EFI_SYSTEM_TABLE and returns the number of ASCII 
  characters that printed to StdErr.  If the length of the formatted ASCII 
  string is greater than PcdUefiLibMaxPrintBufferSize, then only the first 
  PcdUefiLibMaxPrintBufferSize characters are sent to StdErr.

  @param Format   Null-terminated ASCII format string.
  @param ...      VARARG list consumed to process Format.
  If Format is NULL, then ASSERT().
  If Format is not aligned on a 16-bit boundary, then ASSERT().

**/
UINTN
EFIAPI
AsciiErrorPrint (
  IN CONST CHAR8  *Format,
  ...
  );

/**
  Intialize a driver by installing the Driver Binding Protocol onto the
  driver's DriverBindingHandle.  This is typically the same as the driver's
  ImageHandle, but it can be different if the driver produces multiple
  DriverBinding Protocols.  This function also initializes the EFI Driver
  Library that initializes the global variables gST, gBS, gRT.

  @param  ImageHandle          The image handle of the driver
  @param  SystemTable          The EFI System Table that was passed to the driver's entry point
  @param  DriverBinding        A Driver Binding Protocol instance that this driver is producing
  @param  DriverBindingHandle  The handle that DriverBinding is to be installe onto.  If this
                               parameter is NULL, then a new handle is created.

  @retval EFI_SUCCESS          DriverBinding is installed onto DriverBindingHandle
  @retval Other                Status from gBS->InstallProtocolInterface()

**/
EFI_STATUS
EFIAPI
EfiLibInstallDriverBinding (
  IN CONST EFI_HANDLE             ImageHandle,
  IN CONST EFI_SYSTEM_TABLE       *SystemTable,
  IN EFI_DRIVER_BINDING_PROTOCOL  *DriverBinding,
  IN EFI_HANDLE                   DriverBindingHandle
  );

/**
  Intialize a driver by installing the Driver Binding Protocol onto the
  driver's DriverBindingHandle.  This is typically the same as the driver's
  ImageHandle, but it can be different if the driver produces multiple
  DriverBinding Protocols.  This function also initializes the EFI Driver
  Library that initializes the global variables gST, gBS, gRT.

  @ImageHandle                 The image handle of the driver
  @SystemTable                 The EFI System Table that was passed to the driver's entry point
  @DriverBinding               A Driver Binding Protocol instance that this driver is producing
  @DriverBindingHandle         The handle that DriverBinding is to be installe onto.  If this
                               parameter is NULL, then a new handle is created.
  @ComponentName               A Component Name Protocol instance that this driver is producing
  @DriverConfiguration         A Driver Configuration Protocol instance that this driver is producing
  @DriverDiagnostics           A Driver Diagnostics Protocol instance that this driver is producing

  @retval EFI_SUCCESS          DriverBinding is installed onto DriverBindingHandle
  @retval Other                Status from gBS->InstallProtocolInterface()

**/
EFI_STATUS
EFIAPI
EfiLibInstallAllDriverProtocols (
  IN CONST EFI_HANDLE                         ImageHandle,
  IN CONST EFI_SYSTEM_TABLE                   *SystemTable,
  IN EFI_DRIVER_BINDING_PROTOCOL              *DriverBinding,
  IN EFI_HANDLE                               DriverBindingHandle,
  IN CONST EFI_COMPONENT_NAME_PROTOCOL        *ComponentName,       OPTIONAL
  IN CONST EFI_DRIVER_CONFIGURATION_PROTOCOL  *DriverConfiguration, OPTIONAL
  IN CONST EFI_DRIVER_DIAGNOSTICS_PROTOCOL    *DriverDiagnostics    OPTIONAL
  );


/**
  Intialize a driver by installing the Driver Binding Protocol
  onto the driver's DriverBindingHandle.  This is typically the
  same as the driver's ImageHandle, but it can be different if
  the driver produces multiple DriverBinding Protocols.  This
  function also initializes the EFI Driver Library that
  initializes the global variables gST, gBS, gRT.

  @ImageHandle                  The image handle of the driver
  
  @SystemTable                  The EFI System Table that was
                                passed to the driver's entry
                                point
  
  @DriverBinding                A Driver Binding Protocol
                                instance that this driver is
                                producing
  
  @DriverBindingHandle          The handle that DriverBinding is
                                to be installe onto.  If this
                                parameter is NULL, then a new
                                handle is created.
  
  @ComponentName                A Component Name Protocol
                                instance that this driver is
                                producing
  
  @ComponentName2               A Component Name 2 Protocol
                                instance that this driver is
                                producing
  
  @DriverConfiguration          A Driver Configuration Protocol
                                instance that this driver is
                                producing
  
  @DriverDiagnostics            A Driver Diagnostics Protocol
                                instance that this driver is
                                producing
  
  @DriverDiagnostics2           A Driver Diagnostics 2 Protocol
                                instance that this driver is
                                producing

  @retval EFI_SUCCESS          DriverBinding is installed onto DriverBindingHandle
  @retval Other                Status from gBS->InstallProtocolInterface()

**/
EFI_STATUS
EFIAPI
EfiLibInstallAllDriverProtocols2 (
  IN CONST EFI_HANDLE                         ImageHandle,
  IN CONST EFI_SYSTEM_TABLE                   *SystemTable,
  IN EFI_DRIVER_BINDING_PROTOCOL              *DriverBinding,
  IN EFI_HANDLE                               DriverBindingHandle,
  IN CONST EFI_COMPONENT_NAME_PROTOCOL        *ComponentName,       OPTIONAL
  IN CONST EFI_COMPONENT_NAME2_PROTOCOL       *ComponentName2,      OPTIONAL
  IN CONST EFI_DRIVER_CONFIGURATION_PROTOCOL  *DriverConfiguration, OPTIONAL
  IN CONST EFI_DRIVER_DIAGNOSTICS_PROTOCOL    *DriverDiagnostics,   OPTIONAL
  IN CONST EFI_DRIVER_DIAGNOSTICS2_PROTOCOL   *DriverDiagnostics2   OPTIONAL
  );


#endif
