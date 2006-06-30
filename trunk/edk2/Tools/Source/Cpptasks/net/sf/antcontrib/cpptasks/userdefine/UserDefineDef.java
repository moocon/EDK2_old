/*
 * 
 * Copyright 2002-2006 The Ant-Contrib project
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 */
package net.sf.antcontrib.cpptasks.userdefine;

import java.io.BufferedReader;
import java.io.File;
import java.io.FileReader;
import java.util.Iterator;
import java.util.LinkedHashSet;
import java.util.Set;
import java.util.Vector;

import org.apache.tools.ant.BuildException;
import org.apache.tools.ant.Project;

import net.sf.antcontrib.cpptasks.ProcessorDef;
import net.sf.antcontrib.cpptasks.types.ConditionalPath;
import net.sf.antcontrib.cpptasks.types.IncludePath;
import net.sf.antcontrib.cpptasks.types.LibrarySet;

public class UserDefineDef extends ProcessorDef{
    
    public UserDefineDef () {}
    
    private String type = "CC";
    
    private String family = "MSFT";
    
    private String cmd;
    
    private String includepathDelimiter;
    
    private String outputDelimiter;
    
    private File workdir;
    
    private Vector includePaths= new Vector();
    
    private String outputFile;
    
    private Vector _libset = new Vector();
    
    public void addLibset(LibrarySet libset) {
        if (isReference()) {
            throw noChildrenAllowed();
        }
        if (libset == null) {
            throw new NullPointerException("libset");
        }
        
        _libset.add(libset);
    }
    
    public void execute() throws org.apache.tools.ant.BuildException {
        throw new org.apache.tools.ant.BuildException(
                        "Not an actual task, but looks like one for documentation purposes");
    }


    public void addConfiguredArgument(UserDefineArgument arg) {
        if (isReference()) {
            throw noChildrenAllowed();
        }
        addConfiguredProcessorArg(arg);
    }
    
    /**
     * Creates an include path.
     */
    public IncludePath createIncludePath() {
        Project p = getProject();
        if (p == null) {
            throw new java.lang.IllegalStateException("project must be set");
        }
        if (isReference()) {
            throw noChildrenAllowed();
        }
        IncludePath path = new IncludePath(p);
        includePaths.addElement(path);
        return path;
    }
    
    
    /**
     * Add a <includepath> if specify the file attribute
     * 
     * @throws BuildException
     *             if the specify file not exist
     */
    protected void loadFile(Vector activePath, File file) throws BuildException {
        FileReader fileReader;
        BufferedReader in;
        String str;
        if (!file.exists()) {
            throw new BuildException("The file " + file + " is not existed");
        }
        try {
            fileReader = new FileReader(file);
            in = new BufferedReader(fileReader);
            while ((str = in.readLine()) != null) {
                if (str.trim() == "") {
                    continue;
                }
                str = getProject().replaceProperties(str);
                activePath.addElement(str.trim());
            }
        } catch (Exception e) {
            throw new BuildException(e.getMessage());
        }
    }
    
    /**
     * Returns the specific include path.
     */
    public String[] getActiveIncludePaths() {
        if (isReference()) {
            return ((UserDefineDef) getCheckedRef(UserDefineDef.class,
                            "UserDefineDef")).getActiveIncludePaths();
        }
        return getActivePaths(includePaths);
    }
    
    private String[] getActivePaths(Vector paths) {
        Project p = getProject();
        if (p == null) {
            throw new java.lang.IllegalStateException("project not set");
        }
        Vector activePaths = new Vector(paths.size());
        for (int i = 0; i < paths.size(); i++) {
            ConditionalPath path = (ConditionalPath) paths.elementAt(i);
            if (path.isActive(p)) {
                if (path.getFile() == null) {
                    String[] pathEntries = path.list();
                    for (int j = 0; j < pathEntries.length; j++) {
                        activePaths.addElement(pathEntries[j]);
                    }
                } else {
                    loadFile(activePaths, path.getFile());
                }
            }
        }
        String[] pathNames = new String[activePaths.size()];
        activePaths.copyInto(pathNames);
        return pathNames;
    }
    
    public String getIncludepathDelimiter() {
        if (isReference()) {
            return ((UserDefineDef) getCheckedRef(UserDefineDef.class,
                            "UserDefineDef")).getIncludepathDelimiter();
        }
        return includepathDelimiter;
    }

    public void setIncludepathDelimiter(String includepathDelimiter) {
        if (isReference()) {
            throw tooManyAttributes();
        }
        this.includepathDelimiter = includepathDelimiter;
    }

    public String getType() {
        if (isReference()) {
            return ((UserDefineDef) getCheckedRef(UserDefineDef.class,
                            "UserDefineDef")).getType();
        }
        return type;
    }

    public void setType(String type) {
        if (isReference()) {
            throw tooManyAttributes();
        }
        this.type = type;
    }

    public String getCmd() {
        return cmd;
    }

    public void setCmd(String cmd) {
        if (isReference()) {
            throw tooManyAttributes();
        }
        this.cmd = cmd;
    }

    public String getFamily() {
        return family;
    }

    public void setFamily(String family) {
        if (isReference()) {
            throw tooManyAttributes();
        }
        this.family = family;
    }

    public String getOutputFile() {
        return outputFile;
    }

    public void setOutputFile(String outputFile) {
        if (isReference()) {
            throw tooManyAttributes();
        }
        this.outputFile = outputFile;
    }

    public File getWorkdir() {
        return workdir;
    }

    public void setWorkdir(File workdir) {
        if (isReference()) {
            throw tooManyAttributes();
        }
        this.workdir = workdir;
    }

    public String[] get_libset() {
        Set libs = new LinkedHashSet();
        Iterator iter = _libset.iterator();
        while (iter.hasNext()) {
            LibrarySet librarySet = (LibrarySet)iter.next();
            File basedir = librarySet.getDir(getProject());
            String[] libStrArray = librarySet.getLibs();
            for (int i = 0 ; i < libStrArray.length; i ++) {
                if (basedir != null) {
                    File libFile = new File(libStrArray[i]);
                    if (libFile.isAbsolute()) {
                        libs.add(libFile.getPath());
                    }
                    else {
                        libs.add(basedir.getPath() + File.separatorChar + libFile.getPath());
                    }
                }
                else {
                    libs.add(libStrArray[i]);
                }
            }
        }
        return (String[])libs.toArray(new String[libs.size()]);
    }

    public String getOutputDelimiter() {
        return outputDelimiter;
    }

    public void setOutputDelimiter(String outputDelimiter) {
        this.outputDelimiter = outputDelimiter;
    }

}
