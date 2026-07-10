# Microsoft Developer Studio Project File - Name="app.maintenance_management.mms_agent.MmsAgent" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Console Application" 0x0103

CFG=app.maintenance_management.mms_agent.MmsAgent - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "app.maintenance_management.mms_agent.MmsAgent.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "app.maintenance_management.mms_agent.MmsAgent.mak" CFG="app.maintenance_management.mms_agent.MmsAgent - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "app.maintenance_management.mms_agent.MmsAgent - Win32 Release" (based on "Win32 (x86) Console Application")
!MESSAGE "app.maintenance_management.mms_agent.MmsAgent - Win32 Debug" (based on "Win32 (x86) Console Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName "app.maintenance_management.mms_agent.MmsAgent"
# PROP Scc_LocalPath "."
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "app.maintenance_management.mms_agent.MmsAgent - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "../../../../../bin/win32_nr"
# PROP Intermediate_Dir "../../../../../build/win32_nr/transactive/app/maintenance_management/mms_agent"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
MTL=midl.exe
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /c
# ADD CPP /nologo /MD /W3 /GR /GX /O2 /I "./src/" /I "../../../" /I "../../../../../sdk/include/transactive" /I "../../../../../sdk/omniORB/omniORB_4.0.5/include" /I "../../../../../sdk/include/transactive/core/exceptions/src" /I "../../../../../sdk/include/transactive/bus/scada/common_library/IDL/src" /I "../../../../../sdk/include/transactive/core/message/IDL/src" /I "../../../../../sdk/include/transactive/core/process_management/idl/src" /I "../../../../../sdk/include/transactive/core/alarm/IDL/src" /I "../../../../../sdk/include/transactive/core/exceptions/IDL/src" /I "../../../../../sdk/include/transactive/bus/maintenance_management/mmscorbadef/IDL/src" /I "../../../../" /I "../../../../../sdk/include/cots/boost/boost_1_31_0" /I "../../../../cots/BEAMessageQ/include" /I "..\..\..\..\..\sdk\include\cots\ACE\5_3\ACE_wrappers" /D "__WIN32__" /D "__x86__" /D "_NT_" /D __OSVERSION__=4 /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /D _WIN32_WINNT=0x0400 /FD /c
# SUBTRACT CPP /X /YX
# ADD BASE RSC /l 0xc09 /d "NDEBUG"
# ADD RSC /l 0xc09 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386
# ADD LINK32 WS2_32.lib ace.lib TA_Base_bus.lib TA_IRS_Core.lib TA_IRS_bus.lib dmqcl32.lib winmm.lib omniORB405_rt.lib omniDynamic405_rt.lib omnithread30_rt.lib COS405_rt.lib TA_Base_Core.lib oci.lib Rpcrt4.lib stracelib.lib /nologo /subsystem:console /pdb:none /machine:I386 /out:"../../../../../bin/win32_nr/MmsAgent.exe" /libpath:"../../../../../sdk/win32_nr" /libpath:"../../../../../build/win32_nr" /libpath:"../../../../../sdk/include/cots/ACE/5_3/ACE_wrappers/ace" /libpath:"../../../../cots/MmsApi/Winnt/ver_1_0/stub/lib" /libpath:"../../../../../sdk/lib" /libpath:"../../../../cots/BEAMessageQ/lib" /libpath:"../../../../../sdk/omniORB/omniORB_4.0.5/lib/x86_win32" /libpath:"../../../../../sdk/include/cots/WinStackTrace\build\Release"
# SUBTRACT LINK32 /map /debug

!ELSEIF  "$(CFG)" == "app.maintenance_management.mms_agent.MmsAgent - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "../../../../../bin/win32_nd"
# PROP Intermediate_Dir "../../../../../build/win32_nd/transactive/app/maintenance_management/mms_agent"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
MTL=midl.exe
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /GZ /c
# ADD CPP /nologo /MDd /W3 /Gm /GR /GX /Zi /Od /I "./src/" /I "../../../" /I "../../../../../sdk/include/transactive" /I "../../../../../sdk/omniORB/omniORB_4.0.5/include" /I "../../../../../sdk/include/transactive/core/exceptions/src" /I "../../../../../sdk/include/transactive/bus/scada/common_library/IDL/src" /I "../../../../../sdk/include/transactive/core/message/IDL/src" /I "../../../../../sdk/include/transactive/core/process_management/idl/src" /I "../../../../../sdk/include/transactive/core/alarm/IDL/src" /I "../../../../../sdk/include/transactive/core/exceptions/IDL/src" /I "../../../../../sdk/include/transactive/bus/maintenance_management/mmscorbadef/IDL/src" /I "../../../../" /I "../../../../../sdk/include/cots/boost/boost_1_31_0" /I "../../../../cots/BEAMessageQ/include" /I "..\..\..\..\..\sdk\include\cots\ACE\5_3\ACE_wrappers" /D "_DEBUG" /D "__WIN32__" /D "__x86__" /D "_NT_" /D __OSVERSION__=4 /D "WIN32" /D "_CONSOLE" /D "_MBCS" /D _WIN32_WINNT=0x0400 /D WINVER=0x0400 /FR /FD /GZ /c
# SUBTRACT CPP /X /YX
# ADD BASE RSC /l 0xc09 /d "_DEBUG"
# ADD RSC /l 0xc09 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386 /pdbtype:sept
# ADD LINK32 omniORB405_rtd.lib omniDynamic405_rtd.lib COS405_rtd.lib omnithread30_rtd.lib oci.lib Rpcrt4.lib WS2_32.lib aced.lib TA_Base_Core_d.lib TA_Base_bus_d.lib TA_IRS_Core_d.lib TA_IRS_bus_d.lib dmqcl32.lib winmm.lib stracelib.lib /nologo /subsystem:console /incremental:no /debug /debugtype:both /machine:I386 /out:"../../../../../bin/win32_nd/MmsAgent.exe" /pdbtype:sept /libpath:"../../../../../sdk/win32_nd" /libpath:"../../../../../build/win32_nd" /libpath:"../../../../../sdk/omniORB/omniORB_4.0.5/lib/x86_win32" /libpath:"../../../../../sdk/include/cots/ACE/5_3/ACE_wrappers/ace" /libpath:"../../../../cots/MmsApi/Winnt/ver_1_0/stub/lib" /libpath:"../../../../../sdk/lib" /libpath:"../../../../cots/BEAMessageQ/lib" /libpath:"../../../../../sdk/include/cots/WinStackTrace\build\Debug"
# SUBTRACT LINK32 /map

!ENDIF 

# Begin Target

# Name "app.maintenance_management.mms_agent.MmsAgent - Win32 Release"
# Name "app.maintenance_management.mms_agent.MmsAgent - Win32 Debug"
# Begin Group "src"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat;h"
# Begin Source File

SOURCE=.\src\DatabaseCache.cpp
# End Source File
# Begin Source File

SOURCE=.\src\DatabaseCache.h
# End Source File
# Begin Source File

SOURCE=.\src\InvalidArgException.h
# End Source File
# Begin Source File

SOURCE=.\src\main.cpp
# End Source File
# Begin Source File

SOURCE=.\src\MmsAgent.cpp
# End Source File
# Begin Source File

SOURCE=.\src\MmsAgent.h
# End Source File
# Begin Source File

SOURCE=.\src\MmsAlarmSubmitter.cpp
# End Source File
# Begin Source File

SOURCE=.\src\MmsAlarmSubmitter.h
# End Source File
# Begin Source File

SOURCE=.\src\MmsApiWrapper.cpp
# End Source File
# Begin Source File

SOURCE=.\src\MmsApiWrapper.h
# End Source File
# Begin Source File

SOURCE=.\src\MmsConnection.cpp
# End Source File
# Begin Source File

SOURCE=.\src\MmsConnection.h
# End Source File
# Begin Source File

SOURCE=.\src\MmsConnectionException.h
# End Source File
# Begin Source File

SOURCE=.\src\MmsConnectionTester.cpp
# End Source File
# Begin Source File

SOURCE=.\src\MmsConnectionTester.h
# End Source File
# Begin Source File

SOURCE=.\src\MmsJobRequestSubmitter.cpp
# End Source File
# Begin Source File

SOURCE=.\src\MmsJobRequestSubmitter.h
# End Source File
# Begin Source File

SOURCE=.\src\MmsMessageUtil.cpp
# End Source File
# Begin Source File

SOURCE=.\src\MmsMessageUtil.h
# End Source File
# Begin Source File

SOURCE=.\src\MmsPeriodicSubmitter.cpp
# End Source File
# Begin Source File

SOURCE=.\src\MmsPeriodicSubmitter.h
# End Source File
# Begin Source File

SOURCE=.\src\MmsServerState.cpp
# End Source File
# Begin Source File

SOURCE=.\src\MmsServerState.h
# End Source File
# Begin Source File

SOURCE=.\src\PeriodicThread.cpp
# End Source File
# Begin Source File

SOURCE=.\src\PeriodicThread.h
# End Source File
# Begin Source File

SOURCE=.\src\VersionGen.cpp
# End Source File
# End Group
# End Target
# End Project
