/** @file

Copyright (c) 2005 - 2007, Intel Corporation
All rights reserved. This program and the accompanying materials
are licensed and made available under the terms and conditions of the BSD License
which accompanies this distribution.  The full text of the license may be found at
http://opensource.org/licenses/bsd-license.php

THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.


Module Name:

  Ip4If.c

Abstract:

  Implement IP4 pesudo interface.


**/

#include "Ip4Impl.h"

//
// Mac address with all zero, used to determine whethter the ARP
// resolve succeeded. Failed ARP requests zero the MAC address buffer.
//
STATIC EFI_MAC_ADDRESS  mZeroMacAddress;

STATIC
VOID
EFIAPI
Ip4OnFrameSent (
  IN EFI_EVENT              Event,
  IN VOID                   *Context
  );

STATIC
VOID
EFIAPI
Ip4OnArpResolved (
  IN EFI_EVENT              Event,
  IN VOID                   *Context
  );

STATIC
VOID
EFIAPI
Ip4OnFrameReceived (
  IN EFI_EVENT              Event,
  IN VOID                   *Context
  );

STATIC
VOID
Ip4CancelFrameArp (
  IN IP4_ARP_QUE            *ArpQue,
  IN EFI_STATUS             IoStatus,
  IN IP4_FRAME_TO_CANCEL    FrameToCancel, OPTIONAL
  IN VOID                   *Context
  );


/**
  Wrap a transmit request into a newly allocated IP4_LINK_TX_TOKEN.

  @param  Interface             The interface to send out from
  @param  IpInstance            The IpInstance that transmit the packet.  NULL if
                                the packet is sent by the IP4 driver itself.
  @param  Packet                The packet to transmit
  @param  CallBack              Call back function to execute if transmission
                                finished.
  @param  Context               Opaque parameter to the call back.

  @return The wrapped token if succeed or NULL

**/
STATIC
IP4_LINK_TX_TOKEN *
Ip4WrapLinkTxToken (
  IN IP4_INTERFACE          *Interface,
  IN IP4_PROTOCOL           *IpInstance,    OPTIONAL
  IN NET_BUF                *Packet,
  IN IP4_FRAME_CALLBACK     CallBack,
  IN VOID                   *Context
  )
{
  EFI_MANAGED_NETWORK_COMPLETION_TOKEN  *MnpToken;
  EFI_MANAGED_NETWORK_TRANSMIT_DATA     *MnpTxData;
  IP4_LINK_TX_TOKEN                     *Token;
  EFI_STATUS                            Status;
  UINT32                                Count;

  Token = NetAllocatePool (sizeof (IP4_LINK_TX_TOKEN) + \
            (Packet->BlockOpNum - 1) * sizeof (EFI_MANAGED_NETWORK_FRAGMENT_DATA));

  if (Token == NULL) {
    return NULL;
  }

  Token->Signature = IP4_FRAME_TX_SIGNATURE;
  NetListInit (&Token->Link);

  Token->Interface  = Interface;
  Token->IpInstance = IpInstance;
  Token->CallBack   = CallBack;
  Token->Packet     = Packet;
  Token->Context    = Context;
  CopyMem (&Token->DstMac, &mZeroMacAddress, sizeof (Token->DstMac));
  CopyMem (&Token->SrcMac, &Interface->Mac, sizeof (Token->SrcMac));

  MnpToken          = &(Token->MnpToken);
  MnpToken->Status  = EFI_NOT_READY;

  Status = gBS->CreateEvent (
                  EVT_NOTIFY_SIGNAL,
                  TPL_CALLBACK,
                  Ip4OnFrameSent,
                  Token,
                  &MnpToken->Event
                  );

  if (EFI_ERROR (Status)) {
    NetFreePool (Token);
    return NULL;
  }

  MnpTxData                     = &Token->MnpTxData;
  MnpToken->Packet.TxData       = MnpTxData;

  MnpTxData->DestinationAddress = &Token->DstMac;
  MnpTxData->SourceAddress      = &Token->SrcMac;
  MnpTxData->ProtocolType       = IP4_ETHER_PROTO;
  MnpTxData->DataLength         = Packet->TotalSize;
  MnpTxData->HeaderLength       = 0;

  Count                         = Packet->BlockOpNum;

  NetbufBuildExt (Packet, (NET_FRAGMENT *) MnpTxData->FragmentTable, &Count);
  MnpTxData->FragmentCount      = (UINT16)Count;

  return Token;
}


/**
  Free the link layer transmit token. It will close the event
  then free the memory used.

  @param  Token                 Token to free

  @return NONE

**/
STATIC
VOID
Ip4FreeLinkTxToken (
  IN IP4_LINK_TX_TOKEN      *Token
  )
{
  NET_CHECK_SIGNATURE (Token, IP4_FRAME_TX_SIGNATURE);

  gBS->CloseEvent (Token->MnpToken.Event);
  NetFreePool (Token);
}


/**
  Create an IP_ARP_QUE structure to request ARP service.

  @param  Interface             The interface to send ARP from.
  @param  DestIp                The destination IP (host byte order) to request MAC
                                for

  @return Point to newly created IP4_ARP_QUE if succeed, otherwise NULL.

**/
STATIC
IP4_ARP_QUE *
Ip4CreateArpQue (
  IN IP4_INTERFACE          *Interface,
  IN IP4_ADDR               DestIp
  )
{
  IP4_ARP_QUE               *ArpQue;
  EFI_STATUS                Status;

  ArpQue = NetAllocatePool (sizeof (IP4_ARP_QUE));

  if (ArpQue == NULL) {
    return NULL;
  }

  ArpQue->Signature = IP4_FRAME_ARP_SIGNATURE;
  NetListInit (&ArpQue->Link);

  NetListInit (&ArpQue->Frames);
  ArpQue->Interface = Interface;

  Status = gBS->CreateEvent (
                  EVT_NOTIFY_SIGNAL,
                  TPL_CALLBACK,
                  Ip4OnArpResolved,
                  ArpQue,
                  &ArpQue->OnResolved
                  );

  if (EFI_ERROR (Status)) {
    NetFreePool (ArpQue);
    return NULL;
  }

  ArpQue->Ip  = DestIp;
  CopyMem (&ArpQue->Mac, &mZeroMacAddress, sizeof (ArpQue->Mac));

  return ArpQue;
}


/**
  Remove all the transmit requests queued on the ARP queue, then free it.

  @param  ArpQue                Arp queue to free
  @param  IoStatus              The transmit status returned to transmit requests'
                                callback.

  @return NONE

**/
STATIC
VOID
Ip4FreeArpQue (
  IN IP4_ARP_QUE            *ArpQue,
  IN EFI_STATUS             IoStatus
  )
{
  NET_CHECK_SIGNATURE (ArpQue, IP4_FRAME_ARP_SIGNATURE);

  //
  // Remove all the frame waiting the ARP response
  //
  Ip4CancelFrameArp (ArpQue, IoStatus, NULL, NULL);

  gBS->CloseEvent (ArpQue->OnResolved);
  NetFreePool (ArpQue);
}


/**
  Create a link layer receive token to wrap the receive request

  @param  Interface             The interface to receive from
  @param  IpInstance            The instance that request the receive (NULL for IP4
                                driver itself)
  @param  CallBack              Call back function to execute when finished.
  @param  Context               Opaque parameters to the callback

  @return Point to created IP4_LINK_RX_TOKEN if succeed, otherwise NULL.

**/
STATIC
IP4_LINK_RX_TOKEN *
Ip4CreateLinkRxToken (
  IN IP4_INTERFACE          *Interface,
  IN IP4_PROTOCOL           *IpInstance,
  IN IP4_FRAME_CALLBACK     CallBack,
  IN VOID                   *Context
  )
{
  EFI_MANAGED_NETWORK_COMPLETION_TOKEN  *MnpToken;
  IP4_LINK_RX_TOKEN                     *Token;
  EFI_STATUS                            Status;

  Token = NetAllocatePool (sizeof (IP4_LINK_RX_TOKEN));
  if (Token == NULL) {
    return NULL;
  }

  Token->Signature  = IP4_FRAME_RX_SIGNATURE;
  Token->Interface  = Interface;
  Token->IpInstance = IpInstance;
  Token->CallBack   = CallBack;
  Token->Context    = Context;

  MnpToken          = &Token->MnpToken;
  MnpToken->Status  = EFI_NOT_READY;

  Status = gBS->CreateEvent (
                  EVT_NOTIFY_SIGNAL,
                  TPL_CALLBACK,
                  Ip4OnFrameReceived,
                  Token,
                  &MnpToken->Event
                  );

  if (EFI_ERROR (Status)) {
    NetFreePool (Token);
    return NULL;
  }

  MnpToken->Packet.RxData = NULL;
  return Token;
}


/**
  Free the link layer request token. It will close the event
  then free the memory used.

  @param  Token                 Request token to free

  @return NONE

**/
STATIC
VOID
Ip4FreeFrameRxToken (
  IN IP4_LINK_RX_TOKEN      *Token
  )
{

  NET_CHECK_SIGNATURE (Token, IP4_FRAME_RX_SIGNATURE);

  gBS->CloseEvent (Token->MnpToken.Event);
  NetFreePool (Token);
}


/**
  Remove all the frames on the ARP queue that pass the FrameToCancel,
  that is, either FrameToCancel is NULL or it returns true for the frame.

  @param  ArpQue                ARP frame to remove the frames from.
  @param  IoStatus              The status returned to the cancelled frames'
                                callback function.
  @param  FrameToCancel         Function to select which frame to cancel.
  @param  Context               Opaque parameter to the FrameToCancel.

  @return NONE

**/
STATIC
VOID
Ip4CancelFrameArp (
  IN IP4_ARP_QUE            *ArpQue,
  IN EFI_STATUS             IoStatus,
  IN IP4_FRAME_TO_CANCEL    FrameToCancel, OPTIONAL
  IN VOID                   *Context
  )
{
  NET_LIST_ENTRY            *Entry;
  NET_LIST_ENTRY            *Next;
  IP4_LINK_TX_TOKEN         *Token;

  NET_LIST_FOR_EACH_SAFE (Entry, Next, &ArpQue->Frames) {
    Token = NET_LIST_USER_STRUCT (Entry, IP4_LINK_TX_TOKEN, Link);

    if ((FrameToCancel == NULL) || FrameToCancel (Token, Context)) {
      NetListRemoveEntry (Entry);

      Token->CallBack (Token->IpInstance, Token->Packet, IoStatus, 0, Token->Context);
      Ip4FreeLinkTxToken (Token);
    }
  }
}


/**
  Remove all the frames on the interface that pass the FrameToCancel,
  either queued on ARP queues or that have already been delivered to
  MNP and not yet recycled.

  @param  Interface             Interface to remove the frames from
  @param  IoStatus              The transmit status returned to the frames'
                                callback
  @param  FrameToCancel         Function to select the frame to cancel, NULL to
                                select all
  @param  Context               Opaque parameters passed to FrameToCancel

  @return NONE

**/
VOID
Ip4CancelFrames (
  IN IP4_INTERFACE          *Interface,
  IN EFI_STATUS             IoStatus,
  IN IP4_FRAME_TO_CANCEL    FrameToCancel,   OPTIONAL
  IN VOID                   *Context
  )
{
  NET_LIST_ENTRY            *Entry;
  NET_LIST_ENTRY            *Next;
  IP4_ARP_QUE               *ArpQue;
  IP4_LINK_TX_TOKEN         *Token;

  //
  // Cancel all the pending frames on ARP requests
  //
  NET_LIST_FOR_EACH_SAFE (Entry, Next, &Interface->ArpQues) {
    ArpQue = NET_LIST_USER_STRUCT (Entry, IP4_ARP_QUE, Link);

    Ip4CancelFrameArp (ArpQue, IoStatus, FrameToCancel, Context);

    if (NetListIsEmpty (&ArpQue->Frames)) {
      NetListRemoveEntry (Entry);

      Interface->Arp->Cancel (Interface->Arp, &ArpQue->Ip, ArpQue->OnResolved);
      Ip4FreeArpQue (ArpQue, EFI_ABORTED);
    }
  }

  //
  // Cancel all the frames that have been delivered to MNP
  // but not yet recycled.
  //
  NET_LIST_FOR_EACH_SAFE (Entry, Next, &Interface->SentFrames) {
    Token = NET_LIST_USER_STRUCT (Entry, IP4_LINK_TX_TOKEN, Link);

    if ((FrameToCancel == NULL) || FrameToCancel (Token, Context)) {
      NetListRemoveEntry (Entry);

      Interface->Mnp->Cancel (Interface->Mnp, &Token->MnpToken);
      Token->CallBack (Token->IpInstance, Token->Packet, IoStatus, 0, Token->Context);
      Ip4FreeLinkTxToken (Token);
    }
  }
}


/**
  Create an IP4_INTERFACE. Delay the creation of ARP instance until
  the interface is configured.

  @param  Mnp                   The shared MNP child of this IP4 service binding
                                instance
  @param  Controller            The controller this IP4 service binding instance
                                is installed. Most like the UNDI handle.
  @param  ImageHandle           This driver's image handle

  @return Point to the created IP4_INTERFACE, otherwise NULL.

**/
IP4_INTERFACE *
Ip4CreateInterface (
  IN  EFI_MANAGED_NETWORK_PROTOCOL  *Mnp,
  IN  EFI_HANDLE                    Controller,
  IN  EFI_HANDLE                    ImageHandle
  )
{
  IP4_INTERFACE             *Interface;
  EFI_SIMPLE_NETWORK_MODE   SnpMode;

  Interface = NetAllocatePool (sizeof (IP4_INTERFACE));

  if ((Interface == NULL) || (Mnp == NULL)) {
    return NULL;
  }

  Interface->Signature = IP4_INTERFACE_SIGNATURE;
  NetListInit (&Interface->Link);
  Interface->RefCnt     = 1;

  Interface->Ip         = IP4_ALLZERO_ADDRESS;
  Interface->SubnetMask = IP4_ALLZERO_ADDRESS;
  Interface->Configured = FALSE;

  Interface->Controller = Controller;
  Interface->Image      = ImageHandle;
  Interface->Mnp        = Mnp;
  Interface->Arp        = NULL;
  Interface->ArpHandle  = NULL;

  NetListInit (&Interface->ArpQues);
  NetListInit (&Interface->SentFrames);

  Interface->RecvRequest = NULL;

  //
  // Get the interface's Mac address and broadcast mac address from SNP
  //
  if (EFI_ERROR (Mnp->GetModeData (Mnp, NULL, &SnpMode))) {
    NetFreePool (Interface);
    return NULL;
  }

  CopyMem (&Interface->Mac, &SnpMode.CurrentAddress, sizeof (Interface->Mac));
  CopyMem (&Interface->BroadcastMac, &SnpMode.BroadcastAddress, sizeof (Interface->BroadcastMac));
  Interface->HwaddrLen    = SnpMode.HwAddressSize;

  NetListInit (&Interface->IpInstances);
  Interface->PromiscRecv = FALSE;

  return Interface;
}


/**
  Set the interface's address, create and configure
  the ARP child if necessary.

  @param  Interface             The interface to set the address
  @param  IpAddr                The interface's IP address
  @param  SubnetMask            The interface's netmask

  @retval EFI_SUCCESS           The interface is configured with Ip/netmask pair,
                                and a ARP is created for it.
  @retval Others                Failed to set the interface's address.

**/
EFI_STATUS
Ip4SetAddress (
  IN  IP4_INTERFACE         *Interface,
  IN  IP4_ADDR              IpAddr,
  IN  IP4_ADDR              SubnetMask
  )
{
  EFI_ARP_CONFIG_DATA       ArpConfig;
  EFI_STATUS                Status;
  INTN                      Type;
  INTN                      Len;
  IP4_ADDR                  Netmask;

  NET_CHECK_SIGNATURE (Interface, IP4_INTERFACE_SIGNATURE);

  ASSERT (!Interface->Configured);

  //
  // Set the ip/netmask, then compute the subnet broadcast
  // and network broadcast for easy access. When computing
  // nework broadcast, the subnet mask is most like longer
  // than the default netmask (not subneted) as defined in
  // RFC793. If that isn't the case, we are aggregating the
  // networks, use the subnet's mask instead.
  //
  Interface->Ip             = IpAddr;
  Interface->SubnetMask     = SubnetMask;
  Interface->SubnetBrdcast  = (IpAddr | ~SubnetMask);

  Type                      = NetGetIpClass (IpAddr);
  Len                       = NetGetMaskLength (SubnetMask);
  Netmask                   = mIp4AllMasks[NET_MIN (Len, Type << 3)];
  Interface->NetBrdcast     = (IpAddr | ~Netmask);

  //
  // If the address is NOT all zero, create then configure an ARP child.
  // Pay attention: DHCP configures its station address as 0.0.0.0/0
  //
  Interface->Arp            = NULL;
  Interface->ArpHandle      = NULL;

  if (IpAddr != IP4_ALLZERO_ADDRESS) {
    Status = NetLibCreateServiceChild (
               Interface->Controller,
               Interface->Image,
               &gEfiArpServiceBindingProtocolGuid,
               &Interface->ArpHandle
               );

    if (EFI_ERROR (Status)) {
      return Status;;
    }

    Status = gBS->OpenProtocol (
                    Interface->ArpHandle,
                    &gEfiArpProtocolGuid,
                    (VOID **) &Interface->Arp,
                    Interface->Image,
                    Interface->Controller,
                    EFI_OPEN_PROTOCOL_BY_DRIVER
                    );

    if (EFI_ERROR (Status)) {
      goto ON_ERROR;
    }

    IpAddr                    = HTONL (IpAddr);
    ArpConfig.SwAddressType   = IP4_ETHER_PROTO;
    ArpConfig.SwAddressLength = 4;
    ArpConfig.StationAddress  = &IpAddr;
    ArpConfig.EntryTimeOut    = 0;
    ArpConfig.RetryCount      = 0;
    ArpConfig.RetryTimeOut    = 0;

    Status = Interface->Arp->Configure (Interface->Arp, &ArpConfig);

    if (EFI_ERROR (Status)) {
      gBS->CloseProtocol (
            Interface->ArpHandle,
            &gEfiArpProtocolGuid,
            Interface->Image,
            Interface->Controller
            );

      goto ON_ERROR;
    }
  }

  Interface->Configured = TRUE;
  return EFI_SUCCESS;

ON_ERROR:
  NetLibDestroyServiceChild (
    Interface->Controller,
    Interface->Image,
    &gEfiArpServiceBindingProtocolGuid,
    &Interface->ArpHandle
    );

  return Status;
}


/**
  Fileter function to cancel all the frame related to an IP instance.

  @param  Frame                 The transmit request to test whether to cancel
  @param  Context               The context which is the Ip instance that issued
                                the transmit.

  @retval TRUE                  The frame belongs to this instance and is to be
                                removed
  @retval FALSE                 The frame doesn't belong to this instance.

**/
STATIC
BOOLEAN
Ip4CancelInstanceFrame (
  IN IP4_LINK_TX_TOKEN *Frame,
  IN VOID              *Context
  )
{
  if (Frame->IpInstance == (IP4_PROTOCOL *) Context) {
    return TRUE;
  }

  return FALSE;
}



/**
  If there is a pending receive request, cancel it. Don't call
  the receive request's callback because this function can be only
  called if the instance or driver is tearing itself down. It
  doesn't make sense to call it back. But it is necessary to call
  the transmit token's callback to give it a chance to free the
  packet and update the upper layer's transmit request status, say
  that from the UDP.

  @param  Interface             The interface used by the IpInstance

  @return None

**/
VOID
Ip4CancelReceive (
  IN IP4_INTERFACE          *Interface
  )
{
  EFI_TPL                   OldTpl;
  IP4_LINK_RX_TOKEN         *Token;

  if ((Token = Interface->RecvRequest) != NULL) {
    OldTpl = NET_RAISE_TPL (NET_TPL_LOCK);

    Interface->RecvRequest = NULL;
    Interface->Mnp->Cancel (Interface->Mnp, &Token->MnpToken);
    Ip4FreeFrameRxToken (Token);

    NET_RESTORE_TPL (OldTpl);
  }
}


/**
  Free the interface used by IpInstance. All the IP instance with
  the same Ip/Netmask pair share the same interface. It is reference
  counted. All the frames haven't been sent will be cancelled.
  Because the IpInstance is optional, the caller must remove
  IpInstance from the interface's instance list itself.

  @param  Interface             The interface used by the IpInstance
  @param  IpInstance            The Ip instance that free the interface. NULL if
                                the  Ip driver is releasing the default interface.

  @retval EFI_SUCCESS           The interface use IpInstance is freed.

**/
EFI_STATUS
Ip4FreeInterface (
  IN  IP4_INTERFACE         *Interface,
  IN  IP4_PROTOCOL          *IpInstance           OPTIONAL
  )
{
  NET_CHECK_SIGNATURE (Interface, IP4_INTERFACE_SIGNATURE);
  ASSERT (Interface->RefCnt > 0);

  //
  // Remove all the pending transmit token related to this IP instance.
  //
  Ip4CancelFrames (Interface, EFI_ABORTED, Ip4CancelInstanceFrame, IpInstance);

  if (--Interface->RefCnt > 0) {
    return EFI_SUCCESS;
  }

  //
  // Destory the interface if this is the last IP instance that
  // has the address. Remove all the system transmitted packets
  // from this interface, cancel the receive request if there is
  // one, and destory the ARP requests.
  //
  Ip4CancelFrames (Interface, EFI_ABORTED, Ip4CancelInstanceFrame, NULL);
  Ip4CancelReceive (Interface);

  ASSERT (NetListIsEmpty (&Interface->IpInstances));
  ASSERT (NetListIsEmpty (&Interface->ArpQues));
  ASSERT (NetListIsEmpty (&Interface->SentFrames));

  if (Interface->Arp != NULL) {
    gBS->CloseProtocol (
          Interface->ArpHandle,
          &gEfiArpProtocolGuid,
          Interface->Image,
          Interface->Controller
          );

    NetLibDestroyServiceChild (
      Interface->Controller,
      Interface->Image,
      &gEfiArpServiceBindingProtocolGuid,
      Interface->ArpHandle
      );
  }

  NetListRemoveEntry (&Interface->Link);
  NetFreePool (Interface);

  return EFI_SUCCESS;
}


/**
  Callback function when ARP request are finished. It will cancelled
  all the queued frame if the ARP requests failed. Or transmit them
  if the request succeed.

  @param  Event                 The Arp request event
  @param  Context               The context of the callback, a point to the ARP
                                queue

  @return None

**/
STATIC
VOID
EFIAPI
Ip4OnArpResolved (
  IN EFI_EVENT              Event,
  IN VOID                   *Context
  )
{
  NET_LIST_ENTRY            *Entry;
  NET_LIST_ENTRY            *Next;
  IP4_ARP_QUE               *ArpQue;
  IP4_INTERFACE             *Interface;
  IP4_LINK_TX_TOKEN         *Token;
  EFI_STATUS                Status;

  ArpQue = (IP4_ARP_QUE *) Context;
  NET_CHECK_SIGNATURE (ArpQue, IP4_FRAME_ARP_SIGNATURE);

  NetListRemoveEntry (&ArpQue->Link);

  //
  // ARP resolve failed for some reason. Release all the frame
  // and ARP queue itself. Ip4FreeArpQue will call the frame's
  // owner back.
  //
  if (NET_MAC_EQUAL (&ArpQue->Mac, &mZeroMacAddress, ArpQue->Interface->HwaddrLen)) {
    Ip4FreeArpQue (ArpQue, EFI_NO_MAPPING);

    return ;
  }

  //
  // ARP resolve succeeded, Transmit all the frame. Release the ARP
  // queue. It isn't necessary for us to cache the ARP binding because
  // we always check the ARP cache first before transmit.
  //
  Interface = ArpQue->Interface;

  NET_LIST_FOR_EACH_SAFE (Entry, Next, &ArpQue->Frames) {
    NetListRemoveEntry (Entry);

    Token         = NET_LIST_USER_STRUCT (Entry, IP4_LINK_TX_TOKEN, Link);
    CopyMem (&Token->DstMac, &ArpQue->Mac, sizeof (Token->DstMac));

    Status = Interface->Mnp->Transmit (Interface->Mnp, &Token->MnpToken);

    if (EFI_ERROR (Status)) {
      Token->CallBack (Token->IpInstance, Token->Packet, Status, 0, Token->Context);

      Ip4FreeLinkTxToken (Token);
      continue;
    }

    NetListInsertTail (&Interface->SentFrames, &Token->Link);
  }

  Ip4FreeArpQue (ArpQue, EFI_SUCCESS);
}


/**
  Callback funtion when frame transmission is finished. It will
  call the frame owner's callback function to tell it the result.

  @param  Event                 The transmit token's event
  @param  Context               Context which is point to the token.

  @return None.

**/
STATIC
VOID
EFIAPI
Ip4OnFrameSent (
  IN EFI_EVENT               Event,
  IN VOID                    *Context
  )
{
  IP4_LINK_TX_TOKEN         *Token;

  Token = (IP4_LINK_TX_TOKEN *) Context;
  NET_CHECK_SIGNATURE (Token, IP4_FRAME_TX_SIGNATURE);

  NetListRemoveEntry (&Token->Link);

  Token->CallBack (
          Token->IpInstance,
          Token->Packet,
          Token->MnpToken.Status,
          0,
          Token->Context
          );

  Ip4FreeLinkTxToken (Token);
}



/**
  Send a frame from the interface. If the next hop is broadcast or
  multicast address, it is transmitted immediately. If the next hop
  is a unicast, it will consult ARP to resolve the NextHop's MAC.
  If some error happened, the CallBack won't be called. So, the caller
  must test the return value, and take action when there is an error.

  @param  Interface             The interface to send the frame from
  @param  IpInstance            The IP child that request the transmission.  NULL
                                if it is the IP4 driver itself.
  @param  Packet                The packet to transmit.
  @param  NextHop               The immediate destination to transmit the packet
                                to.
  @param  CallBack              Function to call back when transmit finished.
  @param  Context               Opaque parameter to the call back.

  @retval EFI_OUT_OF_RESOURCES  Failed to allocate resource to send the frame
  @retval EFI_NO_MAPPING        Can't resolve the MAC for the nexthop
  @retval EFI_SUCCESS           The packet is successfully transmitted.

**/
EFI_STATUS
Ip4SendFrame (
  IN  IP4_INTERFACE         *Interface,
  IN  IP4_PROTOCOL          *IpInstance,      OPTIONAL
  IN  NET_BUF               *Packet,
  IN  IP4_ADDR              NextHop,
  IN  IP4_FRAME_CALLBACK    CallBack,
  IN  VOID                  *Context
  )
{
  IP4_LINK_TX_TOKEN         *Token;
  NET_LIST_ENTRY            *Entry;
  IP4_ARP_QUE               *ArpQue;
  EFI_ARP_PROTOCOL          *Arp;
  EFI_STATUS                Status;

  ASSERT (Interface->Configured);

  Token = Ip4WrapLinkTxToken (Interface, IpInstance, Packet, CallBack, Context);

  if (Token == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }

  //
  // Get the destination MAC address for multicast and broadcasts.
  // Don't depend on ARP to solve the address since there maybe no
  // ARP at all. Ip4Output has set NextHop to 255.255.255.255 for
  // all the broadcasts.
  //
  if (NextHop == IP4_ALLONE_ADDRESS) {
    CopyMem (&Token->DstMac, &Interface->BroadcastMac, sizeof (Token->DstMac));
    goto SEND_NOW;

  } else if (IP4_IS_MULTICAST (NextHop)) {

    Status = Ip4GetMulticastMac (Interface->Mnp, NextHop, &Token->DstMac);

    if (EFI_ERROR (Status)) {
      goto ON_ERROR;
    }

    goto SEND_NOW;
  }

  //
  // Can only send out multicast/broadcast if the IP address is zero
  //
  if ((Arp = Interface->Arp) == NULL) {
    Status = EFI_NO_MAPPING;
    goto ON_ERROR;
  }

  //
  // First check whether this binding is in the ARP cache.
  //
  NextHop = HTONL (NextHop);
  Status  = Arp->Request (Arp, &NextHop, NULL, &Token->DstMac);

  if (Status == EFI_SUCCESS) {
    goto SEND_NOW;

  } else if (Status != EFI_NOT_READY) {
    goto ON_ERROR;
  }

  //
  // Have to do asynchronous ARP resolution. First check
  // whether there is already a pending request.
  //
  ArpQue = NULL;

  NET_LIST_FOR_EACH (Entry, &Interface->ArpQues) {
    ArpQue = NET_LIST_USER_STRUCT (Entry, IP4_ARP_QUE, Link);

    if (ArpQue->Ip == NextHop) {
      break;
    }
  }

  //
  // Found a pending ARP request, enqueue the frame then return
  //
  if (Entry != &Interface->ArpQues) {
    NetListInsertTail (&ArpQue->Frames, &Token->Link);
    return EFI_SUCCESS;
  }

  //
  // First frame to NextHop, issue an asynchronous ARP requests
  //
  ArpQue = Ip4CreateArpQue (Interface, NextHop);

  if (ArpQue == NULL) {
    Status = EFI_OUT_OF_RESOURCES;
    goto ON_ERROR;
  }

  Status = Arp->Request (Arp, &ArpQue->Ip, ArpQue->OnResolved, ArpQue->Mac.Addr);

  if (EFI_ERROR (Status) && (Status != EFI_NOT_READY)) {
    Ip4FreeArpQue (ArpQue, EFI_NO_MAPPING);
    goto ON_ERROR;
  }

  NetListInsertHead (&ArpQue->Frames, &Token->Link);
  NetListInsertHead (&Interface->ArpQues, &ArpQue->Link);
  return EFI_SUCCESS;

SEND_NOW:
  Status = Interface->Mnp->Transmit (Interface->Mnp, &Token->MnpToken);

  if (EFI_ERROR (Status)) {
    goto ON_ERROR;
  }

  NetListInsertTail (&Interface->SentFrames, &Token->Link);
  return EFI_SUCCESS;

ON_ERROR:
  Ip4FreeLinkTxToken (Token);
  return Status;
}


/**
  Call back function when the received packet is freed.
  Check Ip4OnFrameReceived for information.

  @param  Context               Context, which is the IP4_LINK_RX_TOKEN.

  @return None.

**/
STATIC
VOID
Ip4RecycleFrame (
  IN VOID                   *Context
  )
{
  IP4_LINK_RX_TOKEN         *Frame;

  Frame = (IP4_LINK_RX_TOKEN *) Context;
  NET_CHECK_SIGNATURE (Frame, IP4_FRAME_RX_SIGNATURE);

  gBS->SignalEvent (Frame->MnpToken.Packet.RxData->RecycleEvent);
  Ip4FreeFrameRxToken (Frame);
}


/**
  Received a frame from MNP, wrap it in net buffer then deliver
  it to IP's input function. The ownship of the packet also
  transferred to IP. When Ip is finished with this packet, it
  will call NetbufFree to release the packet, NetbufFree will
  again call the Ip4RecycleFrame to signal MNP's event and free
  the token used.

  @param  Event                 The receive event delivered to MNP for receive.
  @param  Context               Context for the callback.

  @return None.

**/
STATIC
VOID
EFIAPI
Ip4OnFrameReceived (
  IN EFI_EVENT                Event,
  IN VOID                     *Context
  )
{
  EFI_MANAGED_NETWORK_COMPLETION_TOKEN  *MnpToken;
  EFI_MANAGED_NETWORK_RECEIVE_DATA      *MnpRxData;
  IP4_LINK_RX_TOKEN                     *Token;
  NET_FRAGMENT                          Netfrag;
  NET_BUF                               *Packet;
  UINT32                                Flag;

  Token = (IP4_LINK_RX_TOKEN *) Context;
  NET_CHECK_SIGNATURE (Token, IP4_FRAME_RX_SIGNATURE);

  //
  // First clear the interface's receive request in case the
  // caller wants to call Ip4ReceiveFrame in the callback.
  //
  Token->Interface->RecvRequest = NULL;

  MnpToken  = &Token->MnpToken;
  MnpRxData = MnpToken->Packet.RxData;

  if (EFI_ERROR (MnpToken->Status) || (MnpRxData == NULL)) {
    Token->CallBack (Token->IpInstance, NULL, MnpToken->Status, 0, Token->Context);
    Ip4FreeFrameRxToken (Token);

    return ;
  }

  //
  // Wrap the frame in a net buffer then deliever it to IP input.
  // IP will reassemble the packet, and deliver it to upper layer
  //
  Netfrag.Len  = MnpRxData->DataLength;
  Netfrag.Bulk = MnpRxData->PacketData;

  Packet = NetbufFromExt (&Netfrag, 1, 0, IP4_MAX_HEADLEN, Ip4RecycleFrame, Token);

  if (Packet == NULL) {
    gBS->SignalEvent (MnpRxData->RecycleEvent);

    Token->CallBack (Token->IpInstance, NULL, EFI_OUT_OF_RESOURCES, 0, Token->Context);
    Ip4FreeFrameRxToken (Token);

    return ;
  }

  Flag  = (MnpRxData->BroadcastFlag ? IP4_LINK_BROADCAST : 0);
  Flag |= (MnpRxData->MulticastFlag ? IP4_LINK_MULTICAST : 0);
  Flag |= (MnpRxData->PromiscuousFlag ? IP4_LINK_PROMISC : 0);

  Token->CallBack (Token->IpInstance, Packet, EFI_SUCCESS, Flag, Token->Context);
}


/**
  Request to receive the packet from the interface.

  @param  Interface             The interface to receive the frames from
  @param  IpInstance            The instance that requests the receive. NULL for
                                the driver itself.
  @param  CallBack              Function to call when receive finished.
  @param  Context               Opaque parameter to the callback

  @retval EFI_ALREADY_STARTED   There is already a pending receive request.
  @retval EFI_OUT_OF_RESOURCES  Failed to allocate resource to receive
  @retval EFI_SUCCESS           The recieve request has been started.

**/
EFI_STATUS
Ip4ReceiveFrame (
  IN  IP4_INTERFACE         *Interface,
  IN  IP4_PROTOCOL          *IpInstance,      OPTIONAL
  IN  IP4_FRAME_CALLBACK    CallBack,
  IN  VOID                  *Context
  )
{
  IP4_LINK_RX_TOKEN *Token;
  EFI_STATUS        Status;

  NET_CHECK_SIGNATURE (Interface, IP4_INTERFACE_SIGNATURE);

  if (Interface->RecvRequest != NULL) {
    return EFI_ALREADY_STARTED;
  }

  Token = Ip4CreateLinkRxToken (Interface, IpInstance, CallBack, Context);

  if (Token == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }

  Status = Interface->Mnp->Receive (Interface->Mnp, &Token->MnpToken);

  if (EFI_ERROR (Status)) {
    Ip4FreeFrameRxToken (Token);
    return Status;
  }

  Interface->RecvRequest = Token;
  return EFI_SUCCESS;
}
