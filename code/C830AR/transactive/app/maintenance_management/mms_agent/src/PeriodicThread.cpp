
/**
  * The source code in this file is the property of 
  * Ripple Systems and is not for redistribution
  * in any form.
  *
  * Source:   $File: //depot/4669_T01271350/4669/transactive/app/maintenance_management/mms_agent/src/PeriodicThread.cpp $
  * @author:  C. DeWolfe
  * @version: $Revision: #3 $
  *
  * Last modification: $DateTime: 2010/01/25 18:07:02 $
  * Last modified by:  $Author: builder $
  * 
  * Implimentation of PeriodicThread
  *
  */
#pragma warning(disable:4786)

#include "app/maintenance_management/mms_agent/src/PeriodicThread.h"
#include "app/maintenance_management/mms_agent/src/MmsApiWrapper.h"
#include "app/maintenance_management/mms_agent/src/MmsMessageUtil.h"
#include "app/maintenance_management/mms_agent/src/MmsServerState.h"
#include "app/maintenance_management/mms_agent/src/MmsAgent.h"


#include "core/data_access_interface/entity_access/src/MmsPeriodicEntityData.h"
#include "core/data_access_interface/entity_access/src/MmsAgentEntityData.h"
#include "core/message/src/ConfigUpdateMessageSender.h"
#include "core/message/src/MessagePublicationManager.h"
#include "core/message/types/ConfigUpdate_MessageTypes.h"
#include "core/alarm/src/AlarmHelperManager.h"
// Code Commented by Hu Wenguang
// Bug 635 (TD10892)
//#include "core/alarm/src/AlarmHelper.h"
// Bug 635 (TD10892)
// Hu Wenguang
#include "core/message/types/MmsAlarm_MessageTypes.h"

#include "app/maintenance_management/mms_agent/src/MmsConnectionException.h"
#include "core/utilities/src/TAAssert.h"
//#include "bus/maintenance_management/mmsCorbaDef/idl/src/IMmsConnectionCorbaDef.h"
#include "app/maintenance_management/mms_agent/src/MmsConnection.h"
//TD16871 
//Mintao++
#include "core/configuration_updates/src/OnlineUpdateListener.h"

#include <ctime>
#include <algorithm>
#include <sstream>

namespace TA_IRS_App
{

	const CORBA::Double PeriodicThread::BAD_READ_VALUE = -999999998.0;

    // Modified by Hu Wenguang
    // Bug 635 (TD10892)
	PeriodicThread::PeriodicThread()
		: m_scanPeriodMinutes(0), 
		  m_backCheckHours(0), m_stopRequest(true), m_configUpdateSender(0)
		  /*/TD18093,jianghp, m_alarmHelper(TA_Base_Core::AlarmHelperManager::getInstance().getAlarmHelper())*/
    // Bug 635 (TD10892)
    // Hu Wenguang
	{
		FUNCTION_ENTRY("PeriodicThread");

		m_dataAccess = &(TA_IRS_Core::MmSchedulingAccessFactory::getInstance());

		TA_ASSERT(0 != m_dataAccess, "MmSchedulingAccessFactory::getInstance() returned 0");
        //TD16871 
        //Mintao++
        TA_Base_Core::OnlineUpdateListener::getInstance().registerAllInterests(TA_Base_Core::ENTITY, *this);

		FUNCTION_EXIT;
	}



	PeriodicThread::~PeriodicThread()
	{
		FUNCTION_ENTRY("~PeriodicThread");

		if(0 != m_dataAccess)
		{
			try
			{
				m_dataAccess->cleanUp();
			}
			catch(...)
			{
				LOG_GENERIC(SourceInfo, TA_Base_Core::DebugUtil::DebugError,
					"m_dataAccess->cleanUp() threw.");
			}
		}

		TA_Base_Core::AlarmHelperManager::getInstance().cleanUp();

        //TD16871 
        //Mintao++
        TA_Base_Core::OnlineUpdateListener::getInstance().deregisterAllInterests(TA_Base_Core::ENTITY, *this);

		FUNCTION_EXIT;
	}

   void PeriodicThread::submitConfigUpdate()
    {
        FUNCTION_ENTRY("submitConfigUpdate");   
    
        if (0 == m_configUpdateSender)
        {

            m_configUpdateSender = 
                TA_Base_Core::MessagePublicationManager::getInstance().
                getConfigUpdateMessageSender( TA_Base_Core::ConfigUpdate::Context );
        }
        
        try
        {
            TA_ASSERT(m_configUpdateSender != 0, "m_configUpdateSender is NULL");

			std::vector<std::string> changes;
			std::string colName("LAST_SENT"); //we only change the LAST_SENT column
			changes.push_back(colName);
            m_configUpdateSender->sendConfigUpdateMessage(
                TA_Base_Core::ConfigUpdate::ConfigMmsSchedule,           // Message Type
                -1,                                          // We use -1 as a flag to say one or more records
                TA_Base_Core::Update,                                   // EModificationType (upd/del)
                changes);                                                 // FilterableData

            
			LOG_GENERIC(SourceInfo, TA_Base_Core::DebugUtil::DebugInfo,
				"sendConfigUpdateMessage( ) called to update LAST_SENT");

        }
        catch(...)
        {
            std::string msg ("Failed to submit the ConfigMmsSchedule Config Update Message");
            LOG(SourceInfo, TA_Base_Core::DebugUtil::GenericLog,  TA_Base_Core::DebugUtil::DebugError, msg.c_str());
        }    

        FUNCTION_EXIT;
    }

	void PeriodicThread::terminate()
	{
		FUNCTION_ENTRY("terminate");

		m_stopRequest = true;

		LOG_GENERIC(SourceInfo, TA_Base_Core::DebugUtil::DebugInfo,
			"terminate( ) called");

		FUNCTION_EXIT;
	}

	void PeriodicThread::initConfiguration(TA_Base_Core::MmsPeriodicEntityDataPtr pData)
	{
		FUNCTION_ENTRY("initConfiguration");

		LOG_GENERIC(SourceInfo, TA_Base_Core::DebugUtil::DebugInfo,
			"initConfiguration( ) called");

		m_periodicData = pData;
		//chenlei--
		//move this initialization to the beginning of the run() function
		//setBacklogPeriodHours( pData->getBackLogCheckPeriodHours() );
		//setScanPeriodMinutes( pData->getScanPeriodMinutes() );

		FUNCTION_EXIT;
	}

	//chenlei++
	//this function is used to initialize the configuration
	//using the entity data from the member
	void PeriodicThread::initConfiguration()
	{
		m_periodicData->invalidate();
		setBacklogPeriodHours( m_periodicData->getBackLogCheckPeriodHours() );
		setScanPeriodMinutes( m_periodicData->getScanPeriodMinutes() );
	}

	void PeriodicThread::setScanPeriodMinutes(unsigned int minutes)
	{
		FUNCTION_ENTRY("setScanPeriodMinutes");

		m_scanPeriodMinutes = minutes;

		if(0 == m_scanPeriodMinutes)
		{
			LOG_GENERIC(SourceInfo, TA_Base_Core::DebugUtil::DebugWarn,
				"Scanning for due periodic messages is currently disabled.");
		}
		else
		{
			LOG_GENERIC(SourceInfo, TA_Base_Core::DebugUtil::DebugInfo,
				"Now scanning for due periodic messages every %u minutes.", minutes);
		}

		FUNCTION_EXIT;
	}

	void PeriodicThread::setBacklogPeriodHours(unsigned int hours)
	{
		FUNCTION_ENTRY("setBacklogPeriodHours");

		m_backCheckHours = hours;


		LOG_GENERIC(SourceInfo, TA_Base_Core::DebugUtil::DebugWarn,
			"Backlog check period set to %u hours", hours);


		FUNCTION_EXIT;
	}

	void PeriodicThread::run()
	{
		FUNCTION_ENTRY("sendBackLog");
		m_stopRequest = false;

		//chenlei++
		//initiliazing the configuration
		initConfiguration();

		LOG_GENERIC(SourceInfo, TA_Base_Core::DebugUtil::DebugInfo,
			"PeriodicThread::run() called");

		//so that other components can init before this thing
		//goes about it's business...
		sleep(10000);

		readScheduleData();

		updateObsoleteLastSent();
			
		while(m_stopRequest == false)
		{
            // AZ++ TES 4673
            try
            {
			    //note that 0 value disables periodic messages
			    if(m_scanPeriodMinutes != 0)
			    {
				    sendDueMessages();			
			    }
			    else
			    {
				    sleep(2000);
				    continue;
			    }
            }
            catch (TA_Base_Core::TransactiveException& e)
            {
                LOG_GENERIC(SourceInfo, TA_Base_Core::DebugUtil::DebugWarn, e.what());
            }
            catch (...)
            {
                LOG_GENERIC(SourceInfo, TA_Base_Core::DebugUtil::DebugWarn, "Unknown exception");
            }
            // ++AZ

			//have a nap for the time remaining
			time_t lastScan;
			time(&lastScan);
			time_t now;
			time(&now);
			while( ( (now - lastScan) / 60 ) < static_cast<long>(m_scanPeriodMinutes) )
			{
				if(m_stopRequest == true) break;						
				sleep(2000);
				time(&now);
			}			
		}

	}


	void PeriodicThread::sendDueMessages()
	{
		FUNCTION_ENTRY("sendDueMessages");

		//get the subsystems that are due to be reported
		std::vector<unsigned int> subsystemKeys = getDueSubsystemKeys();


		// dahe++
		if (0 == subsystemKeys.size())
		{
			return;
		}
		// ++dahe

		LOG_GENERIC(SourceInfo, TA_Base_Core::DebugUtil::DebugInfo,
			"getDueSubsystemKeys() returned %d subsytems", subsystemKeys.size());


		//get the set of periods that coorospnd to today's date
		const std::vector<unsigned int> periods = getTodaysPeriods();

		std::vector<TA_Base_Bus::ScadaUtilities::MaintenanceData> data;
		std::vector<TA_Base_Bus::ScadaUtilities::MaintenanceData> allData;

		//a list of the subsystems that we succesfully report
		std::vector<unsigned int> subsystemsReported;

		//a list of the subsystems that we fail to report
		std::vector<unsigned int> subsystemsFailed;

		//get the data from the datapoints in each of the subsystems
        TA_Base_Bus::ScadaUtilities& scadaUtilities = TA_Base_Bus::ScadaUtilities::getInstance();

        for(std::vector<unsigned int>::const_iterator it = subsystemKeys.begin(); it != subsystemKeys.end(); ++it)
		{
			try
			{
				data = scadaUtilities.getMaintenanceData(*it, periods);
				allData.insert( allData.end(), data.begin(), data.end() );

				
				//add the key for this subsystem to the list of subsystems reported
				subsystemsReported.push_back(*it);

			}
			catch(...)
			{
				//add the key for this subsystem to the list of subsystems failed
				subsystemsFailed.push_back(*it);				
			}
		}

		LOG_GENERIC(SourceInfo, TA_Base_Core::DebugUtil::DebugInfo,
			"retrieved MaintenanceData on %d points", allData.size());

		//create and send a message for each datapoint
		for(unsigned long j = 0; j < allData.size(); j++)
		{
			sendMessage( allData[j] );
		}


		recordSubsystemsSent(subsystemsReported);

		reportSubsystemsFailed(subsystemsFailed);
		
	}

	void PeriodicThread::reportSubsystemsFailed(const std::vector<unsigned int>& subsystemKeys)
	{
		FUNCTION_ENTRY("reportSubsystemsFailed");

		if(subsystemKeys.size() == 0)
		{
			LOG_GENERIC(SourceInfo, TA_Base_Core::DebugUtil::DebugDebug,
				"There are no failed subsystems to report");

			//clear the alarm
			//API REF:
			//AlarmHelper::closeAlarm(const TA_Base_Core::MessageType& alarmType,
			//								unsigned long entityKey)

			try
			{
                // Modified by Hu Wenguang
                // Bug 635 (TD10892)
				TA_Base_Core::AlarmHelperManager::getInstance().getAlarmHelper().closeAlarmAtLocation( 
                    TA_Base_Core::MmsAlarm::MmsSubsystemReportFailure,
					MmsAgent::getAgentData()->getKey(),
                    MmsAgent::getAgentData()->getLocation(), ""
                    );
                // Bug 635 (TD10892)
                // Hu Wenguang
			}
			catch(...)
			{
				LOG_GENERIC(SourceInfo, TA_Base_Core::DebugUtil::DebugError,
					"Could not close alarm");
			}

			return;
		}
		

		//create a comma-space (", ") seperated list of the names of the 
		//failed subsystems:
		std::vector<unsigned int>::const_iterator iter;
		std::ostringstream strm;
		
		for(iter = subsystemKeys.begin();iter !=subsystemKeys.end() ;iter++)
		{
			std::string subName = MmsMessageUtil::getInstance()->getSubsystemName( *iter );

			if( subName.empty() )
			{
				strm << "subsystem pkey " << *iter << ", ";
			}
			else
			{
				strm << subName << ", ";
			}

		}

		std::string subNames = strm.str();

		//raise ( or update ) the alarm:

		// Set up the description parameter
		TA_Base_Core::DescriptionParameters parameters;
		parameters.push_back(new TA_Base_Core::NameValuePair("SubsystemNames", subNames));


			try
			{
                // Modified by Hu Wenguang
                // Bug 635 (TD10892)
				TA_Base_Core::AlarmHelperManager::getInstance().getAlarmHelper().submitAlarmWithAsset(
					TA_Base_Core::MmsAlarm::MmsSubsystemReportFailure,
					MmsAgent::getAgentData()->getKey(),
					MmsAgent::getAgentData()->getTypeKey(),
					parameters,
					MmsAgent::getAgentData()->getName(),
					MmsAgent::getAgentData()->getLocation(),
					MmsAgent::getAgentData()->getSubsystem(),
					MmsAgent::getAgentData()->getAssetName(),
					TA_Base_Core::AlarmConstants::defaultTime );
                // Bug 635 (TD10892)
                // Hu Wenguang
			}
			catch(...)
			{
				LOG_GENERIC(SourceInfo, TA_Base_Core::DebugUtil::DebugError,
					"Could not raise alarm");
			}

		FUNCTION_EXIT;
	}

	void PeriodicThread::recordSubsystemsSent(const std::vector<unsigned int>& subsystemKeys)
	{
		FUNCTION_ENTRY("recordSubsystemsSent");

		if(subsystemKeys.size() == 0)
		{
			return;
		}
		
		updateDataIfRequired();

		time_t now;
		time(&now);

		std::vector<unsigned int>::const_iterator iter;

		//for each record see if the key matches one to be reported
		for(unsigned long  i = 0; i < m_data.size(); i++)
		{
			unsigned int subKey = m_data[i]->getSubsystemKey();
			iter = std::find(subsystemKeys.begin(), subsystemKeys.end(), subKey);
		
			//if the key is in the list...
			if(subsystemKeys.end() != iter)
			{
				
				try
				{
					//setLastReportedTime for this subystem 
					m_data[i]->setLastReportedTime(now);
					m_data[i]->applyChanges();

					LOG_GENERIC(SourceInfo, TA_Base_Core::DebugUtil::DebugInfo,
						"called setLastReportedTime(now) and applyChanges() on %s subsystem", m_data[i]->getSubsystemName().c_str());
				}
				catch(...)
				{
					LOG_GENERIC(SourceInfo, TA_Base_Core::DebugUtil::DebugError,
						"Failed to write MMS scheduling LastReportedTime to database");
				}
			}
		}

		submitConfigUpdate();

		FUNCTION_EXIT;
	}

	std::string PeriodicThread::maintenanceDataToString(const TA_Base_Bus::ScadaUtilities::MaintenanceData& data)
	{
		/* API REF:
		struct MaintenanceData
		{
			//taken from the MMS_REPORT_TYPE param of the datapoint config
			enum MessageType {CPT, VAL};
			MessageType type;

			//the asset name of the parent datanode 
			std::string assetName;

			//the portion of the entity name of the datapoint that follows the last '.' character
			std::string pointName;

			//the description field of the datapoint entity
			std::string description;

			//indicates that the value could not be read and should not be considered valid
			bool readFailed;

			//a integer representation of the current datapoint value:
			// bool values should map to 0(false) 1(true)
			// decimal values are passed after engineering units applied.
			double value;

		};
		*/

		std::ostringstream str;

		str << "MessageType: ";
		if(data.type == TA_Base_Bus::ScadaUtilities::MaintenanceData::CPT)
		{
			str  << "CPT\n";
		}
		else
		{
			str  << "VAL\n";
		}

		str  << "assetName: ";
		str  <<  data.assetName;

		str  <<  "\npointName: ";
		str  <<  data.pointName;

		str  <<  "\ndescription: ";
		str  <<  data.description;

		str  <<  "\nreadFailed: ";
		if(data.readFailed)
		{
			str  <<  "true";
		}
		else
		{
			str << "false";
		}

		str  <<  "\nvalue: ";
		str  << data.value;
		
		std::string result;
		result = str.str();

		return result;
	}

	void PeriodicThread::sendMessage(const TA_Base_Bus::ScadaUtilities::MaintenanceData& data)
	{
		FUNCTION_ENTRY("sendMessage");

		CORBA::Double dataPointValue = BAD_READ_VALUE;

		//todo: set this to debug
		LOG_GENERIC(SourceInfo, TA_Base_Core::DebugUtil::DebugInfo,	
			"zhangnan sendMessage( data ) called where data is: %s", maintenanceDataToString( data ).c_str() );				


		//if the read failed
		if(data.readFailed)
		{
			recordDataFailure(data);
		}
		else //read was ok
		{
			dataPointValue = data.value;
		}

		//for logging:
		std::string assetName = data.assetName;

		try
		{
			if(data.type == TA_Base_Bus::ScadaUtilities::MaintenanceData::CPT)
			{
				MmsConnection::getInstance()->submitCounterMessage(data.assetName.c_str(), data.pointName.c_str(), 
					data.description.c_str(), dataPointValue);
			}
			else
			{
				MmsConnection::getInstance()->submitMeasurementMessage(data.assetName.c_str(), data.pointName.c_str(), 
					data.description.c_str(), dataPointValue);
			}

			MmsServerState::getInstance()->reportMmsServerSucess();
		}
		catch(TA_Base_Bus::MmsConnectionFailure& ex)
		{
			LOG_GENERIC(SourceInfo, TA_Base_Core::DebugUtil::DebugError,	
				"submitCounterMessage( ) or submitMeasurementMessage( ) call threw MmsConnectionFailure on point with assetName %s", assetName.c_str() );				

			MmsServerState::getInstance()->reportMmsServerFailure(ex);
		}
		catch(...)
		{

			LOG_GENERIC(SourceInfo, TA_Base_Core::DebugUtil::DebugError,	
				"submitCounterMessage( ) or submitMeasurementMessage( ) call threw unk on point with assetName %s", assetName.c_str() );				

			TA_Base_Bus::MmsConnectionFailure ex("message submission failed", 0);
			MmsServerState::getInstance()->reportMmsServerFailure(ex);
		}

		FUNCTION_EXIT;
	}


	void PeriodicThread::recordDataFailure(const TA_Base_Bus::ScadaUtilities::MaintenanceData& data)
	{
		FUNCTION_ENTRY("recordDataFailure");

		m_dataFailures.push_back(data);

		FUNCTION_EXIT;
	}


	void PeriodicThread::reportDataFailures(const TA_Base_Bus::ScadaUtilities::MaintenanceData& data)
	{
		FUNCTION_ENTRY("reportDataFailures");


		for(unsigned long  i = 0; i < m_dataFailures.size(); i++)
		{
			//todo

			//should we be doing additional reporting, e.g. event?
		}

		if(m_dataFailures.size() > 0)
		{
			LOG_GENERIC(SourceInfo, TA_Base_Core::DebugUtil::DebugError,	"%d datapoint read failures were were reported during message submission.",  
				m_dataFailures.size());	
		}

		//then clear
		m_dataFailures.clear();

		FUNCTION_EXIT;
	}


	std::vector<unsigned int> PeriodicThread::getTodaysPeriods()
	{
		FUNCTION_ENTRY("getTodaysPeriods");

		time_t now;
		//this is in seconds since midnight (00:00:00), January 1, 1970
		time(&now);

		//convert seconds to days;
		//I have added OFFSET so that mondays always have a 7 period
		//todo adjust this if they want something else but if you change it
		//update the GUI utility as well
		unsigned int OFFSET = 3;
		const unsigned int days = (now / 60 / 60 / 24) + OFFSET;

		std::vector<unsigned int> periods;

		//now see what ints divide evenly into the number of days
		for(unsigned int factor = 1; factor <= days && factor <= 365; factor++)
		{
			if(0 == days % factor)
			{
				periods.push_back(factor);
			}
		}

		FUNCTION_EXIT;
		return periods;
	}

	/**
	*
	* @return true if the subsystem is due if lastReported DAY is before TODAY and
	* report time is before or == current time
	*
	* @param now represents the current time
	*/
	bool PeriodicThread::subsystemIsDue(const time_t now, TA_IRS_Core::IMmSchedulingData  * const data)
	{
		
		time_t lastRun = data->getLastReported();
		time_t scheduledTime = data->getScheduledTime();


		tm* tempTm = 0;
		tempTm = localtime( &now );
		tm currentTime = *tempTm;
		unsigned int currentMinuteOfDay = currentTime.tm_hour * 60 + currentTime.tm_min;

		tempTm = localtime( &lastRun );
		tm lastReportedDate = *tempTm;

		tempTm = localtime( &scheduledTime );
		tm configuredTime = *tempTm;
		unsigned int configuredMinuteOfDay = configuredTime.tm_hour * 60 + configuredTime.tm_min;


		//for logging:
		std::string name = data->getSubsystemName();


		//if the configured time is later than the current time this subsystem is not due
		if(configuredMinuteOfDay > currentMinuteOfDay)
		{
			LOG_GENERIC(SourceInfo, TA_Base_Core::DebugUtil::DebugInfo,
				"subsystemIsDue() returning FALSE for subsystem %s ", name.c_str());
			return false;
		}



        //Mintao++ TD12989
		unsigned int lastSendMinuteOfDay = lastReportedDate.tm_hour * 60 + lastReportedDate.tm_min;
		//if lastReported YEAR is before now YEAR or
		// same year and lastReported DAY is before TODAY or
        // same year, same day and lastReport Min is before the sub is due
		if(lastReportedDate.tm_year < currentTime.tm_year || 
			( lastReportedDate.tm_year == currentTime.tm_year && lastReportedDate.tm_yday < currentTime.tm_yday)  ||
			( lastReportedDate.tm_year == currentTime.tm_year && lastReportedDate.tm_yday == currentTime.tm_yday  && lastReportedDate.tm_hour < currentTime.tm_hour && lastSendMinuteOfDay < configuredMinuteOfDay) ||
			( lastReportedDate.tm_year == currentTime.tm_year && lastReportedDate.tm_yday == currentTime.tm_yday  && lastReportedDate.tm_hour == currentTime.tm_hour && lastReportedDate.tm_min < currentTime.tm_min && lastSendMinuteOfDay < configuredMinuteOfDay) 
            )
		{

			LOG_GENERIC(SourceInfo, TA_Base_Core::DebugUtil::DebugInfo,
				"subsystemIsDue() returning TRUE for subsystem %s", name.c_str());

			return true;
		}

		LOG_GENERIC(SourceInfo, TA_Base_Core::DebugUtil::DebugInfo,
			"subsystemIsDue() returning FALSE for subsystem %s ", name.c_str());


		return false;
	}


	std::vector<unsigned int> PeriodicThread::getDueSubsystemKeys()
	{
		FUNCTION_ENTRY("getDueSubsystemKeys");

		updateDataIfRequired();

		std::vector<unsigned int> keys;


		time_t now;
		time(&now);

		for(unsigned int i = 0; i < m_data.size(); i++)
		{

			if( subsystemIsDue(now, m_data[i]) )
			{
				keys.push_back(m_data[i]->getSubsystemKey());

				LOG_GENERIC(SourceInfo, TA_Base_Core::DebugUtil::DebugInfo,	"%s subsystem is due for reporting",  
					m_data[i]->getSubsystemName().c_str());				
			}
		}


		FUNCTION_EXIT;
		return keys;
	}

	void PeriodicThread::updateObsoleteLastSent()
	{
		FUNCTION_ENTRY("updateObsoleteLastSent");
		
		updateDataIfRequired();

		time_t now;
		time(&now);

		//we will change this value if we find any subs that 
		//need updatating.
		bool updatedSubsystem = false;

		for(unsigned int i = 0; i < m_data.size(); i++)
		{
			time_t lastReported = m_data[i]->getLastReported();
			time_t ageInSeconds = now - lastReported;
			if(ageInSeconds < 0 ) ageInSeconds = 1;

			unsigned backLogSeconds = m_backCheckHours * 60 * 60;

			if(ageInSeconds > static_cast<long>(backLogSeconds))
			{
				std::string name = m_data[i]->getSubsystemName();

				try
				{
					updatedSubsystem = true;
					
					LOG_GENERIC(SourceInfo, TA_Base_Core::DebugUtil::DebugInfo,
						"will now call setLastReportedTime(now) and applyChanges() on subsystem %s", name.c_str());

					m_data[i]->setLastReportedTime(now);
					m_data[i]->applyChanges();

		
				}
				catch(...)
				{
					LOG_GENERIC(SourceInfo, TA_Base_Core::DebugUtil::DebugError,
						"setLastReportedTime(now) or applyChanges() threw on subsystem %s", name.c_str());
				}
			}

		}

		//if we have updated one or more subsystems submit a config update message:
		if(updatedSubsystem)
		{
			submitConfigUpdate();
		}

		FUNCTION_EXIT;
	}




	void PeriodicThread::updateDataIfRequired()
	{
		FUNCTION_ENTRY("updateDataIfRequired");

		if(m_dataAccess->hasChanged())
		{
			readScheduleData();
		}

		FUNCTION_EXIT;
	}



	void PeriodicThread::readScheduleData()
	{
		FUNCTION_ENTRY("readScheduleData");

		try
		{
			std::vector<TA_IRS_Core::IMmSchedulingData*> data = m_dataAccess->getAllRecords(true);

			m_data.clear();

			for(unsigned int i = 0; i < data.size(); i++)
			{
				TA_IRS_Core::IConfigMmScheduling* pData = dynamic_cast<TA_IRS_Core::IConfigMmScheduling*>(data[i]);

				if (pData == 0)
				{
					LOG_GENERIC(SourceInfo, TA_Base_Core::DebugUtil::DebugError,	"dynamic cast failed on scheduling data" );

				}
				else
				{
					m_data.push_back(pData);
				}
			}

			int numRecords = m_data.size();

			LOG_GENERIC(SourceInfo, TA_Base_Core::DebugUtil::DebugInfo,
				"readScheduleData() stored %d records", numRecords);


		}
		catch(...)
		{
			LOG_GENERIC(SourceInfo, TA_Base_Core::DebugUtil::DebugError,
				"Failed to read MMS scheduling data from database");
		}

		FUNCTION_EXIT;
	}

    //TD16871 
    //Mintao++
    void PeriodicThread::processUpdate(const TA_Base_Core::ConfigUpdateDetails& updateEvent)
    {
		FUNCTION_ENTRY("processUpdate");
        //need to inform there are changes in the entity
        TA_Base_Bus::ScadaUtilities& scadaUtilities = TA_Base_Bus::ScadaUtilities::getInstance();
        scadaUtilities.processOnlineUpdateOfMaintenanceData();
		FUNCTION_EXIT;
    }

}

