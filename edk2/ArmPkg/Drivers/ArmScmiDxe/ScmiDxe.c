/** @file

  Copyright (c) 2017-2018, Arm Limited. All rights reserved.

  This program and the accompanying materials
  are licensed and made available under the terms and conditions of the BSD License
  which accompanies this distribution.  The full text of the license may be found at
  http://opensource.org/licenses/bsd-license.php

  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

  System Control and Management Interface V1.0
    http://infocenter.arm.com/help/topic/com.arm.doc.den0056a/
    DEN0056A_System_Control_and_Management_Interface.pdf
**/

#include <Base.h>
#include <Library/DebugLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Protocol/ArmScmiBaseProtocol.h>
#include <Protocol/ArmScmiClockProtocol.h>
#include <Protocol/ArmScmiPerformanceProtocol.h>

#include "ArmScmiBaseProtocolPrivate.h"
#include "ArmScmiClockProtocolPrivate.h"
#include "ArmScmiPerformanceProtocolPrivate.h"
#include "ScmiDxe.h"
#include "ScmiPrivate.h"

STATIC CONST SCMI_PROTOCOL_INIT_TABLE ProtocolInitFxns[MAX_PROTOCOLS] = {
  { ScmiBaseProtocolInit },
  { NULL },
  { NULL },
  { ScmiPerformanceProtocolInit },
  { ScmiClockProtocolInit },
  { NULL }
};

/** ARM SCMI driver entry point function.

  This function installs the SCMI Base protocol and a list of other
  protocols is queried using the Base protocol. If protocol is supported,
  driver will call each protocol init function to install the protocol on
  the ImageHandle.

  @param[in] ImageHandle     Handle to this EFI Image which will be used to
                             install Base, Clock and Performance protocols.
  @param[in] SystemTable     A pointer to boot time system table.

  @retval EFI_SUCCESS       Driver initalized successfully.
  @retval EFI_UNSUPPORTED   If SCMI base protocol version is not supported.
  @retval !(EFI_SUCCESS)    Other errors.
**/
EFI_STATUS
EFIAPI
ArmScmiDxeEntryPoint (
  IN EFI_HANDLE             ImageHandle,
  IN EFI_SYSTEM_TABLE       *SystemTable
  )
{
  EFI_STATUS          Status;
  SCMI_BASE_PROTOCOL  *BaseProtocol;
  UINT32              Version;
  UINT32              Index;
  UINT32              NumProtocols;
  UINT32              ProtocolNo;
  UINT8               SupportedList[MAX_PROTOCOLS];
  UINT32              SupportedListSize = sizeof (SupportedList);

  ProtocolNo = SCMI_PROTOCOL_ID_BASE & PROTOCOL_ID_MASK;

  // Every SCMI implementation must implement the base protocol.
  Status = ProtocolInitFxns[ProtocolNo].Init (&ImageHandle);
  if (EFI_ERROR (Status)) {
    ASSERT (FALSE);
    return Status;
  }

  Status = gBS->LocateProtocol (
                  &gArmScmiBaseProtocolGuid,
                  NULL,
                  (VOID**)&BaseProtocol
                  );
  if (EFI_ERROR (Status)) {
    ASSERT (FALSE);
    return Status;
  }

  // Get SCMI Base protocol version.
  Status = BaseProtocol->GetVersion (BaseProtocol, &Version);
  if (EFI_ERROR (Status)) {
    ASSERT (FALSE);
    return Status;
  }

  if (Version != BASE_PROTOCOL_VERSION) {
    ASSERT (FALSE);
    return EFI_UNSUPPORTED;
  }

  // Apart from Base protocol, SCMI may implement various other protocols,
  // query total protocols implemented by the SCP firmware.
  NumProtocols = 0;
  Status = BaseProtocol->GetTotalProtocols (BaseProtocol, &NumProtocols);
  if (EFI_ERROR (Status)) {
    ASSERT (FALSE);
    return Status;
  }

  ASSERT (NumProtocols != 0);

  // Get the list of protocols supported by SCP firmware on the platform.
  Status = BaseProtocol->DiscoverListProtocols (
             BaseProtocol,
             &SupportedListSize,
             SupportedList
             );
  if (EFI_ERROR (Status)) {
    ASSERT (FALSE);
    return Status;
  }

  // Install supported protocol on ImageHandle.
  for (Index = 0; Index < NumProtocols; Index++) {
    ProtocolNo = SupportedList[Index] & PROTOCOL_ID_MASK;
    if (ProtocolInitFxns[ProtocolNo].Init != NULL) {
      Status = ProtocolInitFxns[ProtocolNo].Init (&ImageHandle);
      if (EFI_ERROR (Status)) {
        ASSERT (FALSE);
        return Status;
      }
    }
  }

  return EFI_SUCCESS;
}
