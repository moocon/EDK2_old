/** @file
*
*  Copyright (c) 2011, ARM Limited. All rights reserved.
*
*  This program and the accompanying materials
*  are licensed and made available under the terms and conditions of the BSD License
*  which accompanies this distribution.  The full text of the license may be found at
*  http://opensource.org/licenses/bsd-license.php
*
*  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
*  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.
*
**/

#ifndef _PREPI_H_
#define _PREPI_H_

#include <PiPei.h>

#include <Library/PcdLib.h>
#include <Library/ArmLib.h>
#include <Library/DebugLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/HobLib.h>
#include <Library/SerialPortLib.h>
#include <Library/ArmPlatformLib.h>

#include <Chipset/ArmV7.h>

#define ARM_PRIMARY_CORE  0
#define SerialPrint(txt)  SerialPortWrite (txt, AsciiStrLen(txt)+1);

// Vector Table for PrePi Phase
VOID
PrePiVectorTable (
  VOID
  );

RETURN_STATUS
EFIAPI
TimerConstructor (
  VOID
  );

VOID
PrePiMain (
  IN  UINTN                     UefiMemoryBase,
  IN  UINT64                    StartTimeStamp
  );

EFI_STATUS
EFIAPI
MemoryPeim (
  IN EFI_PHYSICAL_ADDRESS       UefiMemoryBase,
  IN UINT64                     UefiMemorySize
  );

EFI_STATUS
EFIAPI
PlatformPeim (
  VOID
  );

VOID
PrimaryMain (
  IN  UINTN                     UefiMemoryBase,
  IN  UINT64                    StartTimeStamp
  );

VOID
SecondaryMain (
  IN  UINTN                     CoreId
  );

// Either implemented by PrePiLib or by MemoryInitPei
VOID
BuildMemoryTypeInformationHob (
  VOID
  );

#endif /* _PREPI_H_ */
