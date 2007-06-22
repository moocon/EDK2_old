/** @file
  Library functions that abstract driver model protocols
  installation.

  Copyright (c) 2006 - 2007, Intel Corporation<BR> All rights
  reserved. This program and the accompanying materials are
  licensed and made available under the terms and conditions of the BSD License
  which accompanies this distribution.  The full text of the license may be found at
  http://opensource.org/licenses/bsd-license.php
  
  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/ 

//
// Include common header file for this module.
//
#include "CommonHeader.h"

/**
  Intialize a driver by installing the Driver Binding Protocol onto the
  driver's DriverBindingHandle.  This is typically the same as the driver's
  ImageHandle, but it can be different if the driver produces multiple
  DriverBinding Protocols.  This function also initializes the EFI Driver
  Library that initializes the global variables gST, gBS, gRT.

  @param  ImageHandle          The image handle of the driver
  @param  SystemTable          The EFI System Table that was passed to the driver's entry point
  @param  DriverBinding        A Driver Binding Protocol instance that this driver is producing
  @param  DriverBindingHandle  The handle that DriverBinding is to be installe onto.  If this
                               parameter is NULL, then a new handle is created.

  @retval EFI_SUCCESS          DriverBinding is installed onto DriverBindingHandle
  @retval Other                Status from gBS->InstallProtocolInterface()

**/
EFI_STATUS
EFIAPI
EfiLibInstallDriverBinding (
  IN const EFI_HANDLE             ImageHandle,
  IN const EFI_SYSTEM_TABLE       *SystemTable,
  IN EFI_DRIVER_BINDING_PROTOCOL  *DriverBinding,
  IN EFI_HANDLE                   DriverBindingHandle
  )
{
  EFI_STATUS  Status;

  ASSERT (NULL != DriverBinding);

  Status = gBS->InstallMultipleProtocolInterfaces (
                  &DriverBindingHandle,
                  &gEfiDriverBindingProtocolGuid, DriverBinding,
                  NULL
                  );
  //
  // ASSERT if the call to InstallMultipleProtocolInterfaces() failed
  //
  ASSERT_EFI_ERROR (Status);

  //
  // Update the ImageHandle and DriverBindingHandle fields of the Driver Binding Protocol
  //
  DriverBinding->ImageHandle         = ImageHandle;
  DriverBinding->DriverBindingHandle = DriverBindingHandle;

  return Status;
}

/**
  Intialize a driver by installing the Driver Binding Protocol onto the
  driver's DriverBindingHandle.  This is typically the same as the driver's
  ImageHandle, but it can be different if the driver produces multiple
  DriverBinding Protocols.  This function also initializes the EFI Driver
  Library that initializes the global variables gST, gBS, gRT.

  @ImageHandle                 The image handle of the driver
  @SystemTable                 The EFI System Table that was passed to the driver's entry point
  @DriverBinding               A Driver Binding Protocol instance that this driver is producing
  @DriverBindingHandle         The handle that DriverBinding is to be installe onto.  If this
                               parameter is NULL, then a new handle is created.
  @ComponentName               A Component Name Protocol instance that this driver is producing
  @DriverConfiguration         A Driver Configuration Protocol instance that this driver is producing
  @DriverDiagnostics           A Driver Diagnostics Protocol instance that this driver is producing

  @retval EFI_SUCCESS          DriverBinding is installed onto DriverBindingHandle
  @retval Other                Status from gBS->InstallProtocolInterface()

**/
EFI_STATUS
EFIAPI
EfiLibInstallAllDriverProtocols (
  IN const EFI_HANDLE                         ImageHandle,
  IN const EFI_SYSTEM_TABLE                   *SystemTable,
  IN EFI_DRIVER_BINDING_PROTOCOL              *DriverBinding,
  IN EFI_HANDLE                               DriverBindingHandle,
  IN const EFI_COMPONENT_NAME_PROTOCOL        *ComponentName,       OPTIONAL
  IN const EFI_DRIVER_CONFIGURATION_PROTOCOL  *DriverConfiguration, OPTIONAL
  IN const EFI_DRIVER_DIAGNOSTICS_PROTOCOL    *DriverDiagnostics    OPTIONAL
  )
{
  EFI_STATUS  Status;

  ASSERT (NULL != DriverBinding);

  if (DriverDiagnostics == NULL || FeaturePcdGet(PcdDriverDiagnosticsDisable)) {
    if (DriverConfiguration == NULL) {
      if (ComponentName == NULL || FeaturePcdGet(PcdComponentNameDisable)) {
        Status = gBS->InstallMultipleProtocolInterfaces (
                        &DriverBindingHandle,
                        &gEfiDriverBindingProtocolGuid, DriverBinding,
                        NULL
                        );
      } else {
        Status = gBS->InstallMultipleProtocolInterfaces (
                        &DriverBindingHandle,
                        &gEfiDriverBindingProtocolGuid, DriverBinding,
                        &gEfiComponentNameProtocolGuid, ComponentName,
                        NULL
                        );
      }
    } else {
      if (ComponentName == NULL || FeaturePcdGet(PcdComponentNameDisable)) {
        Status = gBS->InstallMultipleProtocolInterfaces (
                        &DriverBindingHandle,
                        &gEfiDriverBindingProtocolGuid,       DriverBinding,
                        &gEfiDriverConfigurationProtocolGuid, DriverConfiguration,
                        NULL
                        );
      } else {
        Status = gBS->InstallMultipleProtocolInterfaces (
                        &DriverBindingHandle,
                        &gEfiDriverBindingProtocolGuid,       DriverBinding,
                        &gEfiComponentNameProtocolGuid,       ComponentName,
                        &gEfiDriverConfigurationProtocolGuid, DriverConfiguration,
                        NULL
                        );
      }
    }
  } else {
    if (DriverConfiguration == NULL) {
      if (ComponentName == NULL || FeaturePcdGet(PcdComponentNameDisable)) {
        Status = gBS->InstallMultipleProtocolInterfaces (
                        &DriverBindingHandle,
                        &gEfiDriverBindingProtocolGuid,     DriverBinding,
                        &gEfiDriverDiagnosticsProtocolGuid, DriverDiagnostics,
                        NULL
                        );
      } else {
        Status = gBS->InstallMultipleProtocolInterfaces (
                        &DriverBindingHandle,
                        &gEfiDriverBindingProtocolGuid,     DriverBinding,
                        &gEfiComponentNameProtocolGuid,     ComponentName,
                        &gEfiDriverDiagnosticsProtocolGuid, DriverDiagnostics,
                        NULL
                        );
      }
    } else {
      if (ComponentName == NULL || FeaturePcdGet(PcdComponentNameDisable)) {
       Status = gBS->InstallMultipleProtocolInterfaces (
                        &DriverBindingHandle,
                        &gEfiDriverBindingProtocolGuid,       DriverBinding,
                        &gEfiDriverConfigurationProtocolGuid, DriverConfiguration,
                        &gEfiDriverDiagnosticsProtocolGuid,   DriverDiagnostics,
                        NULL
                        );
      } else {
        Status = gBS->InstallMultipleProtocolInterfaces (
                        &DriverBindingHandle,
                        &gEfiDriverBindingProtocolGuid,       DriverBinding,
                        &gEfiComponentNameProtocolGuid,       ComponentName,
                        &gEfiDriverConfigurationProtocolGuid, DriverConfiguration,
                        &gEfiDriverDiagnosticsProtocolGuid,   DriverDiagnostics,
                        NULL
                        );
      }
    }
  }

  //
  // ASSERT if the call to InstallMultipleProtocolInterfaces() failed
  //
  ASSERT_EFI_ERROR (Status);

  //
  // Update the ImageHandle and DriverBindingHandle fields of the Driver Binding Protocol
  //
  DriverBinding->ImageHandle         = ImageHandle;
  DriverBinding->DriverBindingHandle = DriverBindingHandle;

  return Status;
}


/**
  Intialize a driver by installing the Driver Binding Protocol onto the
  driver's DriverBindingHandle.  This is typically the same as the driver's
  ImageHandle, but it can be different if the driver produces multiple
  DriverBinding Protocols.  This function also initializes the EFI Driver
  Library that initializes the global variables gST, gBS, gRT.

  @ImageHandle                 The image handle of the driver
  @SystemTable                 The EFI System Table that was passed to the driver's entry point
  @DriverBinding               A Driver Binding Protocol instance that this driver is producing
  @DriverBindingHandle         The handle that DriverBinding is to be installe onto.  If this
                               parameter is NULL, then a new handle is created.
  @ComponentName               A Component Name Protocol instance that this driver is producing
  @DriverConfiguration         A Driver Configuration Protocol instance that this driver is producing
  @DriverDiagnostics           A Driver Diagnostics Protocol instance that this driver is producing

  @retval EFI_SUCCESS          DriverBinding is installed onto DriverBindingHandle
  @retval Other                Status from gBS->InstallProtocolInterface()

**/
EFI_STATUS
EFIAPI
EfiLibInstallAllDriverProtocols2 (
  IN const EFI_HANDLE                         ImageHandle,
  IN const EFI_SYSTEM_TABLE                   *SystemTable,
  IN EFI_DRIVER_BINDING_PROTOCOL              *DriverBinding,
  IN EFI_HANDLE                               DriverBindingHandle,
  IN const EFI_COMPONENT_NAME_PROTOCOL        *ComponentName,       OPTIONAL
  IN const EFI_COMPONENT_NAME2_PROTOCOL       *ComponentName2,      OPTIONAL
  IN const EFI_DRIVER_CONFIGURATION_PROTOCOL  *DriverConfiguration, OPTIONAL
  IN const EFI_DRIVER_DIAGNOSTICS_PROTOCOL    *DriverDiagnostics,   OPTIONAL
  IN const EFI_DRIVER_DIAGNOSTICS2_PROTOCOL   *DriverDiagnostics2   OPTIONAL
  )
{
  EFI_STATUS  Status;

  ASSERT (NULL != DriverBinding);

  if (DriverConfiguration == NULL) {
    if (DriverDiagnostics == NULL || FeaturePcdGet(PcdDriverDiagnosticsDisable)) {
      if (DriverDiagnostics2 == NULL || FeaturePcdGet(PcdDriverDiagnostics2Disable)) {
        if (ComponentName == NULL || FeaturePcdGet(PcdComponentNameDisable)) {
          if (ComponentName2 == NULL || FeaturePcdGet(PcdComponentName2Disable)) {
            Status = gBS->InstallMultipleProtocolInterfaces (
                            &DriverBindingHandle,
                            &gEfiDriverBindingProtocolGuid, DriverBinding,
                            NULL
                            );
          } else {
            Status = gBS->InstallMultipleProtocolInterfaces (
                            &DriverBindingHandle,
                            &gEfiDriverBindingProtocolGuid, DriverBinding,
                            &gEfiComponentName2ProtocolGuid, ComponentName2,
                            NULL
                            );
          }
        } else {
          if (ComponentName2 == NULL || FeaturePcdGet(PcdComponentName2Disable)) {
            Status = gBS->InstallMultipleProtocolInterfaces (
                            &DriverBindingHandle,
                            &gEfiDriverBindingProtocolGuid, DriverBinding,
                            &gEfiComponentNameProtocolGuid, ComponentName,
                            NULL
                            );
          } else {
            Status = gBS->InstallMultipleProtocolInterfaces (
                            &DriverBindingHandle,
                            &gEfiDriverBindingProtocolGuid, DriverBinding,
                            &gEfiComponentNameProtocolGuid, ComponentName,
                            &gEfiComponentName2ProtocolGuid, ComponentName2,
                            NULL
                            );
          }
        }
      } else {
        if (ComponentName == NULL || FeaturePcdGet(PcdComponentNameDisable)) {
          if (ComponentName2 == NULL || FeaturePcdGet(PcdComponentName2Disable)) {
            Status = gBS->InstallMultipleProtocolInterfaces (
                            &DriverBindingHandle,
                            &gEfiDriverBindingProtocolGuid, DriverBinding,
                            &gEfiDriverDiagnostics2ProtocolGuid, DriverDiagnostics2,
                            NULL
                            );
          } else {
            Status = gBS->InstallMultipleProtocolInterfaces (
                            &DriverBindingHandle,
                            &gEfiDriverBindingProtocolGuid, DriverBinding,
                            &gEfiComponentName2ProtocolGuid, ComponentName2,
                            &gEfiDriverDiagnostics2ProtocolGuid, DriverDiagnostics2,
                            NULL
                            );
          }
        } else {
          if (ComponentName2 == NULL || FeaturePcdGet(PcdComponentName2Disable)) {
            Status = gBS->InstallMultipleProtocolInterfaces (
                            &DriverBindingHandle,
                            &gEfiDriverBindingProtocolGuid, DriverBinding,
                            &gEfiComponentNameProtocolGuid, ComponentName,
                            &gEfiDriverDiagnostics2ProtocolGuid, DriverDiagnostics2,
                            NULL
                            );
          } else {
            Status = gBS->InstallMultipleProtocolInterfaces (
                            &DriverBindingHandle,
                            &gEfiDriverBindingProtocolGuid, DriverBinding,
                            &gEfiComponentNameProtocolGuid, ComponentName,
                            &gEfiComponentName2ProtocolGuid, ComponentName2,
                            &gEfiDriverDiagnostics2ProtocolGuid, DriverDiagnostics2,
                            NULL
                            );
          }
        }
      }
    } else {
      if (DriverDiagnostics2 == NULL || FeaturePcdGet(PcdDriverDiagnostics2Disable)) {
        if (ComponentName == NULL || FeaturePcdGet(PcdComponentNameDisable)) {
          if (ComponentName2 == NULL || FeaturePcdGet(PcdComponentName2Disable)) {
            Status = gBS->InstallMultipleProtocolInterfaces (
                            &DriverBindingHandle,
                            &gEfiDriverBindingProtocolGuid, DriverBinding,
                            &gEfiDriverDiagnosticsProtocolGuid, DriverDiagnostics,
                            NULL
                            );
          } else {
            Status = gBS->InstallMultipleProtocolInterfaces (
                            &DriverBindingHandle,
                            &gEfiDriverBindingProtocolGuid, DriverBinding,
                            &gEfiComponentName2ProtocolGuid, ComponentName2,
                            &gEfiDriverDiagnosticsProtocolGuid, DriverDiagnostics,
                            NULL
                            );
          }
        } else {
          if (ComponentName2 == NULL || FeaturePcdGet(PcdComponentName2Disable)) {
            Status = gBS->InstallMultipleProtocolInterfaces (
                            &DriverBindingHandle,
                            &gEfiDriverBindingProtocolGuid, DriverBinding,
                            &gEfiComponentNameProtocolGuid, ComponentName,
                            &gEfiDriverDiagnosticsProtocolGuid, DriverDiagnostics,
                            NULL
                            );
          } else {
            Status = gBS->InstallMultipleProtocolInterfaces (
                            &DriverBindingHandle,
                            &gEfiDriverBindingProtocolGuid, DriverBinding,
                            &gEfiComponentNameProtocolGuid, ComponentName,
                            &gEfiComponentName2ProtocolGuid, ComponentName2,
                            &gEfiDriverDiagnosticsProtocolGuid, DriverDiagnostics,
                            NULL
                            );
          }
        }
      } else {
        if (ComponentName == NULL || FeaturePcdGet(PcdComponentNameDisable)) {
          if (ComponentName2 == NULL || FeaturePcdGet(PcdComponentName2Disable)) {
            Status = gBS->InstallMultipleProtocolInterfaces (
                            &DriverBindingHandle,
                            &gEfiDriverBindingProtocolGuid, DriverBinding,
                            &gEfiDriverDiagnosticsProtocolGuid, DriverDiagnostics,
                            &gEfiDriverDiagnostics2ProtocolGuid, DriverDiagnostics2,
                            NULL
                            );
          } else {
            Status = gBS->InstallMultipleProtocolInterfaces (
                            &DriverBindingHandle,
                            &gEfiDriverBindingProtocolGuid, DriverBinding,
                            &gEfiComponentName2ProtocolGuid, ComponentName2,
                            &gEfiDriverDiagnosticsProtocolGuid, DriverDiagnostics,
                            &gEfiDriverDiagnostics2ProtocolGuid, DriverDiagnostics2,
                            NULL
                            );
          }
        } else {
          if (ComponentName2 == NULL || FeaturePcdGet(PcdComponentName2Disable)) {
            Status = gBS->InstallMultipleProtocolInterfaces (
                            &DriverBindingHandle,
                            &gEfiDriverBindingProtocolGuid, DriverBinding,
                            &gEfiComponentNameProtocolGuid, ComponentName,
                            &gEfiDriverDiagnosticsProtocolGuid, DriverDiagnostics,
                            &gEfiDriverDiagnostics2ProtocolGuid, DriverDiagnostics2,
                            NULL
                            );
          } else {
            Status = gBS->InstallMultipleProtocolInterfaces (
                            &DriverBindingHandle,
                            &gEfiDriverBindingProtocolGuid, DriverBinding,
                            &gEfiComponentNameProtocolGuid, ComponentName,
                            &gEfiComponentName2ProtocolGuid, ComponentName2,
                            &gEfiDriverDiagnosticsProtocolGuid, DriverDiagnostics,
                            &gEfiDriverDiagnostics2ProtocolGuid, DriverDiagnostics2,
                            NULL
                            );
          }
        }
      }
    }
  } else {
    if (DriverDiagnostics == NULL || FeaturePcdGet(PcdDriverDiagnosticsDisable)) {
      if (DriverDiagnostics2 == NULL || FeaturePcdGet(PcdDriverDiagnostics2Disable)) {
        if (ComponentName == NULL || FeaturePcdGet(PcdComponentNameDisable)) {
          if (ComponentName2 == NULL || FeaturePcdGet(PcdComponentName2Disable)) {
            Status = gBS->InstallMultipleProtocolInterfaces (
                            &DriverBindingHandle,
                            &gEfiDriverBindingProtocolGuid, DriverBinding,
                            &gEfiDriverConfigurationProtocolGuid, DriverConfiguration,
                            NULL
                            );
          } else {
            Status = gBS->InstallMultipleProtocolInterfaces (
                            &DriverBindingHandle,
                            &gEfiDriverBindingProtocolGuid, DriverBinding,
                            &gEfiComponentName2ProtocolGuid, ComponentName2,
                            &gEfiDriverConfigurationProtocolGuid, DriverConfiguration,
                            NULL
                            );
          }
        } else {
          if (ComponentName2 == NULL || FeaturePcdGet(PcdComponentName2Disable)) {
            Status = gBS->InstallMultipleProtocolInterfaces (
                            &DriverBindingHandle,
                            &gEfiDriverBindingProtocolGuid, DriverBinding,
                            &gEfiComponentNameProtocolGuid, ComponentName,
                            &gEfiDriverConfigurationProtocolGuid, DriverConfiguration,
                            NULL
                            );
          } else {
            Status = gBS->InstallMultipleProtocolInterfaces (
                            &DriverBindingHandle,
                            &gEfiDriverBindingProtocolGuid, DriverBinding,
                            &gEfiComponentNameProtocolGuid, ComponentName,
                            &gEfiComponentName2ProtocolGuid, ComponentName2,
                            &gEfiDriverConfigurationProtocolGuid, DriverConfiguration,
                            NULL
                            );
          }
        }
      } else {
        if (ComponentName == NULL || FeaturePcdGet(PcdComponentNameDisable)) {
          if (ComponentName2 == NULL || FeaturePcdGet(PcdComponentName2Disable)) {
            Status = gBS->InstallMultipleProtocolInterfaces (
                            &DriverBindingHandle,
                            &gEfiDriverBindingProtocolGuid, DriverBinding,
                            &gEfiDriverConfigurationProtocolGuid, DriverConfiguration,
                            &gEfiDriverDiagnostics2ProtocolGuid, DriverDiagnostics2,
                            NULL
                            );
          } else {
            Status = gBS->InstallMultipleProtocolInterfaces (
                            &DriverBindingHandle,
                            &gEfiDriverBindingProtocolGuid, DriverBinding,
                            &gEfiComponentName2ProtocolGuid, ComponentName2,
                            &gEfiDriverConfigurationProtocolGuid, DriverConfiguration,
                            &gEfiDriverDiagnostics2ProtocolGuid, DriverDiagnostics2,
                            NULL
                            );
          }
        } else {
          if (ComponentName2 == NULL || FeaturePcdGet(PcdComponentName2Disable)) {
            Status = gBS->InstallMultipleProtocolInterfaces (
                            &DriverBindingHandle,
                            &gEfiDriverBindingProtocolGuid, DriverBinding,
                            &gEfiComponentNameProtocolGuid, ComponentName,
                            &gEfiDriverConfigurationProtocolGuid, DriverConfiguration,
                            &gEfiDriverDiagnostics2ProtocolGuid, DriverDiagnostics2,
                            NULL
                            );
          } else {
            Status = gBS->InstallMultipleProtocolInterfaces (
                            &DriverBindingHandle,
                            &gEfiDriverBindingProtocolGuid, DriverBinding,
                            &gEfiComponentNameProtocolGuid, ComponentName,
                            &gEfiComponentName2ProtocolGuid, ComponentName2,
                            &gEfiDriverConfigurationProtocolGuid, DriverConfiguration,
                            &gEfiDriverDiagnostics2ProtocolGuid, DriverDiagnostics2,
                            NULL
                            );
          }
        }
      }
    } else {
      if (DriverDiagnostics2 == NULL || FeaturePcdGet(PcdDriverDiagnostics2Disable)) {
        if (ComponentName == NULL || FeaturePcdGet(PcdComponentNameDisable)) {
          if (ComponentName2 == NULL || FeaturePcdGet(PcdComponentName2Disable)) {
            Status = gBS->InstallMultipleProtocolInterfaces (
                            &DriverBindingHandle,
                            &gEfiDriverBindingProtocolGuid, DriverBinding,
                            &gEfiDriverConfigurationProtocolGuid, DriverConfiguration,
                            &gEfiDriverDiagnosticsProtocolGuid, DriverDiagnostics,
                            NULL
                            );
          } else {
            Status = gBS->InstallMultipleProtocolInterfaces (
                            &DriverBindingHandle,
                            &gEfiDriverBindingProtocolGuid, DriverBinding,
                            &gEfiComponentName2ProtocolGuid, ComponentName2,
                            &gEfiDriverConfigurationProtocolGuid, DriverConfiguration,
                            &gEfiDriverDiagnosticsProtocolGuid, DriverDiagnostics,
                            NULL
                            );
          }
        } else {
          if (ComponentName2 == NULL || FeaturePcdGet(PcdComponentName2Disable)) {
            Status = gBS->InstallMultipleProtocolInterfaces (
                            &DriverBindingHandle,
                            &gEfiDriverBindingProtocolGuid, DriverBinding,
                            &gEfiComponentNameProtocolGuid, ComponentName,
                            &gEfiDriverConfigurationProtocolGuid, DriverConfiguration,
                            &gEfiDriverDiagnosticsProtocolGuid, DriverDiagnostics,
                            NULL
                            );
          } else {
            Status = gBS->InstallMultipleProtocolInterfaces (
                            &DriverBindingHandle,
                            &gEfiDriverBindingProtocolGuid, DriverBinding,
                            &gEfiComponentNameProtocolGuid, ComponentName,
                            &gEfiComponentName2ProtocolGuid, ComponentName2,
                            &gEfiDriverConfigurationProtocolGuid, DriverConfiguration,
                            &gEfiDriverDiagnosticsProtocolGuid, DriverDiagnostics,
                            NULL
                            );
          }
        }
      } else {
        if (ComponentName == NULL || FeaturePcdGet(PcdComponentNameDisable)) {
          if (ComponentName2 == NULL || FeaturePcdGet(PcdComponentName2Disable)) {
            Status = gBS->InstallMultipleProtocolInterfaces (
                            &DriverBindingHandle,
                            &gEfiDriverBindingProtocolGuid, DriverBinding,
                            &gEfiDriverConfigurationProtocolGuid, DriverConfiguration,
                            &gEfiDriverDiagnosticsProtocolGuid, DriverDiagnostics,
                            &gEfiDriverDiagnostics2ProtocolGuid, DriverDiagnostics2,
                            NULL
                            );
          } else {
            Status = gBS->InstallMultipleProtocolInterfaces (
                            &DriverBindingHandle,
                            &gEfiDriverBindingProtocolGuid, DriverBinding,
                            &gEfiComponentName2ProtocolGuid, ComponentName2,
                            &gEfiDriverConfigurationProtocolGuid, DriverConfiguration,
                            &gEfiDriverDiagnosticsProtocolGuid, DriverDiagnostics,
                            &gEfiDriverDiagnostics2ProtocolGuid, DriverDiagnostics2,
                            NULL
                            );
          }
        } else {
          if (ComponentName2 == NULL || FeaturePcdGet(PcdComponentName2Disable)) {
            Status = gBS->InstallMultipleProtocolInterfaces (
                            &DriverBindingHandle,
                            &gEfiDriverBindingProtocolGuid, DriverBinding,
                            &gEfiComponentNameProtocolGuid, ComponentName,
                            &gEfiDriverConfigurationProtocolGuid, DriverConfiguration,
                            &gEfiDriverDiagnosticsProtocolGuid, DriverDiagnostics,
                            &gEfiDriverDiagnostics2ProtocolGuid, DriverDiagnostics2,
                            NULL
                            );
          } else {
            Status = gBS->InstallMultipleProtocolInterfaces (
                            &DriverBindingHandle,
                            &gEfiDriverBindingProtocolGuid, DriverBinding,
                            &gEfiComponentNameProtocolGuid, ComponentName,
                            &gEfiComponentName2ProtocolGuid, ComponentName2,
                            &gEfiDriverConfigurationProtocolGuid, DriverConfiguration,
                            &gEfiDriverDiagnosticsProtocolGuid, DriverDiagnostics,
                            &gEfiDriverDiagnostics2ProtocolGuid, DriverDiagnostics2,
                            NULL
                            );
          }
        }
      }
    }
  }

  //
  // ASSERT if the call to InstallMultipleProtocolInterfaces() failed
  //
  ASSERT_EFI_ERROR (Status);

  //
  // Update the ImageHandle and DriverBindingHandle fields of the Driver Binding Protocol
  //
  DriverBinding->ImageHandle         = ImageHandle;
  DriverBinding->DriverBindingHandle = DriverBindingHandle;

  return Status;
}


