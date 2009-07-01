/** @file
  Provides the parent dispatch service for the standby button SMI source generator.

  The SMM Standby Button Dispatch Protocol is defined in 
  the Intel Platform Innovation Framework for EFI SMM Core Interface Specification
  (SMM CIS) Version 0.9.

  Copyright (c) 2007, 2009 Intel Corporation
  All rights reserved. This program and the accompanying materials
  are licensed and made available under the terms and conditions of the BSD License
  which accompanies this distribution.  The full text of the license may be found at
  http://opensource.org/licenses/bsd-license.php

  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

  @par Revision Reference:
  This Protocol is defined in Framework of EFI SMM Core Interface Spec
  Version 0.9.

**/

#ifndef _EFI_SMM_STANDBY_BUTTON_DISPATCH_H_
#define _EFI_SMM_STANDBY_BUTTON_DISPATCH_H_

#include <FrameworkSmm.h>

//
// Global ID for the Standby Button SMI Protocol
//
#define EFI_SMM_STANDBY_BUTTON_DISPATCH_PROTOCOL_GUID \
  { \
    0x78965b98, 0xb0bf, 0x449e, {0x8b, 0x22, 0xd2, 0x91, 0x4e, 0x49, 0x8a, 0x98 } \
  }

typedef struct _EFI_SMM_STANDBY_BUTTON_DISPATCH_PROTOCOL  EFI_SMM_STANDBY_BUTTON_DISPATCH_PROTOCOL;

//
// Related Definitions
//

///
/// Standby Button. Example, Use for changing LEDs before ACPI OS is on.
///    - DXE/BDS Phase
///    - OS Install Phase
///
typedef enum {
  EfiStandbyButtonEntry,
  EfiStandbyButtonExit
} EFI_STANDBY_BUTTON_PHASE;

typedef struct {
  ///  Describes whether the child handler should be invoked upon the entry to the button
  ///  activation or upon exit (i.e., upon receipt of the button press event or upon release of
  ///  the event).
  EFI_STANDBY_BUTTON_PHASE  Phase;
} EFI_SMM_STANDBY_BUTTON_DISPATCH_CONTEXT;

//
// Member functions
//

/**
  Dispatch function for a Standby Button SMI handler.

  @param  DispatchHandle        Handle of this dispatch function.
  @param  DispatchContext       Pointer to the dispatch function's context.
                                The DispatchContext fields are filled in
                                by the dispatching driver prior to
                                invoking this dispatch function.

**/
typedef
VOID
(EFIAPI *EFI_SMM_STANDBY_BUTTON_DISPATCH)(
  IN  EFI_HANDLE                                DispatchHandle,
  IN  EFI_SMM_STANDBY_BUTTON_DISPATCH_CONTEXT   *DispatchContext
  );

/**
  Provides the parent dispatch service for a given SMI source generator

  @param  This                  Pointer to the EFI_SMM_STANDBY_BUTTON_DISPATCH_PROTOCOL instance.
  @param  DispatchFunction      Function to install.
  @param  DispatchContext       Pointer to the dispatch function's context.
                                The caller fills this context in before calling
                                the register function to indicate to the register
                                function the Standby Button SMI phase for which the dispatch
                                function should be invoked.
  @param  DispatchHandle        Handle generated by the dispatcher to track the function instance.

  @retval EFI_SUCCESS           The dispatch function has been successfully
                                registered and the SMI source has been enabled.
  @retval EFI_DEVICE_ERROR      The driver was unable to enable the SMI source.
  @retval EFI_OUT_OF_RESOURCES  Not enough memory (system or SMM) to manage this
                                child.
  @retval EFI_INVALID_PARAMETER DispatchContext is invalid. The Standby Button SMI
                                phase is not within valid range.

**/
typedef
EFI_STATUS
(EFIAPI *EFI_SMM_STANDBY_BUTTON_REGISTER)(
  IN EFI_SMM_STANDBY_BUTTON_DISPATCH_PROTOCOL          *This,
  IN EFI_SMM_STANDBY_BUTTON_DISPATCH                   DispatchFunction,
  IN EFI_SMM_STANDBY_BUTTON_DISPATCH_CONTEXT           *DispatchContext,
  OUT EFI_HANDLE                                       *DispatchHandle
  );

/**
  Unregister a child SMI source dispatch function with a parent SMM driver

  @param  This                   Pointer to the EFI_SMM_STANDBY_BUTTON_DISPATCH_PROTOCOL instance.
  @param  DispatchHandle         Handle of the service to remove.

  @retval EFI_SUCCESS            The dispatch function has been successfully
                                 unregistered and the SMI source has been disabled
                                 if there are no other registered child dispatch
                                 functions for this SMI source.
  @retval EFI_INVALID_PARAMETER  Handle is invalid.

**/
typedef
EFI_STATUS
(EFIAPI *EFI_SMM_STANDBY_BUTTON_UNREGISTER)(
  IN EFI_SMM_STANDBY_BUTTON_DISPATCH_PROTOCOL  *This,
  IN EFI_HANDLE                                DispatchHandle
  );

//
// Interface structure for the SMM Standby Button SMI Dispatch Protocol
//
/**
	This protocol  provices the parent dispatch service for the standby button SMI source generator.
	Provides the ability to install child handlers for the given event types.
 **/
struct _EFI_SMM_STANDBY_BUTTON_DISPATCH_PROTOCOL {
	/// 
  /// Installs a child service to be dispatched by this protocol.
  ///
  EFI_SMM_STANDBY_BUTTON_REGISTER   Register;\
	///
	/// Removes a child service dispatched by this protocol.
	///
  EFI_SMM_STANDBY_BUTTON_UNREGISTER UnRegister;
};

extern EFI_GUID gEfiSmmStandbyButtonDispatchProtocolGuid;

#endif
