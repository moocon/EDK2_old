/*
 * 
 * Copyright 2001-2005 The Ant-Contrib project
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
package net.sf.antcontrib.cpptasks.intel;

import java.util.Vector;

import net.sf.antcontrib.cpptasks.compiler.CommandLineAslcompiler;
import net.sf.antcontrib.cpptasks.compiler.LinkType;
import net.sf.antcontrib.cpptasks.compiler.Linker;
import net.sf.antcontrib.cpptasks.devstudio.DevStudioLinker;
/**
 * Adapter for Intel ASL compiler
 *
 */
public final class IntelWin32Aslcompiler extends CommandLineAslcompiler{
   private final static String[] sourceExtensions = new String[]{".asl"};
   private final static String[] headerExtensions = new String[]{};
   private final static String[] defaultflags = new String[]{};
   private static final IntelWin32Aslcompiler instance = new IntelWin32Aslcompiler("iasl", 
       sourceExtensions, headerExtensions, false);
   
   /**
    * Gets gcc adapter
    */
   public static IntelWin32Aslcompiler getInstance() {
       return instance;
   }
   
   /**
    * Private constructor. Use GccAssembler.getInstance() to get singleton
    * instance of this class. 
    */
   private IntelWin32Aslcompiler(String command, String[] sourceExtensions,
       String[] headerExtensions, boolean isLibtool){
       super(command, null, sourceExtensions, headerExtensions, 
               ".aml");
   }
   public void addImpliedArgs(Vector args, boolean debug,
       Boolean defaultflag){
       if (defaultflag != null && defaultflag.booleanValue()) {
           for (int i = 0; i < defaultflags.length; i++) {
               args.addElement(defaultflags[i]);
           }
       }
   }
   public int getMaximumCommandLength() {
       return Integer.MAX_VALUE;
   }
   public Linker getLinker(LinkType linkType) {
     return DevStudioLinker.getInstance().getLinker(linkType);
   }
}