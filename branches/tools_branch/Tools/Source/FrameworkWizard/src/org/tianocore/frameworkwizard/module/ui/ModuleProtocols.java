/** @file
 
 The file is used to create, update Protocol of MSA/MBD file
 
 Copyright (c) 2006, Intel Corporation
 All rights reserved. This program and the accompanying materials
 are licensed and made available under the terms and conditions of the BSD License
 which accompanies this distribution.  The full text of the license may be found at
 http://opensource.org/licenses/bsd-license.php
 
 THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
 WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.
 
 **/

package org.tianocore.frameworkwizard.module.ui;

import java.awt.Dimension;
import java.awt.event.ActionEvent;

import javax.swing.JButton;
import javax.swing.JComboBox;
import javax.swing.JLabel;
import javax.swing.JPanel;
import javax.swing.JRadioButton;
import javax.swing.JTextField;

import org.tianocore.ProtocolNotifyUsage;
import org.tianocore.ProtocolUsage;
import org.tianocore.ProtocolsDocument;
import org.tianocore.frameworkwizard.common.DataValidation;
import org.tianocore.frameworkwizard.common.Log;
import org.tianocore.frameworkwizard.common.Tools;
import org.tianocore.frameworkwizard.common.ui.IDefaultMutableTreeNode;
import org.tianocore.frameworkwizard.common.ui.IInternalFrame;
import org.tianocore.frameworkwizard.common.ui.StarLabel;

/**
 The class is used to create, update Protocol of MSA/MBD file
 It extends IInternalFrame
 
 @since ModuleEditor 1.0

 **/
public class ModuleProtocols extends IInternalFrame {

    ///
    /// Define class Serial Version UID
    ///
    private static final long serialVersionUID = -9084913640747858848L;

    //
    //Define class members
    //
    private ProtocolsDocument.Protocols protocols = null;

    private int location = -1;

    private JPanel jContentPane = null;

    private JLabel jLabelC_Name = null;

    private JTextField jTextFieldC_Name = null;

    private JLabel jLabelGuid = null;

    private JTextField jTextFieldGuid = null;

    private JLabel jLabelFeatureFlag = null;

    private JTextField jTextFieldFeatureFlag = null;

    private JButton jButtonOk = null;

    private JButton jButtonCancel = null;

    private JLabel jLabelUsage = null;

    private JComboBox jComboBoxUsage = null;

    private JLabel jLabelEnableFeature = null;

    private JRadioButton jRadioButtonEnableFeature = null;

    private JRadioButton jRadioButtonDisableFeature = null;

    private JRadioButton jRadioButtonProtocol = null;

    private JRadioButton jRadioButtonProtocolNotify = null;

    private JButton jButtonGenerateGuid = null;

    private StarLabel jStarLabel1 = null;

    private StarLabel jStarLabel2 = null;

    private JLabel jLabelProtocolType = null;

    /**
     This method initializes jTextFieldC_Name 
     
     @return javax.swing.JTextField jTextFieldC_Name
     
     **/
    private JTextField getJTextFieldProtocolName() {
        if (jTextFieldC_Name == null) {
            jTextFieldC_Name = new JTextField();
            jTextFieldC_Name.setBounds(new java.awt.Rectangle(160, 35, 320, 20));
        }
        return jTextFieldC_Name;
    }

    /**
     This method initializes jTextFieldGuid 
     
     @return javax.swing.JTextField jTextFieldGuid
     
     **/
    private JTextField getJTextFieldGuid() {
        if (jTextFieldGuid == null) {
            jTextFieldGuid = new JTextField();
            jTextFieldGuid.setBounds(new java.awt.Rectangle(160, 60, 250, 20));
        }
        return jTextFieldGuid;
    }

    /**
     This method initializes jTextFieldFeatureFlag 
     
     @return javax.swing.JTextField jTextFieldFeatureFlag
     
     **/
    private JTextField getJTextFieldFeatureFlag() {
        if (jTextFieldFeatureFlag == null) {
            jTextFieldFeatureFlag = new JTextField();
            jTextFieldFeatureFlag.setBounds(new java.awt.Rectangle(160, 135, 320, 20));
        }
        return jTextFieldFeatureFlag;
    }

    /**
     This method initializes jButtonOk 
     
     @return javax.swing.JButton jButtonOk
     
     **/
    private JButton getJButtonOk() {
        if (jButtonOk == null) {
            jButtonOk = new JButton();
            jButtonOk.setText("OK");
            jButtonOk.setBounds(new java.awt.Rectangle(290, 190, 90, 20));
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
            jButtonCancel.setBounds(new java.awt.Rectangle(390, 190, 90, 20));
            jButtonCancel.setPreferredSize(new Dimension(90, 20));
            jButtonCancel.addActionListener(this);
        }
        return jButtonCancel;
    }

    /**
     This method initializes jComboBoxUsage 
     
     @return javax.swing.JComboBox jComboBoxUsage
     
     **/
    private JComboBox getJComboBoxProtocolUsage() {
        if (jComboBoxUsage == null) {
            jComboBoxUsage = new JComboBox();
            jComboBoxUsage.setBounds(new java.awt.Rectangle(160, 85, 320, 20));
        }
        return jComboBoxUsage;
    }

    /**
     This method initializes jRadioButtonEnableFeature 
     
     @return javax.swing.JRadioButton jRadioButtonEnableFeature
     
     **/
    private JRadioButton getJRadioButtonEnableFeature() {
        if (jRadioButtonEnableFeature == null) {
            jRadioButtonEnableFeature = new JRadioButton();
            jRadioButtonEnableFeature.setText("Enable");
            jRadioButtonEnableFeature.setBounds(new java.awt.Rectangle(160, 110, 90, 20));
            jRadioButtonEnableFeature.addActionListener(this);
            jRadioButtonEnableFeature.setSelected(true);
        }
        return jRadioButtonEnableFeature;
    }

    /**
     This method initializes jRadioButtonDisableFeature 
     
     @return javax.swing.JRadioButton jRadioButtonDisableFeature
     
     **/
    private JRadioButton getJRadioButtonDisableFeature() {
        if (jRadioButtonDisableFeature == null) {
            jRadioButtonDisableFeature = new JRadioButton();
            jRadioButtonDisableFeature.setText("Disable");
            jRadioButtonDisableFeature.setBounds(new java.awt.Rectangle(320, 110, 90, 20));
            jRadioButtonDisableFeature.addActionListener(this);
        }
        return jRadioButtonDisableFeature;
    }

    /**
     This method initializes jRadioButtonProtocol 
     
     @return javax.swing.JRadioButton jRadioButtonProtocol
     
     **/
    private JRadioButton getJRadioButtonProtocol() {
        if (jRadioButtonProtocol == null) {
            jRadioButtonProtocol = new JRadioButton();
            jRadioButtonProtocol.setText("Protocol");
            jRadioButtonProtocol.setBounds(new java.awt.Rectangle(160, 10, 90, 20));
            jRadioButtonProtocol.setSelected(true);
            jRadioButtonProtocol.addActionListener(this);
        }
        return jRadioButtonProtocol;
    }

    /**
     This method initializes jRadioButtonProtocolNotify 
     
     @return javax.swing.JRadioButton jRadioButtonProtocolNotify
     
     **/
    private JRadioButton getJRadioButtonProtocolNotify() {
        if (jRadioButtonProtocolNotify == null) {
            jRadioButtonProtocolNotify = new JRadioButton();
            jRadioButtonProtocolNotify.setText("Protocol Notify");
            jRadioButtonProtocolNotify.setBounds(new java.awt.Rectangle(320, 10, 120, 20));
            jRadioButtonProtocolNotify.addActionListener(this);
        }
        return jRadioButtonProtocolNotify;
    }

    /**
     This method initializes jButtonGenerateGuid 
     
     @return javax.swing.JButton jButtonGenerateGuid
     
     **/
    private JButton getJButtonGenerateGuid() {
        if (jButtonGenerateGuid == null) {
            jButtonGenerateGuid = new JButton();
            jButtonGenerateGuid.setBounds(new java.awt.Rectangle(415, 60, 65, 20));
            jButtonGenerateGuid.setText("GEN");
            jButtonGenerateGuid.addActionListener(this);
        }
        return jButtonGenerateGuid;
    }

    public static void main(String[] args) {

    }

    /**
     This is the default constructor
     
     **/
    public ModuleProtocols() {
        super();
        init();
        this.setVisible(true);
    }

    /**
     This is the override edit constructor
     
     @param inProtocol The input data of ProtocolsDocument.Protocols
     
     **/
    public ModuleProtocols(ProtocolsDocument.Protocols inProtocol) {
        super();
        init(inProtocol);
        this.setVisible(true);
    }

    /**
     This is the override edit constructor
     
     @param inProtocol The input data of ProtocolsDocument.Protocols
     @param type The input data of node type
     @param index The input data of node index
     
     **/
    public ModuleProtocols(ProtocolsDocument.Protocols inProtocol, int type, int index) {
        super();
        init(inProtocol, type, index);
        this.setVisible(true);
    }

    /**
     This method initializes this
     
     @param inProtocol The input data of ProtocolsDocument.Protocols
     
     **/
    private void init(ProtocolsDocument.Protocols inProtocol) {
        init();
        this.setProtocols(inProtocol);
    }

    /**
     This method initializes this
     Fill values to all fields if these values are not empty
     
     @param inProtocol The input data of ProtocolsDocument.Protocols
     @param type The input data of node type
     @param index The input data of node index
     
     **/
    private void init(ProtocolsDocument.Protocols inProtocol, int type, int index) {
        init(inProtocol);
        this.location = index;
        if (type == IDefaultMutableTreeNode.MSA_PROTOCOLS) {
            this.jRadioButtonProtocol.setSelected(true);
            this.jRadioButtonProtocolNotify.setSelected(false);
            if (this.protocols.getProtocolArray(index).getStringValue() != null) {
                this.jTextFieldC_Name.setText(this.protocols.getProtocolArray(index).getStringValue());
            }
            if (this.protocols.getProtocolArray(index).getGuid() != null) {
                this.jTextFieldGuid.setText(this.protocols.getProtocolArray(index).getGuid());
            }
            if (this.protocols.getProtocolArray(index).getUsage() != null) {
                this.jComboBoxUsage.setSelectedItem(this.protocols.getProtocolArray(index).getUsage().toString());
            }
            if (this.protocols.getProtocolArray(index).getFeatureFlag() != null) {
                this.jTextFieldFeatureFlag.setText(this.protocols.getProtocolArray(index).getFeatureFlag());
            }
        } else if (type == IDefaultMutableTreeNode.MSA_PROTOCOLS) {
            this.jRadioButtonProtocol.setSelected(false);
            this.jRadioButtonProtocolNotify.setSelected(true);
            this.jTextFieldFeatureFlag.setEditable(false);
            this.jRadioButtonDisableFeature.setEnabled(false);
            this.jRadioButtonEnableFeature.setEnabled(false);
            this.jComboBoxUsage.setEnabled(false);
            if (this.protocols.getProtocolNotifyArray(index).getStringValue() != null) {
                this.jTextFieldC_Name.setText(this.protocols.getProtocolNotifyArray(index).getStringValue());
            }
            if (this.protocols.getProtocolNotifyArray(index).getGuid() != null) {
                this.jTextFieldGuid.setText(this.protocols.getProtocolNotifyArray(index).getGuid());
            }
            if (this.protocols.getProtocolNotifyArray(index).getUsage() != null) {
                this.jComboBoxUsage.setSelectedItem(this.protocols.getProtocolNotifyArray(index).getUsage().toString());
            }
        }
        this.jRadioButtonProtocol.setEnabled(false);
        this.jRadioButtonProtocolNotify.setEnabled(false);
    }

    /**
     This method initializes this
     
     **/
    private void init() {
        this.setSize(500, 515);
        this.setName("JFrame");
        this.setContentPane(getJContentPane());
        this.setTitle("Protocols");
        initFrame();
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
            this.jRadioButtonProtocol.setEnabled(!isView);
            this.jRadioButtonProtocolNotify.setEnabled(!isView);
            this.jTextFieldC_Name.setEnabled(!isView);
            this.jTextFieldGuid.setEnabled(!isView);
            this.jComboBoxUsage.setEnabled(!isView);
            this.jRadioButtonEnableFeature.setEnabled(!isView);
            this.jRadioButtonDisableFeature.setEnabled(!isView);
            this.jTextFieldFeatureFlag.setEnabled(!isView);
            this.jButtonGenerateGuid.setEnabled(!isView);
        }
    }

    /**
     This method initializes jContentPane
     
     @return javax.swing.JPanel jContentPane
     
     **/
    private JPanel getJContentPane() {
        if (jContentPane == null) {
            jLabelProtocolType = new JLabel();
            jLabelProtocolType.setBounds(new java.awt.Rectangle(15, 10, 140, 20));
            jLabelProtocolType.setText("Protocol Type");
            jLabelEnableFeature = new JLabel();
            jLabelEnableFeature.setText("Enable Feature");
            jLabelEnableFeature.setBounds(new java.awt.Rectangle(15, 110, 140, 20));
            jLabelUsage = new JLabel();
            jLabelUsage.setText("Usage");
            jLabelUsage.setBounds(new java.awt.Rectangle(15, 85, 140, 20));
            jLabelFeatureFlag = new JLabel();
            jLabelFeatureFlag.setText("Feature Flag");
            jLabelFeatureFlag.setBounds(new java.awt.Rectangle(15, 135, 140, 20));
            jLabelGuid = new JLabel();
            jLabelGuid.setText("Guid");
            jLabelGuid.setBounds(new java.awt.Rectangle(15, 60, 140, 20));
            jLabelC_Name = new JLabel();
            jLabelC_Name.setText("C_Name");
            jLabelC_Name.setBounds(new java.awt.Rectangle(15, 35, 140, 20));
            jContentPane = new JPanel();
            jContentPane.setLayout(null);
            jContentPane.add(jLabelC_Name, null);
            jContentPane.add(getJTextFieldProtocolName(), null);
            jContentPane.add(jLabelGuid, null);
            jContentPane.add(getJTextFieldGuid(), null);
            jContentPane.add(jLabelFeatureFlag, null);
            jContentPane.add(getJTextFieldFeatureFlag(), null);
            jContentPane.add(getJButtonOk(), null);
            jContentPane.add(getJButtonCancel(), null);
            jContentPane.add(jLabelUsage, null);
            jContentPane.add(getJComboBoxProtocolUsage(), null);
            jContentPane.add(jLabelEnableFeature, null);
            jContentPane.add(getJRadioButtonEnableFeature(), null);
            jContentPane.add(getJRadioButtonDisableFeature(), null);
            jContentPane.add(getJRadioButtonProtocol(), null);
            jContentPane.add(getJRadioButtonProtocolNotify(), null);
            jContentPane.add(getJButtonGenerateGuid(), null);
            jContentPane.add(jLabelProtocolType, null);

            jStarLabel1 = new StarLabel();
            jStarLabel1.setBounds(new java.awt.Rectangle(0, 10, 10, 20));
            jStarLabel2 = new StarLabel();
            jStarLabel2.setBounds(new java.awt.Rectangle(0, 35, 10, 20));

            jContentPane.add(jStarLabel1, null);
            jContentPane.add(jStarLabel2, null);
        }
        return jContentPane;
    }

    /**
     This method initializes Usage type
     
     **/
    private void initFrame() {
        jComboBoxUsage.addItem("ALWAYS_CONSUMED");
        jComboBoxUsage.addItem("SOMETIMES_CONSUMED");
        jComboBoxUsage.addItem("ALWAYS_PRODUCED");
        jComboBoxUsage.addItem("SOMETIMES_PRODUCED");
        jComboBoxUsage.addItem("TO_START");
        jComboBoxUsage.addItem("BY_START");
        jComboBoxUsage.addItem("PRIVATE");
    }

    /* (non-Javadoc)
     * @see java.awt.event.ActionListener#actionPerformed(java.awt.event.ActionEvent)
     *
     * Override actionPerformed to listen all actions
     * 
     */
    public void actionPerformed(ActionEvent arg0) {
        if (arg0.getSource() == jButtonOk) {
            this.setEdited(true);
            this.save();
            this.dispose();
        }
        if (arg0.getSource() == jButtonCancel) {
            this.dispose();
        }

        //
        // Contorl the selected status when click RadionButton
        // Do not use Radio Button Group
        //
        if (arg0.getSource() == jRadioButtonProtocol) {
            if (jRadioButtonProtocol.isSelected()) {
                jRadioButtonProtocolNotify.setSelected(false);
                jRadioButtonEnableFeature.setEnabled(true);
                jRadioButtonDisableFeature.setEnabled(true);
                jTextFieldFeatureFlag.setEnabled(true);
                jComboBoxUsage.setEnabled(true);
            }
            if (!jRadioButtonProtocolNotify.isSelected() && !jRadioButtonProtocol.isSelected()) {
                jRadioButtonProtocol.setSelected(true);
                jRadioButtonEnableFeature.setEnabled(true);
                jRadioButtonDisableFeature.setEnabled(true);
                jTextFieldFeatureFlag.setEnabled(true);
                jComboBoxUsage.setEnabled(true);
            }
        }

        if (arg0.getSource() == jRadioButtonProtocolNotify) {
            if (jRadioButtonProtocolNotify.isSelected()) {
                jRadioButtonProtocol.setSelected(false);
                jRadioButtonEnableFeature.setEnabled(false);
                jRadioButtonDisableFeature.setEnabled(false);
                jTextFieldFeatureFlag.setEnabled(false);
                jComboBoxUsage.setSelectedIndex(1);
                jComboBoxUsage.setEnabled(false);
            }
            if (!jRadioButtonProtocolNotify.isSelected() && !jRadioButtonProtocol.isSelected()) {
                jRadioButtonProtocolNotify.setSelected(true);
                jRadioButtonEnableFeature.setEnabled(false);
                jRadioButtonDisableFeature.setEnabled(false);
                jTextFieldFeatureFlag.setEnabled(false);
                jComboBoxUsage.setSelectedIndex(1);
                jComboBoxUsage.setEnabled(false);
            }
        }

        //
        // Contorl the selected status when click RadionButton
        // Do not use Radio Button Group
        //
        if (arg0.getSource() == jRadioButtonEnableFeature) {
            if (jRadioButtonEnableFeature.isSelected()) {
                jRadioButtonDisableFeature.setSelected(false);
            }
            if (!jRadioButtonDisableFeature.isSelected() && !jRadioButtonEnableFeature.isSelected()) {
                jRadioButtonEnableFeature.setSelected(true);
            }
        }

        if (arg0.getSource() == jRadioButtonDisableFeature) {
            if (jRadioButtonDisableFeature.isSelected()) {
                jRadioButtonEnableFeature.setSelected(false);
            }
            if (!jRadioButtonDisableFeature.isSelected() && !jRadioButtonEnableFeature.isSelected()) {
                jRadioButtonDisableFeature.setSelected(true);
            }
        }

        if (arg0.getSource() == jButtonGenerateGuid) {
            jTextFieldGuid.setText(Tools.generateUuidString());
        }
    }

    /**
     Get ProtocolsDocument.Protocols
     
     @return ProtocolsDocument.Protocols
     
     **/
    public ProtocolsDocument.Protocols getProtocols() {
        return protocols;
    }

    /**
     Set ProtocolsDocument.Protocols
     
     @param protocols The input data of ProtocolsDocument.Protocols
     
     **/
    public void setProtocols(ProtocolsDocument.Protocols protocols) {
        this.protocols = protocols;
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
        if (isEmpty(this.jTextFieldC_Name.getText())) {
            Log.err("C_Name couldn't be empty");
            return false;
        }

        //
        // Check if all fields have correct data types 
        //
        if (!DataValidation.isCName(this.jTextFieldC_Name.getText())) {
            Log.err("Incorrect data type for C_Name");
            return false;
        }
        if (!isEmpty(this.jTextFieldGuid.getText()) && !DataValidation.isGuid(this.jTextFieldGuid.getText())) {
            Log.err("Incorrect data type for Guid");
            return false;
        }
        if (!isEmpty(this.jTextFieldFeatureFlag.getText())
            && !DataValidation.isFeatureFlag(this.jTextFieldFeatureFlag.getText())) {
            Log.err("Incorrect data type for Feature Flag");
            return false;
        }

        return true;
    }

    /**
     Save all components of Protocols
     if exists protocols, set the value directly
     if not exists protocols, new an instance first
     
     **/
    public void save() {
        try {
            if (this.protocols == null) {
                protocols = ProtocolsDocument.Protocols.Factory.newInstance();
            }
            if (this.jRadioButtonProtocol.isSelected()) {
                ProtocolsDocument.Protocols.Protocol protocol = ProtocolsDocument.Protocols.Protocol.Factory
                                                                                                            .newInstance();
                protocol.setStringValue(this.jTextFieldC_Name.getText());
                if (!isEmpty(this.jTextFieldGuid.getText())) {
                    protocol.setGuid(this.jTextFieldGuid.getText());
                }
                protocol.setUsage(ProtocolUsage.Enum.forString(jComboBoxUsage.getSelectedItem().toString()));
                if (!isEmpty(this.jTextFieldFeatureFlag.getText())) {
                    protocol.setFeatureFlag(this.jTextFieldFeatureFlag.getText());
                }
                if (location > -1) {
                    protocols.setProtocolArray(location, protocol);
                } else {
                    protocols.addNewProtocol();
                    protocols.setProtocolArray(protocols.getProtocolList().size() - 1, protocol);
                }
            }
            if (this.jRadioButtonProtocolNotify.isSelected()) {
                ProtocolsDocument.Protocols.ProtocolNotify protocolNofity = ProtocolsDocument.Protocols.ProtocolNotify.Factory
                                                                                                                              .newInstance();
                protocolNofity.setStringValue(this.jTextFieldC_Name.getText());
                if (!isEmpty(this.jTextFieldGuid.getText())) {
                    protocolNofity.setGuid(this.jTextFieldGuid.getText());
                }
                protocolNofity
                              .setUsage(ProtocolNotifyUsage.Enum.forString(jComboBoxUsage.getSelectedItem().toString()));
                if (location > -1) {
                    protocols.setProtocolNotifyArray(location, protocolNofity);
                } else {
                    protocols.addNewProtocolNotify();
                    protocols.setProtocolNotifyArray(protocols.getProtocolNotifyList().size() - 1, protocolNofity);
                }
            }
        } catch (Exception e) {
            Log.err("Update Protocols", e.getMessage());
        }
    }
}
