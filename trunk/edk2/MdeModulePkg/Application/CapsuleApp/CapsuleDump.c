/** @file
  Dump Capsule image information.

  Copyright (c) 2016, Intel Corporation. All rights reserved.<BR>
  This program and the accompanying materials
  are licensed and made available under the terms and conditions of the BSD License
  which accompanies this distribution.  The full text of the license may be found at
  http://opensource.org/licenses/bsd-license.php

  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

#include <PiDxe.h>
#include <Library/BaseLib.h>
#include <Library/DebugLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiRuntimeServicesTableLib.h>
#include <Library/UefiLib.h>
#include <Library/PrintLib.h>
#include <Protocol/FirmwareManagement.h>
#include <Guid/ImageAuthentication.h>
#include <Guid/CapsuleReport.h>
#include <Guid/SystemResourceTable.h>
#include <Guid/FmpCapsule.h>
#include <IndustryStandard/WindowsUxCapsule.h>

/**
  Read a file.

  @param[in] FileName        The file to be read.
  @param[in] BufferSize      The file buffer size
  @param[in] Buffer          The file buffer

  @retval EFI_SUCCESS    Read file successfully
  @retval EFI_NOT_FOUND  File not found
**/
EFI_STATUS
ReadFileToBuffer (
  IN  CHAR16                               *FileName,
  OUT UINTN                                *BufferSize,
  OUT VOID                                 **Buffer
  );

/**
  Dump UX capsule information.

  @param[in] CapsuleHeader      The UX capsule header
**/
VOID
DumpUxCapsule (
  IN EFI_CAPSULE_HEADER  *CapsuleHeader
  )
{
  EFI_DISPLAY_CAPSULE                           *DisplayCapsule;
  DisplayCapsule = (EFI_DISPLAY_CAPSULE *)CapsuleHeader;
  Print(L"[UxCapusule]\n");
  Print(L"CapsuleHeader:\n");
  Print(L"  CapsuleGuid      - %g\n", &DisplayCapsule->CapsuleHeader.CapsuleGuid);
  Print(L"  HeaderSize       - 0x%x\n", DisplayCapsule->CapsuleHeader.HeaderSize);
  Print(L"  Flags            - 0x%x\n", DisplayCapsule->CapsuleHeader.Flags);
  Print(L"  CapsuleImageSize - 0x%x\n", DisplayCapsule->CapsuleHeader.CapsuleImageSize);
  Print(L"ImagePayload:\n");
  Print(L"  Version          - 0x%x\n", DisplayCapsule->ImagePayload.Version);
  Print(L"  Checksum         - 0x%x\n", DisplayCapsule->ImagePayload.Checksum);
  Print(L"  ImageType        - 0x%x\n", DisplayCapsule->ImagePayload.ImageType);
  Print(L"  Mode             - 0x%x\n", DisplayCapsule->ImagePayload.Mode);
  Print(L"  OffsetX          - 0x%x\n", DisplayCapsule->ImagePayload.OffsetX);
  Print(L"  OffsetY          - 0x%x\n", DisplayCapsule->ImagePayload.OffsetY);
}

/**
  Dump FMP image authentication information.

  @param[in] Image      The FMP capsule image
  @param[in] ImageSize  The size of the FMP capsule image in bytes.

  @return the size of FMP authentication.
**/
UINTN
DumpImageAuthentication (
  IN VOID   *Image,
  IN UINTN  ImageSize
  )
{
  EFI_FIRMWARE_IMAGE_AUTHENTICATION             *ImageAuthentication;

  ImageAuthentication = Image;
  if (CompareGuid(&ImageAuthentication->AuthInfo.CertType, &gEfiCertPkcs7Guid) ||
      CompareGuid(&ImageAuthentication->AuthInfo.CertType, &gEfiCertTypeRsa2048Sha256Guid)) {
    Print(L"[ImageAuthentication]\n");
    Print(L"  MonotonicCount   - 0x%lx\n", ImageAuthentication->MonotonicCount);
    Print(L"WIN_CERTIFICATE:\n");
    Print(L"  dwLength         - 0x%x\n", ImageAuthentication->AuthInfo.Hdr.dwLength);
    Print(L"  wRevision        - 0x%x\n", ImageAuthentication->AuthInfo.Hdr.wRevision);
    Print(L"  wCertificateType - 0x%x\n", ImageAuthentication->AuthInfo.Hdr.wCertificateType);
    Print(L"  CertType         - %g\n", &ImageAuthentication->AuthInfo.CertType);
    return sizeof(ImageAuthentication->MonotonicCount) + ImageAuthentication->AuthInfo.Hdr.dwLength;
  } else {
    return 0;
  }
}

/**
  Dump a non-nested FMP capsule.

  @param[in]  CapsuleHeader  A pointer to CapsuleHeader
**/
VOID
DumpFmpCapsule (
  IN EFI_CAPSULE_HEADER  *CapsuleHeader
  )
{
  EFI_FIRMWARE_MANAGEMENT_CAPSULE_HEADER        *FmpCapsuleHeader;
  UINT64                                        *ItemOffsetList;
  UINTN                                         Index;
  UINTN                                         Count;
  EFI_FIRMWARE_MANAGEMENT_CAPSULE_IMAGE_HEADER  *FmpImageHeader;

  Print(L"[FmpCapusule]\n");
  Print(L"CapsuleHeader:\n");
  Print(L"  CapsuleGuid      - %g\n", &CapsuleHeader->CapsuleGuid);
  Print(L"  HeaderSize       - 0x%x\n", CapsuleHeader->HeaderSize);
  Print(L"  Flags            - 0x%x\n", CapsuleHeader->Flags);
  Print(L"  CapsuleImageSize - 0x%x\n", CapsuleHeader->CapsuleImageSize);

  FmpCapsuleHeader = (EFI_FIRMWARE_MANAGEMENT_CAPSULE_HEADER *)((UINT8 *)CapsuleHeader + CapsuleHeader->HeaderSize);
  ItemOffsetList = (UINT64 *)(FmpCapsuleHeader + 1);
  Print(L"FmpHeader:\n");
  Print(L"  Version             - 0x%x\n", FmpCapsuleHeader->Version);
  Print(L"  EmbeddedDriverCount - 0x%x\n", FmpCapsuleHeader->EmbeddedDriverCount);
  Print(L"  PayloadItemCount    - 0x%x\n", FmpCapsuleHeader->PayloadItemCount);
  Count = FmpCapsuleHeader->EmbeddedDriverCount + FmpCapsuleHeader->PayloadItemCount;
  for (Index = 0; Index < Count; Index++) {
    Print(L"  Offset[%d]           - 0x%x\n", Index, ItemOffsetList[Index]);
  }

  for (Index = FmpCapsuleHeader->EmbeddedDriverCount; Index < Count; Index++) {
    Print(L"FmpPayload[%d] ImageHeader:\n", Index);
    FmpImageHeader = (EFI_FIRMWARE_MANAGEMENT_CAPSULE_IMAGE_HEADER *)((UINT8 *)FmpCapsuleHeader + ItemOffsetList[Index]);
    Print(L"  Version                - 0x%x\n", FmpImageHeader->Version);
    Print(L"  UpdateImageTypeId      - %g\n", &FmpImageHeader->UpdateImageTypeId);
    Print(L"  UpdateImageIndex       - 0x%x\n", FmpImageHeader->UpdateImageIndex);
    Print(L"  UpdateImageSize        - 0x%x\n", FmpImageHeader->UpdateImageSize);
    Print(L"  UpdateVendorCodeSize   - 0x%x\n", FmpImageHeader->UpdateVendorCodeSize);
    if (FmpImageHeader->Version >= EFI_FIRMWARE_MANAGEMENT_CAPSULE_IMAGE_HEADER_INIT_VERSION) {
      Print(L"  UpdateHardwareInstance - 0x%lx\n", FmpImageHeader->UpdateHardwareInstance);
    }
  }
}

/**
  Return if there is a FMP header below capsule header.

  @param[in] CapsuleHeader A pointer to EFI_CAPSULE_HEADER

  @retval TRUE  There is a FMP header below capsule header.
  @retval FALSE There is not a FMP header below capsule header
**/
BOOLEAN
IsNestedFmpCapsule (
  IN EFI_CAPSULE_HEADER         *CapsuleHeader
  )
{
  EFI_STATUS                 Status;
  EFI_SYSTEM_RESOURCE_TABLE  *Esrt;
  EFI_SYSTEM_RESOURCE_ENTRY  *EsrtEntry;
  UINTN                      Index;
  BOOLEAN                    EsrtGuidFound;
  EFI_CAPSULE_HEADER         *NestedCapsuleHeader;
  UINTN                      NestedCapsuleSize;

  //
  // Check ESRT
  //
  EsrtGuidFound = FALSE;
  Status = EfiGetSystemConfigurationTable(&gEfiSystemResourceTableGuid, (VOID **)&Esrt);
  if (!EFI_ERROR(Status)) {
    ASSERT (Esrt != NULL);
    EsrtEntry = (VOID *)(Esrt + 1);
    for (Index = 0; Index < Esrt->FwResourceCount; Index++, EsrtEntry++) {
      if (CompareGuid(&EsrtEntry->FwClass, &CapsuleHeader->CapsuleGuid)) {
        EsrtGuidFound = TRUE;
        break;
      }
    }
  }

  if (!EsrtGuidFound) {
    return FALSE;
  }

  //
  // Check nested capsule header
  // FMP GUID after ESRT one
  //
  NestedCapsuleHeader = (EFI_CAPSULE_HEADER *)((UINT8 *)CapsuleHeader + CapsuleHeader->HeaderSize);
  NestedCapsuleSize = (UINTN)CapsuleHeader + CapsuleHeader->HeaderSize - (UINTN)NestedCapsuleHeader;
  if (NestedCapsuleSize < sizeof(EFI_CAPSULE_HEADER)) {
    return FALSE;
  }
  if (!CompareGuid(&NestedCapsuleHeader->CapsuleGuid, &gEfiFmpCapsuleGuid)) {
    return FALSE;
  }
  return TRUE;
}

/**
  Dump capsule information

  @param[in] CapsuleName  The name of the capsule image.

  @retval EFI_SUCCESS            The capsule information is dumped.
  @retval EFI_UNSUPPORTED        Input parameter is not valid.
**/
EFI_STATUS
DumpCapsule (
  IN CHAR16                                        *CapsuleName
  )
{
  VOID                                          *Buffer;
  UINTN                                         FileSize;
  EFI_CAPSULE_HEADER                            *CapsuleHeader;
  EFI_STATUS                                    Status;

  Status = ReadFileToBuffer(CapsuleName, &FileSize, &Buffer);
  if (EFI_ERROR(Status)) {
    Print(L"CapsuleApp: Capsule (%s) is not found.\n", CapsuleName);
    goto Done;
  }

  CapsuleHeader = Buffer;
  if (CompareGuid(&CapsuleHeader->CapsuleGuid, &gWindowsUxCapsuleGuid)) {
    DumpUxCapsule(CapsuleHeader);
    Status = EFI_SUCCESS;
    goto Done;
  }

  if (CompareGuid(&CapsuleHeader->CapsuleGuid, &gEfiFmpCapsuleGuid)) {
    DumpFmpCapsule(CapsuleHeader);
  }
  if (IsNestedFmpCapsule(CapsuleHeader)) {
    Print(L"[NestedCapusule]\n");
    Print(L"CapsuleHeader:\n");
    Print(L"  CapsuleGuid      - %g\n", &CapsuleHeader->CapsuleGuid);
    Print(L"  HeaderSize       - 0x%x\n", CapsuleHeader->HeaderSize);
    Print(L"  Flags            - 0x%x\n", CapsuleHeader->Flags);
    Print(L"  CapsuleImageSize - 0x%x\n", CapsuleHeader->CapsuleImageSize);
    DumpFmpCapsule((EFI_CAPSULE_HEADER *)((UINTN)CapsuleHeader + CapsuleHeader->HeaderSize));
  }

Done:
  FreePool(Buffer);
  return Status;
}

/**
  Dump capsule status variable.

  @retval EFI_SUCCESS            The capsule status variable is dumped.
  @retval EFI_UNSUPPORTED        Input parameter is not valid.
**/
EFI_STATUS
DmpCapsuleStatusVariable (
  VOID
  )
{
  EFI_STATUS                          Status;
  UINT32                              Index;
  CHAR16                              CapsuleVarName[20];
  CHAR16                              *TempVarName;
  EFI_CAPSULE_RESULT_VARIABLE_HEADER  *CapsuleResult;
  EFI_CAPSULE_RESULT_VARIABLE_FMP     *CapsuleResultFmp;
  UINTN                               CapsuleFileNameSize;
  CHAR16                              CapsuleIndexData[12];
  CHAR16                              *CapsuleIndex;

  Status = GetVariable2(
             L"CapsuleMax",
             &gEfiCapsuleReportGuid,
             (VOID **)&CapsuleIndex,
             NULL
             );
  if (!EFI_ERROR(Status)) {
    CopyMem(CapsuleIndexData, CapsuleIndex, 11 * sizeof(CHAR16));
    CapsuleIndexData[11] = 0;
    Print(L"CapsuleMax - %s\n", CapsuleIndexData);
    FreePool(CapsuleIndex);
  }
  Status = GetVariable2(
             L"CapsuleLast",
             &gEfiCapsuleReportGuid,
             (VOID **)&CapsuleIndex,
             NULL
             );
  if (!EFI_ERROR(Status)) {
    CopyMem(CapsuleIndexData, CapsuleIndex, 11 * sizeof(CHAR16));
    CapsuleIndexData[11] = 0;
    Print(L"CapsuleLast - %s\n", CapsuleIndexData);
    FreePool(CapsuleIndex);
  }


  StrCpyS (CapsuleVarName, sizeof(CapsuleVarName)/sizeof(CapsuleVarName[0]), L"Capsule");
  TempVarName = CapsuleVarName + StrLen (CapsuleVarName);
  Index = 0;

  while (TRUE) {
    UnicodeSPrint (TempVarName, 5 * sizeof(CHAR16), L"%04x", Index);

    Status = GetVariable2 (
               CapsuleVarName,
               &gEfiCapsuleReportGuid,
               (VOID **) &CapsuleResult,
               NULL
               );
    if (Status == EFI_NOT_FOUND) {
      break;
    } else if (EFI_ERROR(Status)) {
      continue;
    }
    ASSERT (CapsuleResult != NULL);

    //
    // display capsule process status
    //
    if (CapsuleResult->VariableTotalSize >= sizeof(EFI_CAPSULE_RESULT_VARIABLE_HEADER)) {
      Print (L"CapsuleName: %s\n", CapsuleVarName);
      Print (L"  Capsule Guid: %g\n", &CapsuleResult->CapsuleGuid);
      Print (L"  Capsule ProcessedTime: %t\n", &CapsuleResult->CapsuleProcessed);
      Print (L"  Capsule Status: %r\n", CapsuleResult->CapsuleStatus);
    }

    if (CompareGuid(&CapsuleResult->CapsuleGuid, &gEfiFmpCapsuleGuid)) {
      if (CapsuleResult->VariableTotalSize >= sizeof(EFI_CAPSULE_RESULT_VARIABLE_HEADER) + sizeof(EFI_CAPSULE_RESULT_VARIABLE_FMP)) {
        CapsuleResultFmp = (EFI_CAPSULE_RESULT_VARIABLE_FMP *)(CapsuleResult + 1);
        Print(L"  Capsule FMP Version: 0x%x\n", CapsuleResultFmp->Version);
        Print(L"  Capsule FMP PayloadIndex: 0x%x\n", CapsuleResultFmp->PayloadIndex);
        Print(L"  Capsule FMP UpdateImageIndex: 0x%x\n", CapsuleResultFmp->UpdateImageIndex);
        Print(L"  Capsule FMP UpdateImageTypeId: %g\n", &CapsuleResultFmp->UpdateImageTypeId);
        if (CapsuleResult->VariableTotalSize > sizeof(EFI_CAPSULE_RESULT_VARIABLE_HEADER) + sizeof(EFI_CAPSULE_RESULT_VARIABLE_FMP)) {
          Print(L"  Capsule FMP CapsuleFileName: %s\n", (CapsuleResultFmp + 1));
          CapsuleFileNameSize = StrSize((CHAR16 *)(CapsuleResultFmp + 1));
          if (CapsuleResult->VariableTotalSize > sizeof(EFI_CAPSULE_RESULT_VARIABLE_HEADER) + sizeof(EFI_CAPSULE_RESULT_VARIABLE_FMP) + CapsuleFileNameSize) {
            Print(L"  Capsule FMP CapsuleTarget: %s\n", (UINT8 *)(CapsuleResultFmp + 1) + CapsuleFileNameSize);
          }
        }
      }
    }

    FreePool(CapsuleResult);

    Index++;
    if (Index > 0xFFFF) {
      break;
    }
  }

  return EFI_SUCCESS;
}

CHAR8 *mFwTypeString[] = {
  "Unknown",
  "SystemFirmware",
  "DeviceFirmware",
  "UefiDriver",
};

CHAR8 *mLastAttemptStatusString[] = {
  "Success",
  "Error: Unsuccessful",
  "Error: Insufficient Resources",
  "Error: Incorrect Version",
  "Error: Invalid Format",
  "Error: Auth Error",
  "Error: Power Event AC",
  "Error: Power Event Battery",
};

/**
  Convert FwType to a string.

  @param[in] FwType  FwType in ESRT

  @return a string for FwType.
**/
CHAR8 *
FwTypeToString (
  IN UINT32  FwType
  )
{
  if (FwType < sizeof(mFwTypeString) / sizeof(mFwTypeString[0])) {
    return mFwTypeString[FwType];
  } else {
    return "Invalid";
  }
}

/**
  Convert LastAttemptStatus to a string.

  @param[in] LastAttemptStatus  LastAttemptStatus in FMP or ESRT

  @return a string for LastAttemptStatus.
**/
CHAR8 *
LastAttemptStatusToString (
  IN UINT32  LastAttemptStatus
  )
{
  if (LastAttemptStatus < sizeof(mLastAttemptStatusString) / sizeof(mLastAttemptStatusString[0])) {
    return mLastAttemptStatusString[LastAttemptStatus];
  } else {
    return "Error: Unknown";
  }
}

/**
  Dump ESRT entry.

  @param[in] EsrtEntry  ESRT entry
**/
VOID
DumpEsrtEntry (
  IN EFI_SYSTEM_RESOURCE_ENTRY  *EsrtEntry
  )
{
  Print(L"  FwClass                  - %g\n", &EsrtEntry->FwClass);
  Print(L"  FwType                   - 0x%x (%a)\n", EsrtEntry->FwType, FwTypeToString(EsrtEntry->FwType));
  Print(L"  FwVersion                - 0x%x\n", EsrtEntry->FwVersion);
  Print(L"  LowestSupportedFwVersion - 0x%x\n", EsrtEntry->LowestSupportedFwVersion);
  Print(L"  CapsuleFlags             - 0x%x\n", EsrtEntry->CapsuleFlags);
  Print(L"    PERSIST_ACROSS_RESET   - 0x%x\n", EsrtEntry->CapsuleFlags & CAPSULE_FLAGS_PERSIST_ACROSS_RESET);
  Print(L"    POPULATE_SYSTEM_TABLE  - 0x%x\n", EsrtEntry->CapsuleFlags & CAPSULE_FLAGS_POPULATE_SYSTEM_TABLE);
  Print(L"    INITIATE_RESET         - 0x%x\n", EsrtEntry->CapsuleFlags & CAPSULE_FLAGS_INITIATE_RESET);
  Print(L"  LastAttemptVersion       - 0x%x\n", EsrtEntry->LastAttemptVersion);
  Print(L"  LastAttemptStatus        - 0x%x (%a)\n", EsrtEntry->LastAttemptStatus, LastAttemptStatusToString(EsrtEntry->LastAttemptStatus));
}

/**
  Dump ESRT table.

  @param[in] Esrt  ESRT table
**/
VOID
DumpEsrt (
  IN EFI_SYSTEM_RESOURCE_TABLE  *Esrt
  )
{
  UINTN                      Index;
  EFI_SYSTEM_RESOURCE_ENTRY  *EsrtEntry;

  if (Esrt == NULL) {
    return ;
  }

  Print(L"EFI_SYSTEM_RESOURCE_TABLE:\n");
  Print(L"FwResourceCount    - 0x%x\n", Esrt->FwResourceCount);
  Print(L"FwResourceCountMax - 0x%x\n", Esrt->FwResourceCountMax);
  Print(L"FwResourceVersion  - 0x%lx\n", Esrt->FwResourceVersion);

  EsrtEntry = (VOID *)(Esrt + 1);
  for (Index = 0; Index < Esrt->FwResourceCount; Index++) {
    Print(L"EFI_SYSTEM_RESOURCE_ENTRY (%d):\n", Index);
    DumpEsrtEntry(EsrtEntry);
    EsrtEntry++;
  }
}

/**
  Dump ESRT info.
**/
VOID
DumpEsrtData (
  VOID
  )
{
  EFI_STATUS                 Status;
  EFI_SYSTEM_RESOURCE_TABLE  *Esrt;

  Print(L"##############\n");
  Print(L"# ESRT TABLE #\n");
  Print(L"##############\n");

  Status = EfiGetSystemConfigurationTable (&gEfiSystemResourceTableGuid, (VOID **)&Esrt);
  if (EFI_ERROR(Status)) {
    Print(L"ESRT - %r\n", Status);
    return;
  }
  DumpEsrt(Esrt);
  Print(L"\n");
}

/**
  Dump FMP information.

  @param[in] ImageInfoSize       The size of ImageInfo, in bytes.
  @param[in] ImageInfo           A pointer to EFI_FIRMWARE_IMAGE_DESCRIPTOR.
  @param[in] DescriptorVersion   The version of EFI_FIRMWARE_IMAGE_DESCRIPTOR.
  @param[in] DescriptorCount     The count of EFI_FIRMWARE_IMAGE_DESCRIPTOR.
  @param[in] DescriptorSize      The size of an individual EFI_FIRMWARE_IMAGE_DESCRIPTOR, in bytes.
  @param[in] PackageVersion      The version of package.
  @param[in] PackageVersionName  The version name of package.
**/
VOID
DumpFmpImageInfo (
  IN UINTN                           ImageInfoSize,
  IN EFI_FIRMWARE_IMAGE_DESCRIPTOR   *ImageInfo,
  IN UINT32                          DescriptorVersion,
  IN UINT8                           DescriptorCount,
  IN UINTN                           DescriptorSize,
  IN UINT32                          PackageVersion,
  IN CHAR16                          *PackageVersionName
  )
{
  EFI_FIRMWARE_IMAGE_DESCRIPTOR                 *CurrentImageInfo;
  UINTN                                         Index;

  Print(L"  DescriptorVersion  - 0x%x\n", DescriptorVersion);
  Print(L"  DescriptorCount    - 0x%x\n", DescriptorCount);
  Print(L"  DescriptorSize     - 0x%x\n", DescriptorSize);
  Print(L"  PackageVersion     - 0x%x\n", PackageVersion);
  Print(L"  PackageVersionName - \"%s\"\n", PackageVersionName);
  CurrentImageInfo = ImageInfo;
  for (Index = 0; Index < DescriptorCount; Index++) {
    Print(L"  ImageDescriptor (%d)\n", Index);
    Print(L"    ImageIndex                  - 0x%x\n", CurrentImageInfo->ImageIndex);
    Print(L"    ImageTypeId                 - %g\n", &CurrentImageInfo->ImageTypeId);
    Print(L"    ImageId                     - 0x%lx\n", CurrentImageInfo->ImageId);
    Print(L"    ImageIdName                 - \"%s\"\n", CurrentImageInfo->ImageIdName);
    Print(L"    Version                     - 0x%x\n", CurrentImageInfo->Version);
    Print(L"    VersionName                 - \"%s\"\n", CurrentImageInfo->VersionName);
    Print(L"    Size                        - 0x%x\n", CurrentImageInfo->Size);
    Print(L"    AttributesSupported         - 0x%lx\n", CurrentImageInfo->AttributesSupported);
    Print(L"      IMAGE_UPDATABLE           - 0x%lx\n", CurrentImageInfo->AttributesSupported & IMAGE_ATTRIBUTE_IMAGE_UPDATABLE);
    Print(L"      RESET_REQUIRED            - 0x%lx\n", CurrentImageInfo->AttributesSupported & IMAGE_ATTRIBUTE_RESET_REQUIRED);
    Print(L"      AUTHENTICATION_REQUIRED   - 0x%lx\n", CurrentImageInfo->AttributesSupported & IMAGE_ATTRIBUTE_AUTHENTICATION_REQUIRED);
    Print(L"      IN_USE                    - 0x%lx\n", CurrentImageInfo->AttributesSupported & IMAGE_ATTRIBUTE_IN_USE);
    Print(L"      UEFI_IMAGE                - 0x%lx\n", CurrentImageInfo->AttributesSupported & IMAGE_ATTRIBUTE_UEFI_IMAGE);
    Print(L"    AttributesSetting           - 0x%lx\n", CurrentImageInfo->AttributesSetting);
    Print(L"      IMAGE_UPDATABLE           - 0x%lx\n", CurrentImageInfo->AttributesSetting & IMAGE_ATTRIBUTE_IMAGE_UPDATABLE);
    Print(L"      RESET_REQUIRED            - 0x%lx\n", CurrentImageInfo->AttributesSetting & IMAGE_ATTRIBUTE_RESET_REQUIRED);
    Print(L"      AUTHENTICATION_REQUIRED   - 0x%lx\n", CurrentImageInfo->AttributesSetting & IMAGE_ATTRIBUTE_AUTHENTICATION_REQUIRED);
    Print(L"      IN_USE                    - 0x%lx\n", CurrentImageInfo->AttributesSetting & IMAGE_ATTRIBUTE_IN_USE);
    Print(L"      UEFI_IMAGE                - 0x%lx\n", CurrentImageInfo->AttributesSetting & IMAGE_ATTRIBUTE_UEFI_IMAGE);
    Print(L"    Compatibilities             - 0x%lx\n", CurrentImageInfo->Compatibilities);
    Print(L"      COMPATIB_CHECK_SUPPORTED  - 0x%lx\n", CurrentImageInfo->Compatibilities & IMAGE_COMPATIBILITY_CHECK_SUPPORTED);
    if (DescriptorVersion > 1) {
      Print(L"    LowestSupportedImageVersion - 0x%x\n", CurrentImageInfo->LowestSupportedImageVersion);
      if (DescriptorVersion > 2) {
        Print(L"    LastAttemptVersion          - 0x%x\n", CurrentImageInfo->LastAttemptVersion);
        Print(L"    LastAttemptStatus           - 0x%x (%a)\n", CurrentImageInfo->LastAttemptStatus, LastAttemptStatusToString(CurrentImageInfo->LastAttemptStatus));
        Print(L"    HardwareInstance            - 0x%lx\n", CurrentImageInfo->HardwareInstance);
      }
    }
    //
    // Use DescriptorSize to move ImageInfo Pointer to stay compatible with different ImageInfo version
    //
    CurrentImageInfo = (EFI_FIRMWARE_IMAGE_DESCRIPTOR *)((UINT8 *)CurrentImageInfo + DescriptorSize);
  }
}

/**
  Dump FMP package information.

  @param[in] PackageVersion             The version of package.
  @param[in] PackageVersionName         The version name of package.
  @param[in] PackageVersionNameMaxLen   The maximum length of PackageVersionName.
  @param[in] AttributesSupported        Package attributes that are supported by this device.
  @param[in] AttributesSetting          Package attributes.
**/
VOID
DumpFmpPackageInfo (
  IN UINT32                           PackageVersion,
  IN CHAR16                           *PackageVersionName,
  IN UINT32                           PackageVersionNameMaxLen,
  IN UINT64                           AttributesSupported,
  IN UINT64                           AttributesSetting
  )
{
  Print(L"  PackageVersion              - 0x%x\n", PackageVersion);
  Print(L"  PackageVersionName          - \"%s\"\n", PackageVersionName);
  Print(L"  PackageVersionNameMaxLen    - 0x%x\n", PackageVersionNameMaxLen);
  Print(L"  AttributesSupported         - 0x%lx\n", AttributesSupported);
  Print(L"    IMAGE_UPDATABLE           - 0x%lx\n", AttributesSupported & IMAGE_ATTRIBUTE_IMAGE_UPDATABLE);
  Print(L"    RESET_REQUIRED            - 0x%lx\n", AttributesSupported & IMAGE_ATTRIBUTE_RESET_REQUIRED);
  Print(L"    AUTHENTICATION_REQUIRED   - 0x%lx\n", AttributesSupported & IMAGE_ATTRIBUTE_AUTHENTICATION_REQUIRED);
  Print(L"  AttributesSetting           - 0x%lx\n", AttributesSetting);
  Print(L"    IMAGE_UPDATABLE           - 0x%lx\n", AttributesSetting & IMAGE_ATTRIBUTE_IMAGE_UPDATABLE);
  Print(L"    RESET_REQUIRED            - 0x%lx\n", AttributesSetting & IMAGE_ATTRIBUTE_RESET_REQUIRED);
  Print(L"    AUTHENTICATION_REQUIRED   - 0x%lx\n", AttributesSetting & IMAGE_ATTRIBUTE_AUTHENTICATION_REQUIRED);
}

/**
  Dump FMP protocol info.
**/
VOID
DumpFmpData (
  VOID
  )
{
  EFI_STATUS                                    Status;
  EFI_FIRMWARE_MANAGEMENT_PROTOCOL              *Fmp;
  EFI_HANDLE                                    *HandleBuffer;
  UINTN                                         NumberOfHandles;
  UINTN                                         Index;
  EFI_FIRMWARE_IMAGE_DESCRIPTOR                 *FmpImageInfoBuf;
  UINTN                                         ImageInfoSize;
  UINT32                                        FmpImageInfoDescriptorVer;
  UINT8                                         FmpImageInfoCount;
  UINTN                                         DescriptorSize;
  UINT32                                        PackageVersion;
  CHAR16                                        *PackageVersionName;
  UINT32                                        PackageVersionNameMaxLen;
  UINT64                                        AttributesSupported;
  UINT64                                        AttributesSetting;

  Print(L"############\n");
  Print(L"# FMP DATA #\n");
  Print(L"############\n");
  Status = gBS->LocateHandleBuffer (
                  ByProtocol,
                  &gEfiFirmwareManagementProtocolGuid,
                  NULL,
                  &NumberOfHandles,
                  &HandleBuffer
                  );
  if (EFI_ERROR(Status)) {
    Print(L"FMP protocol - %r\n", EFI_NOT_FOUND);
    return;
  }

  for (Index = 0; Index < NumberOfHandles; Index++) {
    Status = gBS->HandleProtocol(
                    HandleBuffer[Index],
                    &gEfiFirmwareManagementProtocolGuid,
                    (VOID **)&Fmp
                    );
    if (EFI_ERROR(Status)) {
      continue;
    }

    ImageInfoSize = 0;
    Status = Fmp->GetImageInfo (
                    Fmp,
                    &ImageInfoSize,
                    NULL,
                    NULL,
                    NULL,
                    NULL,
                    NULL,
                    NULL
                    );
    if (Status != EFI_BUFFER_TOO_SMALL) {
      continue;
    }

    FmpImageInfoBuf = NULL;
    FmpImageInfoBuf = AllocateZeroPool (ImageInfoSize);
    if (FmpImageInfoBuf == NULL) {
      Status = EFI_OUT_OF_RESOURCES;
      goto EXIT;
    }

    PackageVersionName = NULL;
    Status = Fmp->GetImageInfo (
                    Fmp,
                    &ImageInfoSize,               // ImageInfoSize
                    FmpImageInfoBuf,              // ImageInfo
                    &FmpImageInfoDescriptorVer,   // DescriptorVersion
                    &FmpImageInfoCount,           // DescriptorCount
                    &DescriptorSize,              // DescriptorSize
                    &PackageVersion,              // PackageVersion
                    &PackageVersionName           // PackageVersionName
                    );

    //
    // If FMP GetInformation interface failed, skip this resource
    //
    if (EFI_ERROR(Status)) {
      Print(L"FMP (%d) ImageInfo - %r\n", Index, Status);
      FreePool(FmpImageInfoBuf);
      continue;
    }

    Print(L"FMP (%d) ImageInfo:\n", Index);
    DumpFmpImageInfo(
      ImageInfoSize,               // ImageInfoSize
      FmpImageInfoBuf,             // ImageInfo
      FmpImageInfoDescriptorVer,   // DescriptorVersion
      FmpImageInfoCount,           // DescriptorCount
      DescriptorSize,              // DescriptorSize
      PackageVersion,              // PackageVersion
      PackageVersionName           // PackageVersionName
      );

    if (PackageVersionName != NULL) {
      FreePool(PackageVersionName);
    }
    FreePool(FmpImageInfoBuf);

    //
    // Get package info
    //
    PackageVersionName = NULL;
    Status = Fmp->GetPackageInfo (
                    Fmp,
                    &PackageVersion,              // PackageVersion
                    &PackageVersionName,          // PackageVersionName
                    &PackageVersionNameMaxLen,    // PackageVersionNameMaxLen
                    &AttributesSupported,         // AttributesSupported
                    &AttributesSetting            // AttributesSetting
                    );
    if (EFI_ERROR(Status)) {
      Print(L"FMP (%d) PackageInfo - %r\n", Index, Status);
    } else {
      Print(L"FMP (%d) ImageInfo:\n", Index);
      DumpFmpPackageInfo(
        PackageVersion,              // PackageVersion
        PackageVersionName,          // PackageVersionName
        PackageVersionNameMaxLen,    // PackageVersionNameMaxLen
        AttributesSupported,         // AttributesSupported
        AttributesSetting            // AttributesSetting
        );

      if (PackageVersionName != NULL) {
        FreePool(PackageVersionName);
      }
    }
  }
  Print(L"\n");

EXIT:
  FreePool(HandleBuffer);
}
