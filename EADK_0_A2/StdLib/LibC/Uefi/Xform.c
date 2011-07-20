/** @file
  Value transformations between stdio and the UEFI environment.

  Copyright (c) 2010, Intel Corporation. All rights reserved.<BR>
  This program and the accompanying materials are licensed and made available under
  the terms and conditions of the BSD License that accompanies this distribution.
  The full text of the license may be found at
  http://opensource.org/licenses/bsd-license.php.

  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/
#include  <Uefi.h>

#include  <LibConfig.h>
#include  <sys/EfiCdefs.h>

#include  <errno.h>
#include  <fcntl.h>
#include  "SysEfi.h"

/** Translate the Open flags into a Uefi Open Modes value.

    The Open Flags are:
      O_RDONLY, O_WRONLY,  O_RDWR   // Pick only one

      O_NONBLOCK, O_APPEND, O_CREAT, O_TRUNC, O_EXCL  // ORed with one of the previous

    The UEFI Open modes are:
      // ******************************************************
      // Open Modes
      // ******************************************************
      #define EFI_FILE_MODE_READ         0x0000000000000001
      #define EFI_FILE_MODE_WRITE        0x0000000000000002
      #define EFI_FILE_MODE_CREATE       0x8000000000000000


*/
UINT64
Oflags2EFI( int oflags )
{
  UINT64  flags;

  // Build the Open Modes
  flags = (UINT64)((oflags & O_ACCMODE) + 1);   // Handle the Read/Write flags
  if(oflags & (O_CREAT | O_TRUNC)) {            // Now add the Create flag.
    // Also added if O_TRUNC set since we will need to create a new file.
    // We just set the flags here since the only valid EFI mode with create
    // is Read+Write+Create.
    flags = EFI_FILE_MODE_READ | EFI_FILE_MODE_WRITE | EFI_FILE_MODE_CREATE;
  }
  return flags;
}

/*  Transform the permissions flags from the open() call into the
    Attributes bits needed by UEFI.

    The UEFI File attributes are:
      // ******************************************************
      // File Attributes
      // ******************************************************
      #define EFI_FILE_READ_ONLY         0x0000000000000001
      #define EFI_FILE_HIDDEN            0x0000000000000002
      #define EFI_FILE_SYSTEM            0x0000000000000004
      #define EFI_FILE_RESERVED          0x0000000000000008
      #define EFI_FILE_DIRECTORY         0x0000000000000010
      #define EFI_FILE_ARCHIVE           0x0000000000000020
      #define EFI_FILE_VALID_ATTR        0x0000000000000037

    The input permission flags consist of two groups:
      ( S_IRUSR | S_IRGRP | S_IROTH ) -- S_ACC_READ
      ( S_IWUSR | S_IWGRP | S_IWOTH ) -- S_ACC_WRITE

    The only thing we can set, at this point, is whether or not
    this is a SYSTEM file.  If the group and other bits are
    zero and the user bits are non-zero then set SYSTEM.  Otherwise
    the attributes are zero.

    The attributes can be set later using fcntl().
*/
UINT64
Omode2EFI( int mode)
{
  UINT64  flags = 0;

  if((mode & (S_IRWXG | S_IRWXO)) == 0) {
    if((mode & S_IRWXU) != 0) {
      // Only user permissions so set system
      flags = EFI_FILE_SYSTEM;
    }
  }
  return flags;
}

/* Converts the first several EFI status values into the appropriate errno value.
*/
int
EFI2errno( RETURN_STATUS Status)
{
  int             retval;

  switch(Status) {
    case RETURN_SUCCESS:
      retval = 0;
      break;
    case RETURN_INVALID_PARAMETER:
      retval = EINVAL;
      break;
    case RETURN_UNSUPPORTED:
      retval = ENODEV;
      break;
    case RETURN_BAD_BUFFER_SIZE:
    case RETURN_BUFFER_TOO_SMALL:
      retval = EBUFSIZE;
      break;
    case RETURN_NOT_READY:
      retval = EBUSY;
      break;
    case RETURN_WRITE_PROTECTED:
      retval = EROFS;
      break;
    case RETURN_OUT_OF_RESOURCES:   // May be overridden by specific functions
      retval = ENOMEM;
      break;
    case RETURN_VOLUME_FULL:
      retval = ENOSPC;
      break;
    case RETURN_NOT_FOUND:
    case RETURN_NO_MAPPING:
      retval = ENOENT;
      break;
    case RETURN_TIMEOUT:
      retval = ETIMEDOUT;
      break;
    case RETURN_NOT_STARTED:
      retval = EAGAIN;
      break;
    case RETURN_ALREADY_STARTED:
      retval = EALREADY;
      break;
    case RETURN_ABORTED:
      retval = EINTR;
      break;
    case RETURN_ICMP_ERROR:
    case RETURN_TFTP_ERROR:
    case RETURN_PROTOCOL_ERROR:
      retval = EPROTO;
      break;
    case RETURN_INCOMPATIBLE_VERSION:
      retval = EPERM;
      break;
    case RETURN_ACCESS_DENIED:
    case RETURN_SECURITY_VIOLATION:
      retval = EACCES;
      break;
/*  case RETURN_LOAD_ERROR:
    case RETURN_DEVICE_ERROR:
    case RETURN_VOLUME_CORRUPTED:
    case RETURN_NO_MEDIA:
    case RETURN_MEDIA_CHANGED:
    case RETURN_NO_RESPONSE:
    case RETURN_CRC_ERROR:
    case RETURN_END_OF_MEDIA:
    case RETURN_END_OF_FILE:
    case RETURN_INVALID_LANGUAGE:
*/
    default:
      retval = EIO;
      break;
  }
  return retval;
}
