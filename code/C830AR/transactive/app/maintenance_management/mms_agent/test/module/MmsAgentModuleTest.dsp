# Microsoft Developer Studio Project File - Name="MmsAgentModuleTest" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Console Application" 0x0103

CFG=MmsAgentModuleTest - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "MmsAgentModuleTest.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "MmsAgentModuleTest.mak" CFG="MmsAgentModuleTest - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "MmsAgentModuleTest - Win32 Release" (based on "Win32 (x86) Console Application")
!MESSAGE "MmsAgentModuleTest - Win32 Debug" (based on "Win32 (x86) Console Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "MmsAgentModuleTest - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /c
# ADD CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /c
# ADD BASE RSC /l 0xc09 /d "NDEBUG"
# ADD RSC /l 0xc09 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386

!ELSEIF  "$(CFG)" == "MmsAgentModuleTest - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /GZ /c
# ADD CPP /nologo /MDd /W3 /Gm /GR /GX /Zi /Od /I "." /I "./src" /I "../../../../../../" /I "..\..\..\..\..\\" /I "../" /I "..\..\..\..\..\..\cots\omniORB\omniORB_4.0.5\include" /I "..\..\..\..\..\core/exceptions/src" /I "../../../../../../../sdk/include/transactive" /I "../../../../../../../sdk/include/transactive/core/exceptions/IDL/src" /I "../../../../../../../sdk/include/transactive/core/message/IDL/src" /I "../../../../../../../sdk/include/transactive/core/alarm/IDL/src" /I "../../../../../../../sdk/include/transactive/bus/maintenance_management/mmscorbadef/IDL/src" /D "_DEBUG" /D "__WIN32__" /D "__x86__" /D "_NT_" /D __OSVERSION__=4 /D "WIN32" /D "_CONSOLE" /D "_MBCS" /D _WIN32_WINNT=0x0400 /D WINVER=0x0400 /FR /FD /GZ /c
# SUBTRACT CPP /YX
# ADD BASE RSC /l 0xc09 /d "_DEBUG"
# ADD RSC /l 0xc09 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386 /pdbtype:sept
# ADD LINK32 omniORB405_rtd.lib omniDynamic405_rtd.lib COS405_rtd.lib omnithread30_rtd.lib oci.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib Rpcrt4.lib WS2_32.lib aced.lib TA_Base_Core_d.lib TA_Base_Bus_d.lib MMSAPI.lib /nologo /subsystem:console /incremental:no /debug /machine:I386 /pdbtype:sept /libpath:"..\..\..\..\..\..\cots\CppUnit\lib\nt_x86" /libpath:"..\..\..\..\..\tools\build\core_library\Debug" /libpath:"..\..\..\..\..\tools\build\bus_library\Debug" /libpath:"..\..\..\..\..\..\cots/omniORB/omniORB_4.0.5/lib/x86_win32" /libpath:"..\..\..\..\..\..\cots/oracle/920/winnt/lib" /libpath:"..\..\..\..\..\..\cots\MmsApi\Winnt\ver_1_0\stub\lib" /libpath:"..\..\..\..\..\..\cots/ACE/5_3/ACE_wrappers/ace" /libpath:"../../../../../../../sdk/win32_nd"

!ENDIF 

# Begin Target

# Name "MmsAgentModuleTest - Win32 Release"
# Name "MmsAgentModuleTest - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\src\main.cpp
# End Source File
# Begin Source File

SOURCE=.\src\MmsAlarmSubmitterTest.cpp
# End Source File
# Begin Source File

SOURCE=.\src\MmsConnectionModuleTest.cpp
# End Source File
# Begin Source File

SOURCE=.\src\MmsJobRequestSubmitterTest.cpp
# End Source File
# Begin Source File

SOURCE=.\src\Util.cpp
# End Source File
# Begin Source File

SOURCE=.\src\VersionGen.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\src\MmsAlarmSubmitterTest.h
# End Source File
# Begin Source File

SOURCE=.\src\MmsConnectionModuleTest.h
# End Source File
# Begin Source File

SOURCE=.\src\MmsJobRequestSubmitterTest.h
# End Source File
# Begin Source File

SOURCE=.\src\Util.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# End Group
# Begin Group "cfg"

# PROP Default_Filter "ini"
# Begin Source File

SOURCE=.\cfg\config.ini

!IF  "$(CFG)" == "MmsAgentModuleTest - Win32 Release"

!ELSEIF  "$(CFG)" == "MmsAgentModuleTest - Win32 Debug"

# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# End Group
# End Target
# End Project
