/** @file

  The internal header file includes the common header files, defines
  internal functions used by WatchDogTimer module.  

Copyright (c) 2006 - 2008, Intel Corporation                                                         
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution.  The full text of the license may be found at        
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

**/

#ifndef _WATCHDOG_TIMER_H_
#define _WATCHDOG_TIMER_H_



#include <PiDxe.h>
#include <Library/DebugLib.h>
#include <Library/UefiDriverEntryPoint.h>
#include <Library/ReportStatusCodeLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiRuntimeServicesTableLib.h>
#include <Library/PcdLib.h>
#include <Protocol/WatchdogTimer.h>


/**
  This function registers a handler that is to be invoked when the watchdog
  timer fires.  By default, the EFI_WATCHDOG_TIMER protocol will call the
  Runtime Service ResetSystem() when the watchdog timer fires.  If a
  NotifyFunction is registered, then the NotifyFunction will be called before
  the Runtime Service ResetSystem() is called.  If NotifyFunction is NULL, then
  the watchdog handler is unregistered.  If a watchdog handler is registered,
  then EFI_SUCCESS is returned.  If an attempt is made to register a handler
  when a handler is already registered, then EFI_ALREADY_STARTED is returned.
  If an attempt is made to uninstall a handler when a handler is not installed,
  then return EFI_INVALID_PARAMETER.

  @param  This                  The EFI_WATCHDOG_TIMER_ARCH_PROTOCOL instance.
  @param  NotifyFunction        The function to call when the watchdog timer fires.  If this
                                is NULL, then the handler will be unregistered.

  @return EFI_SUCCESS           The watchdog timer handler was registered or unregistered.
  @return EFI_ALREADY_STARTED   NotifyFunction is not NULL, and a handler is already registered.
  @return EFI_INVALID_PARAMETER NotifyFunction is NULL, and a handler was not previously registered.

**/
EFI_STATUS
EFIAPI
WatchdogTimerDriverRegisterHandler (
  IN EFI_WATCHDOG_TIMER_ARCH_PROTOCOL  *This,
  IN EFI_WATCHDOG_TIMER_NOTIFY         NotifyFunction
  );

/**
  This function sets the amount of time to wait before firing the watchdog
  timer to TimerPeriod 100 nS units.  If TimerPeriod is 0, then the watchdog
  timer is disabled.

  @param  This              The EFI_WATCHDOG_TIMER_ARCH_PROTOCOL instance.
  @param  TimerPeriod       The amount of time in 100 nS units to wait before the watchdog
                            timer is fired.  If TimerPeriod is zero, then the watchdog
                            timer is disabled.

  @return EFI_SUCCESS       The watchdog timer has been programmed to fire in Time
                            100 nS units.
  @return EFI_DEVICE_ERROR  A watchdog timer could not be programmed due to a device
                            error.

**/
EFI_STATUS
EFIAPI
WatchdogTimerDriverSetTimerPeriod (
  IN EFI_WATCHDOG_TIMER_ARCH_PROTOCOL  *This,
  IN UINT64                            TimerPeriod
  );

/**
  This function retrieves the amount of time the system will wait before firing
  the watchdog timer.  This period is returned in TimerPeriod, and EFI_SUCCESS
  is returned.  If TimerPeriod is NULL, then EFI_INVALID_PARAMETER is returned.

  @param  This                    The EFI_WATCHDOG_TIMER_ARCH_PROTOCOL instance.
  @param  TimerPeriod             A pointer to the amount of time in 100 nS units that the system
                                  will wait before the watchdog timer is fired.  If TimerPeriod of
                                  zero is returned, then the watchdog timer is disabled.

  @return EFI_SUCCESS             The amount of time that the system will wait before
                                  firing the watchdog timer was returned in TimerPeriod.
  @return EFI_INVALID_PARAMETER   TimerPeriod is NULL.

**/
EFI_STATUS
EFIAPI
WatchdogTimerDriverGetTimerPeriod (
  IN EFI_WATCHDOG_TIMER_ARCH_PROTOCOL  *This,
  IN UINT64                            *TimerPeriod
  );

/**
  Initialize the Watchdog Timer Architectural Protocol driver.

  @param  ImageHandle             ImageHandle of the loaded driver.
  @param  SystemTable             Pointer to the System Table.

  @return EFI_SUCCESS             Timer Architectural Protocol created.

**/
EFI_STATUS
EFIAPI
WatchdogTimerDriverInitialize (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  );

#endif
