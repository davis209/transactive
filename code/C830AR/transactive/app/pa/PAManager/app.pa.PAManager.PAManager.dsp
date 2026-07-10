# Microsoft Developer Studio Project File - Name="app.pa.PAManager.PAManager" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Application" 0x0101

CFG=app.pa.PAManager.PAManager - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "app.pa.PAManager.PAManager.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "app.pa.PAManager.PAManager.mak" CFG="app.pa.PAManager.PAManager - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "app.pa.PAManager.PAManager - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "app.pa.PAManager.PAManager - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "app.pa.PAManager.PAManager - Win32 Release"

# PROP BASE Use_MFC 6
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 6
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "../../../../../bin/win32_nr"
# PROP Intermediate_Dir "../../../../../build/win32_nr/transactive/app/pa/PAManager"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MD /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_AFXDLL" /Yu"stdafx.h" /FD /c
# ADD CPP /nologo /MD /W3 /GR /GX /O2 /I "../../../../../build/win32_n/transactive" /I "../../../../../sdk/include/transactive" /I "../../../../../sdk/include/transactive/bus/trains/trainagentcorbadef/idl/src" /I "../../../" /I "../../../../../sdk/include/transactive/core/exceptions/src" /I "../../../../../sdk/include/transactive/core/threads/src" /I "../../../../../sdk/include/transactive/core/synchronisation/src" /I "../../../../../sdk/include/transactive/core/process_management\idl\src" /I "..\..\..\..\..\sdk\omniORB\omniORB_4.0.5\include" /I "..\..\..\..\..\sdk\include\cots\boost\boost_1_31_0" /I "..\..\..\..\..\sdk\include\cots\ACE\5_3\ACE_wrappers" /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "_AFXDLL" /D "_MBCS" /D "__WIN32__" /D "__NT__" /D __OSVERSION__=4 /D "__x86__" /D _WIN32_WINNT=0x0500 /FD /Zm200 /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0xc09 /d "NDEBUG" /d "_AFXDLL"
# ADD RSC /l 0xc09 /d "NDEBUG" /d "_AFXDLL"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 /nologo /subsystem:windows /machine:I386
# ADD LINK32 htmlhelp.lib acemfc.lib TA_Base_Bus.lib TA_IRS_Core.lib TA_IRS_Bus.lib omniORB405_rt.lib omniDynamic405_rt.lib omnithread30_rt.lib COS405_rt.lib TA_Base_Core.lib oci.lib Rpcrt4.lib stracelib.lib /nologo /subsystem:windows /pdb:none /machine:I386 /nodefaultlib:"LIBC" /out:"../../../../../bin/win32_nr/PAManager.exe" /libpath:"../../../../../sdk/win32_nr" /libpath:"../../../../../sdk/lib" /libpath:"../../../../../build/win32_nr" /libpath:"../../../../../sdk/omniORB/omniORB_4.0.5/lib/x86_win32" /libpath:"../../../../../sdk/include/cots/WinStackTrace\build\Release"

!ELSEIF  "$(CFG)" == "app.pa.PAManager.PAManager - Win32 Debug"

# PROP BASE Use_MFC 6
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 6
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "../../../../../bin/win32_nd"
# PROP Intermediate_Dir "../../../../../build/win32_nd/transactive/app/pa/PAManager"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_AFXDLL" /Yu"stdafx.h" /FD /GZ /c
# ADD CPP /nologo /MDd /W3 /Gm /GR /GX /ZI /Od /I "../../../../../build/win32_n/transactive" /I "../../../../../sdk/include/transactive" /I "../../../../../sdk/include/transactive/bus/trains/trainagentcorbadef/idl/src" /I "../../../" /I "../../../../../sdk/include/transactive/core/exceptions/src" /I "../../../../../sdk/include/transactive/core/threads/src" /I "../../../../../sdk/include/transactive/core/synchronisation/src" /I "../../../../../sdk/include/transactive/core/process_management\idl\src" /I "..\..\..\..\..\sdk\omniORB\omniORB_4.0.5\include" /I "..\..\..\..\..\sdk\include\cots\boost\boost_1_31_0" /I "..\..\..\..\..\sdk\include\cots\ACE\5_3\ACE_wrappers" /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /D "_AFXDLL" /D "_MBCS" /D "__WIN32__" /D "__NT__" /D __OSVERSION__=4 /D "__x86__" /FD /GZ /Zm200 /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0xc09 /d "_DEBUG" /d "_AFXDLL"
# ADD RSC /l 0xc09 /d "_DEBUG" /d "_AFXDLL"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept
# ADD LINK32 COS405_rtd.lib omniORB405_rtd.lib omnithread30_rtd.lib omniDynamic405_rtd.lib oci.lib htmlhelp.lib Rpcrt4.lib acemfcd.lib TA_Base_Core_d.lib TA_Base_Bus_d.lib TA_IRS_Core_d.lib TA_IRS_Bus_d.lib stracelib.lib /nologo /subsystem:windows /debug /debugtype:both /machine:I386 /out:"../../../../../bin/win32_nd/PAManager.exe" /pdbtype:sept /libpath:"../../../../../sdk/omniORB/omniORB_4.0.5/lib/x86_win32" /libpath:"../../../../../sdk/win32_nd" /libpath:"../../../../../sdk/lib" /libpath:"../../../../../build/win32_nd" /libpath:"../../../../../sdk/include/cots/WinStackTrace\build\Debug"

!ENDIF 

# Begin Target

# Name "app.pa.PAManager.PAManager - Win32 Release"
# Name "app.pa.PAManager.PAManager - Win32 Debug"
# Begin Group "Common"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\src\CachedConfig.cpp
# End Source File
# Begin Source File

SOURCE=.\src\CachedConfig.h
# End Source File
# Begin Source File

SOURCE=.\src\CachedMapLoader.cpp
# End Source File
# Begin Source File

SOURCE=.\src\CachedMapLoader.h
# End Source File
# Begin Source File

SOURCE=.\src\GraphworxComms.cpp
# End Source File
# Begin Source File

SOURCE=.\src\GraphworxComms.h
# End Source File
# Begin Source File

SOURCE=.\src\PAManager.cpp
# End Source File
# Begin Source File

SOURCE=.\src\PAManager.h
# End Source File
# Begin Source File

SOURCE=.\src\PAManager.rc
# End Source File
# Begin Source File

SOURCE=.\src\PAManagerDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\src\PAManagerDlg.h
# End Source File
# Begin Source File

SOURCE=.\src\PAManagerGUI.cpp
# End Source File
# Begin Source File

SOURCE=.\src\PAManagerGUI.h
# End Source File
# Begin Source File

SOURCE=.\src\PAManagerModel.cpp
# End Source File
# Begin Source File

SOURCE=.\src\PAManagerModel.h
# End Source File
# Begin Source File

SOURCE=.\src\PaRightsManager.cpp
# End Source File
# Begin Source File

SOURCE=.\src\PaRightsManager.h
# End Source File
# Begin Source File

SOURCE=.\src\SplashScreenEx.cpp
# End Source File
# Begin Source File

SOURCE=.\src\SplashScreenEx.h
# End Source File
# Begin Source File

SOURCE=.\src\TAPropertySheet.cpp
# End Source File
# Begin Source File

SOURCE=.\src\TAPropertySheet.h
# End Source File
# Begin Source File

SOURCE=.\src\VersionGen.cpp
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=.\res\ahr_dele.ico
# End Source File
# Begin Source File

SOURCE=.\res\ahr_play.ico
# End Source File
# Begin Source File

SOURCE=.\res\ahr_rec.ico
# End Source File
# Begin Source File

SOURCE=.\res\dva_icon.ico
# End Source File
# Begin Source File

SOURCE=.\res\ico00001.ico
# End Source File
# Begin Source File

SOURCE=.\res\ico00002.ico
# End Source File
# Begin Source File

SOURCE=.\res\ico00003.ico
# End Source File
# Begin Source File

SOURCE=.\res\ico00004.ico
# End Source File
# Begin Source File

SOURCE=.\res\ico00005.ico
# End Source File
# Begin Source File

SOURCE=.\res\icon1.ico
# End Source File
# Begin Source File

SOURCE=.\res\icon2.ico
# End Source File
# Begin Source File

SOURCE=.\res\icon_rec.ico
# End Source File
# Begin Source File

SOURCE=.\res\live_icon.ico
# End Source File
# Begin Source File

SOURCE=.\res\music_dpt.ico
# End Source File
# Begin Source File

SOURCE=.\res\music_occ.ico
# End Source File
# Begin Source File

SOURCE=.\res\music_off.ico
# End Source File
# Begin Source File

SOURCE=.\res\music_stn.ico
# End Source File
# Begin Source File

SOURCE=.\res\PAFormDVA.bmp
# End Source File
# Begin Source File

SOURCE=.\res\PAFormLive.bmp
# End Source File
# Begin Source File

SOURCE=.\res\PAFormOCCMusic.bmp
# End Source File
# Begin Source File

SOURCE=.\res\PAFormPlay.bmp
# End Source File
# Begin Source File

SOURCE=.\res\PAFormRecord.bmp
# End Source File
# Begin Source File

SOURCE=.\res\PAFormStnMusic.bmp
# End Source File
# Begin Source File

SOURCE=.\res\PAFormStop.bmp
# End Source File
# Begin Source File

SOURCE=.\res\PAManager.ico
# End Source File
# Begin Source File

SOURCE=.\res\play.ico
# End Source File
# Begin Source File

SOURCE=.\res\play_ico.ico
# End Source File
# Begin Source File

SOURCE=.\src\resource.h

!IF  "$(CFG)" == "app.pa.PAManager.PAManager - Win32 Release"

!ELSEIF  "$(CFG)" == "app.pa.PAManager.PAManager - Win32 Debug"

# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\res\splash.bmp
# End Source File
# Begin Source File

SOURCE=.\res\stnmusic.ico
# End Source File
# Begin Source File

SOURCE=.\res\stop.ico
# End Source File
# Begin Source File

SOURCE=.\res\toolbar1.bmp
# End Source File
# End Group
# Begin Group "Station Page Classes"

# PROP Default_Filter ""
# Begin Group "Models"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\src\StationBroadcastModel.cpp
# End Source File
# Begin Source File

SOURCE=.\src\StationBroadcastModel.h
# End Source File
# Begin Source File

SOURCE=.\src\StationDvaStatusModel.cpp
# End Source File
# Begin Source File

SOURCE=.\src\StationDvaStatusModel.h
# End Source File
# Begin Source File

SOURCE=.\src\StationLiveStatusModel.cpp
# End Source File
# Begin Source File

SOURCE=.\src\StationLiveStatusModel.h
# End Source File
# End Group
# Begin Group "Pages"

# PROP Default_Filter ""
# Begin Group "StationStatusDlg"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\src\StationDvaStatusDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\src\StationDvaStatusDlg.h
# End Source File
# Begin Source File

SOURCE=.\src\StationLiveStatusDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\src\StationLiveStatusDlg.h
# End Source File
# End Group
# Begin Source File

SOURCE=.\src\IStationBroadcastPage.h
# End Source File
# Begin Source File

SOURCE=.\src\StationBroadcastPage.cpp
# End Source File
# Begin Source File

SOURCE=.\src\StationBroadcastPage.h
# End Source File
# End Group
# Begin Source File

SOURCE=.\src\AdhocMessageRecordModel.cpp
# End Source File
# Begin Source File

SOURCE=.\src\AdhocMessageRecordModel.h
# End Source File
# Begin Source File

SOURCE=.\src\AdhocMessageRecordPage.cpp
# End Source File
# Begin Source File

SOURCE=.\src\AdhocMessageRecordPage.h
# End Source File
# Begin Source File

SOURCE=.\src\DvaListCtrl.cpp
# End Source File
# Begin Source File

SOURCE=.\src\DvaListCtrl.h
# End Source File
# Begin Source File

SOURCE=.\src\DvaListCtrlModel.cpp
# End Source File
# Begin Source File

SOURCE=.\src\DvaListCtrlModel.h
# End Source File
# Begin Source File

SOURCE=.\src\ListCtrlSort.cpp
# End Source File
# Begin Source File

SOURCE=.\src\ListCtrlSort.h
# End Source File
# Begin Source File

SOURCE=.\src\OperatorTerminalDvaPlaybackModel.cpp
# End Source File
# Begin Source File

SOURCE=.\src\OperatorTerminalDvaPlaybackModel.h
# End Source File
# Begin Source File

SOURCE=.\src\PAZoneListCtrl.cpp
# End Source File
# Begin Source File

SOURCE=.\src\PAZoneListCtrl.h
# End Source File
# Begin Source File

SOURCE=.\src\SortedListCtrl.h
# End Source File
# End Group
# Begin Group "Train Page Classes"

# PROP Default_Filter ""
# Begin Group "Train selector"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\..\bus\trains\TrainSelectionComponent\src\ITrainSelectorGUI.h
# End Source File
# Begin Source File

SOURCE=..\..\..\bus\trains\TrainSelectionComponent\src\TrainSelector.h
# End Source File
# End Group
# Begin Group "Model"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\src\TrainBroadcastModel.cpp
# End Source File
# Begin Source File

SOURCE=.\src\TrainBroadcastModel.h
# End Source File
# Begin Source File

SOURCE=.\src\TrainDvaStatusModel.cpp
# End Source File
# Begin Source File

SOURCE=.\src\TrainDvaStatusModel.h
# End Source File
# Begin Source File

SOURCE=.\src\TrainLiveStatusModel.cpp
# End Source File
# Begin Source File

SOURCE=.\src\TrainLiveStatusModel.h
# End Source File
# End Group
# Begin Group "Components"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\src\TrainListUpdateContainer.cpp
# End Source File
# Begin Source File

SOURCE=.\src\TrainListUpdateContainer.h
# End Source File
# Begin Source File

SOURCE=.\src\TrainRadioPttMonitor.cpp
# End Source File
# Begin Source File

SOURCE=.\src\TrainRadioPttMonitor.h
# End Source File
# End Group
# Begin Group "Page"

# PROP Default_Filter ""
# Begin Group "TrainStatusDlg"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\src\TrainDvaStatusDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\src\TrainDvaStatusDlg.h
# End Source File
# Begin Source File

SOURCE=.\src\TrainLiveStatusDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\src\TrainLiveStatusDlg.h
# End Source File
# End Group
# Begin Source File

SOURCE=.\src\ITrainBroadcastPage.h
# End Source File
# Begin Source File

SOURCE=.\src\TrainBroadcastPage.cpp
# End Source File
# Begin Source File

SOURCE=.\src\TrainBroadcastPage.h
# End Source File
# End Group
# End Group
# Begin Group "Versions Page Classes"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\src\DvaVersionListCtrl.cpp
# End Source File
# Begin Source File

SOURCE=.\src\DvaVersionListCtrl.h
# End Source File
# Begin Source File

SOURCE=.\src\DVAVersionsPage.cpp
# End Source File
# Begin Source File

SOURCE=.\src\DVAVersionsPage.h
# End Source File
# Begin Source File

SOURCE=.\src\TrainDvaVersionsPage.cpp
# End Source File
# Begin Source File

SOURCE=.\src\TrainDvaVersionsPage.h
# End Source File
# End Group
# Begin Group "misc"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\src\AsyncCancelDialog.cpp
# End Source File
# Begin Source File

SOURCE=.\src\AsyncCancelDialog.h
# End Source File
# Begin Source File

SOURCE=.\src\BroadcastParametersDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\src\BroadcastParametersDlg.h
# End Source File
# Begin Source File

SOURCE=.\src\CreateNewGroupDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\src\CreateNewGroupDlg.h
# End Source File
# Begin Source File

SOURCE=.\src\DlgStatusNameWarning.cpp
# End Source File
# Begin Source File

SOURCE=.\src\DlgStatusNameWarning.h
# End Source File
# Begin Source File

SOURCE=.\src\LockControlRedraw.cpp
# End Source File
# Begin Source File

SOURCE=.\src\LockControlRedraw.h
# End Source File
# Begin Source File

SOURCE=.\src\MemTracker.cpp
# End Source File
# Begin Source File

SOURCE=.\src\MemTracker.h
# End Source File
# Begin Source File

SOURCE=.\src\PAErrorHandler.cpp
# End Source File
# Begin Source File

SOURCE=.\src\PAErrorHandler.h
# End Source File
# Begin Source File

SOURCE=.\src\StdAfx.cpp
# End Source File
# Begin Source File

SOURCE=.\src\StdAfx.h
# End Source File
# End Group
# Begin Group "From PA Agent"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\..\core\message\types\PAAgentComms_MessageTypes.h
# End Source File
# Begin Source File

SOURCE=..\..\..\core\exceptions\src\PAAgentExceptions.h
# End Source File
# End Group
# Begin Group "Common_GUI"

# PROP Default_Filter ""
# Begin Group "Status Dialog"

# PROP Default_Filter ""
# Begin Group "DVA Status"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\src\ChangeBroadcastId.cpp
# End Source File
# Begin Source File

SOURCE=.\src\ChangeBroadcastId.h
# End Source File
# Begin Source File

SOURCE=.\src\DVAStatusModel.cpp
# End Source File
# Begin Source File

SOURCE=.\src\DVAStatusModel.h
# End Source File
# Begin Source File

SOURCE=.\src\DvaStatusPage.cpp
# End Source File
# Begin Source File

SOURCE=.\src\DvaStatusPage.h
# End Source File
# End Group
# Begin Group "Live Status"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\src\LiveStatusModel.cpp
# End Source File
# Begin Source File

SOURCE=.\src\LiveStatusModel.h
# End Source File
# Begin Source File

SOURCE=.\src\LiveStatusPage.cpp
# End Source File
# Begin Source File

SOURCE=.\src\LiveStatusPage.h
# End Source File
# End Group
# Begin Source File

SOURCE=.\src\GenericStatusModel.cpp
# End Source File
# Begin Source File

SOURCE=.\src\GenericStatusModel.h
# End Source File
# Begin Source File

SOURCE=.\src\GenericStatusPage.cpp
# End Source File
# Begin Source File

SOURCE=.\src\GenericStatusPage.h
# End Source File
# End Group
# Begin Group "Broadcast"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\src\GenericBroadcastModel.cpp
# End Source File
# Begin Source File

SOURCE=.\src\GenericBroadcastModel.h
# End Source File
# Begin Source File

SOURCE=.\src\IGenericBroadcastPage.h
# End Source File
# End Group
# Begin Source File

SOURCE=.\src\PALayoutHelper.cpp
# End Source File
# Begin Source File

SOURCE=.\src\PALayoutHelper.h
# End Source File
# Begin Source File

SOURCE=.\src\StationStatusHelper.cpp
# End Source File
# Begin Source File

SOURCE=.\src\StationStatusHelper.h
# End Source File
# Begin Source File

SOURCE=.\src\TrainStatusHelper.cpp
# End Source File
# Begin Source File

SOURCE=.\src\TrainStatusHelper.h
# End Source File
# End Group
# Begin Group "Listeners"

# PROP Default_Filter ""
# Begin Group "Helper Components"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\src\CachedMapChangedWindowNotificationHandler.cpp
# End Source File
# Begin Source File

SOURCE=.\src\CachedMapChangedWindowNotificationHandler.h
# End Source File
# Begin Source File

SOURCE=..\PAAgent\src\ICachedMapObserverRegistry.h
# End Source File
# Begin Source File

SOURCE=.\src\IRightsChangeListener.h
# End Source File
# Begin Source File

SOURCE=.\src\RightsChangedWindowNotificationHandler.cpp
# End Source File
# Begin Source File

SOURCE=.\src\RightsChangedWindowNotificationHandler.h
# End Source File
# Begin Source File

SOURCE=.\src\RunParamListener.cpp
# End Source File
# Begin Source File

SOURCE=.\src\RunParamListener.h
# End Source File
# End Group
# Begin Source File

SOURCE=.\src\DvaMessageChangeSubscriber.cpp
# End Source File
# Begin Source File

SOURCE=.\src\DvaMessageChangeSubscriber.h
# End Source File
# Begin Source File

SOURCE=.\src\DvaPrivateVersionChangeSubscriber.cpp
# End Source File
# Begin Source File

SOURCE=.\src\DvaPrivateVersionChangeSubscriber.h
# End Source File
# Begin Source File

SOURCE=.\src\DvaPublicVersionChangeSubscriber.cpp
# End Source File
# Begin Source File

SOURCE=.\src\DvaPublicVersionChangeSubscriber.h
# End Source File
# Begin Source File

SOURCE=.\src\DvaVersionChangeSubscriber.cpp
# End Source File
# Begin Source File

SOURCE=.\src\DvaVersionChangeSubscriber.h
# End Source File
# Begin Source File

SOURCE=.\src\HMISelectionListener.cpp
# End Source File
# Begin Source File

SOURCE=.\src\HMISelectionListener.h
# End Source File
# Begin Source File

SOURCE=.\src\PAProgressReportSubscriber.cpp
# End Source File
# Begin Source File

SOURCE=.\src\PAProgressReportSubscriber.h
# End Source File
# Begin Source File

SOURCE=.\src\PAStatusNameChangeSubscriber.cpp
# End Source File
# Begin Source File

SOURCE=.\src\PAStatusNameChangeSubscriber.h
# End Source File
# Begin Source File

SOURCE=.\src\PrioritySchemeUpdateSubscriber.cpp
# End Source File
# Begin Source File

SOURCE=.\src\PrioritySchemeUpdateSubscriber.h
# End Source File
# End Group
# Begin Source File

SOURCE=..\..\..\..\..\sdk\include\transactive\app\pa\PAManager\src\PAErrorHandler.h
# End Source File
# Begin Source File

SOURCE=.\ReadMe.txt
# End Source File
# End Target
# End Project
# Section app.pa.PAManager.PAManager : {BEF6E003-A874-101A-8BBA-00AA00300CAB}
# 	2:5:Class:COleFont
# 	2:10:HeaderFile:font.h
# 	2:8:ImplFile:font.cpp
# End Section
# Section app.pa.PAManager.PAManager : {20DD1B9B-87C4-11D1-8BE3-0000F8754DA1}
# 	2:5:Class:CDTPicker
# 	2:10:HeaderFile:dtpicker.h
# 	2:8:ImplFile:dtpicker.cpp
# End Section
# Section app.pa.PAManager.PAManager : {7BF80981-BF32-101A-8BBB-00AA00300CAB}
# 	2:5:Class:CPicture
# 	2:10:HeaderFile:picture.h
# 	2:8:ImplFile:picture.cpp
# End Section
# Section app.pa.PAManager.PAManager : {20DD1B9E-87C4-11D1-8BE3-0000F8754DA1}
# 	2:21:DefaultSinkHeaderFile:dtpicker.h
# 	2:16:DefaultSinkClass:CDTPicker
# End Section
