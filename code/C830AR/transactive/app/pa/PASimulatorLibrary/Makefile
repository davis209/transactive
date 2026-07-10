# The source code in this file is the property of
# Ripple Systems and is not for redistribution
# in any form.
#
# Source:   $File: //depot/4669_T01271350/4669/transactive/app/pa/PASimulatorLibrary/Makefile $
# @author:  Ripple
# @version: $Revision: #1 $
#
# Last modification: $DateTime: 2008/11/28 16:26:01 $
# Last modified by:  $Author: builder $
#
# Make file for the PASimulatorLibrary 
#

PROJ_ROOT_DIR = ../../..
PROJ_NAME     = PASimulatorLibrary
PROJ_SOURCE   = PACommandMonitor.cpp \
		PACommsObserver.cpp \
		PASimPackets.cpp \
		PASimTable.cpp \
		PASimUtility.cpp \
		PASimulator.cpp \
		PASimulatorCommandProcessor.cpp \
		PATable100.cpp\
		PATable1200.cpp\
		PATable1201.cpp\
		PATable1204.cpp\
		PATable1301.cpp\
		PATable1303.cpp\
		PATable1610.cpp\
		PATable200.cpp\
		PATable201.cpp\
		PATable202.cpp\
		PATable204.cpp\
		PATable301.cpp\
		PATable302.cpp\
		PATable303.cpp\
		PATable350.cpp\
		PATable550.cpp\
		PATable560.cpp\
		PATable570.cpp\
		PATable590.cpp\
		PATable603.cpp\
		PATable604.cpp\
		PATable610.cpp\
		PATable630.cpp\
		PATableInvalid.cpp \
		PATableRequestProcessor.cpp


PROJ_TYPE     = lib
PROJ_OBJDIR   = ./obj
PROJ_INCDIR   = ./src \
		$(PROJ_ROOT_DIR)

PROJ_LIBDIRS  =	$(PROJ_ROOT_DIR)/core/versioning/obj \
				$(PROJ_ROOT_DIR)/core/synchronisation/obj \
				$(PROJ_ROOT_DIR)/core/utilities/obj \
				$(PROJ_ROOT_DIR)/core/threads/obj \
				$(PROJ_ROOT_DIR)/core/logging/obj \
				$(PROJ_ROOT_DIR)/core/sockets/obj \
				$(PROJ_ROOT_DIR)/bus/generic_simulator

PROJ_LIBS =	TA_GenericSimulator \
		TA_Utilities \
		TA_Version \
		TA_Thread \
		TA_Sockets \
		TA_Synchronisation \
		TA_Exceptions \
		TA_Utilities \
		# clntsh

PROJ_IDL_INC  = $(PROJ_ROOT_DIR)

PROJ_INCDIRS  = \
		$(PROJ_ROOT_DIR)/core/logging/src \
		$(PROJ_ROOT_DIR)/core/utilities/src \
		$(PROJ_ROOT_DIR)/core/threads/src \
		$(PROJ_ROOT_DIR)/core/sockets/src \
		$(PROJ_ROOT_DIR)/core/synchronisation/src \
		$(PROJ_ROOT_DIR)/core/exceptions/src \
		$(PROJ_ROOT_DIR)/bus/generic_simulator


DEPENDENT_LIBRARIES =	$(PROJ_ROOT_DIR)/core/exceptions \
						$(PROJ_ROOT_DIR)/core/versioning \
						$(PROJ_ROOT_DIR)/core/synchronisation \
						$(PROJ_ROOT_DIR)/core/threads \
						$(PROJ_ROOT_DIR)/core/utilities \
						$(PROJ_ROOT_DIR)/core/logging \
						$(PROJ_ROOT_DIR)/core/sockets \
						$(PROJ_ROOT_DIR)/bus/generic_simulator
 
include $(PROJ_ROOT_DIR)/tools/build/CommonMakefile.mak

install:
	#Do Nothing
