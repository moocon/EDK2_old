/** @file
 
 The file is used to create, update MsaHeader of MSA file
 
 Copyright (c) 2006, Intel Corporation
 All rights reserved. This program and the accompanying materials
 are licensed and made available under the terms and conditions of the BSD License
 which accompanies this distribution.  The full text of the license may be found at
 http://opensource.org/licenses/bsd-license.php
 
 THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
 WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.
 
 **/

package org.tianocore.frameworkwizard.module.ui;

import java.awt.event.ActionEvent;
import java.awt.event.ComponentEvent;

import javax.swing.JButton;
import javax.swing.JComboBox;
import javax.swing.JLabel;
import javax.swing.JPanel;
import javax.swing.JScrollPane;
import javax.swing.JTextArea;
import javax.swing.JTextField;

import org.tianocore.FrameworkComponentTypes;
import org.tianocore.LicenseDocument;
import org.tianocore.ModuleTypeDef;
import org.tianocore.MsaHeaderDocument;
import org.tianocore.SpecificationDocument;
import org.tianocore.frameworkwizard.common.DataType;
import org.tianocore.frameworkwizard.common.DataValidation;
import org.tianocore.frameworkwizard.common.Log;
import org.tianocore.frameworkwizard.common.Tools;
import org.tianocore.frameworkwizard.common.ui.IInternalFrame;
import org.tianocore.frameworkwizard.common.ui.StarLabel;

/**
 The class is used to create, update MsaHeader of MSA file
 It extends IInternalFrame
 
 @since ModuleEditor 1.0

 **/
public class MsaHeader extends IInternalFrame {

    ///
    /// Define class Serial Version UID
    ///
    private static final long serialVersionUID = -8152099582923006900L;

    //
    //Define class members
    //
    private JPanel jContentPane = null;

    private JLabel jLabelBaseName = null;

    private JTextField jTextFieldBaseName = null;

    private JLabel jLabelGuid = null;

    private JTextField jTextFieldGuid = null;

    private JLabel jLabelVersion = null;

    private JTextField jTextFieldVersion = null;

    private JButton jButtonGenerateGuid = null;

    private JLabel jLabelLicense = null;

    private JTextArea jTextAreaLicense = null;

    private JLabel jLabelCopyright = null;

    private JLabel jLabelDescription = null;

    private JTextArea jTextAreaDescription = null;

    private JLabel jLabelSpecification = null;

    private JTextField jTextFieldSpecification = null;

    private JButton jButtonOk = null;

    private JButton jButtonCancel = null;

    private JScrollPane jScrollPaneLicense = null;

    private JScrollPane jScrollPaneDescription = null;

    private JLabel jLabelAbstract = null;

    private JTextField jTextFieldAbstract = null;

    private JLabel jLabelModuleType = null;

    private JLabel jLabelCompontentType = null;

    private JComboBox jComboBoxCompontentType = null;

    private JComboBox jComboBoxModuleType = null;

    private StarLabel jStarLabel1 = null;

    private StarLabel jStarLabel2 = null;

    private StarLabel jStarLabel3 = null;

    private StarLabel jStarLabel4 = null;

    private StarLabel jStarLabel5 = null;

    private StarLabel jStarLabel6 = null;

    private StarLabel jStarLabel7 = null;

    private StarLabel jStarLabel8 = null;

    private StarLabel jStarLabel9 = null;

    private MsaHeaderDocument.MsaHeader msaHeader = null;

    private JTextField jTextFieldCopyright = null;

    /**
     This method initializes jTextFieldBaseName 
     
     @return javax.swing.JTextField jTextFieldBaseName
     
     **/
    private JTextField getJTextFieldBaseName() {
        if (jTextFieldBaseName == null) {
            jTextFieldBaseName = new JTextField();
            jTextFieldBaseName.setBounds(new java.awt.Rectangle(160, 10, 320, 20));
            jTextFieldBaseName.setPreferredSize(new java.awt.Dimension(320,20));
        }
        return jTextFieldBaseName;
    }

    /**
     This method initializes jTextFieldGuid 
     
     @return javax.swing.JTextField jTextFieldGuid
     
     **/
    private JTextField getJTextFieldGuid() {
        if (jTextFieldGuid == null) {
            jTextFieldGuid = new JTextField();
            jTextFieldGuid.setBounds(new java.awt.Rectangle(160, 35, 250, 20));
            jTextFieldGuid.setPreferredSize(new java.awt.Dimension(250,20));
        }
        return jTextFieldGuid;
    }

    /**
     This method initializes jTextFieldVersion 
     
     @return javax.swing.JTextField jTextFieldVersion
     
     **/
    private JTextField getJTextFieldVersion() {
        if (jTextFieldVersion == null) {
            jTextFieldVersion = new JTextField();
            jTextFieldVersion.setBounds(new java.awt.Rectangle(160, 60, 320, 20));
            jTextFieldVersion.setPreferredSize(new java.awt.Dimension(320,20));
        }
        return jTextFieldVersion;
    }

    /**
     This method initializes jButtonGenerateGuid 
     
     @return javax.swing.JButton jButtonGenerateGuid
     
     **/
    private JButton getJButtonGenerateGuid() {
        if (jButtonGenerateGuid == null) {
            jButtonGenerateGuid = new JButton();
            jButtonGenerateGuid.setBounds(new java.awt.Rectangle(415, 35, 65, 20));
            jButtonGenerateGuid.setText("GEN");
            jButtonGenerateGuid.addActionListener(this);
        }
        return jButtonGenerateGuid;
    }

    /**
     This method initializes jTextAreaLicense 
     
     @return javax.swing.JTextArea jTextAreaLicense
     
     **/
    private JTextArea getJTextAreaLicense() {
        if (jTextAreaLicense == null) {
            jTextAreaLicense = new JTextArea();
            jTextAreaLicense.setText("");
            jTextAreaLicense.setPreferredSize(new java.awt.Dimension(317,77));
            jTextAreaLicense.setLineWrap(true);
        }
        return jTextAreaLicense;
    }

    /**
     This method initializes jTextAreaDescription 
     
     @return javax.swing.JTextArea jTextAreaDescription
     
     **/
    private JTextArea getJTextAreaDescription() {
        if (jTextAreaDescription == null) {
            jTextAreaDescription = new JTextArea();
            jTextAreaDescription.setLineWrap(true);
            jTextAreaDescription.setPreferredSize(new java.awt.Dimension(317,77));
        }
        return jTextAreaDescription;
    }

    /**
     This method initializes jTextFieldSpecification 
     
     @return javax.swing.JTextField jTextFieldSpecification
     
     **/
    private JTextField getJTextFieldSpecification() {
        if (jTextFieldSpecification == null) {
            jTextFieldSpecification = new JTextField();
            jTextFieldSpecification.setBounds(new java.awt.Rectangle(160, 280, 320, 20));
            jTextFieldSpecification.setPreferredSize(new java.awt.Dimension(320,20));
        }
        return jTextFieldSpecification;
    }

    /**
     This method initializes jButtonOk 
     
     @return javax.swing.JButton jButtonOk
     
     **/
    private JButton getJButtonOk() {
        if (jButtonOk == null) {
            jButtonOk = new JButton();
            jButtonOk.setText("OK");
            jButtonOk.setBounds(new java.awt.Rectangle(290, 445, 90, 20));
            jButtonOk.addActionListener(this);
        }
        return jButtonOk;
    }

    /**
     This method initializes jButtonCancel 
     
     @return javax.swing.JButton jButtonCancel
     
     **/
    private JButton getJButtonCancel() {
        if (jButtonCancel == null) {
            jButtonCancel = new JButton();
            jButtonCancel.setText("Cancel");
            jButtonCancel.setBounds(new java.awt.Rectangle(390, 445, 90, 20));
            jButtonCancel.addActionListener(this);
        }
        return jButtonCancel;
    }

    /**
     This method initializes jScrollPaneLicense 
     
     @return javax.swing.JScrollPane jScrollPaneLicense
     
     **/
    private JScrollPane getJScrollPaneLicense() {
        if (jScrollPaneLicense == null) {
            jScrollPaneLicense = new JScrollPane();
            jScrollPaneLicense.setBounds(new java.awt.Rectangle(160, 85, 320, 80));
            jScrollPaneLicense.setHorizontalScrollBarPolicy(javax.swing.JScrollPane.HORIZONTAL_SCROLLBAR_NEVER);
            jScrollPaneLicense.setPreferredSize(new java.awt.Dimension(320,80));
            jScrollPaneLicense.setViewportView(getJTextAreaLicense());
        }
        return jScrollPaneLicense;
    }

    /**
     This method initializes jScrollPaneDescription 
     
     @return javax.swing.JScrollPane jScrollPaneDescription
     
     **/
    private JScrollPane getJScrollPaneDescription() {
        if (jScrollPaneDescription == null) {
            jScrollPaneDescription = new JScrollPane();
            jScrollPaneDescription.setBounds(new java.awt.Rectangle(160, 195, 320, 80));
            jScrollPaneDescription.setHorizontalScrollBarPolicy(javax.swing.JScrollPane.HORIZONTAL_SCROLLBAR_NEVER);
            jScrollPaneDescription.setViewportView(getJTextAreaDescription());
        }
        return jScrollPaneDescription;
    }

    /**
     This method initializes jTextFieldAbstract 
     
     @return javax.swing.JTextField jTextFieldAbstract
     
     **/
    private JTextField getJTextFieldAbstract() {
        if (jTextFieldAbstract == null) {
            jTextFieldAbstract = new JTextField();
            jTextFieldAbstract.setBounds(new java.awt.Rectangle(160, 305, 320, 20));
            jTextFieldAbstract.setPreferredSize(new java.awt.Dimension(320, 20));
        }
        return jTextFieldAbstract;
    }

    /**
     This method initializes jComboBoxCompontentType 
     
     @return javax.swing.JComboBox jComboBoxCompontentType
     
     **/
    private JComboBox getJComboBoxCompontentType() {
        if (jComboBoxCompontentType == null) {
            jComboBoxCompontentType = new JComboBox();
            jComboBoxCompontentType.setBounds(new java.awt.Rectangle(160, 380, 320, 20));
            jComboBoxCompontentType.setPreferredSize(new java.awt.Dimension(320,20));
        }
        return jComboBoxCompontentType;
    }

    /**
     This method initializes jComboBoxModuleType 
     
     @return javax.swing.JComboBox jComboBoxModuleType
     
     **/
    private JComboBox getJComboBoxModuleType() {
        if (jComboBoxModuleType == null) {
            jComboBoxModuleType = new JComboBox();
            jComboBoxModuleType.setBounds(new java.awt.Rectangle(160, 355, 320, 20));
            jComboBoxModuleType.setPreferredSize(new java.awt.Dimension(320,20));
        }
        return jComboBoxModuleType;
    }

    /**
      This method initializes jTextFieldCopyright	
      	
      @return javax.swing.JTextField jTextFieldCopyright
     
     **/
    private JTextField getJTextFieldCopyright() {
        if (jTextFieldCopyright == null) {
            jTextFieldCopyright = new JTextField();
            jTextFieldCopyright.setBounds(new java.awt.Rectangle(160,170,320, 20));
            jTextFieldCopyright.setPreferredSize(new java.awt.Dimension(320,20));
        }
        return jTextFieldCopyright;
    }

    public static void main(String[] args) {

    }

    /**
     This is the default constructor
     
     **/
    public MsaHeader() {
        super();
        init();
        this.setVisible(true);
    }

    /**
     This is the override edit constructor
     
     @param inMsaHeader The input data of MsaHeaderDocument.MsaHeader
     
     **/
    public MsaHeader(MsaHeaderDocument.MsaHeader inMsaHeader) {
        super();
        init(inMsaHeader);
        this.setVisible(true);
        this.setViewMode(false);
    }

    /**
     Disable all components when the mode is view
     
     @param isView true - The view mode; false - The non-view mode
     
     **/
    public void setViewMode(boolean isView) {
        this.jButtonOk.setVisible(false);
        this.jButtonCancel.setVisible(false);
        if (isView) {
            this.jTextFieldBaseName.setEnabled(!isView);
            this.jTextFieldGuid.setEnabled(!isView);
            this.jTextFieldVersion.setEnabled(!isView);
            this.jTextAreaLicense.setEnabled(!isView);
            this.jTextFieldCopyright.setEnabled(!isView);
            this.jTextAreaDescription.setEnabled(!isView);
            this.jTextFieldSpecification.setEnabled(!isView);            
            this.jTextFieldAbstract.setEnabled(!isView);
            this.jComboBoxModuleType.setEnabled(!isView);
            this.jComboBoxCompontentType.setEnabled(!isView);
            this.jButtonCancel.setEnabled(!isView);
            this.jButtonGenerateGuid.setEnabled(!isView);
            this.jButtonOk.setEnabled(!isView);
        }
    }

    /**
     This method initializes this
     
     **/
    private void init() {
        //this.setSize(500, 515);
        this.setContentPane(getJContentPane());
        this.setTitle("Module Surface Area Header");
        initFrame();
    }

    /**
     This method initializes this
     Fill values to all fields if these values are not empty
     
     @param inMsaHeader  The input data of MsaHeaderDocument.MsaHeader
     
     **/
    private void init(MsaHeaderDocument.MsaHeader inMsaHeader) {
        init();
        if (inMsaHeader != null) {
            setMsaHeader(inMsaHeader);
            if (this.msaHeader.getBaseName() != null) {
                this.jTextFieldBaseName.setText(this.msaHeader.getBaseName());
            }
            if (this.msaHeader.getGuidValue() != null) {
                this.jTextFieldGuid.setText(this.msaHeader.getGuidValue());
            }
            if (this.msaHeader.getVersion() != null) {
                this.jTextFieldVersion.setText(this.msaHeader.getVersion());
            }
            if (this.msaHeader.getLicense() != null) {
                this.jTextAreaLicense.setText(this.msaHeader.getLicense().getStringValue());
            }
            if (this.msaHeader.getCopyright() != null) {
                this.jTextFieldCopyright.setText(this.msaHeader.getCopyright());
            }
            if (this.msaHeader.getDescription() != null) {
                this.jTextAreaDescription.setText(this.msaHeader.getDescription());
            }
            if (this.msaHeader.getSpecification() != null) {
                this.jTextFieldSpecification.setText(this.msaHeader.getSpecification().getStringValue());
            }
            if (this.msaHeader.getAbstract() != null) {
                this.jTextFieldAbstract.setText(this.msaHeader.getAbstract());
            }
            if (this.msaHeader.getModuleType() != null) {
                this.jComboBoxModuleType.setSelectedItem(this.msaHeader.getModuleType().toString());
            }
            if (this.msaHeader.getComponentType() != null) {
                this.jComboBoxCompontentType.setSelectedItem(this.msaHeader.getComponentType().toString());
            }
        }
    }

    /**
     This method initializes jContentPane
     
     @return javax.swing.JPanel jContentPane
     
     **/
    private JPanel getJContentPane() {
        if (jContentPane == null) {
        	jContentPane = new JPanel();
            jContentPane.setLayout(null);
            jContentPane.setLocation(new java.awt.Point(0, 0));
            jContentPane.setSize(new java.awt.Dimension(500, 524));
            jLabelCompontentType = new JLabel();
            jLabelCompontentType.setBounds(new java.awt.Rectangle(15, 380, 140, 20));
            jLabelCompontentType.setText("Compontent Type");
            jLabelModuleType = new JLabel();
            jLabelModuleType.setBounds(new java.awt.Rectangle(15, 355, 140, 20));
            jLabelModuleType.setText("Module Type");
            jLabelAbstract = new JLabel();
            jLabelAbstract.setBounds(new java.awt.Rectangle(15, 305, 140, 20));
            jLabelAbstract.setText("Abstract");
            jLabelSpecification = new JLabel();
            jLabelSpecification.setText("Specification");
            jLabelSpecification.setBounds(new java.awt.Rectangle(15, 280, 140, 20));
            jLabelDescription = new JLabel();
            jLabelDescription.setText("Description");
            jLabelDescription.setBounds(new java.awt.Rectangle(15, 195, 140, 20));
            jLabelCopyright = new JLabel();
            jLabelCopyright.setText("Copyright");
            jLabelCopyright.setBounds(new java.awt.Rectangle(15, 170, 140, 20));
            jLabelLicense = new JLabel();
            jLabelLicense.setText("License");
            jLabelLicense.setBounds(new java.awt.Rectangle(15, 85, 140, 20));
            jLabelVersion = new JLabel();
            jLabelVersion.setText("Version");
            jLabelVersion.setBounds(new java.awt.Rectangle(15, 60, 140, 20));
            jLabelGuid = new JLabel();
            jLabelGuid.setPreferredSize(new java.awt.Dimension(25, 15));
            jLabelGuid.setBounds(new java.awt.Rectangle(15, 35, 140, 20));
            jLabelGuid.setText("Guid");
            jLabelBaseName = new JLabel();
            jLabelBaseName.setText("Base Name");
            jLabelBaseName.setBounds(new java.awt.Rectangle(15, 10, 140, 20));
            jContentPane.add(jLabelBaseName, null);
            jContentPane.add(getJTextFieldBaseName(), null);
            jContentPane.add(jLabelGuid, null);
            jContentPane.add(getJTextFieldGuid(), null);
            jContentPane.add(jLabelVersion, null);
            jContentPane.add(getJTextFieldVersion(), null);
            jContentPane.add(getJButtonGenerateGuid(), null);
            jContentPane.add(jLabelLicense, null);
            jContentPane.add(jLabelCopyright, null);
            jContentPane.add(jLabelDescription, null);
            jContentPane.add(jLabelSpecification, null);
            jContentPane.add(getJTextFieldSpecification(), null);
            jContentPane.add(getJButtonOk(), null);
            jContentPane.add(getJButtonCancel(), null);
            jContentPane.add(getJScrollPaneLicense(), null);
            jContentPane.add(getJScrollPaneDescription(), null);
            jContentPane.add(jLabelAbstract, null);
            jContentPane.add(getJTextFieldAbstract(), null);
            jContentPane.add(jLabelModuleType, null);
            jContentPane.add(jLabelCompontentType, null);
            jContentPane.add(getJComboBoxCompontentType(), null);
            jContentPane.add(getJComboBoxModuleType(), null);

            jStarLabel1 = new StarLabel();
            jStarLabel1.setLocation(new java.awt.Point(0, 10));
            jStarLabel2 = new StarLabel();
            jStarLabel2.setLocation(new java.awt.Point(0, 35));
            jStarLabel3 = new StarLabel();
            jStarLabel3.setLocation(new java.awt.Point(0, 60));
            jStarLabel4 = new StarLabel();
            jStarLabel4.setLocation(new java.awt.Point(0, 85));
            jStarLabel5 = new StarLabel();
            jStarLabel5.setLocation(new java.awt.Point(0, 170));
            jStarLabel6 = new StarLabel();
            jStarLabel6.setLocation(new java.awt.Point(0, 195));
            jStarLabel7 = new StarLabel();
            jStarLabel7.setLocation(new java.awt.Point(0, 305));
            jStarLabel8 = new StarLabel();
            jStarLabel8.setLocation(new java.awt.Point(0, 355));
            jStarLabel9 = new StarLabel();
            jStarLabel9.setLocation(new java.awt.Point(0, 380));

            jContentPane.add(jStarLabel1, null);
            jContentPane.add(jStarLabel2, null);
            jContentPane.add(jStarLabel3, null);
            jContentPane.add(jStarLabel4, null);
            jContentPane.add(jStarLabel5, null);
            jContentPane.add(jStarLabel6, null);
            jContentPane.add(jStarLabel7, null);
            jContentPane.add(jStarLabel8, null);
            jContentPane.add(jStarLabel9, null);
            jContentPane.add(getJTextFieldCopyright(), null);

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
        if (arg0.getSource() == jButtonOk) {
            this.save();
            this.setEdited(true);
        }
        if (arg0.getSource() == jButtonCancel) {
            this.setEdited(false);
        }
        if (arg0.getSource() == jButtonGenerateGuid) {
            jTextFieldGuid.setText(Tools.generateUuidString());
        }
    }

    /**
     Data validation for all fields
     
     @retval true - All datas are valid
     @retval false - At least one data is invalid
     
     **/
    public boolean check() {
        //
        // Check if all required fields are not empty
        //
        if (isEmpty(this.jTextFieldBaseName.getText())) {
            Log.err("Base Name couldn't be empty");
            return false;
        }
        if (isEmpty(this.jTextFieldGuid.getText())) {
            Log.err("Guid couldn't be empty");
            return false;
        }
        if (isEmpty(this.jTextFieldVersion.getText())) {
            Log.err("Version couldn't be empty");
            return false;
        }
        if (isEmpty(this.jTextAreaLicense.getText())) {
            Log.err("License couldn't be empty");
            return false;
        }
        if (isEmpty(this.jTextFieldCopyright.getText())) {
            Log.err("Copyright couldn't be empty");
            return false;
        }
        if (isEmpty(this.jTextAreaDescription.getText())) {
            Log.err("Description couldn't be empty");
            return false;
        }
        if (isEmpty(this.jTextFieldAbstract.getText())) {
            Log.err("Abstract couldn't be empty");
            return false;
        }

        //
        // Check if all fields have correct data types 
        //
        if (!DataValidation.isBaseName(this.jTextFieldBaseName.getText())) {
            Log.err("Incorrect data type for Base Name");
            return false;
        }
        if (!DataValidation.isGuid((this.jTextFieldGuid).getText())) {
            Log.err("Incorrect data type for Guid");
            return false;
        }
        if (!DataValidation.isAbstract(this.jTextFieldAbstract.getText())) {
            Log.err("Incorrect data type for Abstract");
            return false;
        }
        if (!DataValidation.isCopyright(this.jTextFieldCopyright.getText())) {
            Log.err("Incorrect data type for Copyright");
            return false;
        }
        return true;
    }

    /**
     Save all components of Msa Header
     if exists msaHeader, set the value directly
     if not exists msaHeader, new an instance first
     
     **/
    public void save() {
        try {
            if (this.msaHeader == null) {
                msaHeader = MsaHeaderDocument.MsaHeader.Factory.newInstance();
            }
            if (!isEmpty(this.jTextFieldBaseName.getText())) {
                this.msaHeader.setBaseName(this.jTextFieldBaseName.getText());
            }
            if (!isEmpty(this.jTextFieldGuid.getText())) {
            	this.msaHeader.setGuidValue(this.jTextFieldGuid.getText());
            }

            this.msaHeader.setVersion(this.jTextFieldVersion.getText());

            if (this.msaHeader.getLicense() != null) {
                this.msaHeader.getLicense().setStringValue(this.jTextAreaLicense.getText());
            } else {
                LicenseDocument.License mLicense = LicenseDocument.License.Factory.newInstance();
                mLicense.setStringValue(this.jTextAreaLicense.getText());
                this.msaHeader.setLicense(mLicense);
            }

            this.msaHeader.setCopyright(this.jTextFieldCopyright.getText());
            this.msaHeader.setDescription(this.jTextAreaDescription.getText());

            if (this.msaHeader.getSpecification() != null) {
                this.msaHeader.getSpecification().setStringValue(this.jTextFieldSpecification.getText());
            } else {
                SpecificationDocument.Specification mSpecification = SpecificationDocument.Specification.Factory
                                                                                                                .newInstance();
                mSpecification.setStringValue(this.jTextFieldSpecification.getText());
                this.msaHeader.setSpecification(mSpecification);
            }

            if (this.msaHeader.getAbstract() != null) {
                this.msaHeader.setAbstract(this.jTextFieldAbstract.getText());
            }
            this.msaHeader.setModuleType(ModuleTypeDef.Enum.forString(this.jComboBoxModuleType.getSelectedItem()
                                                                                              .toString()));
            this.msaHeader
                          .setComponentType(FrameworkComponentTypes.Enum
                                                                        .forString(this.jComboBoxCompontentType
                                                                                                               .getSelectedItem()
                                                                                                               .toString()));
            if (this.msaHeader.getCreatedDate() == null) {
                this.msaHeader.setCreatedDate(Tools.getCurrentDateTime());
            }
        } catch (Exception e) {
            Log.err("Save Module", e.getMessage());
        }
    }

    /**
     This method initializes Module type and Compontent type
     
     **/
    private void initFrame() {
        jComboBoxModuleType.addItem("BASE");
        jComboBoxModuleType.addItem("SEC");
        jComboBoxModuleType.addItem("PEI_CORE");
        jComboBoxModuleType.addItem("PEIM");
        jComboBoxModuleType.addItem("DXE_CORE");
        jComboBoxModuleType.addItem("DXE_DRIVER");
        jComboBoxModuleType.addItem("DXE_RUNTIME_DRIVER");
        jComboBoxModuleType.addItem("DXE_SAL_DRIVER");
        jComboBoxModuleType.addItem("DXE_SMM_DRIVER");
        jComboBoxModuleType.addItem("TOOLS");
        jComboBoxModuleType.addItem("UEFI_DRIVER");
        jComboBoxModuleType.addItem("UEFI_APPLICATION");
        jComboBoxModuleType.addItem("USER_DEFINED");

        jComboBoxCompontentType.addItem("APRIORI");
        jComboBoxCompontentType.addItem("LIBRARY");
        jComboBoxCompontentType.addItem("FV_IMAGE_FILE");
        jComboBoxCompontentType.addItem("BS_DRIVER");
        jComboBoxCompontentType.addItem("RT_DRIVER");
        jComboBoxCompontentType.addItem("SAL_RT_DRIVER");
        jComboBoxCompontentType.addItem("PE32_PEIM");
        jComboBoxCompontentType.addItem("PIC_PEIM");
        jComboBoxCompontentType.addItem("COMBINED_PEIM_DRIVER");
        jComboBoxCompontentType.addItem("PEI_CORE");
        jComboBoxCompontentType.addItem("DXE_CORE");
        jComboBoxCompontentType.addItem("APPLICATION");
        jComboBoxCompontentType.addItem("BS_DRIVER_EFI");
        jComboBoxCompontentType.addItem("SHELLAPP");
    }

    /**
     Get MsaHeaderDocument.MsaHeader
     
     @return MsaHeaderDocument.MsaHeader
     
     **/
    public MsaHeaderDocument.MsaHeader getMsaHeader() {
        return msaHeader;
    }

    /**
     Set MsaHeaderDocument.MsaHeader
     
     @param msaHeader The input data of MsaHeaderDocument.MsaHeader
     
     **/
    public void setMsaHeader(MsaHeaderDocument.MsaHeader msaHeader) {
        this.msaHeader = msaHeader;
    }
    
	/* (non-Javadoc)
	 * @see java.awt.event.ComponentListener#componentResized(java.awt.event.ComponentEvent)
	 * 
	 * Override componentResized to resize all components when frame's size is changed
	 */
	public void componentResized(ComponentEvent arg0) {
        int intTempWidth = this.getWidth();
		resizeComponentWidth(this.jTextFieldBaseName, intTempWidth);
		resizeComponentWidth(this.jTextFieldGuid, intTempWidth);
		resizeComponentWidth(this.jTextFieldVersion, intTempWidth);
		resizeComponentWidth(this.jScrollPaneLicense, intTempWidth);
		resizeComponentWidth(this.jTextFieldCopyright, intTempWidth);
		resizeComponentWidth(this.jScrollPaneDescription, intTempWidth);
		resizeComponentWidth(this.jTextFieldSpecification, intTempWidth);
		resizeComponentWidth(this.jTextFieldAbstract, intTempWidth);
		resizeComponentWidth(this.jComboBoxModuleType, intTempWidth);
		resizeComponentWidth(this.jComboBoxCompontentType, intTempWidth);
		relocateComponentX(this.jButtonGenerateGuid, intTempWidth, DataType.SPACE_TO_RIGHT_FOR_GENERATE_BUTTON);
	}
}
