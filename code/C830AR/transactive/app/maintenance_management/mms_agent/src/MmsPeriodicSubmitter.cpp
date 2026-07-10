/**
  * The source code in this file is the property of
  * Ripple Systems and is not for redistribution in any form.
  * 
  * Source:    $File: //depot/4669_T01271350/4669/transactive/app/maintenance_management/mms_agent/src/MmsPeriodicSubmitter.cpp $
  * @author:   Chris DeWolfe
  * @version:  $Revision: #1 $
  * @see:      9999-Q01-02-0016-GenericAgent_Usage_Guidelines.doc
  *
  * Last modification: $DateTime: 2008/11/28 16:26:01 $
  * Last mofified by:  $Author: builder $
  *
  * MmsPeriodicSubmitter class implimentation. See header file for class descrption
  */

#if defined(_MSC_VER)
#pragma warning(disable:4786)
#endif // defined (_MSC_VER)


#include "app/maintenance_management/mms_agent/src/MmsPeriodicSubmitter.h"
#include "core/data_access_interface/entity_access/src/MmsPeriodicEntityData.h"
#include "app/maintenance_management/mms_agent/src/MmsConnectionException.h"

#include "app/maintenance_management/mms_agent/src/MmsMessageUtil.h"
#include "app/maintenance_management/mms_agent/src/MmsServerState.h"

#include "core/data_access_interface/entity_access/src/EntityAccessFactory.h"
#include "bus/generic_agent/src/GenericAgent.h"
#include "core/utilities/src/TAAssert.h"
#include "core/message/src/MessageConfig.h"
#include "core/message/src/NameValuePair.h"

#include <sstream>
#include <string>
#include <iomanip> 
#include <algorithm>


namespace TA_IRS_App
{

	MmsPeriodicSubmitter::MmsPeriodicSubmitter()
		:m_thread(0)
	{
		FUNCTION_ENTRY("MmsPeriodicSubmitter");		

		FUNCTION_EXIT;
	}




	MmsPeriodicSubmitter::~MmsPeriodicSubmitter()
	{
		FUNCTION_ENTRY("~MmsPeriodicSubmitter");

		terminateAndDeleteThread();

		FUNCTION_EXIT;
	}

	void MmsPeriodicSubmitter::terminateAndDeleteThread()
	{
		FUNCTION_ENTRY("terminateAndDeleteThread");


		if(m_thread != 0)
		{
			try
			{
				//stop the thread if required
				if(TA_Base_Core::Thread::THREAD_STATE_UNKNOWN != m_thread->getCurrentState() &&
				   TA_Base_Core::Thread::THREAD_STATE_TERMINATED != m_thread->getCurrentState() )	
				{
					m_thread->terminateAndWait();

					LOG_GENERIC(SourceInfo, TA_Base_Core::DebugUtil::DebugInfo,
						"MmsPeriodicSubmitter::setToMonitorMode() has stopped the thread" );
				}
				else
				{
					LOG_GENERIC(SourceInfo, TA_Base_Core::DebugUtil::DebugInfo,
						"MmsPeriodicSubmitter::setToMonitorMode() says the thread is already stopped" );
				}
			}
			catch(...)
			{
				LOG_GENERIC(SourceInfo, TA_Base_Core::DebugUtil::DebugError,
					"The attempt to stop the thread threw" );
			}

			try
			{
				delete m_thread;
			}
			catch(...)
			{
				LOG_GENERIC(SourceInfo, TA_Base_Core::DebugUtil::DebugError,
					"The attempt to delete the thread threw" );
			}
			m_thread = 0;
		} 
		else //m_thread == 0
		{
			LOG_GENERIC(SourceInfo, TA_Base_Core::DebugUtil::DebugInfo,
				"m_thread is already set to 0" );
		}


		FUNCTION_EXIT;
	}

	void MmsPeriodicSubmitter::createAndStartThread()
	{
		FUNCTION_ENTRY("createAndStartThread");

		if(m_thread == 0)
		{
			m_thread = new PeriodicThread();
			//chenlei++
			//init the thread if it needs creation
			TA_ASSERT(m_data.get() != 0, "failed m_data != 0" );
			m_data->invalidate();
			m_thread->initConfiguration(m_data);
		}
		else
		{
			LOG_GENERIC(SourceInfo, TA_Base_Core::DebugUtil::DebugError,
				"m_thread != 0, but it should!" );
		}


		//ensure that the thread has the correct data
		//chenlei--
		//remove this to the MmsPeriodicSubmitter::initConfiguration() fuction
		//also this thread is created there
		//TA_ASSERT(m_data.get() != 0, "failed m_data != 0" );
		//m_data->invalidate();
		//m_thread->initConfiguration(m_data);


		try
		{
			if(TA_Base_Core::Thread::THREAD_STATE_RUNNING != m_thread->getCurrentState())
			{
				m_thread->start();

				LOG_GENERIC(SourceInfo, TA_Base_Core::DebugUtil::DebugInfo,
					"MmsPeriodicSubmitter::setToControlMode() started the thread" );
			}
			else
			{
				LOG_GENERIC(SourceInfo, TA_Base_Core::DebugUtil::DebugInfo,
					"MmsPeriodicSubmitter::setToControlMode() says the thread is already started" );
			}


		}
		catch(...)
		{
			LOG_GENERIC(SourceInfo, TA_Base_Core::DebugUtil::DebugError,
				"attempt to start thread failed" );

			TA_ASSERT(false, "attempt to start thread failed" );
		}



		FUNCTION_EXIT;
	}


	void MmsPeriodicSubmitter::initConfiguration(TA_Base_Core::MmsPeriodicEntityDataPtr pData)
	{
		FUNCTION_ENTRY("initConfiguration");

		m_data = pData;

		m_entityName = pData->getName();
                m_entityKey = pData->getKey();

		pData->invalidate();

		//chenlei++
		//create and init the periodic thread here
		if(m_thread == 0)
		{
			m_thread = new PeriodicThread();
		}
		m_thread->initConfiguration(m_data);


		//Following is done when set to control
		//update the connection tester
		//if(0 != m_thread) m_thread->initConfiguration(pData);

		FUNCTION_EXIT;
	}

	//IEntity methods:

    bool MmsPeriodicSubmitter::isValid()
	{
		FUNCTION_ENTRY("isValid");


		FUNCTION_EXIT;

		//Can't think why this would ever not be valid
		return true;
	}


    void MmsPeriodicSubmitter::start()
	{
		FUNCTION_ENTRY("start");

		FUNCTION_EXIT;
	}


    void MmsPeriodicSubmitter::stop()
	{
		FUNCTION_ENTRY("stop");		

		FUNCTION_EXIT;
	}


    void MmsPeriodicSubmitter::update(const TA_Base_Core::ConfigUpdateDetails& updateEvent)
	{
		FUNCTION_ENTRY("update");
		
		//verify that this update is for me
		unsigned long keyToUpdate = updateEvent.getKey();
		if(keyToUpdate != m_entityKey)
		{
			FUNCTION_EXIT;
			return;
		}

		
		TA_Base_Core::MmsPeriodicEntityData* pData = 
			dynamic_cast<TA_Base_Core::MmsPeriodicEntityData*>(
			TA_Base_Core::EntityAccessFactory::getInstance().getEntity(updateEvent.getKey()));


		if (pData == 0)
		{
			LOG_GENERIC(SourceInfo, TA_Base_Core::DebugUtil::DebugError,
				"update() was called with ConfigUpdateDetails of unexpected subtype" );

			FUNCTION_EXIT;
			return;

		}

		if(m_thread == 0)
		{
			LOG_GENERIC(SourceInfo, TA_Base_Core::DebugUtil::DebugError,
				"there is no thread to update" );

			FUNCTION_EXIT;
			return;
		}

		pData->invalidate();

        std::vector<std::string> paramNames = updateEvent.getChangedParams();
		std::vector<std::string>::const_iterator itNames;
		for(itNames = paramNames.begin(); itNames != paramNames.end(); itNames++)
		{
			if(TA_Base_Core::MmsPeriodicEntityData::SCAN_PERIOD_MIN == *itNames)
			{
				m_thread->setScanPeriodMinutes( pData->getScanPeriodMinutes() );
			}
			if(TA_Base_Core::MmsPeriodicEntityData::BACKLOG_CHECK_PERIOD_HOURS == *itNames)
			{
				m_thread->setBacklogPeriodHours( pData->getBackLogCheckPeriodHours() );
			}
		}


		FUNCTION_EXIT;
	}


    void MmsPeriodicSubmitter::remove()
	{
		FUNCTION_ENTRY("remove");

        //does nothing
		FUNCTION_EXIT;
	}


    void MmsPeriodicSubmitter::setToControlMode()
	{
		FUNCTION_ENTRY("setToControlMode");

		createAndStartThread();

		FUNCTION_EXIT;
	}


    void MmsPeriodicSubmitter::setToMonitorMode()
	{
		FUNCTION_ENTRY("setToMonitorMode");

		terminateAndDeleteThread();

		FUNCTION_EXIT;
	}


	//end IEntity methods



} //end namespace TA_IRS_App



