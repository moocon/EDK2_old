#
# Copyright (c) 2006, Intel Corporation
# All rights reserved. This program and the accompanying materials
# are licensed and made available under the terms and conditions of the BSD License
# which accompanies this distribution.  The full text of the license may be found at
# http://opensource.org/licenses/bsd-license.php
# 
# THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
# WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

# Setup the environment for unix-like systems running a bash-like shell.
# This file must be "sourced" not executed. For example: ". edksetup.sh"

if [ "$WORKSPACE" == "" ]
then
  echo "Please set WORKSPACE before sourcing this script."
else
if [ "$JAVA_HOME" == "" ]
then
  echo "Please set JAVA_HOME before sourcing this script."
else

# These should be ok as they are.
#export ANT_HOME=$WORKSPACE/Tools/bin/apache-ant
#export XMLBEANS_HOME=$WORKSPACE/Tools/bin/xmlbeans
export CLASSPATH=$WORKSPACE/Tools/Jars/SurfaceArea.jar:$WORKSPACE/Tools/Jars/frameworktasks.jar:$WORKSPACE/Tools/Jars/saxon8.jar:$WORKSPACE/Tools/Jars/cpptasks.jar:$WORKSPACE/Tools/Jars/GenBuild.jar:$XMLBEANS_HOME/lib/resolver.jar:$XMLBEANS_HOME/lib/xbean.jar:$XMLBEANS_HOME/lib/xmlpublic.jar:$XMLBEANS_HOME/lib/jsr173_1.0_api.jar:$XMLBEANS_HOME/lib/saxon8.jar:$XMLBEANS_HOME/lib/xbean_xpath.jar
export Framework_Tools_Path=$WORKSPACE/Tools/bin
export PATH=$Framework_Tools_Path:$ANT_HOME/bin:$JAVA_HOME/bin:$PATH

# Handle any particulars down here.
case "`uname`" in
  CYGWIN*) 
    # Convert paths to windows format.
    export WORKSPACE=`cygpath -w $WORKSPACE`
    export CLASSPATH=`cygpath -w -p $CLASSPATH`
    ;;
esac

# Now we need to build the tools.
echo "If you have not done so, please build the tools by issuing 'ant -f \$WORKSPACE/Tools/build.xml'."
fi
fi
