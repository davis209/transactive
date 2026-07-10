# The source code in this file is the property of
# Ripple Systems and is not for redistribution
# in any form.
#
# Source:   $File: //depot/4669_T01271350/4669/transactive/app/pa/PASimulator/Makefile $
# @author:  Ripple
# @version: $Revision: #1 $
#
# Last modification: $DateTime: 2008/11/28 16:26:01 $
# Last modified by:  $Author: builder $
#
# Make file for the PA Simulator 
#

PROJ_ROOT_DIR = ../../..
PROJ_NAME     = PASimulator
PROJ_SOURCE   =	main.cpp \
		VersionGen.cpp

PROJ_TYPE     = exe
PROJ_OBJDIR   = ./obj
PROJ_INCDIR   = ./src \
		$(PROJ_ROOT_DIR)

PROJ_LIBDIRS  =	$(PROJ_ROOT_DIR)/core/versioning/obj \
			$(PROJ_ROOT_DIR)/core/synchronisation/obj \
			$(PROJ_ROOT_DIR)/core/exceptions/obj \
			$(PROJ_ROOT_DIR)/core/utilities/obj \
			$(PROJ_ROOT_DIR)/core/threads/obj \
			$(PROJ_ROOT_DIR)/core/logging/obj \
			$(PROJ_ROOT_DIR)/core/sockets/obj \
			$(PROJ_ROOT_DIR)/app/pa/PASimulatorLibrary/obj \
			$(PROJ_ROOT_DIR)/bus/generic_simulator/obj

PROJ_LIBS = PASimulatorLibrary \
			TA_GenericSimulator \
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
			$(PROJ_ROOT_DIR)/cots/omniORB/omniORB_4.0.3/Solaris8/omni/include \
			$(PROJ_ROOT_DIR)/core/utilities/src \
			$(PROJ_ROOT_DIR)/core/threads/src \
			$(PROJ_ROOT_DIR)/core/sockets/src \
			$(PROJ_ROOT_DIR)/core/synchronisation/src \
			$(PROJ_ROOT_DIR)/core/exceptions/src \
			$(PROJ_ROOT_DIR)/app/pa/PASimulatorLibrary/src \
			$(PROJ_ROOT_DIR)/bus/generic_simulator/src


DEPENDENT_LIBRARIES =	$(PROJ_ROOT_DIR)/core/exceptions \
			$(PROJ_ROOT_DIR)/core/versioning \
			$(PROJ_ROOT_DIR)/core/synchronisation \
			$(PROJ_ROOT_DIR)/core/threads \
			$(PROJ_ROOT_DIR)/core/utilities \
			$(PROJ_ROOT_DIR)/core/logging \
			$(PROJ_ROOT_DIR)/core/sockets \
			$(PROJ_ROOT_DIR)/app/pa/PASimulatorLibrary \
			$(PROJ_ROOT_DIR)/bus/generic_simulator
 
include $(PROJ_ROOT_DIR)/tools/build/CommonMakefile.mak

install:
	#Do Nothing
