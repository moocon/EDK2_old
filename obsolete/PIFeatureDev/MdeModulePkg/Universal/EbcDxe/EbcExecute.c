/** @file
  Contains code that implements the virtual machine.

Copyright (c) 2006, Intel Corporation
All rights reserved. This program and the accompanying materials
are licensed and made available under the terms and conditions of the BSD License
which accompanies this distribution.  The full text of the license may be found at
http://opensource.org/licenses/bsd-license.php

THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

#include "EbcInt.h"
#include "EbcExecute.h"


//
// Define some useful data size constants to allow switch statements based on
// size of operands or data.
//
#define DATA_SIZE_INVALID 0
#define DATA_SIZE_8       1
#define DATA_SIZE_16      2
#define DATA_SIZE_32      4
#define DATA_SIZE_64      8
#define DATA_SIZE_N       48  // 4 or 8
//
// Structure we'll use to dispatch opcodes to execute functions.
//
typedef struct {
  EFI_STATUS (*ExecuteFunction) (IN VM_CONTEXT * VmPtr);
}
VM_TABLE_ENTRY;

typedef
UINT64
(*DATA_MANIP_EXEC_FUNCTION) (
  IN VM_CONTEXT * VmPtr,
  IN UINT64     Op1,
  IN UINT64     Op2
  );

STATIC
INT16
VmReadIndex16 (
  IN VM_CONTEXT *VmPtr,
  IN UINT32     CodeOffset
  );

STATIC
INT32
VmReadIndex32 (
  IN VM_CONTEXT *VmPtr,
  IN UINT32     CodeOffset
  );

STATIC
INT64
VmReadIndex64 (
  IN VM_CONTEXT *VmPtr,
  IN UINT32     CodeOffset
  );

STATIC
UINT8
VmReadMem8 (
  IN VM_CONTEXT *VmPtr,
  IN UINTN      Addr
  );

STATIC
UINT16
VmReadMem16 (
  IN VM_CONTEXT *VmPtr,
  IN UINTN      Addr
  );

STATIC
UINT32
VmReadMem32 (
  IN VM_CONTEXT *VmPtr,
  IN UINTN      Addr
  );

STATIC
UINT64
VmReadMem64 (
  IN VM_CONTEXT *VmPtr,
  IN UINTN      Addr
  );

STATIC
UINTN
VmReadMemN (
  IN VM_CONTEXT *VmPtr,
  IN UINTN      Addr
  );

STATIC
EFI_STATUS
VmWriteMem8 (
  IN VM_CONTEXT *VmPtr,
  UINTN         Addr,
  IN UINT8      Data
  );

STATIC
EFI_STATUS
VmWriteMem16 (
  IN VM_CONTEXT *VmPtr,
  UINTN         Addr,
  IN UINT16     Data
  );

STATIC
EFI_STATUS
VmWriteMem32 (
  IN VM_CONTEXT *VmPtr,
  UINTN         Addr,
  IN UINT32     Data
  );

STATIC
UINT16
VmReadCode16 (
  IN VM_CONTEXT *VmPtr,
  IN UINT32     Offset
  );

STATIC
UINT32
VmReadCode32 (
  IN VM_CONTEXT *VmPtr,
  IN UINT32     Offset
  );

STATIC
UINT64
VmReadCode64 (
  IN VM_CONTEXT *VmPtr,
  IN UINT32     Offset
  );

STATIC
INT8
VmReadImmed8 (
  IN VM_CONTEXT *VmPtr,
  IN UINT32     Offset
  );

STATIC
INT16
VmReadImmed16 (
  IN VM_CONTEXT *VmPtr,
  IN UINT32     Offset
  );

STATIC
INT32
VmReadImmed32 (
  IN VM_CONTEXT *VmPtr,
  IN UINT32     Offset
  );

STATIC
INT64
VmReadImmed64 (
  IN VM_CONTEXT *VmPtr,
  IN UINT32     Offset
  );

STATIC
UINTN
ConvertStackAddr (
  IN VM_CONTEXT   *VmPtr,
  IN UINTN        Addr
  );

STATIC
EFI_STATUS
ExecuteDataManip (
  IN VM_CONTEXT   *VmPtr,
  IN BOOLEAN      IsSignedOperation
  );

//
// Functions that execute VM opcodes
//
STATIC
EFI_STATUS
ExecuteBREAK (
  IN VM_CONTEXT *VmPtr
  );

STATIC
EFI_STATUS
ExecuteJMP (
  IN VM_CONTEXT *VmPtr
  );

STATIC
EFI_STATUS
ExecuteJMP8 (
  IN VM_CONTEXT *VmPtr
  );

STATIC
EFI_STATUS
ExecuteCALL (
  IN VM_CONTEXT *VmPtr
  );

STATIC
EFI_STATUS
ExecuteRET (
  IN VM_CONTEXT *VmPtr
  );

STATIC
EFI_STATUS
ExecuteCMP (
  IN VM_CONTEXT *VmPtr
  );

STATIC
EFI_STATUS
ExecuteCMPI (
  IN VM_CONTEXT *VmPtr
  );

STATIC
EFI_STATUS
ExecuteMOVxx (
  IN VM_CONTEXT *VmPtr
  );

STATIC
EFI_STATUS
ExecuteMOVI (
  IN VM_CONTEXT *VmPtr
  );

STATIC
EFI_STATUS
ExecuteMOVIn (
  IN VM_CONTEXT *VmPtr
  );

STATIC
EFI_STATUS
ExecuteMOVREL (
  IN VM_CONTEXT *VmPtr
  );

STATIC
EFI_STATUS
ExecutePUSHn (
  IN VM_CONTEXT *VmPtr
  );

STATIC
EFI_STATUS
ExecutePUSH (
  IN VM_CONTEXT *VmPtr
  );

STATIC
EFI_STATUS
ExecutePOPn (
  IN VM_CONTEXT *VmPtr
  );

STATIC
EFI_STATUS
ExecutePOP (
  IN VM_CONTEXT *VmPtr
  );

STATIC
EFI_STATUS
ExecuteSignedDataManip (
  IN VM_CONTEXT *VmPtr
  );

STATIC
EFI_STATUS
ExecuteUnsignedDataManip (
  IN VM_CONTEXT *VmPtr
  );

STATIC
EFI_STATUS
ExecuteLOADSP (
  IN VM_CONTEXT *VmPtr
  );

STATIC
EFI_STATUS
ExecuteSTORESP (
  IN VM_CONTEXT *VmPtr
  );

STATIC
EFI_STATUS
ExecuteMOVsnd (
  IN VM_CONTEXT *VmPtr
  );

STATIC
EFI_STATUS
ExecuteMOVsnw (
  IN VM_CONTEXT *VmPtr
  );

//
// Data manipulation subfunctions
//
STATIC
UINT64
ExecuteNOT (
  IN VM_CONTEXT *VmPtr,
  IN UINT64     Op1,
  IN UINT64     Op2
  );

STATIC
UINT64
ExecuteNEG (
  IN VM_CONTEXT *VmPtr,
  IN UINT64     Op1,
  IN UINT64     Op2
  );

STATIC
UINT64
ExecuteADD (
  IN VM_CONTEXT *VmPtr,
  IN UINT64     Op1,
  IN UINT64     Op2
  );

STATIC
UINT64
ExecuteSUB (
  IN VM_CONTEXT *VmPtr,
  IN UINT64     Op1,
  IN UINT64     Op2
  );

STATIC
UINT64
ExecuteMUL (
  IN VM_CONTEXT *VmPtr,
  IN UINT64     Op1,
  IN UINT64     Op2
  );

STATIC
UINT64
ExecuteMULU (
  IN VM_CONTEXT *VmPtr,
  IN UINT64     Op1,
  IN UINT64     Op2
  );

STATIC
UINT64
ExecuteDIV (
  IN VM_CONTEXT *VmPtr,
  IN UINT64     Op1,
  IN UINT64     Op2
  );

STATIC
UINT64
ExecuteDIVU (
  IN VM_CONTEXT *VmPtr,
  IN UINT64     Op1,
  IN UINT64     Op2
  );

STATIC
UINT64
ExecuteMOD (
  IN VM_CONTEXT *VmPtr,
  IN UINT64     Op1,
  IN UINT64     Op2
  );

STATIC
UINT64
ExecuteMODU (
  IN VM_CONTEXT *VmPtr,
  IN UINT64     Op1,
  IN UINT64     Op2
  );

STATIC
UINT64
ExecuteAND (
  IN VM_CONTEXT *VmPtr,
  IN UINT64     Op1,
  IN UINT64     Op2
  );

STATIC
UINT64
ExecuteOR (
  IN VM_CONTEXT *VmPtr,
  IN UINT64     Op1,
  IN UINT64     Op2
  );

STATIC
UINT64
ExecuteXOR (
  IN VM_CONTEXT *VmPtr,
  IN UINT64     Op1,
  IN UINT64     Op2
  );

STATIC
UINT64
ExecuteSHL (
  IN VM_CONTEXT *VmPtr,
  IN UINT64     Op1,
  IN UINT64     Op2
  );

STATIC
UINT64
ExecuteSHR (
  IN VM_CONTEXT *VmPtr,
  IN UINT64     Op1,
  IN UINT64     Op2
  );

STATIC
UINT64
ExecuteASHR (
  IN VM_CONTEXT *VmPtr,
  IN UINT64     Op1,
  IN UINT64     Op2
  );

STATIC
UINT64
ExecuteEXTNDB (
  IN VM_CONTEXT *VmPtr,
  IN UINT64     Op1,
  IN UINT64     Op2
  );

STATIC
UINT64
ExecuteEXTNDW (
  IN VM_CONTEXT *VmPtr,
  IN UINT64     Op1,
  IN UINT64     Op2
  );

STATIC
UINT64
ExecuteEXTNDD (
  IN VM_CONTEXT *VmPtr,
  IN UINT64     Op1,
  IN UINT64     Op2
  );

//
// Once we retrieve the operands for the data manipulation instructions,
// call these functions to perform the operation.
//
static CONST DATA_MANIP_EXEC_FUNCTION mDataManipDispatchTable[] = {
  ExecuteNOT,
  ExecuteNEG,
  ExecuteADD,
  ExecuteSUB,
  ExecuteMUL,
  ExecuteMULU,
  ExecuteDIV,
  ExecuteDIVU,
  ExecuteMOD,
  ExecuteMODU,
  ExecuteAND,
  ExecuteOR,
  ExecuteXOR,
  ExecuteSHL,
  ExecuteSHR,
  ExecuteASHR,
  ExecuteEXTNDB,
  ExecuteEXTNDW,
  ExecuteEXTNDD,
};

static CONST VM_TABLE_ENTRY           mVmOpcodeTable[] = {
  { ExecuteBREAK },             // opcode 0x00
  { ExecuteJMP },               // opcode 0x01
  { ExecuteJMP8 },              // opcode 0x02
  { ExecuteCALL },              // opcode 0x03
  { ExecuteRET },               // opcode 0x04
  { ExecuteCMP },               // opcode 0x05 CMPeq
  { ExecuteCMP },               // opcode 0x06 CMPlte
  { ExecuteCMP },               // opcode 0x07 CMPgte
  { ExecuteCMP },               // opcode 0x08 CMPulte
  { ExecuteCMP },               // opcode 0x09 CMPugte
  { ExecuteUnsignedDataManip }, // opcode 0x0A NOT
  { ExecuteSignedDataManip },   // opcode 0x0B NEG
  { ExecuteSignedDataManip },   // opcode 0x0C ADD
  { ExecuteSignedDataManip },   // opcode 0x0D SUB
  { ExecuteSignedDataManip },   // opcode 0x0E MUL
  { ExecuteUnsignedDataManip }, // opcode 0x0F MULU
  { ExecuteSignedDataManip },   // opcode 0x10 DIV
  { ExecuteUnsignedDataManip }, // opcode 0x11 DIVU
  { ExecuteSignedDataManip },   // opcode 0x12 MOD
  { ExecuteUnsignedDataManip }, // opcode 0x13 MODU
  { ExecuteUnsignedDataManip }, // opcode 0x14 AND
  { ExecuteUnsignedDataManip }, // opcode 0x15 OR
  { ExecuteUnsignedDataManip }, // opcode 0x16 XOR
  { ExecuteUnsignedDataManip }, // opcode 0x17 SHL
  { ExecuteUnsignedDataManip }, // opcode 0x18 SHR
  { ExecuteSignedDataManip },   // opcode 0x19 ASHR
  { ExecuteUnsignedDataManip }, // opcode 0x1A EXTNDB
  { ExecuteUnsignedDataManip }, // opcode 0x1B EXTNDW
  { ExecuteUnsignedDataManip }, // opcode 0x1C EXTNDD
  { ExecuteMOVxx },             // opcode 0x1D MOVBW
  { ExecuteMOVxx },             // opcode 0x1E MOVWW
  { ExecuteMOVxx },             // opcode 0x1F MOVDW
  { ExecuteMOVxx },             // opcode 0x20 MOVQW
  { ExecuteMOVxx },             // opcode 0x21 MOVBD
  { ExecuteMOVxx },             // opcode 0x22 MOVWD
  { ExecuteMOVxx },             // opcode 0x23 MOVDD
  { ExecuteMOVxx },             // opcode 0x24 MOVQD
  { ExecuteMOVsnw },            // opcode 0x25 MOVsnw
  { ExecuteMOVsnd },            // opcode 0x26 MOVsnd
  { NULL },                     // opcode 0x27
  { ExecuteMOVxx },             // opcode 0x28 MOVqq
  { ExecuteLOADSP },            // opcode 0x29 LOADSP SP1, R2
  { ExecuteSTORESP },           // opcode 0x2A STORESP R1, SP2
  { ExecutePUSH },              // opcode 0x2B PUSH {@}R1 [imm16]
  { ExecutePOP },               // opcode 0x2C POP {@}R1 [imm16]
  { ExecuteCMPI },              // opcode 0x2D CMPIEQ
  { ExecuteCMPI },              // opcode 0x2E CMPILTE
  { ExecuteCMPI },              // opcode 0x2F CMPIGTE
  { ExecuteCMPI },              // opcode 0x30 CMPIULTE
  { ExecuteCMPI },              // opcode 0x31 CMPIUGTE
  { ExecuteMOVxx },             // opcode 0x32 MOVN
  { ExecuteMOVxx },             // opcode 0x33 MOVND
  { NULL },                     // opcode 0x34
  { ExecutePUSHn },             // opcode 0x35
  { ExecutePOPn },              // opcode 0x36
  { ExecuteMOVI },              // opcode 0x37 - mov immediate data
  { ExecuteMOVIn },             // opcode 0x38 - mov immediate natural
  { ExecuteMOVREL }             // opcode 0x39 - move data relative to PC
};

//
// Length of JMP instructions, depending on upper two bits of opcode.
//
static CONST UINT8                    mJMPLen[] = { 2, 2, 6, 10 };

//
// Simple Debugger Protocol GUID
//
EFI_GUID mEbcSimpleDebuggerProtocolGuid = EFI_EBC_SIMPLE_DEBUGGER_PROTOCOL_GUID;


/**
  Given a pointer to a new VM context, execute one or more instructions. This
  function is only used for test purposes via the EBC VM test protocol.

  @param  This              pointer to protocol interface
  @param  VmPtr             pointer to a VM context
  @param  InstructionCount  how many instructions to execute. 0 if don't count.

  @return EFI_UNSUPPORTED
  @return EFI_SUCCESS

**/
EFI_STATUS
EbcExecuteInstructions (
  IN EFI_EBC_VM_TEST_PROTOCOL *This,
  IN VM_CONTEXT               *VmPtr,
  IN OUT UINTN                *InstructionCount
  )
{
  UINTN       ExecFunc;
  EFI_STATUS  Status;
  UINTN       InstructionsLeft;
  UINTN       SavedInstructionCount;

  Status = EFI_SUCCESS;

  if (*InstructionCount == 0) {
    InstructionsLeft = 1;
  } else {
    InstructionsLeft = *InstructionCount;
  }

  SavedInstructionCount = *InstructionCount;
  *InstructionCount     = 0;

  //
  // Index into the opcode table using the opcode byte for this instruction.
  // This gives you the execute function, which we first test for null, then
  // call it if it's not null.
  //
  while (InstructionsLeft != 0) {
    ExecFunc = (UINTN) mVmOpcodeTable[(*VmPtr->Ip & 0x3F)].ExecuteFunction;
    if (ExecFunc == (UINTN) NULL) {
      EbcDebugSignalException (EXCEPT_EBC_INVALID_OPCODE, EXCEPTION_FLAG_FATAL, VmPtr);
      return EFI_UNSUPPORTED;
    } else {
      mVmOpcodeTable[(*VmPtr->Ip & 0x3F)].ExecuteFunction (VmPtr);
      *InstructionCount = *InstructionCount + 1;
    }

    //
    // Decrement counter if applicable
    //
    if (SavedInstructionCount != 0) {
      InstructionsLeft--;
    }
  }

  return Status;
}


/**
  Execute an EBC image from an entry point or from a published protocol.

  @param  VmPtr             pointer to prepared VM context.

  @return Standard EBC status.

**/
EFI_STATUS
EbcExecute (
  IN VM_CONTEXT *VmPtr
  )
{
  UINTN                             ExecFunc;
  UINT8                             StackCorrupted;
  EFI_STATUS                        Status;
  EFI_EBC_SIMPLE_DEBUGGER_PROTOCOL  *EbcSimpleDebugger;

  mVmPtr            = VmPtr;
  EbcSimpleDebugger = NULL;
  Status            = EFI_SUCCESS;
  StackCorrupted    = 0;

  //
  // Make sure the magic value has been put on the stack before we got here.
  //
  if (*VmPtr->StackMagicPtr != (UINTN) VM_STACK_KEY_VALUE) {
    StackCorrupted = 1;
  }

  VmPtr->FramePtr = (VOID *) ((UINT8 *) (UINTN) VmPtr->R[0] + 8);

  //
  // Try to get the debug support for EBC
  //
  DEBUG_CODE_BEGIN ();
    Status = gBS->LocateProtocol (
                    &mEbcSimpleDebuggerProtocolGuid,
                    NULL,
                    (VOID **) &EbcSimpleDebugger
                    );
    if (EFI_ERROR (Status)) {
      EbcSimpleDebugger = NULL;
    }
  DEBUG_CODE_END ();

  //
  // Save the start IP for debug. For example, if we take an exception we
  // can print out the location of the exception relative to the entry point,
  // which could then be used in a disassembly listing to find the problem.
  //
  VmPtr->EntryPoint = (VOID *) VmPtr->Ip;

  //
  // We'll wait for this flag to know when we're done. The RET
  // instruction sets it if it runs out of stack.
  //
  VmPtr->StopFlags = 0;
  while (!(VmPtr->StopFlags & STOPFLAG_APP_DONE)) {
    //
    // If we've found a simple debugger protocol, call it
    //
    DEBUG_CODE_BEGIN ();
      if (EbcSimpleDebugger != NULL) {
        EbcSimpleDebugger->Debugger (EbcSimpleDebugger, VmPtr);
      }
    DEBUG_CODE_END ();

    //
    // Verify the opcode is in range. Otherwise generate an exception.
    //
    if ((*VmPtr->Ip & OPCODE_M_OPCODE) >= (sizeof (mVmOpcodeTable) / sizeof (mVmOpcodeTable[0]))) {
      EbcDebugSignalException (EXCEPT_EBC_INVALID_OPCODE, EXCEPTION_FLAG_FATAL, VmPtr);
      Status = EFI_UNSUPPORTED;
      goto Done;
    }
    //
    // Use the opcode bits to index into the opcode dispatch table. If the
    // function pointer is null then generate an exception.
    //
    ExecFunc = (UINTN) mVmOpcodeTable[(*VmPtr->Ip & OPCODE_M_OPCODE)].ExecuteFunction;
    if (ExecFunc == (UINTN) NULL) {
      EbcDebugSignalException (EXCEPT_EBC_INVALID_OPCODE, EXCEPTION_FLAG_FATAL, VmPtr);
      Status = EFI_UNSUPPORTED;
      goto Done;
    }
    //
    // The EBC VM is a strongly ordered processor, so perform a fence operation before
    // and after each instruction is executed.
    //
    MemoryFence ();

    mVmOpcodeTable[(*VmPtr->Ip & OPCODE_M_OPCODE)].ExecuteFunction (VmPtr);

    MemoryFence ();

    //
    // If the step flag is set, signal an exception and continue. We don't
    // clear it here. Assuming the debugger is responsible for clearing it.
    //
    if (VMFLAG_ISSET (VmPtr, VMFLAGS_STEP)) {
      EbcDebugSignalException (EXCEPT_EBC_STEP, EXCEPTION_FLAG_NONE, VmPtr);
    }
    //
    // Make sure stack has not been corrupted. Only report it once though.
    //
    if (!StackCorrupted && (*VmPtr->StackMagicPtr != (UINTN) VM_STACK_KEY_VALUE)) {
      EbcDebugSignalException (EXCEPT_EBC_STACK_FAULT, EXCEPTION_FLAG_FATAL, VmPtr);
      StackCorrupted = 1;
    }
    if (!StackCorrupted && ((UINT64)VmPtr->R[0] <= (UINT64)(UINTN) VmPtr->StackTop)) {
      EbcDebugSignalException (EXCEPT_EBC_STACK_FAULT, EXCEPTION_FLAG_FATAL, VmPtr);
      StackCorrupted = 1;
    }
  }

Done:
  mVmPtr          = NULL;

  return Status;
}


/**
  Execute the MOVxx instructions.

  @param  VmPtr             pointer to a VM context.

  @return EFI_UNSUPPORTED
  @return EFI_SUCCESS
  @return Instruction format:
  @return MOV[b|w|d|q|n]{w|d} {@}R1 {Index16|32}, {@}R2 {Index16|32}
  @return MOVqq {@}R1 {Index64}, {@}R2 {Index64}
  @return Copies contents of [R2] -> [R1], zero extending where required.
  @return First character indicates the size of the move.
  @return Second character indicates the size of the index(s).
  @return Invalid to have R1 direct with index.

**/
STATIC
EFI_STATUS
ExecuteMOVxx (
  IN VM_CONTEXT *VmPtr
  )
{
  UINT8   Opcode;
  UINT8   OpcMasked;
  UINT8   Operands;
  UINT8   Size;
  UINT8   MoveSize;
  INT16   Index16;
  INT32   Index32;
  INT64   Index64Op1;
  INT64   Index64Op2;
  UINT64  Data64;
  UINT64  DataMask;
  UINTN   Source;

  Opcode    = GETOPCODE (VmPtr);
  OpcMasked = (UINT8) (Opcode & OPCODE_M_OPCODE);

  //
  // Get the operands byte so we can get R1 and R2
  //
  Operands = GETOPERANDS (VmPtr);

  //
  // Assume no indexes
  //
  Index64Op1  = 0;
  Index64Op2  = 0;
  Data64      = 0;

  //
  // Determine if we have an index/immediate data. Base instruction size
  // is 2 (opcode + operands). Add to this size each index specified.
  //
  Size = 2;
  if (Opcode & (OPCODE_M_IMMED_OP1 | OPCODE_M_IMMED_OP2)) {
    //
    // Determine size of the index from the opcode. Then get it.
    //
    if ((OpcMasked <= OPCODE_MOVQW) || (OpcMasked == OPCODE_MOVNW)) {
      //
      // MOVBW, MOVWW, MOVDW, MOVQW, and MOVNW have 16-bit immediate index.
      // Get one or both index values.
      //
      if (Opcode & OPCODE_M_IMMED_OP1) {
        Index16     = VmReadIndex16 (VmPtr, 2);
        Index64Op1  = (INT64) Index16;
        Size += sizeof (UINT16);
      }

      if (Opcode & OPCODE_M_IMMED_OP2) {
        Index16     = VmReadIndex16 (VmPtr, Size);
        Index64Op2  = (INT64) Index16;
        Size += sizeof (UINT16);
      }
    } else if ((OpcMasked <= OPCODE_MOVQD) || (OpcMasked == OPCODE_MOVND)) {
      //
      // MOVBD, MOVWD, MOVDD, MOVQD, and MOVND have 32-bit immediate index
      //
      if (Opcode & OPCODE_M_IMMED_OP1) {
        Index32     = VmReadIndex32 (VmPtr, 2);
        Index64Op1  = (INT64) Index32;
        Size += sizeof (UINT32);
      }

      if (Opcode & OPCODE_M_IMMED_OP2) {
        Index32     = VmReadIndex32 (VmPtr, Size);
        Index64Op2  = (INT64) Index32;
        Size += sizeof (UINT32);
      }
    } else if (OpcMasked == OPCODE_MOVQQ) {
      //
      // MOVqq -- only form with a 64-bit index
      //
      if (Opcode & OPCODE_M_IMMED_OP1) {
        Index64Op1 = VmReadIndex64 (VmPtr, 2);
        Size += sizeof (UINT64);
      }

      if (Opcode & OPCODE_M_IMMED_OP2) {
        Index64Op2 = VmReadIndex64 (VmPtr, Size);
        Size += sizeof (UINT64);
      }
    } else {
      //
      // Obsolete MOVBQ, MOVWQ, MOVDQ, and MOVNQ have 64-bit immediate index
      //
      EbcDebugSignalException (
        EXCEPT_EBC_INSTRUCTION_ENCODING,
        EXCEPTION_FLAG_FATAL,
        VmPtr
        );
      return EFI_UNSUPPORTED;
    }
  }
  //
  // Determine the size of the move, and create a mask for it so we can
  // clear unused bits.
  //
  if ((OpcMasked == OPCODE_MOVBW) || (OpcMasked == OPCODE_MOVBD)) {
    MoveSize  = DATA_SIZE_8;
    DataMask  = 0xFF;
  } else if ((OpcMasked == OPCODE_MOVWW) || (OpcMasked == OPCODE_MOVWD)) {
    MoveSize  = DATA_SIZE_16;
    DataMask  = 0xFFFF;
  } else if ((OpcMasked == OPCODE_MOVDW) || (OpcMasked == OPCODE_MOVDD)) {
    MoveSize  = DATA_SIZE_32;
    DataMask  = 0xFFFFFFFF;
  } else if ((OpcMasked == OPCODE_MOVQW) || (OpcMasked == OPCODE_MOVQD) || (OpcMasked == OPCODE_MOVQQ)) {
    MoveSize  = DATA_SIZE_64;
    DataMask  = (UINT64)~0;
  } else if ((OpcMasked == OPCODE_MOVNW) || (OpcMasked == OPCODE_MOVND)) {
    MoveSize  = DATA_SIZE_N;
    DataMask  = (UINT64)~0 >> (64 - 8 * sizeof (UINTN));
  } else {
    //
    // We were dispatched to this function and we don't recognize the opcode
    //
    EbcDebugSignalException (EXCEPT_EBC_UNDEFINED, EXCEPTION_FLAG_FATAL, VmPtr);
    return EFI_UNSUPPORTED;
  }
  //
  // Now get the source address
  //
  if (OPERAND2_INDIRECT (Operands)) {
    //
    // Indirect form @R2. Compute address of operand2
    //
    Source = (UINTN) (VmPtr->R[OPERAND2_REGNUM (Operands)] + Index64Op2);
    //
    // Now get the data from the source. Always 0-extend and let the compiler
    // sign-extend where required.
    //
    switch (MoveSize) {
    case DATA_SIZE_8:
      Data64 = (UINT64) (UINT8) VmReadMem8 (VmPtr, Source);
      break;

    case DATA_SIZE_16:
      Data64 = (UINT64) (UINT16) VmReadMem16 (VmPtr, Source);
      break;

    case DATA_SIZE_32:
      Data64 = (UINT64) (UINT32) VmReadMem32 (VmPtr, Source);
      break;

    case DATA_SIZE_64:
      Data64 = (UINT64) VmReadMem64 (VmPtr, Source);
      break;

    case DATA_SIZE_N:
      Data64 = (UINT64) (UINTN) VmReadMemN (VmPtr, Source);
      break;

    default:
      //
      // not reached
      //
      break;
    }
  } else {
    //
    // Not indirect source: MOVxx {@}Rx, Ry [Index]
    //
    Data64 = VmPtr->R[OPERAND2_REGNUM (Operands)] + Index64Op2;
    //
    // Did Operand2 have an index? If so, treat as two signed values since
    // indexes are signed values.
    //
    if (Opcode & OPCODE_M_IMMED_OP2) {
      //
      // NOTE: need to find a way to fix this, most likely by changing the VM
      // implementation to remove the stack gap. To do that, we'd need to
      // allocate stack space for the VM and actually set the system
      // stack pointer to the allocated buffer when the VM starts.
      //
      // Special case -- if someone took the address of a function parameter
      // then we need to make sure it's not in the stack gap. We can identify
      // this situation if (Operand2 register == 0) && (Operand2 is direct)
      // && (Index applies to Operand2) && (Index > 0) && (Operand1 register != 0)
      // Situations that to be aware of:
      //   * stack adjustments at beginning and end of functions R0 = R0 += stacksize
      //
      if ((OPERAND2_REGNUM (Operands) == 0) &&
          (!OPERAND2_INDIRECT (Operands)) &&
          (Index64Op2 > 0) &&
          (OPERAND1_REGNUM (Operands) == 0) &&
          (OPERAND1_INDIRECT (Operands))
          ) {
        Data64 = (UINT64) ConvertStackAddr (VmPtr, (UINTN) (INT64) Data64);
      }
    }
  }
  //
  // Now write it back
  //
  if (OPERAND1_INDIRECT (Operands)) {
    //
    // Reuse the Source variable to now be dest.
    //
    Source = (UINTN) (VmPtr->R[OPERAND1_REGNUM (Operands)] + Index64Op1);
    //
    // Do the write based on the size
    //
    switch (MoveSize) {
    case DATA_SIZE_8:
      VmWriteMem8 (VmPtr, Source, (UINT8) Data64);
      break;

    case DATA_SIZE_16:
      VmWriteMem16 (VmPtr, Source, (UINT16) Data64);
      break;

    case DATA_SIZE_32:
      VmWriteMem32 (VmPtr, Source, (UINT32) Data64);
      break;

    case DATA_SIZE_64:
      VmWriteMem64 (VmPtr, Source, Data64);
      break;

    case DATA_SIZE_N:
      VmWriteMemN (VmPtr, Source, (UINTN) Data64);
      break;

    default:
      //
      // not reached
      //
      break;
    }
  } else {
    //
    // Operand1 direct.
    // Make sure we didn't have an index on operand1.
    //
    if (Opcode & OPCODE_M_IMMED_OP1) {
      EbcDebugSignalException (
        EXCEPT_EBC_INSTRUCTION_ENCODING,
        EXCEPTION_FLAG_FATAL,
        VmPtr
        );
      return EFI_UNSUPPORTED;
    }
    //
    // Direct storage in register. Clear unused bits and store back to
    // register.
    //
    VmPtr->R[OPERAND1_REGNUM (Operands)] = Data64 & DataMask;
  }
  //
  // Advance the instruction pointer
  //
  VmPtr->Ip += Size;
  return EFI_SUCCESS;
}


/**
  Execute the EBC BREAK instruction

  @param  VmPtr             pointer to current VM context

  @return EFI_UNSUPPORTED
  @return EFI_SUCCESS

**/
STATIC
EFI_STATUS
ExecuteBREAK (
  IN VM_CONTEXT *VmPtr
  )
{
  UINT8       Operands;
  VOID        *EbcEntryPoint;
  VOID        *Thunk;
  UINT64      U64EbcEntryPoint;
  INT32       Offset;

  Operands = GETOPERANDS (VmPtr);
  switch (Operands) {
  //
  // Runaway program break. Generate an exception and terminate
  //
  case 0:
    EbcDebugSignalException (EXCEPT_EBC_BAD_BREAK, EXCEPTION_FLAG_FATAL, VmPtr);
    break;

  //
  // Get VM version -- return VM revision number in R7
  //
  case 1:
    //
    // Bits:
    //  63-17 = 0
    //  16-8  = Major version
    //  7-0   = Minor version
    //
    VmPtr->R[7] = GetVmVersion ();
    break;

  //
  // Debugger breakpoint
  //
  case 3:
    VmPtr->StopFlags |= STOPFLAG_BREAKPOINT;
    //
    // See if someone has registered a handler
    //
    EbcDebugSignalException (
      EXCEPT_EBC_BREAKPOINT,
      EXCEPTION_FLAG_NONE,
      VmPtr
      );
    break;

  //
  // System call, which there are none, so NOP it.
  //
  case 4:
    break;

  //
  // Create a thunk for EBC code. R7 points to a 32-bit (in a 64-bit slot)
  // "offset from self" pointer to the EBC entry point.
  // After we're done, *(UINT64 *)R7 will be the address of the new thunk.
  //
  case 5:
    Offset            = (INT32) VmReadMem32 (VmPtr, (UINTN) VmPtr->R[7]);
    U64EbcEntryPoint  = (UINT64) (VmPtr->R[7] + Offset + 4);
    EbcEntryPoint     = (VOID *) (UINTN) U64EbcEntryPoint;

    //
    // Now create a new thunk
    //
    EbcCreateThunks (VmPtr->ImageHandle, EbcEntryPoint, &Thunk, 0);

    //
    // Finally replace the EBC entry point memory with the thunk address
    //
    VmWriteMem64 (VmPtr, (UINTN) VmPtr->R[7], (UINT64) (UINTN) Thunk);
    break;

  //
  // Compiler setting version per value in R7
  //
  case 6:
    VmPtr->CompilerVersion = (UINT32) VmPtr->R[7];
    //
    // Check compiler version against VM version?
    //
    break;

  //
  // Unhandled break code. Signal exception.
  //
  default:
    EbcDebugSignalException (EXCEPT_EBC_BAD_BREAK, EXCEPTION_FLAG_FATAL, VmPtr);
    break;
  }
  //
  // Advance IP
  //
  VmPtr->Ip += 2;
  return EFI_SUCCESS;
}


/**
  Execute the JMP instruction

  @param  VmPtr             pointer to VM context

  @return Standard EFI_STATUS
  @return Instruction syntax:
  @return JMP64{cs|cc} Immed64
  @return JMP32{cs|cc} {@}R1 {Immed32|Index32}
  @return Encoding:
  @retval b0.7              immediate data present
  @retval b0.6              1 = 64 bit immediate data 0 = 32 bit immediate data
  @retval b1.7              1 = conditional b1.6    1 = CS (condition set) 0 = CC
                            (condition clear) b1.4    1 = relative address 0 =
                            absolute address b1.3    1 = operand1 indirect b1.2-0
                            operand 1

**/
STATIC
EFI_STATUS
ExecuteJMP (
  IN VM_CONTEXT *VmPtr
  )
{
  UINT8   Opcode;
  UINT8   CompareSet;
  UINT8   ConditionFlag;
  UINT8   Size;
  UINT8   Operand;
  UINT64  Data64;
  INT32   Index32;
  UINTN   Addr;

  Operand = GETOPERANDS (VmPtr);
  Opcode  = GETOPCODE (VmPtr);

  //
  // Get instruction length from the opcode. The upper two bits are used here
  // to index into the length array.
  //
  Size = mJMPLen[(Opcode >> 6) & 0x03];

  //
  // Decode instruction conditions
  // If we haven't met the condition, then simply advance the IP and return.
  //
  CompareSet    = (UINT8) ((Operand & JMP_M_CS) ? 1 : 0);
  ConditionFlag = (UINT8) VMFLAG_ISSET (VmPtr, VMFLAGS_CC);
  if (Operand & CONDITION_M_CONDITIONAL) {
    if (CompareSet != ConditionFlag) {
      VmPtr->Ip += Size;
      return EFI_SUCCESS;
    }
  }
  //
  // Check for 64-bit form and do it right away since it's the most
  // straight-forward form.
  //
  if (Opcode & OPCODE_M_IMMDATA64) {
    //
    // Double check for immediate-data, which is required. If not there,
    // then signal an exception
    //
    if (!(Opcode & OPCODE_M_IMMDATA)) {
      EbcDebugSignalException (
        EXCEPT_EBC_INSTRUCTION_ENCODING,
        EXCEPTION_FLAG_ERROR,
        VmPtr
        );
      return EFI_UNSUPPORTED;
    }
    //
    // 64-bit immediate data is full address. Read the immediate data,
    // check for alignment, and jump absolute.
    //
    Data64 = VmReadImmed64 (VmPtr, 2);
    if (!IS_ALIGNED ((UINTN) Data64, sizeof (UINT16))) {
      EbcDebugSignalException (
        EXCEPT_EBC_ALIGNMENT_CHECK,
        EXCEPTION_FLAG_FATAL,
        VmPtr
        );

      return EFI_UNSUPPORTED;
    }

    //
    // Take jump -- relative or absolute
    //
    if (Operand & JMP_M_RELATIVE) {
      VmPtr->Ip += (UINTN) Data64 + Size;
    } else {
      VmPtr->Ip = (VMIP) (UINTN) Data64;
    }

    return EFI_SUCCESS;
  }
  //
  // 32-bit forms:
  // Get the index if there is one. May be either an index, or an immediate
  // offset depending on indirect operand.
  //   JMP32 @R1 Index32 -- immediate data is an index
  //   JMP32 R1 Immed32  -- immedate data is an offset
  //
  if (Opcode & OPCODE_M_IMMDATA) {
    if (OPERAND1_INDIRECT (Operand)) {
      Index32 = VmReadIndex32 (VmPtr, 2);
    } else {
      Index32 = VmReadImmed32 (VmPtr, 2);
    }
  } else {
    Index32 = 0;
  }
  //
  // Get the register data. If R == 0, then special case where it's ignored.
  //
  if (OPERAND1_REGNUM (Operand) == 0) {
    Data64 = 0;
  } else {
    Data64 = OPERAND1_REGDATA (VmPtr, Operand);
  }
  //
  // Decode the forms
  //
  if (OPERAND1_INDIRECT (Operand)) {
    //
    // Form: JMP32 @Rx {Index32}
    //
    Addr = VmReadMemN (VmPtr, (UINTN) Data64 + Index32);
    if (!IS_ALIGNED ((UINTN) Addr, sizeof (UINT16))) {
      EbcDebugSignalException (
        EXCEPT_EBC_ALIGNMENT_CHECK,
        EXCEPTION_FLAG_FATAL,
        VmPtr
        );

      return EFI_UNSUPPORTED;
    }

    if (Operand & JMP_M_RELATIVE) {
      VmPtr->Ip += (UINTN) Addr + Size;
    } else {
      VmPtr->Ip = (VMIP) Addr;
    }
  } else {
    //
    // Form: JMP32 Rx {Immed32}
    //
    Addr = (UINTN) (Data64 + Index32);
    if (!IS_ALIGNED ((UINTN) Addr, sizeof (UINT16))) {
      EbcDebugSignalException (
        EXCEPT_EBC_ALIGNMENT_CHECK,
        EXCEPTION_FLAG_FATAL,
        VmPtr
        );

      return EFI_UNSUPPORTED;
    }

    if (Operand & JMP_M_RELATIVE) {
      VmPtr->Ip += (UINTN) Addr + Size;
    } else {
      VmPtr->Ip = (VMIP) Addr;
    }
  }

  return EFI_SUCCESS;
}


/**
  Execute the EBC JMP8 instruction

  @param  VmPtr             pointer to a VM context

  @return Standard EFI_STATUS
  @return Instruction syntax:
  @return JMP8{cs|cc}  Offset/2

**/
STATIC
EFI_STATUS
ExecuteJMP8 (
  IN VM_CONTEXT *VmPtr
  )
{
  UINT8 Opcode;
  UINT8 ConditionFlag;
  UINT8 CompareSet;
  INT8  Offset;

  //
  // Decode instruction.
  //
  Opcode        = GETOPCODE (VmPtr);
  CompareSet    = (UINT8) ((Opcode & JMP_M_CS) ? 1 : 0);
  ConditionFlag = (UINT8) VMFLAG_ISSET (VmPtr, VMFLAGS_CC);

  //
  // If we haven't met the condition, then simply advance the IP and return
  //
  if (Opcode & CONDITION_M_CONDITIONAL) {
    if (CompareSet != ConditionFlag) {
      VmPtr->Ip += 2;
      return EFI_SUCCESS;
    }
  }
  //
  // Get the offset from the instruction stream. It's relative to the
  // following instruction, and divided by 2.
  //
  Offset = VmReadImmed8 (VmPtr, 1);
  //
  // Want to check for offset == -2 and then raise an exception?
  //
  VmPtr->Ip += (Offset * 2) + 2;
  return EFI_SUCCESS;
}


/**
  Execute the EBC MOVI

  @param  VmPtr             pointer to a VM context

  @return Standard EFI_STATUS
  @return Instruction syntax:
  @return MOVI[b|w|d|q][w|d|q] {@}R1 {Index16}, ImmData16|32|64
  @return First variable character specifies the move size
  @return Second variable character specifies size of the immediate data
  @return Sign-extend the immediate data to the size of the operation, and zero-extend
  @return if storing to a register.
  @return Operand1 direct with index/immed is invalid.

**/
STATIC
EFI_STATUS
ExecuteMOVI (
  IN VM_CONTEXT *VmPtr
  )
{
  UINT8   Opcode;
  UINT8   Operands;
  UINT8   Size;
  INT16   Index16;
  INT64   ImmData64;
  UINT64  Op1;
  UINT64  Mask64;

  //
  // Get the opcode and operands byte so we can get R1 and R2
  //
  Opcode    = GETOPCODE (VmPtr);
  Operands  = GETOPERANDS (VmPtr);

  //
  // Get the index (16-bit) if present
  //
  if (Operands & MOVI_M_IMMDATA) {
    Index16 = VmReadIndex16 (VmPtr, 2);
    Size    = 4;
  } else {
    Index16 = 0;
    Size    = 2;
  }
  //
  // Extract the immediate data. Sign-extend always.
  //
  if ((Opcode & MOVI_M_DATAWIDTH) == MOVI_DATAWIDTH16) {
    ImmData64 = (INT64) (INT16) VmReadImmed16 (VmPtr, Size);
    Size += 2;
  } else if ((Opcode & MOVI_M_DATAWIDTH) == MOVI_DATAWIDTH32) {
    ImmData64 = (INT64) (INT32) VmReadImmed32 (VmPtr, Size);
    Size += 4;
  } else if ((Opcode & MOVI_M_DATAWIDTH) == MOVI_DATAWIDTH64) {
    ImmData64 = (INT64) VmReadImmed64 (VmPtr, Size);
    Size += 8;
  } else {
    //
    // Invalid encoding
    //
    EbcDebugSignalException (
      EXCEPT_EBC_INSTRUCTION_ENCODING,
      EXCEPTION_FLAG_FATAL,
      VmPtr
      );
    return EFI_UNSUPPORTED;
  }
  //
  // Now write back the result
  //
  if (!OPERAND1_INDIRECT (Operands)) {
    //
    // Operand1 direct. Make sure it didn't have an index.
    //
    if (Operands & MOVI_M_IMMDATA) {
      EbcDebugSignalException (
        EXCEPT_EBC_INSTRUCTION_ENCODING,
        EXCEPTION_FLAG_FATAL,
        VmPtr
        );
      return EFI_UNSUPPORTED;
    }
    //
    // Writing directly to a register. Clear unused bits.
    //
    if ((Operands & MOVI_M_MOVEWIDTH) == MOVI_MOVEWIDTH8) {
      Mask64 = 0x000000FF;
    } else if ((Operands & MOVI_M_MOVEWIDTH) == MOVI_MOVEWIDTH16) {
      Mask64 = 0x0000FFFF;
    } else if ((Operands & MOVI_M_MOVEWIDTH) == MOVI_MOVEWIDTH32) {
      Mask64 = 0x00000000FFFFFFFF;
    } else {
      Mask64 = (UINT64)~0;
    }

    VmPtr->R[OPERAND1_REGNUM (Operands)] = ImmData64 & Mask64;
  } else {
    //
    // Get the address then write back based on size of the move
    //
    Op1 = (UINT64) VmPtr->R[OPERAND1_REGNUM (Operands)] + Index16;
    if ((Operands & MOVI_M_MOVEWIDTH) == MOVI_MOVEWIDTH8) {
      VmWriteMem8 (VmPtr, (UINTN) Op1, (UINT8) ImmData64);
    } else if ((Operands & MOVI_M_MOVEWIDTH) == MOVI_MOVEWIDTH16) {
      VmWriteMem16 (VmPtr, (UINTN) Op1, (UINT16) ImmData64);
    } else if ((Operands & MOVI_M_MOVEWIDTH) == MOVI_MOVEWIDTH32) {
      VmWriteMem32 (VmPtr, (UINTN) Op1, (UINT32) ImmData64);
    } else {
      VmWriteMem64 (VmPtr, (UINTN) Op1, ImmData64);
    }
  }
  //
  // Advance the instruction pointer
  //
  VmPtr->Ip += Size;
  return EFI_SUCCESS;
}


/**
  Execute the EBC MOV immediate natural. This instruction moves an immediate
  index value into a register or memory location.

  @param  VmPtr             pointer to a VM context

  @return Standard EFI_STATUS
  @return Instruction syntax:
  @return MOVIn[w|d|q] {@}R1 {Index16}, Index16|32|64

**/
STATIC
EFI_STATUS
ExecuteMOVIn (
  IN VM_CONTEXT *VmPtr
  )
{
  UINT8   Opcode;
  UINT8   Operands;
  UINT8   Size;
  INT16   Index16;
  INT16   ImmedIndex16;
  INT32   ImmedIndex32;
  INT64   ImmedIndex64;
  UINT64  Op1;

  //
  // Get the opcode and operands byte so we can get R1 and R2
  //
  Opcode    = GETOPCODE (VmPtr);
  Operands  = GETOPERANDS (VmPtr);

  //
  // Get the operand1 index (16-bit) if present
  //
  if (Operands & MOVI_M_IMMDATA) {
    Index16 = VmReadIndex16 (VmPtr, 2);
    Size    = 4;
  } else {
    Index16 = 0;
    Size    = 2;
  }
  //
  // Extract the immediate data and convert to a 64-bit index.
  //
  if ((Opcode & MOVI_M_DATAWIDTH) == MOVI_DATAWIDTH16) {
    ImmedIndex16  = VmReadIndex16 (VmPtr, Size);
    ImmedIndex64  = (INT64) ImmedIndex16;
    Size += 2;
  } else if ((Opcode & MOVI_M_DATAWIDTH) == MOVI_DATAWIDTH32) {
    ImmedIndex32  = VmReadIndex32 (VmPtr, Size);
    ImmedIndex64  = (INT64) ImmedIndex32;
    Size += 4;
  } else if ((Opcode & MOVI_M_DATAWIDTH) == MOVI_DATAWIDTH64) {
    ImmedIndex64 = VmReadIndex64 (VmPtr, Size);
    Size += 8;
  } else {
    //
    // Invalid encoding
    //
    EbcDebugSignalException (
      EXCEPT_EBC_INSTRUCTION_ENCODING,
      EXCEPTION_FLAG_FATAL,
      VmPtr
      );
    return EFI_UNSUPPORTED;
  }
  //
  // Now write back the result
  //
  if (!OPERAND1_INDIRECT (Operands)) {
    //
    // Check for MOVIn R1 Index16, Immed (not indirect, with index), which
    // is illegal
    //
    if (Operands & MOVI_M_IMMDATA) {
      EbcDebugSignalException (
        EXCEPT_EBC_INSTRUCTION_ENCODING,
        EXCEPTION_FLAG_FATAL,
        VmPtr
        );
      return EFI_UNSUPPORTED;
    }

    VmPtr->R[OPERAND1_REGNUM (Operands)] = ImmedIndex64;
  } else {
    //
    // Get the address
    //
    Op1 = (UINT64) VmPtr->R[OPERAND1_REGNUM (Operands)] + Index16;
    VmWriteMemN (VmPtr, (UINTN) Op1, (INTN) ImmedIndex64);
  }
  //
  // Advance the instruction pointer
  //
  VmPtr->Ip += Size;
  return EFI_SUCCESS;
}


/**
  Execute the EBC MOVREL instruction.
  Dest <- Ip + ImmData

  @param  VmPtr             pointer to a VM context

  @return Standard EFI_STATUS
  @return Instruction syntax:
  @return MOVREL[w|d|q] {@}R1 {Index16}, ImmData16|32|64

**/
STATIC
EFI_STATUS
ExecuteMOVREL (
  IN VM_CONTEXT *VmPtr
  )
{
  UINT8   Opcode;
  UINT8   Operands;
  UINT8   Size;
  INT16   Index16;
  INT64   ImmData64;
  UINT64  Op1;
  UINT64  Op2;

  //
  // Get the opcode and operands byte so we can get R1 and R2
  //
  Opcode    = GETOPCODE (VmPtr);
  Operands  = GETOPERANDS (VmPtr);

  //
  // Get the Operand 1 index (16-bit) if present
  //
  if (Operands & MOVI_M_IMMDATA) {
    Index16 = VmReadIndex16 (VmPtr, 2);
    Size    = 4;
  } else {
    Index16 = 0;
    Size    = 2;
  }
  //
  // Get the immediate data.
  //
  if ((Opcode & MOVI_M_DATAWIDTH) == MOVI_DATAWIDTH16) {
    ImmData64 = (INT64) VmReadImmed16 (VmPtr, Size);
    Size += 2;
  } else if ((Opcode & MOVI_M_DATAWIDTH) == MOVI_DATAWIDTH32) {
    ImmData64 = (INT64) VmReadImmed32 (VmPtr, Size);
    Size += 4;
  } else if ((Opcode & MOVI_M_DATAWIDTH) == MOVI_DATAWIDTH64) {
    ImmData64 = VmReadImmed64 (VmPtr, Size);
    Size += 8;
  } else {
    //
    // Invalid encoding
    //
    EbcDebugSignalException (
      EXCEPT_EBC_INSTRUCTION_ENCODING,
      EXCEPTION_FLAG_FATAL,
      VmPtr
      );
    return EFI_UNSUPPORTED;
  }
  //
  // Compute the value and write back the result
  //
  Op2 = (UINT64) ((INT64) ((UINT64) (UINTN) VmPtr->Ip) + (INT64) ImmData64 + Size);
  if (!OPERAND1_INDIRECT (Operands)) {
    //
    // Check for illegal combination of operand1 direct with immediate data
    //
    if (Operands & MOVI_M_IMMDATA) {
      EbcDebugSignalException (
        EXCEPT_EBC_INSTRUCTION_ENCODING,
        EXCEPTION_FLAG_FATAL,
        VmPtr
        );
      return EFI_UNSUPPORTED;
    }

    VmPtr->R[OPERAND1_REGNUM (Operands)] = (VM_REGISTER) Op2;
  } else {
    //
    // Get the address = [Rx] + Index16
    // Write back the result. Always a natural size write, since
    // we're talking addresses here.
    //
    Op1 = (UINT64) VmPtr->R[OPERAND1_REGNUM (Operands)] + Index16;
    VmWriteMemN (VmPtr, (UINTN) Op1, (UINTN) Op2);
  }
  //
  // Advance the instruction pointer
  //
  VmPtr->Ip += Size;
  return EFI_SUCCESS;
}


/**
  Execute the EBC MOVsnw instruction. This instruction loads a signed
  natural value from memory or register to another memory or register. On
  32-bit machines, the value gets sign-extended to 64 bits if the destination
  is a register.

  @param  VmPtr             pointer to a VM context

  @return Standard EFI_STATUS
  @return Instruction syntax:
  @return MOVsnw {@}R1 {Index16}, {@}R2 {Index16|Immed16}
  @return 0:7 1=>operand1 index present
  @return 0:6 1=>operand2 index present

**/
STATIC
EFI_STATUS
ExecuteMOVsnw (
  IN VM_CONTEXT *VmPtr
  )
{
  UINT8   Opcode;
  UINT8   Operands;
  UINT8   Size;
  INT16   Op1Index;
  INT16   Op2Index;
  UINT64  Op2;

  //
  // Get the opcode and operand bytes
  //
  Opcode              = GETOPCODE (VmPtr);
  Operands            = GETOPERANDS (VmPtr);

  Op1Index            = Op2Index = 0;

  //
  // Get the indexes if present.
  //
  Size = 2;
  if (Opcode & OPCODE_M_IMMED_OP1) {
    if (OPERAND1_INDIRECT (Operands)) {
      Op1Index = VmReadIndex16 (VmPtr, 2);
    } else {
      //
      // Illegal form operand1 direct with index:  MOVsnw R1 Index16, {@}R2
      //
      EbcDebugSignalException (
        EXCEPT_EBC_INSTRUCTION_ENCODING,
        EXCEPTION_FLAG_FATAL,
        VmPtr
        );
      return EFI_UNSUPPORTED;
    }

    Size += sizeof (UINT16);
  }

  if (Opcode & OPCODE_M_IMMED_OP2) {
    if (OPERAND2_INDIRECT (Operands)) {
      Op2Index = VmReadIndex16 (VmPtr, Size);
    } else {
      Op2Index = VmReadImmed16 (VmPtr, Size);
    }

    Size += sizeof (UINT16);
  }
  //
  // Get the data from the source.
  //
  Op2 = (INT64) ((INTN) (VmPtr->R[OPERAND2_REGNUM (Operands)] + Op2Index));
  if (OPERAND2_INDIRECT (Operands)) {
    Op2 = (INT64) (INTN) VmReadMemN (VmPtr, (UINTN) Op2);
  }
  //
  // Now write back the result.
  //
  if (!OPERAND1_INDIRECT (Operands)) {
    VmPtr->R[OPERAND1_REGNUM (Operands)] = Op2;
  } else {
    VmWriteMemN (VmPtr, (UINTN) (VmPtr->R[OPERAND1_REGNUM (Operands)] + Op1Index), (UINTN) Op2);
  }
  //
  // Advance the instruction pointer
  //
  VmPtr->Ip += Size;
  return EFI_SUCCESS;
}


/**
  Execute the EBC MOVsnw instruction. This instruction loads a signed
  natural value from memory or register to another memory or register. On
  32-bit machines, the value gets sign-extended to 64 bits if the destination
  is a register.

  @param  VmPtr             pointer to a VM context

  @return Standard EFI_STATUS
  @return Instruction syntax:
  @return MOVsnd {@}R1 {Indx32}, {@}R2 {Index32|Immed32}
  @return 0:7 1=>operand1 index present
  @return 0:6 1=>operand2 index present

**/
STATIC
EFI_STATUS
ExecuteMOVsnd (
  IN VM_CONTEXT *VmPtr
  )
{
  UINT8   Opcode;
  UINT8   Operands;
  UINT8   Size;
  INT32   Op1Index;
  INT32   Op2Index;
  UINT64  Op2;

  //
  // Get the opcode and operand bytes
  //
  Opcode              = GETOPCODE (VmPtr);
  Operands            = GETOPERANDS (VmPtr);

  Op1Index            = Op2Index = 0;

  //
  // Get the indexes if present.
  //
  Size = 2;
  if (Opcode & OPCODE_M_IMMED_OP1) {
    if (OPERAND1_INDIRECT (Operands)) {
      Op1Index = VmReadIndex32 (VmPtr, 2);
    } else {
      //
      // Illegal form operand1 direct with index:  MOVsnd R1 Index16,..
      //
      EbcDebugSignalException (
        EXCEPT_EBC_INSTRUCTION_ENCODING,
        EXCEPTION_FLAG_FATAL,
        VmPtr
        );
      return EFI_UNSUPPORTED;
    }

    Size += sizeof (UINT32);
  }

  if (Opcode & OPCODE_M_IMMED_OP2) {
    if (OPERAND2_INDIRECT (Operands)) {
      Op2Index = VmReadIndex32 (VmPtr, Size);
    } else {
      Op2Index = VmReadImmed32 (VmPtr, Size);
    }

    Size += sizeof (UINT32);
  }
  //
  // Get the data from the source.
  //
  Op2 = (INT64) ((INTN) (VmPtr->R[OPERAND2_REGNUM (Operands)] + Op2Index));
  if (OPERAND2_INDIRECT (Operands)) {
    Op2 = (INT64) (INTN) VmReadMemN (VmPtr, (UINTN) Op2);
  }
  //
  // Now write back the result.
  //
  if (!OPERAND1_INDIRECT (Operands)) {
    VmPtr->R[OPERAND1_REGNUM (Operands)] = Op2;
  } else {
    VmWriteMemN (VmPtr, (UINTN) (VmPtr->R[OPERAND1_REGNUM (Operands)] + Op1Index), (UINTN) Op2);
  }
  //
  // Advance the instruction pointer
  //
  VmPtr->Ip += Size;
  return EFI_SUCCESS;
}


/**
  Execute the EBC PUSHn instruction

  @param  VmPtr             pointer to a VM context

  @return Standard EFI_STATUS
  @return Instruction syntax:
  @return PUSHn {@}R1 {Index16|Immed16}

**/
STATIC
EFI_STATUS
ExecutePUSHn (
  IN VM_CONTEXT *VmPtr
  )
{
  UINT8 Opcode;
  UINT8 Operands;
  INT16 Index16;
  UINTN DataN;

  //
  // Get opcode and operands
  //
  Opcode    = GETOPCODE (VmPtr);
  Operands  = GETOPERANDS (VmPtr);

  //
  // Get index if present
  //
  if (Opcode & PUSHPOP_M_IMMDATA) {
    if (OPERAND1_INDIRECT (Operands)) {
      Index16 = VmReadIndex16 (VmPtr, 2);
    } else {
      Index16 = VmReadImmed16 (VmPtr, 2);
    }

    VmPtr->Ip += 4;
  } else {
    Index16 = 0;
    VmPtr->Ip += 2;
  }
  //
  // Get the data to push
  //
  if (OPERAND1_INDIRECT (Operands)) {
    DataN = VmReadMemN (VmPtr, (UINTN) (VmPtr->R[OPERAND1_REGNUM (Operands)] + Index16));
  } else {
    DataN = (UINTN) (VmPtr->R[OPERAND1_REGNUM (Operands)] + Index16);
  }
  //
  // Adjust the stack down.
  //
  VmPtr->R[0] -= sizeof (UINTN);
  VmWriteMemN (VmPtr, (UINTN) VmPtr->R[0], DataN);
  return EFI_SUCCESS;
}


/**
  Execute the EBC PUSH instruction

  @param  VmPtr             pointer to a VM context

  @return Standard EFI_STATUS
  @return Instruction syntax:
  @return PUSH[32|64] {@}R1 {Index16|Immed16}

**/
STATIC
EFI_STATUS
ExecutePUSH (
  IN VM_CONTEXT *VmPtr
  )
{
  UINT8   Opcode;
  UINT8   Operands;
  UINT32  Data32;
  UINT64  Data64;
  INT16   Index16;

  //
  // Get opcode and operands
  //
  Opcode    = GETOPCODE (VmPtr);
  Operands  = GETOPERANDS (VmPtr);
  //
  // Get immediate index if present, then advance the IP.
  //
  if (Opcode & PUSHPOP_M_IMMDATA) {
    if (OPERAND1_INDIRECT (Operands)) {
      Index16 = VmReadIndex16 (VmPtr, 2);
    } else {
      Index16 = VmReadImmed16 (VmPtr, 2);
    }

    VmPtr->Ip += 4;
  } else {
    Index16 = 0;
    VmPtr->Ip += 2;
  }
  //
  // Get the data to push
  //
  if (Opcode & PUSHPOP_M_64) {
    if (OPERAND1_INDIRECT (Operands)) {
      Data64 = VmReadMem64 (VmPtr, (UINTN) (VmPtr->R[OPERAND1_REGNUM (Operands)] + Index16));
    } else {
      Data64 = (UINT64) VmPtr->R[OPERAND1_REGNUM (Operands)] + Index16;
    }
    //
    // Adjust the stack down, then write back the data
    //
    VmPtr->R[0] -= sizeof (UINT64);
    VmWriteMem64 (VmPtr, (UINTN) VmPtr->R[0], Data64);
  } else {
    //
    // 32-bit data
    //
    if (OPERAND1_INDIRECT (Operands)) {
      Data32 = VmReadMem32 (VmPtr, (UINTN) (VmPtr->R[OPERAND1_REGNUM (Operands)] + Index16));
    } else {
      Data32 = (UINT32) VmPtr->R[OPERAND1_REGNUM (Operands)] + Index16;
    }
    //
    // Adjust the stack down and write the data
    //
    VmPtr->R[0] -= sizeof (UINT32);
    VmWriteMem32 (VmPtr, (UINTN) VmPtr->R[0], Data32);
  }

  return EFI_SUCCESS;
}


/**
  Execute the EBC POPn instruction

  @param  VmPtr             pointer to a VM context

  @return Standard EFI_STATUS
  @return Instruction syntax:
  @return POPn {@}R1 {Index16|Immed16}

**/
STATIC
EFI_STATUS
ExecutePOPn (
  IN VM_CONTEXT *VmPtr
  )
{
  UINT8 Opcode;
  UINT8 Operands;
  INT16 Index16;
  UINTN DataN;

  //
  // Get opcode and operands
  //
  Opcode    = GETOPCODE (VmPtr);
  Operands  = GETOPERANDS (VmPtr);
  //
  // Get immediate data if present, and advance the IP
  //
  if (Opcode & PUSHPOP_M_IMMDATA) {
    if (OPERAND1_INDIRECT (Operands)) {
      Index16 = VmReadIndex16 (VmPtr, 2);
    } else {
      Index16 = VmReadImmed16 (VmPtr, 2);
    }

    VmPtr->Ip += 4;
  } else {
    Index16 = 0;
    VmPtr->Ip += 2;
  }
  //
  // Read the data off the stack, then adjust the stack pointer
  //
  DataN = VmReadMemN (VmPtr, (UINTN) VmPtr->R[0]);
  VmPtr->R[0] += sizeof (UINTN);
  //
  // Do the write-back
  //
  if (OPERAND1_INDIRECT (Operands)) {
    VmWriteMemN (VmPtr, (UINTN) (VmPtr->R[OPERAND1_REGNUM (Operands)] + Index16), DataN);
  } else {
    VmPtr->R[OPERAND1_REGNUM (Operands)] = (INT64) (UINT64) ((UINTN) DataN + Index16);
  }

  return EFI_SUCCESS;
}


/**
  Execute the EBC POP instruction

  @param  VmPtr             pointer to a VM context

  @return Standard EFI_STATUS
  @return Instruction syntax:
  @return POP {@}R1 {Index16|Immed16}

**/
STATIC
EFI_STATUS
ExecutePOP (
  IN VM_CONTEXT *VmPtr
  )
{
  UINT8   Opcode;
  UINT8   Operands;
  INT16   Index16;
  INT32   Data32;
  UINT64  Data64;

  //
  // Get opcode and operands
  //
  Opcode    = GETOPCODE (VmPtr);
  Operands  = GETOPERANDS (VmPtr);
  //
  // Get immediate data if present, and advance the IP.
  //
  if (Opcode & PUSHPOP_M_IMMDATA) {
    if (OPERAND1_INDIRECT (Operands)) {
      Index16 = VmReadIndex16 (VmPtr, 2);
    } else {
      Index16 = VmReadImmed16 (VmPtr, 2);
    }

    VmPtr->Ip += 4;
  } else {
    Index16 = 0;
    VmPtr->Ip += 2;
  }
  //
  // Get the data off the stack, then write it to the appropriate location
  //
  if (Opcode & PUSHPOP_M_64) {
    //
    // Read the data off the stack, then adjust the stack pointer
    //
    Data64 = VmReadMem64 (VmPtr, (UINTN) VmPtr->R[0]);
    VmPtr->R[0] += sizeof (UINT64);
    //
    // Do the write-back
    //
    if (OPERAND1_INDIRECT (Operands)) {
      VmWriteMem64 (VmPtr, (UINTN) (VmPtr->R[OPERAND1_REGNUM (Operands)] + Index16), Data64);
    } else {
      VmPtr->R[OPERAND1_REGNUM (Operands)] = Data64 + Index16;
    }
  } else {
    //
    // 32-bit pop. Read it off the stack and adjust the stack pointer
    //
    Data32 = (INT32) VmReadMem32 (VmPtr, (UINTN) VmPtr->R[0]);
    VmPtr->R[0] += sizeof (UINT32);
    //
    // Do the write-back
    //
    if (OPERAND1_INDIRECT (Operands)) {
      VmWriteMem32 (VmPtr, (UINTN) (VmPtr->R[OPERAND1_REGNUM (Operands)] + Index16), Data32);
    } else {
      VmPtr->R[OPERAND1_REGNUM (Operands)] = (INT64) Data32 + Index16;
    }
  }

  return EFI_SUCCESS;
}


/**
  Implements the EBC CALL instruction.
  Instruction format:
  CALL64 Immed64
  CALL32 {@}R1 {Immed32|Index32}
  CALLEX64 Immed64
  CALLEX16 {@}R1 {Immed32}
  If Rx == R0, then it's a PC relative call to PC = PC + imm32.

  @param  VmPtr             pointer to a VM context.

  @return Standard EFI_STATUS

**/
STATIC
EFI_STATUS
ExecuteCALL (
  IN VM_CONTEXT *VmPtr
  )
{
  UINT8 Opcode;
  UINT8 Operands;
  INT32 Immed32;
  UINT8 Size;
  INT64 Immed64;
  VOID  *FramePtr;

  //
  // Get opcode and operands
  //
  Opcode    = GETOPCODE (VmPtr);
  Operands  = GETOPERANDS (VmPtr);
  //
  // Assign these as well to avoid compiler warnings
  //
  Immed64   = 0;
  Immed32   = 0;

  FramePtr  = VmPtr->FramePtr;
  //
  // Determine the instruction size, and get immediate data if present
  //
  if (Opcode & OPCODE_M_IMMDATA) {
    if (Opcode & OPCODE_M_IMMDATA64) {
      Immed64 = VmReadImmed64 (VmPtr, 2);
      Size    = 10;
    } else {
      //
      // If register operand is indirect, then the immediate data is an index
      //
      if (OPERAND1_INDIRECT (Operands)) {
        Immed32 = VmReadIndex32 (VmPtr, 2);
      } else {
        Immed32 = VmReadImmed32 (VmPtr, 2);
      }

      Size = 6;
    }
  } else {
    Size = 2;
  }
  //
  // If it's a call to EBC, adjust the stack pointer down 16 bytes and
  // put our return address and frame pointer on the VM stack.
  //
  if ((Operands & OPERAND_M_NATIVE_CALL) == 0) {
    VmPtr->R[0] -= 8;
    VmWriteMemN (VmPtr, (UINTN) VmPtr->R[0], (UINTN) FramePtr);
    VmPtr->FramePtr = (VOID *) (UINTN) VmPtr->R[0];
    VmPtr->R[0] -= 8;
    VmWriteMem64 (VmPtr, (UINTN) VmPtr->R[0], (UINT64) (UINTN) (VmPtr->Ip + Size));
  }
  //
  // If 64-bit data, then absolute jump only
  //
  if (Opcode & OPCODE_M_IMMDATA64) {
    //
    // Native or EBC call?
    //
    if ((Operands & OPERAND_M_NATIVE_CALL) == 0) {
      VmPtr->Ip = (VMIP) (UINTN) Immed64;
    } else {
      //
      // Call external function, get the return value, and advance the IP
      //
      EbcLLCALLEX (VmPtr, (UINTN) Immed64, (UINTN) VmPtr->R[0], FramePtr, Size);
    }
  } else {
    //
    // Get the register data. If operand1 == 0, then ignore register and
    // take immediate data as relative or absolute address.
    // Compiler should take care of upper bits if 32-bit machine.
    //
    if (OPERAND1_REGNUM (Operands) != 0) {
      Immed64 = (UINT64) (UINTN) VmPtr->R[OPERAND1_REGNUM (Operands)];
    }
    //
    // Get final address
    //
    if (OPERAND1_INDIRECT (Operands)) {
      Immed64 = (INT64) (UINT64) (UINTN) VmReadMemN (VmPtr, (UINTN) (Immed64 + Immed32));
    } else {
      Immed64 += Immed32;
    }
    //
    // Now determine if external call, and then if relative or absolute
    //
    if ((Operands & OPERAND_M_NATIVE_CALL) == 0) {
      //
      // EBC call. Relative or absolute? If relative, then it's relative to the
      // start of the next instruction.
      //
      if (Operands & OPERAND_M_RELATIVE_ADDR) {
        VmPtr->Ip += Immed64 + Size;
      } else {
        VmPtr->Ip = (VMIP) (UINTN) Immed64;
      }
    } else {
      //
      // Native call. Relative or absolute?
      //
      if (Operands & OPERAND_M_RELATIVE_ADDR) {
        EbcLLCALLEX (VmPtr, (UINTN) (Immed64 + VmPtr->Ip + Size), (UINTN) VmPtr->R[0], FramePtr, Size);
      } else {
        if (VmPtr->StopFlags & STOPFLAG_BREAK_ON_CALLEX) {
          CpuBreakpoint ();
        }

        EbcLLCALLEX (VmPtr, (UINTN) Immed64, (UINTN) VmPtr->R[0], FramePtr, Size);
      }
    }
  }

  return EFI_SUCCESS;
}


/**
  Execute the EBC RET instruction

  @param  VmPtr             pointer to a VM context

  @return Standard EFI_STATUS
  @return Instruction syntax:
  @return RET

**/
STATIC
EFI_STATUS
ExecuteRET (
  IN VM_CONTEXT *VmPtr
  )
{
  //
  // If we're at the top of the stack, then simply set the done
  // flag and return
  //
  if (VmPtr->StackRetAddr == (UINT64) VmPtr->R[0]) {
    VmPtr->StopFlags |= STOPFLAG_APP_DONE;
  } else {
    //
    // Pull the return address off the VM app's stack and set the IP
    // to it
    //
    if (!IS_ALIGNED ((UINTN) VmPtr->R[0], sizeof (UINT16))) {
      EbcDebugSignalException (
        EXCEPT_EBC_ALIGNMENT_CHECK,
        EXCEPTION_FLAG_FATAL,
        VmPtr
        );
    }
    //
    // Restore the IP and frame pointer from the stack
    //
    VmPtr->Ip = (VMIP) (UINTN) VmReadMem64 (VmPtr, (UINTN) VmPtr->R[0]);
    VmPtr->R[0] += 8;
    VmPtr->FramePtr = (VOID *) VmReadMemN (VmPtr, (UINTN) VmPtr->R[0]);
    VmPtr->R[0] += 8;
  }

  return EFI_SUCCESS;
}


/**
  Execute the EBC CMP instruction

  @param  VmPtr             pointer to a VM context

  @return Standard EFI_STATUS
  @return Instruction syntax:
  @return CMP[32|64][eq|lte|gte|ulte|ugte] R1, {@}R2 {Index16|Immed16}

**/
STATIC
EFI_STATUS
ExecuteCMP (
  IN VM_CONTEXT *VmPtr
  )
{
  UINT8   Opcode;
  UINT8   Operands;
  UINT8   Size;
  INT16   Index16;
  UINT32  Flag;
  INT64   Op2;
  INT64   Op1;

  //
  // Get opcode and operands
  //
  Opcode    = GETOPCODE (VmPtr);
  Operands  = GETOPERANDS (VmPtr);
  //
  // Get the register data we're going to compare to
  //
  Op1 = VmPtr->R[OPERAND1_REGNUM (Operands)];
  //
  // Get immediate data
  //
  if (Opcode & OPCODE_M_IMMDATA) {
    if (OPERAND2_INDIRECT (Operands)) {
      Index16 = VmReadIndex16 (VmPtr, 2);
    } else {
      Index16 = VmReadImmed16 (VmPtr, 2);
    }

    Size = 4;
  } else {
    Index16 = 0;
    Size    = 2;
  }
  //
  // Now get Op2
  //
  if (OPERAND2_INDIRECT (Operands)) {
    if (Opcode & OPCODE_M_64BIT) {
      Op2 = (INT64) VmReadMem64 (VmPtr, (UINTN) (VmPtr->R[OPERAND2_REGNUM (Operands)] + Index16));
    } else {
      //
      // 32-bit operations. 0-extend the values for all cases.
      //
      Op2 = (INT64) (UINT64) ((UINT32) VmReadMem32 (VmPtr, (UINTN) (VmPtr->R[OPERAND2_REGNUM (Operands)] + Index16)));
    }
  } else {
    Op2 = VmPtr->R[OPERAND2_REGNUM (Operands)] + Index16;
  }
  //
  // Now do the compare
  //
  Flag = 0;
  if (Opcode & OPCODE_M_64BIT) {
    //
    // 64-bit compares
    //
    switch (Opcode & OPCODE_M_OPCODE) {
    case OPCODE_CMPEQ:
      if (Op1 == Op2) {
        Flag = 1;
      }
      break;

    case OPCODE_CMPLTE:
      if (Op1 <= Op2) {
        Flag = 1;
      }
      break;

    case OPCODE_CMPGTE:
      if (Op1 >= Op2) {
        Flag = 1;
      }
      break;

    case OPCODE_CMPULTE:
      if ((UINT64) Op1 <= (UINT64) Op2) {
        Flag = 1;
      }
      break;

    case OPCODE_CMPUGTE:
      if ((UINT64) Op1 >= (UINT64) Op2) {
        Flag = 1;
      }
      break;

    default:
      ASSERT (0);
    }
  } else {
    //
    // 32-bit compares
    //
    switch (Opcode & OPCODE_M_OPCODE) {
    case OPCODE_CMPEQ:
      if ((INT32) Op1 == (INT32) Op2) {
        Flag = 1;
      }
      break;

    case OPCODE_CMPLTE:
      if ((INT32) Op1 <= (INT32) Op2) {
        Flag = 1;
      }
      break;

    case OPCODE_CMPGTE:
      if ((INT32) Op1 >= (INT32) Op2) {
        Flag = 1;
      }
      break;

    case OPCODE_CMPULTE:
      if ((UINT32) Op1 <= (UINT32) Op2) {
        Flag = 1;
      }
      break;

    case OPCODE_CMPUGTE:
      if ((UINT32) Op1 >= (UINT32) Op2) {
        Flag = 1;
      }
      break;

    default:
      ASSERT (0);
    }
  }
  //
  // Now set the flag accordingly for the comparison
  //
  if (Flag) {
    VMFLAG_SET (VmPtr, VMFLAGS_CC);
  } else {
    VMFLAG_CLEAR (VmPtr, VMFLAGS_CC);
  }
  //
  // Advance the IP
  //
  VmPtr->Ip += Size;
  return EFI_SUCCESS;
}


/**
  Execute the EBC CMPI instruction

  @param  VmPtr             pointer to a VM context

  @return Standard EFI_STATUS
  @return Instruction syntax:
  @return CMPI[32|64]{w|d}[eq|lte|gte|ulte|ugte] {@}Rx {Index16}, Immed16|Immed32

**/
STATIC
EFI_STATUS
ExecuteCMPI (
  IN VM_CONTEXT *VmPtr
  )
{
  UINT8   Opcode;
  UINT8   Operands;
  UINT8   Size;
  INT64   Op1;
  INT64   Op2;
  INT16   Index16;
  UINT32  Flag;

  //
  // Get opcode and operands
  //
  Opcode    = GETOPCODE (VmPtr);
  Operands  = GETOPERANDS (VmPtr);

  //
  // Get operand1 index if present
  //
  Size = 2;
  if (Operands & OPERAND_M_CMPI_INDEX) {
    Index16 = VmReadIndex16 (VmPtr, 2);
    Size += 2;
  } else {
    Index16 = 0;
  }
  //
  // Get operand1 data we're going to compare to
  //
  Op1 = (INT64) VmPtr->R[OPERAND1_REGNUM (Operands)];
  if (OPERAND1_INDIRECT (Operands)) {
    //
    // Indirect operand1. Fetch 32 or 64-bit value based on compare size.
    //
    if (Opcode & OPCODE_M_CMPI64) {
      Op1 = (INT64) VmReadMem64 (VmPtr, (UINTN) Op1 + Index16);
    } else {
      Op1 = (INT64) VmReadMem32 (VmPtr, (UINTN) Op1 + Index16);
    }
  } else {
    //
    // Better not have been an index with direct. That is, CMPI R1 Index,...
    // is illegal.
    //
    if (Operands & OPERAND_M_CMPI_INDEX) {
      EbcDebugSignalException (
        EXCEPT_EBC_INSTRUCTION_ENCODING,
        EXCEPTION_FLAG_ERROR,
        VmPtr
        );
      VmPtr->Ip += Size;
      return EFI_UNSUPPORTED;
    }
  }
  //
  // Get immediate data -- 16- or 32-bit sign extended
  //
  if (Opcode & OPCODE_M_CMPI32_DATA) {
    Op2 = (INT64) VmReadImmed32 (VmPtr, Size);
    Size += 4;
  } else {
    //
    // 16-bit immediate data. Sign extend always.
    //
    Op2 = (INT64) ((INT16) VmReadImmed16 (VmPtr, Size));
    Size += 2;
  }
  //
  // Now do the compare
  //
  Flag = 0;
  if (Opcode & OPCODE_M_CMPI64) {
    //
    // 64 bit comparison
    //
    switch (Opcode & OPCODE_M_OPCODE) {
    case OPCODE_CMPIEQ:
      if (Op1 == (INT64) Op2) {
        Flag = 1;
      }
      break;

    case OPCODE_CMPILTE:
      if (Op1 <= (INT64) Op2) {
        Flag = 1;
      }
      break;

    case OPCODE_CMPIGTE:
      if (Op1 >= (INT64) Op2) {
        Flag = 1;
      }
      break;

    case OPCODE_CMPIULTE:
      if ((UINT64) Op1 <= (UINT64) ((UINT32) Op2)) {
        Flag = 1;
      }
      break;

    case OPCODE_CMPIUGTE:
      if ((UINT64) Op1 >= (UINT64) ((UINT32) Op2)) {
        Flag = 1;
      }
      break;

    default:
      ASSERT (0);
    }
  } else {
    //
    // 32-bit comparisons
    //
    switch (Opcode & OPCODE_M_OPCODE) {
    case OPCODE_CMPIEQ:
      if ((INT32) Op1 == Op2) {
        Flag = 1;
      }
      break;

    case OPCODE_CMPILTE:
      if ((INT32) Op1 <= Op2) {
        Flag = 1;
      }
      break;

    case OPCODE_CMPIGTE:
      if ((INT32) Op1 >= Op2) {
        Flag = 1;
      }
      break;

    case OPCODE_CMPIULTE:
      if ((UINT32) Op1 <= (UINT32) Op2) {
        Flag = 1;
      }
      break;

    case OPCODE_CMPIUGTE:
      if ((UINT32) Op1 >= (UINT32) Op2) {
        Flag = 1;
      }
      break;

    default:
      ASSERT (0);
    }
  }
  //
  // Now set the flag accordingly for the comparison
  //
  if (Flag) {
    VMFLAG_SET (VmPtr, VMFLAGS_CC);
  } else {
    VMFLAG_CLEAR (VmPtr, VMFLAGS_CC);
  }
  //
  // Advance the IP
  //
  VmPtr->Ip += Size;
  return EFI_SUCCESS;
}


/**
  Execute the EBC NOT instruction

  @param  VmPtr             pointer to a VM context
  @param  Op1               Operand 1 from the instruction
  @param  Op2               Operand 2 from the instruction

  @return ~Op2
  @return Instruction syntax:
  @return NOT[32|64] {@}R1, {@}R2 {Index16|Immed16}

**/
STATIC
UINT64
ExecuteNOT (
  IN VM_CONTEXT     *VmPtr,
  IN UINT64         Op1,
  IN UINT64         Op2
  )
{
  return ~Op2;
}


/**
  Execute the EBC NEG instruction

  @param  VmPtr             pointer to a VM context
  @param  Op1               Operand 1 from the instruction
  @param  Op2               Operand 2 from the instruction

  @return Op2 * -1
  @return Instruction syntax:
  @return NEG[32|64] {@}R1, {@}R2 {Index16|Immed16}

**/
STATIC
UINT64
ExecuteNEG (
  IN VM_CONTEXT   *VmPtr,
  IN UINT64       Op1,
  IN UINT64       Op2
  )
{
  return ~Op2 + 1;
}


/**
  Execute the EBC ADD instruction

  @param  VmPtr             pointer to a VM context
  @param  Op1               Operand 1 from the instruction
  @param  Op2               Operand 2 from the instruction

  @return Op1 + Op2
  @return Instruction syntax:
  @return ADD[32|64] {@}R1, {@}R2 {Index16}

**/
STATIC
UINT64
ExecuteADD (
  IN VM_CONTEXT   *VmPtr,
  IN UINT64       Op1,
  IN UINT64       Op2
  )
{
  return Op1 + Op2;
}


/**
  Execute the EBC SUB instruction

  @param  VmPtr             pointer to a VM context
  @param  Op1               Operand 1 from the instruction
  @param  Op2               Operand 2 from the instruction

  @retval Op1               Op2 Standard EFI_STATUS
  @return Instruction syntax:
  @return SUB[32|64] {@}R1, {@}R2 {Index16|Immed16}

**/
STATIC
UINT64
ExecuteSUB (
  IN VM_CONTEXT   *VmPtr,
  IN UINT64       Op1,
  IN UINT64       Op2
  )
{
  if (*VmPtr->Ip & DATAMANIP_M_64) {
    return (UINT64) ((INT64) ((INT64) Op1 - (INT64) Op2));
  } else {
    return (UINT64) ((INT64) ((INT32) Op1 - (INT32) Op2));
  }
}


/**
  Execute the EBC MUL instruction

  @param  VmPtr             pointer to a VM context
  @param  Op1               Operand 1 from the instruction
  @param  Op2               Operand 2 from the instruction

  @return Op1 * Op2
  @return Instruction syntax:
  @return MUL[32|64] {@}R1, {@}R2 {Index16|Immed16}

**/
STATIC
UINT64
ExecuteMUL (
  IN VM_CONTEXT   *VmPtr,
  IN UINT64       Op1,
  IN UINT64       Op2
  )
{
  if (*VmPtr->Ip & DATAMANIP_M_64) {
    return MultS64x64 ((INT64)Op1, (INT64)Op2);
  } else {
    return (UINT64) ((INT64) ((INT32) Op1 * (INT32) Op2));
  }
}


/**
  Execute the EBC MULU instruction

  @param  VmPtr             pointer to a VM context
  @param  Op1               Operand 1 from the instruction
  @param  Op2               Operand 2 from the instruction

  @return (unsigned)Op1 * (unsigned)Op2
  @return Instruction syntax:
  @return MULU[32|64] {@}R1, {@}R2 {Index16|Immed16}

**/
STATIC
UINT64
ExecuteMULU (
  IN VM_CONTEXT   *VmPtr,
  IN UINT64       Op1,
  IN UINT64       Op2
  )
{
  if (*VmPtr->Ip & DATAMANIP_M_64) {
    return MultU64x64 (Op1, Op2);
  } else {
    return (UINT64) ((UINT32) Op1 * (UINT32) Op2);
  }
}


/**
  Execute the EBC DIV instruction

  @param  VmPtr             pointer to a VM context
  @param  Op1               Operand 1 from the instruction
  @param  Op2               Operand 2 from the instruction

  @return Op1/Op2
  @return Instruction syntax:
  @return DIV[32|64] {@}R1, {@}R2 {Index16|Immed16}

**/
STATIC
UINT64
ExecuteDIV (
  IN VM_CONTEXT   *VmPtr,
  IN UINT64       Op1,
  IN UINT64       Op2
  )
{
  INT64   Remainder;

  //
  // Check for divide-by-0
  //
  if (Op2 == 0) {
    EbcDebugSignalException (
      EXCEPT_EBC_DIVIDE_ERROR,
      EXCEPTION_FLAG_FATAL,
      VmPtr
      );

    return 0;
  } else {
    if (*VmPtr->Ip & DATAMANIP_M_64) {
      return (UINT64) (DivS64x64Remainder (Op1, Op2, &Remainder));
    } else {
      return (UINT64) ((INT64) ((INT32) Op1 / (INT32) Op2));
    }
  }
}


/**
  Execute the EBC DIVU instruction

  @param  VmPtr             pointer to a VM context
  @param  Op1               Operand 1 from the instruction
  @param  Op2               Operand 2 from the instruction

  @return (unsigned)Op1 / (unsigned)Op2
  @return Instruction syntax:
  @return DIVU[32|64] {@}R1, {@}R2 {Index16|Immed16}

**/
STATIC
UINT64
ExecuteDIVU (
  IN VM_CONTEXT   *VmPtr,
  IN UINT64       Op1,
  IN UINT64       Op2
  )
{
  UINT64  Remainder;

  //
  // Check for divide-by-0
  //
  if (Op2 == 0) {
    EbcDebugSignalException (
      EXCEPT_EBC_DIVIDE_ERROR,
      EXCEPTION_FLAG_FATAL,
      VmPtr
      );
    return 0;
  } else {
    //
    // Get the destination register
    //
    if (*VmPtr->Ip & DATAMANIP_M_64) {
      return (UINT64) (DivU64x64Remainder ((INT64)Op1, (INT64)Op2, &Remainder));
    } else {
      return (UINT64) ((UINT32) Op1 / (UINT32) Op2);
    }
  }
}


/**
  Execute the EBC MOD instruction

  @param  VmPtr             pointer to a VM context
  @param  Op1               Operand 1 from the instruction
  @param  Op2               Operand 2 from the instruction

  @return Op1 MODULUS Op2
  @return Instruction syntax:
  @return MOD[32|64] {@}R1, {@}R2 {Index16|Immed16}

**/
STATIC
UINT64
ExecuteMOD (
  IN VM_CONTEXT   *VmPtr,
  IN UINT64       Op1,
  IN UINT64       Op2
  )
{
  INT64   Remainder;

  //
  // Check for divide-by-0
  //
  if (Op2 == 0) {
    EbcDebugSignalException (
      EXCEPT_EBC_DIVIDE_ERROR,
      EXCEPTION_FLAG_FATAL,
      VmPtr
      );
    return 0;
  } else {
    DivS64x64Remainder ((INT64)Op1, (INT64)Op2, &Remainder);
    return Remainder;
  }
}


/**
  Execute the EBC MODU instruction

  @param  VmPtr             pointer to a VM context
  @param  Op1               Operand 1 from the instruction
  @param  Op2               Operand 2 from the instruction

  @return Op1 UNSIGNED_MODULUS Op2
  @return Instruction syntax:
  @return MODU[32|64] {@}R1, {@}R2 {Index16|Immed16}

**/
STATIC
UINT64
ExecuteMODU (
  IN VM_CONTEXT   *VmPtr,
  IN UINT64       Op1,
  IN UINT64       Op2
  )
{
  UINT64  Remainder;

  //
  // Check for divide-by-0
  //
  if (Op2 == 0) {
    EbcDebugSignalException (
      EXCEPT_EBC_DIVIDE_ERROR,
      EXCEPTION_FLAG_FATAL,
      VmPtr
      );
    return 0;
  } else {
    DivU64x64Remainder (Op1, Op2, &Remainder);
    return Remainder;
  }
}


/**
  Execute the EBC AND instruction

  @param  VmPtr             pointer to a VM context
  @param  Op1               Operand 1 from the instruction
  @param  Op2               Operand 2 from the instruction

  @return Op1 AND Op2
  @return Instruction syntax:
  @return AND[32|64] {@}R1, {@}R2 {Index16|Immed16}

**/
STATIC
UINT64
ExecuteAND (
  IN VM_CONTEXT   *VmPtr,
  IN UINT64       Op1,
  IN UINT64       Op2
  )
{
  return Op1 & Op2;
}


/**
  Execute the EBC OR instruction

  @param  VmPtr             pointer to a VM context
  @param  Op1               Operand 1 from the instruction
  @param  Op2               Operand 2 from the instruction

  @return Op1 OR Op2
  @return Instruction syntax:
  @return OR[32|64] {@}R1, {@}R2 {Index16|Immed16}

**/
STATIC
UINT64
ExecuteOR (
  IN VM_CONTEXT   *VmPtr,
  IN UINT64       Op1,
  IN UINT64       Op2
  )
{
  return Op1 | Op2;
}


/**
  Execute the EBC XOR instruction

  @param  VmPtr             pointer to a VM context
  @param  Op1               Operand 1 from the instruction
  @param  Op2               Operand 2 from the instruction

  @return Op1 XOR Op2
  @return Instruction syntax:
  @return XOR[32|64] {@}R1, {@}R2 {Index16|Immed16}

**/
STATIC
UINT64
ExecuteXOR (
  IN VM_CONTEXT   *VmPtr,
  IN UINT64       Op1,
  IN UINT64       Op2
  )
{
  return Op1 ^ Op2;
}


/**
  Execute the EBC SHL shift left instruction

  @param  VmPtr             pointer to a VM context
  @param  Op1               Operand 1 from the instruction
  @param  Op2               Operand 2 from the instruction

  @return Op1 << Op2
  @return Instruction syntax:
  @return SHL[32|64] {@}R1, {@}R2 {Index16|Immed16}

**/
STATIC
UINT64
ExecuteSHL (
  IN VM_CONTEXT   *VmPtr,
  IN UINT64       Op1,
  IN UINT64       Op2
  )
{
  if (*VmPtr->Ip & DATAMANIP_M_64) {
    return LShiftU64 (Op1, (UINTN)Op2);
  } else {
    return (UINT64) ((UINT32) ((UINT32) Op1 << (UINT32) Op2));
  }
}


/**
  Execute the EBC SHR instruction

  @param  VmPtr             pointer to a VM context
  @param  Op1               Operand 1 from the instruction
  @param  Op2               Operand 2 from the instruction

  @return Op1 >> Op2  (unsigned operands)
  @return Instruction syntax:
  @return SHR[32|64] {@}R1, {@}R2 {Index16|Immed16}

**/
STATIC
UINT64
ExecuteSHR (
  IN VM_CONTEXT   *VmPtr,
  IN UINT64       Op1,
  IN UINT64       Op2
  )
{
  if (*VmPtr->Ip & DATAMANIP_M_64) {
    return RShiftU64 (Op1, (UINTN)Op2);
  } else {
    return (UINT64) ((UINT32) Op1 >> (UINT32) Op2);
  }
}


/**
  Execute the EBC ASHR instruction

  @param  VmPtr             pointer to a VM context
  @param  Op1               Operand 1 from the instruction
  @param  Op2               Operand 2 from the instruction

  @return Op1 >> Op2 (signed)
  @return Instruction syntax:
  @return ASHR[32|64] {@}R1, {@}R2 {Index16|Immed16}

**/
STATIC
UINT64
ExecuteASHR (
  IN VM_CONTEXT   *VmPtr,
  IN UINT64       Op1,
  IN UINT64       Op2
  )
{
  if (*VmPtr->Ip & DATAMANIP_M_64) {
    return ARShiftU64 (Op1, (UINTN)Op2);
  } else {
    return (UINT64) ((INT64) ((INT32) Op1 >> (UINT32) Op2));
  }
}


/**
  Execute the EBC EXTNDB instruction to sign-extend a byte value.

  @param  VmPtr             pointer to a VM context
  @param  Op1               Operand 1 from the instruction
  @param  Op2               Operand 2 from the instruction

  @return (INT64)(INT8)Op2
  @return Instruction syntax:
  @return EXTNDB[32|64] {@}R1, {@}R2 {Index16|Immed16}

**/
STATIC
UINT64
ExecuteEXTNDB (
  IN VM_CONTEXT   *VmPtr,
  IN UINT64       Op1,
  IN UINT64       Op2
  )
{
  INT8  Data8;
  INT64 Data64;
  //
  // Convert to byte, then return as 64-bit signed value to let compiler
  // sign-extend the value
  //
  Data8   = (INT8) Op2;
  Data64  = (INT64) Data8;

  return (UINT64) Data64;
}


/**
  Execute the EBC EXTNDW instruction to sign-extend a 16-bit value.

  @param  VmPtr             pointer to a VM context
  @param  Op1               Operand 1 from the instruction
  @param  Op2               Operand 2 from the instruction

  @return (INT64)(INT16)Op2
  @return Instruction syntax:
  @return EXTNDW[32|64] {@}R1, {@}R2 {Index16|Immed16}

**/
STATIC
UINT64
ExecuteEXTNDW (
  IN VM_CONTEXT   *VmPtr,
  IN UINT64       Op1,
  IN UINT64       Op2
  )
{
  INT16 Data16;
  INT64 Data64;
  //
  // Convert to word, then return as 64-bit signed value to let compiler
  // sign-extend the value
  //
  Data16  = (INT16) Op2;
  Data64  = (INT64) Data16;

  return (UINT64) Data64;
}
//
// Execute the EBC EXTNDD instruction.
//
// Format: EXTNDD {@}Rx, {@}Ry [Index16|Immed16]
//         EXTNDD Dest, Source
//
// Operation:  Dest <- SignExtended((DWORD)Source))
//

/**
  Execute the EBC EXTNDD instruction to sign-extend a 32-bit value.

  @param  VmPtr             pointer to a VM context
  @param  Op1               Operand 1 from the instruction
  @param  Op2               Operand 2 from the instruction

  @return (INT64)(INT32)Op2
  @return Instruction syntax:
  @return EXTNDD[32|64] {@}R1, {@}R2 {Index16|Immed16}

**/
STATIC
UINT64
ExecuteEXTNDD (
  IN VM_CONTEXT   *VmPtr,
  IN UINT64       Op1,
  IN UINT64       Op2
  )
{
  INT32 Data32;
  INT64 Data64;
  //
  // Convert to 32-bit value, then return as 64-bit signed value to let compiler
  // sign-extend the value
  //
  Data32  = (INT32) Op2;
  Data64  = (INT64) Data32;

  return (UINT64) Data64;
}

STATIC
EFI_STATUS
ExecuteSignedDataManip (
  IN VM_CONTEXT   *VmPtr
  )
{
  //
  // Just call the data manipulation function with a flag indicating this
  // is a signed operation.
  //
  return ExecuteDataManip (VmPtr, TRUE);
}

STATIC
EFI_STATUS
ExecuteUnsignedDataManip (
  IN VM_CONTEXT   *VmPtr
  )
{
  //
  // Just call the data manipulation function with a flag indicating this
  // is not a signed operation.
  //
  return ExecuteDataManip (VmPtr, FALSE);
}


/**
  Execute all the EBC data manipulation instructions.
  Since the EBC data manipulation instructions all have the same basic form,
  they can share the code that does the fetch of operands and the write-back
  of the result. This function performs the fetch of the operands (even if
  both are not needed to be fetched, like NOT instruction), dispatches to the
  appropriate subfunction, then writes back the returned result.

  @param  VmPtr             pointer to VM context

  @return Standard EBC status
  @return Format:
  @return INSTRUCITON[32|64] {@}R1, {@}R2 {Immed16|Index16}

**/
STATIC
EFI_STATUS
ExecuteDataManip (
  IN VM_CONTEXT   *VmPtr,
  IN BOOLEAN      IsSignedOp
  )
{
  UINT8   Opcode;
  INT16   Index16;
  UINT8   Operands;
  UINT8   Size;
  UINT64  Op1;
  UINT64  Op2;

  //
  // Get opcode and operands
  //
  Opcode    = GETOPCODE (VmPtr);
  Operands  = GETOPERANDS (VmPtr);

  //
  // Determine if we have immediate data by the opcode
  //
  if (Opcode & DATAMANIP_M_IMMDATA) {
    //
    // Index16 if Ry is indirect, or Immed16 if Ry direct.
    //
    if (OPERAND2_INDIRECT (Operands)) {
      Index16 = VmReadIndex16 (VmPtr, 2);
    } else {
      Index16 = VmReadImmed16 (VmPtr, 2);
    }

    Size = 4;
  } else {
    Index16 = 0;
    Size    = 2;
  }
  //
  // Now get operand2 (source). It's of format {@}R2 {Index16|Immed16}
  //
  Op2 = (UINT64) VmPtr->R[OPERAND2_REGNUM (Operands)] + Index16;
  if (OPERAND2_INDIRECT (Operands)) {
    //
    // Indirect form: @R2 Index16. Fetch as 32- or 64-bit data
    //
    if (Opcode & DATAMANIP_M_64) {
      Op2 = VmReadMem64 (VmPtr, (UINTN) Op2);
    } else {
      //
      // Read as signed value where appropriate.
      //
      if (IsSignedOp) {
        Op2 = (UINT64) (INT64) ((INT32) VmReadMem32 (VmPtr, (UINTN) Op2));
      } else {
        Op2 = (UINT64) VmReadMem32 (VmPtr, (UINTN) Op2);
      }
    }
  } else {
    if ((Opcode & DATAMANIP_M_64) == 0) {
      if (IsSignedOp) {
        Op2 = (UINT64) (INT64) ((INT32) Op2);
      } else {
        Op2 = (UINT64) ((UINT32) Op2);
      }
    }
  }
  //
  // Get operand1 (destination and sometimes also an actual operand)
  // of form {@}R1
  //
  Op1 = VmPtr->R[OPERAND1_REGNUM (Operands)];
  if (OPERAND1_INDIRECT (Operands)) {
    if (Opcode & DATAMANIP_M_64) {
      Op1 = VmReadMem64 (VmPtr, (UINTN) Op1);
    } else {
      if (IsSignedOp) {
        Op1 = (UINT64) (INT64) ((INT32) VmReadMem32 (VmPtr, (UINTN) Op1));
      } else {
        Op1 = (UINT64) VmReadMem32 (VmPtr, (UINTN) Op1);
      }
    }
  } else {
    if ((Opcode & DATAMANIP_M_64) == 0) {
      if (IsSignedOp) {
        Op1 = (UINT64) (INT64) ((INT32) Op1);
      } else {
        Op1 = (UINT64) ((UINT32) Op1);
      }
    }
  }
  //
  // Dispatch to the computation function
  //
  if (((Opcode & OPCODE_M_OPCODE) - OPCODE_NOT) >=
        (sizeof (mDataManipDispatchTable) / sizeof (mDataManipDispatchTable[0]))
        ) {
    EbcDebugSignalException (
      EXCEPT_EBC_INVALID_OPCODE,
      EXCEPTION_FLAG_ERROR,
      VmPtr
      );
    //
    // Advance and return
    //
    VmPtr->Ip += Size;
    return EFI_UNSUPPORTED;
  } else {
    Op2 = mDataManipDispatchTable[(Opcode & OPCODE_M_OPCODE) - OPCODE_NOT](VmPtr, Op1, Op2);
  }
  //
  // Write back the result.
  //
  if (OPERAND1_INDIRECT (Operands)) {
    Op1 = VmPtr->R[OPERAND1_REGNUM (Operands)];
    if (Opcode & DATAMANIP_M_64) {
      VmWriteMem64 (VmPtr, (UINTN) Op1, Op2);
    } else {
      VmWriteMem32 (VmPtr, (UINTN) Op1, (UINT32) Op2);
    }
  } else {
    //
    // Storage back to a register. Write back, clearing upper bits (as per
    // the specification) if 32-bit operation.
    //
    VmPtr->R[OPERAND1_REGNUM (Operands)] = Op2;
    if ((Opcode & DATAMANIP_M_64) == 0) {
      VmPtr->R[OPERAND1_REGNUM (Operands)] &= 0xFFFFFFFF;
    }
  }
  //
  // Advance the instruction pointer
  //
  VmPtr->Ip += Size;
  return EFI_SUCCESS;
}


/**
  Execute the EBC LOADSP instruction

  @param  VmPtr             pointer to a VM context

  @return Standard EFI_STATUS
  @return Instruction syntax:
  @return LOADSP  SP1, R2

**/
STATIC
EFI_STATUS
ExecuteLOADSP (
  IN VM_CONTEXT *VmPtr
  )
{
  UINT8 Operands;

  //
  // Get the operands
  //
  Operands = GETOPERANDS (VmPtr);

  //
  // Do the operation
  //
  switch (OPERAND1_REGNUM (Operands)) {
  //
  // Set flags
  //
  case 0:
    //
    // Spec states that this instruction will not modify reserved bits in
    // the flags register.
    //
    VmPtr->Flags = (VmPtr->Flags &~VMFLAGS_ALL_VALID) | (VmPtr->R[OPERAND2_REGNUM (Operands)] & VMFLAGS_ALL_VALID);
    break;

  default:
    EbcDebugSignalException (
      EXCEPT_EBC_INSTRUCTION_ENCODING,
      EXCEPTION_FLAG_WARNING,
      VmPtr
      );
    VmPtr->Ip += 2;
    return EFI_UNSUPPORTED;
  }

  VmPtr->Ip += 2;
  return EFI_SUCCESS;
}


/**
  Execute the EBC STORESP instruction

  @param  VmPtr             pointer to a VM context

  @return Standard EFI_STATUS
  @return Instruction syntax:
  @return STORESP  Rx, FLAGS|IP

**/
STATIC
EFI_STATUS
ExecuteSTORESP (
  IN VM_CONTEXT *VmPtr
  )
{
  UINT8 Operands;

  //
  // Get the operands
  //
  Operands = GETOPERANDS (VmPtr);

  //
  // Do the operation
  //
  switch (OPERAND2_REGNUM (Operands)) {
  //
  // Get flags
  //
  case 0:
    //
    // Retrieve the value in the flags register, then clear reserved bits
    //
    VmPtr->R[OPERAND1_REGNUM (Operands)] = (UINT64) (VmPtr->Flags & VMFLAGS_ALL_VALID);
    break;

  //
  // Get IP -- address of following instruction
  //
  case 1:
    VmPtr->R[OPERAND1_REGNUM (Operands)] = (UINT64) (UINTN) VmPtr->Ip + 2;
    break;

  default:
    EbcDebugSignalException (
      EXCEPT_EBC_INSTRUCTION_ENCODING,
      EXCEPTION_FLAG_WARNING,
      VmPtr
      );
    VmPtr->Ip += 2;
    return EFI_UNSUPPORTED;
    break;
  }

  VmPtr->Ip += 2;
  return EFI_SUCCESS;
}


/**
  Decode a 16-bit index to determine the offset. Given an index value:
  b15     - sign bit
  b14:12  - number of bits in this index assigned to natural units (=a)
  ba:11   - constant units = C
  b0:a    - natural units = N
  Given this info, the offset can be computed by:
  offset = sign_bit * (C + N * sizeof(UINTN))
  Max offset is achieved with index = 0x7FFF giving an offset of
  0x27B (32-bit machine) or 0x477 (64-bit machine).
  Min offset is achieved with index =

  @param  VmPtr             pointer to VM context
  @param  CodeOffset        offset from IP of the location of the 16-bit index to
                            decode

  @return The decoded offset.

**/
STATIC
INT16
VmReadIndex16 (
  IN VM_CONTEXT     *VmPtr,
  IN UINT32         CodeOffset
  )
{
  UINT16  Index;
  INT16   Offset;
  INT16   C;
  INT16   N;
  INT16   NBits;
  INT16   Mask;

  //
  // First read the index from the code stream
  //
  Index = VmReadCode16 (VmPtr, CodeOffset);

  //
  // Get the mask for N. First get the number of bits from the index.
  //
  NBits = (INT16) ((Index & 0x7000) >> 12);

  //
  // Scale it for 16-bit indexes
  //
  NBits *= 2;

  //
  // Now using the number of bits, create a mask.
  //
  Mask = (INT16) ((INT16)~0 << NBits);

  //
  // Now using the mask, extract N from the lower bits of the index.
  //
  N = (INT16) (Index &~Mask);

  //
  // Now compute C
  //
  C       = (INT16) (((Index &~0xF000) & Mask) >> NBits);

  Offset  = (INT16) (N * sizeof (UINTN) + C);

  //
  // Now set the sign
  //
  if (Index & 0x8000) {
    //
    // Do it the hard way to work around a bogus compiler warning
    //
    // Offset = -1 * Offset;
    //
    Offset = (INT16) ((INT32) Offset * -1);
  }

  return Offset;
}


/**
  Decode a 32-bit index to determine the offset.

  @param  VmPtr             pointer to VM context
  @param  CodeOffset        offset from IP of the location of the 32-bit index to
                            decode

  @return Converted index per EBC VM specification

**/
STATIC
INT32
VmReadIndex32 (
  IN VM_CONTEXT     *VmPtr,
  IN UINT32         CodeOffset
  )
{
  UINT32  Index;
  INT32   Offset;
  INT32   C;
  INT32   N;
  INT32   NBits;
  INT32   Mask;

  Index = VmReadImmed32 (VmPtr, CodeOffset);

  //
  // Get the mask for N. First get the number of bits from the index.
  //
  NBits = (Index & 0x70000000) >> 28;

  //
  // Scale it for 32-bit indexes
  //
  NBits *= 4;

  //
  // Now using the number of bits, create a mask.
  //
  Mask = (INT32)~0 << NBits;

  //
  // Now using the mask, extract N from the lower bits of the index.
  //
  N = Index &~Mask;

  //
  // Now compute C
  //
  C       = ((Index &~0xF0000000) & Mask) >> NBits;

  Offset  = N * sizeof (UINTN) + C;

  //
  // Now set the sign
  //
  if (Index & 0x80000000) {
    Offset = Offset * -1;
  }

  return Offset;
}


/**
  Decode a 64-bit index to determine the offset.

  @param  VmPtr             pointer to VM context
  @param  CodeOffset        offset from IP of the location of the 64-bit index to
                            decode

  @return Converted index per EBC VM specification

**/
STATIC
INT64
VmReadIndex64 (
  IN VM_CONTEXT     *VmPtr,
  IN UINT32         CodeOffset
  )
{
  UINT64  Index;
  INT64   Offset;
  INT64   C;
  INT64   N;
  INT64   NBits;
  INT64   Mask;

  Index = VmReadCode64 (VmPtr, CodeOffset);

  //
  // Get the mask for N. First get the number of bits from the index.
  //
  NBits = RShiftU64 ((Index & 0x7000000000000000ULL), 60);

  //
  // Scale it for 64-bit indexes (multiply by 8 by shifting left 3)
  //
  NBits = LShiftU64 ((UINT64)NBits, 3);

  //
  // Now using the number of bits, create a mask.
  //
  Mask = (LShiftU64 ((UINT64)~0, (UINTN)NBits));

  //
  // Now using the mask, extract N from the lower bits of the index.
  //
  N = Index &~Mask;

  //
  // Now compute C
  //
  C       = ARShiftU64 (((Index &~0xF000000000000000ULL) & Mask), (UINTN)NBits);

  Offset  = MultU64x64 (N, sizeof (UINTN)) + C;

  //
  // Now set the sign
  //
  if (Index & 0x8000000000000000ULL) {
    Offset = MultS64x64 (Offset, -1);
  }

  return Offset;
}


/**
  The following VmWriteMem? routines are called by the EBC data
  movement instructions that write to memory. Since these writes
  may be to the stack, which looks like (high address on top) this,
  [EBC entry point arguments]
  [VM stack]
  [EBC stack]
  we need to detect all attempts to write to the EBC entry point argument
  stack area and adjust the address (which will initially point into the
  VM stack) to point into the EBC entry point arguments.

  @param  VmPtr             pointer to a VM context
  @param  Addr              adddress to write to
  @param  Data              value to write to Addr

  @return Standard EFI_STATUS

**/
STATIC
EFI_STATUS
VmWriteMem8 (
  IN VM_CONTEXT    *VmPtr,
  IN UINTN         Addr,
  IN UINT8         Data
  )
{
  //
  // Convert the address if it's in the stack gap
  //
  Addr            = ConvertStackAddr (VmPtr, Addr);
  *(UINT8 *) Addr = Data;
  return EFI_SUCCESS;
}

STATIC
EFI_STATUS
VmWriteMem16 (
  IN VM_CONTEXT   *VmPtr,
  IN UINTN        Addr,
  IN UINT16       Data
  )
{
  EFI_STATUS  Status;

  //
  // Convert the address if it's in the stack gap
  //
  Addr = ConvertStackAddr (VmPtr, Addr);

  //
  // Do a simple write if aligned
  //
  if (IS_ALIGNED (Addr, sizeof (UINT16))) {
    *(UINT16 *) Addr = Data;
  } else {
    //
    // Write as two bytes
    //
    MemoryFence ();
    if ((Status = VmWriteMem8 (VmPtr, Addr, (UINT8) Data)) != EFI_SUCCESS) {
      return Status;
    }

    MemoryFence ();
    if ((Status = VmWriteMem8 (VmPtr, Addr + 1, (UINT8) (Data >> 8))) != EFI_SUCCESS) {
      return Status;
    }

    MemoryFence ();
  }

  return EFI_SUCCESS;
}

STATIC
EFI_STATUS
VmWriteMem32 (
  IN VM_CONTEXT   *VmPtr,
  IN UINTN        Addr,
  IN UINT32       Data
  )
{
  EFI_STATUS  Status;

  //
  // Convert the address if it's in the stack gap
  //
  Addr = ConvertStackAddr (VmPtr, Addr);

  //
  // Do a simple write if aligned
  //
  if (IS_ALIGNED (Addr, sizeof (UINT32))) {
    *(UINT32 *) Addr = Data;
  } else {
    //
    // Write as two words
    //
    MemoryFence ();
    if ((Status = VmWriteMem16 (VmPtr, Addr, (UINT16) Data)) != EFI_SUCCESS) {
      return Status;
    }

    MemoryFence ();
    if ((Status = VmWriteMem16 (VmPtr, Addr + sizeof (UINT16), (UINT16) (Data >> 16))) != EFI_SUCCESS) {
      return Status;
    }

    MemoryFence ();
  }

  return EFI_SUCCESS;
}

EFI_STATUS
VmWriteMem64 (
  IN VM_CONTEXT   *VmPtr,
  IN UINTN        Addr,
  IN UINT64       Data
  )
{
  EFI_STATUS  Status;
  UINT32      Data32;

  //
  // Convert the address if it's in the stack gap
  //
  Addr = ConvertStackAddr (VmPtr, Addr);

  //
  // Do a simple write if aligned
  //
  if (IS_ALIGNED (Addr, sizeof (UINT64))) {
    *(UINT64 *) Addr = Data;
  } else {
    //
    // Write as two 32-bit words
    //
    MemoryFence ();
    if ((Status = VmWriteMem32 (VmPtr, Addr, (UINT32) Data)) != EFI_SUCCESS) {
      return Status;
    }

    MemoryFence ();
    Data32 = (UINT32) (((UINT32 *) &Data)[1]);
    if ((Status = VmWriteMem32 (VmPtr, Addr + sizeof (UINT32), Data32)) != EFI_SUCCESS) {
      return Status;
    }

    MemoryFence ();
  }

  return EFI_SUCCESS;
}

EFI_STATUS
VmWriteMemN (
  IN VM_CONTEXT   *VmPtr,
  IN UINTN        Addr,
  IN UINTN        Data
  )
{
  EFI_STATUS  Status;
  UINTN       Index;

  Status = EFI_SUCCESS;

  //
  // Convert the address if it's in the stack gap
  //
  Addr = ConvertStackAddr (VmPtr, Addr);

  //
  // Do a simple write if aligned
  //
  if (IS_ALIGNED (Addr, sizeof (UINTN))) {
    *(UINTN *) Addr = Data;
  } else {
    for (Index = 0; Index < sizeof (UINTN) / sizeof (UINT32); Index++) {
      MemoryFence ();
      Status = VmWriteMem32 (VmPtr, Addr + Index * sizeof (UINT32), (UINT32) Data);
      MemoryFence ();
      Data = (UINTN)RShiftU64 ((UINT64)Data, 32);
    }
  }

  return Status;
}


/**
  The following VmReadImmed routines are called by the EBC execute
  functions to read EBC immediate values from the code stream.
  Since we can't assume alignment, each tries to read in the biggest
  chunks size available, but will revert to smaller reads if necessary.

  @param  VmPtr             pointer to a VM context
  @param  Offset            offset from IP of the code bytes to read.

  @return Signed data of the requested size from the specified address.

**/
STATIC
INT8
VmReadImmed8 (
  IN VM_CONTEXT *VmPtr,
  IN UINT32     Offset
  )
{
  //
  // Simply return the data in flat memory space
  //
  return * (INT8 *) (VmPtr->Ip + Offset);
}

STATIC
INT16
VmReadImmed16 (
  IN VM_CONTEXT *VmPtr,
  IN UINT32     Offset
  )
{
  //
  // Read direct if aligned
  //
  if (IS_ALIGNED ((UINTN) VmPtr->Ip + Offset, sizeof (INT16))) {
    return * (INT16 *) (VmPtr->Ip + Offset);
  } else {
    //
    // All code word reads should be aligned
    //
    EbcDebugSignalException (
      EXCEPT_EBC_ALIGNMENT_CHECK,
      EXCEPTION_FLAG_WARNING,
      VmPtr
      );
  }
  //
  // Return unaligned data
  //
  return (INT16) (*(UINT8 *) (VmPtr->Ip + Offset) + (*(UINT8 *) (VmPtr->Ip + Offset + 1) << 8));
}

STATIC
INT32
VmReadImmed32 (
  IN VM_CONTEXT *VmPtr,
  IN UINT32     Offset
  )
{
  UINT32  Data;

  //
  // Read direct if aligned
  //
  if (IS_ALIGNED ((UINTN) VmPtr->Ip + Offset, sizeof (UINT32))) {
    return * (INT32 *) (VmPtr->Ip + Offset);
  }
  //
  // Return unaligned data
  //
  Data = (UINT32) VmReadCode16 (VmPtr, Offset);
  Data |= (UINT32) (VmReadCode16 (VmPtr, Offset + 2) << 16);
  return Data;
}

STATIC
INT64
VmReadImmed64 (
  IN VM_CONTEXT *VmPtr,
  IN UINT32     Offset
  )
{
  UINT64  Data64;
  UINT32  Data32;
  UINT8   *Ptr;

  //
  // Read direct if aligned
  //
  if (IS_ALIGNED ((UINTN) VmPtr->Ip + Offset, sizeof (UINT64))) {
    return * (UINT64 *) (VmPtr->Ip + Offset);
  }
  //
  // Return unaligned data.
  //
  Ptr             = (UINT8 *) &Data64;
  Data32          = VmReadCode32 (VmPtr, Offset);
  *(UINT32 *) Ptr = Data32;
  Ptr += sizeof (Data32);
  Data32          = VmReadCode32 (VmPtr, Offset + sizeof (UINT32));
  *(UINT32 *) Ptr = Data32;
  return Data64;
}


/**
  The following VmReadCode() routines provide the ability to read raw
  unsigned data from the code stream.

  @param  VmPtr             pointer to VM context
  @param  Offset            offset from current IP to the raw data to read.

  @return The raw unsigned 16-bit value from the code stream.

**/
STATIC
UINT16
VmReadCode16 (
  IN VM_CONTEXT *VmPtr,
  IN UINT32     Offset
  )
{
  //
  // Read direct if aligned
  //
  if (IS_ALIGNED ((UINTN) VmPtr->Ip + Offset, sizeof (UINT16))) {
    return * (UINT16 *) (VmPtr->Ip + Offset);
  } else {
    //
    // All code word reads should be aligned
    //
    EbcDebugSignalException (
      EXCEPT_EBC_ALIGNMENT_CHECK,
      EXCEPTION_FLAG_WARNING,
      VmPtr
      );
  }
  //
  // Return unaligned data
  //
  return (UINT16) (*(UINT8 *) (VmPtr->Ip + Offset) + (*(UINT8 *) (VmPtr->Ip + Offset + 1) << 8));
}

STATIC
UINT32
VmReadCode32 (
  IN VM_CONTEXT *VmPtr,
  IN UINT32     Offset
  )
{
  UINT32  Data;
  //
  // Read direct if aligned
  //
  if (IS_ALIGNED ((UINTN) VmPtr->Ip + Offset, sizeof (UINT32))) {
    return * (UINT32 *) (VmPtr->Ip + Offset);
  }
  //
  // Return unaligned data
  //
  Data = (UINT32) VmReadCode16 (VmPtr, Offset);
  Data |= (VmReadCode16 (VmPtr, Offset + 2) << 16);
  return Data;
}

STATIC
UINT64
VmReadCode64 (
  IN VM_CONTEXT *VmPtr,
  IN UINT32     Offset
  )
{
  UINT64  Data64;
  UINT32  Data32;
  UINT8   *Ptr;

  //
  // Read direct if aligned
  //
  if (IS_ALIGNED ((UINTN) VmPtr->Ip + Offset, sizeof (UINT64))) {
    return * (UINT64 *) (VmPtr->Ip + Offset);
  }
  //
  // Return unaligned data.
  //
  Ptr             = (UINT8 *) &Data64;
  Data32          = VmReadCode32 (VmPtr, Offset);
  *(UINT32 *) Ptr = Data32;
  Ptr += sizeof (Data32);
  Data32          = VmReadCode32 (VmPtr, Offset + sizeof (UINT32));
  *(UINT32 *) Ptr = Data32;
  return Data64;
}

STATIC
UINT8
VmReadMem8 (
  IN VM_CONTEXT   *VmPtr,
  IN UINTN        Addr
  )
{
  //
  // Convert the address if it's in the stack gap
  //
  Addr = ConvertStackAddr (VmPtr, Addr);
  //
  // Simply return the data in flat memory space
  //
  return * (UINT8 *) Addr;
}

STATIC
UINT16
VmReadMem16 (
  IN VM_CONTEXT *VmPtr,
  IN UINTN      Addr
  )
{
  //
  // Convert the address if it's in the stack gap
  //
  Addr = ConvertStackAddr (VmPtr, Addr);
  //
  // Read direct if aligned
  //
  if (IS_ALIGNED (Addr, sizeof (UINT16))) {
    return * (UINT16 *) Addr;
  }
  //
  // Return unaligned data
  //
  return (UINT16) (*(UINT8 *) Addr + (*(UINT8 *) (Addr + 1) << 8));
}

STATIC
UINT32
VmReadMem32 (
  IN VM_CONTEXT *VmPtr,
  IN UINTN      Addr
  )
{
  UINT32  Data;

  //
  // Convert the address if it's in the stack gap
  //
  Addr = ConvertStackAddr (VmPtr, Addr);
  //
  // Read direct if aligned
  //
  if (IS_ALIGNED (Addr, sizeof (UINT32))) {
    return * (UINT32 *) Addr;
  }
  //
  // Return unaligned data
  //
  Data = (UINT32) VmReadMem16 (VmPtr, Addr);
  Data |= (VmReadMem16 (VmPtr, Addr + 2) << 16);
  return Data;
}

STATIC
UINT64
VmReadMem64 (
  IN VM_CONTEXT   *VmPtr,
  IN UINTN        Addr
  )
{
  UINT64  Data;
  UINT32  Data32;

  //
  // Convert the address if it's in the stack gap
  //
  Addr = ConvertStackAddr (VmPtr, Addr);

  //
  // Read direct if aligned
  //
  if (IS_ALIGNED (Addr, sizeof (UINT64))) {
    return * (UINT64 *) Addr;
  }
  //
  // Return unaligned data. Assume little endian.
  //
  Data    = (UINT64) VmReadMem32 (VmPtr, Addr);
  Data32  = VmReadMem32 (VmPtr, Addr + sizeof (UINT32));
  *(UINT32 *) ((UINT32 *) &Data + 1) = Data32;
  return Data;
}


/**
  Given an address that EBC is going to read from or write to, return
  an appropriate address that accounts for a gap in the stack.
  The stack for this application looks like this (high addr on top)
  [EBC entry point arguments]
  [VM stack]
  [EBC stack]
  The EBC assumes that its arguments are at the top of its stack, which
  is where the VM stack is really. Therefore if the EBC does memory
  accesses into the VM stack area, then we need to convert the address
  to point to the EBC entry point arguments area. Do this here.

  @param  VmPtr             pointer to VM context
  @param  Addr              address of interest

  @return The unchanged address if it's not in the VM stack region. Otherwise,
  @return adjust for the stack gap and return the modified address.

**/
STATIC
UINTN
ConvertStackAddr (
  IN VM_CONTEXT    *VmPtr,
  IN UINTN         Addr
  )
{
  ASSERT(((Addr < VmPtr->LowStackTop) || (Addr > VmPtr->HighStackBottom)));
  return Addr;
}


/**
  Read a natural value from memory. May or may not be aligned.

  @param  VmPtr             current VM context
  @param  Addr              the address to read from

  @return The natural value at address Addr.

**/
STATIC
UINTN
VmReadMemN (
  IN VM_CONTEXT    *VmPtr,
  IN UINTN         Addr
  )
{
  UINTN   Data;
  volatile UINT32  Size;
  UINT8   *FromPtr;
  UINT8   *ToPtr;
  //
  // Convert the address if it's in the stack gap
  //
  Addr = ConvertStackAddr (VmPtr, Addr);
  //
  // Read direct if aligned
  //
  if (IS_ALIGNED (Addr, sizeof (UINTN))) {
    return * (UINTN *) Addr;
  }
  //
  // Return unaligned data
  //
  Data    = 0;
  FromPtr = (UINT8 *) Addr;
  ToPtr   = (UINT8 *) &Data;

  for (Size = 0; Size < sizeof (Data); Size++) {
    *ToPtr = *FromPtr;
    ToPtr++;
    FromPtr++;
  }

  return Data;
}

UINT64
GetVmVersion (
  VOID
  )
{
  return (UINT64) (((VM_MAJOR_VERSION & 0xFFFF) << 16) | ((VM_MINOR_VERSION & 0xFFFF)));
}
