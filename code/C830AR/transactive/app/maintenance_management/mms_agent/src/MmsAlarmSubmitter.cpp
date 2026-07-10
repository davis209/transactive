/**
  * The source code in this file is the property of
  * Ripple Systems and is not for redistribution in any form.
  * 
  * Source:    $File: //depot/4669_T01271350/4669/transactive/app/maintenance_management/mms_agent/src/MmsAlarmSubmitter.cpp $
  * @author:   Chris DeWolfe
  * @version:  $Revision: #1 $
  * @see:      9999-Q01-02-0016-GenericAgent_Usage_Guidelines.doc
  *
  * Last modification: $DateTime: 2008/11/28 16:26:01 $
  * Last mofified by:  $Author: builder $
  *
  * MmsAlarmSubmitter class implimentation. See header file for class descrption
  */

#if defined(_MSC_VER)
#pragma warning(disable:4786)
#endif // defined (_MSC_VER)



#include "app/maintenance_management/mms_agent/src/MmsAlarmSubmitter.h"

#include "core/data_access_interface/entity_access/src/MmsAlarmSubmitterEntityData.h"

#include "core/utilities/src/TAAssert.h"
#include "bus/generic_agent/src/GenericAgent.h"
#include "app/maintenance_management/mms_agent/src/MmsConnection.h"

#include "app/maintenance_management/mms_agent/src/MmsMessageUtil.h"
#include "app/maintenance_management/mms_agent/src/MmsServerState.h"
#include <algorithm>

namespace TA_IRS_App
{



	MmsAlarmSubmitter::MmsAlarmSubmitter()
	{
		FUNCTION_ENTRY("MmsAlarmSubmitter");
		

		FUNCTION_EXIT;
	}


	MmsAlarmSubmitter::~MmsAlarmSubmitter()
	{
		FUNCTION_ENTRY("~MmsAlarmSubmitter");


		FUNCTION_EXIT;
	}

	void MmsAlarmSubmitter::submitMmsAlarmMessage(const TA_Base_Core::AlarmDetailCorbaDef& p_alarm)
	{

		//Thread guard this method
		TA_Base_Core::ThreadGuard guard( m_lock );

		FUNCTION_ENTRY("submitMmsAlarmMessage");

		LOG_GENERIC(SourceInfo, TA_Base_Core::DebugUtil::DebugInfo,
			"MmsAlarmSubmitter::submitAlarmMessage() called");

		checkOperationMode();

		std::string severity = MmsMessageUtil::getInstance()->getAlarmSeverityName(p_alarm);

		LOG_GENERIC(SourceInfo, TA_Base_Core::DebugUtil::DebugInfo,
			"getAlarmSeverityName() returned %s", severity.c_str() );

		try
		{
			MmsConnection::getInstance()->submitAlarmMessage(severity.c_str(), p_alarm);

			LOG_GENERIC(SourceInfo, TA_Base_Core::DebugUtil::DebugInfo,
				"MmsConnection::submitAlarmMessage() call OK. Now calling MmsServerState::reportMmsServerSucess()");


			MmsServerState::getInstance()->reportMmsServerSucess();

		}
		catch(TA_Base_Bus::MmsConnectionFailure& ex)
		{
			LOG_GENERIC(SourceInfo, TA_Base_Core::DebugUtil::DebugError,
				"MmsConnection::submitAlarmMessage() threw MmsConnectionFailure");

			//note that this method does not report failure to the client
			MmsServerState::getInstance()->reportMmsServerFailure(ex);

			//the alarm agent will catch this and report the attempt as "degradded"
			throw;
		}
		catch(...)
		{

			LOG_GENERIC(SourceInfo, TA_Base_Core::DebugUtil::DebugError,
				"MmsConnection::submitAlarmMessage() threw unknown exception type");

			std::string msg = "An error of unknown type occurred during alarm submission.";
			TA_Base_Bus::MmsConnectionFailure ex(msg.c_str(), 0);
			//note that this method does not report failure to the client
			MmsServerState::getInstance()->reportMmsServerFailure(ex);

			//the alarm agent will catch this and report the attempt as "degradded"
			throw TA_Base_Bus::MmsConnectionFailure("unknown", 0);
		}

		FUNCTION_EXIT;
	}

	void MmsAlarmSubmitter::initConfiguration(TA_Base_Core::MmsAlarmSubmitterEntityDataPtr pData)
	{
		m_entityName = pData->getName(); 
	}



	//IEntity methods:

    bool MmsAlarmSubmitter::isValid()
	{
		FUNCTION_ENTRY("isValid");


		FUNCTION_EXIT;

		//Can't think why this would ever not be valid
		return true;
	}


    void MmsAlarmSubmitter::start()
	{
		FUNCTION_ENTRY("start");

		activateServantWithName(m_entityName);

		FUNCTION_EXIT;
	}


    void MmsAlarmSubmitter::stop()
	{
		FUNCTION_ENTRY("stop");
		
		deactivateAndDeleteServant();

		FUNCTION_EXIT;
	}


    void MmsAlarmSubmitter::update(const TA_Base_Core::ConfigUpdateDetails& updateEvent)
	{
		FUNCTION_ENTRY("update");

        //nothing to do. no data to update

		FUNCTION_EXIT;
	}


    void MmsAlarmSubmitter::remove()
	{
		FUNCTION_ENTRY("remove");

        //does nothing
		FUNCTION_EXIT;
	}


    void MmsAlarmSubmitter::setToControlMode()
	{
		FUNCTION_ENTRY("setToControlMode");

		//see class comments
		FUNCTION_EXIT;
	}


    void MmsAlarmSubmitter::setToMonitorMode()
	{
		FUNCTION_ENTRY("setToMonitorMode");

		//see class comments
		FUNCTION_EXIT;
	}

	void MmsAlarmSubmitter::checkOperationMode()
	{
		TA_Base_Bus::GenericAgent::ensureControlMode();
	}

	//end IEntity methods
} //namespace TA_IRS_App



