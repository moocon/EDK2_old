/*++

Copyright (c) 2005, Intel Corporation                                                         
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution.  The full text of the license may be found at        
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module Name:
  PcRtc.h

Abstract:

    Include for real time clock driver

Revision History


--*/

#ifndef _RTC_H_
#define _RTC_H_

#include <FrameworkDxe.h>

#include <Protocol/RealTimeClock.h>

#include <Library/UefiLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/IoLib.h>
#include <Library/TimerLib.h>
#include <Library/BaseMemoryLib.h>

typedef struct {
  EFI_LOCK  RtcLock;
  UINT16    SavedTimeZone;
  UINT8     Daylight;
} PC_RTC_MODULE_GLOBALS;

#define PCAT_RTC_ADDRESS_REGISTER 0x70
#define PCAT_RTC_DATA_REGISTER    0x71

//
// Dallas DS12C887 Real Time Clock
//
#define RTC_ADDRESS_SECONDS           0   // R/W  Range 0..59
#define RTC_ADDRESS_SECONDS_ALARM     1   // R/W  Range 0..59
#define RTC_ADDRESS_MINUTES           2   // R/W  Range 0..59
#define RTC_ADDRESS_MINUTES_ALARM     3   // R/W  Range 0..59
#define RTC_ADDRESS_HOURS             4   // R/W  Range 1..12 or 0..23 Bit 7 is AM/PM
#define RTC_ADDRESS_HOURS_ALARM       5   // R/W  Range 1..12 or 0..23 Bit 7 is AM/PM
#define RTC_ADDRESS_DAY_OF_THE_WEEK   6   // R/W  Range 1..7
#define RTC_ADDRESS_DAY_OF_THE_MONTH  7   // R/W  Range 1..31
#define RTC_ADDRESS_MONTH             8   // R/W  Range 1..12
#define RTC_ADDRESS_YEAR              9   // R/W  Range 0..99
#define RTC_ADDRESS_REGISTER_A        10  // R/W[0..6]  R0[7]
#define RTC_ADDRESS_REGISTER_B        11  // R/W
#define RTC_ADDRESS_REGISTER_C        12  // RO
#define RTC_ADDRESS_REGISTER_D        13  // RO
#define RTC_ADDRESS_CENTURY           50  // R/W  Range 19..20 Bit 8 is R/W
//
// Date and time initial values.
// They are used if the RTC values are invalid during driver initialization
//
#define RTC_INIT_SECOND 0
#define RTC_INIT_MINUTE 0
#define RTC_INIT_HOUR   0
#define RTC_INIT_DAY    1
#define RTC_INIT_MONTH  1
#define RTC_INIT_YEAR   2001

//
// Register initial values
//
#define RTC_INIT_REGISTER_A 0x26
#define RTC_INIT_REGISTER_B 0x02
#define RTC_INIT_REGISTER_D 0x0

#pragma pack(1)
//
// Register A
//
typedef struct {
  UINT8 RS : 4;   // Rate Selection Bits
  UINT8 DV : 3;   // Divisor
  UINT8 UIP : 1;  // Update in progress
} RTC_REGISTER_A_BITS;

typedef union {
  RTC_REGISTER_A_BITS Bits;
  UINT8               Data;
} RTC_REGISTER_A;

//
// Register B
//
typedef struct {
  UINT8 DSE : 1;  // 0 - Daylight saving disabled  1 - Daylight savings enabled
  UINT8 MIL : 1;  // 0 - 12 hour mode              1 - 24 hour mode
  UINT8 DM : 1;   // 0 - BCD Format                1 - Binary Format
  UINT8 SQWE : 1; // 0 - Disable SQWE output       1 - Enable SQWE output
  UINT8 UIE : 1;  // 0 - Update INT disabled       1 - Update INT enabled
  UINT8 AIE : 1;  // 0 - Alarm INT disabled        1 - Alarm INT Enabled
  UINT8 PIE : 1;  // 0 - Periodic INT disabled     1 - Periodic INT Enabled
  UINT8 SET : 1;  // 0 - Normal operation.         1 - Updates inhibited
} RTC_REGISTER_B_BITS;

typedef union {
  RTC_REGISTER_B_BITS Bits;
  UINT8               Data;
} RTC_REGISTER_B;

//
// Register C
//
typedef struct {
  UINT8 Reserved : 4; // Read as zero.  Can not be written.
  UINT8 UF : 1;       // Update End Interrupt Flag
  UINT8 AF : 1;       // Alarm Interrupt Flag
  UINT8 PF : 1;       // Periodic Interrupt Flag
  UINT8 IRQF : 1;     // Iterrupt Request Flag = PF & PIE | AF & AIE | UF & UIE
} RTC_REGISTER_C_BITS;

typedef union {
  RTC_REGISTER_C_BITS Bits;
  UINT8               Data;
} RTC_REGISTER_C;

//
// Register D
//
typedef struct {
  UINT8 Reserved : 7; // Read as zero.  Can not be written.
  UINT8 VRT : 1;      // Valid RAM and Time
} RTC_REGISTER_D_BITS;

typedef union {
  RTC_REGISTER_D_BITS Bits;
  UINT8               Data;
} RTC_REGISTER_D;

#pragma pack()

EFI_STATUS
PcRtcInit (
  IN PC_RTC_MODULE_GLOBALS  *Global
  )
/*++

Routine Description:

  TODO: Add function description

Arguments:

  Global  - TODO: add argument description

Returns:

  TODO: add return values

--*/
;

EFI_STATUS
PcRtcSetTime (
  IN EFI_TIME               *Time,
  IN PC_RTC_MODULE_GLOBALS  *Global
  )
/*++

Routine Description:

  TODO: Add function description

Arguments:

  Time    - TODO: add argument description
  Global  - TODO: add argument description

Returns:

  TODO: add return values

--*/
;

EFI_STATUS
PcRtcGetTime (
  OUT EFI_TIME              *Time,
  IN  EFI_TIME_CAPABILITIES *Capabilities,
  IN  PC_RTC_MODULE_GLOBALS *Global
  )
/*++

Routine Description:

  TODO: Add function description

Arguments:

  Time          - TODO: add argument description
  Capabilities  - TODO: add argument description
  Global        - TODO: add argument description

Returns:

  TODO: add return values

--*/
;

EFI_STATUS
EFIAPI
PcRtcSetWakeupTime (
  IN BOOLEAN                Enable,
  OUT EFI_TIME              *Time,
  IN  PC_RTC_MODULE_GLOBALS *Global
  )
/*++

Routine Description:

  TODO: Add function description

Arguments:

  Enable  - TODO: add argument description
  Time    - TODO: add argument description
  Global  - TODO: add argument description

Returns:

  TODO: add return values

--*/
;

EFI_STATUS
EFIAPI
PcRtcGetWakeupTime (
  OUT BOOLEAN               *Enabled,
  OUT BOOLEAN               *Pending,
  OUT EFI_TIME              *Time,
  IN  PC_RTC_MODULE_GLOBALS *Global
  )
/*++

Routine Description:

  TODO: Add function description

Arguments:

  Enabled - TODO: add argument description
  Pending - TODO: add argument description
  Time    - TODO: add argument description
  Global  - TODO: add argument description

Returns:

  TODO: add return values

--*/
;

EFI_STATUS
EFIAPI
InitializePcRtc (
  IN EFI_HANDLE                            ImageHandle,
  IN EFI_SYSTEM_TABLE                      *SystemTable
  )
/*++

Routine Description:

  TODO: Add function description

Arguments:

  ImageHandle - TODO: add argument description
  SystemTable - TODO: add argument description

Returns:

  TODO: add return values

--*/
;

EFI_STATUS
RtcTimeFieldsValid (
  IN EFI_TIME *Time
  )
/*++

Routine Description:

  TODO: Add function description

Arguments:

  Time  - TODO: add argument description

Returns:

  TODO: add return values

--*/
;

UINT8
DecimaltoBcd (
  IN  UINT8 DecValue
  )
/*++

Routine Description:

  TODO: Add function description

Arguments:

  DecValue  - TODO: add argument description

Returns:

  TODO: add return values

--*/
;

VOID
ConvertEfiTimeToRtcTime (
  IN EFI_TIME       *Time,
  IN RTC_REGISTER_B RegisterB,
  IN UINT8          *Century
  )
/*++

Routine Description:

  TODO: Add function description

Arguments:

  Time      - TODO: add argument description
  RegisterB - TODO: add argument description
  Century   - TODO: add argument description

Returns:

  TODO: add return values

--*/
;

VOID
ConvertRtcTimeToEfiTime (
  IN EFI_TIME       *Time,
  IN RTC_REGISTER_B RegisterB
  )
/*++

Routine Description:

  TODO: Add function description

Arguments:

  Time      - TODO: add argument description
  RegisterB - TODO: add argument description

Returns:

  TODO: add return values

--*/
;

EFI_STATUS
RtcWaitToUpdate (
  UINTN Timeout
  )
/*++

Routine Description:

  TODO: Add function description

Arguments:

  Timeout - TODO: add argument description

Returns:

  TODO: add return values

--*/
;

UINT8
RtcSaveContext (
  IN  PC_RTC_MODULE_GLOBALS  *Global
  )
/*++

Routine Description:

  TODO: Add function description

Arguments:

  Global  - TODO: add argument description

Returns:

  TODO: add return values

--*/
;

VOID
RtcRestoreContext (
  IN  UINT8                 SavedAddressRegister,
  IN  PC_RTC_MODULE_GLOBALS *Global
  )
/*++

Routine Description:

  TODO: Add function description

Arguments:

  SavedAddressRegister  - TODO: add argument description
  Global                - TODO: add argument description

Returns:

  TODO: add return values

--*/
;

#endif
