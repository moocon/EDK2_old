/** @file
 ModifyInfTask class.

 ModifyInfTask is used to call Modify.exe to generate inf file.
 
 
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
  ModifyInfTask class.

  ModifyInfTask is used to call Modify.exe to generate inf file.
**/
public class ModifyInfTask extends Task implements EfiDefine {
    ///
    /// tool name
    ///
    private String toolName = "ModifyInf";
    
    ///
    /// input FV inf file
    ///
    private String inputFVInfFileName = "";

    ///
    /// output FV inf file
    ///
    private String outputFVInfFileName = "";

    ///
    /// pattern string
    ///
    private String patternStr = "";

	///
	///  Output dir
	/// 
	private String outputDir = "";
   
    /**
     * execute
     * 
     * ModifyInfTask execute function is to assemble tool command line & execute
     * tool command line
     * 
     * @throws BuidException
     */
    public void execute() throws BuildException {

        Project project = this.getOwningTarget().getProject();
        //
        // set Logger
        //
        FrameworkLogger logger = new FrameworkLogger(project, "modifytask");
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
		File file = new File(outputFVInfFileName);
        if (!file.isAbsolute() && (!this.outputDir.equalsIgnoreCase(""))) {
			argument = this.inputFVInfFileName +
				       this.outputDir + 
					   File.separatorChar +
					   this.outputFVInfFileName +
					   this.patternStr;
		} else {
			argument = this.inputFVInfFileName + 
				       this.outputFVInfFileName +
				       this.patternStr;
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
            EdkLog.log(EdkLog.EDK_INFO, Commandline.toString(cmdline
                    .getCommandline()));

            revl = runner.execute();

            if (EFI_SUCCESS == revl) {
                //
                // command execution success
                //
                EdkLog.log(EdkLog.EDK_INFO, "ModifyInfTask succeeded!");
            } else {
                //
                // command execution fail
                //
                EdkLog.log(EdkLog.EDK_ERROR, "ModifyInfTask failed. (error="
                        + Integer.toHexString(revl) + ")");
                throw new BuildException("ModifyInfTask failed. (error="
                        + Integer.toHexString(revl) + ")");

            }
        } catch (Exception e) {
            throw new BuildException(e.getMessage());
        }
    }

    /**
     * getinputFVInfFileName
     * 
     * This function is to get class member "inputFVInfFileName".
     * 
     * @return string of input inf file name.
     */
    public String getinputFVInfFileName() {
        return this.inputFVInfFileName;
    }

    /**
     * setinputFVInfFileName
     * 
     * This function is to set class member "inputFVInfFileName".
     * 
     * @param inputFile
     *            string of input inf file name.
     */
    public void setinputFVInfFileName(String inputFVInfFileName) {
        this.inputFVInfFileName = inputFVInfFileName + " ";
    }

    /**
     * getoutputFVInfFileName
     * 
     * This function is to get class member "outputFVInfFileName"
     * 
     * @return outputFVInfFileName string of output inf file name.
     */
    public String getoutputFVInfFileName() {
        return this.outputFVInfFileName;
    }

    /**
     * setoutputFVInfFileName
     * 
     * This function is to set class member "outputFVInfFileName"
     * 
     * @param outputFVInfFileName
     *            string of output  inf file name.
     */
    public void setoutputFVInfFileName(String outputFVInfFileName) {
        this.outputFVInfFileName = outputFVInfFileName  + " ";
    }

    /**
     * getpatternStr
     * 
     * This function is to get class member "patternStr"
     * 
     * @return patternStr string of pattern.
     */
    public String getpatternStr() {
        return this.patternStr;
    }

    /**
     * setpatternStr
     * 
     * This function is to set class member "patternStr"
     * 
     * @param patternStr
     *            string of patternStr.
     */
    public void setpatternStr(String patternStr) {
        this.patternStr = patternStr;
    }

	/**
     * getoutputDir
     * 
     * This function is to get class member "outputDir"
     * 
     * @return outputDir string of output directory.
     */
    public String getoutputDir() {
        return this.outputDir;
    }

    /**
     * setoutputDir
     * 
     * This function is to set class member "outputDir"
     * 
     * @param patternStr
     *            string of output directory.
     */
    public void setoutputDir(String outputDir) {
        this.outputDir = outputDir;
    }
}
