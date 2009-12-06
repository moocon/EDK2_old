/** @file
  Basic commands and command processing infrastructure for EBL

  Copyright (c) 2007, Intel Corporation<BR>
  Portions copyright (c) 2008-2009, Apple Inc. All rights reserved.

  All rights reserved. This program and the accompanying materials
  are licensed and made available under the terms and conditions of the BSD License
  which accompanies this distribution.  The full text of the license may be found at
  http://opensource.org/licenses/bsd-license.php

  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

#include "Ebl.h"
#include <Protocol/DiskIo.h>
#include <Protocol/BlockIo.h>

UINTN             mCmdTableMaxIndex = EBL_MAX_COMMAND_COUNT;
UINTN             mCmdTableNextFreeIndex = 0;
EBL_COMMAND_TABLE *mCmdTable[EBL_MAX_COMMAND_COUNT];

/**
  Converts a lowercase Ascii character to upper one

  If Chr is lowercase Ascii character, then converts it to upper one.

  If Value >= 0xA0, then ASSERT().
  If (Value & 0x0F) >= 0x0A, then ASSERT().

  @param  chr   one Ascii character

  @return The uppercase value of Ascii character 

**/
STATIC
CHAR8
AsciiToUpper (
  IN      CHAR8                     Chr
  )
{
  return (UINT8) ((Chr >= 'a' && Chr <= 'z') ? Chr - ('a' - 'A') : Chr);
}


/**
  Case insensitve comparison of two Null-terminated Unicode strings with maximum
  lengths, and returns the difference between the first mismatched Unicode
  characters.
  This function compares the Null-terminated Unicode string FirstString to the
  Null-terminated Unicode string SecondString. At most, Length Unicode
  characters will be compared. If Length is 0, then 0 is returned. If
  FirstString is identical to SecondString, then 0 is returned. Otherwise, the
  value returned is the first mismatched Unicode character in SecondString
  subtracted from the first mismatched Unicode character in FirstString.
  
  @param  FirstString   Pointer to a Null-terminated ASCII string.  
  @param  SecondString  Pointer to a Null-terminated ASCII string.
  @param  Length        Max length to compare.
  
  @retval 0   FirstString is identical to SecondString using case insensitive
              comparisons.
  @retval !=0 FirstString is not identical to SecondString using case
              insensitive comparisons.

**/
INTN
EFIAPI
AsciiStrniCmp (
  IN      CONST CHAR8               *FirstString,
  IN      CONST CHAR8               *SecondString,
  IN      UINTN                     Length
  )
{
  if (Length == 0) {
    return 0;
  }

  while ((AsciiToUpper (*FirstString) != '\0') &&
         (AsciiToUpper (*FirstString) == AsciiToUpper (*SecondString)) &&
         (Length > 1)) {
    FirstString++;
    SecondString++;
    Length--;
  }
  
  return AsciiToUpper (*FirstString) - AsciiToUpper (*SecondString);
}



/**
  Add a command to the mCmdTable. If there is no free space in the command 
  table ASSERT. The mCmdTable is maintained in alphabetical order and the 
  new entry is inserted into its sorted possition.

  @param  Entry   Commnad Entry to add to the CmdTable

**/
VOID
EFIAPI
EblAddCommand (
  IN const EBL_COMMAND_TABLE   *Entry
  )
{
  UINTN               Count;

  if (mCmdTableNextFreeIndex == EBL_MAX_COMMAND_COUNT) {
    //
    // Ran out of space to store commands. Increase EBL_MAX_COMMAND_COUNT
    //
    ASSERT (FALSE);
    return;
  }

  //
  // Add command and Insertion sort array in the process
  //
  mCmdTable[mCmdTableNextFreeIndex] = (EBL_COMMAND_TABLE *)Entry;
  if (mCmdTableNextFreeIndex != 0) {
    for (Count = mCmdTableNextFreeIndex; Count > 0; Count--) {
      if (AsciiStriCmp (mCmdTable[Count - 1]->Name, Entry->Name) <= 0) {
        break;
      }
      
      mCmdTable[Count] = mCmdTable[Count - 1];
    }
    mCmdTable[Count] = (EBL_COMMAND_TABLE *)Entry;
  }

  mCmdTableNextFreeIndex++;
}


/**
  Add an set of commands to the command table. Most commonly used on static 
  array of commands.

  @param  EntryArray   Pointer to array of command entries
  @param  ArrayCount   Number of commnad entries to add

**/
VOID
EFIAPI
EblAddCommands (
  IN const EBL_COMMAND_TABLE   *EntryArray,
  IN UINTN                     ArrayCount
  )
{
  UINTN   Index;

  for (Index = 0; Index < ArrayCount; Index++) {
    EblAddCommand (&EntryArray[Index]);
  }
}


EBL_ADD_COMMAND_PROTOCOL gEblAddCommand = {
  EblAddCommand,
  EblAddCommands,
  EblGetCharKey,
  EblAnyKeyToContinueQtoQuit
};



/**
  Return the best matching command for the passed in command name. The match 
  does not have to be exact, it just needs to be unqiue. This enables commands
  to be shortend to the smallest set of starting characters that is unique.

  @param  CommandName   Name of command to search for

  @return NULL  CommandName did not match or was not unique
          Other Pointer to EBL_COMMAND_TABLE entry for CommandName

**/
EBL_COMMAND_TABLE *
EblGetCommand (
  IN CHAR8    *CommandName
  )
{
  UINTN               Index;
  UINTN               BestMatchCount;
  UINTN               Length;
  EBL_COMMAND_TABLE   *Match;

  Length = AsciiStrLen (CommandName);
  for (Index = 0, BestMatchCount = 0, Match = NULL; Index < mCmdTableNextFreeIndex; Index++) {
    if (AsciiStriCmp (mCmdTable[Index]->Name,  CommandName) == 0) {
      // match a command exactly
      return mCmdTable[Index];
    }

    if (AsciiStrniCmp (CommandName, mCmdTable[Index]->Name, Length) == 0)  {
      // partial match, so keep looking to make sure there is only one partial match
      BestMatchCount++;
      Match = mCmdTable[Index];
    }
  }

  if (BestMatchCount == 1) {
    return Match;
  }

  //
  // We had no matches or too many matches
  //
  return NULL;
}



/**
  List out help information on all the commands or print extended information 
  about a specific passed in command.

  Argv[0] - "help"
  Argv[1] - Command to display help about

  @param  Argc   Number of command arguments in Argv
  @param  Argv   Array of strings that represent the parsed command line. 
                 Argv[0] is the comamnd name

  @return EFI_SUCCESS

**/
EFI_STATUS
EblHelpCmd (
  IN UINTN  Argc,
  IN CHAR8  **Argv
  )
{
  UINTN   Index;
  CHAR8   *Ptr;
  UINTN   CurrentRow;

  if (Argc == 1) {
    // Print all the commands
    AsciiPrint ("Embedded Boot Loader (EBL) commands (help command for more info):\n");
    for (Index = 0; Index < mCmdTableNextFreeIndex; Index++) {
      EblSetTextColor (EFI_YELLOW);
      AsciiPrint (" %a", mCmdTable[Index]->Name);
      EblSetTextColor (0);
      AsciiPrint ("%a\n", mCmdTable[Index]->HelpSummary);
    }
  } else if (Argv[1] != NULL) {
    // Print specific help 
    for (Index = 0, CurrentRow = 0; Index < mCmdTableNextFreeIndex; Index++) {
      if (AsciiStriCmp (Argv[1], mCmdTable[Index]->Name) == 0) {
        Ptr = (mCmdTable[Index]->Help == NULL) ? mCmdTable[Index]->HelpSummary : mCmdTable[Index]->Help;
        AsciiPrint ("%a%a\n", Argv[1], Ptr);
        if (EblAnyKeyToContinueQtoQuit (&CurrentRow, FALSE)) {
          break;
        }
      }
    }
  }

  return EFI_SUCCESS;
}


/**
  Exit the EBL. If the commnad processor sees EFI_ABORTED return status it will
  exit the EBL.

  Argv[0] - "exit"

  @param  Argc   Number of command arguments in Argv
  @param  Argv   Array of strings that represent the parsed command line. 
                 Argv[0] is the comamnd name

  @return EFI_ABORTED

**/
EFI_STATUS
EblExitCmd (
  IN UINTN  Argc,
  IN CHAR8  **Argv
  )
{
  EFI_STATUS              Status;
  UINTN                   MemoryMapSize;
  EFI_MEMORY_DESCRIPTOR   *MemoryMap;
  UINTN                   MapKey;
  UINTN                   DescriptorSize;
  UINTN                   DescriptorVersion;
  UINTN                   Pages;

  if (Argc > 1) { 
    if (AsciiStriCmp (Argv[1], "efi") != 0) {
      return EFI_ABORTED;
    }
  } else if (Argc == 1) {
    return EFI_ABORTED;
  }
  
  MemoryMap = NULL;
  MemoryMapSize = 0;
  do {
    Status = gBS->GetMemoryMap (
                    &MemoryMapSize,
                    MemoryMap,
                    &MapKey,
                    &DescriptorSize,
                    &DescriptorVersion
                    );
    if (Status == EFI_BUFFER_TOO_SMALL) {

      Pages = EFI_SIZE_TO_PAGES (MemoryMapSize) + 1;
      MemoryMap = AllocatePages (Pages);
    
      //
      // Get System MemoryMap
      //
      Status = gBS->GetMemoryMap (
                      &MemoryMapSize,
                      MemoryMap,
                      &MapKey,
                      &DescriptorSize,
                      &DescriptorVersion
                      );
      // Don't do anything between the GetMemoryMap() and ExitBootServices()
      if (!EFI_ERROR (Status)) {
        Status = gBS->ExitBootServices (gImageHandle, MapKey);
        if (EFI_ERROR (Status)) {
          FreePages (MemoryMap, Pages);
          MemoryMap = NULL;
          MemoryMapSize = 0;
        }
      }
    }
  } while (EFI_ERROR (Status));

  //
  // At this point it is very dangerous to do things EFI as most of EFI is now gone.
  // This command is useful if you are working with a debugger as it will shutdown
  // DMA and other things that could break a soft resets.
  //  
  CpuDeadLoop ();
  
  // Should never get here, but makes the compiler happy
  return EFI_ABORTED;
}


/**
  Update the screen by decrementing the timeout value.
  This AsciiPrint has to match the AsciiPrint in 
  EblPauseCmd. 

  @param  ElaspedTime   Current timout value remaining

**/
VOID
EFIAPI
EblPauseCallback (
  IN  UINTN   ElapsedTime
  )
{
  AsciiPrint ("\b\b\b\b\b\b\b\b\b\b\b\b   \b\b%3d seconds", ElapsedTime);
}

/**
  Pause until a key is pressed and abort the remaining commands on the command
  line. If no key is pressed continue processing the command line. This command
  allows the user to stop an operation from happening and return control to the
  command prompt.

  Argv[0] - "pause"
  Argv[1] - timeout value is decimal seconds

  @param  Argc   Number of command arguments in Argv
  @param  Argv   Array of strings that represent the parsed command line. 
                 Argv[0] is the comamnd name

  @return EFI_SUCCESS  Timeout expired with no input
  @return EFI_TIMEOUT  Stop procesing other commands on the same command line

**/
EFI_STATUS
EblPauseCmd (
  IN UINTN  Argc,
  IN CHAR8  **Argv
  )
{
  EFI_STATUS      Status;
  UINTN           Delay;
  EFI_INPUT_KEY   Key;

  Delay = (Argc == 1)? 10 : AsciiStrDecimalToUintn (Argv[1]);

  AsciiPrint ("Hit any key to break. You have %3d seconds", Delay);
  Status = EblGetCharKey (&Key, Delay, EblPauseCallback);
  AsciiPrint ("\n");

  // If we timeout then the pause succeded thus return success
  // If we get a key return timout to stop other commnad on this cmd line
  return (Status == EFI_SUCCESS) ? EFI_TIMEOUT : EFI_SUCCESS;;
}


/**
  On a debug build issue a software breakpoint to enter the debugger

  Argv[0] - "break"

  @param  Argc   Number of command arguments in Argv
  @param  Argv   Array of strings that represent the parsed command line. 
                 Argv[0] is the comamnd name

  @return EFI_SUCCESS

**/
EFI_STATUS
EblBreakPointCmd (
  IN UINTN  Argc,
  IN CHAR8  **Argv
  )
{
  CpuBreakpoint ();
  return EFI_SUCCESS;
}


/**
  Reset the system. If no Argument do a Cold reset. If argument use that reset type
  (W)arm = Warm Reset
  (S)hutdown = Shutdown Reset

  Argv[0] - "reset"
  Argv[1] - warm or shutdown reset type

  @param  Argc   Number of command arguments in Argv
  @param  Argv   Array of strings that represent the parsed command line. 
                 Argv[0] is the comamnd name

  @return EFI_SUCCESS

**/
EFI_STATUS
EblResetCmd (
  IN UINTN  Argc,
  IN CHAR8  **Argv
  )
{
  EFI_RESET_TYPE    ResetType;

  ResetType = EfiResetCold;
  if (Argc > 1) {
    switch (*Argv[1]) {
    case 'W':
    case 'w':
      ResetType = EfiResetWarm;
      break;
    case 'S':
    case 's':
      ResetType = EfiResetShutdown;
    }
  } 

  gRT->ResetSystem (ResetType, EFI_SUCCESS, 0, NULL);
  return EFI_SUCCESS;
}


/**
  Toggle page break global. This turns on and off prompting to Quit or hit any
  key to continue when a command is about to scroll the screen with its output

  Argv[0] - "page"
  Argv[1] - on or off

  @param  Argc   Number of command arguments in Argv
  @param  Argv   Array of strings that represent the parsed command line. 
                 Argv[0] is the comamnd name

  @return EFI_SUCCESS

**/
EFI_STATUS
EblPageCmd (
  IN UINTN  Argc,
  IN CHAR8  **Argv
  )
{
  if (Argc <= 1) {
    // toggle setting   
    gPageBreak = (gPageBreak) ? FALSE : TRUE;
  } else {
    // use argv to set the value
    if ((Argv[1][0] == 'o') || (Argv[1][0] == 'O')) {
      if ((Argv[1][1] == 'n') || (Argv[1][1] == 'N')) {
        gPageBreak = TRUE;
      } else if ((Argv[1][1] == 'f') || (Argv[1][1] == 'F')) {
        gPageBreak = FALSE;
      } else {
        return EFI_INVALID_PARAMETER;
      }
    }
  }
  return EFI_SUCCESS;
}

EFI_STATUS
EblSleepCmd (
  IN UINTN Argc,
  IN CHAR8 **Argv
  )
{
  UINTN Delay;

  Delay = (Argc == 1)? 10 : AsciiStrDecimalToUintn (Argv[1]);

  gBS->Stall (Delay * 1000000);

  return EFI_SUCCESS;
}

CHAR8
ConvertToTextLine (
  IN CHAR8  Character
  )
{
  if (Character < ' ' || Character > '~')
  {
    return '.';
  }
  else
  {
    return Character;
  }
}

UINTN
GetBytes (
  IN UINT8  *Address,
  IN UINTN  Bytes
  )
{
  UINTN Result = 0;

  if (Bytes >= 1)
    Result = *Address++;
    
  if (Bytes >= 2)
    Result = (Result << 8) + *Address++;
    
  if (Bytes >= 3)
    Result = (Result << 8) + *Address++;

  return Result;
}

CHAR8 mBlanks[] = "                                           ";

EFI_STATUS
OutputData (
  IN UINT8  *Address,
  IN UINTN  Length,
  IN UINTN  Width,
  IN UINTN  Offset
  )
{
  UINT8 *EndAddress;
  UINTN Line;
  CHAR8 TextLine[0x11];
  UINTN CurrentRow = 0;
  UINTN Bytes;
  UINTN Spaces   = 0;
  CHAR8 Blanks[80];

  AsciiStrCpy (Blanks, mBlanks);
  for (EndAddress = Address + Length; Address < EndAddress; Offset += Line)
  {
    AsciiPrint ("%08x: ", Offset);
    for (Line = 0; (Line < 0x10) && (Address < EndAddress);)
    {
      Bytes = EndAddress - Address;
            
      switch (Width)
      {
        case 4:
          if (Bytes >= 4)
          {
            AsciiPrint ("%08x ", *((UINT32 *)Address));
            TextLine[Line++] = ConvertToTextLine(*Address++);
            TextLine[Line++] = ConvertToTextLine(*Address++);
            TextLine[Line++] = ConvertToTextLine(*Address++);
            TextLine[Line++] = ConvertToTextLine(*Address++);
          }
          else
          {
            AsciiPrint ("%08x ", GetBytes(Address, Bytes));
            Address += Bytes;
            Line    += Bytes;
          }
          break;

        case 2:
          if (Bytes >= 2)
          {
            AsciiPrint ("%04x ", *((UINT16 *)Address));
            TextLine[Line++] = ConvertToTextLine(*Address++);
            TextLine[Line++] = ConvertToTextLine(*Address++);
          }
          else
          {
            AsciiPrint ("%04x ", GetBytes(Address, Bytes));
            Address += Bytes;
            Line    += Bytes;
          }
          break;

        case 1:
          AsciiPrint ("%02x ", *((UINT8 *)Address));
          TextLine[Line++] = ConvertToTextLine(*Address++);
          break;

			  default:
				  AsciiPrint ("Width must be 1, 2, or 4!\n");
				  return EFI_INVALID_PARAMETER;
      }
    }

    // Pad spaces
    if (Line < 0x10)
    {
      switch (Width)
      {
        case 4:
          Spaces = 9 * ((0x10 - Line)/4);
          break;
        case 2:
          Spaces = 5 * ((0x10 - Line)/2);
          break;
        case 1:
          Spaces = 3 * (0x10 - Line);
          break;
      }

      Blanks[Spaces] = '\0';

      AsciiPrint(Blanks);
      
      Blanks[Spaces] = ' ';
    }

    TextLine[Line] = 0;
    AsciiPrint ("|%a|\n", TextLine);

    if (EblAnyKeyToContinueQtoQuit(&CurrentRow, FALSE))
    {
      return EFI_END_OF_FILE;
    }
  }

  if (Length % Width != 0)
  {
    AsciiPrint ("%08x\n", Offset);
  }
  
  return EFI_SUCCESS;
}

#define HEXDUMP_CHUNK 1024

EFI_STATUS
EblHexdumpCmd (
  IN UINTN  Argc,
  IN CHAR8  **Argv
  )
{
  EFI_OPEN_FILE *File;
  VOID          *Location;
  UINTN         Size;
  UINTN         Width = 1;
  UINTN         Offset = 0;
  EFI_STATUS    Status;
  UINTN         Chunk = HEXDUMP_CHUNK;

  if ((Argc < 2) || (Argc > 3))
  {
    return EFI_INVALID_PARAMETER;
  }
  
  if (Argc == 3)
  {
      Width = AsciiStrDecimalToUintn(Argv[2]);
  }
  
  if ((Width != 1) && (Width != 2) && (Width != 4))
  {
    return EFI_INVALID_PARAMETER;
  }

  File = EfiOpen(Argv[1], EFI_FILE_MODE_READ, 0);
  if (File == NULL)
  {
    return EFI_NOT_FOUND;
  }

  Location = AllocatePool(Chunk);
  Size     = EfiTell(File, NULL);

  for (Offset = 0; Offset + HEXDUMP_CHUNK <= Size; Offset += Chunk)
  {
    Chunk = HEXDUMP_CHUNK;
    
    Status = EfiRead(File, Location, &Chunk);
    if (EFI_ERROR(Status))
    {
      AsciiPrint ("Error reading file content\n");
      goto Exit;
    }

    Status = OutputData(Location, Chunk, Width, File->BaseOffset + Offset);
    if (EFI_ERROR(Status))
    {
      if (Status == EFI_END_OF_FILE) {
        Status = EFI_SUCCESS;
      }
      goto Exit;
    }
  }
  
  // Any left over?
  if (Offset < Size)
  {
    Chunk = Size - Offset;
    Status = EfiRead(File, Location, &Chunk);
    if (EFI_ERROR(Status))
    {
      AsciiPrint ("Error reading file content\n");
      goto Exit;
    }

    Status = OutputData(Location, Chunk, Width, File->BaseOffset + Offset);
    if (EFI_ERROR(Status))
    {
      if (Status == EFI_END_OF_FILE) {
        Status = EFI_SUCCESS;
      }
      goto Exit;
    }
  }

Exit:
  EfiClose(File);

  FreePool(Location);

  return EFI_SUCCESS;
}

#define USE_DISKIO 1

EFI_STATUS
EblDiskIoCmd (
  IN UINTN Argc,
  IN CHAR8 **Argv
  )
{
  EFI_STATUS Status;
  UINTN Offset;
  UINT8 *EndOffset;
  UINTN Length;
  UINTN Line;
  UINT8 *Buffer;
  UINT8 *BufferOffset;
  CHAR8 TextLine[0x11];
#if USE_DISKIO
  EFI_DISK_IO_PROTOCOL  *DiskIo;
#else
  EFI_BLOCK_IO_PROTOCOL *BlockIo;
  UINTN                 Lba;
#endif  

  if (AsciiStrCmp(Argv[1], "r") == 0)
  {  
    Offset = AsciiStrHexToUintn(Argv[2]);
    Length = AsciiStrHexToUintn(Argv[3]);

#if USE_DISKIO
    Status = gBS->LocateProtocol(&gEfiDiskIoProtocolGuid, NULL, (VOID **)&DiskIo);
    if (EFI_ERROR(Status))
    {
      AsciiPrint("Did not locate DiskIO\n");
      return Status;
    }

    Buffer = AllocatePool(Length);
    BufferOffset = Buffer;
    
    Status = DiskIo->ReadDisk(DiskIo, SIGNATURE_32('f','l','s','h'), Offset, Length, Buffer);
    if (EFI_ERROR(Status))
    {
      AsciiPrint("DiskIO read failed\n");
      gBS->FreePool(Buffer);
      return Status;
    }    
#else
    Status = gBS->LocateProtocol(&gEfiBlockIoProtocolGuid, NULL, (VOID **)&BlockIo);
    if (EFI_ERROR(Status))
    {
      AsciiPrint("Did not locate BlockIo\n");
      return Status;
    }
    
    Length = BlockIo->Media->BlockSize;
    Buffer = AllocatePool(Length);
    BufferOffset = Buffer;
    Lba = Offset/BlockIo->Media->BlockSize;
    
    Status = BlockIo->ReadBlocks(BlockIo, BlockIo->Media->MediaId, Lba, Length, Buffer);
    if (EFI_ERROR(Status))
    {
      AsciiPrint("BlockIo read failed\n");
      gBS->FreePool(Buffer);
      return Status;
    }
    
    // Whack offset to what we actually read from
    Offset = Lba * BlockIo->Media->BlockSize;
    
    Length = 0x100;
#endif

    for (EndOffset = BufferOffset + Length; BufferOffset < EndOffset; Offset += 0x10)
    {
      AsciiPrint ("%08x: ", Offset);
      
      for (Line = 0; Line < 0x10; Line++)
      {
        AsciiPrint ("%02x ", *BufferOffset);

        if (*BufferOffset < ' ' || *BufferOffset > '~')
          TextLine[Line] = '.';
        else
          TextLine[Line] = *BufferOffset;
          
        BufferOffset++;
      }

      TextLine[Line] = '\0';
      AsciiPrint ("|%a|\n", TextLine);
    }
    
    gBS->FreePool(Buffer);

    return EFI_SUCCESS;
  }
  else if (AsciiStrCmp(Argv[1], "w") == 0)
  {
    Offset = AsciiStrHexToUintn(Argv[2]);
    Length = AsciiStrHexToUintn(Argv[3]);
    Buffer = (UINT8 *)AsciiStrHexToUintn(Argv[4]);
    
#if USE_DISKIO
    Status = gBS->LocateProtocol(&gEfiDiskIoProtocolGuid, NULL, (VOID **)&DiskIo);
    if (EFI_ERROR(Status))
    {
      AsciiPrint("Did not locate DiskIO\n");
      return Status;
    }

    Status = DiskIo->WriteDisk(DiskIo, SIGNATURE_32('f','l','s','h'), Offset, Length, Buffer);
    if (EFI_ERROR(Status))
    {
      AsciiPrint("DiskIO write failed\n");
      return Status;
    }

#else
#endif

    return EFI_SUCCESS;
  }
  else
  {
    return EFI_INVALID_PARAMETER;
  }
}

GLOBAL_REMOVE_IF_UNREFERENCED const EBL_COMMAND_TABLE mCmdTemplate[] =
{
  {
    "reset",
    " [type]; Reset system. type = [warm] [shutdown] default is cold reset",
    NULL,
    EblResetCmd
  },
  {
    "exit",
    "; Exit EBL",
    NULL,
    EblExitCmd
  },
  {
    "help",
    " [cmd]; Help on cmd or a list of all commands if cmd is ommited",
    NULL,
    EblHelpCmd
  },
  {
    "break",
    "; Generate debugging breakpoint",
    NULL,
    EblBreakPointCmd
  },
  {
    "page",
    " [on|off]]; toggle promting on command output larger than screen",
    NULL,
    EblPageCmd
  },
  {
    "pause",
    " [sec]; Pause for sec[10] seconds. ",
    NULL,
    EblPauseCmd
  },
  {
    "sleep",
    " [sec]; Sleep for sec[10] seconds. ",
    NULL,
    EblSleepCmd
  },
  {
    "hexdump",
    " filename ; dump a file as hex bytes",
    NULL,
    EblHexdumpCmd
  },
  {
    "diskio",
    " [r|w] offset [length [dataptr]]; do a DiskIO read or write ",
    NULL,
    EblDiskIoCmd
  }  
};


EFI_HANDLE  gExternalCmdHandle = NULL;

/**
  Initialize the commands in this in this file
**/
VOID
EblInitializeCmdTable (
  VOID
  )
{

  EblAddCommands (mCmdTemplate, sizeof (mCmdTemplate)/sizeof (EBL_COMMAND_TABLE));
  
  gBS->InstallProtocolInterface (
        &gExternalCmdHandle,
        &gEfiEblAddCommandProtocolGuid,
        EFI_NATIVE_INTERFACE,
        &gEblAddCommand
        );

}


VOID
EblShutdownExternalCmdTable (
  VOID
  )
{
  gBS->UninstallProtocolInterface (gExternalCmdHandle, &gEfiEblAddCommandProtocolGuid,  &gEblAddCommand);
}


