/*++

Copyright (c) 2006, Intel Corporation                                                         
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution.  The full text of the license may be found at        
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

Module Name:
  
    EfiScriptLib.c

Abstract:

  Support for EFI script. 

--*/

#include "EfiScriptLib.h"

EFI_BOOT_SCRIPT_SAVE_PROTOCOL *mBootScriptSave;

EFI_STATUS
BootScriptSaveInitialize (
  IN EFI_HANDLE           ImageHandle,
  IN EFI_SYSTEM_TABLE     *SystemTable
  )
/*++

Routine Description:

  Intialize Boot Script Lib if it has not yet been initialized. 

Arguments:

  (Standard EFI Image entry - EFI_IMAGE_ENTRY_POINT)

Returns: 

  EFI_STATUS always returns EFI_SUCCESS

--*/
// GC_TODO:    ImageHandle - add argument and description to function comment
// GC_TODO:    SystemTable - add argument and description to function comment
{
  EFI_STATUS        Status;
  EFI_BOOT_SERVICES *BS;

  BS      = SystemTable->BootServices;

  Status  = BS->LocateProtocol (&gEfiBootScriptSaveGuid, NULL, &mBootScriptSave);
  if (EFI_ERROR (Status) && Status != EFI_ALREADY_STARTED) {
    mBootScriptSave = NULL;
  }

  return EFI_SUCCESS;
}

EFI_STATUS
BootScriptSaveIoWrite (
  IN  UINT16                            TableName,
  IN  EFI_BOOT_SCRIPT_WIDTH             Width,
  IN  UINT64                            Address,
  IN  UINTN                             Count,
  IN  VOID                              *Buffer
  )
/*++

Routine Description:

  Save I/O write to boot script 

Arguments:

  TableName - Desired boot script table

  (Standard EFI IO write script parameter) 

Returns: 
  
  EFI_NOT_FOUND - BootScriptSave Protocol not exist.
  
  EFI_STATUS - BootScriptSave Protocol exist, always returns EFI_SUCCESS

--*/
// GC_TODO:    Width - add argument and description to function comment
// GC_TODO:    Address - add argument and description to function comment
// GC_TODO:    Count - add argument and description to function comment
// GC_TODO:    Buffer - add argument and description to function comment
{
  if (mBootScriptSave == NULL) {
    return EFI_NOT_FOUND;
  }

  mBootScriptSave->Write (
                    mBootScriptSave,
                    TableName,
                    EFI_BOOT_SCRIPT_IO_WRITE_OPCODE,
                    Width,
                    Address,
                    Count,
                    Buffer
                    );

  return EFI_SUCCESS;
}

EFI_STATUS
BootScriptSaveIoReadWrite (
  IN  UINT16                            TableName,
  IN  EFI_BOOT_SCRIPT_WIDTH             Width,
  IN  UINT64                            Address,
  IN  VOID                              *Data,
  IN  VOID                              *DataMask
  )
/*++

Routine Description:

  Save I/O write to boot script 

Arguments:

  TableName - Desired boot script table

  (Standard EFI IO read write script parameter) 

Returns: 
  
  EFI_NOT_FOUND - BootScriptSave Protocol not exist.
  
  EFI_STATUS - BootScriptSave Protocol exist, always returns EFI_SUCCESS

--*/
// GC_TODO:    Width - add argument and description to function comment
// GC_TODO:    Address - add argument and description to function comment
// GC_TODO:    Data - add argument and description to function comment
// GC_TODO:    DataMask - add argument and description to function comment
{
  if (mBootScriptSave == NULL) {
    return EFI_NOT_FOUND;
  }

  mBootScriptSave->Write (
                    mBootScriptSave,
                    TableName,
                    EFI_BOOT_SCRIPT_IO_READ_WRITE_OPCODE,
                    Width,
                    Address,
                    Data,
                    DataMask
                    );

  return EFI_SUCCESS;
}

EFI_STATUS
BootScriptSaveMemWrite (
  IN  UINT16                            TableName,
  IN  EFI_BOOT_SCRIPT_WIDTH             Width,
  IN  UINT64                            Address,
  IN  UINTN                             Count,
  IN  VOID                              *Buffer
  )
/*++

Routine Description:

  Save I/O write to boot script 

Arguments:

  TableName - Desired boot script table

  (Standard EFI MEM write script parameter) 

Returns: 
  
  EFI_NOT_FOUND - BootScriptSave Protocol not exist.
  
  EFI_STATUS - BootScriptSave Protocol exist, always returns EFI_SUCCESS

--*/
// GC_TODO:    Width - add argument and description to function comment
// GC_TODO:    Address - add argument and description to function comment
// GC_TODO:    Count - add argument and description to function comment
// GC_TODO:    Buffer - add argument and description to function comment
{
  if (mBootScriptSave == NULL) {
    return EFI_NOT_FOUND;
  }

  mBootScriptSave->Write (
                    mBootScriptSave,
                    TableName,
                    EFI_BOOT_SCRIPT_MEM_WRITE_OPCODE,
                    Width,
                    Address,
                    Count,
                    Buffer
                    );

  return EFI_SUCCESS;
}

EFI_STATUS
BootScriptSaveMemReadWrite (
  IN  UINT16                            TableName,
  IN  EFI_BOOT_SCRIPT_WIDTH             Width,
  IN  UINT64                            Address,
  IN  VOID                              *Data,
  IN  VOID                              *DataMask
  )
/*++

Routine Description:

  Save I/O write to boot script 

Arguments:

  TableName - Desired boot script table

  (Standard EFI MEM read write script parameter) 

Returns: 
  
  EFI_NOT_FOUND - BootScriptSave Protocol not exist.
  
  EFI_STATUS - BootScriptSave Protocol exist, always returns EFI_SUCCESS

--*/
// GC_TODO:    Width - add argument and description to function comment
// GC_TODO:    Address - add argument and description to function comment
// GC_TODO:    Data - add argument and description to function comment
// GC_TODO:    DataMask - add argument and description to function comment
{
  if (mBootScriptSave == NULL) {
    return EFI_NOT_FOUND;
  }

  mBootScriptSave->Write (
                    mBootScriptSave,
                    TableName,
                    EFI_BOOT_SCRIPT_MEM_READ_WRITE_OPCODE,
                    Width,
                    Address,
                    Data,
                    DataMask
                    );

  return EFI_SUCCESS;
}

EFI_STATUS
BootScriptSavePciCfgWrite (
  IN  UINT16                            TableName,
  IN  EFI_BOOT_SCRIPT_WIDTH             Width,
  IN  UINT64                            Address,
  IN  UINTN                             Count,
  IN  VOID                              *Buffer
  )
/*++

Routine Description:

  Save I/O write to boot script 

Arguments:

  TableName - Desired boot script table

  (Standard EFI PCI write script parameter) 

Returns: 
  
  EFI_NOT_FOUND - BootScriptSave Protocol not exist.
  
  EFI_STATUS - BootScriptSave Protocol exist, always returns EFI_SUCCESS

--*/
// GC_TODO:    Width - add argument and description to function comment
// GC_TODO:    Address - add argument and description to function comment
// GC_TODO:    Count - add argument and description to function comment
// GC_TODO:    Buffer - add argument and description to function comment
{
  if (mBootScriptSave == NULL) {
    return EFI_NOT_FOUND;
  }

  mBootScriptSave->Write (
                    mBootScriptSave,
                    TableName,
                    EFI_BOOT_SCRIPT_PCI_CONFIG_WRITE_OPCODE,
                    Width,
                    Address,
                    Count,
                    Buffer
                    );

  return EFI_SUCCESS;
}

EFI_STATUS
BootScriptSavePciCfgReadWrite (
  IN  UINT16                            TableName,
  IN  EFI_BOOT_SCRIPT_WIDTH             Width,
  IN  UINT64                            Address,
  IN  VOID                              *Data,
  IN  VOID                              *DataMask
  )
/*++

Routine Description:

  Save I/O write to boot script 

Arguments:

  TableName - Desired boot script table

  (Standard EFI PCI read write script parameter) 

Returns: 
  
  EFI_NOT_FOUND - BootScriptSave Protocol not exist.
  
  EFI_STATUS - BootScriptSave Protocol exist, always returns EFI_SUCCESS

--*/
// GC_TODO:    Width - add argument and description to function comment
// GC_TODO:    Address - add argument and description to function comment
// GC_TODO:    Data - add argument and description to function comment
// GC_TODO:    DataMask - add argument and description to function comment
{
  if (mBootScriptSave == NULL) {
    return EFI_NOT_FOUND;
  }

  mBootScriptSave->Write (
                    mBootScriptSave,
                    TableName,
                    EFI_BOOT_SCRIPT_PCI_CONFIG_READ_WRITE_OPCODE,
                    Width,
                    Address,
                    Data,
                    DataMask
                    );

  return EFI_SUCCESS;
}

EFI_STATUS
BootScriptSaveSmbusExecute (
  IN  UINT16                            TableName,
  IN  EFI_SMBUS_DEVICE_ADDRESS          SlaveAddress,
  IN  EFI_SMBUS_DEVICE_COMMAND          Command,
  IN  EFI_SMBUS_OPERATION               Operation,
  IN  BOOLEAN                           PecCheck,
  IN  UINTN                             *Length,
  IN  VOID                              *Buffer
  )
/*++

Routine Description:

  Save I/O write to boot script 

Arguments:

  TableName - Desired boot script table

  (Standard EFI Smbus execute script parameter) 

Returns: 
  
  EFI_NOT_FOUND - BootScriptSave Protocol not exist.
  
  EFI_STATUS - BootScriptSave Protocol exist, always returns EFI_SUCCESS

--*/
// GC_TODO:    SlaveAddress - add argument and description to function comment
// GC_TODO:    Command - add argument and description to function comment
// GC_TODO:    Operation - add argument and description to function comment
// GC_TODO:    PecCheck - add argument and description to function comment
// GC_TODO:    Length - add argument and description to function comment
// GC_TODO:    Buffer - add argument and description to function comment
{
  if (mBootScriptSave == NULL) {
    return EFI_NOT_FOUND;
  }

  mBootScriptSave->Write (
                    mBootScriptSave,
                    TableName,
                    EFI_BOOT_SCRIPT_SMBUS_EXECUTE_OPCODE,
                    SlaveAddress,
                    Command,
                    Operation,
                    PecCheck,
                    Length,
                    Buffer
                    );

  return EFI_SUCCESS;
}

EFI_STATUS
BootScriptSaveStall (
  IN  UINT16                            TableName,
  IN  UINTN                             Duration
  )
/*++

Routine Description:

  Save I/O write to boot script 

Arguments:

  TableName - Desired boot script table

  (Standard EFI stall script parameter) 

Returns: 
  
  EFI_NOT_FOUND - BootScriptSave Protocol not exist.
  
  EFI_STATUS - BootScriptSave Protocol exist, always returns EFI_SUCCESS

--*/
// GC_TODO:    Duration - add argument and description to function comment
{
  if (mBootScriptSave == NULL) {
    return EFI_NOT_FOUND;
  }

  mBootScriptSave->Write (
                    mBootScriptSave,
                    TableName,
                    EFI_BOOT_SCRIPT_STALL_OPCODE,
                    Duration
                    );

  return EFI_SUCCESS;
}

EFI_STATUS
BootScriptSaveDispatch (
  IN  UINT16                            TableName,
  IN  EFI_PHYSICAL_ADDRESS              EntryPoint
  )
/*++

Routine Description:

  GC_TODO: Add function description

Arguments:

  TableName   - GC_TODO: add argument description
  EntryPoint  - GC_TODO: add argument description

Returns:

  EFI_NOT_FOUND - GC_TODO: Add description for return value
  EFI_SUCCESS - GC_TODO: Add description for return value

--*/
{
  if (mBootScriptSave == NULL) {
    return EFI_NOT_FOUND;
  }

  mBootScriptSave->Write (
                    mBootScriptSave,
                    TableName,
                    EFI_BOOT_SCRIPT_DISPATCH_OPCODE,
                    EntryPoint
                    );

  return EFI_SUCCESS;

}

EFI_STATUS
EFIAPI
BootScriptSaveInformation (
  IN  UINT16                                 TableName,
  IN  UINT32                                 Length, 
  IN  EFI_PHYSICAL_ADDRESS                   Buffer
  )
/*++

Routine Description:

  Save a Information Opcode record in table specified with TableName

Arguments:

  TableName   - Desired boot script table
  Length         - Length of information in bytes
  Buffer          - Content of information that will be saved in script table

Returns:

  EFI_NOT_FOUND - BootScriptSave Protocol not exist.
  
  EFI_STATUS - BootScriptSave Protocol exist, always returns EFI_SUCCESS

--*/
{
  if (mBootScriptSave == NULL) {
    return EFI_NOT_FOUND;
  }

  mBootScriptSave->Write (
                    mBootScriptSave,
                    TableName,
                    EFI_BOOT_SCRIPT_INFORMATION_OPCODE,
                    Length,
                    Buffer
                    );

  return EFI_SUCCESS;

}

EFI_STATUS
EFIAPI
BootScriptSaveInformationUnicodeString (
  IN        UINT16              TableName,
  IN        CHAR16              *String
  )
/*++

Routine Description:

  Save a Information Opcode record in table specified with TableName, the information
  is a unicode string.

Arguments:

  TableName   - Desired boot script table
  String          - The string that will be saved in script table

Returns:

  EFI_NOT_FOUND - BootScriptSave Protocol not exist.
  
  EFI_STATUS - BootScriptSave Protocol exist, always returns EFI_SUCCESS

--*/
{
  return BootScriptSaveInformation (      
           TableName,
           (UINT32) EfiStrLen (String) * 2 + 2, 
           (EFI_PHYSICAL_ADDRESS)String
           );
}

EFI_STATUS
EFIAPI
BootScriptSaveInformationAsciiString (
  IN        UINT16              TableName,
  IN        CHAR8               *String
  )
/*++

Routine Description:

  Save a Information Opcode record in table specified with TableName, the information
  is a ascii string.

Arguments:

  TableName   - Desired boot script table
  String          - The string that will be saved in script table

Returns:

  EFI_NOT_FOUND - BootScriptSave Protocol not exist.
  
  EFI_STATUS - BootScriptSave Protocol exist, always returns EFI_SUCCESS

--*/
{
  return BootScriptSaveInformation (      
           TableName,
           (UINT32) EfiAsciiStrLen (String) + 1, 
           (EFI_PHYSICAL_ADDRESS)String
           );
}

