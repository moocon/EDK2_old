/*++

Copyright (c) 2006, Intel Corporation
All rights reserved. This program and the accompanying materials
are licensed and made available under the terms and conditions of the BSD License
which accompanies this distribution.  The full text of the license may be found at
http://opensource.org/licenses/bsd-license.php

THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

Module Name:

  Runtime.c

Abstract:

  Runtime Architectural Protocol as defined in the DXE CIS

  This code is used to produce the EFI runtime virtual switch over

  THIS IS VERY DANGEROUS CODE BE VERY CAREFUL IF YOU CHANGE IT

  The transition for calling EFI Runtime functions in physical mode to calling
  them in virtual mode is very very complex. Every pointer in needs to be
  converted from physical mode to virtual mode. Be very careful walking linked
  lists! Then to make it really hard the code it's self needs be relocated into
  the new virtual address space.

  So here is the concept. The code in this module will never ever be called in
  virtual mode. This is the code that collects the information needed to convert
  to virtual mode (DXE core registers runtime stuff with this code). Since this
  code is used to fixup all runtime images, it CAN NOT fix it's self up. So some
  code has to stay behind and that is us.

  Also you need to be careful about when you allocate memory, as once we are in
  runtime (including our EVT_SIGNAL_EXIT_BOOT_SERVICES event) you can no longer
  allocate memory.

  Any runtime driver that gets loaded before us will not be callable in virtual
  mode. This is due to the fact that the DXE core can not register the info
  needed with us. This is good, since it keeps the code in this file from
  getting registered.


Revision History:

  - Move the CalculateCrc32 function from Runtime Arch Protocol to Boot Service.
  Runtime Arch Protocol definition no longer contains CalculateCrc32. Boot Service
  Table now contains an item named CalculateCrc32.

--*/

#include "Runtime.h"

//
// Global Variables
//
EFI_MEMORY_DESCRIPTOR         *mVirtualMap                = NULL;
UINTN                         mVirtualMapDescriptorSize;
UINTN                         mVirtualMapMaxIndex;
VOID                          *mMyImageBase;

//
// The handle onto which the Runtime Architectural Protocol instance is installed
//
EFI_HANDLE                    mRuntimeHandle = NULL;

//
// The Runtime Architectural Protocol instance produced by this driver
//
EFI_RUNTIME_ARCH_PROTOCOL     mRuntime = {
  INITIALIZE_LIST_HEAD_VARIABLE (mRuntime.ImageHead),
  INITIALIZE_LIST_HEAD_VARIABLE (mRuntime.EventHead),

  //
  // Make sure Size != sizeof (EFI_MEMORY_DESCRIPTOR). This will
  // prevent people from having pointer math bugs in their code.
  // now you have to use *DescriptorSize to make things work.
  //
  sizeof (EFI_MEMORY_DESCRIPTOR) + sizeof (UINT64) - (sizeof (EFI_MEMORY_DESCRIPTOR) % sizeof (UINT64)),
  EFI_MEMORY_DESCRIPTOR_VERSION,
  0,
  NULL,
  NULL,
  FALSE,
  FALSE
};

//
// Worker Functions
//
STATIC
VOID
RuntimeDriverCalculateEfiHdrCrc (
  IN OUT EFI_TABLE_HEADER  *Hdr
  )
/*++

Routine Description:

  Calcualte the 32-bit CRC in a EFI table using the Runtime Drivers
  internal function.  The EFI Boot Services Table can not be used because
  the EFI Boot Services Table was destroyed at ExitBootServices()

Arguments:

  Hdr  - Pointer to an EFI standard header

Returns:

  None

--*/
{
  UINT32  Crc;

  Hdr->CRC32  = 0;

  Crc         = 0;
  RuntimeDriverCalculateCrc32 ((UINT8 *) Hdr, Hdr->HeaderSize, &Crc);
  Hdr->CRC32 = Crc;
}

EFI_STATUS
EFIAPI
RuntimeDriverConvertPointer (
  IN     UINTN  DebugDisposition,
  IN OUT VOID   **ConvertAddress
  )
/*++

Routine Description:

  Determines the new virtual address that is to be used on subsequent memory accesses.

Arguments:

  DebugDisposition    - Supplies type information for the pointer being converted.
  ConvertAddress      - A pointer to a pointer that is to be fixed to be the value needed
                        for the new virtual address mappings being applied.

Returns:

  EFI_SUCCESS             - The pointer pointed to by Address was modified.
  EFI_NOT_FOUND           - The pointer pointed to by Address was not found to be part
                            of the current memory map. This is normally fatal.
  EFI_INVALID_PARAMETER   - One of the parameters has an invalid value.

--*/
{
  UINTN                 Address;
  UINT64                VirtEndOfRange;
  EFI_MEMORY_DESCRIPTOR *VirtEntry;
  UINTN                 Index;

  //
  // Make sure ConvertAddress is a valid pointer
  //
  if (ConvertAddress == NULL) {
    return EFI_INVALID_PARAMETER;
  }
  //
  // Get the address to convert
  //
  Address = (UINTN) *ConvertAddress;

  //
  // If this is a null pointer, return if it's allowed
  //
  if (Address == 0) {
    if (DebugDisposition & EFI_OPTIONAL_POINTER) {
      return EFI_SUCCESS;
    }

    return EFI_INVALID_PARAMETER;
  }

  VirtEntry             = mVirtualMap;
  for (Index = 0; Index < mVirtualMapMaxIndex; Index++) {
    //
    // To prevent the inclusion of 64-bit math functions a UINTN was placed in
    //  front of VirtEntry->NumberOfPages to cast it to a 32-bit thing on IA-32
    //  platforms. If you get this ASSERT remove the UINTN and do a 64-bit
    //  multiply.
    //
    ASSERT (((UINTN) VirtEntry->NumberOfPages < 0xffffffff) || (sizeof (UINTN) > 4));

    if ((VirtEntry->Attribute & EFI_MEMORY_RUNTIME) == EFI_MEMORY_RUNTIME) {
      if (Address >= VirtEntry->PhysicalStart) {
        VirtEndOfRange = VirtEntry->PhysicalStart + (((UINTN) VirtEntry->NumberOfPages) * EFI_PAGE_SIZE);
        if (Address < VirtEndOfRange) {
          //
          // Compute new address
          //
          *ConvertAddress = (VOID *) (Address - (UINTN) VirtEntry->PhysicalStart + (UINTN) VirtEntry->VirtualStart);
          return EFI_SUCCESS;
        }
      }
    }

    VirtEntry = NextMemoryDescriptor (VirtEntry, mVirtualMapDescriptorSize);
  }

  return EFI_NOT_FOUND;
}

STATIC
EFI_STATUS
RuntimeDriverConvertInternalPointer (
  IN OUT VOID   **ConvertAddress
  )
/*++

Routine Description:

  Determines the new virtual address that is to be used on subsequent memory accesses
  for internal pointers.

Arguments:

  ConvertAddress  - A pointer to a pointer that is to be fixed to be the value needed
                    for the new virtual address mappings being applied.

Returns:

  EFI_SUCCESS             - The pointer pointed to by Address was modified.
  EFI_NOT_FOUND           - The pointer pointed to by Address was not found to be part
                            of the current memory map. This is normally fatal.
  EFI_INVALID_PARAMETER   - One of the parameters has an invalid value.

--*/
{
  return RuntimeDriverConvertPointer (0x0, ConvertAddress);
}

EFI_STATUS
EFIAPI
RuntimeDriverSetVirtualAddressMap (
  IN UINTN                  MemoryMapSize,
  IN UINTN                  DescriptorSize,
  IN UINT32                 DescriptorVersion,
  IN EFI_MEMORY_DESCRIPTOR  *VirtualMap
  )
/*++

Routine Description:

  Changes the runtime addressing mode of EFI firmware from physical to virtual.

Arguments:

  MemoryMapSize     - The size in bytes of VirtualMap.
  DescriptorSize    - The size in bytes of an entry in the VirtualMap.
  DescriptorVersion - The version of the structure entries in VirtualMap.
  VirtualMap        - An array of memory descriptors which contain new virtual
                      address mapping information for all runtime ranges.

Returns:

  EFI_SUCCESS           - The virtual address map has been applied.
  EFI_UNSUPPORTED       - EFI firmware is not at runtime, or the EFI firmware is already in
                          virtual address mapped mode.
  EFI_INVALID_PARAMETER - DescriptorSize or DescriptorVersion is invalid.
  EFI_NO_MAPPING        - A virtual address was not supplied for a range in the memory
                          map that requires a mapping.
  EFI_NOT_FOUND         - A virtual address was supplied for an address that is not found
                          in the memory map.

--*/
{
  EFI_STATUS                    Status;
  EFI_RUNTIME_EVENT_ENTRY       *RuntimeEvent;
  EFI_RUNTIME_IMAGE_ENTRY       *RuntimeImage;
  LIST_ENTRY                    *Link;
  EFI_PHYSICAL_ADDRESS          VirtImageBase;

  //
  // Can only switch to virtual addresses once the memory map is locked down,
  // and can only set it once
  //
  if (!mRuntime.AtRuntime || mRuntime.VirtualMode) {
    return EFI_UNSUPPORTED;
  }
  //
  // Only understand the original descriptor format
  //
  if (DescriptorVersion != EFI_MEMORY_DESCRIPTOR_VERSION || DescriptorSize < sizeof (EFI_MEMORY_DESCRIPTOR)) {
    return EFI_INVALID_PARAMETER;
  }
  //
  // We are now committed to go to virtual mode, so lets get to it!
  //
  mRuntime.VirtualMode = TRUE;

  //
  // ConvertPointer() needs this mVirtualMap to do the conversion. So set up
  // globals we need to parse the virtual address map.
  //
  mVirtualMapDescriptorSize = DescriptorSize;
  mVirtualMapMaxIndex       = MemoryMapSize / DescriptorSize;
  mVirtualMap               = VirtualMap;

  //
  // Currently the bug in StatusCode/RuntimeLib has been fixed, it will
  // check whether in Runtime or not (this is judged by looking at
  // mEfiAtRuntime global So this ReportStatusCode will work
  //
  REPORT_STATUS_CODE (EFI_PROGRESS_CODE, PcdGet32 (PcdStatusCodeValueSetVirtualAddressMap));

  //
  // Signal all the EVT_SIGNAL_VIRTUAL_ADDRESS_CHANGE events.
  // All runtime events are stored in a list in Runtime AP.
  //
  for (Link = mRuntime.EventHead.ForwardLink; Link != &mRuntime.EventHead; Link = Link->ForwardLink) {
    RuntimeEvent = _CR (Link, EFI_RUNTIME_EVENT_ENTRY, Link);
    if ((RuntimeEvent->Type & EVT_SIGNAL_VIRTUAL_ADDRESS_CHANGE) == EVT_SIGNAL_VIRTUAL_ADDRESS_CHANGE) {
      RuntimeEvent->NotifyFunction (
                      RuntimeEvent->Event,
                      RuntimeEvent->NotifyContext
                      );
    }
  }

  //
  // Relocate runtime images. All runtime images are stored in a list in Runtime AP.
  //
  for (Link = mRuntime.ImageHead.ForwardLink; Link != &mRuntime.ImageHead; Link = Link->ForwardLink) {
    RuntimeImage = _CR (Link, EFI_RUNTIME_IMAGE_ENTRY, Link);
    //
    // We don't want to relocate our selves, as we only run in physical mode.
    //
    if (mMyImageBase != RuntimeImage->ImageBase) {

      VirtImageBase = (EFI_PHYSICAL_ADDRESS) (UINTN) RuntimeImage->ImageBase;
      Status  = RuntimeDriverConvertPointer (0, (VOID **) &VirtImageBase);
      ASSERT_EFI_ERROR (Status);

      PeCoffLoaderRelocateImageForRuntime (
        (EFI_PHYSICAL_ADDRESS) (UINTN) RuntimeImage->ImageBase,
        VirtImageBase,
        (UINTN) RuntimeImage->ImageSize,
        RuntimeImage->RelocationData
        );

      InvalidateInstructionCacheRange (RuntimeImage->ImageBase, (UINTN)RuntimeImage->ImageSize);
    }
  }

  //
  // Convert all the Runtime Services except ConvertPointer() and SetVirtualAddressMap()
  // and recompute the CRC-32
  //
  RuntimeDriverConvertInternalPointer ((VOID **) &gRT->GetTime);
  RuntimeDriverConvertInternalPointer ((VOID **) &gRT->SetTime);
  RuntimeDriverConvertInternalPointer ((VOID **) &gRT->GetWakeupTime);
  RuntimeDriverConvertInternalPointer ((VOID **) &gRT->SetWakeupTime);
  RuntimeDriverConvertInternalPointer ((VOID **) &gRT->ResetSystem);
  RuntimeDriverConvertInternalPointer ((VOID **) &gRT->GetNextHighMonotonicCount);
  RuntimeDriverConvertInternalPointer ((VOID **) &gRT->GetVariable);
  RuntimeDriverConvertInternalPointer ((VOID **) &gRT->SetVariable);
  RuntimeDriverConvertInternalPointer ((VOID **) &gRT->GetNextVariableName);
  RuntimeDriverConvertInternalPointer ((VOID **) &gRT->QueryVariableInfo);
  RuntimeDriverConvertInternalPointer ((VOID **) &gRT->UpdateCapsule);
  RuntimeDriverConvertInternalPointer ((VOID **) &gRT->QueryCapsuleCapabilities);
  RuntimeDriverCalculateEfiHdrCrc (&gRT->Hdr);

  //
  // BugBug: PI requires System Configuration Tables Conversion.
  // Currently, we do not implement it.
  //

  //
  // Convert the runtime fields of the EFI System Table and recompute the CRC-32
  //
  RuntimeDriverConvertInternalPointer ((VOID **) &gST->FirmwareVendor);
  RuntimeDriverConvertInternalPointer ((VOID **) &gST->ConfigurationTable);
  RuntimeDriverConvertInternalPointer ((VOID **) &gST->RuntimeServices);
  RuntimeDriverCalculateEfiHdrCrc (&gST->Hdr);

  //
  // At this point, gRT and gST are physical pointers, but the contents of these tables
  // have been converted to runtime.
  //
  //
  // mVirtualMap is only valid during SetVirtualAddressMap() call
  //
  mVirtualMap = NULL;

  return EFI_SUCCESS;
}

EFI_STATUS
EFIAPI
RuntimeDriverInitialize (
  IN EFI_HANDLE                            ImageHandle,
  IN EFI_SYSTEM_TABLE                      *SystemTable
  )
/*++

Routine Description:
  Install Runtime AP. This code includes the EfiDriverLib, but it functions at
  RT in physical mode. The only Lib services are gBS, gRT, and the DEBUG and
  ASSERT macros (they do ReportStatusCode).

Arguments:
  (Standard EFI Image entry - EFI_IMAGE_ENTRY_POINT)

Returns:

  EFI_SUCEESS - Runtime Driver Architectural Protocol Installed

  Other       - Return value from gBS->InstallMultipleProtocolInterfaces

--*/
{
  EFI_STATUS                Status;
  EFI_LOADED_IMAGE_PROTOCOL *MyLoadedImage;

  //
  // This image needs to be exclued from relocation for virtual mode, so cache
  // a copy of the Loaded Image protocol to test later.
  //
  Status = gBS->HandleProtocol (
                  ImageHandle,
                  &gEfiLoadedImageProtocolGuid,
                  (VOID**)&MyLoadedImage
                  );
  ASSERT_EFI_ERROR (Status);
  mMyImageBase = MyLoadedImage->ImageBase;

  //
  // Initialize the table used to compute 32-bit CRCs
  //
  RuntimeDriverInitializeCrc32Table ();

  //
  // Fill in the entries of the EFI Boot Services and EFI Runtime Services Tables
  //
  gBS->CalculateCrc32       = RuntimeDriverCalculateCrc32;
  gRT->SetVirtualAddressMap = RuntimeDriverSetVirtualAddressMap;
  gRT->ConvertPointer       = RuntimeDriverConvertPointer;

  //
  // Install the Runtime Architectural Protocol onto a new handle
  //
  Status = gBS->InstallMultipleProtocolInterfaces (
                  &mRuntimeHandle,
                  &gEfiRuntimeArchProtocolGuid,
                  &mRuntime,
                  NULL
                  );
  ASSERT_EFI_ERROR (Status);

  return EFI_SUCCESS;
}