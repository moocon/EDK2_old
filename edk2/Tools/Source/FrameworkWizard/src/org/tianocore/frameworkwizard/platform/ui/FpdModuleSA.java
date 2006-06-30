package org.tianocore.frameworkwizard.platform.ui;

import java.awt.BorderLayout;

import javax.swing.DefaultCellEditor;
import javax.swing.JComboBox;
import javax.swing.JFrame;
import javax.swing.JOptionPane;
import javax.swing.JPanel;
import javax.swing.JDialog;
import javax.swing.JTabbedPane;
import javax.swing.JLabel;
import javax.swing.JScrollPane;
import javax.swing.JTable;
import javax.swing.JTextPane;
import javax.swing.JTextArea;
import javax.swing.JSplitPane;
import javax.swing.JButton;
import javax.swing.ListSelectionModel;
import javax.swing.event.ListSelectionEvent;
import javax.swing.event.ListSelectionListener;
import javax.swing.event.TableModelEvent;
import javax.swing.event.TableModelListener;
import javax.swing.table.DefaultTableModel;
import javax.swing.table.TableColumn;
import javax.swing.table.TableModel;

import org.apache.xmlbeans.XmlObject;
import org.tianocore.frameworkwizard.platform.ui.global.GlobalData;
import org.tianocore.frameworkwizard.platform.ui.global.SurfaceAreaQuery;
import org.tianocore.frameworkwizard.platform.ui.id.ModuleIdentification;
import org.tianocore.frameworkwizard.platform.ui.id.PackageIdentification;

import java.awt.FlowLayout;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.Iterator;
import java.util.ListIterator;
import java.util.Map;
import java.util.Set;

public class FpdModuleSA extends JDialog implements ActionListener {

    static JFrame frame;
    private JPanel jContentPane = null;
    private JTabbedPane jTabbedPane = null;
    private JPanel jPanel = null;
    private JPanel jPanel1 = null;
    private JLabel jLabel = null;
    private JScrollPane jScrollPane = null;
    private JTable jTable = null;
    private JPanel jPanel2 = null;
    private JScrollPane jScrollPane1 = null;
    private JTextArea jTextArea = null;
    private JPanel jPanel3 = null;
    private JSplitPane jSplitPane = null;
    private JPanel jPanel4 = null;
    private JPanel jPanel5 = null;
    private JLabel jLabel1 = null;
    private JLabel jLabel2 = null;
    private JScrollPane jScrollPane2 = null;
    private JTable jTable1 = null;
    private JScrollPane jScrollPane3 = null;
    private JTable jTable2 = null;
    private JScrollPane jScrollPane4 = null;
    private JTable jTable3 = null;
    private JPanel jPanel6 = null;
    private JPanel jPanel7 = null;
    private JScrollPane jScrollPane5 = null;
    private JTextArea jTextArea1 = null;
    private JLabel jLabel3 = null;
    private JLabel jLabel4 = null;
    private JButton jButton = null;
    private JButton jButton1 = null;
    private JLabel jLabel5 = null;
    private JButton jButton2 = null;
    private JButton jButton3 = null;
    private PartialEditableTableModel model = null;
    private LibraryTableModel model1 = null;
    private LibraryTableModel model2 = null;
    private LibraryTableModel model3 = null;
    private FpdFileContents ffc = null;
    private String moduleKey = null;
//    private int selectedRow = -1;
    private HashMap<String, String> instancePreferMap = null;
    private HashMap<String, ArrayList<String>> classInstanceMap = null;
    private ArrayList<String> classProduced = null;
//    private ArrayList<String> classConsumed = null;
    private HashMap<String, ArrayList<String>> classConsumed = null;
    /**
     * This is the default constructor
     */
    public FpdModuleSA() {
        super();
        initialize();
    }
    public FpdModuleSA(FpdFileContents ffc) {
        this();
        this.ffc = ffc;
    }
    
    public void setKey(String k){
        this.moduleKey = k;
    }

    /**
      init will be called each time FpdModuleSA object is to be shown.
      @param key Module information.
     **/
    public void init(String key) {
        //
        // display pcd for key.
        //
        model.setRowCount(0);
        int pcdCount = ffc.getPcdDataCount(key);
        if (pcdCount != 0) {
            String[][] saa = new String[pcdCount][6];
            ffc.getPcdData(key, saa);
            for (int i = 0; i < saa.length; ++i) {
                model.addRow(saa[i]);
            }
        }
        
        //
        // display lib instances already selected for key
        //
        model1.setRowCount(0);
        int instanceCount = ffc.getLibraryInstancesCount(key);
        if (instanceCount != 0) {
            String[][] saa = new String[instanceCount][5];
            ffc.getLibraryInstances(key, saa);
            for (int i = 0; i < saa.length; ++i) {
                if (getModuleId(saa[i][1] + " " + saa[i][2] + " " + saa[i][3] + " " + saa[i][4]) != null) {
                    saa[i][0] = getModuleId(saa[i][1] + " " + saa[i][2] + " " + saa[i][3] + " " + saa[i][4]).getName();
                }
                
                model1.addRow(saa[i]);
            }
        }
        //
        // display library classes that need to be resolved. also potential instances for them.
        //
        resolveLibraryInstances(key);
    }
    
    private void resolveLibraryInstances(String key) {
        ModuleIdentification mi = getModuleId(key);
        PackageIdentification[] depPkgList = null;
        try{
            Map<String, XmlObject> m = GlobalData.getNativeMsa(mi);
            SurfaceAreaQuery.setDoc(m);
            //
            // Get dependency pkg list into which we will search lib instances.
            //
            depPkgList = SurfaceAreaQuery.getDependencePkg(null);
            //
            // Get the lib class consumed, produced by this module itself.
            //
            String[] classConsumed = SurfaceAreaQuery.getLibraryClasses("ALWAYS_CONSUMED");
            
            if (this.classConsumed == null) {
                this.classConsumed = new HashMap<String, ArrayList<String>>();
            }
            
            for(int i = 0; i < classConsumed.length; ++i){
                ArrayList<String> consumedBy = this.classConsumed.get(classConsumed[i]);
                if (consumedBy == null) {
                    consumedBy = new ArrayList<String>();
                }
                consumedBy.add(key);
                this.classConsumed.put(classConsumed[i], consumedBy);
            }
            
            String[] classProduced = SurfaceAreaQuery.getLibraryClasses("ALWAYS_PRODUCED");
            if (this.classProduced == null) {
                this.classProduced = new ArrayList<String>();
            }
            for(int i = 0; i < classProduced.length; ++i){
                if (!this.classProduced.contains(classProduced[i])){
                    this.classProduced.add(classProduced[i]);
                }
            }
            //
            // Get classes unresolved
            //
//            Iterator<String> lip = this.classProduced.listIterator();
//            while(lip.hasNext()){
//                String clsProduced = lip.next();
//                this.classConsumed.remove(clsProduced);
//
//            }
            //
            // find potential instances in all dependency pkgs for classes still in classConsumed.
            //
            if (classInstanceMap == null){
                classInstanceMap = new HashMap<String, ArrayList<String>>();
            }
            Iterator<String> lic = this.classConsumed.keySet().iterator();
            while(lic.hasNext()){
                String cls = lic.next();
                if (this.classProduced.contains(cls) || classInstanceMap.containsKey(cls)) {
                    continue;
                }
                ArrayList<String> instances = getInstancesForClass(cls, depPkgList);
                if (instances.size() == 0){
                    JOptionPane.showMessageDialog(frame, "No Applicable Instance for Library Class " + 
                                                  cls + ", Platform Build will Fail.");
                }
                classInstanceMap.put(cls, instances);
                
            }
            
            showClassToResolved();
        }
        catch(Exception e) {
            e.printStackTrace();
        }
    }
    
    private ArrayList<String> getInstancesForClass(String cls, PackageIdentification[] depPkgList) throws Exception{
        ArrayList<String> al = new ArrayList<String>();
        
        for (int i = 0; i < depPkgList.length; ++i) {
            Set<ModuleIdentification> smi = GlobalData.getModules(depPkgList[i]);
            Iterator ismi = smi.iterator();
            while(ismi.hasNext()) {
                ModuleIdentification mi = (ModuleIdentification)ismi.next();
                
                String[] clsProduced = getClassProduced(mi);
                
                boolean isPotential = false;
                for (int j = 0; j < clsProduced.length; ++j) {
                    if (clsProduced[j] == null) {
                        continue;
                    }
                    if (clsProduced[j].equals(cls)){
                        isPotential = true;
                    }
                    if (classProduced.contains(clsProduced[j])) {
                        isPotential = false;
                        break;
                    }
                }
                if (isPotential) {
                    al.add(mi.getGuid() + " " + mi.getVersion() + " " + 
                           depPkgList[i].getGuid() + " " + depPkgList[i].getVersion());
                }
            }
        }
        
        return al;
    }
    
    private void removeInstance(String key) {
        ModuleIdentification mi = getModuleId(key); 
        //
        // remove pcd information of instance from current ModuleSA
        //
        ffc.removePcdDataFromLibraryInstance(moduleKey, key);
        //
        // remove class produced by this instance and add back these produced class to be bound.
        //
        String[] clsProduced = getClassProduced(mi);
        for (int i = 0; i < clsProduced.length; ++i) {
            
            classProduced.remove(clsProduced[i]);
        }
        //
        // remove class consumed by this instance. we do not need to bound it now.
        //
        String[] clsConsumed = getClassConsumed(mi);
        for (int i = 0; i < clsConsumed.length; ++i) {
            ArrayList<String> al = classConsumed.get(clsConsumed[i]);
            
            if (al == null ) {
                classConsumed.remove(clsConsumed[i]);
                continue;
            }
            al.remove(key);
            if (al.size() == 0) {
                classConsumed.remove(clsConsumed[i]);
            }
           
        }

        showClassToResolved();
        
    }
    
    private ModuleIdentification getModuleId(String key){
        //
        // Get ModuleGuid, ModuleVersion, PackageGuid, PackageVersion into string array.
        //
        String[] keyPart = key.split(" ");
        Set<PackageIdentification> spi = GlobalData.getPackageList();
        Iterator ispi = spi.iterator();
        
        while(ispi.hasNext()) {
            PackageIdentification pi = (PackageIdentification)ispi.next();
            if ( !pi.getGuid().equals(keyPart[2]) || !pi.getVersion().equals(keyPart[3])){
                continue;
            }
            Set<ModuleIdentification> smi = GlobalData.getModules(pi);
            Iterator ismi = smi.iterator();
            while(ismi.hasNext()) {
                ModuleIdentification mi = (ModuleIdentification)ismi.next();
                if (mi.getGuid().equals(keyPart[0]) && mi.getVersion().equals(keyPart[1])){
                    return mi;
                }
            }
        }
        return null;
    }
    
    private String[] getClassProduced(ModuleIdentification mi){
        
        try{
            Map<String, XmlObject> m = GlobalData.getNativeMsa(mi);
            SurfaceAreaQuery.setDoc(m);
            String[] clsProduced = SurfaceAreaQuery.getLibraryClasses("ALWAYS_PRODUCED");
            return clsProduced;
            
        }catch (Exception e) {
            e.printStackTrace();
        }
        return new String[0];
        
    }
    
    private String[] getClassConsumed(ModuleIdentification mi){
        
        String[] clsConsumed = null;
        try{
            Map<String, XmlObject> m = GlobalData.getNativeMsa(mi);
            SurfaceAreaQuery.setDoc(m);
            clsConsumed = SurfaceAreaQuery.getLibraryClasses("ALWAYS_CONSUMED");
            
        }catch (Exception e) {
            e.printStackTrace();
        }
        return clsConsumed;
    }
    
    private void showClassToResolved(){
        model2.setRowCount(0);
        if (classConsumed.size() == 0) {
            return;
        }
        Iterator<String> li = classConsumed.keySet().iterator();
        while(li.hasNext()){
            
            String[] s = {li.next()};
            if (classConsumed.get(s[0]) == null) {
                continue;
            }
            if (classConsumed.get(s[0]).size() == 0) {
                continue;
            }
            if (!classProduced.contains(s[0])){
                model2.addRow(s);
            }
        }
        model3.setRowCount(0);
    }
    /**
     * This method initializes this
     * 
     * @return void
     */
    private void initialize() {
        this.setSize(664, 515);
        this.setModal(true);
        this.setTitle("Module Settings");
        this.setContentPane(getJContentPane());
    }

    /**
     * This method initializes jContentPane
     * 
     * @return javax.swing.JPanel
     */
    private JPanel getJContentPane() {
        if (jContentPane == null) {
            jContentPane = new JPanel();
            jContentPane.setLayout(new BorderLayout());
            jContentPane.add(getJTabbedPane(), java.awt.BorderLayout.CENTER);
            jContentPane.add(getJPanel3(), java.awt.BorderLayout.SOUTH);
        }
        return jContentPane;
    }

    /**
     * This method initializes jTabbedPane	
     * 	
     * @return javax.swing.JTabbedPane	
     */
    private JTabbedPane getJTabbedPane() {
        if (jTabbedPane == null) {
            jTabbedPane = new JTabbedPane();
            jTabbedPane.addTab("PCD Build Definition", null, getJPanel(), null);
            jTabbedPane.addTab("Libraries", null, getJPanel1(), null);
        }
        return jTabbedPane;
    }

    /**
     * This method initializes jPanel	
     * 	
     * @return javax.swing.JPanel	
     */
    private JPanel getJPanel() {
        if (jPanel == null) {
            jLabel = new JLabel();
            jLabel.setText("PcdData");
            jPanel = new JPanel();
            jPanel.setLayout(new BorderLayout());
            jPanel.add(jLabel, java.awt.BorderLayout.NORTH);
            jPanel.add(getJScrollPane(), java.awt.BorderLayout.CENTER);
            jPanel.add(getJPanel2(), java.awt.BorderLayout.SOUTH);
            jPanel.addComponentListener(new java.awt.event.ComponentAdapter() {
                public void componentShown(java.awt.event.ComponentEvent e) {
                    init(moduleKey);
                }
            });
            
        }
        return jPanel;
    }

    /**
     * This method initializes jPanel1	
     * 	
     * @return javax.swing.JPanel	
     */
    private JPanel getJPanel1() {
        if (jPanel1 == null) {
            jPanel1 = new JPanel();
            jPanel1.setLayout(new BorderLayout());
            jPanel1.add(getJSplitPane(), java.awt.BorderLayout.NORTH);
            jPanel1.add(getJPanel6(), java.awt.BorderLayout.SOUTH);
            jPanel1.add(getJPanel7(), java.awt.BorderLayout.CENTER);
            jPanel1.addComponentListener(new java.awt.event.ComponentAdapter() {
                public void componentShown(java.awt.event.ComponentEvent e) {
                }
            });
        }
        return jPanel1;
    }

    /**
     * This method initializes jScrollPane	
     * 	
     * @return javax.swing.JScrollPane	
     */
    private JScrollPane getJScrollPane() {
        if (jScrollPane == null) {
            jScrollPane = new JScrollPane();
            jScrollPane.setViewportView(getJTable());
        }
        return jScrollPane;
    }

    /**
     * This method initializes jTable	
     * 	
     * @return javax.swing.JTable	
     */
    private JTable getJTable() {
        if (jTable == null) {
            model = new PartialEditableTableModel();
            jTable = new JTable(model);
            model.addColumn("CName");
            model.addColumn("TokenSpaceGUID");
            model.addColumn("ItemType");
            model.addColumn("Token");
            model.addColumn("DataType");
            model.addColumn("DefaultValue");
                        
            jTable.setSelectionMode(ListSelectionModel.SINGLE_SELECTION);
            jTable.getSelectionModel().addListSelectionListener(new ListSelectionListener(){
                public void valueChanged(ListSelectionEvent e) {
                    int selectedRow = -1;
                    if (e.getValueIsAdjusting()){
                        return;
                    }
                    ListSelectionModel lsm = (ListSelectionModel)e.getSource();
                    if (lsm.isSelectionEmpty()) {
                        return;
                    }
                    else{
                        selectedRow = lsm.getMinSelectionIndex();
                        
                        
                    }
                }
            });
            
            jTable.getModel().addTableModelListener(new TableModelListener() {
                public void tableChanged(TableModelEvent arg0) {
                    // TODO Auto-generated method stub
                    int row = arg0.getFirstRow();
                    TableModel m = (TableModel)arg0.getSource();
                    if (arg0.getType() == TableModelEvent.INSERT) {
                        //
                        // Set combo box values for item type according to pcd values added.
                        //
                        
                    }
                    if (arg0.getType() == TableModelEvent.UPDATE){
                        //ToDo Data Validition check.
                        
                    }
                }
            });
        }
        return jTable;
    }

    /**
     * This method initializes jPanel2	
     * 	
     * @return javax.swing.JPanel	
     */
    private JPanel getJPanel2() {
        if (jPanel2 == null) {
            jLabel5 = new JLabel();
            jLabel5.setText("PCD Description");
            jPanel2 = new JPanel();
            jPanel2.add(jLabel5, null);
            jPanel2.add(getJScrollPane1(), null);
        }
        return jPanel2;
    }

    /**
     * This method initializes jScrollPane1	
     * 	
     * @return javax.swing.JScrollPane	
     */
    private JScrollPane getJScrollPane1() {
        if (jScrollPane1 == null) {
            jScrollPane1 = new JScrollPane();
            jScrollPane1.setPreferredSize(new java.awt.Dimension(500,100));
            jScrollPane1.setViewportView(getJTextArea());
        }
        return jScrollPane1;
    }

    /**
     * This method initializes jTextArea	
     * 	
     * @return javax.swing.JTextArea	
     */
    private JTextArea getJTextArea() {
        if (jTextArea == null) {
            jTextArea = new JTextArea();
            jTextArea.setEditable(false);
        }
        return jTextArea;
    }

    /**
     * This method initializes jPanel3	
     * 	
     * @return javax.swing.JPanel	
     */
    private JPanel getJPanel3() {
        if (jPanel3 == null) {
            FlowLayout flowLayout = new FlowLayout();
            flowLayout.setAlignment(java.awt.FlowLayout.RIGHT);
            jPanel3 = new JPanel();
            jPanel3.setLayout(flowLayout);
            jPanel3.add(getJButton2(), null);
            jPanel3.add(getJButton3(), null);
        }
        return jPanel3;
    }

    /**
     * This method initializes jSplitPane	
     * 	
     * @return javax.swing.JSplitPane	
     */
    private JSplitPane getJSplitPane() {
        if (jSplitPane == null) {
            jSplitPane = new JSplitPane();
            jSplitPane.setDividerLocation(200);
            jSplitPane.setLeftComponent(getJPanel4());
            jSplitPane.setRightComponent(getJPanel5());
            jSplitPane.setPreferredSize(new java.awt.Dimension(202,200));
        }
        return jSplitPane;
    }

    /**
     * This method initializes jPanel4	
     * 	
     * @return javax.swing.JPanel	
     */
    private JPanel getJPanel4() {
        if (jPanel4 == null) {
            jLabel1 = new JLabel();
            jLabel1.setText("Library Classes Consumed");
            jPanel4 = new JPanel();
            jPanel4.add(jLabel1, null);
            jPanel4.add(getJScrollPane3(), null);
        }
        return jPanel4;
    }

    /**
     * This method initializes jPanel5	
     * 	
     * @return javax.swing.JPanel	
     */
    private JPanel getJPanel5() {
        if (jPanel5 == null) {
            jLabel2 = new JLabel();
            jLabel2.setText("Instances Available");
            jPanel5 = new JPanel();
            jPanel5.add(jLabel2, null);
            jPanel5.add(getJScrollPane4(), null);
        }
        return jPanel5;
    }

    /**
     * This method initializes jScrollPane2	
     * 	
     * @return javax.swing.JScrollPane	
     */
    private JScrollPane getJScrollPane2() {
        if (jScrollPane2 == null) {
            jScrollPane2 = new JScrollPane();
            jScrollPane2.setPreferredSize(new java.awt.Dimension(453,150));
            jScrollPane2.setViewportView(getJTable1());
        }
        return jScrollPane2;
    }

    /**
     * This method initializes jTable1	
     * 	
     * @return javax.swing.JTable	
     */
    private JTable getJTable1() {
        if (jTable1 == null) {
            model1 = new LibraryTableModel();
            model1.addColumn("Name");
            model1.addColumn("ModuleGUID");
            model1.addColumn("ModuleVersion");
            model1.addColumn("PackageGUID");
            model1.addColumn("PackageVersion");
            jTable1 = new JTable(model1);
            
            jTable1.setSelectionMode(ListSelectionModel.SINGLE_SELECTION);
            jTable1.getSelectionModel().addListSelectionListener(new ListSelectionListener(){
                public void valueChanged(ListSelectionEvent e) {
                    int selectedRow1 = -1;
                    if (e.getValueIsAdjusting()){
                        return;
                    }
                    ListSelectionModel lsm = (ListSelectionModel)e.getSource();
                    if (lsm.isSelectionEmpty()) {
                        return;
                    }
                    else{
                        selectedRow1 = lsm.getMinSelectionIndex();
                        
                        
                    }
                }
            });
            
            
        }
        return jTable1;
    }

    /**
     * This method initializes jScrollPane3	
     * 	
     * @return javax.swing.JScrollPane	
     */
    private JScrollPane getJScrollPane3() {
        if (jScrollPane3 == null) {
            jScrollPane3 = new JScrollPane();
            jScrollPane3.setPreferredSize(new java.awt.Dimension(200,170));
            jScrollPane3.setViewportView(getJTable2());
        }
        return jScrollPane3;
    }

    /**
     * This method initializes jTable2	
     * 	
     * @return javax.swing.JTable	
     */
    private JTable getJTable2() {
        if (jTable2 == null) {
            model2 = new LibraryTableModel();
            model2.addColumn("LibraryClass");
            jTable2 = new JTable(model2);
            
            jTable2.setSelectionMode(ListSelectionModel.SINGLE_SELECTION);
            jTable2.getSelectionModel().addListSelectionListener(new ListSelectionListener(){
                public void valueChanged(ListSelectionEvent e) {
                    if (e.getValueIsAdjusting()){
                        return;
                    }
                    ListSelectionModel lsm = (ListSelectionModel)e.getSource();
                    if (lsm.isSelectionEmpty()) {
                        return;
                    }
                    else{
                        int selectedRow2 = lsm.getMinSelectionIndex();
                        if (selectedRow2 < 0) {
                            return;
                        }
                        //
                        // display potential lib instances according to class selection
                        //
                        model3.setRowCount(0);
                        String cls = model2.getValueAt(selectedRow2, 0).toString();
                        ArrayList<String> al = classInstanceMap.get(cls);
                        ListIterator<String> li = al.listIterator();
                        while(li.hasNext()) {
                            String instance = li.next();
                            String[] s = {"", "", "", "", ""};
                            if (getModuleId(instance) != null) {
                                s[0] = getModuleId(instance).getName();
                            }
                            
                            String[] instancePart = instance.split(" ");
                            for (int i = 0; i < instancePart.length; ++i){
                                s[i+1] = instancePart[i];
                            }
                            model3.addRow(s);
                        }
                        
                    }
                }
            });
        }
        return jTable2;
    }

    /**
     * This method initializes jScrollPane4	
     * 	
     * @return javax.swing.JScrollPane	
     */
    private JScrollPane getJScrollPane4() {
        if (jScrollPane4 == null) {
            jScrollPane4 = new JScrollPane();
            jScrollPane4.setPreferredSize(new java.awt.Dimension(450,170));
            jScrollPane4.setViewportView(getJTable3());
        }
        return jScrollPane4;
    }

    /**
     * This method initializes jTable3	
     * 	
     * @return javax.swing.JTable	
     */
    private JTable getJTable3() {
        if (jTable3 == null) {
            model3 = new LibraryTableModel();
            model3.addColumn("Name");
            model3.addColumn("ModuleGUID");
            model3.addColumn("ModuleVersion");
            model3.addColumn("PackageGUID");
            model3.addColumn("PackageVersion");
            jTable3 = new JTable(model3);
            
            jTable3.setSelectionMode(ListSelectionModel.SINGLE_SELECTION);
            jTable3.getSelectionModel().addListSelectionListener(new ListSelectionListener(){
                public void valueChanged(ListSelectionEvent e) {
                    int selectedRow3 = -1;
                    if (e.getValueIsAdjusting()){
                        return;
                    }
                    ListSelectionModel lsm = (ListSelectionModel)e.getSource();
                    if (lsm.isSelectionEmpty()) {
                        return;
                    }
                    else{
                        selectedRow3 = lsm.getMinSelectionIndex();
                        
                        
                    }
                }
            });
        }
        return jTable3;
    }

    /**
     * This method initializes jPanel6	
     * 	
     * @return javax.swing.JPanel	
     */
    private JPanel getJPanel6() {
        if (jPanel6 == null) {
            jPanel6 = new JPanel();
        }
        return jPanel6;
    }

    /**
     * This method initializes jPanel7	
     * 	
     * @return javax.swing.JPanel	
     */
    private JPanel getJPanel7() {
        if (jPanel7 == null) {
            jLabel4 = new JLabel();
            jLabel4.setText("Instance Description");
            jLabel3 = new JLabel();
            jLabel3.setText("Selected Instances");
            jPanel7 = new JPanel();
            jPanel7.add(jLabel4, null);
            jPanel7.add(getJScrollPane5(), null);
            jPanel7.add(getJButton(), null);
            jPanel7.add(getJButton1(), null);
            jPanel7.add(jLabel3, null);
            jPanel7.add(getJScrollPane2(), null);
        }
        return jPanel7;
    }

    /**
     * This method initializes jScrollPane5	
     * 	
     * @return javax.swing.JScrollPane	
     */
    private JScrollPane getJScrollPane5() {
        if (jScrollPane5 == null) {
            jScrollPane5 = new JScrollPane();
            jScrollPane5.setPreferredSize(new java.awt.Dimension(300,50));
            jScrollPane5.setViewportView(getJTextArea1());
        }
        return jScrollPane5;
    }

    /**
     * This method initializes jTextArea1	
     * 	
     * @return javax.swing.JTextArea	
     */
    private JTextArea getJTextArea1() {
        if (jTextArea1 == null) {
            jTextArea1 = new JTextArea();
            jTextArea1.setEditable(false);
        }
        return jTextArea1;
    }

    /**
     * This method initializes jButton	
     * 	
     * @return javax.swing.JButton	
     */
    private JButton getJButton() {
        if (jButton == null) {
            jButton = new JButton();
            jButton.setPreferredSize(new java.awt.Dimension(80,20));
            jButton.setText("Add");
            jButton.addActionListener(new java.awt.event.ActionListener() {
                public void actionPerformed(java.awt.event.ActionEvent e) {
                    int row = jTable3.getSelectedRow();
                    if (row < 0) {
                        return;
                    }
                    Object[] s = {model3.getValueAt(row, 0), model3.getValueAt(row, 1),
                                  model3.getValueAt(row, 2), model3.getValueAt(row, 3),
                                  model3.getValueAt(row, 4)};
                    model1.addRow(s);
                    String instanceValue = model3.getValueAt(row, 1) + " " + 
                    model3.getValueAt(row, 2) + " " +
                    model3.getValueAt(row, 3) + " " +
                    model3.getValueAt(row, 4);
                    //
                    // Add pcd information of selected instance to current moduleSA
                    //
                    ffc.addFrameworkModulesPcdBuildDefs(getModuleId(instanceValue), ffc.getModuleSA(moduleKey));
                    
                    resolveLibraryInstances(instanceValue);
                }
            });
        }
        return jButton;
    }

    /**
     * This method initializes jButton1	
     * 	
     * @return javax.swing.JButton	
     */
    private JButton getJButton1() {
        if (jButton1 == null) {
            jButton1 = new JButton();
            jButton1.setPreferredSize(new java.awt.Dimension(80,20));
            jButton1.setText("Delete");
            jButton1.addActionListener(new java.awt.event.ActionListener() {
                public void actionPerformed(java.awt.event.ActionEvent e) {
                    int row = jTable1.getSelectedRow();
                    if (row < 0) {
                        return;
                    }
                    removeInstance(model1.getValueAt(row, 1) + " " + 
                                   model1.getValueAt(row, 2) + " " +
                                   model1.getValueAt(row, 3) + " " +
                                   model1.getValueAt(row, 4));
                    model1.removeRow(row);
                    
                }
            });
        }
        return jButton1;
    }

    /**
     * This method initializes jButton2	
     * 	
     * @return javax.swing.JButton	
     */
    private JButton getJButton2() {
        if (jButton2 == null) {
            jButton2 = new JButton();
            jButton2.setPreferredSize(new java.awt.Dimension(80,20));
            jButton2.setText("Ok");
            jButton2.addActionListener(this);
        }
        return jButton2;
    }

    /**
     * This method initializes jButton3	
     * 	
     * @return javax.swing.JButton	
     */
    private JButton getJButton3() {
        if (jButton3 == null) {
            jButton3 = new JButton();
            jButton3.setPreferredSize(new java.awt.Dimension(80,20));
            jButton3.setText("Cancel");
            jButton3.setVisible(false);
        }
        return jButton3;
    }
    public void actionPerformed(ActionEvent arg0) {

        if (arg0.getSource() == jButton2) {
            ffc.removeLibraryInstances(moduleKey);
            for (int i = 0; i < model1.getRowCount(); ++i) {
                String mg = model1.getValueAt(i, 1)+"";
                String mv = model1.getValueAt(i, 2)+"";
                String pg = model1.getValueAt(i, 3)+"";
                String pv = model1.getValueAt(i, 4)+"";
                ffc.genLibraryInstance(mg, mv, pg, pv, moduleKey);
            }
            this.setVisible(false);
        }
    }

}  //  @jve:decl-index=0:visual-constraint="10,10"

class PartialEditableTableModel extends DefaultTableModel {
    public boolean isCellEditable(int row, int col) {
        switch (col){
        case 2:
            return true;
        default:
            return false; 
        }
           
    }
}

class LibraryTableModel extends DefaultTableModel {
    public boolean isCellEditable(int row, int col) {
        return false;
    }
}
