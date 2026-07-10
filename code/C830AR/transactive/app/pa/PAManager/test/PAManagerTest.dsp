# Microsoft Developer Studio Project File - Name="PAManagerTest" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Console Application" 0x0103

CFG=PAManagerTest - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "PAManagerTest.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "PAManagerTest.mak" CFG="PAManagerTest - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "PAManagerTest - Win32 Release" (based on "Win32 (x86) Console Application")
!MESSAGE "PAManagerTest - Win32 Debug" (based on "Win32 (x86) Console Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "PAManagerTest - Win32 Release"

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

!ELSEIF  "$(CFG)" == "PAManagerTest - Win32 Debug"

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
# ADD CPP /nologo /MDd /W3 /Gm /GR /GX /ZI /Od /I "..\..\..\.." /I "..\..\..\..\..\cots\omniORB\omniORB_4.0.5\include" /I "..\..\..\..\core\message\IDL\src" /I "..\..\..\..\..\cots\CppUnit\src" /I "..\..\..\..\core\message\src" /I "..\..\..\..\core\message\types" /I "..\..\..\..\core\exceptions\src" /I "src" /I "../../../../../../sdk/include/transactive" /I "../../../../../../build\win32_n\transactive" /I "../../../../../../sdk\include\transactive\core\process_management\idl\src" /D TA_ASSERT_FAIL=EXCEPTION /D "_AFXDLL" /D "STUBBED_PA_AGENT" /D "_DEBUG" /D "__WIN32__" /D "__x86__" /D "_NT_" /D __OSVERSION__=4 /D "WIN32" /D "_CONSOLE" /D "_MBCS" /FD /GZ /Zm200 /c
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0xc09 /i "..\\" /d "_DEBUG" /d "_AFXDLL"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386 /pdbtype:sept
# ADD LINK32 omniORB405_rtd.lib omniDynamic405_rtd.lib COS405_rtd.lib omnithread30_rtd.lib oci.lib Rpcrt4.lib htmlhelp.lib TA_Base_Core_d.lib TA_Base_Bus_d.lib /nologo /subsystem:console /debug /machine:I386 /pdbtype:sept /libpath:"../../../../tools/build/core_library/Debug" /libpath:"../../../../tools/build/bus_library/Debug" /libpath:"../../../../cots/htmlhelp/lib" /libpath:"../../../../cots/omniORB/omniORB_4.0.3/Winnt40/omni/lib/x86_win32" /libpath:"../../../../cots/oracle/920/winnt/lib" /libpath:"../../../../../../sdk/include/transactive" /libpath:"../../../../../../sdk\include\transactive\core\process_management\idl\src"
# SUBTRACT LINK32 /pdb:none /incremental:no

!ENDIF 

# Begin Target

# Name "PAManagerTest - Win32 Release"
# Name "PAManagerTest - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\src\Common.cpp
# End Source File
# Begin Source File

SOURCE=.\src\DvaListCtrlModelTestCases.cpp
# End Source File
# Begin Source File

SOURCE=.\src\DvaListCtrlTestCases.cpp
# End Source File
# Begin Source File

SOURCE=.\src\DvaMessageChangeSubscriberTestCases.cpp
# End Source File
# Begin Source File

SOURCE=.\src\HmiSelectionListenerTestCases.cpp
# End Source File
# Begin Source File

SOURCE=.\src\main.cpp
# End Source File
# Begin Source File

SOURCE=.\src\PAAgentNamedObjectStub.cpp
# End Source File
# Begin Source File

SOURCE=..\src\PAManager.rc
# End Source File
# Begin Source File

SOURCE=.\src\PaManagerStub.cpp
# End Source File
# Begin Source File

SOURCE=.\src\PAManagerTestCases.cpp
# End Source File
# Begin Source File

SOURCE=.\src\RunParamListenerTestCases.cpp
# End Source File
# Begin Source File

SOURCE=.\src\TestUtils.cpp
# End Source File
# Begin Source File

SOURCE=.\src\TrainListCtrlTestCases.cpp
# End Source File
# Begin Source File

SOURCE=.\src\TrainLiveStatusDlgTestCases.cpp
# End Source File
# Begin Source File

SOURCE=.\src\TrainLiveStatusModelTestCases.cpp
# End Source File
# Begin Source File

SOURCE=.\src\UnitTestAccessBridge.cpp
# End Source File
# Begin Source File

SOURCE=.\src\VersionGen.cpp
# End Source File
# Begin Source File

SOURCE=.\src\WindowStub.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\src\Common.h
# End Source File
# Begin Source File

SOURCE=.\src\DvaListCtrlModelTestCases.h
# End Source File
# Begin Source File

SOURCE=.\src\DvaListCtrlTestCases.h
# End Source File
# Begin Source File

SOURCE=.\src\DvaMessageChangeSubscriberTestCases.h
# End Source File
# Begin Source File

SOURCE=.\src\GenericStatusPageStub.h
# End Source File
# Begin Source File

SOURCE=.\src\HmiSelectionListenerTestCases.h
# End Source File
# Begin Source File

SOURCE=.\src\PAAgentNamedObjectStub.h
# End Source File
# Begin Source File

SOURCE=.\src\PAManagerTestCases.h
# End Source File
# Begin Source File

SOURCE=.\src\RunParamListenerTestCases.h
# End Source File
# Begin Source File

SOURCE=.\src\StdAfx.h
# End Source File
# Begin Source File

SOURCE=.\src\TestUtils.h
# End Source File
# Begin Source File

SOURCE=.\src\TrainListCtrlTestCases.h
# End Source File
# Begin Source File

SOURCE=.\src\TrainLiveStatusDlgTestCases.h
# End Source File
# Begin Source File

SOURCE=.\src\TrainLiveStatusModelTestCases.h
# End Source File
# Begin Source File

SOURCE=.\src\UnitTestAccessBridge.h
# End Source File
# Begin Source File

SOURCE=.\src\WindowStub.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=..\res\ahr_dele.ico
# End Source File
# Begin Source File

SOURCE=..\res\ahr_rec.ico
# End Source File
# Begin Source File

SOURCE=..\res\dva_icon.ico
# End Source File
# Begin Source File

SOURCE=..\res\live_icon.ico
# End Source File
# Begin Source File

SOURCE=..\res\music_dpt.ico
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

SOURCE=..\res\PAFormDVA.bmp
# End Source File
# Begin Source File

SOURCE=..\res\PAFormLive.bmp
# End Source File
# Begin Source File

SOURCE=..\res\PAFormOCCMusic.bmp
# End Source File
# Begin Source File

SOURCE=..\res\PAFormPlay.bmp
# End Source File
# Begin Source File

SOURCE=..\res\PAFormRecord.bmp
# End Source File
# Begin Source File

SOURCE=..\res\PAFormStnMusic.bmp
# End Source File
# Begin Source File

SOURCE=..\res\PAFormStop.bmp
# End Source File
# Begin Source File

SOURCE=..\res\PAManager.ico
# End Source File
# Begin Source File

SOURCE=..\res\play.ico
# End Source File
# Begin Source File

SOURCE=..\res\stop.ico
# End Source File
# End Group
# Begin Group "PAManager source"

# PROP Default_Filter ""
# Begin Group "TestTodoList"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\src\AdhocMessageRecordModel.cpp
# End Source File
# Begin Source File

SOURCE=..\src\AdhocMessageRecordModel.h
# End Source File
# Begin Source File

SOURCE=..\src\AsyncCancelDialog.cpp
# End Source File
# Begin Source File

SOURCE=..\src\AsyncCancelDialog.h
# End Source File
# Begin Source File

SOURCE=..\src\ChangeBroadcastId.cpp
# End Source File
# Begin Source File

SOURCE=..\src\ChangeBroadcastId.h
# End Source File
# Begin Source File

SOURCE=..\src\DVAStatusModel.cpp
# End Source File
# Begin Source File

SOURCE=..\src\DVAStatusModel.h
# End Source File
# Begin Source File

SOURCE=..\src\GenericStatusModel.cpp
# End Source File
# Begin Source File

SOURCE=..\src\GenericStatusModel.h
# End Source File
# Begin Source File

SOURCE=..\src\GraphworxComms.cpp
# End Source File
# Begin Source File

SOURCE=..\src\GraphworxComms.h
# End Source File
# Begin Source File

SOURCE=..\src\LiveStatusModel.cpp
# End Source File
# Begin Source File

SOURCE=..\src\LiveStatusModel.h
# End Source File
# Begin Source File

SOURCE=..\src\StationDvaStatusModel.cpp
# End Source File
# Begin Source File

SOURCE=..\src\StationDvaStatusModel.h
# End Source File
# Begin Source File

SOURCE=..\src\StationLiveStatusModel.cpp
# End Source File
# Begin Source File

SOURCE=..\src\StationLiveStatusModel.h
# End Source File
# Begin Source File

SOURCE=..\src\StationStatusHelper.cpp
# End Source File
# Begin Source File

SOURCE=..\src\StationStatusHelper.h
# End Source File
# Begin Source File

SOURCE=..\src\TrainDvaStatusModel.cpp
# End Source File
# Begin Source File

SOURCE=..\src\TrainDvaStatusModel.h
# End Source File
# Begin Source File

SOURCE=..\src\TrainListUpdateContainer.cpp
# End Source File
# Begin Source File

SOURCE=..\src\TrainListUpdateContainer.h
# End Source File
# Begin Source File

SOURCE=..\src\TrainStatusHelper.cpp
# End Source File
# Begin Source File

SOURCE=..\src\TrainStatusHelper.h
# End Source File
# End Group
# Begin Group "Not Testable"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\src\AdhocMessageRecordPage.cpp
# End Source File
# Begin Source File

SOURCE=..\src\AdhocMessageRecordPage.h
# End Source File
# Begin Source File

SOURCE=..\src\BroadcastParametersDlg.cpp
# End Source File
# Begin Source File

SOURCE=..\src\BroadcastParametersDlg.h
# End Source File
# Begin Source File

SOURCE=..\src\CreateNewGroupDlg.cpp
# End Source File
# Begin Source File

SOURCE=..\src\Createnewgroupdlg.h
# End Source File
# Begin Source File

SOURCE=..\src\DlgStatusNameWarning.cpp
# End Source File
# Begin Source File

SOURCE=..\src\DlgStatusNameWarning.h
# End Source File
# Begin Source File

SOURCE=..\src\DvaStatusPage.cpp
# End Source File
# Begin Source File

SOURCE=..\src\DvaStatusPage.h
# End Source File
# Begin Source File

SOURCE=..\src\DvaVersionListCtrl.cpp
# End Source File
# Begin Source File

SOURCE=..\src\DvaVersionListCtrl.h
# End Source File
# Begin Source File

SOURCE=..\src\DVAVersionsPage.cpp
# End Source File
# Begin Source File

SOURCE=..\src\DVAVersionsPage.h
# End Source File
# Begin Source File

SOURCE=..\src\GenericStatusPage.cpp
# End Source File
# Begin Source File

SOURCE=..\src\GenericStatusPage.h
# End Source File
# Begin Source File

SOURCE=..\src\IGenericBroadcastPage.h
# End Source File
# Begin Source File

SOURCE=..\src\IMultipleInstanceIdentifier.h
# End Source File
# Begin Source File

SOURCE=..\src\IRightsChangeListener.h
# End Source File
# Begin Source File

SOURCE=..\src\IRunParamChangedListener.h
# End Source File
# Begin Source File

SOURCE=..\src\IStationBroadcastPage.h
# End Source File
# Begin Source File

SOURCE=..\src\ITrainBroadcastPage.h
# End Source File
# Begin Source File

SOURCE=..\src\IWindow.h
# End Source File
# Begin Source File

SOURCE=..\src\LiveStatusPage.cpp
# End Source File
# Begin Source File

SOURCE=..\src\LiveStatusPage.h
# End Source File
# Begin Source File

SOURCE=..\src\LockControlRedraw.cpp
# End Source File
# Begin Source File

SOURCE=..\src\LockControlRedraw.h
# End Source File
# Begin Source File

SOURCE=..\src\PAManagerDlg.cpp
# End Source File
# Begin Source File

SOURCE=..\src\PAManagerDlg.h
# End Source File
# Begin Source File

SOURCE=..\src\PAZoneListCtrl.cpp
# End Source File
# Begin Source File

SOURCE=..\src\PAZoneListCtrl.h
# End Source File
# Begin Source File

SOURCE=..\src\StationBroadcastPage.cpp
# End Source File
# Begin Source File

SOURCE=..\src\StationBroadcastPage.h
# End Source File
# Begin Source File

SOURCE=..\src\StationDvaStatusDlg.cpp
# End Source File
# Begin Source File

SOURCE=..\src\StationDvaStatusDlg.h
# End Source File
# Begin Source File

SOURCE=..\src\StationLiveStatusDlg.cpp
# End Source File
# Begin Source File

SOURCE=..\src\StationLiveStatusDlg.h
# End Source File
# Begin Source File

SOURCE=..\src\TAPropertySheet.cpp
# End Source File
# Begin Source File

SOURCE=..\src\TAPropertySheet.h
# End Source File
# Begin Source File

SOURCE=..\src\TrainBroadcastPage.cpp
# End Source File
# Begin Source File

SOURCE=..\src\TrainBroadcastPage.h
# End Source File
# Begin Source File

SOURCE=..\src\TrainDvaStatusDlg.cpp
# End Source File
# Begin Source File

SOURCE=..\src\TrainDvaStatusDlg.h
# End Source File
# Begin Source File

SOURCE=..\src\TrainDvaVersionsPage.cpp
# End Source File
# Begin Source File

SOURCE=..\src\TrainDvaVersionsPage.h
# End Source File
# Begin Source File

SOURCE=..\src\TrainListCtrl.cpp
# End Source File
# Begin Source File

SOURCE=..\src\TrainListCtrl.h
# End Source File
# End Group
# Begin Group "Indeterminate"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\src\CachedConfig.cpp
# End Source File
# Begin Source File

SOURCE=..\src\CachedConfig.h
# End Source File
# Begin Source File

SOURCE=..\src\PALayoutHelper.cpp
# End Source File
# Begin Source File

SOURCE=..\src\PALayoutHelper.h
# End Source File
# End Group
# Begin Group "Low priority/risk"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\src\CachedMapChangedWindowNotificationHandler.cpp
# End Source File
# Begin Source File

SOURCE=..\src\CachedMapChangedWindowNotificationHandler.h
# End Source File
# Begin Source File

SOURCE=..\src\CachedMapLoader.cpp
# End Source File
# Begin Source File

SOURCE=..\src\CachedMapLoader.h
# End Source File
# Begin Source File

SOURCE=..\src\GenericBroadcastModel.cpp
# End Source File
# Begin Source File

SOURCE=..\src\GenericBroadcastModel.h
# End Source File
# Begin Source File

SOURCE=..\src\PAManager.cpp

!IF  "$(CFG)" == "PAManagerTest - Win32 Release"

!ELSEIF  "$(CFG)" == "PAManagerTest - Win32 Debug"

# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\src\PAManager.h
# End Source File
# Begin Source File

SOURCE=..\src\PAManagerGUI.cpp

!IF  "$(CFG)" == "PAManagerTest - Win32 Release"

!ELSEIF  "$(CFG)" == "PAManagerTest - Win32 Debug"

# SUBTRACT CPP /WX

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\src\PAManagerGUI.h
# End Source File
# Begin Source File

SOURCE=..\src\PAProgressReportSubscriber.cpp
# End Source File
# Begin Source File

SOURCE=..\src\PAProgressReportSubscriber.h
# End Source File
# Begin Source File

SOURCE=..\src\PAStatusNameChangeSubscriber.cpp
# End Source File
# Begin Source File

SOURCE=..\src\PAStatusNameChangeSubscriber.h
# End Source File
# Begin Source File

SOURCE=..\src\RightsChangedWindowNotificationHandler.cpp
# End Source File
# Begin Source File

SOURCE=..\src\RightsChangedWindowNotificationHandler.h
# End Source File
# Begin Source File

SOURCE=..\src\TrainRadioPttMonitor.cpp
# End Source File
# Begin Source File

SOURCE=..\src\TrainRadioPttMonitor.h
# End Source File
# End Group
# Begin Group "High Risk"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\src\OperatorTerminalDvaPlaybackModel.cpp
# End Source File
# Begin Source File

SOURCE=..\src\OperatorTerminalDvaPlaybackModel.h
# End Source File
# Begin Source File

SOURCE=..\src\PAErrorHandler.cpp
# End Source File
# Begin Source File

SOURCE=..\src\PAErrorHandler.h
# End Source File
# Begin Source File

SOURCE=..\src\PAManagerModel.cpp
# End Source File
# Begin Source File

SOURCE=..\src\PAManagerModel.h
# End Source File
# Begin Source File

SOURCE=..\src\PaRightsManager.cpp
# End Source File
# Begin Source File

SOURCE=..\src\PaRightsManager.h
# End Source File
# Begin Source File

SOURCE=..\src\StationBroadcastModel.cpp
# End Source File
# Begin Source File

SOURCE=..\src\StationBroadcastModel.h
# End Source File
# Begin Source File

SOURCE=..\src\TrainBroadcastModel.cpp
# End Source File
# Begin Source File

SOURCE=..\src\TrainBroadcastModel.h
# End Source File
# End Group
# Begin Group "No need to test"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\src\DvaVersionChangeSubscriber.cpp
# End Source File
# Begin Source File

SOURCE=..\src\DvaVersionChangeSubscriber.h
# End Source File
# Begin Source File

SOURCE=..\src\MemTracker.cpp
# End Source File
# Begin Source File

SOURCE=..\src\MemTracker.h
# End Source File
# Begin Source File

SOURCE=..\src\PrioritySchemeUpdateSubscriber.cpp
# End Source File
# Begin Source File

SOURCE=..\src\PrioritySchemeUpdateSubscriber.h
# End Source File
# Begin Source File

SOURCE=..\src\resource.h
# End Source File
# Begin Source File

SOURCE=..\src\StdAfx.cpp
# End Source File
# Begin Source File

SOURCE=..\src\StdAfx.h
# End Source File
# End Group
# Begin Group "Tested"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\src\DvaListCtrl.cpp
# End Source File
# Begin Source File

SOURCE=..\src\DvaListCtrl.h
# End Source File
# Begin Source File

SOURCE=..\src\DvaListCtrlModel.cpp
# End Source File
# Begin Source File

SOURCE=..\src\DvaListCtrlModel.h
# End Source File
# Begin Source File

SOURCE=..\src\DvaMessageChangeSubscriber.cpp
# End Source File
# Begin Source File

SOURCE=..\src\DvaMessageChangeSubscriber.h
# End Source File
# Begin Source File

SOURCE=..\src\Element.h
# End Source File
# Begin Source File

SOURCE=..\src\Element.inl
# End Source File
# Begin Source File

SOURCE=..\src\GenericMessageSubscriber.h
# End Source File
# Begin Source File

SOURCE=..\src\GenericMessageSubscriber.inl
# End Source File
# Begin Source File

SOURCE=..\src\HMISelectionListener.cpp
# End Source File
# Begin Source File

SOURCE=..\src\HMISelectionListener.h
# End Source File
# Begin Source File

SOURCE=..\src\RunParamListener.cpp
# End Source File
# Begin Source File

SOURCE=..\src\RunParamListener.h
# End Source File
# Begin Source File

SOURCE=..\src\UpdateBuffer.h
# End Source File
# Begin Source File

SOURCE=..\src\UpdateBuffer.inl
# End Source File
# End Group
# Begin Group "intermediate"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\src\TrainLiveStatusDlg.cpp
# End Source File
# Begin Source File

SOURCE=..\src\TrainLiveStatusDlg.h
# End Source File
# Begin Source File

SOURCE=..\src\TrainLiveStatusModel.cpp
# End Source File
# Begin Source File

SOURCE=..\src\TrainLiveStatusModel.h
# End Source File
# End Group
# End Group
# Begin Group "business_tests"

# PROP Default_Filter ""
# Begin Group "MessageSubscriber"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\..\..\bus\pa\MessageSubscriber\test\src\GenericMessageSubscriberTest.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\..\bus\pa\MessageSubscriber\test\src\GenericMessageSubscriberTest.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\bus\pa\MessageSubscriber\test\src\GenericMessageSubscriberTestCases.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\..\bus\pa\MessageSubscriber\test\src\GenericMessageSubscriberTestCases.h
# End Source File
# End Group
# Begin Group "Common"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\..\..\bus\pa\Common\test\src\ElementTestCases.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\..\bus\pa\Common\test\src\ElementTestCases.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\bus\pa\Common\test\src\UpdateBufferTestCases.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\..\bus\pa\Common\test\src\UpdateBufferTestCases.h
# End Source File
# End Group
# End Group
# End Target
# End Project
