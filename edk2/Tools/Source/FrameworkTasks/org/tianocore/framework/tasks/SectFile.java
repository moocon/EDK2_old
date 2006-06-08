/** @file
This file is to define nested element which is meant for specifying section file

Copyright (c) 2006, Intel Corporation
All rights reserved. This program and the accompanying materials
are licensed and made available under the terms and conditions of the BSD License
which accompanies this distribution.  The full text of the license may be found at
http://opensource.org/licenses/bsd-license.php

THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/
package org.tianocore.framework.tasks;

import java.io.DataInputStream;
import java.io.DataOutputStream;
import java.io.File;
import java.io.FileInputStream;
import org.apache.tools.ant.BuildException;

/**
 Class SectFile is to define a class corresponding to ANT nested element. It's
 used to specify section file(s) when used with GenFfsFile task
 **/
public class SectFile implements Section {
    private String fileName = ""; /// section file name

    /**
     Get method of ANT task/datatype for "FileName" attribute
  
     @returns The name of section file
     **/
    public String getFileName() {
        return fileName;
    }

    /**
     Set method of ANT task/datatype for "FileName" attribute
  
     @param   fileName    The name of section file
     **/
    public void setFileName(String fileName) {
        this.fileName = fileName;   
    }

    public SectFile (){
    }

    /**
     Put the content of section file into specified buffer with extra bytes for 
     alignment requirement.
  
     @param   Buffer  buffer to contain the section file content with alignment
     **/
    public void toBuffer (DataOutputStream buffer, DataOutputStream orgBuffer){
        File   sectFile;
        byte   data;
        long   fileLen;

        ///
        /// open file
        ///
        sectFile = new File (this.fileName);  

        ///
        /// check if file exist.
        ///     
        if (! sectFile.exists()) {
            throw new BuildException("The file  " + this.fileName + "  is not exist!\n");     
        }


        ///
        /// Read section file and add it's contains to buffer.
        ///
        try {

            FileInputStream fs = new FileInputStream (sectFile.getAbsoluteFile());
            DataInputStream In = new DataInputStream (fs);
            fileLen            = sectFile.length();

            int i = 0;
            while (i < fileLen) {
                data = In.readByte();
                buffer.writeByte(data);
                //
                // Add data to org file 
                //
                orgBuffer.writeByte(data);
                i++;
            }

            ///
            /// 4 byte alignment
            ///
            while ((fileLen & 0x03)!= 0) {
                fileLen ++;
                buffer.writeByte(0);
                //
                // Add data to org file 
                //
                orgBuffer.writeByte(0);
            } 

            ///
            /// close inputStream
            ///
            In.close();

        } catch (Exception e) {
            System.out.print(e.getMessage());
            throw new BuildException("section file2Buffer failed!\n");            
        }
    }
}