/**@file

Copyright (c) 2006, Intel Corporation. All rights reserved.<BR>
SPDX-License-Identifier: BSD-2-Clause-Patent

Module Name:
  WinNtFwh.c
    
Abstract:
  PEIM to abstract construction of firmware volume in a Windows NT environment.

Revision History

**/

//
// The package level header files this module uses
//
#include <PiPei.h>
#include <WinNtPeim.h>
//
// The protocols, PPI and GUID defintions for this module
//
#include <Ppi/NtFwh.h>
//
// The Library classes this module consumes
//
#include <Library/DebugLib.h>
#include <Library/PeimEntryPoint.h>
#include <Library/HobLib.h>
#include <Library/PcdLib.h>

EFI_STATUS
EFIAPI
PeimInitializeWinNtFwh (
  IN EFI_FFS_FILE_HEADER       *FfsHeader,
  IN EFI_PEI_SERVICES          **PeiServices
  )
/*++

Routine Description:
  Perform a call-back into the SEC simulator to get address of the Firmware Hub

Arguments:
  FfsHeader   - Ffs Header available to every PEIM
  PeiServices - General purpose services available to every PEIM.
    
Returns:
  None

--*/
{
  EFI_STATUS                  Status;
  EFI_PEI_PPI_DESCRIPTOR      *PpiDescriptor;
  NT_FWH_PPI                  *FwhPpi;
  EFI_PHYSICAL_ADDRESS        FdBase;
  EFI_FIRMWARE_VOLUME_HEADER  *FvHeader;
  UINT64                      FdSize;
  UINTN                       Index;

  DEBUG ((EFI_D_ERROR, "NT 32 Firmware Volume PEIM Loaded\n"));

  //
  // Get the Fwh Information PPI
  //
  Status = (**PeiServices).LocatePpi (
                            (const EFI_PEI_SERVICES **)PeiServices,
                            &gNtFwhPpiGuid, // GUID
                            0,              // INSTANCE
                            &PpiDescriptor, // EFI_PEI_PPI_DESCRIPTOR
                            (VOID**)&FwhPpi         // PPI
                            );
  ASSERT_EFI_ERROR (Status);

  Index = 0;
  do {
    //
    // Get information about all the FD's in the system
    //
    Status = FwhPpi->NtFwh (Index, &FdBase, &FdSize);
    if (!EFI_ERROR (Status)) {
      //
      // Assume the FD starts with an FV header
      //
      FvHeader = (EFI_FIRMWARE_VOLUME_HEADER *) (UINTN) FdBase;

      //
      // Make an FV Hob for the first FV in the FD
      //
      BuildFvHob (FdBase, FvHeader->FvLength);

      if (Index == 0) {
        //
        // Assume the first FD was produced by the NT32.DSC
        //  All these strange offests are needed to keep in
        //  sync with the FlashMap and NT32.dsc file
        //
        BuildResourceDescriptorHob (
          EFI_RESOURCE_FIRMWARE_DEVICE,
          (EFI_RESOURCE_ATTRIBUTE_PRESENT | EFI_RESOURCE_ATTRIBUTE_INITIALIZED | EFI_RESOURCE_ATTRIBUTE_UNCACHEABLE),
          FdBase,
          ( 
            FvHeader->FvLength + 
            PcdGet32 (PcdFlashNvStorageVariableSize) +
            PcdGet32 (PcdFlashNvStorageFtwWorkingSize) +
            PcdGet32 (PcdFlashNvStorageFtwSpareSize) +
            PcdGet32 (PcdWinNtFlashNvStorageEventLogSize)
          )
          );
        //
        // Hard code the address of the spare block and variable services.
        //  Assume it's a hard coded offset from FV0 in FD0.
        //
        FdSize  = 
          PcdGet32 (PcdFlashNvStorageVariableSize) +
          PcdGet32 (PcdFlashNvStorageFtwWorkingSize) +
          PcdGet32 (PcdFlashNvStorageFtwSpareSize) +
          PcdGet32 (PcdWinNtFlashNvStorageEventLogSize);

        BuildFvHob (FdBase + PcdGet32 (PcdWinNtFlashNvStorageVariableBase), FdSize);
      } else {
        //
        // For other FD's just map them in.
        //
        BuildResourceDescriptorHob (
          EFI_RESOURCE_FIRMWARE_DEVICE,
          (EFI_RESOURCE_ATTRIBUTE_PRESENT | EFI_RESOURCE_ATTRIBUTE_INITIALIZED | EFI_RESOURCE_ATTRIBUTE_UNCACHEABLE),
          FdBase,
          FdSize
          );
      }
    }

    Index++;
  } while (!EFI_ERROR (Status));

  return Status;
}
