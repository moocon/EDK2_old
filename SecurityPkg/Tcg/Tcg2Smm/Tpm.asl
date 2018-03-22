/** @file
  The TPM2 definition block in ACPI table for TCG2 physical presence  
  and MemoryClear.

Copyright (c) 2015 - 2017, Intel Corporation. All rights reserved.<BR>
This program and the accompanying materials 
are licensed and made available under the terms and conditions of the BSD License 
which accompanies this distribution.  The full text of the license may be found at 
http://opensource.org/licenses/bsd-license.php

THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS, 
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

DefinitionBlock (
  "Tpm.aml",
  "SSDT",
  2,
  "INTEL ",
  "Tpm2Tabl",
  0x1000
  )
{
  Scope (\_SB)
  {
    Device (TPM)
    {
      //
      // TCG2
      //

      //
      //  TAG for patching TPM2.0 _HID
      //
      Name (_HID, "NNNN0000")

      Name (_CID, "MSFT0101")

      //
      // Readable name of this device, don't know if this way is correct yet
      //
      Name (_STR, Unicode ("TPM 2.0 Device"))

      //


      // Operational region for Smi port access
      //
      OperationRegion (SMIP, SystemIO, 0xB2, 1)
      Field (SMIP, ByteAcc, NoLock, Preserve)
      { 
          IOB2, 8
      }

      //
      // Operational region for TPM access
      //
      OperationRegion (TPMR, SystemMemory, 0xfed40000, 0x5000)
      Field (TPMR, AnyAcc, NoLock, Preserve)
      {
        ACC0, 8,  // TPM_ACCESS_0
        Offset(0x8),
        INTE, 32, // TPM_INT_ENABLE_0
        INTV, 8,  // TPM_INT_VECTOR_0
        Offset(0x10),
        INTS, 32, // TPM_INT_STATUS_0
        INTF, 32, // TPM_INTF_CAPABILITY_0
        STS0, 32, // TPM_STS_0
        Offset(0x24),
        FIFO, 32, // TPM_DATA_FIFO_0
        Offset(0x30),
        TID0, 32, // TPM_INTERFACE_ID_0
                  // ignore the rest
      }

      //
      // Operational region for TPM support, TPM Physical Presence and TPM Memory Clear
      // Region Offset 0xFFFF0000 and Length 0xF0 will be fixed in C code.
      //
      OperationRegion (TNVS, SystemMemory, 0xFFFF0000, 0xF0)
      Field (TNVS, AnyAcc, NoLock, Preserve)
      {
        PPIN,   8,  //   Software SMI for Physical Presence Interface
        PPIP,   32, //   Used for save physical presence paramter
        PPRP,   32, //   Physical Presence request operation response
        PPRQ,   32, //   Physical Presence request operation
        PPRM,   32, //   Physical Presence request operation parameter
        LPPR,   32, //   Last Physical Presence request operation
        FRET,   32, //   Physical Presence function return code
        MCIN,   8,  //   Software SMI for Memory Clear Interface
        MCIP,   32, //   Used for save the Mor paramter
        MORD,   32, //   Memory Overwrite Request Data
        MRET,   32  //   Memory Overwrite function return code
      }

      Name(RESO, ResourceTemplate () {
        Memory32Fixed (ReadWrite, 0xfed40000, 0x5000, REGS)
        Interrupt(ResourceConsumer, Level, ActiveLow, Shared, , , IRQ) {12}
      })

      //
      // Return the resource consumed by TPM device.
      //
      Method(_CRS,0,Serialized)
      {
        Return(RESO)
      }

      //
      // Set resources consumed by the TPM device. This is used to
      // assign an interrupt number to the device. The input byte stream
      // has to be the same as returned by _CRS (according to ACPI spec).
      //
      Method(_SRS,1,Serialized)
      {
        //
        // Update resource descriptor
        // Use the field name to identify the offsets in the argument
        // buffer and RESO buffer.
        //
        CreateDWordField(Arg0, ^IRQ._INT, IRQ0)
        CreateDWordField(RESO, ^IRQ._INT, LIRQ)
        Store(IRQ0, LIRQ)

        CreateBitField(Arg0, ^IRQ._HE, ITRG)
        CreateBitField(RESO, ^IRQ._HE, LTRG)
        Store(ITRG, LTRG)

        CreateBitField(Arg0, ^IRQ._LL, ILVL)
        CreateBitField(RESO, ^IRQ._LL, LLVL)
        Store(ILVL, LLVL)

        //
        // Update TPM FIFO PTP/TIS interface only, identified by TPM_INTERFACE_ID_x lowest
        // nibble.
        // 0000 - FIFO interface as defined in PTP for TPM 2.0 is active
        // 1111 - FIFO interface as defined in TIS1.3 is active
        //
        If (LOr(LEqual (And (TID0, 0x0F), 0x00), LEqual (And (TID0, 0x0F), 0x0F))) {
          //
          // If FIFO interface, interrupt vector register is
          // available. TCG PTP specification allows only
          // values 1..15 in this field. For other interrupts
          // the field should stay 0.
          //
          If (LLess (IRQ0, 16)) {
            Store (And(IRQ0, 0xF), INTV)
          }
          //
          // Interrupt enable register (TPM_INT_ENABLE_x) bits 3:4
          // contains settings for interrupt polarity.
          // The other bits of the byte enable individual interrupts.
          // They should be all be zero, but to avoid changing the
          // configuration, the other bits are be preserved.
          // 00 - high level
          // 01 - low level
          // 10 - rising edge
          // 11 - falling edge
          //
          // ACPI spec definitions:
          // _HE: '1' is Edge, '0' is Level
          // _LL: '1' is ActiveHigh, '0' is ActiveLow (inverted from TCG spec)
          //
          If (LEqual (ITRG, 1)) {
            Or(INTE, 0x00000010, INTE)
          } Else {
            And(INTE, 0xFFFFFFEF, INTE)
          }
          if (LEqual (ILVL, 0)) {
            Or(INTE, 0x00000008, INTE)
          } Else {
            And(INTE, 0xFFFFFFF7, INTE)
          }
        }
      }

      //
      // Possible resource settings.
      // The format of the data has to follow the same format as
      // _CRS (according to ACPI spec).
      //
      Name (_PRS, ResourceTemplate() {
        Memory32Fixed (ReadWrite, 0xfed40000, 0x5000)
        Interrupt(ResourceConsumer, Level, ActiveLow, Shared, , , SIRQ) {1,2,3,4,5,6,7,8,9,10,11,12,13,14,15}
      })

      Method (PTS, 1, Serialized)
      {  
        //
        // Detect Sx state for MOR, only S4, S5 need to handle
        //
        If (LAnd (LLess (Arg0, 6), LGreater (Arg0, 3)))
        {   
          //
          // Bit4 -- DisableAutoDetect. 0 -- Firmware MAY autodetect.
          //
          If (LNot (And (MORD, 0x10)))
          {
            //
            // Triggle the SMI through ACPI _PTS method.
            //
            Store (0x02, MCIP)
              
            //
            // Triggle the SMI interrupt
            //
            Store (MCIN, IOB2)
          }
        }
        Return (0)
      }   

      Method (_STA, 0)
      {
        if (LEqual (ACC0, 0xff))
        {
            Return (0)
        }
        Return (0x0f)
      }

      //
      // TCG Hardware Information
      //
      Method (HINF, 3, Serialized, 0, {BuffObj, PkgObj}, {UnknownObj, UnknownObj, UnknownObj}) // IntObj, IntObj, PkgObj
      {
        //
        // Switch by function index
        //
        Switch (ToInteger(Arg1))
        {
          Case (0)
          {
            //
            // Standard query
            //
            Return (Buffer () {0x03})
          }
          Case (1)
          {
            //
            // Return failure if no TPM present
            //
            Name(TPMV, Package () {0x01, Package () {0x2, 0x0}})
            if (LEqual (_STA (), 0x00))
            {
              Return (Package () {0x00})
            }

            //
            // Return TPM version
            //
            Return (TPMV)
          }
          Default {BreakPoint}
        }
        Return (Buffer () {0})
      }

      Name(TPM2, Package (0x02){
        Zero, 
        Zero
      })

      Name(TPM3, Package (0x03){
        Zero, 
        Zero,
        Zero
      })

      //
      // TCG Physical Presence Interface
      //
      Method (TPPI, 3, Serialized, 0, {BuffObj, PkgObj, IntObj, StrObj}, {UnknownObj, UnknownObj, UnknownObj}) // IntObj, IntObj, PkgObj
      {        
        //
        // Switch by function index
        //
        Switch (ToInteger(Arg1))
        {
          Case (0)
          {
            //
            // Standard query, supports function 1-8
            //
            Return (Buffer () {0xFF, 0x01})
          }
          Case (1)
          {
            //
            // a) Get Physical Presence Interface Version
            //
            Return ("$PV")
          }
          Case (2)
          {
            //
            // b) Submit TPM Operation Request to Pre-OS Environment
            //
                  
            Store (DerefOf (Index (Arg2, 0x00)), PPRQ)
            Store (0x02, PPIP)
              
            //
            // Triggle the SMI interrupt
            //
            Store (PPIN, IOB2)
            Return (FRET)


          }
          Case (3)
          {
            //
            // c) Get Pending TPM Operation Requested By the OS
            //
                  
            Store (PPRQ, Index (TPM2, 0x01))
            Return (TPM2)
          }
          Case (4)
          {
            //
            // d) Get Platform-Specific Action to Transition to Pre-OS Environment
            //
            Return (2)
          }
          Case (5)
          {
            //
            // e) Return TPM Operation Response to OS Environment
            //
            Store (0x05, PPIP)
                  
            //
            // Triggle the SMI interrupt
            //
            Store (PPIN, IOB2)
                  
            Store (LPPR, Index (TPM3, 0x01))
            Store (PPRP, Index (TPM3, 0x02))

            Return (TPM3)
          }
          Case (6)
          {

            //
            // f) Submit preferred user language (Not implemented)
            //

            Return (3)

          }
          Case (7)
          {
            //
            // g) Submit TPM Operation Request to Pre-OS Environment 2
            //
            Store (7, PPIP)
            Store (DerefOf (Index (Arg2, 0x00)), PPRQ)
            Store (0, PPRM)
            If (LEqual (PPRQ, 23)) {
              Store (DerefOf (Index (Arg2, 0x01)), PPRM)
            }
                
            //
            // Triggle the SMI interrupt 
            //
            Store (PPIN, IOB2)  
            Return (FRET)
          }
          Case (8)
          {
            //
            // e) Get User Confirmation Status for Operation
            //
            Store (8, PPIP)
            Store (DerefOf (Index (Arg2, 0x00)), PPRQ)
                  
            //
            // Triggle the SMI interrupt
            //
            Store (PPIN, IOB2)
                  
            Return (FRET)
          }

          Default {BreakPoint}
        }
        Return (1)
      }

      Method (TMCI, 3, Serialized, 0, IntObj, {UnknownObj, UnknownObj, UnknownObj}) // IntObj, IntObj, PkgObj
      {
        //
        // Switch by function index
        //
        Switch (ToInteger (Arg1))
        {
          Case (0)
          {
            //
            // Standard query, supports function 1-1
            //
            Return (Buffer () {0x03})
          }
          Case (1)
          {
            //
            // Save the Operation Value of the Request to MORD (reserved memory)
            //
            Store (DerefOf (Index (Arg2, 0x00)), MORD)
                  
            //
            // Triggle the SMI through ACPI _DSM method.
            //
            Store (0x01, MCIP)
                  
            //
            // Triggle the SMI interrupt
            //
            Store (MCIN, IOB2)
            Return (MRET)
          }
          Default {BreakPoint}
        }
        Return (1)        
      }

      Method (_DSM, 4, Serialized, 0, UnknownObj, {BuffObj, IntObj, IntObj, PkgObj})
      {

        //
        // TCG Hardware Information
        //
        If(LEqual(Arg0, ToUUID ("cf8e16a5-c1e8-4e25-b712-4f54a96702c8")))
        {
          Return (HINF (Arg1, Arg2, Arg3))
        }

        //
        // TCG Physical Presence Interface
        //
        If(LEqual(Arg0, ToUUID ("3dddfaa6-361b-4eb4-a424-8d10089d1653")))
        {
          Return (TPPI (Arg1, Arg2, Arg3))
        }

        //
        // TCG Memory Clear Interface
        //
        If(LEqual(Arg0, ToUUID ("376054ed-cc13-4675-901c-4756d7f2d45d")))
        {
          Return (TMCI (Arg1, Arg2, Arg3))
        }

        Return (Buffer () {0})
      }
    }
  }
}
