# Microsoft Developer Studio Project File - Name="PAAgentTest" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Console Application" 0x0103

CFG=PAAgentTest - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "PAAgentTest.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "PAAgentTest.mak" CFG="PAAgentTest - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "PAAgentTest - Win32 Release" (based on "Win32 (x86) Console Application")
!MESSAGE "PAAgentTest - Win32 Debug" (based on "Win32 (x86) Console Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "PAAgentTest - Win32 Release"

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
# ADD CPP /nologo /MD /W3 /GR /GX /O2 /I "..\..\..\..\.." /I "..\..\..\..\..\cots\omniORB\omniORB_4.0.3\Winnt40\omni\include" /I "..\..\..\..\..\core\message\IDL\src" /I "..\..\..\..\..\cots\CppUnit\src" /I ".\src" /I "..\..\..\..\app\signs\tis_agent\IDL\src" /I "..\..\..\..\core\exceptions\src" /I "..\..\..\.." /I "..\..\..\..\cots\omniORB\omniORB_4.0.3\Winnt40\omni\include" /I "src" /I "..\..\..\..\\" /I "..\..\..\..\cots\CppUnit\src" /D "NDEBUG" /D "_DEBUG" /D "__WIN32__" /D "__x86__" /D "_NT_" /D __OSVERSION__=4 /D "WIN32" /D "_CONSOLE" /D "_MBCS" /FD /c
# SUBTRACT CPP /YX
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386 /libpath:"../../../../cots/omniORB/omniORB_4.0.3/Winnt40/omni/lib/x86_win32" /libpath:"../../../../cots/oracle/920/winnt/lib"

!ELSEIF  "$(CFG)" == "PAAgentTest - Win32 Debug"

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
# ADD CPP /nologo /MDd /W3 /Gm /GR /GX /ZI /Od /I "../../../../../" /I "..\..\..\..\\" /I "..\..\..\..\..\cots\omniORB\omniORB_4.0.5\include" /I "..\..\..\..\..\cots\ACE\5_3\ACE_wrappers\\" /I "..\..\..\..\core\message\IDL\src" /I "..\..\..\..\..\cots\CppUnit\src" /I "..\..\..\..\core\message\src" /I "..\..\..\..\core\message\types" /I ".\src" /I "..\..\..\..\app\signs\tis_agent\IDL\src" /I "..\..\..\..\core\exceptions\src" /I "src" /I "..\..\..\..\..\..\..\base\code\transactive" /I "../../../../../../sdk/include/transactive" /I "../../../../../../sdk/include/transactive/core/message/IDL/src" /I "../../../../../../sdk/win32_n/transactive" /I "../../../../../../sdk/include/transactive/core/process_management/idl/src" /I "../../../../../../build/win32_n/transactive" /I "../../../../../../sdk/include/transactive/core/exceptions/IDL/src" /I "../../../../../../../sdk/include/transactive/" /D "PAS_DEMO_CODE" /D "_DEBUG" /D "__WIN32__" /D "__x86__" /D "_NT_" /D __OSVERSION__=4 /D "WIN32" /D "_CONSOLE" /D "_MBCS" /D TA_ASSERT_FAIL=EXCEPTION /FD /GZ /Zm200 /c
# SUBTRACT CPP /Fr
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386 /pdbtype:sept
# ADD LINK32 omniORB403_rtd.lib omniDynamic403_rtd.lib COS403_rtd.lib omnithread30_rtd.lib oci.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib Rpcrt4.lib ws2_32.lib aced.lib TA_Base_Core_d.lib TA_Base_Bus_d.lib /nologo /subsystem:console /incremental:no /debug /machine:I386 /pdbtype:sept /libpath:"../../../../cots/omniORB/omniORB_4.0.3/Winnt40/omni/lib/x86_win32" /libpath:"../../../../cots/oracle/920/winnt/lib" /libpath:"..\..\..\..\tools\build\core_library\Debug" /libpath:"..\..\..\..\tools\build\bus_library\Debug" /libpath:"..\..\..\..\cots/ACE/5_3/ACE_wrappers/ace"
# SUBTRACT LINK32 /pdb:none

!ENDIF 

# Begin Target

# Name "PAAgentTest - Win32 Release"
# Name "PAAgentTest - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=..\..\..\..\bus\pa\CachedMaps\test\src\GenericCachedMapBaseTestCases.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\..\bus\pa\CachedMaps\test\src\GenericCachedMapTest.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\..\bus\pa\CachedMaps\test\src\GenericCachedMapTestCases.cpp
# End Source File
# Begin Source File

SOURCE=.\src\LockAcquiringThread.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\..\bus\pa\CachedMaps\test\src\LockedPacketTestCases.cpp
# End Source File
# Begin Source File

SOURCE=.\src\main.cpp
# End Source File
# Begin Source File

SOURCE=.\src\Table201TestCases.cpp
# End Source File
# Begin Source File

SOURCE=.\src\UnitTestAccessBridge.cpp
# End Source File
# Begin Source File

SOURCE=.\src\VersionGen.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=..\..\..\..\bus\pa\CachedMaps\test\src\GenericCachedMapBaseTestCases.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\bus\pa\CachedMaps\test\src\GenericCachedMapTest.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\bus\pa\CachedMaps\test\src\GenericCachedMapTestCases.h
# End Source File
# Begin Source File

SOURCE=.\src\LockAcquiringThread.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\bus\pa\CachedMaps\test\src\LockedPacketTestCases.h
# End Source File
# Begin Source File

SOURCE=.\src\Table201TestCases.h
# End Source File
# Begin Source File

SOURCE=.\src\UnitTestAccessBridge.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# End Group
# Begin Group "CachedMapClasses"

# PROP Default_Filter ""
# End Group
# Begin Group "PAAgentFiles"

# PROP Default_Filter ""
# Begin Group "Tested"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\src\Table201.cpp
# End Source File
# Begin Source File

SOURCE=..\src\Table201.h
# End Source File
# End Group
# Begin Group "UnTested"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\src\AbstractBroadcast.cpp
# End Source File
# Begin Source File

SOURCE=..\src\AbstractBroadcast.h
# End Source File
# Begin Source File

SOURCE=..\src\AbstractBroadcastCircularList.cpp
# End Source File
# Begin Source File

SOURCE=..\src\AbstractBroadcastCircularList.h
# End Source File
# Begin Source File

SOURCE=..\src\AbstractFlaggedPASTable.h
# End Source File
# Begin Source File

SOURCE=..\src\AbstractFlaggedTableCircularList.cpp
# End Source File
# Begin Source File

SOURCE=..\src\AbstractFlaggedTableCircularList.h
# End Source File
# Begin Source File

SOURCE=..\src\AbstractPAAgent.cpp
# End Source File
# Begin Source File

SOURCE=..\src\AbstractPAAgent.h
# End Source File
# Begin Source File

SOURCE=..\src\AbstractPASTable.cpp
# End Source File
# Begin Source File

SOURCE=..\src\AbstractPASTable.h
# End Source File
# Begin Source File

SOURCE=..\src\AbstractReadablePASTable.cpp
# End Source File
# Begin Source File

SOURCE=..\src\AbstractReadablePASTable.h
# End Source File
# Begin Source File

SOURCE=..\src\AbstractStationBroadcast.cpp
# End Source File
# Begin Source File

SOURCE=..\src\AbstractStationBroadcast.h
# End Source File
# Begin Source File

SOURCE=..\src\AbstractTrainBroadcast.cpp
# End Source File
# Begin Source File

SOURCE=..\src\AbstractTrainBroadcast.h
# End Source File
# Begin Source File

SOURCE=..\src\AbstractWritablePASTable.cpp
# End Source File
# Begin Source File

SOURCE=..\src\AbstractWritablePASTable.h
# End Source File
# Begin Source File

SOURCE=..\src\AtsAgentSubscriber.cpp
# End Source File
# Begin Source File

SOURCE=..\src\AtsAgentSubscriber.h
# End Source File
# Begin Source File

SOURCE=..\src\BroadcastManager.cpp
# End Source File
# Begin Source File

SOURCE=..\src\BroadcastManager.h
# End Source File
# Begin Source File

SOURCE=..\src\BroadcastWorker.cpp
# End Source File
# Begin Source File

SOURCE=..\src\BroadcastWorker.h
# End Source File
# Begin Source File

SOURCE=..\src\CachedConfig.cpp
# End Source File
# Begin Source File

SOURCE=..\src\CachedConfig.h
# End Source File
# Begin Source File

SOURCE=..\src\CircularList.h
# End Source File
# Begin Source File

SOURCE=..\src\DataPointWriteQueueProcessor.cpp
# End Source File
# Begin Source File

SOURCE=..\src\DataPointWriteQueueProcessor.h
# End Source File
# Begin Source File

SOURCE=..\src\DVARecordingBroadcast.cpp
# End Source File
# Begin Source File

SOURCE=..\src\DVARecordingBroadcast.h
# End Source File
# Begin Source File

SOURCE=..\src\FakeMessagesArray.h
# End Source File
# Begin Source File

SOURCE=..\src\FastPollingWorker.cpp
# End Source File
# Begin Source File

SOURCE=..\src\FastPollingWorker.h
# End Source File
# Begin Source File

SOURCE=..\src\FlaggedTableWorker.cpp
# End Source File
# Begin Source File

SOURCE=..\src\FlaggedTableWorker.h
# End Source File
# Begin Source File

SOURCE=..\src\ForcedDataPointSynchroniser.cpp
# End Source File
# Begin Source File

SOURCE=..\src\ForcedDataPointSynchroniser.h
# End Source File
# Begin Source File

SOURCE=..\src\ICachedConfigObserver.h
# End Source File
# Begin Source File

SOURCE=..\src\ICachedMapFilter.h
# End Source File
# Begin Source File

SOURCE=..\src\ICachedMapObserverRegistry.h
# End Source File
# Begin Source File

SOURCE=..\src\MasterDVABroadcast.cpp
# End Source File
# Begin Source File

SOURCE=..\src\MasterDVABroadcast.h
# End Source File
# Begin Source File

SOURCE=..\src\MasterLiveBroadcast.cpp
# End Source File
# Begin Source File

SOURCE=..\src\MasterLiveBroadcast.h
# End Source File
# Begin Source File

SOURCE=..\src\MasterPAAgent.cpp
# End Source File
# Begin Source File

SOURCE=..\src\MasterPAAgent.h
# End Source File
# Begin Source File

SOURCE=..\src\PAAgent.cpp
# End Source File
# Begin Source File

SOURCE=..\src\PAAgent.h
# End Source File
# Begin Source File

SOURCE=..\src\PASConnection.cpp
# End Source File
# Begin Source File

SOURCE=..\src\PASConnection.h
# End Source File
# Begin Source File

SOURCE=..\src\PasHelpers.cpp
# End Source File
# Begin Source File

SOURCE=..\src\PasHelpers.h
# End Source File
# Begin Source File

SOURCE=..\src\PASTableFactory.cpp
# End Source File
# Begin Source File

SOURCE=..\src\PASTableFactory.h
# End Source File
# Begin Source File

SOURCE=..\src\ProgressReportSentry.cpp
# End Source File
# Begin Source File

SOURCE=..\src\ProgressReportSentry.h
# End Source File
# Begin Source File

SOURCE=..\src\StationDVABroadcast.cpp
# End Source File
# Begin Source File

SOURCE=..\src\StationDVABroadcast.h
# End Source File
# Begin Source File

SOURCE=..\src\StationLiveBroadcast.cpp
# End Source File
# Begin Source File

SOURCE=..\src\StationLiveBroadcast.h
# End Source File
# Begin Source File

SOURCE=..\src\StationMusicBroadcast.cpp
# End Source File
# Begin Source File

SOURCE=..\src\StationMusicBroadcast.h
# End Source File
# Begin Source File

SOURCE=..\src\StationPAAgent.cpp
# End Source File
# Begin Source File

SOURCE=..\src\StationPAAgent.h
# End Source File
# Begin Source File

SOURCE=..\src\Table100.cpp
# End Source File
# Begin Source File

SOURCE=..\src\Table100.h
# End Source File
# Begin Source File

SOURCE=..\src\Table1200.cpp
# End Source File
# Begin Source File

SOURCE=..\src\Table1200.h
# End Source File
# Begin Source File

SOURCE=..\src\Table1201.cpp
# End Source File
# Begin Source File

SOURCE=..\src\Table1201.h
# End Source File
# Begin Source File

SOURCE=..\src\Table1204.cpp
# End Source File
# Begin Source File

SOURCE=..\src\Table1204.h
# End Source File
# Begin Source File

SOURCE=..\src\Table1301.cpp
# End Source File
# Begin Source File

SOURCE=..\src\Table1301.h
# End Source File
# Begin Source File

SOURCE=..\src\Table1303.cpp
# End Source File
# Begin Source File

SOURCE=..\src\Table1303.h
# End Source File
# Begin Source File

SOURCE=..\src\Table1610.cpp
# End Source File
# Begin Source File

SOURCE=..\src\Table1610.h
# End Source File
# Begin Source File

SOURCE=..\src\Table200.cpp
# End Source File
# Begin Source File

SOURCE=..\src\Table200.h
# End Source File
# Begin Source File

SOURCE=..\src\Table202.cpp
# End Source File
# Begin Source File

SOURCE=..\src\Table202.h
# End Source File
# Begin Source File

SOURCE=..\src\Table204.cpp
# End Source File
# Begin Source File

SOURCE=..\src\Table204.h
# End Source File
# Begin Source File

SOURCE=..\src\Table301.cpp
# End Source File
# Begin Source File

SOURCE=..\src\Table301.h
# End Source File
# Begin Source File

SOURCE=..\src\Table302.cpp
# End Source File
# Begin Source File

SOURCE=..\src\Table302.h
# End Source File
# Begin Source File

SOURCE=..\src\Table303.cpp
# End Source File
# Begin Source File

SOURCE=..\src\Table303.h
# End Source File
# Begin Source File

SOURCE=..\src\Table350.cpp
# End Source File
# Begin Source File

SOURCE=..\src\Table350.h
# End Source File
# Begin Source File

SOURCE=..\src\Table550.cpp
# End Source File
# Begin Source File

SOURCE=..\src\Table550.h
# End Source File
# Begin Source File

SOURCE=..\src\Table560.cpp
# End Source File
# Begin Source File

SOURCE=..\src\Table560.h
# End Source File
# Begin Source File

SOURCE=..\src\Table570.cpp
# End Source File
# Begin Source File

SOURCE=..\src\Table570.h
# End Source File
# Begin Source File

SOURCE=..\src\Table590.cpp
# End Source File
# Begin Source File

SOURCE=..\src\Table590.h
# End Source File
# Begin Source File

SOURCE=..\src\Table603.cpp
# End Source File
# Begin Source File

SOURCE=..\src\Table603.h
# End Source File
# Begin Source File

SOURCE=..\src\Table604.cpp
# End Source File
# Begin Source File

SOURCE=..\src\Table604.h
# End Source File
# Begin Source File

SOURCE=..\src\Table610.cpp
# End Source File
# Begin Source File

SOURCE=..\src\Table610.h
# End Source File
# Begin Source File

SOURCE=..\src\Table630.cpp
# End Source File
# Begin Source File

SOURCE=..\src\Table630.h
# End Source File
# Begin Source File

SOURCE=..\src\TrainAgentSubscriber.cpp
# End Source File
# Begin Source File

SOURCE=..\src\TrainAgentSubscriber.h
# End Source File
# Begin Source File

SOURCE=..\src\TrainDVABroadcast.cpp
# End Source File
# Begin Source File

SOURCE=..\src\TrainDVABroadcast.h
# End Source File
# Begin Source File

SOURCE=..\src\TrainLiveBroadcast.cpp
# End Source File
# Begin Source File

SOURCE=..\src\TrainLiveBroadcast.h
# End Source File
# End Group
# End Group
# End Target
# End Project
