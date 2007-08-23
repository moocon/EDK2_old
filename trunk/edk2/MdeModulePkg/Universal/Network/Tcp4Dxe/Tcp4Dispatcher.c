/** @file

Copyright (c) 2005 - 2006, Intel Corporation
All rights reserved. This program and the accompanying materials
are licensed and made available under the terms and conditions of the BSD License
which accompanies this distribution.  The full text of the license may be found at
http://opensource.org/licenses/bsd-license.php

THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

Module Name:

  Tcp4Dispatcher.c

Abstract:


**/

#include "Tcp4Main.h"

#define TCP_COMP_VAL(Min, Max, Default, Val) \
  ((((Val) <= (Max)) && ((Val) >= (Min))) ? (Val) : (Default))

STATIC
EFI_STATUS
Tcp4Route (
  IN TCP_CB           *Tcb,
  IN TCP4_ROUTE_INFO  *RouteInfo
  )
/*++

Routine Description:

  Add or remove a route entry in the IP route table associated
  with this TCP instance.

Arguments:

  Tcb       - Pointer to the TCP_CB of this TCP instance.
  RouteInfo - Pointer to the route info to be processed.

Returns:

  EFI_SUCCESS          - The operation completed successfully.
  EFI_NOT_STARTED      - The driver instance has not been started.
  EFI_NO_MAPPING       - When using the default address, configuration(DHCP,
                         BOOTP, RARP, etc.) is not finished yet.
  EFI_OUT_OF_RESOURCES - Could not add the entry to the routing table.
  EFI_NOT_FOUND        - This route is not in the routing table
                         (when RouteInfo->DeleteRoute is TRUE).
  EFI_ACCESS_DENIED    - The route is already defined in the routing table
                         (when RouteInfo->DeleteRoute is FALSE).

--*/
{
  EFI_IP4_PROTOCOL  *Ip;

  Ip = Tcb->IpInfo->Ip;

  ASSERT (Ip);

  return Ip->Routes (
              Ip,
              RouteInfo->DeleteRoute,
              RouteInfo->SubnetAddress,
              RouteInfo->SubnetMask,
              RouteInfo->GatewayAddress
              );

}


/**
  Get the operational settings of this TCP instance.

  @param  Tcb                    Pointer to the TCP_CB of this TCP instance.
  @param  Mode                   Pointer to the buffer to store the operational
                                 settings.

  @retval EFI_SUCCESS            The mode data is read.
  @retval EFI_NOT_STARTED        No configuration data is available because this
                                 instance hasn't been started.

**/
STATIC
EFI_STATUS
Tcp4GetMode (
  IN TCP_CB         *Tcb,
  IN TCP4_MODE_DATA *Mode
  )
{
  SOCKET                *Sock;
  EFI_TCP4_CONFIG_DATA  *ConfigData;
  EFI_TCP4_ACCESS_POINT *AccessPoint;
  EFI_TCP4_OPTION       *Option;
  EFI_IP4_PROTOCOL      *Ip;

  Sock = Tcb->Sk;

  if (!SOCK_IS_CONFIGURED (Sock) && (Mode->Tcp4ConfigData != NULL)) {
    return EFI_NOT_STARTED;
  }

  if (Mode->Tcp4State) {
    *(Mode->Tcp4State) = (EFI_TCP4_CONNECTION_STATE) Tcb->State;
  }

  if (Mode->Tcp4ConfigData) {

    ConfigData                      = Mode->Tcp4ConfigData;
    AccessPoint                     = &(ConfigData->AccessPoint);
    Option                          = ConfigData->ControlOption;

    ConfigData->TypeOfService       = Tcb->TOS;
    ConfigData->TimeToLive          = Tcb->TTL;

    AccessPoint->UseDefaultAddress  = Tcb->UseDefaultAddr;

    NetCopyMem (&AccessPoint->StationAddress, &Tcb->LocalEnd.Ip, sizeof (EFI_IPv4_ADDRESS));
    AccessPoint->SubnetMask         = Tcb->SubnetMask;
    AccessPoint->StationPort        = NTOHS (Tcb->LocalEnd.Port);

    NetCopyMem (&AccessPoint->RemoteAddress, &Tcb->RemoteEnd.Ip, sizeof (EFI_IPv4_ADDRESS));
    AccessPoint->RemotePort         = NTOHS (Tcb->RemoteEnd.Port);
    AccessPoint->ActiveFlag         = (BOOLEAN) (Tcb->State != TCP_LISTEN);

    if (Option != NULL) {
      Option->ReceiveBufferSize       = GET_RCV_BUFFSIZE (Tcb->Sk);
      Option->SendBufferSize          = GET_SND_BUFFSIZE (Tcb->Sk);
      Option->MaxSynBackLog           = GET_BACKLOG (Tcb->Sk);

      Option->ConnectionTimeout       = Tcb->ConnectTimeout / TCP_TICK_HZ;
      Option->DataRetries             = Tcb->MaxRexmit;
      Option->FinTimeout              = Tcb->FinWait2Timeout / TCP_TICK_HZ;
      Option->TimeWaitTimeout         = Tcb->TimeWaitTimeout / TCP_TICK_HZ;
      Option->KeepAliveProbes         = Tcb->MaxKeepAlive;
      Option->KeepAliveTime           = Tcb->KeepAliveIdle / TCP_TICK_HZ;
      Option->KeepAliveInterval       = Tcb->KeepAlivePeriod / TCP_TICK_HZ;

      Option->EnableNagle         = (BOOLEAN) (!TCP_FLG_ON (Tcb->CtrlFlag, TCP_CTRL_NO_NAGLE));
      Option->EnableTimeStamp     = (BOOLEAN) (!TCP_FLG_ON (Tcb->CtrlFlag, TCP_CTRL_NO_TS));
      Option->EnableWindowScaling = (BOOLEAN) (!TCP_FLG_ON (Tcb->CtrlFlag, TCP_CTRL_NO_WS))
;

      Option->EnableSelectiveAck      = FALSE;
      Option->EnablePathMtuDiscovery  = FALSE;
    }
  }

  Ip = Tcb->IpInfo->Ip;
  ASSERT (Ip);

  return Ip->GetModeData (Ip, Mode->Ip4ModeData, Mode->MnpConfigData, Mode->SnpModeData);
}


/**
  If AP->StationPort isn't zero, check whether the access point
  is registered, else generate a random station port for this
  access point.

  @param  AP                     Pointer to the access point.

  @retval EFI_SUCCESS            The check is passed or the port is assigned.
  @retval EFI_INVALID_PARAMETER  The non-zero station port is already used.
  @retval EFI_OUT_OF_RESOURCES   No port can be allocated.

**/
STATIC
EFI_STATUS
Tcp4Bind (
  IN EFI_TCP4_ACCESS_POINT *AP
  )
{
  BOOLEAN Cycle;

  if (0 != AP->StationPort) {
    //
    // check if a same endpoint is bound
    //
    if (TcpFindTcbByPeer (&AP->StationAddress, AP->StationPort)) {

      return EFI_INVALID_PARAMETER;
    }
  } else {
    //
    // generate a random port
    //
    Cycle = FALSE;

    if (TCP4_PORT_USER_RESERVED == mTcp4RandomPort) {
      mTcp4RandomPort = TCP4_PORT_KNOWN;
    }

    mTcp4RandomPort++;

    while (TcpFindTcbByPeer (&AP->StationAddress, mTcp4RandomPort)) {

      mTcp4RandomPort++;

      if (mTcp4RandomPort <= TCP4_PORT_KNOWN) {

        if (Cycle) {
          TCP4_DEBUG_ERROR (("Tcp4Bind: no port can be allocated "
            "for this pcb\n"));

          return EFI_OUT_OF_RESOURCES;
        }

        mTcp4RandomPort = TCP4_PORT_KNOWN + 1;

        Cycle = TRUE;
      }

    }

    AP->StationPort = mTcp4RandomPort;
  }

  return EFI_SUCCESS;
}


/**
  Flush the Tcb add its associated protocols..

  @param  Tcb                    Pointer to the TCP_CB to be flushed.

  @retval EFI_SUCCESS            The operation is completed successfully.

**/
STATIC
VOID
Tcp4FlushPcb (
  IN TCP_CB *Tcb
  )
{
  SOCKET           *Sock;
  TCP4_PROTO_DATA  *TcpProto;

  IpIoConfigIp (Tcb->IpInfo, NULL);

  Sock     = Tcb->Sk;
  TcpProto = (TCP4_PROTO_DATA *) Sock->ProtoReserved;

  if (SOCK_IS_CONFIGURED (Sock)) {
    NetListRemoveEntry (&Tcb->List);

    TcpSetVariableData (TcpProto->TcpService);
  }

  NetbufFreeList (&Tcb->SndQue);
  NetbufFreeList (&Tcb->RcvQue);
}

STATIC
EFI_STATUS
Tcp4AttachPcb (
  IN SOCKET  *Sk
  )
{
  TCP_CB            *Tcb;
  TCP4_PROTO_DATA   *ProtoData;
  IP_IO             *IpIo;

  Tcb = NetAllocateZeroPool (sizeof (TCP_CB));

  if (Tcb == NULL) {

    TCP4_DEBUG_ERROR (("Tcp4ConfigurePcb: failed to allocate a TCB\n"));

    return EFI_OUT_OF_RESOURCES;
  }

  ProtoData  = (TCP4_PROTO_DATA *) Sk->ProtoReserved;
  IpIo       = ProtoData->TcpService->IpIo;

  //
  // Create an IpInfo for this Tcb.
  //
  Tcb->IpInfo = IpIoAddIp (IpIo);
  if (Tcb->IpInfo == NULL) {

    NetFreePool (Tcb);
    return EFI_OUT_OF_RESOURCES;
  }

  NetListInit (&Tcb->List);
  NetListInit (&Tcb->SndQue);
  NetListInit (&Tcb->RcvQue);

  Tcb->State        = TCP_CLOSED;
  Tcb->Sk           = Sk;
  ProtoData->TcpPcb = Tcb;

  return EFI_SUCCESS;
}

STATIC
VOID
Tcp4DetachPcb (
  IN SOCKET  *Sk
  )
{
  TCP4_PROTO_DATA  *ProtoData;
  TCP_CB           *Tcb;

  ProtoData = (TCP4_PROTO_DATA *) Sk->ProtoReserved;
  Tcb       = ProtoData->TcpPcb;

  ASSERT (Tcb != NULL);

  Tcp4FlushPcb (Tcb);

  IpIoRemoveIp (ProtoData->TcpService->IpIo, Tcb->IpInfo);

  NetFreePool (Tcb);

  ProtoData->TcpPcb = NULL;
}


/**
  Configure the Tcb using CfgData.

  @param  Sk                     Pointer to the socket of this TCP instance.
  @param  SkTcb                  Pointer to the TCP_CB of this TCP instance.
  @param  CfgData                Pointer to the TCP configuration data.

  @retval EFI_SUCCESS            The operation is completed successfully.
  @retval EFI_INVALID_PARAMETER  A same access point has been configured in
                                 another TCP instance.
  @retval EFI_OUT_OF_RESOURCES   Failed due to resource limit.

**/
STATIC
EFI_STATUS
Tcp4ConfigurePcb (
  IN SOCKET               *Sk,
  IN EFI_TCP4_CONFIG_DATA *CfgData
  )
{
  EFI_IP4_CONFIG_DATA IpCfgData;
  EFI_STATUS          Status;
  EFI_TCP4_OPTION     *Option;
  TCP4_PROTO_DATA     *TcpProto;
  TCP_CB              *Tcb;

  ASSERT (CfgData && Sk && Sk->SockHandle);

  TcpProto = (TCP4_PROTO_DATA *) Sk->ProtoReserved;
  Tcb      = TcpProto->TcpPcb;

  ASSERT (Tcb != NULL);

  //
  // Add Ip for send pkt to the peer
  //
  CopyMem (&IpCfgData, &mIpIoDefaultIpConfigData, sizeof (IpCfgData));
  IpCfgData.DefaultProtocol   = EFI_IP_PROTO_TCP;
  IpCfgData.UseDefaultAddress = CfgData->AccessPoint.UseDefaultAddress;
  IpCfgData.StationAddress    = CfgData->AccessPoint.StationAddress;
  IpCfgData.SubnetMask        = CfgData->AccessPoint.SubnetMask;
  IpCfgData.ReceiveTimeout    = (UINT32) (-1);

  //
  // Configure the IP instance this Tcb consumes.
  //
  Status = IpIoConfigIp (Tcb->IpInfo, &IpCfgData);
  if (EFI_ERROR (Status)) {
    goto OnExit;
  }

  //
  // Get the default address info if the instance is configured to use default address.
  //
  if (CfgData->AccessPoint.UseDefaultAddress) {
    CfgData->AccessPoint.StationAddress = IpCfgData.StationAddress;
    CfgData->AccessPoint.SubnetMask     = IpCfgData.SubnetMask;
  }

  //
  // check if we can bind this endpoint in CfgData
  //
  Status = Tcp4Bind (&(CfgData->AccessPoint));

  if (EFI_ERROR (Status)) {
    TCP4_DEBUG_ERROR (("Tcp4ConfigurePcb: Bind endpoint failed "
      "with %r\n", Status));

    goto OnExit;
  }

  //
  // Initalize the operating information in this Tcb
  //
  ASSERT (Tcb->State == TCP_CLOSED &&
    NetListIsEmpty (&Tcb->SndQue) &&
    NetListIsEmpty (&Tcb->RcvQue));

  TCP_SET_FLG (Tcb->CtrlFlag, TCP_CTRL_NO_KEEPALIVE);
  Tcb->State            = TCP_CLOSED;

  Tcb->SndMss           = 536;
  Tcb->RcvMss           = TcpGetRcvMss (Sk);

  Tcb->SRtt             = 0;
  Tcb->Rto              = 3 * TCP_TICK_HZ;

  Tcb->CWnd             = Tcb->SndMss;
  Tcb->Ssthresh         = 0xffffffff;

  Tcb->CongestState     = TCP_CONGEST_OPEN;

  Tcb->KeepAliveIdle    = TCP_KEEPALIVE_IDLE_MIN;
  Tcb->KeepAlivePeriod  = TCP_KEEPALIVE_PERIOD;
  Tcb->MaxKeepAlive     = TCP_MAX_KEEPALIVE;
  Tcb->MaxRexmit        = TCP_MAX_LOSS;
  Tcb->FinWait2Timeout  = TCP_FIN_WAIT2_TIME;
  Tcb->TimeWaitTimeout  = TCP_TIME_WAIT_TIME;
  Tcb->ConnectTimeout   = TCP_CONNECT_TIME;

  //
  // initialize Tcb in the light of CfgData
  //
  Tcb->TTL            = CfgData->TimeToLive;
  Tcb->TOS            = CfgData->TypeOfService;

  NetCopyMem (&Tcb->LocalEnd.Ip, &CfgData->AccessPoint.StationAddress, sizeof (IP4_ADDR));
  Tcb->LocalEnd.Port  = HTONS (CfgData->AccessPoint.StationPort);
  Tcb->SubnetMask     = CfgData->AccessPoint.SubnetMask;

  NetCopyMem (&Tcb->RemoteEnd.Ip, &CfgData->AccessPoint.RemoteAddress, sizeof (IP4_ADDR));
  Tcb->RemoteEnd.Port = HTONS (CfgData->AccessPoint.RemotePort);

  Option              = CfgData->ControlOption;

  if (Option != NULL) {
    SET_RCV_BUFFSIZE (
      Sk,
      (UINT32) (TCP_COMP_VAL (
                  TCP_RCV_BUF_SIZE_MIN,
                  TCP_RCV_BUF_SIZE,
                  TCP_RCV_BUF_SIZE,
                  Option->ReceiveBufferSize
                  )
               )
      );
    SET_SND_BUFFSIZE (
      Sk,
      (UINT32) (TCP_COMP_VAL (
                  TCP_SND_BUF_SIZE_MIN,
                  TCP_SND_BUF_SIZE,
                  TCP_SND_BUF_SIZE,
                  Option->SendBufferSize
                  )
               )
      );

    SET_BACKLOG (
      Sk,
      (UINT32) (TCP_COMP_VAL (
                  TCP_BACKLOG_MIN,
                  TCP_BACKLOG,
                  TCP_BACKLOG,
                  Option->MaxSynBackLog
                  )
               )
      );

    Tcb->MaxRexmit = (UINT16) TCP_COMP_VAL (
                                TCP_MAX_LOSS_MIN,
                                TCP_MAX_LOSS,
                                TCP_MAX_LOSS,
                                Option->DataRetries
                                );
    Tcb->FinWait2Timeout = TCP_COMP_VAL (
                              TCP_FIN_WAIT2_TIME,
                              TCP_FIN_WAIT2_TIME_MAX,
                              TCP_FIN_WAIT2_TIME,
                              (UINT32) (Option->FinTimeout * TCP_TICK_HZ)
                              );

    if (Option->TimeWaitTimeout != 0) {
      Tcb->TimeWaitTimeout = TCP_COMP_VAL (
                               TCP_TIME_WAIT_TIME,
                               TCP_TIME_WAIT_TIME_MAX,
                               TCP_TIME_WAIT_TIME,
                               (UINT32) (Option->TimeWaitTimeout * TCP_TICK_HZ)
                               );
    } else {
      Tcb->TimeWaitTimeout = 0;
    }

    if (Option->KeepAliveProbes != 0) {
      TCP_CLEAR_FLG (Tcb->CtrlFlag, TCP_CTRL_NO_KEEPALIVE);

      Tcb->MaxKeepAlive = (UINT8) TCP_COMP_VAL (
                                    TCP_MAX_KEEPALIVE_MIN,
                                    TCP_MAX_KEEPALIVE,
                                    TCP_MAX_KEEPALIVE,
                                    Option->KeepAliveProbes
                                    );
      Tcb->KeepAliveIdle = TCP_COMP_VAL (
                             TCP_KEEPALIVE_IDLE_MIN,
                             TCP_KEEPALIVE_IDLE_MAX,
                             TCP_KEEPALIVE_IDLE_MIN,
                             (UINT32) (Option->KeepAliveTime * TCP_TICK_HZ)
                             );
      Tcb->KeepAlivePeriod = TCP_COMP_VAL (
                               TCP_KEEPALIVE_PERIOD_MIN,
                               TCP_KEEPALIVE_PERIOD,
                               TCP_KEEPALIVE_PERIOD,
                               (UINT32) (Option->KeepAliveInterval * TCP_TICK_HZ)
                               );
    }

    Tcb->ConnectTimeout = TCP_COMP_VAL (
                            TCP_CONNECT_TIME_MIN,
                            TCP_CONNECT_TIME,
                            TCP_CONNECT_TIME,
                            (UINT32) (Option->ConnectionTimeout * TCP_TICK_HZ)
                            );

    if (Option->EnableNagle == FALSE) {
      TCP_SET_FLG (Tcb->CtrlFlag, TCP_CTRL_NO_NAGLE);
    }

    if (Option->EnableTimeStamp == FALSE) {
      TCP_SET_FLG (Tcb->CtrlFlag, TCP_CTRL_NO_TS);
    }

    if (Option->EnableWindowScaling == FALSE) {
      TCP_SET_FLG (Tcb->CtrlFlag, TCP_CTRL_NO_WS);
    }
  }

  //
  // update state of Tcb and socket
  //
  if (CfgData->AccessPoint.ActiveFlag == FALSE) {

    TcpSetState (Tcb, TCP_LISTEN);
    SockSetState (Sk, SO_LISTENING);

    Sk->ConfigureState = SO_CONFIGURED_PASSIVE;
  } else {

    Sk->ConfigureState = SO_CONFIGURED_ACTIVE;
  }

  TcpInsertTcb (Tcb);

OnExit:

  return Status;
}


/**
  The procotol handler provided to the socket layer, used to
  dispatch the socket level requests by calling the corresponding
  TCP layer functions.

  @param  Sock                   Pointer to the socket of this TCP instance.
  @param  Request                The code of this operation request.
  @param  Data                   Pointer to the operation specific data passed in
                                 together with the operation request.

  @retval EFI_SUCCESS            The socket request is completed successfully.
  @retval other                  The error status returned by the corresponding TCP
                                 layer function.

**/
EFI_STATUS
Tcp4Dispatcher (
  IN SOCKET                  *Sock,
  IN SOCK_REQUEST            Request,
  IN VOID                    *Data    OPTIONAL
  )
{
  TCP_CB            *Tcb;
  TCP4_PROTO_DATA   *ProtoData;
  EFI_IP4_PROTOCOL  *Ip;

  ProtoData = (TCP4_PROTO_DATA *) Sock->ProtoReserved;
  Tcb       = ProtoData->TcpPcb;

  switch (Request) {
  case SOCK_POLL:
    Ip = ProtoData->TcpService->IpIo->Ip;
    Ip->Poll (Ip);
    break;

  case SOCK_CONSUMED:
    //
    // After user received data from socket buffer, socket will
    // notify TCP using this message to give it a chance to send out
    // window update information
    //
    ASSERT (Tcb);
    TcpOnAppConsume (Tcb);
    break;

  case SOCK_SND:

    ASSERT (Tcb);
    TcpOnAppSend (Tcb);
    break;

  case SOCK_CLOSE:

    TcpOnAppClose (Tcb);

    break;

  case SOCK_ABORT:

    TcpOnAppAbort (Tcb);

    break;

  case SOCK_SNDPUSH:
    Tcb->SndPsh = TcpGetMaxSndNxt (Tcb) + GET_SND_DATASIZE (Tcb->Sk);
    TCP_SET_FLG (Tcb->CtrlFlag, TCP_CTRL_SND_PSH);

    break;

  case SOCK_SNDURG:
    Tcb->SndUp = TcpGetMaxSndNxt (Tcb) + GET_SND_DATASIZE (Tcb->Sk) - 1;
    TCP_SET_FLG (Tcb->CtrlFlag, TCP_CTRL_SND_URG);

    break;

  case SOCK_CONNECT:

    TcpOnAppConnect (Tcb);

    break;

  case SOCK_ATTACH:

    return Tcp4AttachPcb (Sock);

    break;

  case SOCK_FLUSH:

    Tcp4FlushPcb (Tcb);

    break;

  case SOCK_DETACH:

    Tcp4DetachPcb (Sock);

    break;

  case SOCK_CONFIGURE:

    return Tcp4ConfigurePcb (
            Sock,
            (EFI_TCP4_CONFIG_DATA *) Data
            );

    break;

  case SOCK_MODE:

    ASSERT (Data && Tcb);

    return Tcp4GetMode (Tcb, (TCP4_MODE_DATA *) Data);

    break;

  case SOCK_ROUTE:

    ASSERT (Data && Tcb);

    return Tcp4Route (Tcb, (TCP4_ROUTE_INFO *) Data);

  }

  return EFI_SUCCESS;

}
