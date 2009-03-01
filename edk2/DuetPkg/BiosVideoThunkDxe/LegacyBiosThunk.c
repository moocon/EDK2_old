
#include "BiosVideo.h"

#define EFI_CPU_EFLAGS_IF 0x200

THUNK_CONTEXT               mThunkContext;

VOID
InitializeBiosIntCaller (
  IN  BIOS_VIDEO_DEV                 *BiosDev
  )
{
  EFI_STATUS            Status;
  UINT32                RealModeBufferSize;
  UINT32                ExtraStackSize;
  EFI_PHYSICAL_ADDRESS  LegacyRegionBase;
  
  //
  // Get LegacyRegion
  //
  AsmGetThunk16Properties (&RealModeBufferSize, &ExtraStackSize);

  LegacyRegionBase = 0x100000;
  Status = gBS->AllocatePages (
                  AllocateMaxAddress,
                  EfiACPIMemoryNVS,
                  EFI_SIZE_TO_PAGES(RealModeBufferSize + ExtraStackSize + 200),
                  &LegacyRegionBase
                  );
  ASSERT_EFI_ERROR (Status);
  
  mThunkContext.RealModeBuffer     = (VOID*)(UINTN)LegacyRegionBase;
  mThunkContext.RealModeBufferSize = EFI_PAGES_TO_SIZE (RealModeBufferSize);
  mThunkContext.ThunkAttributes    = 3;
  AsmPrepareThunk16(&mThunkContext);
  
}

VOID
InitializeInterruptRedirection (
  IN  BIOS_VIDEO_DEV                 *BiosDev
  )
/*++

  Routine Description:
    Initialize interrupt redirection code and entries, because
    IDT Vectors 0x68-0x6f must be redirected to IDT Vectors 0x08-0x0f.
    Or the interrupt will lost when we do thunk.
    NOTE: We do not reset 8259 vector base, because it will cause pending
    interrupt lost.

  Arguments:
    NONE

  Returns:
    NONE
--*/
{
  EFI_STATUS            Status;
  EFI_PHYSICAL_ADDRESS  LegacyRegionBase;
  UINTN                 LegacyRegionLength;
  UINT32                *IdtArray;
  UINTN                 Index;
  UINT8                 ProtectedModeBaseVector;
  UINT32                InterruptRedirectionCode[] = {
    0x90CF08CD, // INT8; IRET; NOP
    0x90CF09CD, // INT9; IRET; NOP
    0x90CF0ACD, // INTA; IRET; NOP
    0x90CF0BCD, // INTB; IRET; NOP
    0x90CF0CCD, // INTC; IRET; NOP
    0x90CF0DCD, // INTD; IRET; NOP
    0x90CF0ECD, // INTE; IRET; NOP
    0x90CF0FCD  // INTF; IRET; NOP
  };

  //
  // Get LegacyRegion
  //
  LegacyRegionLength = sizeof(InterruptRedirectionCode);
  LegacyRegionBase = 0x100000;
  Status = gBS->AllocatePages (
                  AllocateMaxAddress,
                  EfiACPIMemoryNVS,
                  EFI_SIZE_TO_PAGES(LegacyRegionLength),
                  &LegacyRegionBase
                  );
  ASSERT_EFI_ERROR (Status);

  //
  // Copy code to legacy region
  //
  CopyMem ((VOID *)(UINTN)LegacyRegionBase, InterruptRedirectionCode, sizeof (InterruptRedirectionCode));

  //
  // Get VectorBase, it should be 0x68
  //
  Status = BiosDev->Legacy8259->GetVector (BiosDev->Legacy8259, Efi8259Irq0, &ProtectedModeBaseVector);
  ASSERT_EFI_ERROR (Status);

  //
  // Patch IVT 0x68 ~ 0x6f
  //
  IdtArray = (UINT32 *) 0;
  for (Index = 0; Index < 8; Index++) {
    IdtArray[ProtectedModeBaseVector + Index] = ((EFI_SEGMENT (LegacyRegionBase + Index * 4)) << 16) | (EFI_OFFSET (LegacyRegionBase + Index * 4));
  }

  return ;
}

BOOLEAN
EFIAPI
LegacyBiosInt86 (
  IN  BIOS_VIDEO_DEV                 *BiosDev,
  IN  UINT8                           BiosInt,
  IN  EFI_IA32_REGISTER_SET           *Regs
  )
/*++

  Routine Description:
    Thunk to 16-bit real mode and execute a software interrupt with a vector 
    of BiosInt. Regs will contain the 16-bit register context on entry and 
    exit.

  Arguments:
    This    - Protocol instance pointer.
    BiosInt - Processor interrupt vector to invoke
    Reg     - Register contexted passed into (and returned) from thunk to 
              16-bit mode

  Returns:
    FALSE   - Thunk completed, and there were no BIOS errors in the target code.
              See Regs for status.
    TRUE    - There was a BIOS erro in the target code.

--*/
{
  UINTN                 Status;
  UINTN                 Eflags;
  IA32_REGISTER_SET     ThunkRegSet;
  BOOLEAN               Ret;
  UINT16                *Stack16;
  
  Regs->X.Flags.Reserved1 = 1;
  Regs->X.Flags.Reserved2 = 0;
  Regs->X.Flags.Reserved3 = 0;
  Regs->X.Flags.Reserved4 = 0;
  Regs->X.Flags.IOPL      = 3;
  Regs->X.Flags.NT        = 0;
  Regs->X.Flags.IF        = 1;
  Regs->X.Flags.TF        = 0;
  Regs->X.Flags.CF        = 0;

  ZeroMem (&ThunkRegSet, sizeof (ThunkRegSet));
  ThunkRegSet.E.EDI  = Regs->E.EDI;
  ThunkRegSet.E.ESI  = Regs->E.ESI;
  ThunkRegSet.E.EBP  = Regs->E.EBP;
  ThunkRegSet.E.EBX  = Regs->E.EBX;
  ThunkRegSet.E.EDX  = Regs->E.EDX;
  ThunkRegSet.E.ECX  = Regs->E.ECX;
  ThunkRegSet.E.EAX  = Regs->E.EAX;
  ThunkRegSet.E.DS   = Regs->E.DS;
  ThunkRegSet.E.ES   = Regs->E.ES;

  CopyMem (&(ThunkRegSet.E.EFLAGS), &(Regs->E.EFlags), sizeof (UINT32));
 
  //
  // The call to Legacy16 is a critical section to EFI
  //
  Eflags = AsmReadEflags ();
  if ((Eflags | EFI_CPU_EFLAGS_IF) != 0) {
    DisableInterrupts ();
  }

  //
  // Set Legacy16 state. 0x08, 0x70 is legacy 8259 vector bases.
  //
  Status = BiosDev->Legacy8259->SetMode (BiosDev->Legacy8259, Efi8259LegacyMode, NULL, NULL);
  ASSERT_EFI_ERROR (Status);
  
  Stack16 = (UINT16 *)((UINT8 *) mThunkContext.RealModeBuffer + mThunkContext.RealModeBufferSize - sizeof (UINT16));
  Stack16 -= sizeof (ThunkRegSet.E.EFLAGS) / sizeof (UINT16);
  CopyMem (Stack16, &ThunkRegSet.E.EFLAGS, sizeof (ThunkRegSet.E.EFLAGS));

  ThunkRegSet.E.SS   = (UINT16) (((UINTN) Stack16 >> 16) << 12);
  ThunkRegSet.E.ESP  = (UINT16) (UINTN) Stack16;
  ThunkRegSet.E.Eip  = (UINT16)((UINT32 *)NULL)[BiosInt];
  ThunkRegSet.E.CS   = (UINT16)(((UINT32 *)NULL)[BiosInt] >> 16);
  mThunkContext.RealModeState = &ThunkRegSet;
  AsmThunk16 (&mThunkContext);

  //
  // Restore protected mode interrupt state
  //
  Status = BiosDev->Legacy8259->SetMode (BiosDev->Legacy8259, Efi8259ProtectedMode, NULL, NULL);
  ASSERT_EFI_ERROR (Status);

  //
  // End critical section
  //
  if ((Eflags | EFI_CPU_EFLAGS_IF) != 0) {
    EnableInterrupts ();
  }

  Regs->E.EDI      = ThunkRegSet.E.EDI;      
  Regs->E.ESI      = ThunkRegSet.E.ESI;  
  Regs->E.EBP      = ThunkRegSet.E.EBP;  
  Regs->E.EBX      = ThunkRegSet.E.EBX;  
  Regs->E.EDX      = ThunkRegSet.E.EDX;  
  Regs->E.ECX      = ThunkRegSet.E.ECX;  
  Regs->E.EAX      = ThunkRegSet.E.EAX;
  Regs->E.SS       = ThunkRegSet.E.SS;
  Regs->E.CS       = ThunkRegSet.E.CS;  
  Regs->E.DS       = ThunkRegSet.E.DS;  
  Regs->E.ES       = ThunkRegSet.E.ES;

  CopyMem (&(Regs->E.EFlags), &(ThunkRegSet.E.EFLAGS), sizeof (UINT32));

  Ret = (BOOLEAN) (Regs->E.EFlags.CF == 1);

  return Ret;
}


