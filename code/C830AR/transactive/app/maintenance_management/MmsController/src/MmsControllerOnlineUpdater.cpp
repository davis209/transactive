/**
  * The source code in this file is the property of
  * Ripple Systems and is not for redistribution
  * in any form.
  *
  * Source:   $File: //depot/4669_T01271350/4669/transactive/app/maintenance_management/MmsController/src/MmsControllerOnlineUpdater.cpp $
  * @author:  Ripple
  * @version: $Revision: #1 $
  *
  * Last modification: $DateTime: 2008/11/28 16:26:01 $
  * Last modified by:  $Author: builder $
  *
  */
// MmsControllerOnlineUpdater.cpp: implementation of the MmsControllerOnlineUpdater class.
//
//////////////////////////////////////////////////////////////////////

#include "app/maintenance_management/MmsController/src/stdafx.h"
#include "app/maintenance_management/MmsController/src/mmscontroller.h"
#include "app/maintenance_management/MmsController/src/MmsControllerModel.h"
#include "app/maintenance_management/MmsController/src/MmsControllerOnlineUpdater.h"
#include "core/message/src/MessagePublicationManager.h"
#include "core/message/src/CommsMessageSender.h"
#include "core/message/types/MMSComms_MessageTypes.h"
 

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

namespace TA_IRS_App
{	
	MmsControllerOnlineUpdater::MmsControllerOnlineUpdater() 
		: m_commsMsgSender(TA_Base_Core::MessagePublicationManager::getInstance().getCommsMessageSender(TA_Base_Core::MMSComms::Context))
	{
		FUNCTION_ENTRY("MmsControllerOnlineUpdater");
		FUNCTION_EXIT;
	}
	
	MmsControllerOnlineUpdater::~MmsControllerOnlineUpdater()
	{
		FUNCTION_ENTRY("~MmsControllerOnlineUpdater");
		FUNCTION_EXIT;
	}

	void MmsControllerOnlineUpdater::registerCallback(MmsControllerModel*  mmsModelCallBack)
	{
		FUNCTION_ENTRY("registerCallback - MmsControllerModel");
		subscribeToEntityMMSInhibitUpdateMsg();
		m_callback = mmsModelCallBack;
		FUNCTION_EXIT;
	}

	void MmsControllerOnlineUpdater::deregister()
	{
		FUNCTION_ENTRY("deregister");
		unsubscribeToEntityMMSInhibitUpdateMsg();
		m_callback = NULL;
		FUNCTION_EXIT;
	}

	void MmsControllerOnlineUpdater::receiveSpecialisedMessage(const TA_Base_Core::CommsMessageCorbaDef& message)
	{
		FUNCTION_ENTRY("receiveSpecialisedMessage");

		if (m_callback != NULL)
		{			
			std::string messageTypeKey(message.messageTypeKey);
			EInhibitResult result;
			
			TA_Base_Core::EntityStatusUpdateCorbaDef* entityStatusCorbaData = NULL;
			try
			{	
				if ( (message.messageState >>= entityStatusCorbaData) == 0 )
				{
					LOG(SourceInfo, TA_Base_Core::DebugUtil::GenericLog,  TA_Base_Core::DebugUtil::DebugError, 
						"Failed to interpret message");
					return;
				}
				
				if (messageTypeKey == TA_Base_Core::MMSComms::NonPhysicalEntityMMSAlarmInhibitApplied.getTypeKey())
				{
					result = SYSTEM_ALARM_INHIBIT_APPLIED;
				}
				else if (messageTypeKey == TA_Base_Core::MMSComms::NonPhysicalEntityMMSAlarmInhibitRemoved.getTypeKey())
				{
					result = SYSTEM_ALARM_INHIBIT_REMOVED;
				}
				else
				{
					result = SYSTEM_ALARM_INHIBIT_FAILED;
					LOG_GENERIC( SourceInfo,
						TA_Base_Core::DebugUtil::DebugDebug,
						"receive unknown comms message, ignore");
					return;
				}
			}
			catch (...)
			{
				result = SYSTEM_ALARM_INHIBIT_FAILED;
				LOG(SourceInfo, TA_Base_Core::DebugUtil::GenericLog,  TA_Base_Core::DebugUtil::DebugWarn, 
					"Failed to process entity status data from MMS Controller");
			}
			
			// convert  CORBA string to EntityStatusData
			if ((result != SYSTEM_ALARM_INHIBIT_FAILED) && (NULL != entityStatusCorbaData))
			{
					
				// process update to the non physical entities with MMS inhibit

				TA_Base_Core::EntityStatusData statusData;
				statusData.entityKey = entityStatusCorbaData->entityKey;
				statusData.lastSessionId = std::string(entityStatusCorbaData->lastSessionId);
				statusData.lastModifyDate = (time_t) entityStatusCorbaData->lastModifyDate;
				statusData.entityStatusType = (TA_Base_Core::EEntityStatusType) entityStatusCorbaData->entityStatusType;
				statusData.entityStatusDataValue = std::string(entityStatusCorbaData->entityStatusDataValue);
				statusData.entityStatusDataType = (TA_Base_Core::EStatusDataType) entityStatusCorbaData->entityStatusDataType;
				m_callback->processNonPhysicalEntityMMSUpdate(statusData.entityKey, statusData, result);
			
			}	
			
		}
		FUNCTION_EXIT;
	}

	void MmsControllerOnlineUpdater::sendMmsStateUpdateMessage(const TA_Base_Core::EntityStatusData& statusData, EInhibitResult result)
	{
		FUNCTION_ENTRY("sendMmsStateUpdateMessage");

		TA_Base_Core::EntityStatusUpdateCorbaDef entityStatusCorbaDef;
		
		entityStatusCorbaDef.entityKey = statusData.entityKey;
		entityStatusCorbaDef.lastSessionId = CORBA::string_dup(statusData.lastSessionId.c_str());
		entityStatusCorbaDef.lastModifyDate = (unsigned long) statusData.lastModifyDate;
		entityStatusCorbaDef.entityStatusDataValue = CORBA::string_dup (statusData.entityStatusDataValue.c_str());
		entityStatusCorbaDef.entityStatusType = statusData.entityStatusType;
		entityStatusCorbaDef.entityStatusDataType = statusData.entityStatusDataType;    

		CORBA::Any commsData;
		commsData <<= entityStatusCorbaDef;

		if (result == SYSTEM_ALARM_INHIBIT_APPLIED)
		{			
			m_commsMsgSender->sendBroadcastCommsMessage( TA_Base_Core::MMSComms::NonPhysicalEntityMMSAlarmInhibitApplied,
				commsData,
				NULL );
		}
		else if (result == SYSTEM_ALARM_INHIBIT_REMOVED)
		{
			m_commsMsgSender->sendBroadcastCommsMessage( TA_Base_Core::MMSComms::NonPhysicalEntityMMSAlarmInhibitRemoved,
				commsData,
				NULL );
		}

		FUNCTION_EXIT;
	} 

	TA_Base_Core::EntityStatusData MmsControllerOnlineUpdater::convertEntityStatusCorbaToEntityStatusData(const TA_Base_Core::EntityStatusUpdateCorbaDef& entityStatusCorbaDef)
	{
		FUNCTION_ENTRY("convertEntityStatusCorbaToEntityStatusData");

		TA_Base_Core::EntityStatusData statusData;

		statusData.entityKey = entityStatusCorbaDef.entityKey;
		statusData.lastSessionId = std::string(entityStatusCorbaDef.lastSessionId);
		statusData.lastModifyDate = entityStatusCorbaDef.lastModifyDate;
		statusData.entityStatusType = (TA_Base_Core::EEntityStatusType) entityStatusCorbaDef.entityStatusType;
		statusData.entityStatusDataValue = std::string(entityStatusCorbaDef.entityStatusDataValue);
		statusData.entityStatusDataType = (TA_Base_Core::EStatusDataType) entityStatusCorbaDef.entityStatusDataType;

		FUNCTION_EXIT;
		return statusData;
	}
	
	void MmsControllerOnlineUpdater::subscribeToEntityMMSInhibitUpdateMsg()
    {
		FUNCTION_ENTRY("subscribeToEntityMMSInhibitUpdateMsg");
        TA_Base_Core::MessageSubscriptionManager &manager = TA_Base_Core::MessageSubscriptionManager::getInstance();
        manager.subscribeToBroadcastCommsMessage(TA_Base_Core::MMSComms::Context, this);
			FUNCTION_EXIT;
    }

	void MmsControllerOnlineUpdater::unsubscribeToEntityMMSInhibitUpdateMsg()
    {
		FUNCTION_ENTRY("unsubscribeToEntityMMSInhibitUpdateMsg");
        TA_Base_Core::MessageSubscriptionManager &manager = TA_Base_Core::MessageSubscriptionManager::getInstance();
        manager.unsubscribeToMessages(this);
		FUNCTION_EXIT;
    }
}
