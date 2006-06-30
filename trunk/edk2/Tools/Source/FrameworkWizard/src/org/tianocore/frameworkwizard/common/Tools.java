/** @file
 
 The file is used to provides some useful interfaces 
 
 Copyright (c) 2006, Intel Corporation
 All rights reserved. This program and the accompanying materials
 are licensed and made available under the terms and conditions of the BSD License
 which accompanies this distribution.  The full text of the license may be found at
 http://opensource.org/licenses/bsd-license.php
 
 THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
 WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.
 
 **/

package org.tianocore.frameworkwizard.common;

import java.io.File;
import java.text.SimpleDateFormat;
import java.util.Date;
import java.util.List;
import java.util.UUID;
import java.util.Vector;

import javax.swing.JComboBox;
import javax.swing.JOptionPane;

/**
 The class is used to provides some useful interfaces  
 
 **/
public class Tools {

    //
    // The dir user selected to create new package in
    //
    public static String dirForNewSpd = null;

    /**
     Used for test
     
     @param args
     
     **/
    public static void main(String[] args) {
        System.out.println(getCurrentDateTime());
    }

    /**
     Get current date and time and format it as "yyyy-MM-dd HH:mm"
     
     @return formatted current date and time
     
     **/
    public static String getCurrentDateTime() {
        Date now = new Date(System.currentTimeMillis());
        SimpleDateFormat sdf = new SimpleDateFormat("yyyy-MM-dd HH:mm");
        return sdf.format(now);
    }

    /**
     Generate a UUID
     
     @return the created UUID
     
     **/
    public static String generateUuidString() {
        return UUID.randomUUID().toString();
    }

    /**
     Use current file separator in the path
     
     @param strPath
     @return
     
     **/
    public static String convertPathToCurrentOsType(String strPath) {
        strPath = strPath.replace(DataType.DOS_FILE_SEPARATOR, DataType.FILE_SEPARATOR);
        strPath = strPath.replace(DataType.UNIX_FILE_SEPARATOR, DataType.FILE_SEPARATOR);
        return strPath;
    }

    /**
     Use Unix file separator in the path
     
     @param strPath
     @return
     
     **/
    public static String convertPathToUnixType(String strPath) {
        strPath = strPath.replace(DataType.DOS_FILE_SEPARATOR, DataType.UNIX_FILE_SEPARATOR);
        return strPath;
    }

    /**
     Use Dos file separator in the path
     
     @param strPath
     @return
     
     **/
    public static String convertPathToDosType(String strPath) {
        strPath = strPath.replace(DataType.UNIX_FILE_SEPARATOR, DataType.DOS_FILE_SEPARATOR);
        return strPath;
    }

    /**
     Get all system properties and output to the console
     
     **/
    public static void getSystemProperties() {
        System.out.println(System.getProperty("java.class.version"));
        System.out.println(System.getProperty("java.class.path"));
        System.out.println(System.getProperty("java.ext.dirs"));
        System.out.println(System.getProperty("os.name"));
        System.out.println(System.getProperty("os.arch"));
        System.out.println(System.getProperty("os.version"));
        System.out.println(System.getProperty("file.separator"));
        System.out.println(System.getProperty("path.separator"));
        System.out.println(System.getProperty("line.separator"));
        System.out.println(System.getProperty("user.name"));
        System.out.println(System.getProperty("user.home"));
        System.out.println(System.getProperty("user.dir"));
        System.out.println(System.getProperty("PATH"));

        System.out.println(System.getenv("PROCESSOR_REVISION"));
    }

    /**
     Generate selection items for JComboBox by input vector
     
     **/
    public static void generateComboBoxByVector(JComboBox jcb, Vector<String> vector) {
        if (jcb != null) {
            jcb.removeAllItems();
        }
        if (vector != null) {
            for (int index = 0; index < vector.size(); index++) {
                jcb.addItem(vector.elementAt(index));
            }
        }
    }
    
    /**
     Get path only from a path
    
     @param filePath
     @return
    
    **/
    public static String getFilePathOnly(String filePath) {
        String path = filePath.substring(0, filePath.length() - getFileNameOnly(filePath).length());
        if (path.endsWith(DataType.FILE_SEPARATOR)) {
            path = path.substring(0, path.length() - DataType.FILE_SEPARATOR.length());
        }
        
        return path;
    }
    
    
    /**
     Get file name from a path
     
     @param filePath
     @return
    
    **/
    public static String getFileNameOnly(String filePath) {
        File f = new File(filePath);
        return f.getAbsoluteFile().getName();
    }
    
    public static String getFileNameWithoutExt(String filePath) {
        filePath = getFileNameOnly(filePath);
        filePath = filePath.substring(0, filePath.lastIndexOf(DataType.FILE_EXT_SEPARATOR));
        return filePath;
    }

    /**
     Get relative path
     
     @param wholePath
     @param commonPath
     @return wholePath - commonPath 
     
     **/
    public static String getRelativePath(String wholePath, String commonPath) {
        String path = "";
        int i = 0;
        i = wholePath.indexOf(commonPath);
        if (i > -1) {
            i = i + commonPath.length();
        } else {
            return "";
        }
        path = wholePath.substring(i);
        //
        // remove file separator of head
        //
        if (path.indexOf(DataType.DOS_FILE_SEPARATOR) == 0) {
            path = path.substring(0 + DataType.DOS_FILE_SEPARATOR.length());
        }
        if (path.indexOf(DataType.UNIX_FILE_SEPARATOR) == 0) {
            path = path.substring(0 + DataType.DOS_FILE_SEPARATOR.length());
        }
        //
        // remove file separator of rear
        //
        if (path.indexOf(DataType.DOS_FILE_SEPARATOR) == path.length() - DataType.DOS_FILE_SEPARATOR.length()) {
            path = path.substring(0, path.length() - DataType.DOS_FILE_SEPARATOR.length());
        }
        if (path.indexOf(DataType.UNIX_FILE_SEPARATOR) == path.length() - DataType.UNIX_FILE_SEPARATOR.length()) {
            path = path.substring(0, path.length() - DataType.DOS_FILE_SEPARATOR.length());
        }
        //
        // convert to UNIX format
        //
        path = Tools.convertPathToUnixType(path);
        return path;
    }

    /**
     Convert List ot Vector
     
     @param list
     @return
     
     **/
    public static Vector<String> convertListToVector(List list) {
        Vector<String> v = new Vector<String>();
        if (list != null && list.size() > 0) {
            for (int index = 0; index < list.size(); index++) {
                v.addElement(list.get(index).toString());
            }
        }
        return v;
    }

    /**
     If the input path missing ext, append the ext to the path
     
     @param path
     @param type
     @return
     
     **/
    public static String addPathExt(String path, int type) {
        String match = "";
        if (type == DataType.RETURN_TYPE_MODULE_SURFACE_AREA) {
            match = DataType.FILE_EXT_SEPARATOR + DataType.MODULE_SURFACE_AREA_EXT;
        }
        if (type == DataType.RETURN_TYPE_PACKAGE_SURFACE_AREA) {
            match = DataType.FILE_EXT_SEPARATOR + DataType.PACKAGE_SURFACE_AREA_EXT;
        }
        if (type == DataType.RETURN_TYPE_PLATFORM_SURFACE_AREA) {
            match = DataType.FILE_EXT_SEPARATOR + DataType.PLATFORM_SURFACE_AREA_EXT;
        }
        if (path.length() <= match.length()) {
            path = path + match;
            return path;
        }
        if (!(path.substring(path.length() - match.length())).equals(match)) {
            path = path + match;
        }
        return path;
    }
    
    /**
     Show a message box
     
     @param arg0
    
    **/
    public static void showInformationMessage(String arg0) {
        JOptionPane.showConfirmDialog(null, arg0, "Error", JOptionPane.DEFAULT_OPTION, JOptionPane.INFORMATION_MESSAGE);
    }
}
