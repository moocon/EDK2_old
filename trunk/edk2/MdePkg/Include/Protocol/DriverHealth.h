/** @file
  EFI Driver Health Protocol

  Copyright (c) 2006 - 2008, Intel Corporation                                                         
  All rights reserved. This program and the accompanying materials                          
  are licensed and made available under the terms and conditions of the BSD License         
  which accompanies this distribution.  The full text of the license may be found at        
  http://opensource.org/licenses/bsd-license.php                                            

  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

**/

#ifndef __EFI_DRIVER_HEALTH_H__
#define __EFI_DRIVER_HEALTH_H__

#define EFI_DRIVER_HEALTH_PROTOCOL_GUID \
  { \
    0x2a534210, 0x9280, 0x41d8, { 0xae, 0x79, 0xca, 0xda, 0x1, 0xa2, 0xb1, 0x27 } \
  }
  
typedef struct _EFI_DRIVER_HEALTH_PROTOCOL  EFI_DRIVER_HEALTH_PROTOCOL;

//
// EFI_DRIVER_HEALTH_HEALTH_STATUS
//
typedef enum {
  EfiDriverHealthStatusHealthy,
  EfiDriverHealthStatusRepairRequired,
  EfiDriverHealthStatusConfigurationRequired,
  EfiDriverHealthStatusFailed,
  EfiDriverHealthStatusReconnectRequired,
  EfiDriverHealthStatusRebootRequired
} EFI_DRIVER_HEALTH_HEALTH_STATUS;

//
// EFI_DRIVER_HEALTH_HII_MESSAGE
//
typedef struct {
  EFI_HII_HANDLE  HiiHandle;
  EFI_STRING_ID   StringId;
  UINT64          Reserved;
} EFI_DRIVER_HEALTH_HII_MESSAGE;

/**
  Reports the progress of a repair operation

  @param  Value            A value between 0 and Limit that identifies the current 
                           progress of the repair operation.
  
  @param  Limit            The maximum value of Value for the current repair operation.
                           For example, a driver that wants to specify progress in 
                           percent would use a Limit value of 100.
**/
typedef
VOID
(EFIAPI *EFI_DRIVER_HEALTH_REPAIR_PROGRESS_NOTIFY)(
  IN UINTN  Value,
  IN UINTN  Limit
  );

/**
  Retrieves the health status of a controller in the platform.  This function can also 
  optionally return warning messages, error messages, and a set of HII Forms that may 
  be repair a controller that is not proper configured. 
  
  @param  This             A pointer to the EFI_DRIVER_HEALTH_PROTOCOL instance.

  @param  ControllerHandle The handle of the controller to retrieve the health status 
                           on.  This is an optional parameter that may be NULL.  If 
                           this parameter is NULL, then the value of ChildHandle is 
                           ignored, and the combined health status of all the devices 
                           that the driver is managing is returned.

  @param  ChildHandle      The handle of the child controller to retrieve the health 
                           status on.  This is an optional parameter that may be NULL.  
                           This parameter is ignored of ControllerHandle is NULL.  It 
                           will be NULL for device drivers.  It will also be NULL for 
                           bus drivers when an attempt is made to collect the health 
                           status of the bus controller.  If will not be NULL when an 
                           attempt is made to collect the health status for a child 
                           controller produced by the driver.

  @param  HealthStatus     A pointer to the health status that is returned by this 
                           function.  This is an optional parameter that may be NULL.  
                           This parameter is ignored of ControllerHandle is NULL.  
                           The health status for the controller specified by 
                           ControllerHandle and ChildHandle is returned. 

  @param  MessageList      A pointer to an array of warning or error messages associated 
                           with the controller specified by ControllerHandle and 
                           ChildHandle.  This is an optional parameter that may be NULL.  
                           MessageList is allocated by this function with the EFI Boot 
                           Service AllocatePool(), and it is the caller's responsibility 
                           to free MessageList with the EFI Boot Service FreePool().  
                           Each message is specified by tuple of an EFI_HII_HANDLE and 
                           an EFI_STRING_ID.  The array of messages is terminated by tuple 
                           containing a EFI_HII_HANDLE with a value of NULL.  The 
                           EFI_HII_STRING_PROTOCOL.GetString() function can be used to 
                           retrieve the warning or error message as a Null-terminated 
                           Unicode string in a specific language.  Messages may be 
                           returned for any of the HealthStatus values except 
                           EfiDriverHealthStatusReconnectRequired and 
                           EfiDriverHealthStatusRebootRequired.

  @param  FormHiiHandle    A pointer to the HII handle for an HII form associated with the 
                           controller specified by ControllerHandle and ChildHandle.  
                           This is an optional parameter that may be NULL.  An HII form 
                           is specified by a combination of an EFI_HII_HANDLE and an 
                           EFI_GUID that identifies the Form Set GUID.  The 
                           EFI_FORM_BROWSER2_PROTOCOL.SendForm() function can be used 
                           to display and allow the user to make configuration changes 
                           to the HII Form.  An HII form may only be returned with a 
                           HealthStatus value of EfiDriverHealthStatusConfigurationRequired.

  @param  FormSetGuid      A pointer to the GUID for an HII form associated with the 
                           controller specified by ControllerHandle and ChildHandle.  
                           This is an optional parameter that may be NULL.  An HII form 
                           is specified by a combination of an EFI_HII_HANDLE and an 
                           EFI_GUID that identifies the Form Set GUID.  The 
                           EFI_FORM_BROWSER2_PROTOCOL.SendForm() function can be used 
                           to display and allow the user to make configuration changes 
                           to the HII Form.  An HII form may only be returned with a 
                           HealthStatus value of EfiDriverHealthStatusConfigurationRequired.

  @retval EFI_SUCCESS           ControllerHandle is NULL, and all the controllers 
                                managed by this driver specified by This have a health 
                                status of EfiDriverHealthStatusHealthy with no warning 
                                messages to be returned.  The ChildHandle, HealthStatus, 
                                MessageList, and FormList parameters are ignored.

  @retval EFI_DEVICE_ERROR      ControllerHandle is NULL, and one or more of the 
                                controllers managed by this driver specified by This 
                                do not have a health status of EfiDriverHealthStatusHealthy.  
                                The ChildHandle, HealthStatus, MessageList, and 
                                FormList parameters are ignored.

  @retval EFI_DEVICE_ERROR      ControllerHandle is NULL, and one or more of the 
                                controllers managed by this driver specified by This 
                                have one or more warning and/or error messages.  
                                The ChildHandle, HealthStatus, MessageList, and 
                                FormList parameters are ignored.

  @retval EFI_SUCCESS           ControllerHandle is not NULL and the health status 
                                of the controller specified by ControllerHandle and 
                                ChildHandle was returned in HealthStatus.  A list 
                                of warning and error messages may be optionally 
                                returned in MessageList, and a list of HII Forms 
                                may be optionally returned in FormList.

  @retval EFI_UNSUPPORTED	      ControllerHandle is not NULL, and the controller 
                                specified by ControllerHandle and ChildHandle is not 
                                currently being managed by the driver specified by This.

  @retval EFI_INVALID_PARAMETER	HealthStatus is NULL.

  @retval EFI_OUT_OF_RESOURCES	MessageList is not NULL, and there are not enough 
                                resource available to allocate memory for MessageList.

**/
typedef
EFI_STATUS
(EFIAPI *EFI_DRIVER_HEALTH_GET_HEALTH_STATUS)(
  IN  EFI_DRIVER_HEALTH_PROTOCOL       *This,
  IN  EFI_HANDLE                       ControllerHandle  OPTIONAL,
  IN  EFI_HANDLE                       ChildHandle       OPTIONAL,
  OUT EFI_DRIVER_HEALTH_HEALTH_STATUS  *HealthStatus,
  OUT EFI_DRIVER_HEALTH_HII_MESSAGE    **MessageList     OPTIONAL,
  OUT EFI_HII_HANDLE                   *FormHiiHandle    OPTIONAL,
  OUT EFI_GUID                         **FormSetGuid     OPTIONAL
  );

/**
  Performs a repair operation on a controller in the platform.  This function can 
  optionally report repair progress information back to the platform. 
  
  @param  This             A pointer to the EFI_DRIVER_HEALTH_PROTOCOL instance.
  @param  ControllerHandle The handle of the controller to repair.
  @param  ChildHandle      The handle of the child controller to repair.  This is 
                           an optional parameter that may be NULL.  It will be NULL 
                           for device drivers.  It will also be NULL for bus 
                           drivers when an attempt is made to repair a bus controller.
                           If will not be NULL when an attempt is made to repair a 
                           child controller produced by the driver.
  @param  ProgressNotification
                           A notification function that may be used by a driver to 
                           report the progress of the repair operation.  This is 
                           an optional parameter that may be NULL.  


  @retval EFI_SUCCESS	          An attempt to repair the controller specified by 
                                ControllerHandle and ChildHandle was performed.  
                                The result of the repair operation can bet 
                                determined by calling GetHealthStatus().
  @retval EFI_UNSUPPORTED	      The driver specified by This is not currently 
                                managing the controller specified by ControllerHandle 
                                and ChildHandle.
  @retval EFI_OUT_OF_RESOURCES	There are not enough resources to perform the 
                                repair operation.

*/
typedef
EFI_STATUS
(EFIAPI *EFI_DRIVER_HEALTH_REPAIR)(
  IN  EFI_DRIVER_HEALTH_PROTOCOL                *This,
  IN  EFI_HANDLE                                ControllerHandle,
  IN  EFI_HANDLE                                ChildHandle           OPTIONAL,
  IN  EFI_DRIVER_HEALTH_REPAIR_PROGRESS_NOTIFY  ProgressNotification  OPTIONAL
  );

/**
  When installed, the Driver Health Protocol produces a collection of services 
  that allow the health status for a controller to be retrieved.  If a controller 
  is not in a usable state, status messages may be reported to the user, repair 
  operations can be invoked, and the user may be asked to make software and/or 
  hardware configuration changes. 

  @par Protocol Description:
  The Driver Health Protocol is optionally produced by a driver that follows the 
  EFI Driver Model.  If an EFI Driver needs to report health status to the platform, 
  provide warning or error messages to the user, perform length repair operations, 
  or request the user to make hardware or software configuration changes, then the 
  Driver Health Protocol must be produced.
  
  A controller that is managed by driver that follows the EFI Driver Model and 
  produces the Driver Health Protocol must report the current health of the 
  controllers that the driver is currently managing.  The controller can initially 
  be healthy, failed, require repair, or require configuration.  If a controller 
  requires configuration, and the user make configuration changes, the controller 
  may then need to be reconnected or the system may need to be rebooted for the 
  configuration changes to take affect.  Figure 2-1 below shows all the possible 
  health states of a controller and the legal transitions between the health states. 

  @param GetHealthStatus     Retrieves the health status of a controller in the 
                             platform.  This function can also optionally return 
                             warning messages, error messages, and a set of HII 
                             Forms that may be repair a controller that is not 
                             properly configured.
  @param Repair              Performs a repair operation on a controller in the 
                             platform.  This function can optionally report repair 
                             progress information back to the platform.

**/
struct _EFI_DRIVER_HEALTH_PROTOCOL {
  EFI_DRIVER_HEALTH_GET_HEALTH_STATUS  GetHealthStatus;
  EFI_DRIVER_HEALTH_REPAIR             Repair;
};

extern EFI_GUID gEfiDriverHealthProtocolGuid;

#endif




