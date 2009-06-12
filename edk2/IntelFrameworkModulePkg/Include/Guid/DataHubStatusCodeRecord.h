/** @file
  GUID used to identify Data Hub records logged by Status Code Runtime Protocol.
  
Copyright (c) 2006 - 2009, Intel Corporation                                                         
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution.  The full text of the license may be found at        
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

**/


#ifndef __DATA_HUB_STATUS_CODE_RECORD_H__
#define __DATA_HUB_STATUS_CODE_RECORD_H__

#define EFI_DATA_HUB_STATUS_CODE_RECORD_GUID \
  { \
    0xd083e94c, 0x6560, 0x42e4, {0xb6, 0xd4, 0x2d, 0xf7, 0x5a, 0xdf, 0x6a, 0x2a } \
  }

typedef struct {
  ///
  /// Status Code type to be reported.
  ///
  EFI_STATUS_CODE_TYPE  CodeType;

  ///
  /// Valu information about the class and subclass is used to
  /// classify the hardware and software entity as well as an operation.
  ///
  EFI_STATUS_CODE_VALUE Value;

  ///
  /// The enumeration of a hardware or software entity within
  /// the system. Valid instance numbers start with 1
  ///
  UINT32                Instance;

  ///
  /// Identify the caller.
  ///
  EFI_GUID              CallerId;

  ///
  /// Additional status code data
  ///
  EFI_STATUS_CODE_DATA  Data;
} DATA_HUB_STATUS_CODE_DATA_RECORD;

extern EFI_GUID gEfiDataHubStatusCodeRecordGuid;

#endif
