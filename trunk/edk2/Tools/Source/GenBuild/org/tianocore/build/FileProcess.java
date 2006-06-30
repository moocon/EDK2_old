/** @file
  File is FileProcess class which is used to generate ANT script to build 
  source files. 
  
Copyright (c) 2006, Intel Corporation
All rights reserved. This program and the accompanying materials
are licensed and made available under the terms and conditions of the BSD License
which accompanies this distribution.  The full text of the license may be found at
http://opensource.org/licenses/bsd-license.php

THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.
**/
package org.tianocore.build;

import java.io.File;
import java.util.Set;

import org.apache.tools.ant.BuildException;
import org.apache.tools.ant.Project;
import org.w3c.dom.Document;
import org.w3c.dom.Element;
import org.w3c.dom.Node;

/**
  <p><code>FileProcess</code> is class to generate ANT script to build source
  files.</p>
  
  <p>If file does not specify file type, <code>FileProcess</code> will judge 
  by its extension. Following is the current supported extensions. </p>
  
  <pre>   
 Source File Suffix     File Type       Description
    .h                   CHeader      C header file
    .c                   CCode        C source file
    .inc                 ASMHeader    Assembly header file
    .asm                 ASM          Assembly source file, usually for IA32 and X64 Arch and MSFT tool chain
    .S                   ASM          Assembly source file, usually for IPF Arch
    .s                   ASM          Assembly source file, usually for IA32 and X64 Arch and GCC tool chain
    .uni                 UNI          Unicode file
    .vfr                 VFR          Visual Forms Representation File
    .fv                  FV           Firmware Volume
    .SEC                 FFS          Firmware File System file
    .PEI                 FFS          Firmware File System file
    .DXE                 FFS          Firmware File System file
    .APP                 FFS          Firmware File System file
    .FVI                 FFS          Firmware File System file
    .FFS                 FFS          Firmware File System file
    .bmp                 BMP          Graphic File
    .i                   PPCode       IPF PreProcessor Code
  </pre>
  
  @since GenBuild 1.0
**/
public class FileProcess {
    ///
    ///  The mapping information about source suffix, result suffix, file type.
    ///
    public final String[][] fileTypes = { {".h", "", "CHeader" }, 
                                          {".c", "", "CCode" },
                                          {".inc", "", "ASMHeader" },
                                          {".asm", "", "ASM" }, 
                                          {".S", "", "ASM" },
                                          {".s", "", "ASM" },
                                          {".uni", "", "UNI" },
                                          {".vfr", "", "VFR" },
                                          {".Vfr", "", "VFR" },
                                          {".dxs", "", "DPX"},
                                          {".fv", "", "FV" },
                                          {".efi", "", "EFI" },
                                          {".SEC", "", "FFS" },
                                          {".PEI", "", "FFS" },
                                          {".DXE", "", "FFS" },
                                          {".APP", "", "FFS" },
                                          {".FYI", "", "FFS" },
                                          {".FFS", "", "FFS" },
                                          {".bmp", "", "BMP" },
                                          {".i", "", "PPCode"}};
    ///
    /// Current ANT context. 
    ///
    private Project project;

    ///
    /// Current module's include pathes
    ///
    private Set<String> includes;
    
    ///
    /// Xml Document.
    ///
    private Document document;
    
    ///
    /// The flag to ensure all unicode files build before others. 
    ///
    private boolean unicodeFirst = true;
    
    ///
    /// The flag present whether current module contains Unicode files or not.
    ///
    private boolean unicodeExist = false;

    /**
      Initialize the project, includes, sourceFiles, document members.
      
      @param project ANT project
      @param includes Module include pathes
      @param sourceFiles Modules source files
      @param document XML document
    **/
    public void init(Project project, Set<String> includes, Document document) {
        this.document = document;
        this.includes = includes;
        this.project = project;
    }

    /**
      Parse file without file type. 
      
      @param filename Source file name
      @param root Root node
      @param unicodeFirst whether build Unicode file firstly or not
    **/
    public synchronized void parseFile(String filename, Node root, boolean unicodeFirst) {
        this.unicodeFirst = unicodeFirst;
        parseFile(filename, root);
    }
    
    /**
      Get whether current module contains Unicode files or not.
      
      @return Whether current module contains Unicode files or not
    **/
    public boolean isUnicodeExist() {
        return unicodeExist;
    }

    /**
      Parse file.
      
      @param filename Source file name
      @param filetype Source file type
      @param root Root node
      @param unicodeFirst whether build Unicode file firstly or not
    **/
    public synchronized void parseFile(String filename, String filetype, Node root, boolean unicodeFirst) {
        this.unicodeFirst = unicodeFirst;
        parseFile(filename, filetype, root);
    }
    
    /**
      Find out source file's type. 
      
      @param filename Source file name
      @param root Root node
    **/
    public synchronized void parseFile(String filename, Node root) throws BuildException {
        boolean flag = false;
        for (int i = 0; i < fileTypes.length; i++) {
            if (filename.endsWith(fileTypes[i][0])) {
                flag = true;
                parseFile(filename, fileTypes[i][2], root);
            }
        }
        if (!flag) {
            throw new BuildException("File [" + filename + "] is not known from its suffix.");
        }
    }

    /**
      Parse file. If flag <code>unicodeFirst</code> is true, then build all
      unicode files firstly. 
      
      <p>Note that AutoGen.c is processed specially. It's output path is always
      <code>${DEST_DIR_OUTPUT}</code>, others are <code>${DEST_DIR_OUTPUT}</code>
      and relative to module path. </p>
      
      @param filename Source file name
      @param filetype Source file type
      @param root Root node
    **/
    public synchronized void parseFile(String filename, String filetype, Node root) {
        if (unicodeFirst) {
            if ( ! filetype.equalsIgnoreCase("UNI")){
                return ;
            }
            unicodeExist= true;
        } else {
            if (filetype.equalsIgnoreCase("UNI")){
                return ;
            }
        }
        
        //
        // If file is C or ASM header file, skip it
        //
        if (filetype.equalsIgnoreCase("CHeader") || filetype.equalsIgnoreCase("ASMHeader")) {
            return;
        }
        
        //
        // If file is pre-processor file, skip it
        // 
        if (filetype.equalsIgnoreCase("PPCode")) {
            return;
        }
        
        //
        // If define CC_EXT in tools_def.txt file, the source file with 
        // different suffix is skipped
        //
        String toolsDefExtName = project.getProperty(filetype + "_EXT");
        if (toolsDefExtName != null) {
            String[] exts = toolsDefExtName.split(" ");
            for (int i = 0; i < exts.length; i++) {
                if ( ! filename.endsWith(exts[i])) {
                    return ;
                }
            }
        }
        
        String module_path = project.getProperty("MODULE_DIR");
        File moduleFile = new File(module_path);
        File sourceFile = new File(filename);
        
        //
        // If source file is AutoGen.c, then Filepath is .
        //
        String sourceFilepath = "";
        String sourceFilename = "";
        String sourceFileext = "";
        if (sourceFile.getPath().endsWith("AutoGen.c")) {
            sourceFilepath = ".";
            sourceFilename = "AutoGen";
            sourceFileext = ".c";
            filetype = "AUTOGEN";
        } else {
            // sourceFile.
            String str = sourceFile.getPath().substring(moduleFile.getPath().length() + 1);
            int index = str.lastIndexOf(File.separatorChar);
            sourceFilepath = ".";
            if (index > 0) {
                sourceFilepath = str.substring(0, index);
                str = str.substring(index + 1);
            }
            sourceFilename = str;
            index = str.lastIndexOf('.');
            if (index > 0) {
                sourceFilename = str.substring(0, index);
                sourceFileext = str.substring(index);
            }
        }
        // <Build_filetype FILEPATH="" FILENAME="" />
        Element ele = document.createElement("Build_" + filetype);
        ele.setAttribute("FILEPATH", sourceFilepath);
        ele.setAttribute("FILENAME", sourceFilename);
        ele.setAttribute("FILEEXT", sourceFileext.substring(1));
        String[] includePaths = includes.toArray(new String[includes.size()]);
        Element includesEle = document.createElement("EXTRA.INC");
        for (int i = 0; i < includePaths.length; i++) {
            Element includeEle = document.createElement("includepath");
            includeEle.setAttribute("path", includePaths[i]);
            includesEle.appendChild(includeEle);
        }
        ele.appendChild(includesEle);
        root.appendChild(ele);
    }
}