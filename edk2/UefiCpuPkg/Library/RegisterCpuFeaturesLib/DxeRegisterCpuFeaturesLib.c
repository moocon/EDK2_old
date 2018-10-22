/** @file
  CPU Register Table Library functions.

  Copyright (c) 2017, Intel Corporation. All rights reserved.<BR>
  This program and the accompanying materials
  are licensed and made available under the terms and conditions of the BSD License
  which accompanies this distribution.  The full text of the license may be found at
  http://opensource.org/licenses/bsd-license.php

  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

#include <PiDxe.h>

#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiLib.h>

#include "RegisterCpuFeatures.h"

CPU_FEATURES_DATA          mCpuFeaturesData = {0};
EFI_MP_SERVICES_PROTOCOL   *mCpuFeaturesMpServices = NULL;

/**
  Worker function to get CPU_FEATURES_DATA pointer.

  @return Pointer to CPU_FEATURES_DATA.
**/
CPU_FEATURES_DATA *
GetCpuFeaturesData (
  VOID
  )
{
  return &mCpuFeaturesData;
}

/**
  Worker function to get EFI_MP_SERVICES_PROTOCOL pointer.

  @return Pointer to EFI_MP_SERVICES_PROTOCOL.
**/
EFI_MP_SERVICES_PROTOCOL *
GetMpProtocol (
  VOID
  )
{
  EFI_STATUS             Status;

  if (mCpuFeaturesMpServices == NULL) {
    //
    // Get MP Services Protocol
    //
    Status = gBS->LocateProtocol (
                  &gEfiMpServiceProtocolGuid,
                  NULL,
                  (VOID **)&mCpuFeaturesMpServices
                  );
    ASSERT_EFI_ERROR (Status);
  }

  ASSERT (mCpuFeaturesMpServices != NULL);
  return mCpuFeaturesMpServices;
}

/**
  Worker function to return processor index.

  @return  The processor index.
**/
UINTN
GetProcessorIndex (
  VOID
  )
{
  EFI_STATUS                           Status;
  UINTN                                ProcessorIndex;
  EFI_MP_SERVICES_PROTOCOL             *MpServices;

  MpServices = GetMpProtocol ();
  Status = MpServices->WhoAmI(MpServices, &ProcessorIndex);
  ASSERT_EFI_ERROR (Status);
  return ProcessorIndex;
}

/**
  Gets detailed MP-related information on the requested processor at the
  instant this call is made.

  @param[in]  ProcessorNumber       The handle number of processor.
  @param[out] ProcessorInfoBuffer   A pointer to the buffer where information for
                                    the requested processor is deposited.

  @return Status of MpServices->GetProcessorInfo().
**/
EFI_STATUS
GetProcessorInformation (
  IN  UINTN                            ProcessorNumber,
  OUT EFI_PROCESSOR_INFORMATION        *ProcessorInfoBuffer
  )
{
  EFI_STATUS                           Status;
  EFI_MP_SERVICES_PROTOCOL             *MpServices;

  MpServices = GetMpProtocol ();
  Status = MpServices->GetProcessorInfo (
               MpServices,
               ProcessorNumber,
               ProcessorInfoBuffer
               );
  return Status;
}

/**
  Worker function to execute a caller provided function on all enabled APs.

  @param[in]  Procedure               A pointer to the function to be run on
                                      enabled APs of the system.
  @param[in]  MpEvent                 A pointer to the event to be used later
                                      to check whether procedure has done.
**/
VOID
StartupAPsWorker (
  IN  EFI_AP_PROCEDURE                 Procedure,
  IN  EFI_EVENT                        MpEvent
  )
{
  EFI_STATUS                           Status;
  EFI_MP_SERVICES_PROTOCOL             *MpServices;
  CPU_FEATURES_DATA                    *CpuFeaturesData;

  CpuFeaturesData = GetCpuFeaturesData ();

  MpServices = GetMpProtocol ();
  //
  // Wakeup all APs
  //
  Status = MpServices->StartupAllAPs (
                 MpServices,
                 Procedure,
                 FALSE,
                 MpEvent,
                 0,
                 CpuFeaturesData,
                 NULL
                 );
  ASSERT_EFI_ERROR (Status);
}

/**
  Worker function to switch the requested AP to be the BSP from that point onward.

  @param[in] ProcessorNumber   The handle number of AP that is to become the new BSP.
**/
VOID
SwitchNewBsp (
  IN  UINTN                            ProcessorNumber
  )
{
  EFI_STATUS                           Status;
  EFI_MP_SERVICES_PROTOCOL             *MpServices;

  MpServices = GetMpProtocol ();
  //
  // Wakeup all APs
  //
  Status = MpServices->SwitchBSP (
                 MpServices,
                 ProcessorNumber,
                 TRUE
                 );
  ASSERT_EFI_ERROR (Status);
}

/**
  Worker function to retrieve the number of logical processor in the platform.

  @param[out] NumberOfCpus                Pointer to the total number of logical
                                          processors in the system, including the BSP
                                          and disabled APs.
  @param[out] NumberOfEnabledProcessors   Pointer to the number of enabled logical
                                          processors that exist in system, including
                                          the BSP.
**/
VOID
GetNumberOfProcessor (
  OUT UINTN                            *NumberOfCpus,
  OUT UINTN                            *NumberOfEnabledProcessors
  )
{
  EFI_STATUS                           Status;
  EFI_MP_SERVICES_PROTOCOL             *MpServices;

  MpServices = GetMpProtocol ();

  //
  // Get the number of CPUs
  //
  Status = MpServices->GetNumberOfProcessors (
                         MpServices,
                         NumberOfCpus,
                         NumberOfEnabledProcessors
                         );
  ASSERT_EFI_ERROR (Status);
}

/**
  Performs CPU features Initialization.

  This service will invoke MP service to perform CPU features
  initialization on BSP/APs per user configuration.

  @note This service could be called by BSP only.
**/
VOID
EFIAPI
CpuFeaturesInitialize (
  VOID
  )
{
  CPU_FEATURES_DATA          *CpuFeaturesData;
  UINTN                      OldBspNumber;
  EFI_EVENT                  MpEvent;
  EFI_STATUS                 Status;

  CpuFeaturesData = GetCpuFeaturesData ();

  OldBspNumber = GetProcessorIndex();
  CpuFeaturesData->BspNumber = OldBspNumber;

  Status = gBS->CreateEvent (
                  EVT_NOTIFY_WAIT,
                  TPL_CALLBACK,
                  EfiEventEmptyFunction,
                  NULL,
                  &MpEvent
                  );
  ASSERT_EFI_ERROR (Status);

  //
  // Wakeup all APs for programming.
  //
  StartupAPsWorker (SetProcessorRegister, MpEvent);
  //
  // Programming BSP
  //
  SetProcessorRegister (CpuFeaturesData);

  //
  // Wait all processors to finish the task.
  //
  do {
    Status = gBS->CheckEvent (MpEvent);
  } while (Status == EFI_NOT_READY);
  ASSERT_EFI_ERROR (Status);

  //
  // Switch to new BSP if required
  //
  if (CpuFeaturesData->BspNumber != OldBspNumber) {
    SwitchNewBsp (CpuFeaturesData->BspNumber);
  }
}

