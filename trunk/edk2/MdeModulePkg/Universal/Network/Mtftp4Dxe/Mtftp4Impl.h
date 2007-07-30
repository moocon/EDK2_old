/** @file

Copyright (c) 2006 - 2007, Intel Corporation
All rights reserved. This program and the accompanying materials
are licensed and made available under the terms and conditions of the BSD License
which accompanies this distribution.  The full text of the license may be found at
http://opensource.org/licenses/bsd-license.php

THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

Module Name:

  Mtftp4Impl.h

Abstract:

 Mtftp4 Implementation, it supports the following RFCs:
   RFC1350 - THE TFTP PROTOCOL (REVISION 2)
   RFC2090 - TFTP Multicast Option
   RFC2347 - TFTP Option Extension
   RFC2348 - TFTP Blocksize Option
   RFC2349 - TFTP Timeout Interval and Transfer Size Options


**/

#ifndef __EFI_MTFTP4_IMPL_H__
#define __EFI_MTFTP4_IMPL_H__

#include <PiDxe.h>

#include <Protocol/Udp4.h>
#include <Protocol/Mtftp4.h>

#include <Library/DebugLib.h>
#include <Library/UefiDriverEntryPoint.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiLib.h>
#include <Library/BaseLib.h>
#include <Library/UdpIoLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/BaseMemoryLib.h>

typedef struct _MTFTP4_SERVICE  MTFTP4_SERVICE;
typedef struct _MTFTP4_PROTOCOL MTFTP4_PROTOCOL;

#include "Mtftp4Driver.h"
#include "Mtftp4Option.h"
#include "Mtftp4Support.h"

enum {
  MTFTP4_SERVICE_SIGNATURE   = EFI_SIGNATURE_32 ('T', 'F', 'T', 'P'),
  MTFTP4_PROTOCOL_SIGNATURE  = EFI_SIGNATURE_32 ('t', 'f', 't', 'p'),

  MTFTP4_DEFAULT_SERVER_PORT = 69,
  MTFTP4_DEFAULT_TIMEOUT     = 3,
  MTFTP4_DEFAULT_RETRY       = 5,
  MTFTP4_DEFAULT_BLKSIZE     = 512,
  MTFTP4_TIME_TO_GETMAP      = 5,

  MTFTP4_STATE_UNCONFIGED    = 0,
  MTFTP4_STATE_CONFIGED,
  MTFTP4_STATE_DESTORY,
};

typedef struct _MTFTP4_SERVICE {
  UINT32                        Signature;
  EFI_SERVICE_BINDING_PROTOCOL  ServiceBinding;

  BOOLEAN                       InDestory;

  UINT16                        ChildrenNum;
  NET_LIST_ENTRY                Children;

  EFI_EVENT                     Timer;  // Ticking timer for all the MTFTP clients
  EFI_EVENT                     TimerToGetMap;

  EFI_HANDLE                    Controller;
  EFI_HANDLE                    Image;

  //
  // This UDP child is used to keep the connection between the UDP
  // and MTFTP, so MTFTP will be notified when UDP is uninstalled.
  //
  UDP_IO_PORT                   *ConnectUdp;
};

typedef struct _MTFTP4_PROTOCOL {
  UINT32                        Signature;
  NET_LIST_ENTRY                Link;
  EFI_MTFTP4_PROTOCOL           Mtftp4;

  INTN                          State;
  BOOLEAN                       Indestory;

  MTFTP4_SERVICE                *Service;
  EFI_HANDLE                    Handle;

  EFI_MTFTP4_CONFIG_DATA        Config;

  //
  // Operation parameters: token and requested options.
  //
  EFI_MTFTP4_TOKEN              *Token;
  MTFTP4_OPTION                 RequestOption;
  UINT16                        Operation;

  //
  // Blocks is a list of MTFTP4_BLOCK_RANGE which contains
  // holes in the file
  //
  UINT16                        BlkSize;
  UINT16                        LastBlock;
  NET_LIST_ENTRY                Blocks;

  //
  // The server's communication end point: IP and two ports. one for
  // initial request, one for its selected port.
  //
  IP4_ADDR                      ServerIp;
  UINT16                        ListeningPort;
  UINT16                        ConnectedPort;
  IP4_ADDR                      Gateway;
  UDP_IO_PORT                   *UnicastPort;

  //
  // Timeout and retransmit status
  //
  NET_BUF                       *LastPacket;
  UINT32                        PacketToLive;
  UINT32                        CurRetry;
  UINT32                        MaxRetry;
  UINT32                        Timeout;

  //
  // Parameter used by RRQ's multicast download.
  //
  IP4_ADDR                      McastIp;
  UINT16                        McastPort;
  BOOLEAN                       Master;
  UDP_IO_PORT                   *McastUdpPort;
};

typedef struct {
  EFI_MTFTP4_PACKET             **Packet;
  UINT32                        *PacketLen;
  EFI_STATUS                    Status;
} MTFTP4_GETINFO_STATE;

VOID
Mtftp4CleanOperation (
  IN MTFTP4_PROTOCOL            *Instance,
  IN EFI_STATUS                 Result
  );

EFI_STATUS
Mtftp4WrqStart (
  IN MTFTP4_PROTOCOL            *Instance,
  IN UINT16                     Operation
  );

EFI_STATUS
Mtftp4RrqStart (
  IN MTFTP4_PROTOCOL            *Instance,
  IN UINT16                     Operation
  );

#define MTFTP4_SERVICE_FROM_THIS(a)   \
  CR (a, MTFTP4_SERVICE, ServiceBinding, MTFTP4_SERVICE_SIGNATURE)

#define MTFTP4_PROTOCOL_FROM_THIS(a)  \
  CR (a, MTFTP4_PROTOCOL, Mtftp4, MTFTP4_PROTOCOL_SIGNATURE)

extern EFI_MTFTP4_PROTOCOL  gMtftp4ProtocolTemplate;
#endif
