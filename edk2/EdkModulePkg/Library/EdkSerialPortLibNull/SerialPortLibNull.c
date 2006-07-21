/** @file
  Serial I/O Port library functions with no library constructor/destructor

  Copyright (c) 2006, Intel Corporation
  All rights reserved. This program and the accompanying materials
  are licensed and made available under the terms and conditions of the BSD License
  which accompanies this distribution.  The full text of the license may be found at
  http://opensource.org/licenses/bsd-license.php

  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

  Module Name:  SerialPortLibNull.c

**/

/**

  Programmed hardware of Serial port.

**/
EFI_STATUS
EFIAPI
SerialPortInitialize (
  VOID
  )
{
  return EFI_UNSUPPORTED;
}

/**
  Write data to serial device. 
 
  If the buffer is NULL, then ASSERT(); 
  if NumberOfBytes is zero, then ASSERT(). 

  @param  Buffer           Point of data buffer which need to be writed.
  @param  NumberOfBytes    Number of output bytes which are cached in Buffer.

  @retval 0                Write data failed.
  @retval !0               Actual number of bytes writed to serial device.

**/
UINTN
EFIAPI
SerialPortWrite (
  IN UINT8 	   *Buffer,
  IN UINTN 	   NumberOfBytes
)
{
  return 0;
}


/**
  Read data from serial device and save the datas in buffer.
 
  If the buffer is NULL, then ASSERT(); 
  if NumberOfBytes is zero, then ASSERT(). 

  @param  Buffer           Point of data buffer which need to be writed.
  @param  NumberOfBytes    Number of output bytes which are cached in Buffer.

  @retval 0                Read data failed.
  @retval !0               Aactual number of bytes read from serial device.

**/
UINTN
EFIAPI
SerialPortRead (
  OUT UINT8 	*Buffer,
  IN  UINTN 	NumberOfBytes
)
{
  return 0;
}

