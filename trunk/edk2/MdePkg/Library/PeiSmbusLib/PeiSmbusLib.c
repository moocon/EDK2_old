/** @file
Implementation of SmBusLib class library for PEI phase.

Copyright (c) 2006, Intel Corporation<BR>
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution.  The full text of the license may be found at        
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             


Module Name: PeiSmbusLib.c

**/

#include "InternalSmbusLib.h"

/**
  Gets Smbus PPIs.

  This internal function retrieves Smbus PPI from PPI database.

  @param  PeiServices   An indirect pointer to the EFI_PEI_SERVICES published by the PEI Foundation.

  @return The pointer to Smbus PPI.

**/
EFI_PEI_SMBUS_PPI *
InternalGetSmbusPpi (
  EFI_PEI_SERVICES      **PeiServices
  ) 
{
  EFI_STATUS            Status;
  EFI_PEI_SMBUS_PPI     *SmbusPpi;

  Status = (*PeiServices)->LocatePpi (PeiServices, &gEfiPeiSmbusPpiGuid, 0, NULL, (VOID **) &SmbusPpi);
  ASSERT_EFI_ERROR (Status);
  ASSERT (SmbusPpi != NULL);

  return SmbusPpi;
}

/**
  Executes an SMBus operation to an SMBus controller. 

  This function provides a standard way to execute Smbus script
  as defined in the SmBus Specification. The data can either be of
  the Length byte, word, or a block of data.

  @param  SmbusOperation  Signifies which particular SMBus hardware protocol instance that it will use to
                          execute the SMBus transactions.
  @param  SmBusAddress    Address that encodes the SMBUS Slave Address,
                          SMBUS Command, SMBUS Data Length, and PEC.
  @param  Length          Signifies the number of bytes that this operation will do. The maximum number of
                          bytes can be revision specific and operation specific.
  @param  Buffer          Contains the value of data to execute to the SMBus slave device. Not all operations
                          require this argument. The length of this buffer is identified by Length.
  @param  Status          Return status for the executed command.
                          This is an optional parameter and may be NULL.

  @return The actual number of bytes that are executed for this operation..

**/
UINTN
InternalSmBusExec (
  IN     EFI_SMBUS_OPERATION        SmbusOperation,
  IN     UINTN                      SmBusAddress,
  IN     UINTN                      Length,
  IN     VOID                       *Buffer,
     OUT RETURN_STATUS              *Status        OPTIONAL
  )
{
  EFI_PEI_SMBUS_PPI         *SmbusPpi;
  EFI_PEI_SERVICES          **PeiServices;
  RETURN_STATUS             ReturnStatus;
  EFI_SMBUS_DEVICE_ADDRESS  SmbusDeviceAddress;

  PeiServices = GetPeiServicesTablePointer ();
  SmbusPpi    = InternalGetSmbusPpi (PeiServices);
  SmbusDeviceAddress.SmbusDeviceAddress = SMBUS_LIB_SLAVE_ADDRESS (SmBusAddress);

  ReturnStatus = SmbusPpi->Execute (
                             PeiServices,
                             SmbusPpi,
                             SmbusDeviceAddress,
                             SMBUS_LIB_COMMAND (SmBusAddress),
                             SmbusOperation,
                             SMBUS_LIB_PEC (SmBusAddress),  
                             &Length,
                             Buffer
                             );
  if (Status != NULL) {
    *Status = ReturnStatus;
  }

  return Length;
}

/**
  Assigns an SMBUS slave addresses.

  Assigns the SMBUS device specified by Uuid the slave address specified by SmBusAddress.
  The status of the executed command is returned.
  If Command in SmBusAddress is not zero, then ASSERT().
  If Length in SmBusAddress is not zero, then ASSERT().
  If PEC in SmBusAddress is set, then ASSERT().
  If any reserved bits of SmBusAddress are set, then ASSERT().

  @param  SmBusAddress        Address that encodes the SMBUS Slave Address,
                              SMBUS Command, SMBUS Data Length, and PEC.
  @param  Uuid                Pointer to the UUID of the device to assign a slave address.
                              It will assign to all SMBUS slave devices if it is NULL.

  @retval RETURN_SUCCESS      The SMBUS command was executed.
  @retval RETURN_TIMEOUT      A timeout occurred while executing the SMBUS command.
  @retval RETURN_DEVICE_ERROR The request was not completed because a failure reflected
                              in the Host Status Register bit.
                              Device errors are a result of a transaction collision, illegal command field,
                              unclaimed cycle (host initiated), or bus errors (collisions).

**/
RETURN_STATUS
InternalSmBusArpDevice (
  IN UINTN          SmBusAddress,
  IN CONST GUID     *Uuid       OPTIONAL 
  )
{
  EFI_PEI_SMBUS_PPI         *SmbusPpi;
  EFI_PEI_SERVICES          **PeiServices;
  EFI_SMBUS_DEVICE_ADDRESS  SmbusDeviceAddress;

  PeiServices = GetPeiServicesTablePointer ();
  SmbusPpi    = InternalGetSmbusPpi (PeiServices);

  SmbusDeviceAddress.SmbusDeviceAddress = SMBUS_LIB_SLAVE_ADDRESS (SmBusAddress);
  return (RETURN_STATUS) SmbusPpi->ArpDevice (
                                     PeiServices,
                                     SmbusPpi,
                                     (BOOLEAN) (Uuid == NULL),
                                     (EFI_SMBUS_UDID *) Uuid,
                                     &SmbusDeviceAddress
                                     );
}

/**
  Retrieves the mapping of all the SMBus devices.

  The GetArpMap() function returns the mapping of all the SMBus devices 
  that are enumerated by the SMBus host driver. 
 
  @param  Length              Size of the buffer that contains the SMBus device map.
  @param  SmbusDeviceMap      The pointer to the device map as enumerated by the SMBus controller driver.

  @retval RETURN_SUCCESS      The SMBUS command was executed.
  @retval RETURN_TIMEOUT      A timeout occurred while executing the SMBUS command.
  @retval RETURN_DEVICE_ERROR The request was not completed because a failure reflected
                              in the Host Status Register bit.
                              Device errors are a result of a transaction collision, illegal command field,
                              unclaimed cycle (host initiated), or bus errors (collisions).

**/
RETURN_STATUS
InternalGetArpMap (
  OUT UINTN                         *Length,
  OUT EFI_SMBUS_DEVICE_MAP          **SmbusDeviceMap
  )
{
  EFI_PEI_SMBUS_PPI         *SmbusPpi;
  EFI_PEI_SERVICES          **PeiServices;

  PeiServices  = GetPeiServicesTablePointer ();
  SmbusPpi     = InternalGetSmbusPpi (PeiServices);
  
  return (RETURN_STATUS) SmbusPpi->GetArpMap (PeiServices, SmbusPpi, Length, SmbusDeviceMap);
}
