# Microsoft Developer Studio Project File - Name="app.maintenance_management.job_request_gui.JobRequestMgr" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Application" 0x0101

CFG=app.maintenance_management.job_request_gui.JobRequestMgr - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "app.maintenance_management.job_request_gui.JobRequestMgr.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "app.maintenance_management.job_request_gui.JobRequestMgr.mak" CFG="app.maintenance_management.job_request_gui.JobRequestMgr - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "app.maintenance_management.job_request_gui.JobRequestMgr - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "app.maintenance_management.job_request_gui.JobRequestMgr - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "app.maintenance_management.job_request_gui.JobRequestMgr - Win32 Release"

# PROP BASE Use_MFC 6
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 6
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "../../../../../bin/win32_nr"
# PROP Intermediate_Dir "../../../../../build/win32_nr/transactive/app/maintenance_management/job_request_gui"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MD /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_AFXDLL" /Yu"stdafx.h" /FD /c
# ADD CPP /nologo /MD /W3 /GR /GX /O2 /I "./src/" /I "../../../" /I "../../../../../sdk/include/transactive" /I "../../../../../sdk/omniORB/omniORB_4.0.5/include" /I "../../../../../sdk/include/transactive/core/exceptions/src" /I "../../../../../sdk/include/transactive/bus/scada/datapointcorbadef/src" /I "../../../../../sdk/include/transactive/core/process_management/idl/src" /I "../../../../../sdk/include/transactive/core/exceptions/IDL/src" /I "../../../../../sdk/include/transactive/bus/maintenance_management/mmscorbadef/IDL/src" /I "../../../../" /I "../../../../../sdk/include/transactive/core/message/IDL/src" /I "../../../../../sdk/include/transactive/core/synchronisation/src" /I "../../../../../sdk/include/transactive/core/utilities/src" /I "../../../../../sdk/include/transactive/core/data_access_interface/user_settings/src" /I "../../../../../sdk/include/transactive/core/alarm/IDL/src" /I "../../../../../sdk/include/cots/boost/boost_1_31_0" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_AFXDLL" /D "_MBCS" /D "__WIN32__" /D "__x86__" /D "__NT__" /D __OSVERSION__=4 /FD /c
# SUBTRACT CPP /YX /Yc /Yu
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0xc09 /d "NDEBUG" /d "_AFXDLL"
# ADD RSC /l 0xc09 /d "NDEBUG" /d "_AFXDLL"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 /nologo /subsystem:windows /machine:I386
# ADD LINK32 htmlhelp.lib acemfc.lib TA_IRS_Core.lib TA_IRS_Bus.lib TA_Base_Bus.lib omniORB405_rt.lib omniDynamic405_rt.lib omnithread30_rt.lib COS405_rt.lib TA_Base_Core.lib oci.lib Rpcrt4.lib stracelib.lib /nologo /subsystem:windows /pdb:none /machine:I386 /out:"../../../../../bin/win32_nr/JobRequestManager.exe" /libpath:"../../../../../sdk/win32_nr" /libpath:"../../../../../build/win32_nr" /libpath:"../../../../../sdk/lib" /libpath:"../../../../../sdk/omniORB/omniORB_4.0.5/lib/x86_win32" /libpath:"../../../../../sdk/include/cots/WinStackTrace\build\Release"

!ELSEIF  "$(CFG)" == "app.maintenance_management.job_request_gui.JobRequestMgr - Win32 Debug"

# PROP BASE Use_MFC 6
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 6
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "../../../../../bin/win32_nd"
# PROP Intermediate_Dir "../../../../../build/win32_nd/transactive/app/maintenance_management/job_request_gui"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_AFXDLL" /Yu"stdafx.h" /FD /GZ /c
# ADD CPP /nologo /MDd /W3 /Gm /GR /GX /ZI /Od /I "./src/" /I "../../../" /I "../../../../../sdk/include/transactive" /I "../../../../../sdk/omniORB/omniORB_4.0.5/include" /I "../../../../../sdk/include/transactive/core/exceptions/src" /I "../../../../../sdk/include/transactive/bus/scada/datapointcorbadef/src" /I "../../../../../sdk/include/transactive/core/process_management/idl/src" /I "../../../../../sdk/include/transactive/core/exceptions/IDL/src" /I "../../../../../sdk/include/transactive/bus/maintenance_management/mmscorbadef/IDL/src" /I "../../../../" /I "../../../../../sdk/include/transactive/core/message/IDL/src" /I "../../../../../sdk/include/transactive/core/synchronisation/src" /I "../../../../../sdk/include/transactive/core/utilities/src" /I "../../../../../sdk/include/transactive/core/data_access_interface/user_settings/src" /I "../../../../../sdk/include/transactive/core/alarm/IDL/src" /I "../../../../../sdk/include/cots/boost/boost_1_31_0" /D "_DEBUG" /D "_WINDOWS" /D "_AFXDLL" /D "WIN32" /D "_MBCS_MBCS" /D "__WIN32__" /D "__x86__" /D "__NT__" /D __OSVERSION__=4 /FR /FD /GZ /c
# SUBTRACT CPP /YX /Yc /Yu
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0xc09 /d "_DEBUG" /d "_AFXDLL"
# ADD RSC /l 0xc09 /d "_DEBUG" /d "_AFXDLL"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept
# ADD LINK32 omnithread30_rtd.lib omniORB405_rtd.lib omniDynamic405_rtd.lib COS405_rtd.lib oci.lib htmlhelp.lib Rpcrt4.lib acemfcd.lib TA_IRS_Core_d.lib TA_Base_Core_d.lib TA_IRS_Bus_d.lib TA_Base_Bus_d.lib stracelib.lib /nologo /subsystem:windows /debug /debugtype:both /machine:I386 /out:"../../../../../bin/win32_nd/JobRequestManager.exe" /pdbtype:sept /libpath:"../../../../../sdk/omniORB/omniORB_4.0.5/lib/x86_win32" /libpath:"../../../../../sdk/win32_nd" /libpath:"../../../../../build/win32_nd" /libpath:"../../../../../sdk/lib" /libpath:"../../../../../sdk/include/cots/WinStackTrace\build\Debug"

!ENDIF 

# Begin Target

# Name "app.maintenance_management.job_request_gui.JobRequestMgr - Win32 Release"
# Name "app.maintenance_management.job_request_gui.JobRequestMgr - Win32 Debug"
# Begin Group "src"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat;h"
# Begin Source File

SOURCE=.\src\AuditMessageUtility.cpp
# End Source File
# Begin Source File

SOURCE=.\src\AuditMessageUtility.h
# End Source File
# Begin Source File

SOURCE=.\src\BadRunParamException.h
# End Source File
# Begin Source File

SOURCE=.\src\JobRequestMgr.cpp
# End Source File
# Begin Source File

SOURCE=.\src\JobRequestMgr.h
# End Source File
# Begin Source File

SOURCE=.\src\JobRequestMgr.rc
# End Source File
# Begin Source File

SOURCE=.\src\JobRequestMgrDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\src\JobRequestMgrDlg.h
# End Source File
# Begin Source File

SOURCE=.\src\JobRequestMgrGUI.cpp
# End Source File
# Begin Source File

SOURCE=.\src\JobRequestMgrGUI.h
# End Source File
# Begin Source File

SOURCE=.\src\Resource.h
# End Source File
# Begin Source File

SOURCE=.\src\StdAfx.cpp
# ADD CPP /Yc"stdafx.h"
# End Source File
# Begin Source File

SOURCE=.\src\StdAfx.h
# End Source File
# Begin Source File

SOURCE=.\src\VersionGen.cpp
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=.\res\JobRequestMgr.ico
# End Source File
# Begin Source File

SOURCE=.\res\JobRequestMgr.rc2
# End Source File
# End Group
# Begin Source File

SOURCE=.\src\ReadMe.txt
# End Source File
# End Target
# End Project
