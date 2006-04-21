/*++

Copyright (c) 2006, Intel Corporation                                                         
All rights reserved. This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution.  The full text of the license may be found at        
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

Module Name:
  EdkDxeDepex.h

Abstract:
  This include file is only used in *.DXS files. Do not use this 
  include file in normal DXE code.

  Depex - Dependency Expresion

  The BNF grammar is thus:
     <depex>   ::= before GUID       
                 | after GUID
                 | SOR <bool>
                 | <bool>            
     <bool>    ::= <bool> and <term> 
                 | <bool> or <term>  
                 | <term>            
     <term>    ::= not <factor>      
                 | <factor>          
     <factor>  ::= <bool>            
                 | <boolval>         
                 | <depinst>         
                 | <termval>         
     <boolval> ::= true              
                 | false             
     <depinst> ::= push GUID         
     <termval> ::= end               

--*/

#ifndef __EDK_DXE_DEPEX_H__
#define __EDK_DXE_DEPEX_H__

#include <EdkDxe.h>

//
// The Depex grammer needs the following strings so we must undo
// any pre-processor redefinitions
//
#undef DEPENDENCY_START   
#undef BEFORE             
#undef AFTER              
#undef SOR                               
#undef AND                
#undef OR                 
#undef NOT                
#undef TRUE               
#undef FALSE              
#undef DEPENDENCY_END     

#endif
