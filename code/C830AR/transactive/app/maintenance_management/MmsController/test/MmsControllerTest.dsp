# Microsoft Developer Studio Project File - Name="MmsControllerTest" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Console Application" 0x0103

CFG=MmsControllerTest - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "MmsControllerTest.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "MmsControllerTest.mak" CFG="MmsControllerTest - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "MmsControllerTest - Win32 Release" (based on "Win32 (x86) Console Application")
!MESSAGE "MmsControllerTest - Win32 Debug" (based on "Win32 (x86) Console Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "MmsControllerTest - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /c
# ADD CPP /nologo /MD /W3 /GR /GX /O2 /I "..\..\..\..\.." /I "..\..\..\..\..\cots\omniORB\omniORB_4.0.3\Winnt40\omni\include" /I "..\..\..\..\..\core\message\IDL\src" /I "..\..\..\..\..\cots\CppUnit\src" /I "..\..\..\.." /I "src" /I "..\..\..\..\cots\omniORB\omniORB_4.0.3\Winnt40\omni\include" /I "..\..\..\..\core\message\IDL\src" /I "..\..\..\..\cots\CppUnit\src" /I "..\..\..\..\core\message\src" /I "..\..\..\..\core\message\types" /I "..\..\..\..\core\exceptions\src" /D "NDEBUG" /D "STUBBED_PA_AGENT" /D "_DEBUG" /D "__WIN32__" /D "__x86__" /D "_NT_" /D __OSVERSION__=4 /D "WIN32" /D "_CONSOLE" /D "_MBCS" /FD /Zm200 /c
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386 /libpath:"../../../../cots/omniORB/omniORB_4.0.3/Winnt40/omni/lib/x86_win32" /libpath:"../../../../cots/oracle/920/winnt/lib"

!ELSEIF  "$(CFG)" == "MmsControllerTest - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 2
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /GZ /c
# ADD CPP /nologo /MDd /W3 /Gm /GR /GX /ZI /Od /I ".\src" /I "..\..\..\.." /I "..\..\..\..\..\cots\omniORB\omniORB_4.0.5\include" /I "..\..\..\..\core\message\IDL\src" /I "..\..\..\..\..\cots\CppUnit\src" /I "..\..\..\..\core\message\src" /I "..\..\..\..\core\message\types" /I "..\..\..\..\core\exceptions\src" /I "src" /I "../../../../../../sdk/include/transactive" /I "../../../../../../sdk/include/transactive/core/exceptions/IDL/src" /I "../../../../../../sdk/include/transactive/core/message/IDL/src" /FI"Aliases.h" /D TA_ASSERT_FAIL=EXCEPTION /D "_AFXDLL" /D "STUBBED_PA_AGENT" /D "_DEBUG" /D "__WIN32__" /D "__x86__" /D "_NT_" /D __OSVERSION__=4 /D "WIN32" /D "_CONSOLE" /D "_MBCS" /FD /GZ /Zm200 /c
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0xc09 /i "..\\" /d "_DEBUG" /d "_AFXDLL"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386 /pdbtype:sept
# ADD LINK32 omnithread30_rtd.lib omniORB403_rtd.lib omniDynamic403_rtd.lib COS403_rtd.lib ws2_32.lib oci.lib htmlhelp.lib Rpcrt4.lib omnithread30_rtd.lib omniORB403_rtd.lib omniDynamic403_rtd.lib COS403_rtd.lib oci.lib Rpcrt4.lib Msimg32.lib TA_Core.lib acemfcd.lib TA_Bus.lib /nologo /subsystem:console /debug /machine:I386 /out:"Debug/MmsControllerUnitTest.exe" /pdbtype:sept /libpath:"..\..\..\..\cots\omniORB\omniORB_4.0.3\Winnt40\omni\lib\x86_win32" /libpath:"..\..\..\..\cots\oracle\920\winnt\lib" /libpath:"..\..\..\..\cots\HTMLHelp\lib" /libpath:"..\..\..\..\tools\build\core_library\Debug" /libpath:"..\..\..\..\tools\build\bus_library\Debug" /libpath:"..\..\..\..\cots/ACE/5_3/ACE_wrappers/ace"
# SUBTRACT LINK32 /pdb:none /incremental:no

!ENDIF 

# Begin Target

# Name "MmsControllerTest - Win32 Release"
# Name "MmsControllerTest - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\src\Common.cpp
# End Source File
# Begin Source File

SOURCE=.\src\main.cpp
# End Source File
# Begin Source File

SOURCE=.\src\MmsControllerModelTestCases.cpp
# End Source File
# Begin Source File

SOURCE=.\src\WindowStub.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\src\Aliases.h
# End Source File
# Begin Source File

SOURCE=.\src\Common.h
# End Source File
# Begin Source File

SOURCE=.\src\MmsControllerModelTestCases.h
# End Source File
# Begin Source File

SOURCE=.\src\StdAfx.h
# End Source File
# Begin Source File

SOURCE=.\src\WindowStub.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=..\res\dva_icon.ico
# End Source File
# Begin Source File

SOURCE=..\res\EquipmentStatusViewer.ico
# End Source File
# Begin Source File

SOURCE=..\res\live_icon.ico
# End Source File
# Begin Source File

SOURCE=..\res\MmsController.ico
# End Source File
# Begin Source File

SOURCE=..\res\music_occ.ico
# End Source File
# Begin Source File

SOURCE=..\res\music_off.ico
# End Source File
# Begin Source File

SOURCE=..\res\music_stn.ico
# End Source File
# Begin Source File

SOURCE=..\res\play.ico
# End Source File
# Begin Source File

SOURCE=..\res\stop.ico
# End Source File
# End Group
# Begin Group "MmsController source"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\src\CachedConfig.cpp
# End Source File
# Begin Source File

SOURCE=..\src\CachedConfig.h
# End Source File
# Begin Source File

SOURCE=..\src\IRightsChangeListener.h
# End Source File
# Begin Source File

SOURCE=..\src\MmsController.cpp

!IF  "$(CFG)" == "MmsControllerTest - Win32 Release"

!ELSEIF  "$(CFG)" == "MmsControllerTest - Win32 Debug"

# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\src\MmsController.h
# End Source File
# Begin Source File

SOURCE=..\src\MmsController.rc
# End Source File
# Begin Source File

SOURCE=..\src\MmsControllerDlg.cpp
# End Source File
# Begin Source File

SOURCE=..\src\MmsControllerDlg.h
# End Source File
# Begin Source File

SOURCE=..\src\MmsControllerGUI.cpp
# End Source File
# Begin Source File

SOURCE=..\src\MmsControllerGUI.h
# End Source File
# Begin Source File

SOURCE=..\src\MmsControllerModel.cpp

!IF  "$(CFG)" == "MmsControllerTest - Win32 Release"

!ELSEIF  "$(CFG)" == "MmsControllerTest - Win32 Debug"

# ADD CPP /I "."

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\src\MmsControllerModel.h
# End Source File
# Begin Source File

SOURCE=..\src\MmsEntityFactory.cpp
# End Source File
# Begin Source File

SOURCE=..\src\MmsEntityFactory.h
# End Source File
# Begin Source File

SOURCE=..\src\resource.h
# End Source File
# Begin Source File

SOURCE=..\src\RightsChangedWindowNotificationHandler.cpp
# End Source File
# Begin Source File

SOURCE=..\src\RightsChangedWindowNotificationHandler.h
# End Source File
# Begin Source File

SOURCE=..\src\StdAfx.cpp
# End Source File
# Begin Source File

SOURCE=..\src\StdAfx.h
# End Source File
# Begin Source File

SOURCE=..\src\SubsystemLimitedScadaTree.cpp
# End Source File
# Begin Source File

SOURCE=..\src\SubsystemLimitedScadaTree.h
# End Source File
# Begin Source File

SOURCE=..\src\VersionGen.cpp
# End Source File
# End Group
# Begin Group "Stubs"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\src\AuditMessageSenderStub.cpp
# End Source File
# Begin Source File

SOURCE=.\src\AuditMessageSenderStub.h
# End Source File
# Begin Source File

SOURCE=.\src\EntityAccessFactoryStub.cpp
# End Source File
# Begin Source File

SOURCE=.\src\EntityAccessFactoryStub.h
# End Source File
# Begin Source File

SOURCE=.\src\IDataNodeProxyStub.cpp
# End Source File
# Begin Source File

SOURCE=.\src\IDataNodeProxyStub.h
# End Source File
# Begin Source File

SOURCE=.\src\IResourceStub.h
# End Source File
# Begin Source File

SOURCE=.\src\MessagePublicationManagerStub.cpp
# End Source File
# Begin Source File

SOURCE=.\src\MessagePublicationManagerStub.h
# End Source File
# Begin Source File

SOURCE=.\src\MmsInhibitAgentNamedObjectStub.cpp
# End Source File
# Begin Source File

SOURCE=.\src\MmsInhibitAgentNamedObjectStub.h
# End Source File
# Begin Source File

SOURCE=.\src\ResourceAccessFactoryStub.cpp
# End Source File
# Begin Source File

SOURCE=.\src\ResourceAccessFactoryStub.h
# End Source File
# Begin Source File

SOURCE=.\src\RightsLibraryFactoryStub.cpp
# End Source File
# Begin Source File

SOURCE=.\src\RightsLibraryFactoryStub.h
# End Source File
# Begin Source File

SOURCE=.\src\RightsLibraryStub.cpp
# End Source File
# Begin Source File

SOURCE=.\src\RightsLibraryStub.h
# End Source File
# Begin Source File

SOURCE=.\src\ScadaProxyFactoryStub.cpp
# End Source File
# Begin Source File

SOURCE=.\src\ScadaProxyFactoryStub.h
# End Source File
# End Group
# End Target
# End Project
