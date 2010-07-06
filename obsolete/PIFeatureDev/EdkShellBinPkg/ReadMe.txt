The binaries of EdkShellBinPkg are generated with EDK-Shell 1.04 release and build with Edk Compatibility & BaseTools Package
(r4927)

The following steps can help to re-generate these binaries for customization:
1. Check out EdkCompatibilityPkg (r4927) to a directory EdkCompatibilityPkg in workspace (svn https://edk2.tianocore.org/svn/edk2/trunk/edk2/EdkCompatibilityPkg). 
2. Update to the newest BaseTools package. (r4927 or later)
2. Download EfiShell 1.04.zip from EDK Shell official release https://efi-shell.tianocore.org/servlets/ProjectDocumentList?folderID=52&expandFolder=52&folderID=45
3. Unzip it to be a sub-directory in EdkCompatibilityPkg , i.e. c:\EdkII\EdkCompatibilityPkg\Shell
4. Apply a hot fix Shell_HotFix.diff under EdkShellBinPkg\GenBin directory. This is mainly to fix the unaligned device path node access in shell binary and a rare INF format issue in ver.inf. This patch will be integrated into the later official release.
5. Under workspace directory (i.e. c:\EdkII), execute:
   build -a IA32 -a X64 -a IPF -p EdkShellBinPkg\GenBin\EdkShellPkg.dsc -t WINDDK3790x1830
   The use of WINDDK instead of MYTOOLS is due to the fact that EDK shell source 1.04 is not
   VS2005 clean.

6. Copy the binaries from Build directory to this package. Typically the EFI binary
   of EdkCompatibility\Shell\$(INF_BASENAME).inf is generated at:
   Build\EdkShellPkg\DEBUG_WINDDK3790x1830\$(ARCH)\EdkCompatibility\Shell\$(INF_BASENAME)\OUTPUT\$(BASENAME).efi
   For example:
   The x64 EFI image of EdkCompatibility\Shell\ver\ver.inf is generated at:
   Build\EdkShellPkg\DEBUG_WINDDK3790x1830\X64\EdkCompatibilityPkg\Shell\ver\Ver\OUTPUT\ver.efi

Note: Other\Maintained\Application\Shell\Shell.inf corresponds to Minimum shell binaries.
      Other\Maintained\Application\Shell\ShellFull.inf corresponds to Full Shell binaries.