/** @file
 SecApResetVectorFixupTask class.

 SecApResetVectorFixupTask is used to call SecApResetVectorFixup.exe to place
 Ap reset vector.
 
 
 Copyright (c) 2006, Intel Corporation
 All rights reserved. This program and the accompanying materials
 are licensed and made available under the terms and conditions of the BSD License
 which accompanies this distribution.  The full text of the license may be found at
 http://opensource.org/licenses/bsd-license.php
 
 THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
 WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

 **/
package org.tianocore.framework.tasks;

import java.io.File;

import org.apache.tools.ant.Task;
import org.apache.tools.ant.Project;
import org.apache.tools.ant.BuildException;
import org.apache.tools.ant.taskdefs.Execute;
import org.apache.tools.ant.taskdefs.LogStreamHandler;
import org.apache.tools.ant.types.Commandline;
import org.tianocore.logger.EdkLog;

/**
  SecApResetVectorFixupTask class.

  SecApResetVectorFixupTask is used to call SecApResetVectorFixup.exe to place
  Ap reset vector.
**/
public class SecApResetVectorFixupTask extends Task implements EfiDefine {
    ///
    /// tool name
    ///
    private String toolName = "SecApResetVectorFixup";
    // /
    // / input FV recovery file
    // /
    private String fvInputFile = "";

    // /
    // / output file
    // /
    private String fvOutputFile = "";

    // /
    // / output directory, this variable is added by jave wrap
    // /
    private String outputDir = "";


    /**
     * execute
     * 
     * SecApResetVectorFixupTask execute function is to assemble tool command line & execute
     * tool command line
     * 
     * @throws BuidException
     */
    public void execute() throws BuildException {

        Project project = this.getOwningTarget().getProject();
        //
        // set Logger
        //
        FrameworkLogger logger = new FrameworkLogger(project, "secapresetvectorfixup");
        EdkLog.setLogLevel(project.getProperty("env.LOGLEVEL"));
        EdkLog.setLogger(logger);
        //
        // absolute path of efi tools
        //
        String path = project.getProperty("env.FRAMEWORK_TOOLS_PATH");
        String command;
        String argument;
        if (path == null) {
            command = toolName;
        } else {
            command = path + File.separatorChar + toolName;
        }
        //
        // argument of tools
        //
        File file = new File(this.fvOutputFile);
        if (!file.isAbsolute() && (!this.outputDir.equalsIgnoreCase(""))) {
            argument = this.fvInputFile + " " + outputDir + File.separatorChar
                    + this.fvOutputFile;
        } else {
            argument = this.fvInputFile + " " + this.fvOutputFile;
        }
        //
        // return value of fwimage execution
        //
        int revl = -1;

        try {
            Commandline cmdline = new Commandline();
            cmdline.setExecutable(command);
            cmdline.createArgument().setLine(argument);

            LogStreamHandler streamHandler = new LogStreamHandler(this,
                    Project.MSG_INFO, Project.MSG_WARN);
            Execute runner = new Execute(streamHandler, null);

            runner.setAntRun(project);
            runner.setCommandline(cmdline.getCommandline());
            //
            // Set debug log information.
            //
            EdkLog.log(EdkLog.EDK_INFO, Commandline.toString(cmdline.getCommandline()));
            
            revl = runner.execute();
            
            if (EFI_SUCCESS == revl) {
                //
                // command execution success
                //
                EdkLog.log(EdkLog.EDK_INFO,"SecApResetVectorFixup succeeded!");
            } else {
                //
                // command execution fail
                //
                EdkLog.log(EdkLog.EDK_ERROR, "SecApResetVectorFixup failed. (error="
                        + Integer.toHexString(revl) + ")");
                throw new BuildException("SecApResetVectorFixup failed. (error="
                        + Integer.toHexString(revl) + ")");

            }
        } catch (Exception e) {
            throw new BuildException(e.getMessage());
        }
    }

    /**
     * getInputFile
     * 
     * This function is to get class member "fvInputFile".
     * 
     * @return string of input file name.
     */
    public String getfvInputFile() {
        return this.fvInputFile;
    }

    /**
     * setComponentType
     * 
     * This function is to set class member "fvInputFile".
     * 
     * @param inputFile
     *            string of input file name.
     */
    public void setFvInputFile(String inputFile) {
        this.fvInputFile = inputFile;
    }

    /**
     * getOutputFile
     * 
     * This function is to get class member "fvOutputFile"
     * 
     * @return outputFile string of output file name.
     */
    public String getOutputFile() {
        return this.fvOutputFile;
    }

    /**
     * setOutputFile
     * 
     * This function is to set class member "fvOutputFile"
     * 
     * @param outputFile
     *            string of output file name.
     */
    public void setFvOutputFile(String outputFile) {
        this.fvOutputFile = outputFile;
    }

    /**
     * getOutputDir
     * 
     * This function is to get class member "outputDir"
     * 
     * @return outputDir string of output directory.
     */
    public String getOutputDir() {
        return outputDir;
    }

    /**
     * setOutputDir
     * 
     * This function is to set class member "outputDir"
     * 
     * @param outputDir
     *            string of output directory.
     */
    public void setOutputDir(String outputDir) {
        this.outputDir = outputDir;
    }
}
