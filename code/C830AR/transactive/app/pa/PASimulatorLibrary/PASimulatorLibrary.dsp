# Microsoft Developer Studio Project File - Name="PASimulatorLibrary" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=PASimulatorLibrary - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "PASimulatorLibrary.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "PASimulatorLibrary.mak" CFG="PASimulatorLibrary - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "PASimulatorLibrary - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "PASimulatorLibrary - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "PASimulatorLibrary - Win32 Release"

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
LINK32=link.exe -link
MTL=midl.exe
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /YX /FD /c
# ADD CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /YX /FD /c
# ADD BASE RSC /l 0xc09 /d "NDEBUG"
# ADD RSC /l 0xc09 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /ignore:4006

!ELSEIF  "$(CFG)" == "PASimulatorLibrary - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Target_Dir ""
LINK32=link.exe -link
MTL=midl.exe
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /YX /FD /GZ /c
# ADD CPP /nologo /MDd /W3 /Gm /GR /GX /Zi /Od /I "..\..\..\\" /I "..\..\..\cots\omniORB\omniORB_4.0.3\Winnt40\omni\include" /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /D "__WIN32__" /D "__x86__" /D "__NT__" /D __OSVERSION__=4 /FR /FD /GZ /c
# SUBTRACT CPP /YX
# ADD BASE RSC /l 0xc09 /d "_DEBUG"
# ADD RSC /l 0xc09 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /ignore:4006

!ENDIF 

# Begin Target

# Name "PASimulatorLibrary - Win32 Release"
# Name "PASimulatorLibrary - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\src\PACommandMonitor.cpp
# End Source File
# Begin Source File

SOURCE=.\src\PACommsObserver.cpp
# End Source File
# Begin Source File

SOURCE=.\src\PASimPackets.cpp
# End Source File
# Begin Source File

SOURCE=.\src\PASimulator.cpp
# End Source File
# Begin Source File

SOURCE=.\src\PASimulatorCommandProcessor.cpp
# End Source File
# Begin Source File

SOURCE=.\src\PASimUtility.cpp
# End Source File
# Begin Source File

SOURCE=.\src\PATableRequestProcessor.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\src\PACommandMonitor.h
# End Source File
# Begin Source File

SOURCE=.\src\PACommsObserver.h
# End Source File
# Begin Source File

SOURCE=.\src\PASimPackets.h
# End Source File
# Begin Source File

SOURCE=.\src\PASimTypes.h
# End Source File
# Begin Source File

SOURCE=.\src\PASimulator.h
# End Source File
# Begin Source File

SOURCE=.\src\PASimulatorCommandProcessor.h
# End Source File
# Begin Source File

SOURCE=.\src\PASimUtility.h
# End Source File
# Begin Source File

SOURCE=.\src\PATableRequestProcessor.h
# End Source File
# End Group
# Begin Group "Notes"

# PROP Default_Filter ".txt"
# Begin Source File

SOURCE=.\src\ReadMe_AddingCommands.txt
# End Source File
# Begin Source File

SOURCE=.\src\ReadMe_AddingTables.txt
# End Source File
# End Group
# Begin Group "Tables"

# PROP Default_Filter "cpp;h"
# Begin Group "Source"

# PROP Default_Filter ".cpp"
# Begin Source File

SOURCE=.\src\PASimTable.cpp
# End Source File
# Begin Source File

SOURCE=.\src\PATable100.cpp
# End Source File
# Begin Source File

SOURCE=.\src\PATable1200.cpp
# End Source File
# Begin Source File

SOURCE=.\src\PATable1201.cpp
# End Source File
# Begin Source File

SOURCE=.\src\PATable1204.cpp
# End Source File
# Begin Source File

SOURCE=.\src\PATable1301.cpp
# End Source File
# Begin Source File

SOURCE=.\src\PATable1303.cpp
# End Source File
# Begin Source File

SOURCE=.\src\PATable1610.cpp
# End Source File
# Begin Source File

SOURCE=.\src\PATable200.cpp
# End Source File
# Begin Source File

SOURCE=.\src\PATable201.cpp
# End Source File
# Begin Source File

SOURCE=.\src\PATable202.cpp
# End Source File
# Begin Source File

SOURCE=.\src\PATable204.cpp
# End Source File
# Begin Source File

SOURCE=.\src\PATable301.cpp
# End Source File
# Begin Source File

SOURCE=.\src\PATable302.cpp
# End Source File
# Begin Source File

SOURCE=.\src\PATable303.cpp
# End Source File
# Begin Source File

SOURCE=.\src\PATable350.cpp
# End Source File
# Begin Source File

SOURCE=.\src\PATable550.cpp
# End Source File
# Begin Source File

SOURCE=.\src\PATable560.cpp
# End Source File
# Begin Source File

SOURCE=.\src\PATable570.cpp
# End Source File
# Begin Source File

SOURCE=.\src\PATable590.cpp
# End Source File
# Begin Source File

SOURCE=.\src\PATable603.cpp
# End Source File
# Begin Source File

SOURCE=.\src\PATable604.cpp
# End Source File
# Begin Source File

SOURCE=.\src\PATable610.cpp
# End Source File
# Begin Source File

SOURCE=.\src\PATable630.cpp
# End Source File
# Begin Source File

SOURCE=.\src\PATableInvalid.cpp
# End Source File
# End Group
# Begin Group "Include"

# PROP Default_Filter ".h"
# Begin Source File

SOURCE=.\src\PASimTable.h
# End Source File
# Begin Source File

SOURCE=.\src\PATable100.h
# End Source File
# Begin Source File

SOURCE=.\src\PATable1200.h
# End Source File
# Begin Source File

SOURCE=.\src\PATable1201.h
# End Source File
# Begin Source File

SOURCE=.\src\PATable1204.h
# End Source File
# Begin Source File

SOURCE=.\src\PATable1301.h
# End Source File
# Begin Source File

SOURCE=.\src\PATable1303.h
# End Source File
# Begin Source File

SOURCE=.\src\PATable1610.h
# End Source File
# Begin Source File

SOURCE=.\src\PATable200.h
# End Source File
# Begin Source File

SOURCE=.\src\PATable201.h
# End Source File
# Begin Source File

SOURCE=.\src\PATable202.h
# End Source File
# Begin Source File

SOURCE=.\src\PATable204.h
# End Source File
# Begin Source File

SOURCE=.\src\PATable301.h
# End Source File
# Begin Source File

SOURCE=.\src\PATable302.h
# End Source File
# Begin Source File

SOURCE=.\src\PATable303.h
# End Source File
# Begin Source File

SOURCE=.\src\PATable350.h
# End Source File
# Begin Source File

SOURCE=.\src\PATable550.h
# End Source File
# Begin Source File

SOURCE=.\src\PATable560.h
# End Source File
# Begin Source File

SOURCE=.\src\PATable570.h
# End Source File
# Begin Source File

SOURCE=.\src\PATable590.h
# End Source File
# Begin Source File

SOURCE=.\src\PATable603.h
# End Source File
# Begin Source File

SOURCE=.\src\PATable604.h
# End Source File
# Begin Source File

SOURCE=.\src\PATable610.h
# End Source File
# Begin Source File

SOURCE=.\src\PATable630.h
# End Source File
# Begin Source File

SOURCE=.\src\PATableInvalid.h
# End Source File
# End Group
# End Group
# Begin Group "Commands"

# PROP Default_Filter "cpp;h"
# End Group
# End Target
# End Project
