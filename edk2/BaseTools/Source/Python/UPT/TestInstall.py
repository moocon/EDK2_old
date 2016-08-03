# # @file
# Test Install distribution package
#
# Copyright (c) 2016, Intel Corporation. All rights reserved.<BR>
#
# This program and the accompanying materials are licensed and made available
# under the terms and conditions of the BSD License which accompanies this
# distribution. The full text of the license may be found at
# http://opensource.org/licenses/bsd-license.php
#
# THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
# WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.
#
"""
Test Install multiple distribution package
"""
# #
# Import Modules
#
from Library import GlobalData
import Logger.Log as Logger
from Logger import StringTable as ST
import Logger.ToolError as TE
from Core.DependencyRules import DependencyRules
from InstallPkg import UnZipDp

import shutil
from traceback import format_exc
from platform import python_version
from sys import platform

# # Tool entrance method
#
# This method mainly dispatch specific methods per the command line options.
# If no error found, return zero value so the caller of this tool can know
# if it's executed successfully or not.
#
# @param  Options: command Options
#
def Main(Options=None):
    ContentZipFile, DistFile = None, None
    ReturnCode = 0

    try:
        DataBase = GlobalData.gDB
        WorkspaceDir = GlobalData.gWORKSPACE
        if not Options.DistFiles:
            Logger.Error("TestInstallPkg", TE.OPTION_MISSING, ExtraData=ST.ERR_SPECIFY_PACKAGE)

        DistPkgList = []
        for DistFile in Options.DistFiles:
            DistPkg, ContentZipFile, __, DistFile = UnZipDp(WorkspaceDir, DistFile)
            DistPkgList.append(DistPkg)

        #
        # check dependency
        #
        Dep = DependencyRules(DataBase)
        Result = True
        DpObj = None
        try:
            Result, DpObj = Dep.CheckTestInstallPdDepexSatisfied(DistPkgList)
        except:
            Result = False

        if Result:
            Logger.Quiet(ST.MSG_TEST_INSTALL_PASS)
        else:
            Logger.Quiet(ST.MSG_TEST_INSTALL_FAIL)

    except TE.FatalError, XExcept:
        ReturnCode = XExcept.args[0]
        if Logger.GetLevel() <= Logger.DEBUG_9:
            Logger.Quiet(ST.MSG_PYTHON_ON % (python_version(), platform) + format_exc())

    except Exception, x:
        ReturnCode = TE.CODE_ERROR
        Logger.Error(
                    "\nTestInstallPkg",
                    TE.CODE_ERROR,
                    ST.ERR_UNKNOWN_FATAL_INSTALL_ERR % Options.DistFiles,
                    ExtraData=ST.MSG_SEARCH_FOR_HELP,
                    RaiseError=False
                    )
        Logger.Quiet(ST.MSG_PYTHON_ON % (python_version(), platform) + format_exc())

    finally:
        Logger.Quiet(ST.MSG_REMOVE_TEMP_FILE_STARTED)
        if DistFile:
            DistFile.Close()
        if ContentZipFile:
            ContentZipFile.Close()
        if GlobalData.gUNPACK_DIR:
            shutil.rmtree(GlobalData.gUNPACK_DIR)
            GlobalData.gUNPACK_DIR = None
        Logger.Quiet(ST.MSG_REMOVE_TEMP_FILE_DONE)
    if ReturnCode == 0:
        Logger.Quiet(ST.MSG_FINISH)
    return ReturnCode

