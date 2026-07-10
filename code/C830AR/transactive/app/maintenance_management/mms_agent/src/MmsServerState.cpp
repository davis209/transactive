/**
  * The source code in this file is the property of 
  * Ripple Systems and is not for redistribution
  * in any form.
  *
  * Source:   $File: //depot/4669_T01271350/4669/transactive/app/maintenance_management/mms_agent/src/MmsServerState.cpp $
  * @author: christiandw
  * @version: $Revision: #1 $
  *
  * Last modification: $DateTime: 2008/11/28 16:26:01 $
  * Last modified by:  $Author: builder $
  * 
  * Impliimentation of the MmsServerState class
  */

#include "app/maintenance_management/mms_agent/src/MmsServerState.h"
#include "app/maintenance_management/mms_agent/src/MmsMessageUtil.h"

#include "bus/scada/datapoint_library/src/DataPoint.h"
#include "bus/scada/datapoint_library/src/BooleanDataPoint.h"
#include "core/data_access_interface/entity_access/src/DataPointEntityData.h"

#include "core/utilities/src/TAAssert.h"
#include <time.h>
#include <algorithm>

namespace TA_IRS_App
{
	MmsServerState* MmsServerState::m_instance = 0;
	TA_Base_Core::ReEntrantThreadLockable MmsServerState::m_lock;

	MmsServerState* MmsServerState::getInstance()
	{
		//Thread guard this method
		TA_Base_Core::ThreadGuard guard( m_lock );

		FUNCTION_ENTRY("getInstance");
		
		if(0 == m_instance)
		{
			m_instance = new MmsServerState();
		}

		FUNCTION_EXIT;

		return m_instance;
	}

	MmsServerState::MmsServerState()
		: m_globalInhibitApplied(false),
			m_globalInhibitPoint(0), m_serverStatusPoint(0)
	{
		FUNCTION_ENTRY("MmsServerState");

		FUNCTION_EXIT;
	}

	MmsServerState::~MmsServerState()
	{
		FUNCTION_ENTRY("~MmsServerState");

		if(0 != m_instance)
		{
			delete m_instance;
		}

		FUNCTION_EXIT;
	}

	void MmsServerState::initMmsServerState()
	{
		//Thread guard this method
		TA_Base_Core::ThreadGuard guard( m_lock );

		FUNCTION_ENTRY("initMmsServerState");

		if(0 == m_serverStatusPoint)
		{
			LOG_GENERIC(SourceInfo, TA_Base_Core::DebugUtil::DebugError,
				"0 == m_serverStatusPoint");
			TA_ASSERT(false, "No MMS Server point avaialable");
		}

		//TD
		//no need to set the MmsServer state
//		TA_Base_Bus::DpValue value( m_serverStatusPoint->getDataPointDataType(), m_serverStatusPoint->getEnumLabelsMap(), m_serverStatusPoint->getBooleanLabels() );
//		value.setBoolean( false );
//
//		//QUALITY_BAD_NOT_CONNECTED indicates we've never had a good value
//		TA_Base_Bus::DataPointState newState ( value, time( NULL ), TA_Base_Bus::QUALITY_BAD_NOT_CONNECTED );

		try
		{
			//QUALITY_BAD_NOT_CONNECTED indicates we've never had a good value
            m_serverStatusPoint->setToControlMode(); //TD15478
//			m_serverStatusPoint->updateFieldState( newState );
		}
		catch(...)
		{
			LOG_GENERIC(SourceInfo, TA_Base_Core::DebugUtil::DebugError,
				"m_serverStatusPoint->updateFieldState( newState ) threw");

		}

		TA_ASSERT(0 != m_connectionData.get(), "m_connectionData has not been initialized");
		m_connectionData->invalidate();

		bool lastInhibitState = false;
		try
		{
			lastInhibitState = m_connectionData->getGlobalMmsInhibit();
		}
		catch(...)
		{
			LOG_GENERIC(SourceInfo, TA_Base_Core::DebugUtil::DebugError,
				"m_connectionData->getGlobalMmsInhibit() threw");
			TA_ASSERT(false, "getGlobalMmsInhibit() threw");
		}

        //TD15478
		if(0 == m_globalInhibitPoint)
		{
			LOG_GENERIC(SourceInfo, TA_Base_Core::DebugUtil::DebugError,
				"0 == m_globalInhibitPoint");
			TA_ASSERT(false, "No MMS Global inhibitpiont avaialable");
		}

		try
		{
            m_globalInhibitPoint->setToControlMode();//TD15478
			setGlobalInhibitDataPoint( lastInhibitState );
		}
		catch(...)
		{
			LOG_GENERIC(SourceInfo, TA_Base_Core::DebugUtil::DebugError,
				"setGlobalInhibitDataPoint() threw");
			TA_ASSERT(false, "setGlobalInhibitDataPoint() threw");
		}

		FUNCTION_EXIT;
	}

	void MmsServerState::setData(TA_Base_Core::MmsConnectiontEntityDataPtr pData)
	{
		m_connectionData = pData;
	}
		
	TA_Base_Bus::DataPoint* MmsServerState::createDataPoint(TA_Base_Core::DataPointEntityDataPtr data, TA_Base_Bus::DataPointFactory* dataPointFactory)
	{
		//Thread guard this method
		TA_Base_Core::ThreadGuard guard( m_lock );

		FUNCTION_ENTRY("createDataPoint");

		TA_Base_Bus::DataPoint* dataPoint = 0;
		try
		{
			//Returns a registered datapoint CORBA object
			dataPoint = dataPointFactory->createDataPoint ( data );
		}
		catch(...)
		{
			dataPoint = 0;
		}
		//If we could not create a datapoint with this configuration...
		if (0 == dataPoint)
		{

			LOG_GENERIC(SourceInfo, TA_Base_Core::DebugUtil::DebugError,
				"DataPointFactory::createDataPoint() failed. Throwing TA_Base_Core::BadParameterException()");
			throw new TA_Base_Core::BadParameterException("DataPointFactory::createDataPoint() failed");
		}

		//see if this is a DI point:
		TA_Base_Bus::BooleanDataPoint* pBDP = dynamic_cast<TA_Base_Bus::BooleanDataPoint*>(dataPoint);

		if(0 == pBDP)
		{

			LOG_GENERIC(SourceInfo, TA_Base_Core::DebugUtil::DebugError,
				"The datapoint must be configured as boolean. Throwing TA_Base_Core::BadParameterException()");
			throw new TA_Base_Core::BadParameterException("The datapoint must be configured as boolean. Throwing TA_Base_Core::BadParameterException()");
		}

		//see if the address is set to indicate that is either of the points we care about

        //Mintao++
        //TD14021
        std::string addressMixedCase = dataPoint->getDataPointAddress();
        //Mintao++
        //TD14021
		std::string address(addressMixedCase);

		//convert the string to upper case:
		std::transform(addressMixedCase.begin(), addressMixedCase.end(), address.begin(), ::tolower);


		if(std::string::npos != address.find("globalmmsinhibit") )
		{
			m_globalInhibitPoint = pBDP;

			LOG_GENERIC(SourceInfo, TA_Base_Core::DebugUtil::DebugInfo,
				"Registered datapoint with address %s", addressMixedCase.c_str());
			FUNCTION_EXIT;
			return m_globalInhibitPoint;

		}
		else if(std::string::npos != address.find("mmsserverstatus") )
		{
			m_serverStatusPoint = pBDP;

			LOG_GENERIC(SourceInfo, TA_Base_Core::DebugUtil::DebugInfo,
				"Registered datapoint with address %s", addressMixedCase.c_str());
			FUNCTION_EXIT;
			return m_serverStatusPoint;
		}
		else //the configuration was bad
		{
            delete pBDP;
			LOG_GENERIC(SourceInfo, TA_Base_Core::DebugUtil::DebugError,
				"The datapoint address configured is invalid. Throwing TA_Base_Core::BadParameterException()");
			throw new TA_Base_Core::BadParameterException("The datapoint address configured is invalid. Throwing TA_Base_Core::BadParameterException()");

		}

		FUNCTION_EXIT;
		return 0;
	}

	unsigned long MmsServerState::getGlobalInhibitKey()
	{
		FUNCTION_ENTRY("setGlobalInhibitDataPoint");


		if(0 == m_globalInhibitPoint)
		{
			LOG_GENERIC(SourceInfo, TA_Base_Core::DebugUtil::DebugError,
				"The Global Inhibit datapoint is null");

			FUNCTION_EXIT;
			return 0;

		}

		unsigned int key = m_globalInhibitPoint->getEntityKey();

		FUNCTION_EXIT;
		return key;
	}

	void MmsServerState::setGlobalInhibitDataPoint(bool applied)
	{

		//Thread guard this method
		TA_Base_Core::ThreadGuard guard( m_lock );

		FUNCTION_ENTRY("setGlobalInhibitDataPoint");
		

		m_globalInhibitApplied = applied;

		if(0 == m_globalInhibitPoint)
		{
			LOG_GENERIC(SourceInfo, TA_Base_Core::DebugUtil::DebugError,
				"The Global Inhibit datapoint is null and could not be updated");
			TA_ASSERT(false, "The Global Inhibit datapoint is null and could not be updated");

		}

		//update the datapoint
		TA_Base_Bus::DpValue dpValue( m_globalInhibitPoint->getDataPointDataType(), m_globalInhibitPoint->getEnumLabelsMap(), m_globalInhibitPoint->getBooleanLabels() );
		dpValue.setBoolean( applied );

		TA_Base_Bus::DataPointState newState( dpValue, time( NULL ), TA_Base_Bus::QUALITY_GOOD_NO_SPECIFIC_REASON );

		try
		{
			m_globalInhibitPoint->updateFieldState( newState );

			std::string trueOrFalse = "false";
			if(applied) trueOrFalse = "true";

			LOG_GENERIC(SourceInfo, TA_Base_Core::DebugUtil::DebugInfo,
				"setGlobalInhibitDataPoint( %s ) call was OK.", trueOrFalse.c_str());

		}
		catch(...)
		{
			LOG_GENERIC(SourceInfo, TA_Base_Core::DebugUtil::DebugError,
				"The Global Inhibit datapoint could not be updated");
			TA_ASSERT(false, "The Global Inhibit datapoint is null and could not be updated");

		}

		FUNCTION_EXIT;		
	}

	bool MmsServerState::getGlobalInhibit()
	{
		FUNCTION_ENTRY("getGlobalInhibit");

		return m_globalInhibitApplied;

		FUNCTION_EXIT;		
	}

	void MmsServerState::reportMmsServerFailure(TA_Base_Bus::MmsConnectionFailure& ex)
	{
		//Thread guard this method
		TA_Base_Core::ThreadGuard guard( m_lock );

		FUNCTION_ENTRY("reportMmsServerFailure");

		setMmsServerStatePoint(true);

		FUNCTION_EXIT;
	}

	void MmsServerState::reportMmsServerSucess()
	{
		//Thread guard this method
		TA_Base_Core::ThreadGuard guard( m_lock );

		FUNCTION_ENTRY("reportMmsServerSucess");

		setMmsServerStatePoint(false);

		FUNCTION_EXIT;
	}

	void MmsServerState::setMmsServerStatePoint(bool value)
	{
		if(0 == m_serverStatusPoint)
		{
			LOG_GENERIC(SourceInfo, TA_Base_Core::DebugUtil::DebugError,
				"The MMS Server status datapoint is null and could not be updated");
			return;
		}

		//update the datapoint
		TA_Base_Bus::DpValue dpValue( m_serverStatusPoint->getDataPointDataType(), m_serverStatusPoint->getEnumLabelsMap(), m_serverStatusPoint->getBooleanLabels() );
		dpValue.setBoolean( value );

		TA_Base_Bus::DataPointState newState( dpValue, time( NULL ), TA_Base_Bus::QUALITY_GOOD_NO_SPECIFIC_REASON );

		try
		{
			m_serverStatusPoint->updateFieldState( newState );

			std::string trueOrFalse = "false";
			if(value) trueOrFalse = "true";

			LOG_GENERIC(SourceInfo, TA_Base_Core::DebugUtil::DebugInfo,
				"setMmsServerStatePoint( %s ) call was OK.", trueOrFalse.c_str());
		}
		catch(...)
		{
			LOG_GENERIC(SourceInfo, TA_Base_Core::DebugUtil::DebugError,
				"The MMS Server status datapoint could not be updated");
		}
    }
} //end namespace TA_IRS_App
