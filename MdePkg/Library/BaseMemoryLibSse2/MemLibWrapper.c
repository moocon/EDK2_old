/** @file
  Wrapper functions for Base Memory Library.

  Copyright (c) 2006, Intel Corporation<BR>
  All rights reserved. This program and the accompanying materials
  are licensed and made available under the terms and conditions of the BSD License
  which accompanies this distribution.  The full text of the license may be found at
  http://opensource.org/licenses/bsd-license.php

  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

  Module Name:  MemLibWrapper.c

  The following BaseMemoryLib instances share the same version of this file:

    BaseMemoryLib
    BaseMemoryLibMmx
    BaseMemoryLibSse2
    BaseMemoryLibRepStr
    PeiMemoryLib
    UefiMemoryLib

**/

/**
  Copy Length bytes from Source to Destination.

  @param  Destination Target of copy
  @param  Source Place to copy from
  @param  Length Number of bytes to copy

  @return Destination

**/
VOID *
EFIAPI
InternalMemCopyMem (
  OUT     VOID                      *DestinationBuffer,
  IN      CONST VOID                *SourceBuffer,
  IN      UINTN                     Length
  );

/**
  Set Buffer to Value for Size bytes.

  @param  Buffer Memory to set.
  @param  Size Number of bytes to set
  @param  Value Value of the set operation.

  @return Buffer

**/
VOID *
EFIAPI
InternalMemSetMem (
  OUT     VOID                      *Buffer,
  IN      UINTN                     Length,
  IN      UINT8                     Value
  );

/**
  Fills a target buffer with a 16-bit value, and returns the target buffer.

  @param  Buffer  Pointer to the target buffer to fill.
  @param  Length  Number of bytes in Buffer to fill.
  @param  Value   Value with which to fill Length bytes of Buffer.

  @return Buffer

**/
VOID *
EFIAPI
InternalMemSetMem16 (
  OUT     VOID                      *Buffer,
  IN      UINTN                     Length,
  IN      UINT16                    Value
  );

/**
  Fills a target buffer with a 32-bit value, and returns the target buffer.

  @param  Buffer  Pointer to the target buffer to fill.
  @param  Length  Number of bytes in Buffer to fill.
  @param  Value   Value with which to fill Length bytes of Buffer.

  @return Buffer

**/
VOID *
EFIAPI
InternalMemSetMem32 (
  OUT     VOID                      *Buffer,
  IN      UINTN                     Length,
  IN      UINT32                    Value
  );

/**
  Fills a target buffer with a 64-bit value, and returns the target buffer.

  @param  Buffer  Pointer to the target buffer to fill.
  @param  Length  Number of bytes in Buffer to fill.
  @param  Value   Value with which to fill Length bytes of Buffer.

  @return Buffer

**/
VOID *
EFIAPI
InternalMemSetMem64 (
  OUT     VOID                      *Buffer,
  IN      UINTN                     Length,
  IN      UINT64                    Value
  );

/**
  Set Buffer to 0 for Size bytes.

  @param  Buffer Memory to set.
  @param  Size Number of bytes to set

  @return Buffer

**/
VOID *
EFIAPI
InternalMemZeroMem (
  OUT     VOID                      *Buffer,
  IN      UINTN                     Length
  );

/**
  Compares two memory buffers of a given length.

  @param  DestinationBuffer First memory buffer
  @param  SourceBuffer      Second memory buffer
  @param  Length            Length of DestinationBuffer and SourceBuffer memory
                            regions to compare. Must be non-zero.

  @retval 0     if MemOne == MemTwo

**/
INTN
EFIAPI
InternalMemCompareMem (
  IN      CONST VOID                *DestinationBuffer,
  IN      CONST VOID                *SourceBuffer,
  IN      UINTN                     Length
  );

/**
  Scans a target buffer for an 8-bit value, and returns a pointer to the
  matching 8-bit value in the target buffer.

  @param  Buffer  Pointer to the target buffer to scan.
  @param  Length  Number of bytes in Buffer to scan. Must be non-zero.
  @param  Value   Value to search for in the target buffer.

  @return Pointer to the first occurrence or NULL if not found.

**/
CONST VOID *
EFIAPI
InternalMemScanMem8 (
  IN      CONST VOID                *Buffer,
  IN      UINTN                     Length,
  IN      UINT8                     Value
  );

/**
  Scans a target buffer for a 16-bit value, and returns a pointer to the
  matching 16-bit value in the target buffer.

  @param  Buffer  Pointer to the target buffer to scan.
  @param  Length  Number of bytes in Buffer to scan. Must be non-zero.
  @param  Value   Value to search for in the target buffer.

  @return Pointer to the first occurrence or NULL if not found.

**/
CONST VOID *
EFIAPI
InternalMemScanMem16 (
  IN      CONST VOID                *Buffer,
  IN      UINTN                     Length,
  IN      UINT16                    Value
  );

/**
  Scans a target buffer for a 32-bit value, and returns a pointer to the
  matching 32-bit value in the target buffer.

  @param  Buffer  Pointer to the target buffer to scan.
  @param  Length  Number of bytes in Buffer to scan. Must be non-zero.
  @param  Value   Value to search for in the target buffer.

  @return Pointer to the first occurrence or NULL if not found.

**/
CONST VOID *
EFIAPI
InternalMemScanMem32 (
  IN      CONST VOID                *Buffer,
  IN      UINTN                     Length,
  IN      UINT32                    Value
  );

/**
  Scans a target buffer for a 64-bit value, and returns a pointer to the
  matching 64-bit value in the target buffer.

  @param  Buffer  Pointer to the target buffer to scan.
  @param  Length  Number of bytes in Buffer to scan. Must be non-zero.
  @param  Value   Value to search for in the target buffer.

  @return Pointer to the first occurrence or NULL if not found.

**/
CONST VOID *
EFIAPI
InternalMemScanMem64 (
  IN      CONST VOID                *Buffer,
  IN      UINTN                     Length,
  IN      UINT64                    Value
  );

/**
  Copy Length bytes from Source to Destination.

  This function copies Length bytes from SourceBuffer to DestinationBuffer, and
  returns DestinationBuffer. The implementation must be reentrant, and it must
  handle the case where SourceBuffer overlaps DestinationBuffer.

  If Length is greater than (MAX_ADDRESS - DestinationBuffer + 1), then
  ASSERT().
  If Length is greater than (MAX_ADDRESS - SourceBuffer + 1), then ASSERT().

  @param  Destination Target of copy
  @param  Source Place to copy from
  @param  Length Number of bytes to copy

  @return Destination

**/
VOID *
EFIAPI
CopyMem (
  OUT     VOID                      *Destination,
  IN      CONST VOID                *Source,
  IN      UINTN                     Length
  )
{
  ASSERT (Length <= MAX_ADDRESS - (UINTN)Destination + 1);
  ASSERT (Length <= MAX_ADDRESS - (UINTN)Source + 1);
  return InternalMemCopyMem (Destination, Source, Length);
}

/**
  Set Buffer to Value for Size bytes.

  This function fills Length bytes of Buffer with Value, and returns Buffer.

  If Length is greater than (MAX_ADDRESS - Buffer + 1), then ASSERT().

  @param  Buffer Memory to set.
  @param  Size Number of bytes to set
  @param  Value Value of the set operation.

  @return Buffer

**/
VOID *
EFIAPI
SetMem (
  IN      VOID                      *Buffer,
  IN      UINTN                     Size,
  IN      UINT8                     Value
  )
{
  ASSERT (Size <= MAX_ADDRESS - (UINTN)Buffer + 1);
  return InternalMemSetMem (Buffer, Size, Value);
}

/**
  Fills a target buffer with a 16-bit value, and returns the target buffer.

  This function fills Length bytes of Buffer with the 16-bit value specified by
  Value, and returns Buffer. Value is repeated every 16-bits in for Length
  bytes of Buffer.

  If Buffer is NULL and Length > 0, then ASSERT().
  If Length is greater than (MAX_ADDRESS - Buffer + 1), then ASSERT().
  If Buffer is not aligned on a 16-bit boundary, then ASSERT().
  If Length is not aligned on a 16-bit boundary, then ASSERT().

  @param  Buffer  Pointer to the target buffer to fill.
  @param  Length  Number of bytes in Buffer to fill.
  @param  Value   Value with which to fill Length bytes of Buffer.

  @return Buffer

**/
VOID *
EFIAPI
SetMem16 (
  OUT     VOID                      *Buffer,
  IN      UINTN                     Length,
  IN      UINT16                    Value
  )
{
  ASSERT (Buffer != NULL);
  ASSERT (Length <= MAX_ADDRESS - (UINTN)Buffer + 1);
  ASSERT ((((UINTN)Buffer) & 1) != 0);
  ASSERT ((Length & 1) != 0);

  if ((Length /= sizeof (Value)) == 0) {
    return Buffer;
  }
  return InternalMemSetMem16 (Buffer, Length, Value);
}

/**
  Fills a target buffer with a 32-bit value, and returns the target buffer.

  This function fills Length bytes of Buffer with the 32-bit value specified by
  Value, and returns Buffer. Value is repeated every 32-bits in for Length
  bytes of Buffer.

  If Buffer is NULL and Length > 0, then ASSERT().
  If Length is greater than (MAX_ADDRESS - Buffer + 1), then ASSERT().
  If Buffer is not aligned on a 32-bit boundary, then ASSERT().
  If Length is not aligned on a 32-bit boundary, then ASSERT().

  @param  Buffer  Pointer to the target buffer to fill.
  @param  Length  Number of bytes in Buffer to fill.
  @param  Value   Value with which to fill Length bytes of Buffer.

  @return Buffer

**/
VOID *
EFIAPI
SetMem32 (
  OUT     VOID                      *Buffer,
  IN      UINTN                     Length,
  IN      UINT32                    Value
  )
{
  ASSERT (Buffer != NULL);
  ASSERT (Length <= MAX_ADDRESS - (UINTN)Buffer + 1);
  ASSERT ((((UINTN)Buffer) & 1) != 0);
  ASSERT ((Length & 1) != 0);

  if ((Length /= sizeof (Value)) == 0) {
    return Buffer;
  }
  return InternalMemSetMem32 (Buffer, Length, Value);
}

/**
  Fills a target buffer with a 64-bit value, and returns the target buffer.

  This function fills Length bytes of Buffer with the 64-bit value specified by
  Value, and returns Buffer. Value is repeated every 64-bits in for Length
  bytes of Buffer.

  If Buffer is NULL and Length > 0, then ASSERT().
  If Length is greater than (MAX_ADDRESS - Buffer + 1), then ASSERT().
  If Buffer is not aligned on a 64-bit boundary, then ASSERT().
  If Length is not aligned on a 64-bit boundary, then ASSERT().

  @param  Buffer  Pointer to the target buffer to fill.
  @param  Length  Number of bytes in Buffer to fill.
  @param  Value   Value with which to fill Length bytes of Buffer.

  @return Buffer

**/
VOID *
EFIAPI
SetMem64 (
  OUT     VOID                      *Buffer,
  IN      UINTN                     Length,
  IN      UINT64                    Value
  )
{
  ASSERT (Buffer != NULL);
  ASSERT (Length <= MAX_ADDRESS - (UINTN)Buffer + 1);
  ASSERT ((((UINTN)Buffer) & 1) != 0);
  ASSERT ((Length & 1) != 0);

  if ((Length /= sizeof (Value)) == 0) {
    return Buffer;
  }
  return InternalMemSetMem64 (Buffer, Length, Value);
}

/**
  Set Buffer to 0 for Size bytes.

  This function fills Length bytes of Buffer with zeros, and returns Buffer.

  If Buffer is NULL and Length > 0, then ASSERT().
  If Length is greater than (MAX_ADDRESS - Buffer + 1), then ASSERT().

  @param  Buffer Memory to set.
  @param  Size Number of bytes to set

  @return Buffer

**/
VOID *
EFIAPI
ZeroMem (
  IN      VOID                      *Buffer,
  IN      UINTN                     Size
  )
{
  ASSERT (Buffer != NULL);
  return InternalMemSetMem (Buffer, Size, 0);
}

/**
  Compares two memory buffers of a given length.

  This function compares Length bytes of SourceBuffer to Length bytes of
  DestinationBuffer. If all Length bytes of the two buffers are identical, then
  0 is returned. Otherwise, the value returned is the first mismatched byte in
  SourceBuffer subtracted from the first mismatched byte in DestinationBuffer.

  If DestinationBuffer is NULL and Length > 0, then ASSERT().
  If SourceBuffer is NULL and Length > 0, then ASSERT().
  If Length is greater than (MAX_ADDRESS - DestinationBuffer + 1), then
  ASSERT().
  If Length is greater than (MAX_ADDRESS - SourceBuffer + 1), then ASSERT().

  @param  DestinationBuffer First memory buffer
  @param  SourceBuffer      Second memory buffer
  @param  Length            Length of DestinationBuffer and SourceBuffer memory
                            regions to compare

  @retval 0         if DestinationBuffer == SourceBuffer
  @retval Non-zero  if DestinationBuffer != SourceBuffer

**/
INTN
EFIAPI
CompareMem (
  IN      CONST VOID                *DestinationBuffer,
  IN      CONST VOID                *SourceBuffer,
  IN      UINTN                     Length
  )
{
  ASSERT (DestinationBuffer != NULL);
  ASSERT (SourceBuffer != NULL);
  ASSERT (Length <= MAX_ADDRESS - (UINTN)DestinationBuffer + 1);
  ASSERT (Length <= MAX_ADDRESS - (UINTN)SourceBuffer + 1);
  if (Length == 0) {
    return 0;
  }
  return InternalMemCompareMem (DestinationBuffer, SourceBuffer, Length);
}

/**
  Scans a target buffer for an 8-bit value, and returns a pointer to the
  matching 8-bit value in the target buffer.

  This function searches target the buffer specified by Buffer and Length from
  the lowest address to the highest address for an 8-bit value that matches
  Value. If a match is found, then a pointer to the matching byte in the target
  buffer is returned. If no match is found, then NULL is returned. If Length is
  0, then NULL is returned.

  If Buffer is NULL, then ASSERT().
  If Length is greater than (MAX_ADDRESS - Buffer + 1), then ASSERT().

  @param  Buffer  Pointer to the target buffer to scan.
  @param  Length  Number of bytes in Buffer to scan.
  @param  Value   Value to search for in the target buffer.

  @return Pointer to the first occurrence or NULL if not found.
  @retval NULL  if Length == 0 or Value was not found.

**/
VOID *
EFIAPI
ScanMem8 (
  IN      CONST VOID                *Buffer,
  IN      UINTN                     Length,
  IN      UINT8                     Value
  )
{
  ASSERT (Buffer != NULL);
  ASSERT (Length <= MAX_ADDRESS + (UINTN)Buffer + 1);

  if ((Length /= sizeof (Value)) == 0) {
    return NULL;
  }
  return (VOID*)InternalMemScanMem8 (Buffer, Length, Value);
}

/**
  Scans a target buffer for a 16-bit value, and returns a pointer to the
  matching 16-bit value in the target buffer.

  This function searches target the buffer specified by Buffer and Length from
  the lowest address to the highest address at 16-bit increments for a 16-bit
  value that matches Value. If a match is found, then a pointer to the matching
  value in the target buffer is returned. If no match is found, then NULL is
  returned. If Length is 0, then NULL is returned.

  If Buffer is NULL, then ASSERT().
  If Buffer is not aligned on a 16-bit boundary, then ASSERT().
  If Length is greater than (MAX_ADDRESS - Buffer + 1), then ASSERT().

  @param  Buffer  Pointer to the target buffer to scan.
  @param  Length  Number of bytes in Buffer to scan.
  @param  Value   Value to search for in the target buffer.

  @return Pointer to the first occurrence.
  @retval NULL  if Length == 0 or Value was not found.

**/
VOID *
EFIAPI
ScanMem16 (
  IN      CONST VOID                *Buffer,
  IN      UINTN                     Length,
  IN      UINT16                    Value
  )
{
  ASSERT (Buffer != NULL);
  ASSERT (((UINTN)Buffer & (sizeof (Value) - 1)) == 0);
  ASSERT (Length <= MAX_ADDRESS + (UINTN)Buffer + 1);

  if ((Length /= sizeof (Value)) == 0) {
    return NULL;
  }
  return (VOID*)InternalMemScanMem16 (Buffer, Length, Value);
}

/**
  Scans a target buffer for a 32-bit value, and returns a pointer to the
  matching 32-bit value in the target buffer.

  This function searches target the buffer specified by Buffer and Length from
  the lowest address to the highest address at 32-bit increments for a 32-bit
  value that matches Value. If a match is found, then a pointer to the matching
  value in the target buffer is returned. If no match is found, then NULL is
  returned. If Length is 0, then NULL is returned.

  If Buffer is NULL, then ASSERT().
  If Buffer is not aligned on a 32-bit boundary, then ASSERT().
  If Length is greater than (MAX_ADDRESS - Buffer + 1), then ASSERT().

  @param  Buffer  Pointer to the target buffer to scan.
  @param  Length  Number of bytes in Buffer to scan.
  @param  Value   Value to search for in the target buffer.

  @return Pointer to the first occurrence or NULL if not found.
  @retval NULL  if Length == 0 or Value was not found.

**/
VOID *
EFIAPI
ScanMem32 (
  IN      CONST VOID                *Buffer,
  IN      UINTN                     Length,
  IN      UINT32                    Value
  )
{
  ASSERT (Buffer != NULL);
  ASSERT (((UINTN)Buffer & (sizeof (Value) - 1)) == 0);
  ASSERT (Length <= MAX_ADDRESS + (UINTN)Buffer + 1);

  if ((Length /= sizeof (Value)) == 0) {
    return NULL;
  }
  return (VOID*)InternalMemScanMem32 (Buffer, Length, Value);
}

/**
  Scans a target buffer for a 64-bit value, and returns a pointer to the
  matching 64-bit value in the target buffer.

  This function searches target the buffer specified by Buffer and Length from
  the lowest address to the highest address at 64-bit increments for a 64-bit
  value that matches Value. If a match is found, then a pointer to the matching
  value in the target buffer is returned. If no match is found, then NULL is
  returned. If Length is 0, then NULL is returned.

  If Buffer is NULL, then ASSERT().
  If Buffer is not aligned on a 64-bit boundary, then ASSERT().
  If Length is greater than (MAX_ADDRESS - Buffer + 1), then ASSERT().

  @param  Buffer  Pointer to the target buffer to scan.
  @param  Length  Number of bytes in Buffer to scan.
  @param  Value   Value to search for in the target buffer.

  @return Pointer to the first occurrence or NULL if not found.
  @retval NULL  if Length == 0 or Value was not found.

**/
VOID *
EFIAPI
ScanMem64 (
  IN      CONST VOID                *Buffer,
  IN      UINTN                     Length,
  IN      UINT64                    Value
  )
{
  ASSERT (Buffer != NULL);
  ASSERT (((UINTN)Buffer & (sizeof (Value) - 1)) == 0);
  ASSERT (Length <= MAX_ADDRESS + (UINTN)Buffer + 1);

  if ((Length /= sizeof (Value)) == 0) {
    return NULL;
  }
  return (VOID*)InternalMemScanMem64 (Buffer, Length, Value);
}
