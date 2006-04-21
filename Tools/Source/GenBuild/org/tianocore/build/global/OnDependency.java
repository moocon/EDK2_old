/** @file
This file is to define OnDependency class.

Copyright (c) 2006, Intel Corporation
All rights reserved. This program and the accompanying materials
are licensed and made available under the terms and conditions of the BSD License
which accompanies this distribution.  The full text of the license may be found at
http://opensource.org/licenses/bsd-license.php

THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

--*/
package org.tianocore.build.global;

import org.apache.tools.ant.BuildException;
import org.apache.tools.ant.Task;
import org.apache.tools.ant.taskdefs.Sequential;

import java.io.File;
import java.util.Iterator;

/**
 Class OnDepdendency is used to check the timestamp between source files and
 target files, which can be used to determine if the target files are needed to
 be re-generated from source files.
 **/
public class OnDependency extends Task {
    ///
    /// source files list
    ///
    private DpFileList sources = null;
    ///
    /// target files list
    ///
    private DpFileList targets = null;
    ///
    /// tasks to be performed to generate target files
    ///
    private Sequential  task = null;

    ///
    /// An empty constructor for an ANT task can avoid some potential issues
    ///
    public OnDependency(){
    }

    /**
     Standard execute method of ANT task
     **/
    public void execute() {
        if (isOutOfDate() && task != null) {
            task.perform();
        }
    }

    ///
    /// check if the target files are outofdate
    ///
    private boolean isOutOfDate() {
        ///
        /// if no source files specified, take it as a fresh start
        ///
        if (sources.nameList.size() == 0) {
            return true;
        }

        Iterator dstIt = targets.nameList.iterator();
        while (dstIt.hasNext()) {
            String dstFileName = (String)dstIt.next();
            File dstFile = new File(dstFileName);
            if (!dstFile.exists()) {
                return true;
            }

            long dstTimeStamp = dstFile.lastModified();
            Iterator srcIt = sources.nameList.iterator();
            while (srcIt.hasNext()) {
                String srcFileName = (String)srcIt.next();
                File srcFile = new File(srcFileName);
                if (!srcFile.exists()) {
                    throw new BuildException(srcFileName + " doesn't exist !!!");
                }

                if (dstTimeStamp < srcFile.lastModified()) {
                    return true;
                }
            }
        }

        return false;
    }

    /**
     Add method of ANT task for nested element with Sequential type

     @param     task    Sequential object which contains tasks for generating target files
     **/
    public void addSequential(Sequential task) {
        this.task = task;
    }

    /**
     Add method of ANT task for nested element with DpFileList type

     @param     sources DpFileList object which contains the list of source files
     **/
    public void addSourcefiles(DpFileList sources) {
        this.sources = sources;
    }

    /**
     Add method of ANT task for nested element with DpFileList type

     @param     targets DpFileList object which contains the list of target files
     **/
    public void addTargetfiles(DpFileList targets) {
        this.targets = targets;
    }
}

