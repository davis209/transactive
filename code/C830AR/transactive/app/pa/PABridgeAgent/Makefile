# The source code in this file is the property of
# Ripple Systems and is not for redistribution
# in any form.
#
# Make file for the PA Bridge Agent
#

PROJ_ROOT_DIR   = ../../../../..
PROJ_CODE_DIR   = transactive/app/pa/PABridgeAgent
PROJ_NAME       = PABridgeAgent

PROJ_SOURCE     = PaBridgeAgentMain.cpp \
                PaBridgeAgent.cpp \
                PaMessageSubscriber.cpp \
                PaRestController.cpp \
                RestServer.cpp \
                KafkaProducer.cpp \
                PaJsonConverter.cpp \
                JsonUtil.cpp

PROJ_TYPE       = exe
PROJ_IDL_INC    = $(PROJ_ROOT_DIR)/code/transactive $(PROJ_ROOT_DIR)/sdk/idl/transactive
PROJ_INC_DIR    = ./src \
                $(PROJ_ROOT_DIR)/code/transactive \
                $(PROJ_ROOT_DIR)/sdk/include/transactive \
                $(PROJ_ROOT_DIR)/C830AR/cots/cpp-httplib \
                $(PROJ_ROOT_DIR)/C830AR/cots/nlohmann-json \
                $(PROJ_ROOT_DIR)/C830AR/cots/librdkafka/librdkafka-2.8.0/src
PROJ_OUT_DIR    = $(PROJ_ROOT_DIR)/bin/$(OPERATING_SYSTEM)_$(STRING_FLAG)$(RELEASE_FLAG)
PROJ_LIB_DIRS   = $(PROJ_ROOT_DIR)/sdk/$(OPERATING_SYSTEM)_$(STRING_FLAG)$(RELEASE_FLAG) \
                $(PROJ_ROOT_DIR)/build/$(OPERATING_SYSTEM)_$(STRING_FLAG)$(RELEASE_FLAG) \
                $(PROJ_ROOT_DIR)/C830AR/cots/librdkafka/librdkafka-2.8.0/src

PROJ_LIBS       = \
                rdkafka \
                TA_GenericAgent \
                TA_DataPointAgentCorbaDef_Impl \
                TA_DataNodeAgentCorbaDef_Impl \
                TA_ScadaCommonCorbaDef \
                TA_ScadaCommonLib \
                TA_DataPointLibrary \
                TA_DataNodeLibrary \
                TA_PAAgentCorbaDef_4669 \
                TA_AuthenticationLibrary \
                TA_AuthenticationCorbaDef \
                TA_Message \
                TA_MessageCorbaDef \
                TA_CorbaUtil \
                TA_Utilities \
                TA_Version \
                TA_Thread \
                TA_Naming \
                TA_NamingCorbaDef \
                TA_Synchronisation \
                TA_ManagedProcess \
                TA_ProcessManagementCorbaDef \
                TA_ConfigurationUpdates \
                TA_EntityAccess \
                TA_DAI \
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

PROJ_CFLAGS     = -DTA_ASSERT_LOG=1 -DUSE_LIBRDKAFKA -std=c++11

DEPENDENT_LIBRARIES =

include $(PROJ_ROOT_DIR)/code/tools/build/CommonMakefile.mak

install:
	#Do Nothing
