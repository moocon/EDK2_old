/*++

Copyright (c) 2006, Intel Corporation                                                         
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution.  The full text of the license may be found at        
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module Name:

  ComponentName.c

Abstract:

--*/

//
// The package level header files this module uses
//
#include <PiDxe.h>

//
// The protocols, PPI and GUID defintions for this module
//
#include <Protocol/ScsiIo.h>
#include <Protocol/ComponentName.h>
#include <Protocol/BlockIo.h>
#include <Protocol/DriverBinding.h>
//
// The Library classes this module consumes
//
#include <Library/DebugLib.h>
#include <Library/UefiDriverEntryPoint.h>
#include <Library/UefiLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/ScsiLib.h>
#include <Library/UefiBootServicesTableLib.h>

#include "ScsiDisk.h"

//
// EFI Component Name Protocol
//
EFI_COMPONENT_NAME_PROTOCOL     gScsiDiskComponentName = {
  ScsiDiskComponentNameGetDriverName,
  ScsiDiskComponentNameGetControllerName,
  "eng"
};

static EFI_UNICODE_STRING_TABLE mScsiDiskDriverNameTable[] = {
  { "eng", (CHAR16 *) L"Scsi Disk Driver" },
  { NULL , NULL }
};

EFI_STATUS
EFIAPI
ScsiDiskComponentNameGetDriverName (
  IN  EFI_COMPONENT_NAME_PROTOCOL  *This,
  IN  CHAR8                        *Language,
  OUT CHAR16                       **DriverName
  )
/*++

  Routine Description:
    Retrieves a Unicode string that is the user readable name of the EFI Driver.

  Arguments:
    This       - A pointer to the EFI_COMPONENT_NAME_PROTOCOL instance.
    Language   - A pointer to a three character ISO 639-2 language identifier.
                 This is the language of the driver name that that the caller 
                 is requesting, and it must match one of the languages specified
                 in SupportedLanguages.  The number of languages supported by a 
                 driver is up to the driver writer.
    DriverName - A pointer to the Unicode string to return.  This Unicode string
                 is the name of the driver specified by This in the language 
                 specified by Language.

  Returns:
    EFI_SUCCESS           - The Unicode string for the Driver specified by This
                            and the language specified by Language was returned 
                            in DriverName.
    EFI_INVALID_PARAMETER - Language is NULL.
    EFI_INVALID_PARAMETER - DriverName is NULL.
    EFI_UNSUPPORTED       - The driver specified by This does not support the 
                            language specified by Language.

--*/
{
  return LookupUnicodeString (
          Language,
          gScsiDiskComponentName.SupportedLanguages,
          mScsiDiskDriverNameTable,
          DriverName
          );
}

EFI_STATUS
EFIAPI
ScsiDiskComponentNameGetControllerName (
  IN  EFI_COMPONENT_NAME_PROTOCOL                     *This,
  IN  EFI_HANDLE                                      ControllerHandle,
  IN  EFI_HANDLE                                      ChildHandle        OPTIONAL,
  IN  CHAR8                                           *Language,
  OUT CHAR16                                          **ControllerName
  )
/*++

  Routine Description:
    Retrieves a Unicode string that is the user readable name of the controller
    that is being managed by an EFI Driver.

  Arguments:
    This             - A pointer to the EFI_COMPONENT_NAME_PROTOCOL instance.
    ControllerHandle - The handle of a controller that the driver specified by 
                       This is managing.  This handle specifies the controller 
                       whose name is to be returned.
    ChildHandle      - The handle of the child controller to retrieve the name 
                       of.  This is an optional parameter that may be NULL.  It 
                       will be NULL for device drivers.  It will also be NULL 
                       for a bus drivers that wish to retrieve the name of the 
                       bus controller.  It will not be NULL for a bus driver 
                       that wishes to retrieve the name of a child controller.
    Language         - A pointer to a three character ISO 639-2 language 
                       identifier.  This is the language of the controller name 
                       that that the caller is requesting, and it must match one
                       of the languages specified in SupportedLanguages.  The 
                       number of languages supported by a driver is up to the 
                       driver writer.
    ControllerName   - A pointer to the Unicode string to return.  This Unicode
                       string is the name of the controller specified by 
                       ControllerHandle and ChildHandle in the language 
                       specified by Language from the point of view of the 
                       driver specified by This. 

  Returns:
    EFI_SUCCESS           - The Unicode string for the user readable name in the
                            language specified by Language for the driver 
                            specified by This was returned in DriverName.
    EFI_INVALID_PARAMETER - ControllerHandle is not a valid EFI_HANDLE.
    EFI_INVALID_PARAMETER - ChildHandle is not NULL and it is not a valid 
                            EFI_HANDLE.
    EFI_INVALID_PARAMETER - Language is NULL.
    EFI_INVALID_PARAMETER - ControllerName is NULL.
    EFI_UNSUPPORTED       - The driver specified by This is not currently 
                            managing the controller specified by 
                            ControllerHandle and ChildHandle.
    EFI_UNSUPPORTED       - The driver specified by This does not support the 
                            language specified by Language.

--*/
{
  EFI_STATUS            Status;
  SCSI_DISK_DEV         *ScsiDiskDevice;
  EFI_BLOCK_IO_PROTOCOL *BlockIo;

  //
  // This is a device driver, so ChildHandle must be NULL.
  //
  if (ChildHandle != NULL) {
    return EFI_UNSUPPORTED;
  }
  
  //
  // Make sure this driver is currently managing ControllerHandle
  //
  Status = EfiTestManagedDevice (
             ControllerHandle,
             gScsiDiskDriverBinding.DriverBindingHandle,
             &gEfiScsiIoProtocolGuid
             );
  if (EFI_ERROR (Status)) {
    return Status;
  }
  //
  // Get the device context
  //
  Status = gBS->OpenProtocol (
                  ControllerHandle,
                  &gEfiBlockIoProtocolGuid,
                  (VOID **) &BlockIo,
                  gScsiDiskDriverBinding.DriverBindingHandle,
                  ControllerHandle,
                  EFI_OPEN_PROTOCOL_GET_PROTOCOL
                  );

  if (EFI_ERROR (Status)) {
    return Status;
  }

  ScsiDiskDevice = SCSI_DISK_DEV_FROM_THIS (BlockIo);

  return LookupUnicodeString (
          Language,
          gScsiDiskComponentName.SupportedLanguages,
          ScsiDiskDevice->ControllerNameTable,
          ControllerName
          );

}
