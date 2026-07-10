#ifndef MMS_CONNECTION_TESTER_H
#define MMS_CONNECTION_TESTER_H

/**
  * The source code in this file is the property of 
  * Ripple Systems and is not for redistribution
  * in any form.
  *
  * Source:   $File$
  * @author:  C. DeWolfe
  * @version: $Revision$
  *
  * Last modification: $DateTime$
  * Last modified by:  $Author$
  * 
  * This class tests the MMS 
  */

#include "core/threads/src/Thread.h"
#include <string>
#include "core/synchronisation/src/ReEntrantThreadLockable.h"
#include "core/data_access_interface/entity_access/src/MmsConnectiontEntityData.h"
#include <ctime>

namespace TA_Base_Core
{
	class ConfigUpdateDetails;
}

namespace TA_IRS_App
{
	class MmsConnectionException;

	class MmsConnectionTester : public TA_Base_Core::Thread
	{
	public:

		MmsConnectionTester();

		virtual ~MmsConnectionTester();

		// Define a run() method to do your work. This should block until
		// terminate() is called eg: CorbaUtil::GetInstance().Run() or
		// m_dialog->DoModal()
		//
		// NOTE: do not call this method directly. It needs to be public so that it
		//       can be called from the thread-spawning function
		virtual void run();

		// The terminate() method should cause run() to return. eg:
		// CorbaUtil::GetInstance().Shutdown() or EndDialog(m_dialog)
		//
		// NOTE: probably not a good idea to call this method directly - call
		//       terminateAndWait() instead
		virtual void terminate();

        /**
          * initConfiguration
		  *
		  * Precondition: pData != 0 and has already been invalidated
		  * Will apply the data in the config to the object
		  *
		 */
		void initConfiguration(TA_Base_Core::MmsConnectiontEntityDataPtr pData);

		void setPingPeriodSec(unsigned int seconds);
		void setMsgCheckPeriodSec(unsigned int seconds);

	private: //data
	    volatile bool m_stopRequest;

    	//online config params
		unsigned int m_pingPeriodSec;
		unsigned int m_messageCheckPeriodMin;

		//last time the connection to the server was tested
		time_t m_lastPingTime; 
		time_t m_lastMessageCheckTime; 

	private: //methods

		
		/**
		*
		* checkForMessages
		*
		*The server sends us test messages from time to time. The MMS API returns
		*the appropriate responce message as soon as we call get_Q( ), so this method just clears
		*any waiting messages by calling get_Q: failures are reported to parent connection
		* If the check throws call MmsMessageUtil::getInstance()->raiseAlarm(ex)
		*/
		void checkForMessages();


		/**
		* connectionTest
		*
		*test the connection to the server if a test is due (m_pingPeriodSec)
		*failures are reported to parent connection
		* If the check throws call MmsMessageUtil::getInstance()->raiseAlarm(ex)
		*/
		void connectionTest();

        bool tryTurnOffJournal();

	};
} //end TA_App

#endif  // MMS_CONNECTION_TESTER_H
