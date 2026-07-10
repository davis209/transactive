/**
  * The source code in this file is the property of 
  * Ripple Systems and is not for redistribution
  * in any form.
  *
  * Source:   $File: //depot/4669_T01271350/4669/transactive/app/maintenance_management/job_request_gui/src/AuditMessageUtility.cpp $
  * @author:  
  * @version: $Revision: #1 $
  *
  * Last modification: $DateTime: 2008/11/28 16:26:01 $
  * Last modified by:  $Author: builder $
  * 
  * Impliimentation of the AuditMessageUtility class
  */
#include "app/maintenance_management/job_request_gui/src/AuditMessageUtility.h"

#include "core/utilities/src/RunParams.h"

#include "core/data_access_interface/src/ResourceAccessFactory.h"
#include "core/data_access_interface/src/IResource.h"
#include "core/data_access_interface/src/AlarmAccessFactory.h"
#include "core/data_access_interface/src/IAlarmData.h"
#include "core/data_access_interface/entity_access/src/EntityAccessFactory.h"
#include "core/data_access_interface/entity_access/src/IEntityData.h"

#include "bus/application_types/src/apptypes.h"

using namespace TA_Base_Core;

namespace TA_IRS_App
{
	//init static instance ptr of this class
	AuditMessageUtility* AuditMessageUtility::m_instance = 0;

	AuditMessageUtility* AuditMessageUtility::getInstance()
	{
		FUNCTION_ENTRY("getInstance");

		if(0 == m_instance)
		{
			m_instance = new AuditMessageUtility();
		}

		FUNCTION_EXIT;
		return m_instance;
	}

	void AuditMessageUtility::setJobRequestGuiKey(unsigned int guiKey)
	{
		m_jobRequestGuiKey = guiKey;
	}

	AuditMessageUtility::AuditMessageUtility()
		: m_jobRequestGuiKey(0), m_auditMsgSender(0)
	{
		FUNCTION_ENTRY("AuditMessageUtility");

		try
		{
			m_auditMsgSender = 
				MessagePublicationManager::getInstance().getAuditMessageSender(MmsAudit::Context);
			if(0 == m_auditMsgSender) throw "crap";
		}
		catch(...)
		{
			LOG_GENERIC(SourceInfo, TA_Base_Core::DebugUtil::DebugError,
			"MessagePublicationManager::getInstance().getAuditMessageSender() threw or return 0.");

			m_auditMsgSender = 0;
		}

		FUNCTION_EXIT;
	}

	AuditMessageUtility::~AuditMessageUtility()
	{
		FUNCTION_ENTRY("~AuditMessageUtility");

		if(0 != m_instance)
		{
			delete m_instance;
		}

		FUNCTION_EXIT;
	}

	void AuditMessageUtility::submitJobRequestAuditMessage(const MessageType& type,
		const DescriptionParameters& descParams, const TA_Base_Bus::JobRequestDetailCorbaDef* jobRequest)
	{
		FUNCTION_ENTRY("submitJobRequestAuditMessage");		

		try
		{	

			//asset name 
			std::string assetName = "";
			unsigned long entityKey = 0;

			if(0 != jobRequest)
			{
				entityKey = jobRequest->dataNodeKey;
				assetName = jobRequest->assetId; //TD15090
			}
			if(0 == entityKey) //if we did not get an entity key or 0 is specified
			{
				LOG_GENERIC(SourceInfo, TA_Base_Core::DebugUtil::DebugWarn,
				"No equipment entity key is avaiable to log against. Will use the JRM key if avaiable");

				//use the key of the JRM
				entityKey = m_jobRequestGuiKey;
			}

			if(0 == entityKey)
			{
				LOG_GENERIC(SourceInfo, TA_Base_Core::DebugUtil::DebugError,
				"No entity key is avaiable to log against. No event will be logged");
				FUNCTION_EXIT;
				return;
			}

			std::string sessionId = RunParams::getInstance().get(RPARAM_SESSIONID);


		    //no details
			const std::string details = "";
			
			//alarmID is N/A
			//todo might be worth associating with alarm
			const std::string alarmID = "";

			//incidentID is N/A
			const std::string incidentID = "";

			//parentEventID is N/A
			const std::string parentEventID = "";

			
			
			if(0 == m_auditMsgSender)
			{
				try
				{
					m_auditMsgSender = 
						MessagePublicationManager::getInstance().getAuditMessageSender(MmsAudit::Context);
					if(0 == m_auditMsgSender) throw "crap";
				}
				catch(...)
				{
					LOG_GENERIC(SourceInfo, TA_Base_Core::DebugUtil::DebugError,
					"MessagePublicationManager::getInstance().getAuditMessageSender() threw or return 0. No event will be logged");

					m_auditMsgSender = 0;
					FUNCTION_EXIT;
				}
			}

			//TD15090 - send the asset name in the audit message
			//m_auditMsgSender->sendAuditMessage(
			//   type, //const MessageType& messageType,
			//   entityKey, //unsigned long entityKey,
			//   descParams, //const DescriptionParameters& description,
			//   details, //const std::string& details,
			//   sessionId, //const std::string& sessionID,
			//   alarmID, //const std::string& alarmID,
			//   incidentID, //const std::string& incidentID,
			//   parentEventID //const std::string& parentEventID,
			//    //const FilterData* filterableData = NULL,
            //    //const time_t& timeStamp = 0
			//);

			m_auditMsgSender->sendAuditMessageWithAsset(
				type, //const MessageType& messageType,
				entityKey, //unsigned long entityKey,
				descParams, //const DescriptionParameters& description,
				details, //const std::string& details,
				sessionId, //const std::string& sessionID,
				alarmID, //const std::string& alarmID,
				incidentID, //const std::string& incidentID,
				parentEventID, //const std::string& parentEventID,
				assetName
				//const FilterData* filterableData = NULL,
                //const time_t& timeStamp = 0
				);

			//++lizettejl

		} 	
		catch(...)
		{
			// hey, that's the method contract - "silent failure"
			LOG_GENERIC(SourceInfo, TA_Base_Core::DebugUtil::DebugWarn,
				"sendAuditMessage() threw");

		}

		FUNCTION_EXIT;

	}

	void AuditMessageUtility::submitJobRequestFailureMessage(
		const std::string& eventDesc, const TA_Base_Bus::JobRequestDetailCorbaDef* jobRequest)
	{
		FUNCTION_ENTRY("submitJobRequestFailureMessage");
		
		const MessageType& type = MmsAudit::JobRequestFailure;


		//populate the one and only DescriptionParameter
		DescriptionParameters descParams;
		NameValuePair pair("failureDescription", eventDesc);
		descParams.push_back(&pair);	
			
		submitJobRequestAuditMessage(type, descParams, jobRequest);

		FUNCTION_EXIT;

	}
	

	void AuditMessageUtility::submitJobRequestDenialMessage(
		const std::string& eventDesc, const TA_Base_Bus::JobRequestDetailCorbaDef* jobRequest)
	{
		FUNCTION_ENTRY("submitJobRequestDenialMessage");


		const MessageType& type = MmsAudit::JobRequestDenial;


		//populate the one and only DescriptionParameter
		DescriptionParameters descParams;
		NameValuePair pair("denialDescription", eventDesc);
		descParams.push_back(&pair);	
			
		submitJobRequestAuditMessage(type, descParams, jobRequest);

		FUNCTION_EXIT;
	}


	void AuditMessageUtility::submitJobRequestSubmittedMessage(const TA_Base_Bus::JobRequestDetailCorbaDef* jobRequest)
	{
		FUNCTION_ENTRY("submitJobRequestSubmittedMessage");
	
		const MessageType& type = MmsAudit::JobRequestSubmitted;

		std::string str;
		if(0 != jobRequest)
		{
			std::string alarmId = jobRequest->alarmUID.in();
			
			if("NOT_FROM_ALARM" == alarmId)
			{
				str += " from schematic.";
				
			}
			else
			{
				//TD15090
				try
				{				
					IAlarmData* alarmdata = AlarmAccessFactory::getInstance().getAlarm(alarmId);
					unsigned long ulEntityKey = alarmdata->getEntityKey();                
					IEntityData *entitydata = EntityAccessFactory::getInstance().getEntity(ulEntityKey);					
 					str += "from " + entitydata->getName();
 					delete entitydata;
					delete alarmdata;
					
				}
				catch (...)
				{
					LOG_GENERIC( SourceInfo, TA_Base_Core::DebugUtil::DebugInfo,
						"Can not get the alarm detail from %s.", alarmId);
					// no alarm details was found, just fill in this value in case
					// that happens
					str += " from alarm.";
				}			

			}
		}


		//populate the one and only DescriptionParameter
		DescriptionParameters descParams;
		NameValuePair pair("submissionDescription", str);
		descParams.push_back(&pair);

		submitJobRequestAuditMessage(type, descParams, jobRequest);

		FUNCTION_EXIT;
	}

} // end namespace TA_IRS_App