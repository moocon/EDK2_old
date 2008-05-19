/** @file
Module produce EFI_PEI_READ_ONLY_VARIABLE2_PPI on top of EFI_PEI_READ_ONLY_VARIABLE_PPI.
UEFI PI Spec supersedes Intel's Framework Specs. 
EFI_PEI_READ_ONLY_VARIABLE_PPI defined in Intel Framework Pkg is replaced by EFI_PEI_READ_ONLY_VARIABLE2_PPI
in MdePkg.
This module produces EFI_PEI_READ_ONLY_VARIABLE2_PPI on top of EFI_PEI_READ_ONLY_VARIABLE_PPI. 
This module is used on platform when both of these two conditions are true:
1) Framework module produces EFI_PEI_READ_ONLY_VARIABLE_PPI is present.
2) The platform has PI modules that only consumes EFI_PEI_READ_ONLY_VARIABLE2_PPI.

This module can't be used together with ReadOnlyVariableToReadOnlyVariable2Thunk module.


Copyright (c) 2006 - 2008 Intel Corporation. <BR>
All rights reserved. This program and the accompanying materials
are licensed and made available under the terms and conditions of the BSD License
which accompanies this distribution.  The full text of the license may be found at
http://opensource.org/licenses/bsd-license.php

THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.
Module Name:

**/

#include <PiPei.h>
#include <Ppi/ReadOnlyVariable2.h>
#include <Ppi/ReadOnlyVariable.h>
#include <Ppi/ReadOnlyVariableThunkPresent.h>
#include <Library/DebugLib.h>
#include <Library/PeiServicesTablePointerLib.h>
#include <Library/PeiServicesLib.h>

//
// Function Prototypes
//
EFI_STATUS
EFIAPI
PeiGetVariable (
  IN CONST  EFI_PEI_READ_ONLY_VARIABLE2_PPI *This,
  IN CONST  CHAR16                          *VariableName,
  IN CONST  EFI_GUID                        *VariableGuid,
  OUT       UINT32                          *Attributes,
  IN OUT    UINTN                           *DataSize,
  OUT       VOID                            *Data
  );

EFI_STATUS
EFIAPI
PeiGetNextVariableName (
  IN CONST  EFI_PEI_READ_ONLY_VARIABLE2_PPI *This,
  IN OUT UINTN                              *VariableNameSize,
  IN OUT CHAR16                             *VariableName,
  IN OUT EFI_GUID                           *VariableGuid
  );

//
// Module globals
//
EFI_PEI_READ_ONLY_VARIABLE2_PPI mVariablePpi = {
  PeiGetVariable,
  PeiGetNextVariableName
};

EFI_PEI_PPI_DESCRIPTOR     mPpiListVariable = {
  (EFI_PEI_PPI_DESCRIPTOR_PPI | EFI_PEI_PPI_DESCRIPTOR_TERMINATE_LIST),
  &gEfiPeiReadOnlyVariable2PpiGuid,
  &mVariablePpi
};


EFI_PEI_PPI_DESCRIPTOR     mReadOnlyVariableThunkPresent = {
    (EFI_PEI_PPI_DESCRIPTOR_PPI | EFI_PEI_PPI_DESCRIPTOR_TERMINATE_LIST),
    &gPeiReadonlyVariableThunkPresentPpiGuid,
    NULL
  };

EFI_STATUS
EFIAPI
PeimInitializeReadOnlyVariable2 (
  IN EFI_FFS_FILE_HEADER       *FfsHeader,
  IN EFI_PEI_SERVICES          **PeiServices
  )
/*++

Routine Description:

  Provide the functionality of the variable services.

Arguments:

  FfsHeadher  - The FFS file header
  PeiServices - General purpose services available to every PEIM.

Returns:

  Status -  EFI_SUCCESS if the interface could be successfully
            installed

--*/
{
  VOID        *Interface;
  EFI_STATUS  Status;

  //
  // Make sure ReadOnlyVariable2ToReadOnlyVariable module is not present. If so, the call chain will form a
  // infinite loop: ReadOnlyVariable2 -> ReadOnlyVariable -> ReadOnlyVariable2 -> ....
  //
  Status = PeiServicesLocatePpi (&gPeiReadonlyVariableThunkPresentPpiGuid, 0, NULL, &Interface);
  ASSERT (Status == EFI_NOT_FOUND);
  
  PeiServicesInstallPpi (&mReadOnlyVariableThunkPresent);
  //
  // Publish the variable capability to other modules
  //
  return PeiServicesInstallPpi (&mPpiListVariable);
}

EFI_STATUS
EFIAPI
PeiGetVariable (
  IN CONST  EFI_PEI_READ_ONLY_VARIABLE2_PPI *This,
  IN CONST  CHAR16                          *VariableName,
  IN CONST  EFI_GUID                        *VariableGuid,
  OUT       UINT32                          *Attributes,
  IN OUT    UINTN                           *DataSize,
  OUT       VOID                            *Data
  )
/*++

Routine Description:

  Provide the read variable functionality of the variable services.

Arguments:

  PeiServices - General purpose services available to every PEIM.

  VariableName     - The variable name

  VendorGuid       - The vendor's GUID

  Attributes       - Pointer to the attribute

  DataSize         - Size of data

  Data             - Pointer to data

Returns:

  EFI_SUCCESS           - The interface could be successfully installed

  EFI_NOT_FOUND         - The variable could not be discovered

  EFI_BUFFER_TOO_SMALL  - The caller buffer is not large enough

--*/
{
  EFI_STATUS                     Status;
  EFI_PEI_READ_ONLY_VARIABLE_PPI *ReadOnlyVariable;

  Status = PeiServicesLocatePpi (
             &gEfiPeiReadOnlyVariablePpiGuid,
             0,
             NULL,
             (VOID **)&ReadOnlyVariable
             );
  ASSERT_EFI_ERROR (Status);

  return ReadOnlyVariable->PeiGetVariable (
                             GetPeiServicesTablePointer (),
                             (CHAR16 *)VariableName,
                             (EFI_GUID *)VariableGuid,
                             Attributes,
                             DataSize,
                             Data
                             );
}

EFI_STATUS
EFIAPI
PeiGetNextVariableName (
  IN CONST  EFI_PEI_READ_ONLY_VARIABLE2_PPI *This,
  IN OUT UINTN                              *VariableNameSize,
  IN OUT CHAR16                             *VariableName,
  IN OUT EFI_GUID                           *VariableGuid
  )
/*++

Routine Description:

  Provide the get next variable functionality of the variable services.

Arguments:

  PeiServices        - General purpose services available to every PEIM.
  VariabvleNameSize  - The variable name's size.
  VariableName       - A pointer to the variable's name.
  VariableGuid       - A pointer to the EFI_GUID structure.

  VariableNameSize - Size of the variable name

  VariableName     - The variable name

  VendorGuid       - The vendor's GUID

Returns:

  EFI_SUCCESS - The interface could be successfully installed

  EFI_NOT_FOUND - The variable could not be discovered

--*/
{
  EFI_STATUS                     Status;
  EFI_PEI_READ_ONLY_VARIABLE_PPI *ReadOnlyVariable;

  Status = PeiServicesLocatePpi (
             &gEfiPeiReadOnlyVariablePpiGuid,
             0,
             NULL,
             (VOID **)&ReadOnlyVariable
             );
  ASSERT_EFI_ERROR (Status);

  return ReadOnlyVariable->PeiGetNextVariableName (
                             GetPeiServicesTablePointer (),
                             VariableNameSize,
                             VariableName,
                             VariableGuid
                             );
}
