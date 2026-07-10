#ifndef MMS_PERIODIC_THREAD_H
#define MMS_PERIODIC_THREAD_H

/**
  * The source code in this file is the property of 
  * Ripple Systems and is not for redistribution
  * in any form.
  *
  * Source:   $File: //depot/4669_T01271350/4669/transactive/app/maintenance_management/mms_agent/src/PeriodicThread.h $
  * @author:  C. DeWolfe
  * @version: $Revision: #1 $
  *
  * Last modification: $DateTime: 2008/11/28 16:26:01 $
  * Last modified by:  $Author: builder $
  * 
  * This class checks to see if any periodic maintenance messages are due to be
  * sent and if so sends them
  */

#include "core/threads/src/Thread.h"
#include <vector>
#include "bus/scada/common_library/src/ScadaUtilities.h"
#include "core/data_access_interface/mms_dai/src/MmSchedulingAccessFactory.h"
#include "core/data_access_interface/mms_dai/src/IConfigMmScheduling.h"
#include "core/data_access_interface/entity_access/src/MmsPeriodicEntityData.h"

// Added by Hu Wenguang
#include "core/alarm/src/AlarmHelper.h"
// Hu Wenguang
//TD16871 
//Mintao++
#include "core/configuration_updates/src/IOnlineUpdatable.h"

namespace TA_Base_Core
{
	class MmsPeriodicEntityData;
	class ConfigUpdateMessageSender;
}

namespace TA_IRS_App
{
	class MmsConnectionException;

	class PeriodicThread : public TA_Base_Core::Thread, public virtual TA_Base_Core::IOnlineUpdatable
	{
	public:

		PeriodicThread();

		virtual ~PeriodicThread();

		/** Define a run() method to do your work. This should block until
		  * terminate() is called eg: CorbaUtil::GetInstance().Run() or
		  * m_dialog->DoModal()
		  *
		  * NOTE: do not call this method directly. It needs to be public so that it
		  *       can be called from the thread-spawning function
		 */
		virtual void run();

		/* The terminate() method should cause run() to return. eg:
		  * CorbaUtil::GetInstance().Shutdown() or EndDialog(m_dialog)
		  *
		  * NOTE: probably not a good idea to call this method directly - call
		  *       terminateAndWait() instead
		 */
		virtual void terminate();

        /**
          * initConfiguration
		  *
		  * Precondition: pData != 0 and has already been invalidated
		  * Will apply the data in the config to the object
		  *
		 */
		void initConfiguration(TA_Base_Core::MmsPeriodicEntityDataPtr pData);

		void setScanPeriodMinutes(unsigned int minutes);
		void setBacklogPeriodHours(unsigned int hours);

        //TD16871 
        //Mintao++
        /**
         * processUpdate
         *
         * When there is a configuration update of the type and key matching
         * one registered by this class, this method will be invoked
         * to process the update accordingly.
         */
        virtual void processUpdate(const TA_Base_Core::ConfigUpdateDetails& updateEvent);

	private: //data
		bool m_stopRequest;

    	//online config params
		unsigned int m_scanPeriodMinutes;
		unsigned int m_backCheckHours;

		struct scheduleRecord
		{
			//only the time portion of this is relavent
			time_t timeToSend;

			//only the date portion of this is relavent
			time_t lastSent;

			//physical subsystem key
			unsigned int subsystemKey;
		};
		std::vector<scheduleRecord> m_scheduling;

		std::vector<TA_Base_Bus::ScadaUtilities::MaintenanceData> m_dataFailures;

		TA_IRS_Core::MmSchedulingAccessFactory* m_dataAccess;
		std::vector<TA_IRS_Core::IConfigMmScheduling*> m_data;

        // Sends out config update notifications whenever a change
        // made to scheduling config
        TA_Base_Core::ConfigUpdateMessageSender* m_configUpdateSender;

        // Added by Wenguang Hu
        // Bug 635 (TD10892)
		/*/TD18093,jianghp
        TA_Base_Core::AlarmHelper& m_alarmHelper;
        // Bug 635 (TD10892)
        // Wenguang Hu


	private: //methods

        /**
          * updateDataIfRequired
		  *
		  * update m_data if the dai says it has changed sence last read
		 */
		void updateDataIfRequired();

        /**
          * getTodaysPeriods
		  *
		  * the "periods" returned are calculated as follows:
		  * the max period is 365 (days)
		  * we need all factors of today's number <= 365, as example
		  * if today is day 4 then this should return 1, 2, 4
		  * note that today is defined as the number of days from  
		  * January 1, 1970
		  *
		 */
		std::vector<unsigned int> getTodaysPeriods();
		
		/**
		*
		* updateObsoleteLastSent
		*
		* any last_sent dates older than the m_backCheckHours are set to now.
		* m_scheduling and db are updated
		* fails db write silently
		*/
		void updateObsoleteLastSent();


		/**
		* sendDueMessages
		*
		* Does the following:
		* - figure out what subsystems are due to be reported
		* - figure out what periods should be reported
		* - get the data for the relavent datapoints
		* - send a message to MMS for each
		* - record (locally and to db) the new last sent date for the subsystem
		*/
		void sendDueMessages();

		/**
		* readScheduleData
		*
		* reads the scheduling data from the database and updates m_scheduling if read is ok,
		* else it fails silently and does not modify m_scheduling;
		* 
		*/
		void readScheduleData();

		/**
		* sendMessage
		*
		* construct and send a periodic maintenance message to the MMS Server for 
		* the data specified. Failures are reported via the MmsConnection
		* call and are not visible via this API. 
		*
		*  Will call recordDataFailure()
		* if MaintenanceData::readFailed == true and send 0 as data value in message
		*
		* If call to MMS API fails you can't tell from this method but the failure
		* is recorded in the call
		*/
		void sendMessage(const TA_Base_Bus::ScadaUtilities::MaintenanceData& data);

		/**
		* recordDataFailure
		*
		* adds the data to the list of failures that is reported by reportDataFailures()
		*/
		void recordDataFailure(const TA_Base_Bus::ScadaUtilities::MaintenanceData& data);

		/**
		* reportDataFailures
		*
		* reports all failures recorded by recordDataFailure() since the last time
		* this was called
		*/
		void reportDataFailures(const TA_Base_Bus::ScadaUtilities::MaintenanceData& data);

		/**
		* recordSubsystemSent
		*
		* set last_sent to now in m_scheduling for all subsystems specified and
		* write the change to the db
		* db write fails silently - m_scheduling is updated regardless
		* 
		*/
		void recordSubsystemsSent(const std::vector<unsigned int>& subsystemKeys);

		/**
		* reportSubsystemsFailed
		*
		* raise ( or update ) an alarm indicating that we tried but failed to
		* report preventative maintenance info for the subsystems with the pkeys
		* specified in the 'subsystemKeys' parameter
		* 
		*/
		void reportSubsystemsFailed(const std::vector<unsigned int>& subsystemKeys);



		/**
		* getDueSubsystemKeys
		*
		* @return a key to all the subsytems that are due given the current time and the 
		* configuration
		* 
		*/
		std::vector<unsigned int> getDueSubsystemKeys();

		/**
		*
		* @return true if the subsystem is due if lastReported DAY is before TODAY and
		* report time is before or == current time
		*
		* @param now represents the current time
		*/
		bool subsystemIsDue(const time_t now, TA_IRS_Core::IMmSchedulingData * const data);

		//tell the config editor we have updated one or more LAST_SENT config values
		void submitConfigUpdate();

		//helper method for logging
		std::string maintenanceDataToString(const TA_Base_Bus::ScadaUtilities::MaintenanceData& data);

		static const CORBA::Double BAD_READ_VALUE;

		//chenlei++
		//maintain the Entitydata for initialization
		TA_Base_Core::MmsPeriodicEntityDataPtr m_periodicData;
		void initConfiguration();



	};
} //end TA_App

#endif  // MMS_PERIODIC_THREAD_H
