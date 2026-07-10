# Microsoft Developer Studio Project File - Name="app.pa.PAAgent.PAAgent" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Application" 0x0101

CFG=app.pa.PAAgent.PAAgent - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "app.pa.PAAgent.PAAgent.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "app.pa.PAAgent.PAAgent.mak" CFG="app.pa.PAAgent.PAAgent - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "app.pa.PAAgent.PAAgent - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "app.pa.PAAgent.PAAgent - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "app.pa.PAAgent.PAAgent - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "../../../../../build/win32_nr"
# PROP Intermediate_Dir "../../../../../build/win32_nr/transactive/app/pa/PAAgent"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /YX /FD /c
# ADD CPP /nologo /MD /W3 /GR /GX /O2 /I "../../.." /I "../../../../../sdk/include/transactive" /I "../../../../../build/win32_n/transactive" /I "../../../../../sdk/include/transactive/core/exceptions/IDL/src" /I "../../../../../sdk/include/transactive/core/message/IDL/src" /I "../../../../../sdk/include/transactive/core/alarm/IDL/src" /I "../../../../../sdk/include/transactive/core/process_management/IDL/src" /I "../../../../../sdk/include/transactive/core/data_access_interface/tis_agent_4669/IDL/src" /I "../../../../../sdk/include/transactive/bus/security/duty_agent/IDL/src" /I "../../../../../sdk/include/transactive/bus/security/authentication_agent/IDL/src" /I "../../../../../sdk/include/transactive/bus/scada/common_library/IDL/src" /I "../../../../../sdk/include/transactive/bus/trains/TrainAgentCorba/idl/src" /I "../../../../../sdk/include/transactive/bus/radio/radioagentcorbadef_4669/IDL/src" /I "../../../../../sdk/include/cots/boost/boost_1_31_0" /I "../../../../../sdk/omniORB/omniORB_4.0.5/include" /I "../../../../../sdk/include/cots/ACE/5_3/ACE_wrappers" /D "NDEBUG" /D "__WIN32__" /D "__x86__" /D "_NT_" /D __OSVERSION__=4 /D "WIN32" /D "_CONSOLE" /D "_MBCS" /D _WIN32_WINNT=0x0500 /FR /FD /Zm300 /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0xc09 /d "NDEBUG"
# ADD RSC /l 0xc09 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /machine:I386
# ADD LINK32 ace.lib TA_Base_Bus.lib TA_IRS_Core.lib TA_IRS_Bus.lib ws2_32.lib winmm.lib omniORB405_rt.lib omniDynamic405_rt.lib omnithread30_rt.lib COS405_rt.lib TA_Base_Core.lib oci.lib Rpcrt4.lib stracelib.lib /nologo /subsystem:console /pdb:none /machine:I386 /out:"../../../../../bin/win32_nr/PAAgent.exe" /libpath:"../../../../../sdk/win32_nr" /libpath:"../../../../../build/win32_nr" /libpath:"..\..\..\..\cots\boost\boost_1_31_0\libs\Winnt40" /libpath:"../../../../../sdk/lib" /libpath:"../../../../../sdk/omniORB/omniORB_4.0.5/lib/x86_win32" /libpath:"../../../../../sdk/include/cots/WinStackTrace\build\Release"
# SUBTRACT LINK32 /debug

!ELSEIF  "$(CFG)" == "app.pa.PAAgent.PAAgent - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "../../../../../build/win32_nd"
# PROP Intermediate_Dir "../../../../../build/win32_nd/transactive/app/pa/PAAgent"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /GZ /c
# ADD CPP /nologo /MDd /W3 /Gm /GR /GX /Zi /Od /I "..\..\..\\" /I "..\..\..\..\..\build\win32_n\transactive" /I "..\..\..\..\..\sdk\include\transactive\core\data_access_interface\tis_agent_4669\IDL\src" /I "..\..\..\..\..\sdk\include\transactive" /I "..\..\..\..\..\sdk\include\transactive\core\process_management\IDL\src" /I "..\..\..\..\..\sdk\include\transactive\core\message\IDL\src" /I "..\..\..\..\..\sdk\include\transactive\core\exceptions\IDL\src" /I "..\..\..\..\..\sdk\include\transactive\bus\scada/common_library/IDL/src" /I "..\..\..\..\..\sdk\omniORB\omniORB_4.0.5\include" /I "..\..\..\..\..\sdk\include\cots\ACE\5_3\ACE_wrappers" /I "../../../../../sdk/include/transactive/bus/trains/TrainAgentCorbaDef/IDL/src" /I "../../../../../sdk/include/transactive/core/alarm/IDL/src" /I "../../../../../sdk/include/cots/boost/boost_1_31_0" /I "../../.." /I "../../../../../sdk/include/transactive" /I "../../../../../build/win32_n/transactive" /I "../../../../../sdk/include/transactive/core/exceptions/IDL/src" /I "../../../../../sdk/include/transactive/core/message/IDL/src" /I "../../../../../sdk/include/transactive/core/process_management/IDL/src" /I "../../../../../sdk/include/transactive/core/data_access_interface/tis_agent_4669/IDL/src" /I "../../../../../sdk/include/transactive/bus/security/duty_agent/IDL/src" /I "../../../../../sdk/include/transactive/bus/security/authentication_agent/IDL/src" /I "../../../../../sdk/include/transactive/bus/scada/common_library/IDL/src" /I "../../../../../sdk/include/transactive/bus/trains/TrainAgentCorba/idl/src" /I "../../../../../sdk/include/transactive/bus/radio/radioagentcorbadef_4669/IDL/src" /I "../../../../../sdk/omniORB/omniORB_4.0.5/include" /I "../../../../../sdk/include/cots/ACE/5_3/ACE_wrappers" /D "_DEBUG" /D "__WIN32__" /D "__x86__" /D "_NT_" /D __OSVERSION__=4 /D "WIN32" /D "_CONSOLE" /D "_MBCS" /FR /FD /Zm300 /I /Z".\src" /GZ /c
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386 /pdbtype:sept
# ADD LINK32 omniORB405_rtd.lib omniDynamic405_rtd.lib COS405_rtd.lib omnithread30_rtd.lib aced.lib TA_Base_Core_d.lib TA_Base_Bus_d.lib TA_IRS_Core_d.lib TA_IRS_Bus_d.lib oci.lib ws2_32.lib Rpcrt4.lib winmm.lib stracelib.lib /nologo /subsystem:console /profile /map /debug /debugtype:both /machine:I386 /nodefaultlib:"msvcrt.lib" /nodefaultlib:"msvcprt.lib" /out:"../../../../../bin/win32_nd/PAAgent.exe" /libpath:"../../../../../sdk/win32_nd" /libpath:"../../../../../build/win32_nd" /libpath:"../../../../../sdk/omniORB/omniORB_4.0.5/lib/x86_win32" /libpath:"../../../../../sdk/lib" /libpath:"../../../../../sdk/include/cots/WinStackTrace\build\Debug"

!ENDIF 

# Begin Target

# Name "app.pa.PAAgent.PAAgent - Win32 Release"
# Name "app.pa.PAAgent.PAAgent - Win32 Debug"
# Begin Group "Master PA Agent"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\src\MasterPAAgent.cpp
# End Source File
# Begin Source File

SOURCE=.\src\MasterPAAgent.h
# End Source File
# End Group
# Begin Group "Station PA Agent"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\src\StationPAAgent.cpp
# End Source File
# Begin Source File

SOURCE=.\src\StationPAAgent.h
# End Source File
# End Group
# Begin Group "Common"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\src\AbstractPAAgent.cpp
# End Source File
# Begin Source File

SOURCE=.\src\AbstractPAAgent.h
# End Source File
# Begin Source File

SOURCE=.\src\AtsAgentSubscriber.cpp
# End Source File
# Begin Source File

SOURCE=.\src\AtsAgentSubscriber.h
# End Source File
# Begin Source File

SOURCE=.\src\CachedConfig.cpp
# End Source File
# Begin Source File

SOURCE=.\src\CachedConfig.h
# End Source File
# Begin Source File

SOURCE=.\src\CircularList.h
# End Source File
# Begin Source File

SOURCE=.\src\DataPointWriteQueueProcessor.cpp
# End Source File
# Begin Source File

SOURCE=.\src\DataPointWriteQueueProcessor.h
# End Source File
# Begin Source File

SOURCE=.\src\FireCountdownAckMonitor.cpp
# End Source File
# Begin Source File

SOURCE=.\src\FireCountdownAckMonitor.h
# End Source File
# Begin Source File

SOURCE=.\src\main.cpp
# End Source File
# Begin Source File

SOURCE=.\src\PASConnection.cpp
# End Source File
# Begin Source File

SOURCE=.\src\PASConnection.h
# End Source File
# Begin Source File

SOURCE=.\src\PasHelpers.cpp
# End Source File
# Begin Source File

SOURCE=.\src\PasHelpers.h
# End Source File
# Begin Source File

SOURCE=.\src\VersionGen.cpp
# End Source File
# End Group
# Begin Group "Table Classes"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\src\AbstractPASTable.cpp
# End Source File
# Begin Source File

SOURCE=.\src\AbstractPASTable.h
# End Source File
# Begin Source File

SOURCE=.\src\AbstractWriteablePASTable.cpp
# End Source File
# Begin Source File

SOURCE=.\src\AbstractWriteablePASTable.h
# End Source File
# Begin Source File

SOURCE=.\src\ConnectToPAFEPC.cpp
# End Source File
# Begin Source File

SOURCE=.\src\ConnectToPAFEPC.h
# End Source File
# Begin Source File

SOURCE=.\src\DisconnectFromPAFEPC.cpp
# End Source File
# Begin Source File

SOURCE=.\src\DisconnectFromPAFEPC.h
# End Source File
# Begin Source File

SOURCE=.\src\InvalidatePASDataPoints.cpp
# End Source File
# Begin Source File

SOURCE=.\src\InvalidatePASDataPoints.h
# End Source File
# Begin Source File

SOURCE=.\src\ITableData.h
# End Source File
# Begin Source File

SOURCE=.\src\MasterPasTableManager.cpp
# End Source File
# Begin Source File

SOURCE=.\src\MasterPasTableManager.h
# End Source File
# Begin Source File

SOURCE=.\src\PASEvent.cpp
# End Source File
# Begin Source File

SOURCE=.\src\PASEvent.h
# End Source File
# Begin Source File

SOURCE=.\src\PASReadEvent.cpp
# End Source File
# Begin Source File

SOURCE=.\src\PASReadEvent.h
# End Source File
# Begin Source File

SOURCE=.\src\PasTableManager.cpp
# End Source File
# Begin Source File

SOURCE=.\src\PasTableManager.h
# End Source File
# Begin Source File

SOURCE=.\src\PASWriteEvent.cpp
# End Source File
# Begin Source File

SOURCE=.\src\PASWriteEvent.h
# End Source File
# Begin Source File

SOURCE=.\src\StationPasTableManager.cpp
# End Source File
# Begin Source File

SOURCE=.\src\StationPasTableManager.h
# End Source File
# Begin Source File

SOURCE=.\src\Table100.cpp
# End Source File
# Begin Source File

SOURCE=.\src\Table100.h
# End Source File
# Begin Source File

SOURCE=.\src\Table1200.cpp
# End Source File
# Begin Source File

SOURCE=.\src\Table1200.h
# End Source File
# Begin Source File

SOURCE=.\src\Table1201.cpp
# End Source File
# Begin Source File

SOURCE=.\src\Table1201.h
# End Source File
# Begin Source File

SOURCE=.\src\Table1204.cpp
# End Source File
# Begin Source File

SOURCE=.\src\Table1204.h
# End Source File
# Begin Source File

SOURCE=.\src\Table1301.cpp
# End Source File
# Begin Source File

SOURCE=.\src\Table1301.h
# End Source File
# Begin Source File

SOURCE=.\src\Table1303.cpp
# End Source File
# Begin Source File

SOURCE=.\src\Table1303.h
# End Source File
# Begin Source File

SOURCE=.\src\Table1610.cpp
# End Source File
# Begin Source File

SOURCE=.\src\Table1610.h
# End Source File
# Begin Source File

SOURCE=.\src\Table200.cpp
# End Source File
# Begin Source File

SOURCE=.\src\Table200.h
# End Source File
# Begin Source File

SOURCE=.\src\Table201.cpp
# End Source File
# Begin Source File

SOURCE=.\src\Table201.h
# End Source File
# Begin Source File

SOURCE=.\src\Table202.cpp
# End Source File
# Begin Source File

SOURCE=.\src\Table202.h
# End Source File
# Begin Source File

SOURCE=.\src\Table204.cpp
# End Source File
# Begin Source File

SOURCE=.\src\Table204.h
# End Source File
# Begin Source File

SOURCE=.\src\Table301.cpp
# End Source File
# Begin Source File

SOURCE=.\src\Table301.h
# End Source File
# Begin Source File

SOURCE=.\src\Table302.cpp
# End Source File
# Begin Source File

SOURCE=.\src\Table302.h
# End Source File
# Begin Source File

SOURCE=.\src\Table303.cpp
# End Source File
# Begin Source File

SOURCE=.\src\Table303.h
# End Source File
# Begin Source File

SOURCE=.\src\Table350.cpp
# End Source File
# Begin Source File

SOURCE=.\src\Table350.h
# End Source File
# Begin Source File

SOURCE=.\src\Table550.cpp
# End Source File
# Begin Source File

SOURCE=.\src\Table550.h
# End Source File
# Begin Source File

SOURCE=.\src\Table560.cpp
# End Source File
# Begin Source File

SOURCE=.\src\Table560.h
# End Source File
# Begin Source File

SOURCE=.\src\Table570.cpp
# End Source File
# Begin Source File

SOURCE=.\src\Table570.h
# End Source File
# Begin Source File

SOURCE=.\src\Table590.cpp
# End Source File
# Begin Source File

SOURCE=.\src\Table590.h
# End Source File
# Begin Source File

SOURCE=.\src\Table603.cpp
# End Source File
# Begin Source File

SOURCE=.\src\Table603.h
# End Source File
# Begin Source File

SOURCE=.\src\Table604.cpp
# End Source File
# Begin Source File

SOURCE=.\src\Table604.h
# End Source File
# Begin Source File

SOURCE=.\src\Table610.cpp
# End Source File
# Begin Source File

SOURCE=.\src\Table610.h
# End Source File
# Begin Source File

SOURCE=.\src\Table630.cpp
# End Source File
# Begin Source File

SOURCE=.\src\Table630.h
# End Source File
# End Group
# Begin Group "Broadcast Classes"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\src\AbstractBroadcast.cpp
# End Source File
# Begin Source File

SOURCE=.\src\AbstractBroadcast.h
# End Source File
# Begin Source File

SOURCE=.\src\AbstractBroadcastCircularList.cpp
# End Source File
# Begin Source File

SOURCE=.\src\AbstractBroadcastCircularList.h
# End Source File
# Begin Source File

SOURCE=.\src\AbstractNonTrainDVABroadcast.cpp
# End Source File
# Begin Source File

SOURCE=.\src\AbstractNonTrainDVABroadcast.h
# End Source File
# Begin Source File

SOURCE=.\src\AbstractStationBroadcast.cpp
# End Source File
# Begin Source File

SOURCE=.\src\AbstractStationBroadcast.h
# End Source File
# Begin Source File

SOURCE=.\src\AbstractTrainBroadcast.cpp
# End Source File
# Begin Source File

SOURCE=.\src\AbstractTrainBroadcast.h
# End Source File
# Begin Source File

SOURCE=.\src\BroadcastManager.cpp
# End Source File
# Begin Source File

SOURCE=.\src\BroadcastManager.h
# End Source File
# Begin Source File

SOURCE=.\src\BroadcastWorker.cpp
# End Source File
# Begin Source File

SOURCE=.\src\BroadcastWorker.h
# End Source File
# Begin Source File

SOURCE=.\src\DVARecordingBroadcast.cpp
# End Source File
# Begin Source File

SOURCE=.\src\DVARecordingBroadcast.h
# End Source File
# Begin Source File

SOURCE=.\src\MasterDVABroadcast.cpp
# End Source File
# Begin Source File

SOURCE=.\src\MasterDVABroadcast.h
# End Source File
# Begin Source File

SOURCE=.\src\MasterLiveBroadcast.cpp
# End Source File
# Begin Source File

SOURCE=.\src\MasterLiveBroadcast.h
# End Source File
# Begin Source File

SOURCE=.\src\ProgressReportSentry.cpp
# End Source File
# Begin Source File

SOURCE=.\src\ProgressReportSentry.h
# End Source File
# Begin Source File

SOURCE=.\src\StationDVABroadcast.cpp
# End Source File
# Begin Source File

SOURCE=.\src\StationDVABroadcast.h
# End Source File
# Begin Source File

SOURCE=.\src\StationLiveBroadcast.cpp
# End Source File
# Begin Source File

SOURCE=.\src\StationLiveBroadcast.h
# End Source File
# Begin Source File

SOURCE=.\src\StationMusicBroadcast.cpp
# End Source File
# Begin Source File

SOURCE=.\src\StationMusicBroadcast.h
# End Source File
# Begin Source File

SOURCE=.\src\TrainAgentSubscriber.cpp
# End Source File
# Begin Source File

SOURCE=.\src\TrainAgentSubscriber.h
# End Source File
# Begin Source File

SOURCE=.\src\TrainDVABroadcast.cpp
# End Source File
# Begin Source File

SOURCE=.\src\TrainDVABroadcast.h
# End Source File
# Begin Source File

SOURCE=.\src\TrainLiveBroadcast.cpp
# End Source File
# Begin Source File

SOURCE=.\src\TrainLiveBroadcast.h
# End Source File
# End Group
# Begin Group "misc"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\..\bus\security\access_control\actions\src\AccessControlledActions.h
# End Source File
# Begin Source File

SOURCE=.\notes.txt

!IF  "$(CFG)" == "app.pa.PAAgent.PAAgent - Win32 Release"

!ELSEIF  "$(CFG)" == "app.pa.PAAgent.PAAgent - Win32 Debug"

# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\core\message\types\PAAgentAlarms_MessageTypes.h
# End Source File
# Begin Source File

SOURCE=..\..\..\core\message\types\PAAgentAudit_MessageTypes.h
# End Source File
# Begin Source File

SOURCE=..\..\..\core\message\types\PAAgentComms_MessageTypes.h
# End Source File
# Begin Source File

SOURCE=.\PAAgentTest.gdf

!IF  "$(CFG)" == "app.pa.PAAgent.PAAgent - Win32 Release"

!ELSEIF  "$(CFG)" == "app.pa.PAAgent.PAAgent - Win32 Debug"

# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\..\..\core\exceptions\src\PasExceptions.h
# End Source File
# End Group
# End Target
# End Project
