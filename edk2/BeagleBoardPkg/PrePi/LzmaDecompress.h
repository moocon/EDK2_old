/** @file
  LZMA Decompress Library header file

  Copyright (c) 2006 - 2010, Intel Corporation. All rights reserved.<BR>
  SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#ifndef __LZMA_DECOMPRESS_H___
#define __LZMA_DECOMPRESS_H___

/**
  Examines a GUIDed section and returns the size of the decoded buffer and the
  size of an scratch buffer required to actually decode the data in a GUIDed section.

  Examines a GUIDed section specified by InputSection.
  If GUID for InputSection does not match the GUID that this handler supports,
  then RETURN_UNSUPPORTED is returned.
  If the required information can not be retrieved from InputSection,
  then RETURN_INVALID_PARAMETER is returned.
  If the GUID of InputSection does match the GUID that this handler supports,
  then the size required to hold the decoded buffer is returned in OututBufferSize,
  the size of an optional scratch buffer is returned in ScratchSize, and the Attributes field
  from EFI_GUID_DEFINED_SECTION header of InputSection is returned in SectionAttribute.

  If InputSection is NULL, then ASSERT().
  If OutputBufferSize is NULL, then ASSERT().
  If ScratchBufferSize is NULL, then ASSERT().
  If SectionAttribute is NULL, then ASSERT().


  @param[in]  InputSection       A pointer to a GUIDed section of an FFS formatted file.
  @param[out] OutputBufferSize   A pointer to the size, in bytes, of an output buffer required
                                 if the buffer specified by InputSection were decoded.
  @param[out] ScratchBufferSize  A pointer to the size, in bytes, required as scratch space
                                 if the buffer specified by InputSection were decoded.
  @param[out] SectionAttribute   A pointer to the attributes of the GUIDed section. See the Attributes
                                 field of EFI_GUID_DEFINED_SECTION in the PI Specification.

  @retval  RETURN_SUCCESS            The information about InputSection was returned.
  @retval  RETURN_UNSUPPORTED        The section specified by InputSection does not match the GUID this handler supports.
  @retval  RETURN_INVALID_PARAMETER  The information can not be retrieved from the section specified by InputSection.

**/
RETURN_STATUS
EFIAPI
LzmaGuidedSectionGetInfo (
  IN  CONST VOID  *InputSection,
  OUT UINT32      *OutputBufferSize,
  OUT UINT32      *ScratchBufferSize,
  OUT UINT16      *SectionAttribute
  );

/**
  Decompress a LZAM compressed GUIDed section into a caller allocated output buffer.

  Decodes the GUIDed section specified by InputSection.
  If GUID for InputSection does not match the GUID that this handler supports, then RETURN_UNSUPPORTED is returned.
  If the data in InputSection can not be decoded, then RETURN_INVALID_PARAMETER is returned.
  If the GUID of InputSection does match the GUID that this handler supports, then InputSection
  is decoded into the buffer specified by OutputBuffer and the authentication status of this
  decode operation is returned in AuthenticationStatus.  If the decoded buffer is identical to the
  data in InputSection, then OutputBuffer is set to point at the data in InputSection.  Otherwise,
  the decoded data will be placed in caller allocated buffer specified by OutputBuffer.

  If InputSection is NULL, then ASSERT().
  If OutputBuffer is NULL, then ASSERT().
  If ScratchBuffer is NULL and this decode operation requires a scratch buffer, then ASSERT().
  If AuthenticationStatus is NULL, then ASSERT().


  @param[in]  InputSection  A pointer to a GUIDed section of an FFS formatted file.
  @param[out] OutputBuffer  A pointer to a buffer that contains the result of a decode operation.
  @param[out] ScratchBuffer A caller allocated buffer that may be required by this function
                            as a scratch buffer to perform the decode operation.
  @param[out] AuthenticationStatus
                            A pointer to the authentication status of the decoded output buffer.
                            See the definition of authentication status in the EFI_PEI_GUIDED_SECTION_EXTRACTION_PPI
                            section of the PI Specification. EFI_AUTH_STATUS_PLATFORM_OVERRIDE must
                            never be set by this handler.

  @retval  RETURN_SUCCESS            The buffer specified by InputSection was decoded.
  @retval  RETURN_UNSUPPORTED        The section specified by InputSection does not match the GUID this handler supports.
  @retval  RETURN_INVALID_PARAMETER  The section specified by InputSection can not be decoded.

**/
RETURN_STATUS
EFIAPI
LzmaGuidedSectionExtraction (
  IN CONST  VOID    *InputSection,
  OUT       VOID    **OutputBuffer,
  OUT       VOID    *ScratchBuffer,        OPTIONAL
  OUT       UINT32  *AuthenticationStatus
  );

#endif // __LZMADECOMPRESS_H__

