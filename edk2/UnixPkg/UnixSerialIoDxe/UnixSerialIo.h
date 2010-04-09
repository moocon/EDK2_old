/*++

Copyright (c) 2006 - 2009, Intel Corporation                                                         
Portions copyright (c) 2008-2009 Apple Inc.
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution.  The full text of the license may be found at        
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module Name:

  UnixSerialIo.h

Abstract:


--*/

#ifndef _UNIXPKG_SERIAL_IO_
#define _UNIXPKG_SERIAL_IO_
#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>

#ifdef __APPLE__
#else
#include <stdlib.h>
#include <termio.h>
#endif

#include <fcntl.h>
#include <errno.h>

#include "Uefi.h"
#include <Protocol/SerialIo.h>
#include <Protocol/DevicePath.h>

#include <Library/DebugLib.h>
#include <Library/BaseLib.h>
#include <Library/UefiDriverEntryPoint.h>
#include <Library/UefiLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/DevicePathLib.h>
#include <Library/MemoryAllocationLib.h>
#include "UnixDxe.h"

extern EFI_DRIVER_BINDING_PROTOCOL gUnixSerialIoDriverBinding;
extern EFI_COMPONENT_NAME_PROTOCOL gUnixSerialIoComponentName;

#define SERIAL_MAX_BUFFER_SIZE  256
#define TIMEOUT_STALL_INTERVAL  10

typedef struct {
  UINT32  First;
  UINT32  Last;
  UINT32  Surplus;
  UINT8   Data[SERIAL_MAX_BUFFER_SIZE];
} SERIAL_DEV_FIFO;

#define UNIX_SERIAL_IO_PRIVATE_DATA_SIGNATURE SIGNATURE_32 ('U', 'N', 's', 'i')
typedef struct {
  UINT64                    Signature;

  //
  // Protocol data for the new handle we are going to add
  //
  EFI_HANDLE                Handle;
  EFI_SERIAL_IO_PROTOCOL    SerialIo;
  EFI_SERIAL_IO_MODE        SerialIoMode;
  EFI_DEVICE_PATH_PROTOCOL  *DevicePath;

  //
  // Private Data
  //
  EFI_HANDLE                ControllerHandle;
  EFI_DEVICE_PATH_PROTOCOL  *ParentDevicePath;
  UART_DEVICE_PATH          UartDevicePath;
  EFI_UNIX_THUNK_PROTOCOL   *UnixThunk;

  EFI_UNICODE_STRING_TABLE  *ControllerNameTable;

  //
  // Private NT type Data;
  //
  UINTN                       UnixHandle;
  struct termios              UnixTermios;

  BOOLEAN                   SoftwareLoopbackEnable;
  BOOLEAN                   HardwareFlowControl;
  BOOLEAN                   HardwareLoopbackEnable;

  SERIAL_DEV_FIFO           Fifo;

} UNIX_SERIAL_IO_PRIVATE_DATA;

#define UNIX_SERIAL_IO_PRIVATE_DATA_FROM_THIS(a) \
         CR(a, UNIX_SERIAL_IO_PRIVATE_DATA, SerialIo, UNIX_SERIAL_IO_PRIVATE_DATA_SIGNATURE)

//
// Global Protocol Variables
//
extern EFI_DRIVER_BINDING_PROTOCOL  gUnixSerialIoDriverBinding;
extern EFI_COMPONENT_NAME_PROTOCOL  gUnixSerialIoComponentName;

//
// Macros to convert EFI serial types to NT serial types.
//

//
// one second
//
#define SERIAL_TIMEOUT_DEFAULT  (1000 * 1000) 
#define SERIAL_BAUD_DEFAULT     115200
#define SERIAL_FIFO_DEFAULT     14
#define SERIAL_DATABITS_DEFAULT 8
#define SERIAL_PARITY_DEFAULT   DefaultParity
#define SERIAL_STOPBITS_DEFAULT DefaultStopBits

#define SERIAL_CONTROL_MASK     (EFI_SERIAL_CLEAR_TO_SEND                | \
                                 EFI_SERIAL_DATA_SET_READY               | \
                                 EFI_SERIAL_RING_INDICATE                | \
                                 EFI_SERIAL_CARRIER_DETECT               | \
                                 EFI_SERIAL_REQUEST_TO_SEND              | \
                                 EFI_SERIAL_DATA_TERMINAL_READY          | \
                                 EFI_SERIAL_HARDWARE_LOOPBACK_ENABLE     | \
                                 EFI_SERIAL_SOFTWARE_LOOPBACK_ENABLE     | \
                                 EFI_SERIAL_HARDWARE_FLOW_CONTROL_ENABLE | \
                                 EFI_SERIAL_INPUT_BUFFER_EMPTY)

#define ConvertBaud2Nt(x)       (DWORD) x
#define ConvertData2Nt(x)       (BYTE) x

#define ConvertParity2Nt(x)              \
    (BYTE) (                             \
    x == DefaultParity ? NOPARITY    :   \
    x == NoParity      ? NOPARITY    :   \
    x == EvenParity    ? EVENPARITY  :   \
    x == OddParity     ? ODDPARITY   :   \
    x == MarkParity    ? MARKPARITY  :   \
    x == SpaceParity   ? SPACEPARITY : 0 \
    )

#define ConvertStop2Nt(x)                 \
    (BYTE) (                                \
    x == DefaultParity   ? ONESTOPBIT   :   \
    x == OneFiveStopBits ? ONE5STOPBITS :   \
    x == TwoStopBits     ? TWOSTOPBITS  : 0 \
    )

#define ConvertTime2Nt(x) ((x) / 1000)

//
// 115400 baud with rounding errors
//
#define SERIAL_PORT_MAX_BAUD_RATE 115400  

//
// Fix the differences issue of linux header files termios.h 
// 
#ifndef B460800
#define B460800 0010004
#endif
#ifndef B500000
#define   B500000 0010005
#endif
#ifndef B576000
#define   B576000 0010006
#endif
#ifndef B921600
#define   B921600 0010007
#endif
#ifndef B1000000
#define  B1000000 0010010
#endif
#ifndef B1152000
#define  B1152000 0010011
#endif
#ifndef B1500000
#define  B1500000 0010012
#endif
#ifndef B2000000
#define  B2000000 0010013
#endif
#ifndef B2500000
#define  B2500000 0010014
#endif
#ifndef B3000000
#define  B3000000 0010015
#endif
#ifndef B3500000
#define  B3500000 0010016
#endif
#ifndef B4000000
#define  B4000000 0010017
#endif
#ifndef __MAX_BAUD
#define __MAX_BAUD B4000000
#endif
#ifndef CMSPAR
#define CMSPAR	  010000000000		/* mark or space (stick) parity */
#endif
#ifndef FIONREAD
#define FIONREAD	0x541B
#endif
//
// Function Prototypes
//
EFI_STATUS
EFIAPI
InitializeUnixSerialIo (
  IN EFI_HANDLE           ImageHandle,
  IN EFI_SYSTEM_TABLE     *SystemTable
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
EFIAPI
UnixSerialIoDriverBindingSupported (
  IN  EFI_DRIVER_BINDING_PROTOCOL     *This,
  IN  EFI_HANDLE                      Handle,
  IN  EFI_DEVICE_PATH_PROTOCOL        *RemainingDevicePath
  )
/*++

Routine Description:

  TODO: Add function description

Arguments:

  This                - TODO: add argument description
  Handle              - TODO: add argument description
  RemainingDevicePath - TODO: add argument description

Returns:

  TODO: add return values

--*/
;

EFI_STATUS
EFIAPI
UnixSerialIoDriverBindingStart (
  IN  EFI_DRIVER_BINDING_PROTOCOL     *This,
  IN  EFI_HANDLE                      Handle,
  IN  EFI_DEVICE_PATH_PROTOCOL        *RemainingDevicePath
  )
/*++

Routine Description:

  TODO: Add function description

Arguments:

  This                - TODO: add argument description
  Handle              - TODO: add argument description
  RemainingDevicePath - TODO: add argument description

Returns:

  TODO: add return values

--*/
;

EFI_STATUS
EFIAPI
UnixSerialIoDriverBindingStop (
  IN  EFI_DRIVER_BINDING_PROTOCOL   *This,
  IN  EFI_HANDLE                    Handle,
  IN  UINTN                         NumberOfChildren,
  IN  EFI_HANDLE                    *ChildHandleBuffer
  )
/*++

Routine Description:

  TODO: Add function description

Arguments:

  This              - TODO: add argument description
  Handle            - TODO: add argument description
  NumberOfChildren  - TODO: add argument description
  ChildHandleBuffer - TODO: add argument description

Returns:

  TODO: add return values

--*/
;

EFI_STATUS
EFIAPI
UnixSerialIoReset (
  IN EFI_SERIAL_IO_PROTOCOL *This
  )
/*++

Routine Description:

  TODO: Add function description

Arguments:

  This  - TODO: add argument description

Returns:

  TODO: add return values

--*/
;

EFI_STATUS
EFIAPI
UnixSerialIoSetAttributes (
  IN EFI_SERIAL_IO_PROTOCOL *This,
  IN UINT64                 BaudRate,
  IN UINT32                 ReceiveFifoDepth,
  IN UINT32                 Timeout,
  IN EFI_PARITY_TYPE        Parity,
  IN UINT8                  DataBits,
  IN EFI_STOP_BITS_TYPE     StopBits
  )
/*++

Routine Description:

  TODO: Add function description

Arguments:

  This              - TODO: add argument description
  BaudRate          - TODO: add argument description
  ReceiveFifoDepth  - TODO: add argument description
  Timeout           - TODO: add argument description
  Parity            - TODO: add argument description
  DataBits          - TODO: add argument description
  StopBits          - TODO: add argument description

Returns:

  TODO: add return values

--*/
;

EFI_STATUS
EFIAPI
UnixSerialIoSetControl (
  IN EFI_SERIAL_IO_PROTOCOL *This,
  IN UINT32                 Control
  )
/*++

Routine Description:

  TODO: Add function description

Arguments:

  This    - TODO: add argument description
  Control - TODO: add argument description

Returns:

  TODO: add return values

--*/
;

EFI_STATUS
EFIAPI
UnixSerialIoGetControl (
  IN  EFI_SERIAL_IO_PROTOCOL  *This,
  OUT UINT32                  *Control
  )
/*++

Routine Description:

  TODO: Add function description

Arguments:

  This    - TODO: add argument description
  Control - TODO: add argument description

Returns:

  TODO: add return values

--*/
;

EFI_STATUS
EFIAPI
UnixSerialIoWrite (
  IN EFI_SERIAL_IO_PROTOCOL   *This,
  IN OUT UINTN                *BufferSize,
  IN VOID                     *Buffer
  )
/*++

Routine Description:

  TODO: Add function description

Arguments:

  This        - TODO: add argument description
  BufferSize  - TODO: add argument description
  Buffer      - TODO: add argument description

Returns:

  TODO: add return values

--*/
;

EFI_STATUS
EFIAPI
UnixSerialIoRead (
  IN  EFI_SERIAL_IO_PROTOCOL  *This,
  IN  OUT UINTN               *BufferSize,
  OUT VOID                    *Buffer
  )
/*++

Routine Description:

  TODO: Add function description

Arguments:

  This        - TODO: add argument description
  BufferSize  - TODO: add argument description
  Buffer      - TODO: add argument description

Returns:

  TODO: add return values

--*/
;

BOOLEAN
IsaSerialFifoFull (
  IN SERIAL_DEV_FIFO *Fifo
  )
/*++

Routine Description:

  TODO: Add function description

Arguments:

  Fifo  - TODO: add argument description

Returns:

  TODO: add return values

--*/
;

BOOLEAN
IsaSerialFifoEmpty (
  IN SERIAL_DEV_FIFO *Fifo
  )
/*++

Routine Description:

  TODO: Add function description

Arguments:

  Fifo  - TODO: add argument description

Returns:

  TODO: add return values

--*/
;

EFI_STATUS
IsaSerialFifoAdd (
  IN SERIAL_DEV_FIFO *Fifo,
  IN UINT8           Data
  )
/*++

Routine Description:

  TODO: Add function description

Arguments:

  Fifo  - TODO: add argument description
  Data  - TODO: add argument description

Returns:

  TODO: add return values

--*/
;

EFI_STATUS
IsaSerialFifoRemove (
  IN  SERIAL_DEV_FIFO *Fifo,
  OUT UINT8           *Data
  )
/*++

Routine Description:

  TODO: Add function description

Arguments:

  Fifo  - TODO: add argument description
  Data  - TODO: add argument description

Returns:

  TODO: add return values

--*/
;

EFI_STATUS
IsaSerialReceiveTransmit (
  UNIX_SERIAL_IO_PRIVATE_DATA     *Private
  )
/*++

Routine Description:

  TODO: Add function description

Arguments:

  Private - TODO: add argument description

Returns:

  TODO: add return values

--*/
;

#endif
