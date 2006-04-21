/** @file
  Token class.

  This module contains all classes releted to PCD token.
 
Copyright (c) 2006, Intel Corporation
All rights reserved. This program and the accompanying materials
are licensed and made available under the terms and conditions of the BSD License
which accompanies this distribution.  The full text of the license may be found at
http://opensource.org/licenses/bsd-license.php
 
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/  
package org.tianocore.build.pcd.entity;

import java.util.ArrayList;
import java.util.List;
import java.util.UUID;

import org.tianocore.build.pcd.action.ActionMessage;

/** This class is to descript a PCD token object. The information of a token mainly 
    comes from MSA, SPD and setting produced by platform developer. 
**/
public class Token {
    ///
    /// Enumeration macro defintion for PCD type.
    /// BUGBUG: Not use upcase charater is to facility for reading. It may be changed
    ///         in coding review.
    public enum             PCD_TYPE {FEATURE_FLAG, FIXED_AT_BUILD, PATCHABLE_IN_MODULE, DYNAMIC, 
                                      DYNAMIC_EX, UNKNOWN}

    ///
    /// Enumeration macro definition for datum type. All type mainly comes from ProcessBind.h.
    /// Wizard maybe expand this type as "int, unsigned int, short, unsigned short etc" in 
    /// prompt dialog.
    ///
    public enum             DATUM_TYPE {UINT8, UINT16, UINT32, UINT64, BOOLEAN, POINTER, UNKNOWN}

    ///
    /// Enumeration macor defintion for usage of PCD
    ///
    public enum             PCD_USAGE {ALWAYS_PRODUCED, ALWAYS_CONSUMED, SOMETIMES_PRODUCED,
                                       SOMETIMES_CONSUMED, UNKNOWN}

    ///
    /// cName is to identify a PCD entry and will be used for generating autogen.h/autogen.c.
    /// cName will be defined in MSA, SPD and FPD, can be regarded as primary key with token space guid.
    ///
    public String           cName;

    ///
    /// Token space name is the guid defined by token itself in package or module level. This
    /// name mainly for DynamicEx type. For other PCD type token, his token space name is the 
    /// assignedtokenSpaceName as follows.
    /// tokenSpaceName is defined in MSA, SPD, FPD, can be regarded as primary key with cName.
    ///
    public UUID             tokenSpaceName;

    ///
    /// tokenNumber is allocated by platform. tokenNumber indicate an index for this token in
    /// platform token space.
    /// tokenNumber is defined in SPD, FPD.
    ///
    public int              tokenNumber;

    ///
    /// The token space name assigned by platform. For Non-DynamicEx driver this value is same.
    /// assignedtokenSpaceName is defined in FPD.
    ///
    public UUID             assignedtokenSpaceName;

    ///
    /// The token number assigned by platform. The number indiect the offset of this token in platform
    /// token space.
    /// AssgiendtokenNumber is defined in FPD.
    ///
    public int              assignedtokenNumber;

    ///
    /// pcdType is the PCD item type defined by platform developer.
    ///
    public PCD_TYPE         pcdType;

    ///
    /// PCDtype is set by platform developer. It is final PCD type of this token.
    /// SupportedPcdType is defined in SPD.
    ///
    public PCD_TYPE[]       supportedpcdType;

    ///
    /// datumSize is to descript the fix size or max size for this token. 
    /// datumSize is defined in SPD.
    ///
    public int              datumSize;

    ///
    /// datum type is to descript what type can be expressed by a PCD token.
    /// datumType is defined in SPD.
    ///
    public DATUM_TYPE       datumType;

    ///
    /// Isplatform is to descript whether this token is defined in platform level.
    /// If token is belong to platform level. The value can be different for every
    /// module. All are determined by platform developer.
    ///
    public boolean          isPlatform;

    ///
    /// hiiEnabled is to indicate whether the token support Hii functionality.
    /// hiiEnabled is defined in FPD.
    ///
    public boolean          hiiEnabled;

    ///
    /// variableName is valid only when this token support Hii functionality. variableName
    /// indicates the value of token is associated with what variable.
    /// variableName is defined in FPD.
    ///
    public String           variableName;

    ///
    /// variableGuid is the GUID this token associated with.
    /// variableGuid is defined in FPD.
    ///
    public UUID             variableGuid;

    ///
    /// Variable offset indicate the associated variable's offset in NV storage.
    /// variableOffset is defined in FPD.
    ///
    public int              variableOffset;

    ///
    /// skuEnabled is to indicate whether the token support Sku functionality.
    /// skuEnabled is defined in FPD.
    ///
    public boolean          skuEnabled;

    ///
    /// skuDataArrayEnabled is to indicate wheter use the skuData array or default value.
    ///
    public boolean          skuDataArrayEnabled;

    ///
    /// skuData contains all value for SkuNumber of token.
    /// skuData is defined in FPD.
    ///
    public List<SkuInstance> skuData;

    ///
    /// maxSkuCount indicate the max count of sku data.
    /// maxSkuCount is defined in FPD.
    ///
    public int              maxSkuCount;

    ///
    /// SkuId is the id of current selected SKU.
    /// SkuId is defined in FPD.
    ///
    public int              skuId;

    ///
    /// datum is the value set by platform developer.
    /// datum is defined in FPD.
    ///
    public Object           datum;

    ///
    /// Default value of this token.
    /// This default value is defined in SPD level.
    ///
    public Object           defaultValue;

    ///
    /// BUGBUG: fix comment
    /// vpdEnabled is defined in FPD.
    ///
    public boolean          vpdEnabled;

    ///
    /// BUGBUG: fix comment
    /// vpdOffset is defined in FPD.
    ///
    public long             vpdOffset;

    ///
    /// producers array record all module private information who produce this PCD token.
    ///
    public List<UsageInstance>  producers;

    ///
    /// consumers array record all module private information who consume this PCD token.
    ///
    public List<UsageInstance>  consumers;

    /**
      Constructure function.
     
      Initialize the value of token.
     
      @param cName                   The cName of this token
      @param tokenSpaceName          The tokenSpaceName of this token, it is a GUID.
      @param assignedtokenSpaceName  The assignedtokenSpaceName of this token, it is a GUID.
      
    **/ 
    public Token(String cName, UUID tokenSpaceName, UUID assignedtokenSpaceName) {
        UUID    nullUUID = new UUID(0, 0);

        this.cName                  = cName;
        this.tokenSpaceName         =(tokenSpaceName == null) ? nullUUID : tokenSpaceName;
        this.assignedtokenSpaceName =(assignedtokenSpaceName == null) ? nullUUID : assignedtokenSpaceName;
        this.tokenNumber            = 0;
        this.assignedtokenNumber    = 0;
        this.pcdType                = PCD_TYPE.UNKNOWN;
        this.supportedpcdType       = null;
        this.isPlatform             = false;
        this.datumType              = DATUM_TYPE.UNKNOWN;
        this.datumSize              = -1;
        this.defaultValue           = null;
        this.datum                  = null;
        this.hiiEnabled             = false;
        this.variableGuid           = null;
        this.variableName           = "";
        this.variableOffset         = -1;
        this.skuEnabled             = false;
        this.skuDataArrayEnabled    = false;
        this.skuId                  = -1;
        this.maxSkuCount            = -1;
        this.skuData                = new ArrayList<SkuInstance>();
        this.vpdEnabled             = false;
        this.vpdOffset              = -1;

        this.producers              = new ArrayList<UsageInstance>();
        this.consumers              = new ArrayList<UsageInstance>();
    }

    /**
      Use "TokencName + "-" + SpaceTokenName" as primary key when adding token into database
      
      @param   cName                     Token name.
      @param   tokenSpaceName            The token space guid defined in MSA or SPD
      @param   platformtokenSpaceName    The token space guid for current platform token space,
      
      @return  primary key for this token in token database.
    **/
    public static String getPrimaryKeyString(String cName, UUID tokenSpaceName, 
                                             UUID platformtokenSpaceName) {
        UUID  nullUUID = new UUID(0, 0);

        if (platformtokenSpaceName == nullUUID) {
            return cName + "-" + tokenSpaceName.toString();
        } else {
            return cName + "-" + platformtokenSpaceName.toString();
        }
    }

    /**
      Judge datumType is valid
      
      @param type  The datumType want to be judged.
      
      @retval TRUE  - The type is valid.
      @retval FALSE - The type is invalid.
    **/
    public static boolean isValiddatumType(DATUM_TYPE type) {
        if ((type.ordinal() < DATUM_TYPE.UINT8.ordinal() ) || 
            (type.ordinal() > DATUM_TYPE.POINTER.ordinal())) {
            return false;
        }
        return true;
    }

    /**
      Judge pcdType is valid
      
      @param  type The PCdType want to be judged.
      
      @retval TRUE  - The type is valid.
      @retval FALSE - The type is invalid.
    **/
    public static boolean isValidpcdType(PCD_TYPE  type) {
        if ((type.ordinal() < PCD_TYPE.FEATURE_FLAG.ordinal() ) || 
            (type.ordinal() > PCD_TYPE.DYNAMIC_EX.ordinal())) {
            return false;
        }
        return true;
    }

    /**
      Add an usage instance for token
      
      @param usageInstance   The usage instance
     
      @retval TRUE  - Success to add usage instance.
      @retval FALSE - Fail to add usage instance
    **/
    public boolean addUsageInstance(UsageInstance usageInstance) {
        if (usageInstance.usage == PCD_USAGE.UNKNOWN) {
            return false;
        }

        if ((usageInstance.usage == PCD_USAGE.ALWAYS_PRODUCED) ||
            (usageInstance.usage == PCD_USAGE.SOMETIMES_PRODUCED)) {
            producers.add(usageInstance);
        } else {
            consumers.add(usageInstance);
        }
        return true;
    }

    /**
      Judge whether exist an usage instance for this token
      
      @param moduleName   Use xmlFilePath as keyword to search the usage instance
      
      @retval PCD_USAGE - if UsageInstance exists.
      @retval UNKNOWN   - if UsageInstance does not exist, return UNKONW.
    **/
    public PCD_USAGE isUsageInstanceExist(String moduleName) {
        int           index;
        UsageInstance usageInstance;

        if (moduleName == null) {
            ActionMessage.warning(this, "Error parameter for isUsageInstanceExist() function!");
            return PCD_USAGE.UNKNOWN;
        }

        if (moduleName.length() == 0) {
            return PCD_USAGE.UNKNOWN;
        }

        //
        // Searching the usage instance in module's producer and consumer according to 
        // module's name.
        //
        for (index = 0; index < producers.size(); index ++) {
            usageInstance =(UsageInstance)producers.get(index);
            if (usageInstance.moduleName.equalsIgnoreCase(moduleName)) {
                return usageInstance.usage;
            }
        }

        for (index = 0; index < consumers.size(); index ++) {
            usageInstance =(UsageInstance)consumers.get(index);
            if (usageInstance.moduleName.equalsIgnoreCase(moduleName)) {
                return usageInstance.usage;
            }
        }
        return PCD_USAGE.UNKNOWN;
    }

    /**
      Get usage instance according to a MSA file name
      
      @param moduleName   The file path string of MSA file.

      @return usage instance object.
    **/
    public UsageInstance getUsageInstance(String moduleName) {
        int           usageIndex;
        UsageInstance usageInstance;

        if (moduleName == null) {
            ActionMessage.warning(this, "Error parameter for isUsageInstanceExist() function!");
            return null;
        }

        if (moduleName.length() == 0) {
            return null;
        }

        if (producers.size() != 0) {
            for (usageIndex = 0; usageIndex < producers.size(); usageIndex ++) {
                usageInstance =(UsageInstance)producers.get(usageIndex);
                if (usageInstance.moduleName.equalsIgnoreCase(moduleName)) {
                    return usageInstance;
                }
            }
        }

        if (consumers.size() != 0) {
            for (usageIndex = 0; usageIndex < consumers.size(); usageIndex ++) {
                usageInstance =(UsageInstance)consumers.get(usageIndex);
                if (usageInstance.moduleName.equalsIgnoreCase(moduleName)) {
                    return usageInstance;
                }
            }
        }
        return null;
    }

    /**
      Get the PCD_TYPE according to the string of PCD_TYPE
      
      @param pcdTypeStr    The string of PCD_TYPE
      
      @return PCD_TYPE
    **/
    public static PCD_TYPE getpcdTypeFromString(String pcdTypeStr) {
        if (pcdTypeStr == null) {
            return PCD_TYPE.UNKNOWN;
        }

        if (pcdTypeStr.equalsIgnoreCase("FEATURE_FLAG")) {
            return PCD_TYPE.FEATURE_FLAG;
        } else if (pcdTypeStr.equalsIgnoreCase("FIXED_AT_BUILD")) {
            return PCD_TYPE.FIXED_AT_BUILD;
        } else if (pcdTypeStr.equalsIgnoreCase("PATCHABLE_IN_MODULE")) {
            return PCD_TYPE.PATCHABLE_IN_MODULE;
        } else if (pcdTypeStr.equalsIgnoreCase("DYNAMIC")) {
            return PCD_TYPE.DYNAMIC;
        } else if (pcdTypeStr.equalsIgnoreCase("DYNAMIC_EX")) {
            return PCD_TYPE.DYNAMIC_EX;
        } else {
            return PCD_TYPE.UNKNOWN;
        }
    }

    /**
      Get the string of given datumType. This string will be used for generating autogen files
     
      @param datumType   Given datumType
     
      @return The string of datum type.
    **/
    public static String getStringOfdatumType(DATUM_TYPE  datumType) {
        switch (datumType) {
        case UINT8:
            return "UINT8";
        case UINT16:
            return "UINT16";
        case UINT32:
            return "UINT32";
        case UINT64:
            return "UINT64";
        case POINTER:
            return "POINTER";
        case BOOLEAN:
            return "BOOLEAN";
        }
        return "UNKNOWN";
    }

    /**
      Get the datumType according to a string.
      
      @param datumTypeStr    The string of datumType
     
      @return DATUM_TYPE
    **/
    public static DATUM_TYPE getdatumTypeFromString(String datumTypeStr) {
        if (datumTypeStr.equalsIgnoreCase("UINT8")) {
            return DATUM_TYPE.UINT8;
        } else if (datumTypeStr.equalsIgnoreCase("UINT16")) {
            return DATUM_TYPE.UINT16;
        } else if (datumTypeStr.equalsIgnoreCase("UINT32")) {
            return DATUM_TYPE.UINT32;
        } else if (datumTypeStr.equalsIgnoreCase("UINT64")) {
            return DATUM_TYPE.UINT64;
        } else if (datumTypeStr.equalsIgnoreCase("VOID*")) {
            return DATUM_TYPE.POINTER;
        } else if (datumTypeStr.equalsIgnoreCase("BOOLEAN")) {
            return DATUM_TYPE.BOOLEAN;
        }
        return DATUM_TYPE.UNKNOWN;
    }

    /**
      Get string of given pcdType
      
      @param pcdType  The given PcdType
      
      @return The string of PCD_TYPE.
    **/
    public static String getStringOfpcdType(PCD_TYPE pcdType) {
        switch (pcdType) {
        case FEATURE_FLAG:
            return "FEATURE_FLAG";
        case FIXED_AT_BUILD:
            return "FIXED_AT_BUILD";
        case PATCHABLE_IN_MODULE:
            return "PATCHABLE_IN_MODULE";
        case DYNAMIC:
            return "DYNAMIC";
        case DYNAMIC_EX:
            return "DYNAMIC_EX";
        }
        return "UNKNOWN";
    }

    /**
      Get the PCD_USAGE according to a string
      
      @param usageStr  The string of PCD_USAGE
      
      @return The PCD_USAGE
    **/
    public static PCD_USAGE getUsageFromString(String usageStr) {
        if (usageStr == null) {
            return PCD_USAGE.UNKNOWN;
        }

        if (usageStr.equalsIgnoreCase("ALWAYS_PRODUCED")) {
            return PCD_USAGE.ALWAYS_PRODUCED;
        } else if (usageStr.equalsIgnoreCase("SOMETIMES_PRODUCED")) {
            return PCD_USAGE.SOMETIMES_PRODUCED;
        } else if (usageStr.equalsIgnoreCase("ALWAYS_CONSUMED")) {
            return PCD_USAGE.ALWAYS_CONSUMED;
        } else if (usageStr.equalsIgnoreCase("SOMETIMES_CONSUMED")) {
            return PCD_USAGE.SOMETIMES_CONSUMED;
        }

        return PCD_USAGE.UNKNOWN;
    }

    /**
      Get the string of given PCD_USAGE
      
      @param   usage   The given PCD_USAGE
      
      @return The string of PDC_USAGE.
    **/
    public static String getStringOfUsage(PCD_USAGE usage) {
        switch (usage) {
        case ALWAYS_PRODUCED:
            return "ALWAYS_PRODUCED";
        case ALWAYS_CONSUMED:
            return "ALWAYS_CONSUMED";
        case SOMETIMES_PRODUCED:
            return "SOMETIMES_PRODUCED";
        case SOMETIMES_CONSUMED:
            return "SOMETIMES_CONSUMED";
        }
        return "UNKNOWN";
    }

    /**
      Get the Defined datumType string for autogen. The string is for generating some MACROs in Autogen.h
      
      @param datumType The given datumType

      @return string of datum type for autogen.
    **/
    public static String GetAutogenDefinedatumTypeString(DATUM_TYPE datumType) {
        switch (datumType) {
        
        case UINT8:
            return "8";
        case UINT16:
            return "16";
        case BOOLEAN:
            return "BOOL";
        case POINTER:
            return "PTR";
        case UINT32:
            return "32";
        case UINT64:
            return "64";
        default:
            return null;
        }
    }

    /**
      Get the datumType String for Autogen. This string will be used for generating defintions of PCD token in autogen
      
      @param datumType   The given datumType

      @return string of datum type.
    **/
    public static String getAutogendatumTypeString(DATUM_TYPE datumType) {
        switch (datumType) {
        case UINT8:
            return "UINT8";
        case UINT16:
            return "UINT16";
        case UINT32:
            return "UINT32";
        case UINT64:
            return "UINT64";
        case POINTER:
            return "VOID*";
        case BOOLEAN:
            return "BOOLEAN";
        }
        return null;
    }

    /**
      Get the datumType string for generating some MACROs in autogen file of Library
      
      @param   datumType  The given datumType

      @return String of datum for genrating bit charater.
    **/
    public static String getAutogenLibrarydatumTypeString(DATUM_TYPE datumType) {
        switch (datumType) {
        case UINT8:
            return "8";
        case UINT16:
            return "16";
        case BOOLEAN:
            return "Bool";
        case POINTER:
            return "Ptr";
        case UINT32:
            return "32";
        case UINT64:
            return "64";
        default:
            return null;
        }
    }

    /**
      UUID defined in Schems is object, this function is to tranlate this object 
      to UUID data.
      
      @param uuidObj The object comes from schema.
      
      @return The traslated UUID instance.
    **/
    public static UUID getGUIDFromSchemaObject(Object uuidObj) {
        UUID uuid;
        if (uuidObj.toString().equalsIgnoreCase("0")) {
            uuid = new UUID(0,0);
        } else {
            uuid = UUID.fromString(uuidObj.toString());
        }

        return uuid;
    }
}




