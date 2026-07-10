# The source code in this file is the property of
# Ripple Systems and is not for redistribution
# in any form.
#
# Source:   $File: //depot/C830AR_TIP/C830AR/transactive/app/pa/PAAgent/Makefile $
# @author:  Ripple
# @version: $Revision: #5 $
#
# Last modification: $DateTime: 2024/12/10 11:18:50 $
# Last modified by:  $Author: limin.zhu $
#
# Make file for the PA Agent 
#

PROJ_ROOT_DIR			= ../../../../..
PROJ_CODE_DIR			= transactive/app/pa/PAAgent
PROJ_NAME				= PAAgent
PROJ_SOURCE				= $(filter-out $(PROJ_IGNORE_SOURCE), $(subst src/,,$(wildcard src/*.cpp)))
PROJ_IGNORE_SOURCE		= pch.cpp ForcedDataPointSynchroniser.cpp
PROJ_TYPE				= exe
PROJ_IDL_INC			= $(PROJ_ROOT_DIR)/code/transactive $(PROJ_ROOT_DIR)/sdk/idl/transactive
PROJ_INC_DIR			= ./src $(PROJ_ROOT_DIR)/code/transactive $(PROJ_ROOT_DIR)/sdk/include/transactive
PROJ_OUT_DIR			= $(PROJ_ROOT_DIR)/bin/$(OPERATING_SYSTEM)_$(STRING_FLAG)$(RELEASE_FLAG)
PROJ_LIB_DIRS			= $(PROJ_ROOT_DIR)/sdk/$(OPERATING_SYSTEM)_$(STRING_FLAG)$(RELEASE_FLAG) \
						$(PROJ_ROOT_DIR)/build/$(OPERATING_SYSTEM)_$(STRING_FLAG)$(RELEASE_FLAG)

#
# Order is important here.  Specify with most dependent libraries at the bottom
#
PROJ_LIBS				= \
						TA_PAAgentCorbaDef_4669 \
						TA_PACachedMaps \
						TA_AtsAgentCorbaDef \
						TA_TISAgentAccess_4669 \
						TA_TISManagerCorbaDef_4669 \
						TA_TISAgentCorbaDef_4669 \
						TA_TrainCorbaProxy \
						TA_TrainCommonLibrary \
						TA_TrainAgentObjectNames \
						TA_TrainAgentCorba \
						TA_AuthenticationCorbaDef \
						TA_DataPointAgentCorbaDef_Impl \
						TA_DataNodeAgentCorbaDef_Impl \
						TA_ScadaCommonLib \
						TA_ScadaProxyLibrary \
						TA_DataNodeLibrary \
						TA_DataPointLibrary \
						TA_ScadaProxyLibrary \
						TA_DataNodeLibrary \
						TA_ScadaCommonLib \
						TA_ScadaCommonCorbaDef \
						TA_DerivedDataPointDAI \
						TA_DataRecording \
						TA_DataRecordingDAI \
						TA_MmsCorbaDef \
						TA_GenericAgent \
						TA_EventTriggeredPlans \
						TA_PlanAgentLibrary \
						TA_PlanAgentCorbaDef \
						TA_PlanDAI \
						TA_PlanAgentDataCorbaDef \
						TA_PlanManagerCorbaDef \
						TA_Alarm \
						TA_AlarmAgentCorbaDef \
						TA_EntityStatusUpdateCorbaDef \
						TA_Message \
						TA_EventProcessing \
						TA_Event \
						TA_EventAgentCorbaDef \
						TA_MessageCorbaDef \
						TA_CorbaUtil \
						TA_EventScheduler \
						TA_Utilities \
						TA_Version \
						TA_Timers \
						TA_Thread \
						TA_Naming \
						TA_NamingCorbaDef \
						TA_Synchronisation \
						TA_ManagedProcess \
						TA_ProcessManagementCorbaDef \
						TA_ConfigurationUpdates \
						TA_Utilities \
						TA_MathEvaluation \
						TA_DerivedDataPointDAI \
						TA_RightsLibrary \
						TA_AuthenticationLibrary \
						TA_ProcessManagementCorbaDef \
						TA_DutyAgentCorbaDef \
						TA_SecurityUtilities \
						TA_AuthenticationCorbaDef \
						TA_RightsAgentCorbaDef \
						TA_PADAI_4669 \
						TA_EntityAccess \
						TA_DAI \
						TA_AlarmRuleDAI \
						TA_DataAccess \
						TA_Exceptions \
						TA_CommonExceptionsCorbaDef \
						TA_CorbaDef \
						TA_Sockets \
						TA_Uuid \
						TA_ocl \
						clntsh \
						mysqlcppconn \
						mysqlclient \
						ACE 
 
DEPENDENT_LIBRARIES		= \
						$(PROJ_ROOT_DIR)/code/transactive/bus/pa/cachedmaps \
						$(PROJ_ROOT_DIR)/code/transactive/bus/signs/tis_agent_access \
						$(PROJ_ROOT_DIR)/code/transactive/bus/event \
						$(PROJ_ROOT_DIR)/code/transactive/bus/ats/ats_agent/IDL \
						$(PROJ_ROOT_DIR)/code/transactive/core/data_access_interface/tis_agent \
 
include $(PROJ_ROOT_DIR)/code/tools/build/CommonMakefile.mak

install:
	#Do Nothing
