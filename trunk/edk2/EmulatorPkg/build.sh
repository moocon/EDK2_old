#!/bin/bash
#
# Copyright (c) 2008 - 2011, Apple Inc. All rights reserved.<BR>
# Copyright (c) 2010 - 2011, Intel Corporation. All rights reserved.<BR>
#
# This program and the accompanying materials
# are licensed and made available under the terms and conditions of the BSD License
# which accompanies this distribution.  The full text of the license may be found at
# http://opensource.org/licenses/bsd-license.php
#
# THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
# WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.
#

set -e
shopt -s nocasematch


#
# Setup workspace if it is not set
#
if [ -z "$WORKSPACE" ]
then
  echo Initializing workspace
  if [ ! -e `pwd`/edksetup.sh ]
  then
    cd ..
  fi
# This version is for the tools in the BaseTools project.
# this assumes svn pulls have the same root dir
#  export EDK_TOOLS_PATH=`pwd`/../BaseTools
# This version is for the tools source in edk2
  export EDK_TOOLS_PATH=`pwd`/BaseTools
  echo $EDK_TOOLS_PATH
  source edksetup.sh BaseTools
else
  echo Building from: $WORKSPACE
fi

#
# Configure defaults for various options
#

PROCESSOR=
BUILDTARGET=DEBUG
BUILD_OPTIONS=
PLATFORMFILE=
LAST_ARG=
RUN_EMULATOR=no
CLEAN_TYPE=none
UNIXPKG_TOOLS=GCC44
NETWORK_SUPPORT=
BUILD_NEW_SHELL=
BUILD_FAT=
HOST_PROCESSOR=X64

case `uname` in
  CYGWIN*) echo Cygwin not fully supported yet. ;;
  Darwin*)
      Major=$(uname -r | cut -f 1 -d '.')
      if [[ $Major == 9 ]]
      then
        echo UnixPkg requires Snow Leopard or later OS
        exit 1
      else
        TARGET_TOOLS=XCODE32
        UNIXPKG_TOOLS=XCLANG
      fi
      BUILD_NEW_SHELL="-D BUILD_NEW_SHELL"
      BUILD_FAT="-D BUILD_FAT"
      ;;
  Linux*)
    case `uname -m` in
      i386)
        HOST_PROCESSOR=IA32
        ;;
      i686)
        HOST_PROCESSOR=IA32
        ;;
      x86_64)
        HOST_PROCESSOR=X64
        ;;
    esac
    ;;
esac

#
# Scan command line to override defaults
#

for arg in "$@"
do
  if [ -z "$LAST_ARG" ]; then
    case $arg in
      -a|-b|-t|-p)
        LAST_ARG=$arg
        ;;
      run)
        RUN_EMULATOR=yes
        shift
        break
        ;;
      clean|cleanall)
        CLEAN_TYPE=$arg
        shift
        break
        ;;
      *)
        BUILD_OPTIONS="$BUILD_OPTIONS $arg"
        ;;
    esac
  else
    case $LAST_ARG in
      -a)
        PROCESSOR=$arg
        ;;
      -b)
        BUILDTARGET=$arg
        ;;
      -p)
        PLATFORMFILE=$arg
        ;;
      -t)
        TARGET_TOOLS=$arg
        ;;
      *)
        BUILD_OPTIONS="$BUILD_OPTIONS $arg"
        ;;
    esac
    LAST_ARG=
  fi
  shift
done
if [ -z "$TARGET_TOOLS" ]
then
  TARGET_TOOLS=$UNIXPKG_TOOLS
fi

if [ -z "$PROCESSOR" ]
then
  PROCESSOR=$HOST_PROCESSOR
fi

case $PROCESSOR in
  IA32)
    ARCH_SIZE=32
    BUILD_OUTPUT_DIR=$WORKSPACE/Build/Emulator32
    if [ -d /lib32 ]; then
      export LIB_ARCH_SFX=32
    fi
    ;;
  X64)
    ARCH_SIZE=64
    BUILD_OUTPUT_DIR=$WORKSPACE/Build/Emulator
    if [ -d /lib64 ]; then
      export LIB_ARCH_SFX=64
    fi
    ;;
esac


PLATFORMFILE=$WORKSPACE/EmulatorPkg/EmulatorPkg.dsc
BUILD_ROOT_ARCH=$BUILD_OUTPUT_DIR/DEBUG_"$UNIXPKG_TOOLS"/$PROCESSOR

if  [[ ! -f `which build` || ! -f `which GenFv` ]];
then
  # build the tools if they don't yet exist. Bin scheme
  echo Building tools as they are not in the path
  make -C $WORKSPACE/BaseTools
elif [[ ( -f `which build` ||  -f `which GenFv` )  && ! -d  $EDK_TOOLS_PATH/Source/C/bin ]];
then
  # build the tools if they don't yet exist. BinWrapper scheme
  echo Building tools no $EDK_TOOLS_PATH/Source/C/bin directory
  make -C $WORKSPACE/BaseTools
else
  echo using prebuilt tools
fi


if [[ "$RUN_EMULATOR" == "yes" ]]; then
  case `uname` in
    Darwin*)
      #
      # On Darwin we can't use dlopen, so we have to load the real PE/COFF images.
      # This .gdbinit script sets a breakpoint that loads symbols for the PE/COFFEE
      # images that get loaded in Host
      #
      cp $WORKSPACE/EmulatorPkg/Unix/.gdbinit $BUILD_OUTPUT_DIR/DEBUG_"$UNIXPKG_TOOLS"/$PROCESSOR
      ;;
  esac

  /usr/bin/gdb $BUILD_ROOT_ARCH/Host -q -cd=$BUILD_ROOT_ARCH -x $WORKSPACE/EmulatorPkg/Unix/GdbRun
  exit
fi

case $CLEAN_TYPE in
  clean)
    build -p $WORKSPACE/EmulatorPkg/EmulatorPkg.dsc -a $PROCESSOR -t $TARGET_TOOLS -D UNIX_SEC_BUILD -n 3 clean
    build -p $WORKSPACE/EmulatorPkg/EmulatorPkg.dsc -a $PROCESSOR -t $UNIXPKG_TOOLS -n 3 clean
    exit $?
    ;;
  cleanall)
    make -C $WORKSPACE/BaseTools clean
    build -p $WORKSPACE/EmulatorPkg/EmulatorPkg.dsc -a $PROCESSOR -t $TARGET_TOOLS -D UNIX_SEC_BUILD -n 3 clean
    build -p $WORKSPACE/EmulatorPkg/EmulatorPkg.dsc -a $PROCESSOR -t $UNIXPKG_TOOLS -n 3 clean
    build -p $WORKSPACE/ShellPkg/ShellPkg.dsc -a IA32 -t $UNIXPKG_TOOLS -n 3 clean
    exit $?
    ;;
esac


#
# Build the edk2 EmulatorPkg
#
if [[ $TARGET_TOOLS == $UNIXPKG_TOOLS ]]; then
  build -p $WORKSPACE/EmulatorPkg/EmulatorPkg.dsc -a $PROCESSOR -t $UNIXPKG_TOOLS -D BUILD_$ARCH_SIZE -D UNIX_SEC_BUILD $NETWORK_SUPPORT $BUILD_NEW_SHELL $BUILD_FAT -n 3 $1 $2 $3 $4 $5 $6 $7 $8
else
  build -p $WORKSPACE/EmulatorPkg/EmulatorPkg.dsc -a $PROCESSOR -t $TARGET_TOOLS  -D BUILD_$ARCH_SIZE -D UNIX_SEC_BUILD -D SKIP_MAIN_BUILD -n 3 $1 $2 $3 $4 $5 $6 $7 $8  modules
  build -p $WORKSPACE/EmulatorPkg/EmulatorPkg.dsc -a $PROCESSOR -t $UNIXPKG_TOOLS -D BUILD_$ARCH_SIZE $NETWORK_SUPPORT $BUILD_NEW_SHELL $BUILD_FAT -n 3 $1 $2 $3 $4 $5 $6 $7 $8
  cp $BUILD_OUTPUT_DIR/DEBUG_"$TARGET_TOOLS"/$PROCESSOR/Host $BUILD_ROOT_ARCH
fi
exit $?

