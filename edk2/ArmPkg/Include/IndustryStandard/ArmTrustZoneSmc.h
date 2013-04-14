/** @file
*
*  Copyright (c) 2012-2013, ARM Limited. All rights reserved.
*
*  This program and the accompanying materials
*  are licensed and made available under the terms and conditions of the BSD License
*  which accompanies this distribution.  The full text of the license may be found at
*  http://opensource.org/licenses/bsd-license.php
*
*  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
*  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.
*
**/

#ifndef __ARM_TRUSTZONE_SMC_H__
#define __ARM_TRUSTZONE_SMC_H__

#define ARM_TRUSTZONE_UID_4LETTERID   0x1
#define ARM_TRUSTZONE_UID_MD5         0x2

#define ARM_TRUSTZONE_ARM_UID         0x40524d48 // "ARMH"

#define IS_ARM_TRUSTZONE_SUPPORTED_SMC(Rx,Region)       (((UINTN)(Rx) >= (UINTN)ARM_TRUSTZONE_##Region##_SMC_ID_START) && ((UINTN)(Rx) <= (UINTN)ARM_TRUSTZONE_##Region##_SMC_ID_END))

#define IS_ARM_TRUSTZONE_DEPRECIATED_SMC(Rx)            ((UINTN)(Rx) <= (UINTN)ARM_TRUSTZONE_DEPRECIATED_SMC_ID_END)
#define IS_ARM_TRUSTZONE_TRUSTED_OS_SMC(Rx)             IS_ARM_TRUSTZONE_SUPPORTED_SMC(Rx,TRUSTED_OS)
#define IS_ARM_TRUSTZONE_ARM_FAST_SMC(Rx)               IS_ARM_TRUSTZONE_SUPPORTED_SMC(Rx,ARM_FAST)
#define IS_ARM_TRUSTZONE_SIP_FAST_SMC(Rx)               IS_ARM_TRUSTZONE_SUPPORTED_SMC(Rx,SIP_FAST)
#define IS_ARM_TRUSTZONE_ODM_FAST_SMC(Rx)               IS_ARM_TRUSTZONE_SUPPORTED_SMC(Rx,ODM_FAST)
#define IS_ARM_TRUSTZONE_OEM_FAST_SMC(Rx)               IS_ARM_TRUSTZONE_SUPPORTED_SMC(Rx,OEM_FAST)
#define IS_ARM_TRUSTZONE_TRUSTED_USER_FAST_SMC(Rx)      IS_ARM_TRUSTZONE_SUPPORTED_SMC(Rx,TRUSTED_USER_FAST)
#define IS_ARM_TRUSTZONE_TRUSTED_OS_FAST_SMC(Rx)        IS_ARM_TRUSTZONE_SUPPORTED_SMC(Rx,TRUSTED_OS_FAST)

#define IS_ARM_TRUSTZONE_SUPPORTED_SMC_ID_PRESENCE(Rx,Region) ((Rx) == ARM_TRUSTZONE_##Region##_SMC_ID_PRESENCE)
#define IS_ARM_TRUSTZONE_SUPPORTED_SMC_ID_UID(Rx,Region)      (((Rx) == ARM_TRUSTZONE_##Region##_SMC_ID_UID)   || \
                                                               ((Rx) == ARM_TRUSTZONE_##Region##_SMC_ID_UID+1) || \
                                                               ((Rx) == ARM_TRUSTZONE_##Region##_SMC_ID_UID+2) || \
                                                               ((Rx) == ARM_TRUSTZONE_##Region##_SMC_ID_UID+3) || \
                                                               ((Rx) == ARM_TRUSTZONE_##Region##_SMC_ID_UID+4))
#define IS_ARM_TRUSTZONE_SUPPORTED_SMC_ID_REVISION(Rx,Region) (((Rx) == ARM_TRUSTZONE_##Region##_SMC_ID_REVISION)   || \
                                                               ((Rx) == ARM_TRUSTZONE_##Region##_SMC_ID_REVISION+1))
#define IS_ARM_TRUSTZONE_SUPPORTED_SMC_ID_RPC(Rx,Region)      (((Rx) >= ARM_TRUSTZONE_##Region##_SMC_ID_RPC_START) && \
                                                               ((Rx) <= ARM_TRUSTZONE_##Region##_SMC_ID_RPC_END))

#define ARM_TRUSTZONE_SUPPORTED_SMC_ID_UID_INDEX(Rx,Region)       ((Rx) - ARM_TRUSTZONE_##Region##_SMC_ID_UID)
#define ARM_TRUSTZONE_SUPPORTED_SMC_ID_REVISION_INDEX(Rx,Region)  ((Rx) - ARM_TRUSTZONE_##Region##_SMC_ID_REVISION)
#define ARM_TRUSTZONE_SUPPORTED_SMC_ID_RPC_INDEX(Rx,Region)       ((Rx) - ARM_TRUSTZONE_##Region##_SMC_ID_RPC_START)

#define ARM_TRUSTZONE_TRUSTED_OS_SMC_ID_RPC_INDEX(Rx)       ARM_TRUSTZONE_SUPPORTED_SMC_ID_RPC_INDEX(Rx,TRUSTED_OS)

#define IS_ARM_TRUSTZONE_ARM_FAST_SMC_ID_PRESENCE(Rx)       IS_ARM_TRUSTZONE_SUPPORTED_SMC_ID_PRESENCE(Rx,ARM_FAST)
#define IS_ARM_TRUSTZONE_ARM_FAST_SMC_ID_UID(Rx)            IS_ARM_TRUSTZONE_SUPPORTED_SMC_ID_UID(Rx,ARM_FAST)
#define IS_ARM_TRUSTZONE_ARM_FAST_SMC_ID_REVISION(Rx)       IS_ARM_TRUSTZONE_SUPPORTED_SMC_ID_REVISION(Rx,ARM_FAST)
#define IS_ARM_TRUSTZONE_ARM_FAST_SMC_ID_RPC(Rx)            IS_ARM_TRUSTZONE_SUPPORTED_SMC_ID_RPC(Rx,ARM_FAST)
#define ARM_TRUSTZONE_ARM_FAST_SMC_ID_UID_INDEX(Rx)         ARM_TRUSTZONE_SUPPORTED_SMC_ID_UID_INDEX(Rx,ARM_FAST)
#define ARM_TRUSTZONE_ARM_FAST_SMC_ID_REVISION_INDEX(Rx)    ARM_TRUSTZONE_SUPPORTED_SMC_ID_REVISION_INDEX(Rx,ARM_FAST)
#define ARM_TRUSTZONE_ARM_FAST_SMC_ID_RPC_INDEX(Rx)         ARM_TRUSTZONE_SUPPORTED_SMC_ID_RPC_INDEX(Rx,ARM_FAST)

#define IS_ARM_TRUSTZONE_ODM_FAST_SMC_ID_PRESENCE(Rx)       IS_ARM_TRUSTZONE_SUPPORTED_SMC_ID_PRESENCE(Rx,ODM_FAST)
#define IS_ARM_TRUSTZONE_ODM_FAST_SMC_ID_UID(Rx)            IS_ARM_TRUSTZONE_SUPPORTED_SMC_ID_UID(Rx,ODM_FAST)
#define IS_ARM_TRUSTZONE_ODM_FAST_SMC_ID_REVISION(Rx)       IS_ARM_TRUSTZONE_SUPPORTED_SMC_ID_REVISION(Rx,ODM_FAST)
#define IS_ARM_TRUSTZONE_ODM_FAST_SMC_ID_RPC(Rx)            IS_ARM_TRUSTZONE_SUPPORTED_SMC_ID_RPC(Rx,ODM_FAST)
#define ARM_TRUSTZONE_ODM_FAST_SMC_ID_UID_INDEX(Rx)         ARM_TRUSTZONE_SUPPORTED_SMC_ID_UID_INDEX(Rx,ODM_FAST)
#define ARM_TRUSTZONE_ODM_FAST_SMC_ID_REVISION_INDEX(Rx)    ARM_TRUSTZONE_SUPPORTED_SMC_ID_REVISION_INDEX(Rx,ODM_FAST)
#define ARM_TRUSTZONE_ODM_FAST_SMC_ID_RPC_INDEX(Rx)         ARM_TRUSTZONE_SUPPORTED_SMC_ID_RPC_INDEX(Rx,ODM_FAST)

#define IS_ARM_TRUSTZONE_OEM_FAST_SMC_ID_PRESENCE(Rx)       IS_ARM_TRUSTZONE_SUPPORTED_SMC_ID_PRESENCE(Rx,OEM_FAST)
#define IS_ARM_TRUSTZONE_OEM_FAST_SMC_ID_UID(Rx)            IS_ARM_TRUSTZONE_SUPPORTED_SMC_ID_UID(Rx,OEM_FAST)
#define IS_ARM_TRUSTZONE_OEM_FAST_SMC_ID_REVISION(Rx)       IS_ARM_TRUSTZONE_SUPPORTED_SMC_ID_REVISION(Rx,OEM_FAST)
#define IS_ARM_TRUSTZONE_OEM_FAST_SMC_ID_RPC(Rx)            IS_ARM_TRUSTZONE_SUPPORTED_SMC_ID_RPC(Rx,OEM_FAST)
#define ARM_TRUSTZONE_OEM_FAST_SMC_ID_UID_INDEX(Rx)         ARM_TRUSTZONE_SUPPORTED_SMC_ID_UID_INDEX(Rx,OEM_FAST)
#define ARM_TRUSTZONE_OEM_FAST_SMC_ID_REVISION_INDEX(Rx)    ARM_TRUSTZONE_SUPPORTED_SMC_ID_REVISION_INDEX(Rx,OEM_FAST)
#define ARM_TRUSTZONE_OEM_FAST_SMC_ID_RPC_INDEX(Rx)         ARM_TRUSTZONE_SUPPORTED_SMC_ID_RPC_INDEX(Rx,OEM_FAST)

#define IS_ARM_TRUSTZONE_SIP_FAST_SMC_ID_PRESENCE(Rx)       IS_ARM_TRUSTZONE_SUPPORTED_SMC_ID_PRESENCE(Rx,SIP_FAST)
#define IS_ARM_TRUSTZONE_SIP_FAST_SMC_ID_UID(Rx)            IS_ARM_TRUSTZONE_SUPPORTED_SMC_ID_UID(Rx,SIP_FAST)
#define IS_ARM_TRUSTZONE_SIP_FAST_SMC_ID_REVISION(Rx)       IS_ARM_TRUSTZONE_SUPPORTED_SMC_ID_REVISION(Rx,SIP_FAST)
#define IS_ARM_TRUSTZONE_SIP_FAST_SMC_ID_RPC(Rx)            IS_ARM_TRUSTZONE_SUPPORTED_SMC_ID_RPC(Rx,SIP_FAST)
#define ARM_TRUSTZONE_SIP_FAST_SMC_ID_UID_INDEX(Rx)         ARM_TRUSTZONE_SUPPORTED_SMC_ID_UID_INDEX(Rx,SIP_FAST)
#define ARM_TRUSTZONE_SIP_FAST_SMC_ID_REVISION_INDEX(Rx)    ARM_TRUSTZONE_SUPPORTED_SMC_ID_REVISION_INDEX(Rx,SIP_FAST)
#define ARM_TRUSTZONE_SIP_FAST_SMC_ID_RPC_INDEX(Rx)         ARM_TRUSTZONE_SUPPORTED_SMC_ID_RPC_INDEX(Rx,SIP_FAST)

#define ARM_TRUSTZONE_TRUSTED_USER_FAST_SMC_ID_RPC_INDEX(Rx)    ARM_TRUSTZONE_SUPPORTED_SMC_ID_RPC_INDEX(Rx,TRUSTED_USER_FAST)

#define IS_ARM_TRUSTZONE_TRUSTED_OS_FAST_SMC_ID_PRESENCE(Rx)    IS_ARM_TRUSTZONE_SUPPORTED_SMC_ID_PRESENCE(Rx,TRUSTED_OS_FAST)
#define IS_ARM_TRUSTZONE_TRUSTED_OS_FAST_SMC_ID_UID(Rx)         IS_ARM_TRUSTZONE_SUPPORTED_SMC_ID_UID(Rx,TRUSTED_OS_FAST)
#define IS_ARM_TRUSTZONE_TRUSTED_OS_FAST_SMC_ID_REVISION(Rx)    IS_ARM_TRUSTZONE_SUPPORTED_SMC_ID_REVISION(Rx,TRUSTED_OS_FAST)
#define IS_ARM_TRUSTZONE_TRUSTED_OS_FAST_SMC_ID_RPC(Rx)         IS_ARM_TRUSTZONE_SUPPORTED_SMC_ID_RPC(Rx,TRUSTED_OS_FAST)
#define ARM_TRUSTZONE_TRUSTED_OS_FAST_SMC_ID_UID_INDEX(Rx)      ARM_TRUSTZONE_SUPPORTED_SMC_ID_UID_INDEX(Rx,TRUSTED_OS_FAST)
#define ARM_TRUSTZONE_TRUSTED_OS_FAST_SMC_ID_REVISION_INDEX(Rx) ARM_TRUSTZONE_SUPPORTED_SMC_ID_REVISION_INDEX(Rx,TRUSTED_OS_FAST)
#define ARM_TRUSTZONE_TRUSTED_OS_FAST_SMC_ID_RPC_INDEX(Rx)      ARM_TRUSTZONE_SUPPORTED_SMC_ID_RPC_INDEX(Rx,TRUSTED_OS_FAST)


#define ARM_TRUSTZONE_DEPRECIATED_SMC_ID_START          0x00000000
#define ARM_TRUSTZONE_DEPRECIATED_SMC_ID_END            0x01FFFFFF


#define ARM_TRUSTZONE_TRUSTED_OS_SMC_ID_START           0x02000000
#define ARM_TRUSTZONE_TRUSTED_OS_SMC_ID_END             0x1FFFFFFF

#define ARM_TRUSTZONE_TRUSTED_OS_SMC_ID_RPC_START       0x02000000
#define ARM_TRUSTZONE_TRUSTED_OS_SMC_ID_RPC_END         0x1FFFFFFF


#define ARM_TRUSTZONE_ARM_FAST_SMC_ID_START             0x80000000
#define ARM_TRUSTZONE_ARM_FAST_SMC_ID_END               0x80FFFFFF

#define ARM_TRUSTZONE_ARM_FAST_SMC_ID_RPC_START         0x80000000
#define ARM_TRUSTZONE_ARM_FAST_SMC_ID_RPC_END           0x80FFFEFF
#define ARM_TRUSTZONE_ARM_FAST_SMC_ID_PRESENCE          0x80FFFF00
#define ARM_TRUSTZONE_ARM_FAST_SMC_ID_UID               0x80FFFF10
#define ARM_TRUSTZONE_ARM_FAST_SMC_ID_REVISION          0x80FFFF20


#define ARM_TRUSTZONE_SIP_FAST_SMC_ID_START             0x81000000
#define ARM_TRUSTZONE_SIP_FAST_SMC_ID_END               0x81FFFFFF

#define ARM_TRUSTZONE_SIP_FAST_SMC_ID_RPC_START         0x81000000
#define ARM_TRUSTZONE_SIP_FAST_SMC_ID_RPC_END           0x81FFFEFF
#define ARM_TRUSTZONE_SIP_FAST_SMC_ID_PRESENCE          0x81FFFF00
#define ARM_TRUSTZONE_SIP_FAST_SMC_ID_UID               0x81FFFF10
#define ARM_TRUSTZONE_SIP_FAST_SMC_ID_REVISION          0x81FFFF20


#define ARM_TRUSTZONE_ODM_FAST_SMC_ID_START             0x82000000
#define ARM_TRUSTZONE_ODM_FAST_SMC_ID_END               0x82FFFFFF

#define ARM_TRUSTZONE_ODM_FAST_SMC_ID_RPC_START         0x82000000
#define ARM_TRUSTZONE_ODM_FAST_SMC_ID_RPC_END           0x82FFFEFF
#define ARM_TRUSTZONE_ODM_FAST_SMC_ID_PRESENCE          0x82FFFF00
#define ARM_TRUSTZONE_ODM_FAST_SMC_ID_UID               0x82FFFF10
#define ARM_TRUSTZONE_ODM_FAST_SMC_ID_REVISION          0x82FFFF20


#define ARM_TRUSTZONE_OEM_FAST_SMC_ID_START             0x83000000
#define ARM_TRUSTZONE_OEM_FAST_SMC_ID_END               0x83FFFFFF

#define ARM_TRUSTZONE_OEM_FAST_SMC_ID_RPC_START         0x83000000
#define ARM_TRUSTZONE_OEM_FAST_SMC_ID_RPC_END           0x83FFFEFF
#define ARM_TRUSTZONE_OEM_FAST_SMC_ID_PRESENCE          0x83FFFF00
#define ARM_TRUSTZONE_OEM_FAST_SMC_ID_UID               0x83FFFF10
#define ARM_TRUSTZONE_OEM_FAST_SMC_ID_REVISION          0x83FFFF20


#define ARM_TRUSTZONE_TRUSTED_USER_FAST_SMC_ID_START    0xF0000000
#define ARM_TRUSTZONE_TRUSTED_USER_FAST_SMC_ID_END      0xF1FFFFFF

#define ARM_TRUSTZONE_TRUSTED_USER_FAST_SMC_ID_RPC_START  0xF0000000
#define ARM_TRUSTZONE_TRUSTED_USER_FAST_SMC_ID_RPC_END    0xF1FFFEFF


#define ARM_TRUSTZONE_TRUSTED_OS_FAST_SMC_ID_START      0xF2000000
#define ARM_TRUSTZONE_TRUSTED_OS_FAST_SMC_ID_END        0xFFFFFFFF

#define ARM_TRUSTZONE_TRUSTED_OS_FAST_SMC_ID_RPC_START  0xF2000000
#define ARM_TRUSTZONE_TRUSTED_OS_FAST_SMC_ID_RPC_END    0xFFFFFEFF
#define ARM_TRUSTZONE_TRUSTED_OS_FAST_SMC_ID_PRESENCE   0xF2FFFF00
#define ARM_TRUSTZONE_TRUSTED_OS_FAST_SMC_ID_UID        0xF2FFFF10
#define ARM_TRUSTZONE_TRUSTED_OS_FAST_SMC_ID_REVISION   0xF2FFFF20

#endif
