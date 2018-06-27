## @file
# process FFS generation from FILE statement
#
#  Copyright (c) 2007 - 2018, Intel Corporation. All rights reserved.<BR>
#
#  This program and the accompanying materials
#  are licensed and made available under the terms and conditions of the BSD License
#  which accompanies this distribution.  The full text of the license may be found at
#  http://opensource.org/licenses/bsd-license.php
#
#  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
#  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.
#

##
# Import Modules
#
import Ffs
import Rule
import Common.LongFilePathOs as os
from io import BytesIO
import subprocess

from GenFdsGlobalVariable import GenFdsGlobalVariable
from CommonDataClass.FdfClass import FileStatementClassObject
from Common import EdkLogger
from Common.BuildToolError import *
from Common.Misc import GuidStructureByteArrayToGuidString
from GuidSection import GuidSection
from FvImageSection import FvImageSection
from Common.Misc import SaveFileOnChange
from struct import *

## generate FFS from FILE
#
#
class FileStatement (FileStatementClassObject) :
    ## The constructor
    #
    #   @param  self        The object pointer
    #
    def __init__(self):
        FileStatementClassObject.__init__(self)
        self.CurrentLineNum = None
        self.CurrentLineContent = None
        self.FileName = None
        self.InfFileName = None
        self.SubAlignment = None

    ## GenFfs() method
    #
    #   Generate FFS
    #
    #   @param  self         The object pointer
    #   @param  Dict         dictionary contains macro and value pair
    #   @param  FvChildAddr  Array of the inside FvImage base address
    #   @param  FvParentAddr Parent Fv base address
    #   @retval string       Generated FFS file name
    #
    def GenFfs(self, Dict = {}, FvChildAddr=[], FvParentAddr=None, IsMakefile=False, FvName=None):
        
        if self.NameGuid is not None and self.NameGuid.startswith('PCD('):
            PcdValue = GenFdsGlobalVariable.GetPcdValue(self.NameGuid)
            if len(PcdValue) == 0:
                EdkLogger.error("GenFds", GENFDS_ERROR, '%s NOT defined.' \
                            % (self.NameGuid))
            if PcdValue.startswith('{'):
                PcdValue = GuidStructureByteArrayToGuidString(PcdValue)
            RegistryGuidStr = PcdValue
            if len(RegistryGuidStr) == 0:
                EdkLogger.error("GenFds", GENFDS_ERROR, 'GUID value for %s in wrong format.' \
                            % (self.NameGuid))
            self.NameGuid = RegistryGuidStr
        
        Str = self.NameGuid
        if FvName:
            Str += FvName
        OutputDir = os.path.join(GenFdsGlobalVariable.FfsDir, Str)
        if not os.path.exists(OutputDir):
            os.makedirs(OutputDir)

        Dict.update(self.DefineVarDict)
        SectionAlignments = None
        if self.FvName is not None :
            Buffer = BytesIO('')
            if self.FvName.upper() not in GenFdsGlobalVariable.FdfParser.Profile.FvDict:
                EdkLogger.error("GenFds", GENFDS_ERROR, "FV (%s) is NOT described in FDF file!" % (self.FvName))
            Fv = GenFdsGlobalVariable.FdfParser.Profile.FvDict.get(self.FvName.upper())
            FileName = Fv.AddToBuffer(Buffer)
            SectionFiles = [FileName]

        elif self.FdName is not None:
            if self.FdName.upper() not in GenFdsGlobalVariable.FdfParser.Profile.FdDict:
                EdkLogger.error("GenFds", GENFDS_ERROR, "FD (%s) is NOT described in FDF file!" % (self.FdName))
            Fd = GenFdsGlobalVariable.FdfParser.Profile.FdDict.get(self.FdName.upper())
            FileName = Fd.GenFd()
            SectionFiles = [FileName]

        elif self.FileName is not None:
            if hasattr(self, 'FvFileType') and self.FvFileType == 'RAW':
                if isinstance(self.FileName, list) and isinstance(self.SubAlignment, list) and len(self.FileName) == len(self.SubAlignment):
                    FileContent = ''
                    MaxAlignIndex = 0
                    MaxAlignValue = 1
                    for Index, File in enumerate(self.FileName):
                        try:
                            f = open(File, 'rb')
                        except:
                            GenFdsGlobalVariable.ErrorLogger("Error opening RAW file %s." % (File))
                        Content = f.read()
                        f.close()
                        AlignValue = 1
                        if self.SubAlignment[Index] is not None:
                            AlignValue = GenFdsGlobalVariable.GetAlignment(self.SubAlignment[Index])
                        if AlignValue > MaxAlignValue:
                            MaxAlignIndex = Index
                            MaxAlignValue = AlignValue
                        FileContent += Content
                        if len(FileContent) % AlignValue != 0:
                            Size = AlignValue - len(FileContent) % AlignValue
                            for i in range(0, Size):
                                FileContent += pack('B', 0xFF)

                    if FileContent:
                        OutputRAWFile = os.path.join(GenFdsGlobalVariable.FfsDir, self.NameGuid, self.NameGuid + '.raw')
                        SaveFileOnChange(OutputRAWFile, FileContent, True)
                        self.FileName = OutputRAWFile
                        self.SubAlignment = self.SubAlignment[MaxAlignIndex]

                if self.Alignment and self.SubAlignment:
                    if GenFdsGlobalVariable.GetAlignment (self.Alignment) < GenFdsGlobalVariable.GetAlignment (self.SubAlignment):
                        self.Alignment = self.SubAlignment
                elif self.SubAlignment:
                    self.Alignment = self.SubAlignment

            self.FileName = GenFdsGlobalVariable.ReplaceWorkspaceMacro(self.FileName)
            #Replace $(SAPCE) with real space
            self.FileName = self.FileName.replace('$(SPACE)', ' ')
            SectionFiles = [GenFdsGlobalVariable.MacroExtend(self.FileName, Dict)]

        else:
            SectionFiles = []
            Index = 0
            SectionAlignments = []
            for section in self.SectionList :
                Index = Index + 1
                SecIndex = '%d' %Index
                # process the inside FvImage from FvSection or GuidSection
                if FvChildAddr != []:
                    if isinstance(section, FvImageSection):
                        section.FvAddr = FvChildAddr.pop(0)
                    elif isinstance(section, GuidSection):
                        section.FvAddr = FvChildAddr
                if FvParentAddr is not None and isinstance(section, GuidSection):
                    section.FvParentAddr = FvParentAddr

                if self.KeepReloc == False:
                    section.KeepReloc = False
                sectList, align = section.GenSection(OutputDir, self.NameGuid, SecIndex, self.KeyStringList, None, Dict)
                if sectList != []:
                    for sect in sectList:
                        SectionFiles.append(sect)
                        SectionAlignments.append(align)

        #
        # Prepare the parameter
        #
        FfsFileOutput = os.path.join(OutputDir, self.NameGuid + '.ffs')
        GenFdsGlobalVariable.GenerateFfs(FfsFileOutput, SectionFiles,
                                         Ffs.Ffs.FdfFvFileTypeToFileType.get(self.FvFileType),
                                         self.NameGuid,
                                         Fixed=self.Fixed,
                                         CheckSum=self.CheckSum,
                                         Align=self.Alignment,
                                         SectionAlign=SectionAlignments
                                        )

        return FfsFileOutput



