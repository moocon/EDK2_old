/** @file
  Unaligned access functions of BaseLib for IPF.

  Copyright (c) 2006, Intel Corporation<BR>
  All rights reserved. This program and the accompanying materials
  are licensed and made available under the terms and conditions of the BSD License
  which accompanies this distribution.  The full text of the license may be found at
  http://opensource.org/licenses/bsd-license.php

  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

  Module Name:  Unaligned.c

**/

/**
  Reads a 16-bit value from memory that may be unaligned.

  This function returns the 16-bit value pointed to by Buffer. The function
  guarantees that the read operation does not produce an alignment fault.

  If the Buffer is NULL, then ASSERT().

  @param  Buffer  Pointer to a 16-bit value that may be unaligned.

  @return *Uint16

**/
UINT16
EFIAPI
ReadUnaligned16 (
  IN      CONST UINT16              *Buffer
  )
{
  ASSERT (Buffer != NULL);

  return (UINT16)(((UINT8*)Buffer)[0] | (((UINT8*)Buffer)[1] << 8));
}

/**
  Writes a 16-bit value to memory that may be unaligned.

  This function writes the 16-bit value specified by Value to Buffer. Value is
  returned. The function guarantees that the write operation does not produce
  an alignment fault.

  If the Buffer is NULL, then ASSERT().

  @param  Buffer  Pointer to a 16-bit value that may be unaligned.
  @param  Value   16-bit value to write to Buffer.

  @return Value

**/
UINT16
EFIAPI
WriteUnaligned16 (
  OUT     UINT16                    *Buffer,
  IN      UINT16                    Value
  )
{
  ASSERT (Buffer != NULL);

  ((UINT8*)Buffer)[0] = (UINT8)Value;
  ((UINT8*)Buffer)[1] = (UINT8)(Value >> 8);

  return Value;
}

/**
  Reads a 24-bit value from memory that may be unaligned.

  This function returns the 24-bit value pointed to by Buffer. The function
  guarantees that the read operation does not produce an alignment fault.

  If the Buffer is NULL, then ASSERT().

  @param  Buffer  Pointer to a 24-bit value that may be unaligned.

  @return The value read.

**/
UINT32
EFIAPI
ReadUnaligned24 (
  IN      CONST UINT32              *Buffer
  )
{
  ASSERT (Buffer != NULL);

  return (UINT32)(
            ReadUnaligned16 ((UINT16*)Buffer) |
            (((UINT8*)Buffer)[2] << 16)
            );
}

/**
  Writes a 24-bit value to memory that may be unaligned.

  This function writes the 24-bit value specified by Value to Buffer. Value is
  returned. The function guarantees that the write operation does not produce
  an alignment fault.

  If the Buffer is NULL, then ASSERT().

  @param  Buffer  Pointer to a 24-bit value that may be unaligned.
  @param  Value   24-bit value to write to Buffer.

  @return The value written.

**/
UINT32
EFIAPI
WriteUnaligned24 (
  OUT     UINT32                    *Buffer,
  IN      UINT32                    Value
  )
{
  ASSERT (Buffer != NULL);

  WriteUnaligned16 ((UINT16*)Buffer, (UINT16)Value);
  *(UINT8*)((UINT16*)Buffer + 1) = (UINT8)(Value >> 16);
  return Value;
}

/**
  Reads a 32-bit value from memory that may be unaligned.

  This function returns the 32-bit value pointed to by Buffer. The function
  guarantees that the read operation does not produce an alignment fault.

  If the Buffer is NULL, then ASSERT().

  @param  Buffer  Pointer to a 32-bit value that may be unaligned.

  @return *Uint32

**/
UINT32
EFIAPI
ReadUnaligned32 (
  IN      CONST UINT32              *Buffer
  )
{
  ASSERT (Buffer != NULL);

  return (UINT32)(
           ReadUnaligned16 ((UINT16*)Buffer) |
           (ReadUnaligned16 ((UINT16*)Buffer + 1) << 16)
           );
}

/**
  Writes a 32-bit value to memory that may be unaligned.

  This function writes the 32-bit value specified by Value to Buffer. Value is
  returned. The function guarantees that the write operation does not produce
  an alignment fault.

  If the Buffer is NULL, then ASSERT().

  @param  Buffer  Pointer to a 32-bit value that may be unaligned.
  @param  Value   32-bit value to write to Buffer.

  @return Value

**/
UINT32
EFIAPI
WriteUnaligned32 (
  OUT     UINT32                    *Buffer,
  IN      UINT32                    Value
  )
{
  ASSERT (Buffer != NULL);

  WriteUnaligned16 ((UINT16*)Buffer, (UINT16)Value);
  WriteUnaligned16 ((UINT16*)Buffer + 1, (UINT16)(Value >> 16));
  return Value;
}

/**
  Reads a 64-bit value from memory that may be unaligned.

  This function returns the 64-bit value pointed to by Buffer. The function
  guarantees that the read operation does not produce an alignment fault.

  If the Buffer is NULL, then ASSERT().

  @param  Buffer  Pointer to a 64-bit value that may be unaligned.

  @return *Uint64

**/
UINT64
EFIAPI
ReadUnaligned64 (
  IN      CONST UINT64              *Buffer
  )
{
  ASSERT (Buffer != NULL);

  return (UINT64)(
           ReadUnaligned32 ((UINT32*)Buffer) |
           LShiftU64 (ReadUnaligned32 ((UINT32*)Buffer + 1), 32)
           );
}

/**
  Writes a 64-bit value to memory that may be unaligned.

  This function writes the 64-bit value specified by Value to Buffer. Value is
  returned. The function guarantees that the write operation does not produce
  an alignment fault.

  If the Buffer is NULL, then ASSERT().

  @param  Buffer  Pointer to a 64-bit value that may be unaligned.
  @param  Value   64-bit value to write to Buffer.

  @return Value

**/
UINT64
EFIAPI
WriteUnaligned64 (
  OUT     UINT64                    *Buffer,
  IN      UINT64                    Value
  )
{
  ASSERT (Buffer != NULL);

  WriteUnaligned32 ((UINT32*)Buffer, (UINT32)Value);
  WriteUnaligned32 ((UINT32*)Buffer + 1, (UINT32)RShiftU64 (Value, 32));
  return Value;
}
