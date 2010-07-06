/** @file

Copyright (c) 2004 - 2007, Intel Corporation
All rights reserved. This program and the accompanying materials
are licensed and made available under the terms and conditions of the BSD License
which accompanies this distribution.  The full text of the license may be found at
http://opensource.org/licenses/bsd-license.php

THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

Module Name:

  DevicePath.c

Abstract:

  BDS internal function define the default device path string, it can be
  replaced by platform device path.


**/

#include "InternalBdsLib.h"

//
// Platform Code should implement the Vendor specific Device Path display routine.
//
extern
VOID
DevPathVendor (
  IN OUT POOL_PRINT       *Str,
  IN VOID                 *DevPath
  )
;

EFI_GUID  mEfiDevicePathMessagingUartFlowControlGuid = DEVICE_PATH_MESSAGING_UART_FLOW_CONTROL;

EFI_GUID mEfiDevicePathMessagingSASGuid = DEVICE_PATH_MESSAGING_SAS;


VOID *
ReallocatePool (
  IN VOID                 *OldPool,
  IN UINTN                OldSize,
  IN UINTN                NewSize
  )
/*++

Routine Description:

  Adjusts the size of a previously allocated buffer.

Arguments:

  OldPool               - A pointer to the buffer whose size is being adjusted.

  OldSize               - The size of the current buffer.

  NewSize               - The size of the new buffer.

Returns:

  EFI_SUCEESS           - The requested number of bytes were allocated.

  EFI_OUT_OF_RESOURCES  - The pool requested could not be allocated.

  EFI_INVALID_PARAMETER - The buffer was invalid.

--*/
{
  VOID  *NewPool;

  NewPool = NULL;
  if (NewSize) {
    NewPool = AllocateZeroPool (NewSize);
  }

  if (OldPool) {
    if (NewPool) {
      CopyMem (NewPool, OldPool, OldSize < NewSize ? OldSize : NewSize);
    }

    gBS->FreePool (OldPool);
  }

  return NewPool;
}


/**
  Concatenates a formatted unicode string to allocated pool.
  The caller must free the resulting buffer.

  @param  Str      Tracks the allocated pool, size in use, and  amount of pool
                   allocated.
  @param  fmt      The format string

  @return Allocated buffer with the formatted string printed in it.
  @return The caller must free the allocated buffer.   The buffer
  @return allocation is not packed.

**/
CHAR16 *
CatPrint (
  IN OUT POOL_PRINT   *Str,
  IN CHAR16           *fmt,
  ...
  )
{
  UINT16  *AppendStr;
  VA_LIST args;
  UINTN   strsize;

  AppendStr = AllocateZeroPool (0x1000);
  if (AppendStr == NULL) {
    return Str->str;
  }

  VA_START (args, fmt);
  UnicodeVSPrint (AppendStr, 0x1000, fmt, args);
  VA_END (args);
  if (NULL == Str->str) {
    strsize   = StrSize (AppendStr);
    Str->str  = AllocateZeroPool (strsize);
    ASSERT (Str->str != NULL);
  } else {
    strsize = StrSize (AppendStr);
    strsize += (StrSize (Str->str) - sizeof (UINT16));

    Str->str = ReallocatePool (
                Str->str,
                StrSize (Str->str),
                strsize
                );
    ASSERT (Str->str != NULL);
  }

  Str->maxlen = MAX_CHAR * sizeof (UINT16);
  if (strsize < Str->maxlen) {
    StrCat (Str->str, AppendStr);
    Str->len = strsize - sizeof (UINT16);
  }

  gBS->FreePool (AppendStr);
  return Str->str;
}


/**
  Function unpacks a device path data structure so that all the nodes
  of a device path are naturally aligned.

  @param  DevPath  A pointer to a device path data structure

  @return If the memory for the device path is successfully allocated, then a
  @return pointer to the new device path is returned.  Otherwise, NULL is returned.

**/
EFI_DEVICE_PATH_PROTOCOL *
BdsLibUnpackDevicePath (
  IN EFI_DEVICE_PATH_PROTOCOL  *DevPath
  )
{
  EFI_DEVICE_PATH_PROTOCOL  *Src;
  EFI_DEVICE_PATH_PROTOCOL  *Dest;
  EFI_DEVICE_PATH_PROTOCOL  *NewPath;
  UINTN                     Size;

  //
  // Walk device path and round sizes to valid boundries
  //
  Src   = DevPath;
  Size  = 0;
  for (;;) {
    Size += DevicePathNodeLength (Src);
    Size += ALIGN_SIZE (Size);

    if (IsDevicePathEnd (Src)) {
      break;
    }

    Src = NextDevicePathNode (Src);
  }
  //
  // Allocate space for the unpacked path
  //
  NewPath = AllocateZeroPool (Size);
  if (NewPath) {

    ASSERT (((UINTN) NewPath) % MIN_ALIGNMENT_SIZE == 0);

    //
    // Copy each node
    //
    Src   = DevPath;
    Dest  = NewPath;
    for (;;) {
      Size = DevicePathNodeLength (Src);
      CopyMem (Dest, Src, Size);
      Size += ALIGN_SIZE (Size);
      SetDevicePathNodeLength (Dest, Size);
      Dest->Type |= EFI_DP_TYPE_UNPACKED;
      Dest = (EFI_DEVICE_PATH_PROTOCOL *) (((UINT8 *) Dest) + Size);

      if (IsDevicePathEnd (Src)) {
        break;
      }

      Src = NextDevicePathNode (Src);
    }
  }

  return NewPath;
}

VOID
DevPathPci (
  IN OUT POOL_PRINT       *Str,
  IN VOID                 *DevPath
  )
{
  PCI_DEVICE_PATH *Pci;

  Pci = DevPath;
  CatPrint (Str, L"Pci(%x|%x)", (UINTN) Pci->Device, (UINTN) Pci->Function);
}

VOID
DevPathPccard (
  IN OUT POOL_PRINT       *Str,
  IN VOID                 *DevPath
  )
{
  PCCARD_DEVICE_PATH  *Pccard;

  Pccard = DevPath;
  CatPrint (Str, L"Pcmcia(Function%x)", (UINTN) Pccard->FunctionNumber);
}

VOID
DevPathMemMap (
  IN OUT POOL_PRINT       *Str,
  IN VOID                 *DevPath
  )
{
  MEMMAP_DEVICE_PATH  *MemMap;

  MemMap = DevPath;
  CatPrint (
    Str,
    L"MemMap(%d:%lx-%lx)",
    MemMap->MemoryType,
    MemMap->StartingAddress,
    MemMap->EndingAddress
    );
}

VOID
DevPathController (
  IN OUT POOL_PRINT       *Str,
  IN VOID                 *DevPath
  )
{
  CONTROLLER_DEVICE_PATH  *Controller;

  Controller = DevPath;
  CatPrint (Str, L"Ctrl(%d)", (UINTN) Controller->ControllerNumber);
}


/**
  Convert Vendor device path to device name

  @param  Str      The buffer store device name
  @param  DevPath  Pointer to vendor device path

  @return When it return, the device name have been stored in *Str.

**/
VOID
DevPathVendor (
  IN OUT POOL_PRINT       *Str,
  IN VOID                 *DevPath
  )
{
  VENDOR_DEVICE_PATH  *Vendor;
  CHAR16              *Type;
  UINTN               DataLength;
  UINTN               Index;
  UINT32              FlowControlMap;

  UINT16              Info;

  Vendor  = DevPath;

  switch (DevicePathType (&Vendor->Header)) {
  case HARDWARE_DEVICE_PATH:
    Type = L"Hw";
// bugbug: nt 32 specific definition
#if 0
    //
    // If the device is a winntbus device, we will give it a readable device name.
    //
    if (CompareGuid (&Vendor->Guid, &mEfiWinNtThunkProtocolGuid)) {
      CatPrint (Str, L"%s", L"WinNtBus");
      return ;
    } else if (CompareGuid (&Vendor->Guid, &mEfiWinNtGopGuid)) {
      CatPrint (Str, L"%s", L"GOP");
      return ;
    } else if (CompareGuid (&Vendor->Guid, &mEfiWinNtSerialPortGuid)) {
      CatPrint (Str, L"%s", L"Serial");
      return ;
    }
#endif
    break;

  case MESSAGING_DEVICE_PATH:
    Type = L"Msg";
    if (CompareGuid (&Vendor->Guid, &gEfiPcAnsiGuid)) {
      CatPrint (Str, L"VenPcAnsi()");
      return ;
    } else if (CompareGuid (&Vendor->Guid, &gEfiVT100Guid)) {
      CatPrint (Str, L"VenVt100()");
      return ;
    } else if (CompareGuid (&Vendor->Guid, &gEfiVT100PlusGuid)) {
      CatPrint (Str, L"VenVt100Plus()");
      return ;
    } else if (CompareGuid (&Vendor->Guid, &gEfiVTUTF8Guid)) {
      CatPrint (Str, L"VenUft8()");
      return ;
    } else if (CompareGuid (&Vendor->Guid, &mEfiDevicePathMessagingUartFlowControlGuid)) {
      FlowControlMap = (((UART_FLOW_CONTROL_DEVICE_PATH *) Vendor)->FlowControlMap);
      switch (FlowControlMap & 0x00000003) {
      case 0:
        CatPrint (Str, L"UartFlowCtrl(%s)", L"None");
        break;

      case 1:
        CatPrint (Str, L"UartFlowCtrl(%s)", L"Hardware");
        break;

      case 2:
        CatPrint (Str, L"UartFlowCtrl(%s)", L"XonXoff");
        break;

      default:
        break;
      }

      return ;

    } else if (CompareGuid (&Vendor->Guid, &mEfiDevicePathMessagingSASGuid)) {
      CatPrint (
        Str,
        L"SAS(%lx,%lx,%x,",
        ((SAS_DEVICE_PATH *) Vendor)->SasAddress,
        ((SAS_DEVICE_PATH *) Vendor)->Lun,
        ((SAS_DEVICE_PATH *) Vendor)->RelativeTargetPort
        );
      Info = (((SAS_DEVICE_PATH *) Vendor)->DeviceTopology);
      if ((Info & 0x0f) == 0) {
        CatPrint (Str, L"NoTopology,0,0,0,");
      } else if (((Info & 0x0f) == 1) || ((Info & 0x0f) == 2)) {
        CatPrint (
          Str,
          L"%s,%s,%s,",
          (Info & (0x1 << 4)) ? L"SATA" : L"SAS",
          (Info & (0x1 << 5)) ? L"External" : L"Internal",
          (Info & (0x1 << 6)) ? L"Expanded" : L"Direct"
          );
        if ((Info & 0x0f) == 1) {
          CatPrint (Str, L"0,");
        } else {
          CatPrint (Str, L"%x,", (UINTN) ((Info >> 8) & 0xff));
        }
      } else {
        CatPrint (Str, L"0,0,0,0,");
      }

      CatPrint (Str, L"%x)", (UINTN) ((SAS_DEVICE_PATH *) Vendor)->Reserved);
      return ;

    } else if (CompareGuid (&Vendor->Guid, &gEfiDebugPortProtocolGuid)) {
      CatPrint (Str, L"DebugPort()");
      return ;
    }
    break;

  case MEDIA_DEVICE_PATH:
    Type = L"Media";
    break;

  default:
    Type = L"?";
    break;
  }

  CatPrint (Str, L"Ven%s(%g", Type, &Vendor->Guid);
  DataLength = DevicePathNodeLength (&Vendor->Header) - sizeof (VENDOR_DEVICE_PATH);
  if (DataLength > 0) {
    CatPrint (Str, L",");
    for (Index = 0; Index < DataLength; Index++) {
      CatPrint (Str, L"%02x", (UINTN) ((VENDOR_DEVICE_PATH_WITH_DATA *) Vendor)->VendorDefinedData[Index]);
    }
  }
  CatPrint (Str, L")");
}


VOID
DevPathAcpi (
  IN OUT POOL_PRINT       *Str,
  IN VOID                 *DevPath
  )
{
  ACPI_HID_DEVICE_PATH  *Acpi;

  Acpi = DevPath;
  if ((Acpi->HID & PNP_EISA_ID_MASK) == PNP_EISA_ID_CONST) {
    CatPrint (Str, L"Acpi(PNP%04x,%x)", (UINTN)  EISA_ID_TO_NUM (Acpi->HID), (UINTN) Acpi->UID);
  } else {
    CatPrint (Str, L"Acpi(%08x,%x)", (UINTN) Acpi->HID, (UINTN) Acpi->UID);
  }
}

VOID
DevPathExtendedAcpi (
  IN OUT POOL_PRINT       *Str,
  IN VOID                 *DevPath
  )
{
  ACPI_EXTENDED_HID_DEVICE_PATH   *ExtendedAcpi;
  //
  // Index for HID, UID and CID strings, 0 for non-exist
  //
  UINT16                          HIDSTRIdx;
  UINT16                          UIDSTRIdx;
  UINT16                          CIDSTRIdx;
  UINT16                          Index;
  UINT16                          Length;
  UINT16                          Anchor;
  CHAR8                           *AsChar8Array;

  ASSERT (Str != NULL);
  ASSERT (DevPath != NULL);

  HIDSTRIdx    = 0;
  UIDSTRIdx    = 0;
  CIDSTRIdx    = 0;
  ExtendedAcpi = DevPath;
  Length       = (UINT16) DevicePathNodeLength ((EFI_DEVICE_PATH_PROTOCOL *) ExtendedAcpi);

  ASSERT (Length >= 19);
  AsChar8Array = (CHAR8 *) ExtendedAcpi;

  //
  // find HIDSTR
  //
  Anchor = 16;
  for (Index = Anchor; Index < Length && AsChar8Array[Index]; Index++) {
    ;
  }
  if (Index > Anchor) {
    HIDSTRIdx = Anchor;
  }
  //
  // find UIDSTR
  //
  Anchor = (UINT16) (Index + 1);
  for (Index = Anchor; Index < Length && AsChar8Array[Index]; Index++) {
    ;
  }
  if (Index > Anchor) {
    UIDSTRIdx = Anchor;
  }
  //
  // find CIDSTR
  //
  Anchor = (UINT16) (Index + 1);
  for (Index = Anchor; Index < Length && AsChar8Array[Index]; Index++) {
    ;
  }
  if (Index > Anchor) {
    CIDSTRIdx = Anchor;
  }

  if (HIDSTRIdx == 0 && CIDSTRIdx == 0 && ExtendedAcpi->UID == 0) {
    CatPrint (Str, L"AcpiExp(");
    if ((ExtendedAcpi->HID & PNP_EISA_ID_MASK) == PNP_EISA_ID_CONST) {
      CatPrint (Str, L"PNP%04x,", (UINTN) EISA_ID_TO_NUM (ExtendedAcpi->HID));
    } else {
      CatPrint (Str, L"%08x,", (UINTN) ExtendedAcpi->HID);
    }
    if ((ExtendedAcpi->CID & PNP_EISA_ID_MASK) == PNP_EISA_ID_CONST) {
      CatPrint (Str, L"PNP%04x,", (UINTN)  EISA_ID_TO_NUM (ExtendedAcpi->CID));
    } else {
      CatPrint (Str, L"%08x,", (UINTN)  ExtendedAcpi->CID);
    }
    if (UIDSTRIdx != 0) {
      CatPrint (Str, L"%a)", AsChar8Array + UIDSTRIdx);
    } else {
      CatPrint (Str, L"\"\")");
    }
  } else {
    CatPrint (Str, L"AcpiEx(");
    if ((ExtendedAcpi->HID & PNP_EISA_ID_MASK) == PNP_EISA_ID_CONST) {
      CatPrint (Str, L"PNP%04x,", (UINTN)  EISA_ID_TO_NUM (ExtendedAcpi->HID));
    } else {
      CatPrint (Str, L"%08x,", (UINTN) ExtendedAcpi->HID);
    }
    if ((ExtendedAcpi->CID & PNP_EISA_ID_MASK) == PNP_EISA_ID_CONST) {
      CatPrint (Str, L"PNP%04x,", (UINTN) EISA_ID_TO_NUM (ExtendedAcpi->CID));
    } else {
      CatPrint (Str, L"%08x,", (UINTN) ExtendedAcpi->CID);
    }
    CatPrint (Str, L"%x,", (UINTN) ExtendedAcpi->UID);

    if (HIDSTRIdx != 0) {
      CatPrint (Str, L"%a,", AsChar8Array + HIDSTRIdx);
    } else {
      CatPrint (Str, L"\"\",");
    }
    if (CIDSTRIdx != 0) {
      CatPrint (Str, L"%a,", AsChar8Array + CIDSTRIdx);
    } else {
      CatPrint (Str, L"\"\",");
    }
    if (UIDSTRIdx != 0) {
      CatPrint (Str, L"%a)", AsChar8Array + UIDSTRIdx);
    } else {
      CatPrint (Str, L"\"\")");
    }
  }

}

VOID
DevPathAdrAcpi (
  IN OUT POOL_PRINT       *Str,
  IN VOID                 *DevPath
  )
{
  ACPI_ADR_DEVICE_PATH    *AcpiAdr;
  UINT16                  Index;
  UINT16                  Length;
  UINT16                  AdditionalAdrCount;

  AcpiAdr            = DevPath;
  Length             = (UINT16) DevicePathNodeLength ((EFI_DEVICE_PATH_PROTOCOL *) AcpiAdr);
  AdditionalAdrCount = (UINT16) ((Length - 8) / 4);

  CatPrint (Str, L"AcpiAdr(%x", (UINTN) AcpiAdr->ADR);
  for (Index = 0; Index < AdditionalAdrCount; Index++) {
    CatPrint (Str, L",%x", (UINTN) *(UINT32 *) ((UINT8 *) AcpiAdr + 8 + Index * 4));
  }
  CatPrint (Str, L")");
}

VOID
DevPathAtapi (
  IN OUT POOL_PRINT       *Str,
  IN VOID                 *DevPath
  )
{
  ATAPI_DEVICE_PATH *Atapi;

  Atapi = DevPath;
  CatPrint (
    Str,
    L"Ata(%s,%s)",
    Atapi->PrimarySecondary ? L"Secondary" : L"Primary",
    Atapi->SlaveMaster ? L"Slave" : L"Master"
    );
}

VOID
DevPathScsi (
  IN OUT POOL_PRINT       *Str,
  IN VOID                 *DevPath
  )
{
  SCSI_DEVICE_PATH  *Scsi;

  Scsi = DevPath;
  CatPrint (Str, L"Scsi(Pun%x,Lun%x)", (UINTN) Scsi->Pun, (UINTN) Scsi->Lun);
}

VOID
DevPathFibre (
  IN OUT POOL_PRINT       *Str,
  IN VOID                 *DevPath
  )
{
  FIBRECHANNEL_DEVICE_PATH  *Fibre;

  Fibre = DevPath;
  CatPrint (Str, L"Fibre(Wwn%lx,Lun%x)", Fibre->WWN, Fibre->Lun);
}

VOID
DevPath1394 (
  IN OUT POOL_PRINT       *Str,
  IN VOID                 *DevPath
  )
{
  F1394_DEVICE_PATH *F1394;

  F1394 = DevPath;
  CatPrint (Str, L"1394(%g)", &F1394->Guid);
}

VOID
DevPathUsb (
  IN OUT POOL_PRINT       *Str,
  IN VOID                 *DevPath
  )
{
  USB_DEVICE_PATH *Usb;

  Usb = DevPath;
  CatPrint (Str, L"Usb(%x,%x)", (UINTN) Usb->ParentPortNumber, (UINTN) Usb->InterfaceNumber);
}

VOID
DevPathUsbWWID (
  IN OUT POOL_PRINT       *Str,
  IN VOID                 *DevPath
  )
{
  USB_WWID_DEVICE_PATH  *UsbWWId;

  UsbWWId = DevPath;
  CatPrint (
    Str,
    L"UsbWwid(%x,%x,%x,\"WWID\")",
    (UINTN) UsbWWId->VendorId,
    (UINTN) UsbWWId->ProductId,
    (UINTN) UsbWWId->InterfaceNumber
    );
}

VOID
DevPathLogicalUnit (
  IN OUT POOL_PRINT       *Str,
  IN VOID                 *DevPath
  )
{
  DEVICE_LOGICAL_UNIT_DEVICE_PATH *LogicalUnit;

  LogicalUnit = DevPath;
  CatPrint (Str, L"Unit(%x)", (UINTN) LogicalUnit->Lun);
}

VOID
DevPathUsbClass (
  IN OUT POOL_PRINT       *Str,
  IN VOID                 *DevPath
  )
{
  USB_CLASS_DEVICE_PATH *UsbClass;

  UsbClass = DevPath;
  CatPrint (
    Str,
    L"Usb Class(%x,%x,%x,%x,%x)",
    (UINTN) UsbClass->VendorId,
    (UINTN) UsbClass->ProductId,
    (UINTN) UsbClass->DeviceClass,
    (UINTN) UsbClass->DeviceSubClass,
    (UINTN) UsbClass->DeviceProtocol
    );
}

VOID
DevPathSata (
  IN OUT POOL_PRINT       *Str,
  IN VOID                 *DevPath
  )
{
  SATA_DEVICE_PATH *Sata;

  Sata = DevPath;
  CatPrint (
    Str,
    L"Sata(%x,%x,%x)",
    (UINTN) Sata->HBAPortNumber,
    (UINTN) Sata->PortMultiplierPortNumber,
    (UINTN) Sata->Lun
    );
}

VOID
DevPathI2O (
  IN OUT POOL_PRINT       *Str,
  IN VOID                 *DevPath
  )
{
  I2O_DEVICE_PATH *I2O;

  I2O = DevPath;
  CatPrint (Str, L"I2O(%x)", (UINTN) I2O->Tid);
}

VOID
DevPathMacAddr (
  IN OUT POOL_PRINT       *Str,
  IN VOID                 *DevPath
  )
{
  MAC_ADDR_DEVICE_PATH  *MAC;
  UINTN                 HwAddressSize;
  UINTN                 Index;

  MAC           = DevPath;

  HwAddressSize = sizeof (EFI_MAC_ADDRESS);
  if (MAC->IfType == 0x01 || MAC->IfType == 0x00) {
    HwAddressSize = 6;
  }

  CatPrint (Str, L"Mac(");

  for (Index = 0; Index < HwAddressSize; Index++) {
    CatPrint (Str, L"%02x", (UINTN) MAC->MacAddress.Addr[Index]);
  }

  CatPrint (Str, L")");
}

VOID
DevPathIPv4 (
  IN OUT POOL_PRINT       *Str,
  IN VOID                 *DevPath
  )
{
  IPv4_DEVICE_PATH  *IP;

  IP = DevPath;
  CatPrint (
    Str,
    L"IPv4(%d.%d.%d.%d:%d)",
    (UINTN) IP->RemoteIpAddress.Addr[0],
    (UINTN) IP->RemoteIpAddress.Addr[1],
    (UINTN) IP->RemoteIpAddress.Addr[2],
    (UINTN) IP->RemoteIpAddress.Addr[3],
    (UINTN) IP->RemotePort
    );
}

VOID
DevPathIPv6 (
  IN OUT POOL_PRINT       *Str,
  IN VOID                 *DevPath
  )
{
  IPv6_DEVICE_PATH  *IP;

  IP = DevPath;
  CatPrint (
    Str,
    L"IPv6(%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x)",
    (UINTN) IP->RemoteIpAddress.Addr[0],
    (UINTN) IP->RemoteIpAddress.Addr[1],
    (UINTN) IP->RemoteIpAddress.Addr[2],
    (UINTN) IP->RemoteIpAddress.Addr[3],
    (UINTN) IP->RemoteIpAddress.Addr[4],
    (UINTN) IP->RemoteIpAddress.Addr[5],
    (UINTN) IP->RemoteIpAddress.Addr[6],
    (UINTN) IP->RemoteIpAddress.Addr[7],
    (UINTN) IP->RemoteIpAddress.Addr[8],
    (UINTN) IP->RemoteIpAddress.Addr[9],
    (UINTN) IP->RemoteIpAddress.Addr[10],
    (UINTN) IP->RemoteIpAddress.Addr[11],
    (UINTN) IP->RemoteIpAddress.Addr[12],
    (UINTN) IP->RemoteIpAddress.Addr[13],
    (UINTN) IP->RemoteIpAddress.Addr[14],
    (UINTN) IP->RemoteIpAddress.Addr[15]
    );
}

VOID
DevPathInfiniBand (
  IN OUT POOL_PRINT       *Str,
  IN VOID                 *DevPath
  )
{
  INFINIBAND_DEVICE_PATH  *InfiniBand;

  InfiniBand = DevPath;
  CatPrint (
    Str,
    L"Infiniband(%x,%g,%lx,%lx,%lx)",
    (UINTN) InfiniBand->ResourceFlags,
    InfiniBand->PortGid,
    InfiniBand->ServiceId,
    InfiniBand->TargetPortId,
    InfiniBand->DeviceId
    );
}

VOID
DevPathUart (
  IN OUT POOL_PRINT       *Str,
  IN VOID                 *DevPath
  )
{
  UART_DEVICE_PATH  *Uart;
  CHAR8             Parity;

  Uart = DevPath;
  switch (Uart->Parity) {
  case 0:
    Parity = 'D';
    break;

  case 1:
    Parity = 'N';
    break;

  case 2:
    Parity = 'E';
    break;

  case 3:
    Parity = 'O';
    break;

  case 4:
    Parity = 'M';
    break;

  case 5:
    Parity = 'S';
    break;

  default:
    Parity = 'x';
    break;
  }

  if (Uart->BaudRate == 0) {
    CatPrint (Str, L"Uart(DEFAULT,%c,", Parity);
  } else {
    CatPrint (Str, L"Uart(%d,%c,", Uart->BaudRate, Parity);
  }

  if (Uart->DataBits == 0) {
    CatPrint (Str, L"D,");
  } else {
    CatPrint (Str, L"%d,", (UINTN) Uart->DataBits);
  }

  switch (Uart->StopBits) {
  case 0:
    CatPrint (Str, L"D)");
    break;

  case 1:
    CatPrint (Str, L"1)");
    break;

  case 2:
    CatPrint (Str, L"1.5)");
    break;

  case 3:
    CatPrint (Str, L"2)");
    break;

  default:
    CatPrint (Str, L"x)");
    break;
  }
}

VOID
DevPathiSCSI (
  IN OUT POOL_PRINT       *Str,
  IN VOID                 *DevPath
  )
{
  ISCSI_DEVICE_PATH_WITH_NAME *iSCSI;
  UINT16                      Options;

  ASSERT (Str != NULL);
  ASSERT (DevPath != NULL);

  iSCSI = DevPath;
  CatPrint (
    Str,
    L"iSCSI(%s,%x,%lx,",
    iSCSI->iSCSITargetName,
    iSCSI->TargetPortalGroupTag,
    iSCSI->Lun
    );

  Options = iSCSI->LoginOption;
  CatPrint (Str, L"%s,", ((Options >> 1) & 0x0001) ? L"CRC32C" : L"None");
  CatPrint (Str, L"%s,", ((Options >> 3) & 0x0001) ? L"CRC32C" : L"None");
  if ((Options >> 11) & 0x0001) {
    CatPrint (Str, L"%s,", L"None");
  } else if ((Options >> 12) & 0x0001) {
    CatPrint (Str, L"%s,", L"CHAP_UNI");
  } else {
    CatPrint (Str, L"%s,", L"CHAP_BI");

  }

  CatPrint (Str, L"%s)", (iSCSI->NetworkProtocol == 0) ? L"TCP" : L"reserved");
}

VOID
DevPathHardDrive (
  IN OUT POOL_PRINT       *Str,
  IN VOID                 *DevPath
  )
{
  HARDDRIVE_DEVICE_PATH *Hd;

  Hd = DevPath;
  switch (Hd->SignatureType) {
  case SIGNATURE_TYPE_MBR:
    CatPrint (
      Str,
      L"HD(Part%d,Sig%08x)",
      (UINTN) Hd->PartitionNumber,
      (UINTN) *((UINT32 *) (&(Hd->Signature[0])))
      );
    break;

  case SIGNATURE_TYPE_GUID:
    CatPrint (
      Str,
      L"HD(Part%d,Sig%g)",
      (UINTN) Hd->PartitionNumber,
      (EFI_GUID *) &(Hd->Signature[0])
      );
    break;

  default:
    CatPrint (
      Str,
      L"HD(Part%d,MBRType=%02x,SigType=%02x)",
      (UINTN) Hd->PartitionNumber,
      (UINTN) Hd->MBRType,
      (UINTN) Hd->SignatureType
      );
    break;
  }
}

VOID
DevPathCDROM (
  IN OUT POOL_PRINT       *Str,
  IN VOID                 *DevPath
  )
{
  CDROM_DEVICE_PATH *Cd;

  Cd = DevPath;
  CatPrint (Str, L"CDROM(Entry%x)", (UINTN) Cd->BootEntry);
}

VOID
DevPathFilePath (
  IN OUT POOL_PRINT       *Str,
  IN VOID                 *DevPath
  )
{
  FILEPATH_DEVICE_PATH  *Fp;

  Fp = DevPath;
  CatPrint (Str, L"%s", Fp->PathName);
}

VOID
DevPathMediaProtocol (
  IN OUT POOL_PRINT       *Str,
  IN VOID                 *DevPath
  )
{
  MEDIA_PROTOCOL_DEVICE_PATH  *MediaProt;

  MediaProt = DevPath;
  CatPrint (Str, L"Media(%g)", &MediaProt->Protocol);
}

VOID
DevPathFvFilePath (
  IN OUT POOL_PRINT       *Str,
  IN VOID                 *DevPath
  )
{
  MEDIA_FW_VOL_FILEPATH_DEVICE_PATH *FvFilePath;

  FvFilePath = DevPath;
  CatPrint (Str, L"%g", &FvFilePath->FvFileName);
}

VOID
DevPathBssBss (
  IN OUT POOL_PRINT       *Str,
  IN VOID                 *DevPath
  )
{
  BBS_BBS_DEVICE_PATH *Bbs;
  CHAR16              *Type;

  Bbs = DevPath;
  switch (Bbs->DeviceType) {
  case BBS_TYPE_FLOPPY:
    Type = L"Floppy";
    break;

  case BBS_TYPE_HARDDRIVE:
    Type = L"Harddrive";
    break;

  case BBS_TYPE_CDROM:
    Type = L"CDROM";
    break;

  case BBS_TYPE_PCMCIA:
    Type = L"PCMCIA";
    break;

  case BBS_TYPE_USB:
    Type = L"Usb";
    break;

  case BBS_TYPE_EMBEDDED_NETWORK:
    Type = L"Net";
    break;

  case BBS_TYPE_BEV:
    Type = L"BEV";
    break;

  default:
    Type = L"?";
    break;
  }
  CatPrint (Str, L"Legacy-%s", Type);
}

VOID
DevPathEndInstance (
  IN OUT POOL_PRINT       *Str,
  IN VOID                 *DevPath
  )
{
  CatPrint (Str, L",");
}

VOID
DevPathNodeUnknown (
  IN OUT POOL_PRINT       *Str,
  IN VOID                 *DevPath
  )
{
  CatPrint (Str, L"?");
}

DEVICE_PATH_STRING_TABLE  DevPathTable[] = {
  HARDWARE_DEVICE_PATH,
  HW_PCI_DP,
  DevPathPci,
  HARDWARE_DEVICE_PATH,
  HW_PCCARD_DP,
  DevPathPccard,
  HARDWARE_DEVICE_PATH,
  HW_MEMMAP_DP,
  DevPathMemMap,
  HARDWARE_DEVICE_PATH,
  HW_VENDOR_DP,
  DevPathVendor,
  HARDWARE_DEVICE_PATH,
  HW_CONTROLLER_DP,
  DevPathController,
  ACPI_DEVICE_PATH,
  ACPI_DP,
  DevPathAcpi,
  ACPI_DEVICE_PATH,
  ACPI_EXTENDED_DP,
  DevPathExtendedAcpi,
  ACPI_DEVICE_PATH,
  ACPI_ADR_DP,
  DevPathAdrAcpi,
  MESSAGING_DEVICE_PATH,
  MSG_ATAPI_DP,
  DevPathAtapi,
  MESSAGING_DEVICE_PATH,
  MSG_SCSI_DP,
  DevPathScsi,
  MESSAGING_DEVICE_PATH,
  MSG_FIBRECHANNEL_DP,
  DevPathFibre,
  MESSAGING_DEVICE_PATH,
  MSG_1394_DP,
  DevPath1394,
  MESSAGING_DEVICE_PATH,
  MSG_USB_DP,
  DevPathUsb,
  MESSAGING_DEVICE_PATH,
  MSG_USB_WWID_DP,
  DevPathUsbWWID,
  MESSAGING_DEVICE_PATH,
  MSG_DEVICE_LOGICAL_UNIT_DP,
  DevPathLogicalUnit,
  MESSAGING_DEVICE_PATH,
  MSG_USB_CLASS_DP,
  DevPathUsbClass,
  MESSAGING_DEVICE_PATH,
  MSG_SATA_DP,
  DevPathSata,
  MESSAGING_DEVICE_PATH,
  MSG_I2O_DP,
  DevPathI2O,
  MESSAGING_DEVICE_PATH,
  MSG_MAC_ADDR_DP,
  DevPathMacAddr,
  MESSAGING_DEVICE_PATH,
  MSG_IPv4_DP,
  DevPathIPv4,
  MESSAGING_DEVICE_PATH,
  MSG_IPv6_DP,
  DevPathIPv6,
  MESSAGING_DEVICE_PATH,
  MSG_INFINIBAND_DP,
  DevPathInfiniBand,
  MESSAGING_DEVICE_PATH,
  MSG_UART_DP,
  DevPathUart,
  MESSAGING_DEVICE_PATH,
  MSG_VENDOR_DP,
  DevPathVendor,
  MESSAGING_DEVICE_PATH,
  MSG_ISCSI_DP,
  DevPathiSCSI,
  MEDIA_DEVICE_PATH,
  MEDIA_HARDDRIVE_DP,
  DevPathHardDrive,
  MEDIA_DEVICE_PATH,
  MEDIA_CDROM_DP,
  DevPathCDROM,
  MEDIA_DEVICE_PATH,
  MEDIA_VENDOR_DP,
  DevPathVendor,
  MEDIA_DEVICE_PATH,
  MEDIA_FILEPATH_DP,
  DevPathFilePath,
  MEDIA_DEVICE_PATH,
  MEDIA_PROTOCOL_DP,
  DevPathMediaProtocol,
  MEDIA_DEVICE_PATH,
  MEDIA_PIWG_FW_FILE_DP,
  DevPathFvFilePath,
  BBS_DEVICE_PATH,
  BBS_BBS_DP,
  DevPathBssBss,
  END_DEVICE_PATH_TYPE,
  END_INSTANCE_DEVICE_PATH_SUBTYPE,
  DevPathEndInstance,
  0,
  0,
  NULL
};


/**

**/
CHAR16 *
DevicePathToStr (
  IN EFI_DEVICE_PATH_PROTOCOL     *DevPath
  )
{
  POOL_PRINT                Str;
  EFI_DEVICE_PATH_PROTOCOL  *DevPathNode;
  VOID (*DumpNode) (POOL_PRINT *, VOID *);

  UINTN Index;
  UINTN NewSize;

  EFI_STATUS                       Status;
  CHAR16                           *ToText;
  EFI_DEVICE_PATH_TO_TEXT_PROTOCOL *DevPathToText;

  ZeroMem (&Str, sizeof (Str));

  if (DevPath == NULL) {
    goto Done;
  }

  Status = gBS->LocateProtocol (
                  &gEfiDevicePathToTextProtocolGuid,
                  NULL,
                  (VOID **) &DevPathToText
                  );
  if (!EFI_ERROR (Status)) {
    ToText = DevPathToText->ConvertDevicePathToText (
                              DevPath,
                              FALSE,
                              TRUE
                              );
    ASSERT (ToText != NULL);
    return ToText;
  }

  //
  // Unpacked the device path
  //
  DevPath = BdsLibUnpackDevicePath (DevPath);
  ASSERT (DevPath);

  //
  // Process each device path node
  //
  DevPathNode = DevPath;
  while (!IsDevicePathEnd (DevPathNode)) {
    //
    // Find the handler to dump this device path node
    //
    DumpNode = NULL;
    for (Index = 0; DevPathTable[Index].Function; Index += 1) {

      if (DevicePathType (DevPathNode) == DevPathTable[Index].Type &&
          DevicePathSubType (DevPathNode) == DevPathTable[Index].SubType
          ) {
        DumpNode = DevPathTable[Index].Function;
        break;
      }
    }
    //
    // If not found, use a generic function
    //
    if (!DumpNode) {
      DumpNode = DevPathNodeUnknown;
    }
    //
    //  Put a path seperator in if needed
    //
    if (Str.len && DumpNode != DevPathEndInstance) {
      CatPrint (&Str, L"/");
    }
    //
    // Print this node of the device path
    //
    DumpNode (&Str, DevPathNode);

    //
    // Next device path node
    //
    DevPathNode = NextDevicePathNode (DevPathNode);
  }
  //
  // Shrink pool used for string allocation
  //
  gBS->FreePool (DevPath);

Done:
  NewSize = (Str.len + 1) * sizeof (CHAR16);
  Str.str = ReallocatePool (Str.str, NewSize, NewSize);
  ASSERT (Str.str != NULL);
  Str.str[Str.len] = 0;
  return Str.str;
}


/**
  Function creates a device path data structure that identically matches the
  device path passed in.

  @param  DevPath  A pointer to a device path data structure.

  @return The new copy of DevPath is created to identically match the input.
  @return Otherwise, NULL is returned.

**/
EFI_DEVICE_PATH_PROTOCOL *
LibDuplicateDevicePathInstance (
  IN EFI_DEVICE_PATH_PROTOCOL  *DevPath
  )
{
  EFI_DEVICE_PATH_PROTOCOL  *NewDevPath;
  EFI_DEVICE_PATH_PROTOCOL  *DevicePathInst;
  EFI_DEVICE_PATH_PROTOCOL  *Temp;
  UINTN                     Size;

  //
  // get the size of an instance from the input
  //
  Temp            = DevPath;
  DevicePathInst  = GetNextDevicePathInstance (&Temp, &Size);

  //
  // Make a copy
  //
  NewDevPath = NULL;
  if (Size) {
    NewDevPath = AllocateZeroPool (Size);
    ASSERT (NewDevPath != NULL);
  }

  if (NewDevPath) {
    CopyMem (NewDevPath, DevicePathInst, Size);
  }

  return NewDevPath;
}
