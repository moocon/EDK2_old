/*++

Copyright (c) 2006, Intel Corporation
All rights reserved. This program and the accompanying materials
are licensed and made available under the terms and conditions of the BSD License
which accompanies this distribution.  The full text of the license may be found at
http://opensource.org/licenses/bsd-license.php

THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

Module Name:

  Dispatcher.c

Abstract:

  EFI PEI Core dispatch services

Revision History

--*/

#include <PeiMain.h>

#define INIT_CAR_VALUE 0x5AA55AA5

typedef struct {
  EFI_STATUS_CODE_DATA  DataHeader;
  EFI_HANDLE            Handle;
} PEIM_FILE_HANDLE_EXTENDED_DATA;

VOID
DiscoverPeimsAndOrderWithApriori (
  IN  PEI_CORE_INSTANCE    *Private,
  IN  EFI_PEI_FV_HANDLE    VolumeHandle
  )
/*++

Routine Description:

  Discover all Peims and optional Apriori file in one FV. There is at most one
  Apriori file in one FV.

Arguments:

  Private          - Pointer to the private data passed in from caller
  VolumeHandle     - Fv handle.
Returns:

  NONE

--*/
{
  EFI_STATUS                          Status;
  EFI_PEI_FV_HANDLE                   FileHandle;
  EFI_PEI_FILE_HANDLE                 AprioriFileHandle;
  EFI_GUID                            *Apriori;
  UINTN                               Index;
  UINTN                               Index2;
  UINTN                               PeimIndex;
  UINTN                               PeimCount;
  EFI_GUID                            *Guid;
  EFI_PEI_FV_HANDLE                   TempFileHandles[FixedPcdGet32 (PcdPeiCoreMaxPeimPerFv)];
  EFI_GUID                            FileGuid[FixedPcdGet32 (PcdPeiCoreMaxPeimPerFv)];

  //
  // Walk the FV and find all the PEIMs and the Apriori file.
  //
  AprioriFileHandle = NULL;
  Private->CurrentFvFileHandles[0] = NULL;
  Guid = NULL;
  FileHandle = NULL;

  //
  // If the current Fv has been scanned, directly get its cachable record.
  //
  if (Private->Fv[Private->CurrentPeimFvCount].ScanFv) {
    CopyMem (Private->CurrentFvFileHandles, Private->Fv[Private->CurrentPeimFvCount].FvFileHandles, sizeof (Private->CurrentFvFileHandles));
    return;
  }

  //
  // Go ahead to scan this Fv, and cache FileHandles within it.
  //
  for (PeimCount = 0; PeimCount < FixedPcdGet32 (PcdPeiCoreMaxPeimPerFv); PeimCount++) {
    Status = PeiFindFileEx (
                VolumeHandle,
                NULL,
                PEI_CORE_INTERNAL_FFS_FILE_DISPATCH_TYPE,
                &FileHandle,
                &AprioriFileHandle
                );
    if (Status != EFI_SUCCESS) {
      break;
    }

    Private->CurrentFvFileHandles[PeimCount] = FileHandle;
  }

  Private->AprioriCount = 0;
  if (AprioriFileHandle != NULL) {
    //
    // Read the Apriori file
    //
    Status = PeiServicesFfsFindSectionData (EFI_SECTION_RAW, AprioriFileHandle, (VOID **) &Apriori);
    if (!EFI_ERROR (Status)) {
      //
      // Calculate the number of PEIMs in the A Priori list
      //
      Private->AprioriCount = *(UINT32 *)(((EFI_FFS_FILE_HEADER *)AprioriFileHandle)->Size) & 0x00FFFFFF;
      Private->AprioriCount -= sizeof (EFI_FFS_FILE_HEADER) - sizeof (EFI_COMMON_SECTION_HEADER);
      Private->AprioriCount /= sizeof (EFI_GUID);

      SetMem (FileGuid, sizeof (FileGuid), 0);
      for (Index = 0; Index < PeimCount; Index++) {
        //
        // Make an array of file name guids that matches the FileHandle array so we can convert
        // quickly from file name to file handle
        //
        CopyMem (&FileGuid[Index], &((EFI_FFS_FILE_HEADER *)Private->CurrentFvFileHandles[Index])->Name,sizeof(EFI_GUID));
      }

      //
      // Walk through FileGuid array to find out who is invalid PEIM guid in Apriori file.
      // Add avalible PEIMs in Apriori file into TempFileHandles array at first.
      //
      Index2 = 0;
      for (Index = 0; Index2 < Private->AprioriCount; Index++) {
        while (Index2 < Private->AprioriCount) {
          Guid = ScanGuid (FileGuid, PeimCount * sizeof (EFI_GUID), &Apriori[Index2++]);
          if (Guid != NULL) {
            break;
          }
        }
        if (Guid == NULL) {
          break;
        }
        PeimIndex = ((UINTN)Guid - (UINTN)&FileGuid[0])/sizeof (EFI_GUID);
        TempFileHandles[Index] = Private->CurrentFvFileHandles[PeimIndex];

        //
        // Since we have copied the file handle we can remove it from this list.
        //
        Private->CurrentFvFileHandles[PeimIndex] = NULL;
      }

      //
      // Update valid Aprioricount
      //
      Private->AprioriCount = Index;

      //
      // Add in any PEIMs not in the Apriori file
      //
      for (;Index < PeimCount; Index++) {
        for (Index2 = 0; Index2 < PeimCount; Index2++) {
          if (Private->CurrentFvFileHandles[Index2] != NULL) {
            TempFileHandles[Index] = Private->CurrentFvFileHandles[Index2];
            Private->CurrentFvFileHandles[Index2] = NULL;
            break;
          }
        }
      }
      //
      //Index the end of array contains re-range Pei moudle.
      //
      TempFileHandles[Index] = NULL;

      //
      // Private->CurrentFvFileHandles is currently in PEIM in the FV order.
      // We need to update it to start with files in the A Priori list and
      // then the remaining files in PEIM order.
      //
      CopyMem (Private->CurrentFvFileHandles, TempFileHandles, sizeof (Private->CurrentFvFileHandles));
    }
  }
  //
  // Cache the current Fv File Handle. So that we don't have to scan the Fv again.
  // Instead, we can retrieve the file handles within this Fv from cachable data.
  //
  Private->Fv[Private->CurrentPeimFvCount].ScanFv = TRUE;
  CopyMem (Private->Fv[Private->CurrentPeimFvCount].FvFileHandles, Private->CurrentFvFileHandles, sizeof (Private->CurrentFvFileHandles));

}

VOID*
ShadowPeiCore(
  EFI_PEI_SERVICES     **PeiServices,
  PEI_CORE_INSTANCE    *PrivateInMem
  )
{
  EFI_PEI_FILE_HANDLE  PeiCoreFileHandle;
  EFI_PHYSICAL_ADDRESS EntryPoint;
  EFI_STATUS           Status;
  UINT32               AuthenticationState;

  PeiCoreFileHandle = NULL;

  //
  // Find the PEI Core in the BFV
  //
  Status = PeiFindFileEx (
             (EFI_PEI_FV_HANDLE)PrivateInMem->Fv[0].FvHeader,
             NULL,
             EFI_FV_FILETYPE_PEI_CORE,
             &PeiCoreFileHandle,
             NULL
             );
  ASSERT_EFI_ERROR (Status);

  //
  // Shadow PEI Core into memory so it will run faster
  //
  Status = PeiLoadImage (
              PeiServices,
              *((EFI_PEI_FILE_HANDLE*)&PeiCoreFileHandle),
              &EntryPoint,
              &AuthenticationState
              );
  ASSERT_EFI_ERROR (Status);

  return (VOID*) ((UINTN) EntryPoint + (UINTN) PeiCore - (UINTN) _ModuleEntryPoint);
}

VOID
PeiDispatcher (
  IN CONST EFI_SEC_PEI_HAND_OFF  *SecCoreData,
  IN PEI_CORE_INSTANCE           *Private
  )

/*++

Routine Description:

  Conduct PEIM dispatch.

Arguments:

  SecCoreData          - Points to a data structure containing information about the PEI core's operating
                         environment, such as the size and location of temporary RAM, the stack location and
                         the BFV location.
  PrivateData          - Pointer to the private data passed in from caller
  DispatchData         - Pointer to PEI_CORE_DISPATCH_DATA data.

Returns:

  EFI_SUCCESS   - Successfully dispatched PEIM.
  EFI_NOT_FOUND - The dispatch failed.

--*/
{
  EFI_STATUS                          Status;
  UINT32                              Index1;
  UINT32                              Index2;
  EFI_PEI_SERVICES                    **PeiServices;
  EFI_PEI_FV_HANDLE                   VolumeHandle;
  EFI_PEI_FILE_HANDLE                 PeimFileHandle;
  UINTN                               FvCount;
  UINTN                               PeimCount;
  UINT32                              AuthenticationState;
  EFI_PHYSICAL_ADDRESS                EntryPoint;
  EFI_PEIM_ENTRY_POINT2               PeimEntryPoint;
  BOOLEAN                             PeimNeedingDispatch;
  BOOLEAN                             PeimDispatchOnThisPass;
  UINTN                               SaveCurrentPeimCount;
  UINTN                               SaveCurrentFvCount;
  EFI_PEI_FILE_HANDLE                 SaveCurrentFileHandle;
  PEIM_FILE_HANDLE_EXTENDED_DATA      ExtendedData;
  EFI_PHYSICAL_ADDRESS                NewPermenentMemoryBase;
  TEMPORARY_RAM_SUPPORT_PPI           *TemporaryRamSupportPpi;
  EFI_HOB_HANDOFF_INFO_TABLE          *OldHandOffTable;
  EFI_HOB_HANDOFF_INFO_TABLE          *NewHandOffTable;
  INTN                                Offset;
  PEI_CORE_INSTANCE                   *PrivateInMem;
  UINT64                              NewPeiStackSize;
  UINT64                              OldPeiStackSize;
  UINT64                              StackGap;
  EFI_FV_FILE_INFO                    FvFileInfo;
  UINTN                               OldCheckingTop;
  UINTN                               OldCheckingBottom;


  PeiServices = &Private->PS;
  PeimEntryPoint = NULL;
  PeimFileHandle = NULL;
  EntryPoint     = 0;

  if ((Private->PeiMemoryInstalled) && (Private->HobList.HandoffInformationTable->BootMode != BOOT_ON_S3_RESUME)) {
    //
    // Once real memory is available, shadow the RegisterForShadow modules. And meanwhile
    // update the modules' status from PEIM_STATE_REGISITER_FOR_SHADOW to PEIM_STATE_DONE.
    //
    SaveCurrentPeimCount  = Private->CurrentPeimCount;
    SaveCurrentFvCount    = Private->CurrentPeimFvCount;
    SaveCurrentFileHandle =  Private->CurrentFileHandle;

    for (Index1 = 0; Index1 <= SaveCurrentFvCount; Index1++) {
      for (Index2 = 0; (Index2 < FixedPcdGet32 (PcdPeiCoreMaxPeimPerFv)) && (Private->Fv[Index1].FvFileHandles[Index2] != NULL); Index2++) {
        if (Private->Fv[Index1].PeimState[Index2] == PEIM_STATE_REGISITER_FOR_SHADOW) {
          PeimFileHandle = Private->Fv[Index1].FvFileHandles[Index2];
          Status = PeiLoadImage (
                    &Private->PS,
                    PeimFileHandle,
                    &EntryPoint,
                    &AuthenticationState
                    );
          if (Status == EFI_SUCCESS) {
            //
            // PEIM_STATE_REGISITER_FOR_SHADOW move to PEIM_STATE_DONE
            //
            Private->Fv[Index1].PeimState[Index2]++;
            Private->CurrentFileHandle   = PeimFileHandle;
            Private->CurrentPeimFvCount  = Index1;
            Private->CurrentPeimCount    = Index2;
            //
            // Call the PEIM entry point
            //
            PeimEntryPoint = (EFI_PEIM_ENTRY_POINT2)(UINTN)EntryPoint;

            PERF_START (0, "PEIM", NULL, 0);
            PeimEntryPoint(PeimFileHandle, (const EFI_PEI_SERVICES **) &Private->PS);
            PERF_END (0, "PEIM", NULL, 0);
          }

          //
          // Process the Notify list and dispatch any notifies for
          // newly installed PPIs.
          //
          ProcessNotifyList (Private);
        }
      }
    }
    Private->CurrentFileHandle  = SaveCurrentFileHandle;
    Private->CurrentPeimFvCount = SaveCurrentFvCount;
    Private->CurrentPeimCount   = SaveCurrentPeimCount;
  }

  //
  // This is the main dispatch loop.  It will search known FVs for PEIMs and
  // attempt to dispatch them.  If any PEIM gets dispatched through a single
  // pass of the dispatcher, it will start over from the Bfv again to see
  // if any new PEIMs dependencies got satisfied.  With a well ordered
  // FV where PEIMs are found in the order their dependencies are also
  // satisfied, this dipatcher should run only once.
  //
  do {
    PeimNeedingDispatch = FALSE;
    PeimDispatchOnThisPass = FALSE;

    for (FvCount = Private->CurrentPeimFvCount; FvCount < Private->FvCount; FvCount++) {
      Private->CurrentPeimFvCount = FvCount;
      VolumeHandle = Private->Fv[FvCount].FvHeader;

      if (Private->CurrentPeimCount == 0) {
        //
        // When going through each FV, at first, search Apriori file to
        // reorder all PEIMs to ensure the PEIMs in Apriori file to get
        // dispatch at first.
        //
        DiscoverPeimsAndOrderWithApriori (Private, VolumeHandle);
      }

      //
      // Start to dispatch all modules within the current Fv.
      //
      for (PeimCount = Private->CurrentPeimCount;
           (PeimCount < FixedPcdGet32 (PcdPeiCoreMaxPeimPerFv)) && (Private->CurrentFvFileHandles[PeimCount] != NULL);
           PeimCount++) {
        Private->CurrentPeimCount  = PeimCount;
        PeimFileHandle = Private->CurrentFileHandle = Private->CurrentFvFileHandles[PeimCount];

        if (Private->Fv[FvCount].PeimState[PeimCount] == PEIM_STATE_NOT_DISPATCHED) {
          if (!DepexSatisfied (Private, PeimFileHandle, PeimCount)) {
            PeimNeedingDispatch = TRUE;
          } else {
            Status = PeiFfsGetFileInfo (PeimFileHandle, &FvFileInfo);
            ASSERT_EFI_ERROR (Status);
            if (FvFileInfo.FileType == EFI_FV_FILETYPE_FIRMWARE_VOLUME_IMAGE) {
              //
              // For Fv type file, Produce new FV PPI and FV hob
              //
              Status = ProcessFvFile (PeiServices, PeimFileHandle, &AuthenticationState);
            } else {
              //
              // For PEIM driver, Load its entry point
              //
              Status = PeiLoadImage (
                         PeiServices,
                         PeimFileHandle,
                         &EntryPoint,
                         &AuthenticationState
                         );
            }

            if ((Status == EFI_SUCCESS)) {
              //
              // The PEIM has its dependencies satisfied, and its entry point
              // has been found, so invoke it.
              //
              PERF_START (0, "PEIM", NULL, 0);

              ExtendedData.Handle = (EFI_HANDLE)PeimFileHandle;

              REPORT_STATUS_CODE_WITH_EXTENDED_DATA (
                EFI_PROGRESS_CODE,
                FixedPcdGet32(PcdStatusCodeValuePeimDispatch),
                (VOID *)(&ExtendedData),
                sizeof (ExtendedData)
                );

              Status = VerifyPeim (Private, VolumeHandle, PeimFileHandle);
              if (Status != EFI_SECURITY_VIOLATION && (AuthenticationState == 0)) {
                //
                // PEIM_STATE_NOT_DISPATCHED move to PEIM_STATE_DISPATCHED
                //
                Private->Fv[FvCount].PeimState[PeimCount]++;

                if (FvFileInfo.FileType != EFI_FV_FILETYPE_FIRMWARE_VOLUME_IMAGE) {
                  //
                  // Call the PEIM entry point for PEIM driver
                  //
                  PeimEntryPoint = (EFI_PEIM_ENTRY_POINT2)(UINTN)EntryPoint;
                  PeimEntryPoint (PeimFileHandle, (const EFI_PEI_SERVICES **) PeiServices);
                }

                PeimDispatchOnThisPass = TRUE;
              }

              REPORT_STATUS_CODE_WITH_EXTENDED_DATA (
                EFI_PROGRESS_CODE,
                FixedPcdGet32(PcdStatusCodeValuePeimDispatch),
                (VOID *)(&ExtendedData),
                sizeof (ExtendedData)
                );
              PERF_END (0, "PEIM", NULL, 0);

            }

            if (Private->SwitchStackSignal) {
              //
              // Before switch stack from CAR to permenent memory, caculate the heap and stack
              // usage in temporary memory for debuging.
              //
              DEBUG_CODE_BEGIN ();
                UINTN  *StackPointer;
                
                for (StackPointer = (UINTN*)SecCoreData->StackBase;
                     (StackPointer < (UINTN*)((UINTN)SecCoreData->StackBase + SecCoreData->StackSize)) \
                     && (*StackPointer == INIT_CAR_VALUE);
                     StackPointer ++);
                     
                DEBUG ((EFI_D_INFO, "Total Cache as RAM:    %d bytes.\n", SecCoreData->TemporaryRamSize));
                DEBUG ((EFI_D_INFO, "  CAR stack ever used: %d bytes.\n",
                       (SecCoreData->StackSize - ((UINTN) StackPointer - (UINTN)SecCoreData->StackBase))
                      ));
                DEBUG ((EFI_D_INFO, "  CAR heap used:       %d bytes.\n",
                       ((UINTN) Private->HobList.HandoffInformationTable->EfiFreeMemoryBottom -
                       (UINTN) Private->HobList.Raw)
                      ));
              DEBUG_CODE_END ();
              
              //
              // Reserve the size of new stack at bottom of physical memory
              //
              OldPeiStackSize = Private->StackSize;
              NewPeiStackSize = (RShiftU64 (Private->PhysicalMemoryLength, 1) + EFI_PAGE_MASK) & ~EFI_PAGE_MASK;
              if (FixedPcdGet32(PcdPeiCoreMaxPeiStackSize) > (UINT32) NewPeiStackSize) {
                Private->StackSize = NewPeiStackSize;
              } else {
                Private->StackSize = FixedPcdGet32(PcdPeiCoreMaxPeiStackSize);
              }

              //
              // In theory, the size of new stack in permenent memory should large than
              // size of old stack in temporary memory.
              // But if new stack is smaller than the size of old stack, we also reserve
              // the size of old stack at bottom of permenent memory.
              //
              StackGap = 0;
              if (Private->StackSize > OldPeiStackSize) {
                StackGap = Private->StackSize - OldPeiStackSize;
              }

              //
              // Update HandOffHob for new installed permenent memory
              //
              OldHandOffTable   = Private->HobList.HandoffInformationTable;
              OldCheckingBottom = (UINTN)OldHandOffTable;
              OldCheckingTop    = (UINTN)(OldCheckingBottom + SecCoreData->TemporaryRamSize);

              //
              // The whole temporary memory will be migrated to physical memory.
              // CAUTION: The new base is computed accounding to gap of new stack.
              //
              NewPermenentMemoryBase = Private->PhysicalMemoryBegin + StackGap;
              Offset                 = (UINTN) NewPermenentMemoryBase - (UINTN) SecCoreData->TemporaryRamBase;
              NewHandOffTable        = (EFI_HOB_HANDOFF_INFO_TABLE *)((UINTN)OldHandOffTable + Offset);
              PrivateInMem           = (PEI_CORE_INSTANCE *)((UINTN) (VOID*) Private + Offset);

              //
              // TemporaryRamSupportPpi is produced by platform's SEC
              //
              Status = PeiLocatePpi (
                         (CONST EFI_PEI_SERVICES **) PeiServices,
                         &gEfiTemporaryRamSupportPpiGuid,
                         0,
                         NULL,
                         (VOID**)&TemporaryRamSupportPpi
                         );


              if (!EFI_ERROR (Status)) {
                TemporaryRamSupportPpi->TemporaryRamMigration (
                                          (CONST EFI_PEI_SERVICES **) PeiServices,
                                          (EFI_PHYSICAL_ADDRESS)(UINTN) SecCoreData->TemporaryRamBase,
                                          (EFI_PHYSICAL_ADDRESS)(UINTN) NewPermenentMemoryBase,
                                          SecCoreData->TemporaryRamSize
                                          );

              } else {
                CopyMem (
                  (VOID*)(UINTN) NewPermenentMemoryBase,
                  SecCoreData->TemporaryRamBase,
                  SecCoreData->TemporaryRamSize
                  );
              }


              //
              //
              // Fixup the PeiCore's private data
              //
              PrivateInMem->PS          = &PrivateInMem->ServiceTableShadow;
              PrivateInMem->CpuIo       = &PrivateInMem->ServiceTableShadow.CpuIo;
              PrivateInMem->HobList.Raw = (VOID*) ((UINTN) PrivateInMem->HobList.Raw + Offset);
              PrivateInMem->StackBase   = (EFI_PHYSICAL_ADDRESS)(((UINTN)PrivateInMem->PhysicalMemoryBegin + EFI_PAGE_MASK) & ~EFI_PAGE_MASK);

              PeiServices = &PrivateInMem->PS;

              //
              // Fixup for PeiService's address
              //
              SetPeiServicesTablePointer(PeiServices);

              //
              // Update HandOffHob for new installed permenent memory
              //
              NewHandOffTable->EfiEndOfHobList =
                (EFI_PHYSICAL_ADDRESS)(VOID*)((UINTN) NewHandOffTable->EfiEndOfHobList + Offset);
              NewHandOffTable->EfiMemoryTop        = PrivateInMem->PhysicalMemoryBegin +
                                                     PrivateInMem->PhysicalMemoryLength;
              NewHandOffTable->EfiMemoryBottom     = PrivateInMem->PhysicalMemoryBegin;
              NewHandOffTable->EfiFreeMemoryTop    = PrivateInMem->FreePhysicalMemoryTop;
              NewHandOffTable->EfiFreeMemoryBottom = NewHandOffTable->EfiEndOfHobList +
                                                     sizeof (EFI_HOB_GENERIC_HEADER);

              //
              // We need convert the PPI desciptor's pointer
              //
              ConvertPpiPointers ((CONST EFI_PEI_SERVICES **)PeiServices, 
                                  OldCheckingBottom, 
                                  OldCheckingTop, 
                                  NewHandOffTable);

              DEBUG ((EFI_D_INFO, "Stack Hob: BaseAddress=0x%X Length=0x%X\n",
                                  (UINTN)PrivateInMem->StackBase,
                                  PrivateInMem->StackSize));
              BuildStackHob (PrivateInMem->StackBase, PrivateInMem->StackSize);

              //
              // After the whole temporary memory is migrated, then we can allocate page in
              // permenent memory.
              //
              PrivateInMem->PeiMemoryInstalled     = TRUE;

              //
              // Make sure we don't retry the same PEIM that added memory
              //
              PrivateInMem->CurrentPeimCount++;

              //
              // Shadow PEI Core. When permanent memory is avaiable, shadow
              // PEI Core and PEIMs to get high performance.
              //
              PrivateInMem->ShadowedPeiCore = ShadowPeiCore (
                                                PeiServices,
                                                PrivateInMem
                                                );
              //
              // Process the Notify list and dispatch any notifies for
              // newly installed PPIs.
              //
              ProcessNotifyList (PrivateInMem);

              //
              // Entry PEI Phase 2
              //
              PeiCore (SecCoreData, NULL, PrivateInMem);

              //
              // Code should not come here
              //
              ASSERT_EFI_ERROR(FALSE);
            }

            //
            // Process the Notify list and dispatch any notifies for
            // newly installed PPIs.
            //
            ProcessNotifyList (Private);

            if ((Private->PeiMemoryInstalled) && (Private->Fv[FvCount].PeimState[PeimCount] == PEIM_STATE_REGISITER_FOR_SHADOW) &&   \
                (Private->HobList.HandoffInformationTable->BootMode != BOOT_ON_S3_RESUME)) {
              //
              // If memory is availble we shadow images by default for performance reasons.
              // We call the entry point a 2nd time so the module knows it's shadowed.
              //
              //PERF_START (PeiServices, L"PEIM", PeimFileHandle, 0);
              PeimEntryPoint (PeimFileHandle, (const EFI_PEI_SERVICES **) PeiServices);
              //PERF_END (PeiServices, L"PEIM", PeimFileHandle, 0);

              //
              // PEIM_STATE_REGISITER_FOR_SHADOW move to PEIM_STATE_DONE
              //
              Private->Fv[FvCount].PeimState[PeimCount]++;

              //
              // Process the Notify list and dispatch any notifies for
              // newly installed PPIs.
              //
              ProcessNotifyList (Private);
            }
          }
        }
      }

      //
      // We set to NULL here to optimize the 2nd entry to this routine after
      //  memory is found. This reprevents rescanning of the FV. We set to
      //  NULL here so we start at the begining of the next FV
      //
      Private->CurrentFileHandle = NULL;
      Private->CurrentPeimCount = 0;
      //
      // Before walking through the next FV,Private->CurrentFvFileHandles[]should set to NULL
      //
      SetMem (Private->CurrentFvFileHandles, sizeof (Private->CurrentFvFileHandles), 0);
    }

    //
    // Before making another pass, we should set Private->CurrentPeimFvCount =0 to go
    // through all the FV.
    //
    Private->CurrentPeimFvCount = 0;

    //
    // PeimNeedingDispatch being TRUE means we found a PEIM that did not get
    //  dispatched. So we need to make another pass
    //
    // PeimDispatchOnThisPass being TRUE means we dispatched a PEIM on this
    //  pass. If we did not dispatch a PEIM there is no point in trying again
    //  as it will fail the next time too (nothing has changed).
    //
  } while (PeimNeedingDispatch && PeimDispatchOnThisPass);

}

VOID
InitializeDispatcherData (
  IN PEI_CORE_INSTANCE            *PrivateData,
  IN PEI_CORE_INSTANCE            *OldCoreData,
  IN CONST EFI_SEC_PEI_HAND_OFF   *SecCoreData
  )
/*++

Routine Description:

  Initialize the Dispatcher's data members

Arguments:

  PeiServices          - The PEI core services table.
  OldCoreData          - Pointer to old core data (before switching stack).
                         NULL if being run in non-permament memory mode.
  SecCoreData          - Points to a data structure containing information about the PEI core's operating
                         environment, such as the size and location of temporary RAM, the stack location and
                         the BFV location.

Returns:

  None.

--*/
{
  if (OldCoreData == NULL) {
    PeiInitializeFv (PrivateData, SecCoreData);
  }

  return;
}


BOOLEAN
DepexSatisfied (
  IN PEI_CORE_INSTANCE          *Private,
  IN EFI_PEI_FILE_HANDLE        FileHandle,
  IN UINTN                      PeimCount
  )
/*++

Routine Description:

  This routine parses the Dependency Expression, if available, and
  decides if the module can be executed.

Arguments:
  PeiServices - The PEI Service Table
  CurrentPeimAddress - Address of the PEIM Firmware File under investigation

Returns:
  TRUE  - Can be dispatched
  FALSE - Cannot be dispatched

--*/
{
  EFI_STATUS           Status;
  VOID                 *DepexData;

  if (PeimCount < Private->AprioriCount) {
    //
    // If its in the A priori file then we set Depex to TRUE
    //
    return TRUE;
  }

  //
  // Depex section not in the encapsulated section.
  //
  Status = PeiServicesFfsFindSectionData (
              EFI_SECTION_PEI_DEPEX,
              FileHandle,
              (VOID **)&DepexData
              );

  if (EFI_ERROR (Status)) {
    //
    // If there is no DEPEX, assume the module can be executed
    //
    return TRUE;
  }

  //
  // Evaluate a given DEPEX
  //
  return PeimDispatchReadiness (&Private->PS, DepexData);
}

/**
  This routine enable a PEIM to register itself to shadow when PEI Foundation
  discovery permanent memory.

	@param FileHandle  	File handle of a PEIM.

  @retval EFI_NOT_FOUND  				The file handle doesn't point to PEIM itself.
  @retval EFI_ALREADY_STARTED		Indicate that the PEIM has been registered itself.
  @retval EFI_SUCCESS						Successfully to register itself.

**/
EFI_STATUS
EFIAPI
PeiRegisterForShadow (
  IN EFI_PEI_FILE_HANDLE       FileHandle
  )
{
  PEI_CORE_INSTANCE            *Private;
  Private = PEI_CORE_INSTANCE_FROM_PS_THIS (GetPeiServicesTablePointer ());

  if (Private->CurrentFileHandle != FileHandle) {
    //
    // The FileHandle must be for the current PEIM
    //
    return EFI_NOT_FOUND;
  }

  if (Private->Fv[Private->CurrentPeimFvCount].PeimState[Private->CurrentPeimCount] >= PEIM_STATE_REGISITER_FOR_SHADOW) {
    //
    // If the PEIM has already entered the PEIM_STATE_REGISTER_FOR_SHADOW or PEIM_STATE_DONE then it's already been started
    //
    return EFI_ALREADY_STARTED;
  }

  Private->Fv[Private->CurrentPeimFvCount].PeimState[Private->CurrentPeimCount] = PEIM_STATE_REGISITER_FOR_SHADOW;

  return EFI_SUCCESS;
}



/**
  Get Fv image from the FV type file, then install FV INFO ppi, Build FV hob.

	@param PeiServices          Pointer to the PEI Core Services Table.
	@param FileHandle  	        File handle of a Fv type file.
  @param AuthenticationState  Pointer to attestation authentication state of image.


  @retval EFI_NOT_FOUND  				FV image can't be found.
  @retval EFI_SUCCESS						Successfully to process it.

**/
EFI_STATUS
ProcessFvFile (
  IN  EFI_PEI_SERVICES      **PeiServices,
  IN  EFI_PEI_FILE_HANDLE   FvFileHandle,
  OUT UINT32                *AuthenticationState
  )
{
  EFI_STATUS            Status;
  EFI_PEI_FV_HANDLE     FvImageHandle;
  EFI_FV_INFO           FvImageInfo;
  UINT32                FvAlignment;
  VOID                  *FvBuffer;
  EFI_PEI_HOB_POINTERS  HobFv2;

  FvBuffer             = NULL;
  *AuthenticationState = 0;

  //
  // Check if this EFI_FV_FILETYPE_FIRMWARE_VOLUME_IMAGE file has already
  // been extracted.
  //
  HobFv2.Raw = GetHobList ();
  while ((HobFv2.Raw = GetNextHob (EFI_HOB_TYPE_FV2, HobFv2.Raw)) != NULL) {
    if (CompareGuid (&(((EFI_FFS_FILE_HEADER *)FvFileHandle)->Name), &HobFv2.FirmwareVolume2->FileName)) {
      //
      // this FILE has been dispatched, it will not be dispatched again.
      //
      return EFI_SUCCESS;
    }
    HobFv2.Raw = GET_NEXT_HOB (HobFv2);
  }

  //
  // Find FvImage in FvFile
  //
  Status = PeiFfsFindSectionData (
             (CONST EFI_PEI_SERVICES **) PeiServices,
             EFI_SECTION_FIRMWARE_VOLUME_IMAGE,
             FvFileHandle,
             (VOID **)&FvImageHandle
             );

  if (EFI_ERROR (Status)) {
    return Status;
  }
  //
  // Collect FvImage Info.
  //
  Status = PeiFfsGetVolumeInfo (FvImageHandle, &FvImageInfo);
  ASSERT_EFI_ERROR (Status);
  //
  // FvAlignment must be more than 8 bytes required by FvHeader structure.
  //
  FvAlignment = 1 << ((FvImageInfo.FvAttributes & EFI_FVB2_ALIGNMENT) >> 16);
  if (FvAlignment < 8) {
    FvAlignment = 8;
  }
  //
  // Check FvImage
  //
  if ((UINTN) FvImageInfo.FvStart % FvAlignment != 0) {
    FvBuffer = AllocateAlignedPages (EFI_SIZE_TO_PAGES ((UINT32) FvImageInfo.FvSize), FvAlignment);
    if (FvBuffer == NULL) {
      return EFI_OUT_OF_RESOURCES;
    }
    CopyMem (FvBuffer, FvImageInfo.FvStart, (UINTN) FvImageInfo.FvSize);
    //
    // Update FvImageInfo after reload FvImage to new aligned memory
    //
    PeiFfsGetVolumeInfo ((EFI_PEI_FV_HANDLE) FvBuffer, &FvImageInfo);
  }

  //
  // Install FvPpi and Build FvHob
  //
  PiLibInstallFvInfoPpi (
    NULL,
    FvImageInfo.FvStart,
    (UINT32) FvImageInfo.FvSize,
    &(FvImageInfo.FvName),
    &(((EFI_FFS_FILE_HEADER*)FvFileHandle)->Name)
    );

  //
  // Inform HOB consumer phase, i.e. DXE core, the existance of this FV
  //
  BuildFvHob (
    (EFI_PHYSICAL_ADDRESS) (UINTN) FvImageInfo.FvStart,
    FvImageInfo.FvSize
  );
  //
  // Makes the encapsulated volume show up in DXE phase to skip processing of
  // encapsulated file again.
  //
  BuildFv2Hob (
    (EFI_PHYSICAL_ADDRESS) (UINTN) FvImageInfo.FvStart,
    FvImageInfo.FvSize,
    &FvImageInfo.FvName,
    &(((EFI_FFS_FILE_HEADER *)FvFileHandle)->Name)
    );

  return EFI_SUCCESS;
}
