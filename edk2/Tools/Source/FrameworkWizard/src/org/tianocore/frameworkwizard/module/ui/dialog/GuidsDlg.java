/** @file
 <<The file is used to create, update Guids of the MSA file>>
 
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
import java.util.Vector;

import javax.swing.JButton;
import javax.swing.JComboBox;
import javax.swing.JLabel;
import javax.swing.JPanel;
import javax.swing.JScrollPane;
import javax.swing.JTextField;
import javax.swing.JTextArea;

import org.tianocore.frameworkwizard.common.DataType;
import org.tianocore.frameworkwizard.common.DataValidation;
import org.tianocore.frameworkwizard.common.EnumerationData;
import org.tianocore.frameworkwizard.common.Log;
import org.tianocore.frameworkwizard.common.Tools;
import org.tianocore.frameworkwizard.common.ui.ArchCheckBox;
import org.tianocore.frameworkwizard.common.ui.IDialog;
import org.tianocore.frameworkwizard.common.ui.IFrame;
import org.tianocore.frameworkwizard.common.ui.StarLabel;
import org.tianocore.frameworkwizard.module.Identifications.Guids.GuidsIdentification;
import org.tianocore.frameworkwizard.workspace.WorkspaceTools;

/**
 * The class is used to create, update Guids of the MSA file
 *  
 * It extends IDialog
 * 
 */
public class GuidsDlg extends IDialog {

  // /
  // / Define class Serial Version UID
  // /
  private static final long serialVersionUID = 6710858997766979803L;

  //
  // Define class members
  //
  private JPanel jContentPane = null;

  private JLabel jLabelC_Name = null;

  private JComboBox jComboBoxCName = null;

  private JLabel jLabelUsage = null;

  private JComboBox jComboBoxUsage = null;

  private StarLabel jStarLabel1 = null;

  private StarLabel jStarLabel2 = null;

  private JLabel jLabelFeatureFlag = null;

  private JTextField jTextFieldFeatureFlag = null;

  private JLabel jLabelArch = null;

  private JScrollPane jScrollPane = null;

  private JLabel jLabelHelpText = null;

  private JTextArea jTextAreaHelpText = null;

  private JScrollPane jScrollPaneHelpText = null;
  
  private ArchCheckBox jArchCheckBox = null;

  private JButton jButtonOk = null;

  private JButton jButtonCancel = null;

  //
  // Not used by UI
  //
  private GuidsIdentification id = null;

  private EnumerationData ed = new EnumerationData();

  private WorkspaceTools wt = new WorkspaceTools();

  /**
   * This method initializes jTextFieldC_Name
   * 
   * @return javax.swing.JTextField jTextFieldC_Name
   * 
   */
  private JComboBox getJComboBoxCName() {
    if (jComboBoxCName == null) {
      jComboBoxCName = new JComboBox();
      jComboBoxCName.setBounds(new java.awt.Rectangle(160, 10, 320, 20));
      jComboBoxCName.setPreferredSize(new java.awt.Dimension(320, 20));
      jComboBoxCName.setToolTipText("Select the C Name of the GUID");
    }
    return jComboBoxCName;
  }

  /**
   * This method initializes jComboBoxUsage
   * 
   * @return javax.swing.JComboBox jComboBoxUsage
   * 
   */
  private JComboBox getJComboBoxUsage() {
    if (jComboBoxUsage == null) {
      jComboBoxUsage = new JComboBox();
      jComboBoxUsage.setBounds(new java.awt.Rectangle(160, 35, 320, 20));
      jComboBoxUsage.setPreferredSize(new java.awt.Dimension(320, 20));
      jComboBoxUsage.setToolTipText("<html><table><tr><td>ALWAYS_CONSUMED</td><td>Module always consumes the GUID</td></tr><tr><td>SOMETIMES_CONSUMED</td><td>Module will use the GUID only if it is present</td></tr><tr><td>ALWAYS_PRODUCED</td><td>Module always produces the GUID</td></tr><tr><td>SOMETIMES_PRODUCED</td><td>Module will sometimes produce the GUID</td></tr><tr><td>DEFAULT</td><td>Default is the the GUID that specified the<br>instance of the package</td></tr></table></html>");
    }
    return jComboBoxUsage;
  }

  /**
   * This method initializes jTextFieldFeatureFlag
   * 
   * @return javax.swing.JTextField
   * 
   */
  private JTextField getJTextFieldFeatureFlag() {
    if (jTextFieldFeatureFlag == null) {
      jTextFieldFeatureFlag = new JTextField();
      jTextFieldFeatureFlag.setBounds(new java.awt.Rectangle(160, 105, 320, 20));
      jTextFieldFeatureFlag.setPreferredSize(new java.awt.Dimension(320, 20));
      jTextFieldFeatureFlag.setToolTipText("Postfix expression that must evaluate to TRUE or FALSE");
    }
    return jTextFieldFeatureFlag;
  }

  /**
   * This method initializes jScrollPane
   * 
   * @return javax.swing.JScrollPane
   */
  private JScrollPane getJScrollPane() {
    if (jScrollPane == null) {
      jScrollPane = new JScrollPane();
      jScrollPane.setViewportView(getJContentPane());
    }
    return jScrollPane;
  }

  /**
   * This method initializes jTextAreaHelpText
   * 
   * @return javax.swing.JTextArea
   * 
   */
  private JTextArea getJTextAreaHelpText() {
    if (jTextAreaHelpText == null) {
      jTextAreaHelpText = new JTextArea();
      jTextAreaHelpText.setLineWrap(true);
      jTextAreaHelpText.setWrapStyleWord(true);
    }
    return jTextAreaHelpText;
  }

  /**
   * 
   * This method initializes jScrollPaneHelpText
   * 
   * @return javax.swing.JScrollPane jScrollPaneHelpText
   * 
   **/
  private JScrollPane getJScrollPaneHelpText() {
    if (jScrollPaneHelpText == null){
      jScrollPaneHelpText = new JScrollPane();
      jScrollPaneHelpText.setHorizontalScrollBarPolicy(javax.swing.JScrollPane.HORIZONTAL_SCROLLBAR_NEVER);
      jScrollPaneHelpText.setSize(new java.awt.Dimension(320, 40));
      jScrollPaneHelpText.setLocation(new java.awt.Point(160,60));
      jScrollPaneHelpText.setViewportView(getJTextAreaHelpText());
    }
    return jScrollPaneHelpText;
  }
  /**
   * This method initializes jButtonOk
   * 
   * @return javax.swing.JButton
   * 
   */
  private JButton getJButtonOk() {
    if (jButtonOk == null) {
      jButtonOk = new JButton();
      jButtonOk.setBounds(new java.awt.Rectangle(290, 157, 90, 20));
      jButtonOk.setText("Ok");
      jButtonOk.addActionListener(this);
    }
    return jButtonOk;
  }

  /**
   * This method initializes jButtonCancel
   * 
   * @return javax.swing.JButton
   * 
   */
  private JButton getJButtonCancel() {
    if (jButtonCancel == null) {
      jButtonCancel = new JButton();
      jButtonCancel.setBounds(new java.awt.Rectangle(390, 157, 90, 20));
      jButtonCancel.setText("Cancel");
      jButtonCancel.addActionListener(this);
    }
    return jButtonCancel;
  }

  public static void main(String[] args) {

  }

  /**
   * 
   * This method initializes this
   * 
   */
  private void init() {
    this.setSize(500, 230);
    this.setContentPane(getJScrollPane());
    this.setTitle("Guids");
    initFrame();
    this.setViewMode(false);
    this.centerWindow();
  }

  /**
   * 
   * This method initializes this Fill values to all fields if these values are
   * not empty
   * 
   * @param inGuidsId
   * 
   */
  private void init(GuidsIdentification inGuidsId) {
    init();
    this.id = inGuidsId;

    if (this.id != null) {
      this.jComboBoxCName.setSelectedItem(id.getName());
      this.jComboBoxUsage.setSelectedItem(id.getUsage());
      this.jTextAreaHelpText.setText(id.getHelp());
      this.jTextFieldFeatureFlag.setText(id.getFeatureFlag());
      this.jArchCheckBox.setSelectedItems(id.getSupArchList());
    }
  }

  /**
   * This is the override edit constructor
   * 
   * @param inGuidsIdentification
   * @param iFrame
   * 
   */
  public GuidsDlg(GuidsIdentification inGuidsIdentification, IFrame iFrame) {
    super(iFrame, true);
    init(inGuidsIdentification);
  }

  /**
   * Disable all components when the mode is view
   * 
   * @param isView
   *          true - The view mode; false - The non-view mode
   * 
   */
  public void setViewMode(boolean isView) {
    if (isView) {
      this.jComboBoxUsage.setEnabled(!isView);
    }
  }

  /**
   * This method initializes jContentPane
   * 
   * @return javax.swing.JPanel jContentPane
   * 
   */
  private JPanel getJContentPane() {
    if (jContentPane == null) {
      jStarLabel1 = new StarLabel();
      jStarLabel1.setLocation(new java.awt.Point(2, 10));
      jLabelC_Name = new JLabel();
      jLabelC_Name.setText("Select GUID's C Name");
      jLabelC_Name.setBounds(new java.awt.Rectangle(15, 10, 145, 20));

      jStarLabel2 = new StarLabel();
      jStarLabel2.setLocation(new java.awt.Point(2, 35));
      jLabelUsage = new JLabel();
      jLabelUsage.setText("Usage");
      jLabelUsage.setBounds(new java.awt.Rectangle(15, 35, 145, 20));

      jLabelHelpText = new JLabel();
      jLabelHelpText.setBounds(new java.awt.Rectangle(14, 60, 145, 20));
      jLabelHelpText.setText("Help Text");

      jLabelFeatureFlag = new JLabel();
      jLabelFeatureFlag.setBounds(new java.awt.Rectangle(15, 105, 145, 20));
      jLabelFeatureFlag.setText("Feature Flag Expression");

      jLabelArch = new JLabel();
      jLabelArch.setBounds(new java.awt.Rectangle(15, 130, 145, 20));
      jLabelArch.setText("Supported Architectures");
      jArchCheckBox = new ArchCheckBox();
      jArchCheckBox.setBounds(new java.awt.Rectangle(160, 130, 320, 20));
      jArchCheckBox.setPreferredSize(new java.awt.Dimension(320, 20));

      jContentPane = new JPanel();
      jContentPane.setLayout(null);
      jContentPane.setPreferredSize(new java.awt.Dimension(490, 165));
      
      jContentPane.add(jStarLabel1, null);
      jContentPane.add(jLabelC_Name, null);
      jContentPane.add(getJComboBoxCName(), null);
      jContentPane.add(jStarLabel2, null);
      jContentPane.add(jLabelUsage, null);
      jContentPane.add(getJComboBoxUsage(), null);
      jContentPane.add(jLabelHelpText, null);
      jContentPane.add(getJScrollPaneHelpText(), null);
      jContentPane.add(jLabelFeatureFlag, null);
      jContentPane.add(getJTextFieldFeatureFlag(), null);
      jContentPane.add(jLabelArch, null);
      jContentPane.add(jArchCheckBox, null);
      jContentPane.add(getJButtonOk(), null);
      jContentPane.add(getJButtonCancel(), null);
    }
    return jContentPane;
  }

  /**
   * This method initializes Usage type
   * 
   */
  private void initFrame() {
    Tools.generateComboBoxByVector(jComboBoxCName, wt
        .getAllGuidDeclarationsFromWorkspace());
    Tools.generateComboBoxByVector(jComboBoxUsage, ed.getVGuidUsage());
  }

  /*
   * (non-Javadoc)
   * 
   * @see java.awt.event.ActionListener#actionPerformed(java.awt.event.ActionEvent)
   * 
   * Override actionPerformed to listen all actions
   * 
   */
  public void actionPerformed(ActionEvent arg0) {
    if (arg0.getSource() == jButtonOk) {
      if (checkAdd()) {
        getCurrentGuids();
        this.returnType = DataType.RETURN_TYPE_OK;
        this.setVisible(false);
      }
    }

    if (arg0.getSource() == jButtonCancel) {
      this.returnType = DataType.RETURN_TYPE_CANCEL;
      this.setVisible(false);
    }
  }

  /**
   * Data validation for all fields
   * 
   * @retval true - All datas are valid
   * @retval false - At least one data is invalid
   * 
   */
  public boolean checkAdd() {
    //
    // Check if all fields have correct data types
    //

    //
    // Check Name
    //
    if (!isEmpty(this.jComboBoxCName.getSelectedItem().toString())) {
      if (!DataValidation.isC_NameType(this.jComboBoxCName.getSelectedItem()
          .toString())) {
        Log.wrn("Update Guids", "Incorrect data type for Guid Name");
        return false;
      }
    }

    //
    // Check FeatureFlag
    //
    if (!isEmpty(this.jTextFieldFeatureFlag.getText())) {
      if (!DataValidation.isFeatureFlag(this.jTextFieldFeatureFlag.getText())) {
        Log.wrn("Update Guids", "Incorrect data type for Feature Flag");
        return false;
      }
    }

    return true;
  }

private GuidsIdentification getCurrentGuids() {
        String arg0 = this.jComboBoxCName.getSelectedItem().toString();
        String arg1 = this.jComboBoxUsage.getSelectedItem().toString();

        String arg2 = this.jTextFieldFeatureFlag.getText();
        Vector<String> arg3 = this.jArchCheckBox.getSelectedItemsVector();
        String arg4 = this.jTextAreaHelpText.getText();

        id = new GuidsIdentification(arg0, arg1, arg2, arg3, arg4);
        return id;
    }  public GuidsIdentification getId() {
    return id;
  }

  public void setId(GuidsIdentification id) {
    this.id = id;
  }
}
