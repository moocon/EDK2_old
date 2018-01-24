/** @file
 
 The file is used to create, update SourceFile of MSA/MBD file
 
 Copyright (c) 2006, Intel Corporation
 All rights reserved. This program and the accompanying materials
 are licensed and made available under the terms and conditions of the BSD License
 which accompanies this distribution.  The full text of the license may be found at
 http://opensource.org/licenses/bsd-license.php
 
 THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
 WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.
 
 **/
package org.tianocore.frameworkwizard.module.ui.dialog;

import java.awt.event.ActionEvent;
import java.io.File;
import java.util.Vector;

import javax.swing.JButton;
import javax.swing.JFileChooser;
import javax.swing.JLabel;
import javax.swing.JPanel;
import javax.swing.JScrollPane;
import javax.swing.JTextField;

import org.tianocore.frameworkwizard.common.DataType;
import org.tianocore.frameworkwizard.common.DataValidation;
import org.tianocore.frameworkwizard.common.Log;
import org.tianocore.frameworkwizard.common.Tools;
import org.tianocore.frameworkwizard.common.ui.ArchCheckBox;
import org.tianocore.frameworkwizard.common.ui.IDialog;
import org.tianocore.frameworkwizard.common.ui.IFrame;
import org.tianocore.frameworkwizard.common.ui.StarLabel;
import org.tianocore.frameworkwizard.module.Identifications.SourceFiles.SourceFilesIdentification;

/**
 The class is used to create, update SourceFile of MSA/MBD file
 It extends IInternalFrame
 


 **/
public class SourceFilesDlg extends IDialog {

    ///
    /// Define class Serial Version UID
    ///
    private static final long serialVersionUID = -6765742852142775378L;

    //
    // Define class members
    //
    private JPanel jContentPane = null;

    private JLabel jLabelFileName = null;

    private JTextField jTextFieldFileName = null;

    private JButton jButtonOpenFile = null;

    private JLabel jLabelToolChainFamily = null;

    private StarLabel jStarLabel1 = null;

    private JLabel jLabelArch = null;

    private JScrollPane jScrollPane = null;

    private JLabel jLabelTagName = null;

    private JTextField jTextFieldTagName = null;

    private JLabel jLabelToolCode = null;

    private JTextField jTextFieldToolCode = null;

    private JTextField jTextFieldToolChainFamily = null;

    private JLabel jLabelFeatureFlag = null;

    private JTextField jTextFieldFeatureFlag = null;

    private ArchCheckBox jArchCheckBox = null;
    
    private JButton jButtonOk = null;

    private JButton jButtonCancel = null;

    //
    // Not used by UI
    //
    private SourceFilesIdentification sfid = null;

    private String msaFileName = "";

    /**
     This method initializes jTextFieldFileName 
     
     @return javax.swing.JTextField jTextFieldFileName
     
     **/
    private JTextField getJTextFieldSourceFilesDirectory() {
        if (jTextFieldFileName == null) {
            jTextFieldFileName = new JTextField();
            jTextFieldFileName.setBounds(new java.awt.Rectangle(140, 10, 250, 20));
            jTextFieldFileName.setPreferredSize(new java.awt.Dimension(250, 20));
            jTextFieldFileName.setToolTipText("Path is relative to the MSA file and must include the file name");
        }
        return jTextFieldFileName;
    }

    /**
     This method initializes jButtonOpenFile 
     
     @return javax.swing.JButton jButtonOpenFile
     
     **/
    private JButton getJButtonOpenFile() {
        if (jButtonOpenFile == null) {
            jButtonOpenFile = new JButton();
            jButtonOpenFile.setText("Browse");
            jButtonOpenFile.setBounds(new java.awt.Rectangle(395, 10, 85, 20));
            jButtonOpenFile.setPreferredSize(new java.awt.Dimension(85, 20));
            jButtonOpenFile.addActionListener(this);
        }
        return jButtonOpenFile;
    }

    /**
     This method initializes jScrollPane  
     
     @return javax.swing.JScrollPane  
     */
    private JScrollPane getJScrollPane() {
        if (jScrollPane == null) {
            jScrollPane = new JScrollPane();
            jScrollPane.setViewportView(getJContentPane());
        }
        return jScrollPane;
    }

    /**
     * This method initializes jTextFieldTagName	
     * 	
     * @return javax.swing.JTextField	
     */
    private JTextField getJTextFieldTagName() {
        if (jTextFieldTagName == null) {
            jTextFieldTagName = new JTextField();
            jTextFieldTagName.setBounds(new java.awt.Rectangle(140, 35, 340, 20));
            jTextFieldTagName.setPreferredSize(new java.awt.Dimension(340, 20));
            jTextFieldTagName.setToolTipText("You may specify a specific tool chain tag name, such as BILL1");
        }
        return jTextFieldTagName;
    }

    /**
     * This method initializes jTextFieldToolCode	
     * 	
     * @return javax.swing.JTextField	
     */
    private JTextField getJTextFieldToolCode() {
        if (jTextFieldToolCode == null) {
            jTextFieldToolCode = new JTextField();
            jTextFieldToolCode.setBounds(new java.awt.Rectangle(140, 60, 340, 20));
            jTextFieldToolCode.setPreferredSize(new java.awt.Dimension(340, 20));
            jTextFieldToolCode.setToolTipText("You may specify a specific tool command, such as ASM");
        }
        return jTextFieldToolCode;
    }

    /**
     * This method initializes jTextFieldToolChainFamily	
     * 	
     * @return javax.swing.JTextField	
     */
    private JTextField getJTextFieldToolChainFamily() {
        if (jTextFieldToolChainFamily == null) {
            jTextFieldToolChainFamily = new JTextField();
            jTextFieldToolChainFamily.setBounds(new java.awt.Rectangle(140, 85, 340, 20));
            jTextFieldToolChainFamily.setPreferredSize(new java.awt.Dimension(340, 20));
            jTextFieldToolChainFamily.setToolTipText("You may specify a specific tool chain family, such as GCC");
        }
        return jTextFieldToolChainFamily;
    }

    /**
     * This method initializes jTextFieldFeatureFlag	
     * 	
     * @return javax.swing.JTextField	
     */
    private JTextField getJTextFieldFeatureFlag() {
        if (jTextFieldFeatureFlag == null) {
            jTextFieldFeatureFlag = new JTextField();
            jTextFieldFeatureFlag.setBounds(new java.awt.Rectangle(140, 110, 340, 20));
            jTextFieldFeatureFlag.setPreferredSize(new java.awt.Dimension(340, 20));
            jTextFieldFeatureFlag.setToolTipText("RESERVED FOR FUTURE USE");
        }
        return jTextFieldFeatureFlag;
    }

    /**
     This method initializes jButtonOk    
     
     @return javax.swing.JButton  
     
     **/
    private JButton getJButtonOk() {
        if (jButtonOk == null) {
            jButtonOk = new JButton();
            jButtonOk.setBounds(new java.awt.Rectangle(290, 165, 90, 20));
            jButtonOk.setText("Ok");
            jButtonOk.addActionListener(this);
        }
        return jButtonOk;
    }

    /**
     This method initializes jButtonCancel    
     
     @return javax.swing.JButton  
     
     **/
    private JButton getJButtonCancel() {
        if (jButtonCancel == null) {
            jButtonCancel = new JButton();
            jButtonCancel.setBounds(new java.awt.Rectangle(390, 165, 90, 20));
            jButtonCancel.setText("Cancel");
            jButtonCancel.addActionListener(this);
        }
        return jButtonCancel;
    }

    public static void main(String[] args) {

    }

    /**
     This is the default constructor
     
     **/
    public SourceFilesDlg(SourceFilesIdentification inSourceFilesIdentification, IFrame iFrame, String fileName) {
        super(iFrame, true);
        init(inSourceFilesIdentification, fileName);
    }

    /**
     This method initializes this
     
     **/
    private void init() {
        this.setSize(500, 235);
        this.setContentPane(getJScrollPane());
        this.setTitle("Source Files");
        this.setViewMode(false);
        this.centerWindow();
    }

    /**         
     This method initializes this
     Fill values to all fields if these values are not empty
     
     
     @param inSourceFiles The input data of SourceFilesDocument.SourceFiles
     
     **/
    private void init(SourceFilesIdentification inSourceFilesIdentifications, String fileName) {
        init();
        this.sfid = inSourceFilesIdentifications;
        this.msaFileName = fileName;

        if (this.sfid != null) {
            this.jTextFieldFileName.setText(sfid.getFilename());
            this.jTextFieldTagName.setText(sfid.getTagName());
            this.jTextFieldToolCode.setText(sfid.getToolCode());
            this.jTextFieldToolChainFamily.setText(sfid.getToolChainFamily());
            jTextFieldFeatureFlag.setText(sfid.getFeatureFlag());
            this.jArchCheckBox.setSelectedItems(sfid.getSupArchList());
        }
    }

    /**
     Disable all components when the mode is view
     
     @param isView true - The view mode; false - The non-view mode
     
     **/
    public void setViewMode(boolean isView) {
        if (isView) {
            this.jTextFieldFileName.setEnabled(!isView);
            this.jButtonOpenFile.setEnabled(!isView);
        }
    }

    /**
     This method initializes jContentPane
     
     @return javax.swing.JPanel jContentPane
     
     **/
    private JPanel getJContentPane() {
        if (jContentPane == null) {
            jArchCheckBox = new ArchCheckBox();
            jArchCheckBox.setBounds(new java.awt.Rectangle(140, 135, 340, 20));
            jArchCheckBox.setPreferredSize(new java.awt.Dimension(340, 20));
            jLabelFeatureFlag = new JLabel();
            jLabelFeatureFlag.setBounds(new java.awt.Rectangle(15, 110, 120, 20));
            jLabelFeatureFlag.setText("Feature Flag");
            jLabelToolCode = new JLabel();
            jLabelToolCode.setBounds(new java.awt.Rectangle(15, 60, 120, 20));
            jLabelToolCode.setText("Tool Code");
            jLabelTagName = new JLabel();
            jLabelTagName.setBounds(new java.awt.Rectangle(15, 35, 120, 20));
            jLabelTagName.setText("Tag Name");
            jLabelArch = new JLabel();
            jLabelArch.setBounds(new java.awt.Rectangle(15, 135, 120, 20));
            jLabelArch.setText("Sup Arch List");
            jLabelToolChainFamily = new JLabel();
            jLabelToolChainFamily.setBounds(new java.awt.Rectangle(15, 85, 120, 20));
            jLabelToolChainFamily.setText("Tool Chain Family");
            jLabelFileName = new JLabel();
            jLabelFileName.setText("File Name");
            jLabelFileName.setBounds(new java.awt.Rectangle(15, 10, 120, 20));

            jContentPane = new JPanel();
            jContentPane.setLayout(null);
            jContentPane.setPreferredSize(new java.awt.Dimension(490, 185));

            jContentPane.add(jLabelFileName, null);
            jContentPane.add(getJTextFieldSourceFilesDirectory(), null);
            jContentPane.add(getJButtonOpenFile(), null);
            jContentPane.add(jLabelToolChainFamily, null);
            jStarLabel1 = new StarLabel();
            jStarLabel1.setLocation(new java.awt.Point(0, 10));

            jContentPane.add(jStarLabel1, null);
            jContentPane.add(jLabelArch, null);
            jContentPane.add(jLabelTagName, null);
            jContentPane.add(getJTextFieldTagName(), null);
            jContentPane.add(jLabelToolCode, null);
            jContentPane.add(getJTextFieldToolCode(), null);
            jContentPane.add(getJTextFieldToolChainFamily(), null);
            jContentPane.add(jLabelFeatureFlag, null);
            jContentPane.add(getJTextFieldFeatureFlag(), null);
            jContentPane.add(jArchCheckBox, null);
            jContentPane.add(getJButtonOk(), null);
            jContentPane.add(getJButtonCancel(), null);
        }
        return jContentPane;
    }

    /* (non-Javadoc)
     * @see java.awt.event.ActionListener#actionPerformed(java.awt.event.ActionEvent)
     *
     * Override actionPerformed to listen all actions
     *  
     */
    public void actionPerformed(ActionEvent arg0) {
        if (arg0.getSource() == jButtonOpenFile) {
            selectFile();
        }

        if (arg0.getSource() == jButtonOk) {
            if (checkAdd()) {
                getCurrentSourceFiles();
                this.returnType = DataType.RETURN_TYPE_OK;
                this.setVisible(false);
            }
        }

        if (arg0.getSource() == jButtonCancel) {
            this.returnType = DataType.RETURN_TYPE_CANCEL;
            this.setVisible(false);
        }
    }

    private SourceFilesIdentification getCurrentSourceFiles() {
        String name = this.jTextFieldFileName.getText();
        String tagName = this.jTextFieldTagName.getText();
        String toolCode = this.jTextFieldToolCode.getText();
        String tcf = this.jTextFieldToolChainFamily.getText();
        String featureFlag = this.jTextFieldFeatureFlag.getText();
        Vector<String> arch = this.jArchCheckBox.getSelectedItemsVector();
        sfid = new SourceFilesIdentification(name, tagName, toolCode, tcf, featureFlag, arch);
        return sfid;
    }

    /**
     Data validation for all fields
     
     @retval true - All datas are valid
     @retval false - At least one data is invalid
     
     **/
    public boolean checkAdd() {
        //
        // Check Filename
        //
        if (isEmpty(this.jTextFieldFileName.getText())) {
            Log.err("File Name couldn't be empty");
            return false;
        }
        if (!DataValidation.isFilename(this.jTextFieldFileName.getText())) {
            Log.err("Incorrect data type for File Name");
            return false;
        }

        //
        // Check TagName 
        //
        if (!isEmpty(this.jTextFieldTagName.getText())) {
            if (!DataValidation.isTagName(this.jTextFieldTagName.getText())) {
                Log.err("Incorrect data type for Tag Name");
                return false;
            }
        }

        //
        // Check ToolCode 
        //
        if (!isEmpty(this.jTextFieldToolCode.getText())) {
            if (!DataValidation.isToolCode(this.jTextFieldToolCode.getText())) {
                Log.err("Incorrect data type for Tool Code");
                return false;
            }
        }

        //
        // Check ToolChainFamily 
        //
        if (!isEmpty(this.jTextFieldToolChainFamily.getText())) {
            if (!DataValidation.isToolChainFamily(this.jTextFieldToolChainFamily.getText())) {
                Log.err("Incorrect data type for Tool Chain Family");
                return false;
            }
        }

        //
        // Check FeatureFlag
        //
        if (!isEmpty(this.jTextFieldFeatureFlag.getText())) {
            if (!DataValidation.isFeatureFlag(this.jTextFieldFeatureFlag.getText())) {
                Log.err("Incorrect data type for Feature Flag");
                return false;
            }
        }

        return true;
    }

    /**
     Display a file open browser to let user select file
     
     **/
    private void selectFile() {
        JFileChooser fc = new JFileChooser();
        fc.setCurrentDirectory(new File(Tools.getFilePathOnly(msaFileName)));
        int result = fc.showOpenDialog(new JPanel());
        if (result == JFileChooser.APPROVE_OPTION) {
            this.jTextFieldFileName.setText(fc.getSelectedFile().getName());
        }
    }

    public SourceFilesIdentification getSfid() {
        return sfid;
    }

    public void setSfid(SourceFilesIdentification sfid) {
        this.sfid = sfid;
    }
}