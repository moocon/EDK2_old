/** @file

Copyright (c) 2006 - 2007, Intel Corporation
All rights reserved. This program and the accompanying materials
are licensed and made available under the terms and conditions of the BSD License
which accompanies this distribution.  The full text of the license may be found at
http://opensource.org/licenses/bsd-license.php

THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

Module Name:

  Ip4Config.c

Abstract:

  This code implements the IP4Config and NicIp4Config protocols.


**/

#include "Ip4Config.h"

IP4_CONFIG_INSTANCE *mIp4ConfigNicList[MAX_IP4_CONFIG_IN_VARIABLE];

VOID
EFIAPI
Ip4ConfigOnDhcp4Complete (
  IN EFI_EVENT                  Event,
  IN VOID                       *Context
  );


/**
  Return the name and MAC address for the NIC. The Name, if not NULL,
  has at least IP4_NIC_NAME_LENGTH bytes.

  @param  This                   The NIC IP4 CONFIG protocol
  @param  Name                   The buffer to return the name
  @param  NicAddr                The buffer to return the MAC addr

  @retval EFI_INVALID_PARAMETER  This is NULL
  @retval EFI_SUCCESS            The name or address of the NIC are returned.

**/
STATIC
EFI_STATUS
EFIAPI
EfiNicIp4ConfigGetName (
  IN  EFI_NIC_IP4_CONFIG_PROTOCOL *This,
  IN  UINT16                      *Name,          OPTIONAL
  IN  NIC_ADDR                    *NicAddr       OPTIONAL
  )
{
  IP4_CONFIG_INSTANCE       *Instance;

  if (This == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  Instance = IP4_CONFIG_INSTANCE_FROM_NIC_IP4CONFIG (This);

  if (Name != NULL) {
    NetCopyMem (Name, Instance->NicName, IP4_NIC_NAME_LENGTH);
  }

  if (NicAddr != NULL) {
    CopyMem (NicAddr, &Instance->NicAddr, sizeof (*NicAddr));
  }

  return EFI_SUCCESS;
}


/**
  Get the NIC's configure information from the IP4 configure  variable.
  It will remove the invalid variable.

  @param  NicAddr                The NIC to check

  @return NULL if no configure for the NIC in the variable, or it is invalid.
  @return Otherwise the NIC's IP configure parameter.

**/
NIC_IP4_CONFIG_INFO *
Ip4ConfigGetNicInfo (
  IN  NIC_ADDR              *NicAddr
  )
{
  IP4_CONFIG_VARIABLE       *Variable;
  IP4_CONFIG_VARIABLE       *NewVariable;
  NIC_IP4_CONFIG_INFO       *Config;

  //
  // Read the configuration parameter for this NicAddr from
  // the EFI variable
  //
  Variable = Ip4ConfigReadVariable ();

  if (Variable == NULL) {
    return NULL;
  }

  Config = Ip4ConfigFindNicVariable (Variable, NicAddr);

  if (Config == NULL) {
    NetFreePool (Variable);
    return NULL;
  }

  //
  // Validate the configuration, if the configuration is invalid,
  // remove it from the variable.
  //
  if (!Ip4ConfigIsValid (Config)) {
    NewVariable = Ip4ConfigModifyVariable (Variable, &Config->NicAddr, NULL);
    Ip4ConfigWriteVariable (NewVariable);

    if (NewVariable != NULL) {
      NetFreePool (NewVariable);
    };

    NetFreePool (Config);
    Config = NULL;
  }

  NetFreePool (Variable);
  return Config;
}


/**
  Get the configure parameter for this NIC.

  @param  This                   The NIC IP4 CONFIG protocol
  @param  ConfigLen              The length of the NicConfig buffer.
  @param  NicConfig              The buffer to receive the NIC's configure
                                 parameter.

  @retval EFI_INVALID_PARAMETER  This or ConfigLen is NULL
  @retval EFI_NOT_FOUND          There is no configure parameter for the NIC in
                                 NVRam.

**/
EFI_STATUS
EFIAPI
EfiNicIp4ConfigGetInfo (
  IN  EFI_NIC_IP4_CONFIG_PROTOCOL *This,
  IN OUT  UINTN                   *ConfigLen,
  OUT NIC_IP4_CONFIG_INFO         *NicConfig
  )
{
  IP4_CONFIG_INSTANCE *Instance;
  NIC_IP4_CONFIG_INFO *Config;
  EFI_STATUS          Status;
  UINTN               Len;

  if ((This == NULL) || (ConfigLen == NULL)) {
    return EFI_INVALID_PARAMETER;
  }

  //
  // Read the Nic's configuration parameter from variable
  //
  Instance  = IP4_CONFIG_INSTANCE_FROM_NIC_IP4CONFIG (This);
  Config    = Ip4ConfigGetNicInfo (&Instance->NicAddr);

  if (Config == NULL) {
    return EFI_NOT_FOUND;
  }

  //
  // Copy the data to user's buffer
  //
  Len = SIZEOF_NIC_IP4_CONFIG_INFO (Config);

  if ((*ConfigLen < Len) || (NicConfig == NULL)) {
    Status = EFI_BUFFER_TOO_SMALL;
  } else {
    Status = EFI_SUCCESS;
    NetCopyMem (NicConfig, Config, Len);
  }

  *ConfigLen = Len;

  NetFreePool (Config);
  return Status;
}


/**
  Set the IP configure parameters for this NIC. If Reconfig is TRUE,
  the IP driver will be informed to discard current auto configure
  parameter and restart the auto configuration process. If current
  there is a pending auto configuration, EFI_ALREADY_STARTED is
  returned. You can only change the configure setting when either
  the configure has finished or not started yet. If NicConfig, the
  NIC's configure parameter is removed from the variable.

  @param  This                   The NIC IP4 CONFIG protocol
  @param  NicConfig              The new NIC IP4 configure parameter
  @param  Reconfig               Inform the IP4 driver to restart the auto
                                 configuration

  @retval EFI_INVALID_PARAMETER  This is NULL or the configure parameter is
                                 invalid.
  @retval EFI_ALREADY_STARTED    There is a pending auto configuration.
  @retval EFI_NOT_FOUND          No auto configure parameter is found

**/
EFI_STATUS
EFIAPI
EfiNicIp4ConfigSetInfo (
  IN EFI_NIC_IP4_CONFIG_PROTOCOL  *This,
  IN NIC_IP4_CONFIG_INFO          *NicConfig,     OPTIONAL
  IN BOOLEAN                      Reconfig
  )
{
  IP4_CONFIG_INSTANCE *Instance;
  IP4_CONFIG_VARIABLE *Variable;
  IP4_CONFIG_VARIABLE *NewVariable;
  EFI_STATUS          Status;

  //
  // Validate the parameters
  //
  if (This == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  Instance = IP4_CONFIG_INSTANCE_FROM_NIC_IP4CONFIG (This);

  if ((NicConfig != NULL) && (!Ip4ConfigIsValid (NicConfig) ||
      !NIC_ADDR_EQUAL (&NicConfig->NicAddr, &Instance->NicAddr))) {
    return EFI_INVALID_PARAMETER;
  }

  if (Instance->State == IP4_CONFIG_STATE_STARTED) {
    return EFI_ALREADY_STARTED;
  }

  //
  // Update the parameter in the configure variable
  //
  Variable = Ip4ConfigReadVariable ();

  if ((Variable == NULL) && (NicConfig == NULL)) {
    return EFI_NOT_FOUND;
  }

  NewVariable = Ip4ConfigModifyVariable (Variable, &Instance->NicAddr, NicConfig);
  Status      = Ip4ConfigWriteVariable (NewVariable);

  if (NewVariable != NULL) {
    NetFreePool (NewVariable);
  }

  //
  // Variable is NULL when saving the first configure parameter
  //
  if (Variable != NULL) {
    NetFreePool (Variable);
  }

  if (EFI_ERROR (Status)) {
    return Status;
  }

  //
  // Signal the IP4 to run the auto configuration again
  //
  if (Reconfig && (Instance->ReconfigEvent != NULL)) {
    Status = gBS->SignalEvent (Instance->ReconfigEvent);
  }

  return Status;
}


/**
  Start the auto configuration process.

  @param  This                   The IP4 configure protocol
  @param  DoneEvent              The event to signal when auto configure is done
  @param  ReconfigEvent          The event to signal when reconfigure is necessary.

  @retval EFI_INVALID_PARAMETER  One of the function parameters is NULL.
  @retval EFI_ALREADY_STARTED    The auto configuration has already started.
  @retval EFI_SUCCESS            The auto configure is successfully started.

**/
EFI_STATUS
EFIAPI
EfiIp4ConfigStart (
  IN EFI_IP4_CONFIG_PROTOCOL  *This,
  IN EFI_EVENT                DoneEvent,
  IN EFI_EVENT                ReconfigEvent
  )
{
  IP4_CONFIG_INSTANCE       *Instance;
  EFI_DHCP4_PROTOCOL        *Dhcp4;
  EFI_DHCP4_MODE_DATA       Dhcp4Mode;
  EFI_DHCP4_PACKET_OPTION   *OptionList[1];
  IP4_CONFIG_DHCP4_OPTION   ParaList;
  EFI_STATUS                Status;
  UINT32                    Source;
  EFI_TPL                   OldTpl;

  if ((This == NULL) || (DoneEvent == NULL) || (ReconfigEvent == NULL)) {
    return EFI_INVALID_PARAMETER;
  }

  Instance = IP4_CONFIG_INSTANCE_FROM_IP4CONFIG (This);

  OldTpl = NET_RAISE_TPL (NET_TPL_LOCK);

  if (Instance->State != IP4_CONFIG_STATE_IDLE) {
    Status = EFI_ALREADY_STARTED;

    goto ON_EXIT;
  }

  Instance->DoneEvent     = DoneEvent;
  Instance->ReconfigEvent = ReconfigEvent;

  Instance->NicConfig     = Ip4ConfigGetNicInfo (&Instance->NicAddr);

  if (Instance->NicConfig == NULL) {
    Source = IP4_CONFIG_SOURCE_DHCP;
  } else {
    Source = Instance->NicConfig->Source;
  }

  //
  // If the source is static, the auto configuration is done.
  // return now.
  //
  if (Source == IP4_CONFIG_SOURCE_STATIC) {
    Instance->State  = IP4_CONFIG_STATE_CONFIGURED;
    Instance->Result = EFI_SUCCESS;

    gBS->SignalEvent (Instance->DoneEvent);
    Status = EFI_SUCCESS;
    goto ON_EXIT;
  }

  //
  // Start the dhcp process
  //
  ASSERT ((Source == IP4_CONFIG_SOURCE_DHCP) && (Instance->Dhcp4 == NULL));

  Status = NetLibCreateServiceChild (
             Instance->Controller,
             Instance->Image,
             &gEfiDhcp4ServiceBindingProtocolGuid,
             &Instance->Dhcp4Handle
             );

  if (EFI_ERROR (Status)) {
    goto ON_ERROR;
  }

  Status = gBS->OpenProtocol (
                  Instance->Dhcp4Handle,
                  &gEfiDhcp4ProtocolGuid,
                  (VOID **) &Instance->Dhcp4,
                  Instance->Image,
                  Instance->Controller,
                  EFI_OPEN_PROTOCOL_BY_DRIVER
                  );

  if (EFI_ERROR (Status)) {
    goto ON_ERROR;
  }

  //
  // Check the current DHCP status, if the DHCP process has
  // already finished, return now.
  //
  Dhcp4  = Instance->Dhcp4;
  Status = Dhcp4->GetModeData (Dhcp4, &Dhcp4Mode);

  if (EFI_ERROR (Status)) {
    goto ON_ERROR;
  }

  if (Dhcp4Mode.State == Dhcp4Bound) {
    Ip4ConfigOnDhcp4Complete (NULL, Instance);

    goto ON_EXIT;
  }

  //
  // Try to start the DHCP process. Use most of the current
  // DHCP configuration to avoid problems if some DHCP client
  // yields the control of this DHCP service to us.
  //
  ParaList.Head.OpCode             = DHCP_TAG_PARA_LIST;
  ParaList.Head.Length             = 2;
  ParaList.Head.Data[0]            = DHCP_TAG_NETMASK;
  ParaList.Route                   = DHCP_TAG_ROUTER;
  OptionList[0]                    = &ParaList.Head;
  Dhcp4Mode.ConfigData.OptionCount = 1;
  Dhcp4Mode.ConfigData.OptionList  = OptionList;

  Status = Dhcp4->Configure (Dhcp4, &Dhcp4Mode.ConfigData);

  if (EFI_ERROR (Status)) {
    goto ON_ERROR;
  }

  //
  // Start the DHCP process
  //
  Status = gBS->CreateEvent (
                  EVT_NOTIFY_SIGNAL,
                  NET_TPL_EVENT,
                  Ip4ConfigOnDhcp4Complete,
                  Instance,
                  &Instance->Dhcp4Event
                  );

  if (EFI_ERROR (Status)) {
    goto ON_ERROR;
  }

  Status = Dhcp4->Start (Dhcp4, Instance->Dhcp4Event);

  if (EFI_ERROR (Status)) {
    goto ON_ERROR;
  }

  Instance->State  = IP4_CONFIG_STATE_STARTED;
  Instance->Result = EFI_NOT_READY;

ON_ERROR:
  if (EFI_ERROR (Status)) {
    Ip4ConfigCleanConfig (Instance);
  }

ON_EXIT:
  NET_RESTORE_TPL (OldTpl);

  return Status;
}


/**
  Stop the current auto configuration

  @param  This                   The IP4 CONFIG protocol

  @retval EFI_INVALID_PARAMETER  This is NULL.
  @retval EFI_NOT_STARTED        The auto configuration hasn't been started.
  @retval EFI_SUCCESS            The auto configuration has been stopped.

**/
EFI_STATUS
EFIAPI
EfiIp4ConfigStop (
  IN EFI_IP4_CONFIG_PROTOCOL  *This
  )
{
  IP4_CONFIG_INSTANCE  *Instance;
  EFI_STATUS           Status;
  EFI_TPL              OldTpl;

  if (This == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  Instance = IP4_CONFIG_INSTANCE_FROM_IP4CONFIG (This);

  Status = EFI_SUCCESS;
  OldTpl = NET_RAISE_TPL (NET_TPL_LOCK);

  if (Instance->State == IP4_CONFIG_STATE_IDLE) {
    Status = EFI_NOT_STARTED;
    goto ON_EXIT;
  }

  //
  // Release all the configure parameters. Don't signal the user
  // event. The user wants to abort the configuration, this isn't
  // the configuration done or reconfiguration.
  //
  Ip4ConfigCleanConfig (Instance);

ON_EXIT:
  NET_RESTORE_TPL (OldTpl);

  return Status;
}


/**
  Get the current outcome of the auto configuration process

  @param  This                   The IP4 CONFIG protocol
  @param  ConfigDataSize         The size of the configure data
  @param  ConfigData             The buffer to save the configure data

  @retval EFI_INVALID_PARAMETER  This or ConfigDataSize is NULL
  @retval EFI_BUFFER_TOO_SMALL   The buffer is too small. The needed size is
                                 returned in the ConfigDataSize.
  @retval EFI_SUCCESS            The configure data is put in the buffer

**/
EFI_STATUS
EFIAPI
EfiIp4ConfigGetData (
  IN  EFI_IP4_CONFIG_PROTOCOL *This,
  IN  OUT  UINTN              *ConfigDataSize,
  OUT EFI_IP4_IPCONFIG_DATA   *ConfigData           OPTIONAL
  )
{
  IP4_CONFIG_INSTANCE       *Instance;
  NIC_IP4_CONFIG_INFO       *NicConfig;
  EFI_STATUS                Status;
  EFI_TPL                   OldTpl;
  UINTN                     Len;

  if ((This == NULL) || (ConfigDataSize == NULL)) {
    return EFI_INVALID_PARAMETER;
  }

  Instance  = IP4_CONFIG_INSTANCE_FROM_IP4CONFIG (This);

  Status = EFI_SUCCESS;
  OldTpl = NET_RAISE_TPL (NET_TPL_LOCK);

  if (Instance->State == IP4_CONFIG_STATE_IDLE) {
    Status = EFI_NOT_STARTED;
  } else if (Instance->State == IP4_CONFIG_STATE_STARTED) {
    Status = EFI_NOT_READY;
  }

  if (EFI_ERROR (Status)) {
    goto ON_EXIT;
  }

  //
  // Copy the configure data if auto configuration succeeds.
  //
  Status = Instance->Result;

  if (Status == EFI_SUCCESS) {
    ASSERT (Instance->NicConfig != NULL);

    NicConfig = Instance->NicConfig;
    Len       = SIZEOF_IP4_CONFIG_INFO (&NicConfig->Ip4Info);

    if ((*ConfigDataSize < Len) || (ConfigData == NULL)) {
      Status = EFI_BUFFER_TOO_SMALL;
    } else {
      NetCopyMem (ConfigData, &NicConfig->Ip4Info, Len);
    }

    *ConfigDataSize = Len;
  }

ON_EXIT:
  NET_RESTORE_TPL (OldTpl);

  return Status;
}


/**
  Callback function when DHCP process finished. It will save the
  retrieved IP configure parameter from DHCP to the NVRam.

  @param  Event                  The callback event
  @param  Context                Opaque context to the callback

  @return None

**/
VOID
EFIAPI
Ip4ConfigOnDhcp4Complete (
  IN EFI_EVENT              Event,
  IN VOID                   *Context
  )
{
  IP4_CONFIG_INSTANCE       *Instance;
  EFI_DHCP4_MODE_DATA       Dhcp4Mode;
  EFI_IP4_IPCONFIG_DATA     *Ip4Config;
  EFI_STATUS                Status;
  BOOLEAN                   Perment;
  IP4_ADDR                  Subnet;
  IP4_ADDR                  Ip1;
  IP4_ADDR                  Ip2;

  Instance = (IP4_CONFIG_INSTANCE *) Context;
  ASSERT (Instance->Dhcp4 != NULL);

  Instance->State   = IP4_CONFIG_STATE_CONFIGURED;
  Instance->Result  = EFI_TIMEOUT;

  //
  // Get the DHCP retrieved parameters
  //
  Status = Instance->Dhcp4->GetModeData (Instance->Dhcp4, &Dhcp4Mode);

  if (EFI_ERROR (Status)) {
    goto ON_EXIT;
  }

  if (Dhcp4Mode.State == Dhcp4Bound) {
    //
    // Save the new configuration retrieved by DHCP both in
    // the instance and to NVRam. So, both the IP4 driver and
    // other user can get that address.
    //
    Perment = FALSE;

    if (Instance->NicConfig != NULL) {
      ASSERT (Instance->NicConfig->Source == IP4_CONFIG_SOURCE_DHCP);
      Perment = Instance->NicConfig->Perment;
      NetFreePool (Instance->NicConfig);
    }

    Instance->NicConfig = NetAllocatePool (sizeof (NIC_IP4_CONFIG_INFO) +
                                           sizeof (EFI_IP4_ROUTE_TABLE));

    if (Instance->NicConfig == NULL) {
      Instance->Result = EFI_OUT_OF_RESOURCES;
      goto ON_EXIT;
    }

    CopyMem (&Instance->NicConfig->NicAddr, &Instance->NicAddr, sizeof (Instance->NicConfig->NicAddr));
    Instance->NicConfig->Source  = IP4_CONFIG_SOURCE_DHCP;
    Instance->NicConfig->Perment = Perment;

    Ip4Config                    = &Instance->NicConfig->Ip4Info;
    Ip4Config->StationAddress    = Dhcp4Mode.ClientAddress;
    Ip4Config->SubnetMask        = Dhcp4Mode.SubnetMask;

    //
    // Create a route for the connected network
    //
    Ip4Config->RouteTableSize    = 1;

    NetCopyMem (&Ip1, &Dhcp4Mode.ClientAddress, sizeof (IP4_ADDR));
    NetCopyMem (&Ip2, &Dhcp4Mode.SubnetMask, sizeof (IP4_ADDR));
    
    Subnet = Ip1 & Ip2;

    NetCopyMem (&Ip4Config->RouteTable[0].SubnetAddress, &Subnet, sizeof (EFI_IPv4_ADDRESS));
    NetCopyMem (&Ip4Config->RouteTable[0].SubnetMask, &Dhcp4Mode.SubnetMask, sizeof (EFI_IPv4_ADDRESS));
    NetZeroMem (&Ip4Config->RouteTable[0].GatewayAddress, sizeof (EFI_IPv4_ADDRESS));

    //
    // Create a route if there is a default router.
    //
    if (!EFI_IP4_EQUAL (Dhcp4Mode.RouterAddress, mZeroIp4Addr)) {
      Ip4Config->RouteTableSize = 2;

      NetZeroMem (&Ip4Config->RouteTable[1].SubnetAddress, sizeof (EFI_IPv4_ADDRESS));
      NetZeroMem (&Ip4Config->RouteTable[1].SubnetMask, sizeof (EFI_IPv4_ADDRESS));
      NetCopyMem (&Ip4Config->RouteTable[1].GatewayAddress, &Dhcp4Mode.RouterAddress, sizeof (EFI_IPv4_ADDRESS));
    }

    Instance->Result = EFI_SUCCESS;

    //
    // ignore the return status of EfiNicIp4ConfigSetInfo. Network
    // stack can operate even that failed.
    //
    EfiNicIp4ConfigSetInfo (&Instance->NicIp4Protocol, Instance->NicConfig, FALSE);
  }

ON_EXIT:
  gBS->SignalEvent (Instance->DoneEvent);
  Ip4ConfigCleanDhcp4 (Instance);
  return ;
}


/**
  Release all the DHCP related resources.

  @param  This                   The IP4 configure instance

  @return None

**/
VOID
Ip4ConfigCleanDhcp4 (
  IN IP4_CONFIG_INSTANCE    *This
  )
{
  if (This->Dhcp4 != NULL) {
    This->Dhcp4->Stop (This->Dhcp4);

    gBS->CloseProtocol (
          This->Dhcp4Handle,
          &gEfiDhcp4ProtocolGuid,
          This->Image,
          This->Controller
          );

    This->Dhcp4 = NULL;
  }

  if (This->Dhcp4Handle != NULL) {
    NetLibDestroyServiceChild (
      This->Controller,
      This->Image,
      &gEfiDhcp4ServiceBindingProtocolGuid,
      This->Dhcp4Handle
      );

    This->Dhcp4Handle = NULL;
  }

  if (This->Dhcp4Event == NULL) {
    gBS->CloseEvent (This->Dhcp4Event);
    This->Dhcp4Event = NULL;
  }
}


/**
  Clean up all the configuration parameters

  @param  Instance               The IP4 configure instance

  @return None

**/
VOID
Ip4ConfigCleanConfig (
  IN IP4_CONFIG_INSTANCE        *Instance
  )
{
  if (Instance->NicConfig != NULL) {
    NetFreePool (Instance->NicConfig);
    Instance->NicConfig = NULL;
  }

  Instance->State         = IP4_CONFIG_STATE_IDLE;
  Instance->DoneEvent     = NULL;
  Instance->ReconfigEvent = NULL;

  Ip4ConfigCleanDhcp4 (Instance);
}

EFI_IP4_CONFIG_PROTOCOL     mIp4ConfigProtocolTemplate = {
  EfiIp4ConfigStart,
  EfiIp4ConfigStop,
  EfiIp4ConfigGetData
};

EFI_NIC_IP4_CONFIG_PROTOCOL mNicIp4ConfigProtocolTemplate = {
  EfiNicIp4ConfigGetName,
  EfiNicIp4ConfigGetInfo,
  EfiNicIp4ConfigSetInfo
};
