/**
  * The source code in this file is the property of
  * Ripple Systems and is not for redistribution in any form.
  * 
  * Source:    $File$
  * @author:   Chris DeWolfe
  * @version:  $Revision$
  * @see:      9999-Q01-02-0016-GenericAgent_Usage_Guidelines.doc
  *
  * Last modification: $DateTime$
  * Last mofified by:  $Author$
  *
  * This class provides a OO wrapper around the STE MMS API
  * 
  * The calls in the C style API return an int indicating success or failure.
  * If a call fails you can make additional calls to the API to get an
  * "error code" and "error message". The methods of this class encapsulate all
  * that by checking if a failure is indicated by the return value and if so,
  * throwing a MmsConnectionException that provides access to the retrieved error code
  * and error message.
  *
  * All calls to the API are tried NUM_TRIES times before a failure is reported
  *
  * Also, the C style API provides an exit_Q function. This class does not expose this
  * function beause it is called at the end of each method  call provided by this 
  * API regardless of success or failure of the function call.
  * If the exit_Q call fails we log an error with Debug util but do not throw, 
  * i.e. exit_Q calls fail silently.
  *
  * All std::string arguments in this API are assumed not to contain null characters.
  * The results are undefined if they do.
  *
  * This class is a singleton to ensure that two threads do not make the same calls at
  * the same time. All methods in this class are syncronised with each other, i.e. this
  * class is thread-safe.
  *
  * All methods asuume that we are "ISCS" and the server Q is "MMS"
  */
#ifndef MMS_API_WRAPPER_H
#define MMS_API_WRAPPER_H

#if defined(_MSC_VER)
#pragma warning(disable:4786)
#endif // defined (_MSC_VER)

#include <string>
#include "app/maintenance_management/mms_agent/src/MmsConnectionException.h"
#include "core/data_access_interface/entity_access/src/MmsAgentEntityData.h"
#include "core/synchronisation/src/Semaphore.h"
#include "core/synchronisation/src/ReEntrantThreadLockable.h"

namespace TA_Base_Core
{
	class MmsAgentEntityData;
}

namespace TA_IRS_App
{

    class MmsApiWrapper
    {
    public:

		//All calls to the API are tried NUM_TRIES times before a failure is reported
		static const unsigned int NUM_TRIES;

        /**
          * getInstance
		  *
		  * @return the only instance of this class
          */		
		static MmsApiWrapper* getInstance();

        /**
          * dtor
		  *
		  * deletes the only instance of this class
          */
		virtual ~MmsApiWrapper();

#ifndef MMSAPI_STUB
	    /**
	      * MmsApiWrapper::pingServer
	      * 
	      * using the actual ICMP datapacket to ping the server
	      * 
	      * @return bool 
	      * 
	      * @exception <exceptions> Optional
	      */
    	bool MmsApiWrapper::pingServer();
#endif

        bool tryTurnOffJournal();

        /**
          * ping
		  *
		  * Test if the MMS Server is contactable. 
		  * The method will wait getTimeout()/10 seconds for a responce before assuming failure.
		  *
		  * @throw MmsConnectionException if the MMS Server is contactable.
          */
		void ping();

		//see sendMessage()
		enum messageType {ALARM, COUNTER, MEASUREMENT, JOB_REQUEST, TEST}; 

        /**
          * sendMessage
		  *
		  * send a message to the MMS Queue
		  *
		  * @param message is the body of the message to send. Note that you do not
		  * provide the header - the API does that
		  *
		  * @param type is the type of message to send
		  * @throw MmsConnectionException if the call fails.
          */
		void sendMessage(const std::string& message, MmsApiWrapper::messageType type);


        /**
          * clearMessages
		  *
		  * if there are any messages in the queue waiting for you pop them and throw 
		  * them in the bin. If there are no messages waiting do nothing
		  *
		  * @throw MmsConnectionException if the call fails.
          */
		void clearMessages();


        /**
          * setTimeout
		  *
		  * set the the timeout time refered to in other method contracts. Note units
		  * are in 1/10 second. This value is defaulted to 10 ( 1 second ) when this object
		  * is created.
		  *
		  * @param tenths is the time in TENTHS OF SECONDS that getNextMessage will wait 
		  * for a message to arrive
          */
		void setTimeout(long tenths){m_timeOutTenths = tenths;}

        /**
          * exitQ
		  *
		  * wrapper around exit_Q API call. Does not throw on failure of the call
		  * just logs an error to debug util
          */
		void exitQ();

        /**
          * isConnectedQ
		  *
		  * determine whether connected the queue
          */
		bool isConnectedQ();

        bool isJournalOn();

#ifndef MMSAPI_STUB

	        /**
          * initConfiguration
		  *
		  *initialize the configuration required by the MmsAgent Api
          */
		void initConfiguration(TA_Base_Core::MmsAgentEntityDataPtr pData);

#endif

		
	private: //data members:

		static MmsApiWrapper* m_instance;

		//all public methods in this class are syncronised with each other
		static TA_Base_Core::ReEntrantThreadLockable m_lock;
		static const short BUFFER_SIZE;

		//the time (in 1/10 sec units) that get_Q will wait for a responce
		long m_timeOutTenths;

        //this is what we will get from the BEA client API when the journal file is on
        static const int JORUNAL_FILE_ON_STATUS_CODE;
	    static const double DEFAULT_PING_SECONDS;


	private: //methods:

        /**
          * The only implimnented CTOR
          */
        MmsApiWrapper();

        // Disable copy constructor and assignment operator
        MmsApiWrapper( const MmsApiWrapper& con);
        MmsApiWrapper& operator=(const MmsApiWrapper& con);

        /**
          * getTimeout
		  *
		  * @return the timeout time refered to in other method contracts. Note units
		  * are in 1/10 second
          */
		long getTimeout() const {return m_timeOutTenths;}

        /**
          * getMessageCount
		  *
		  * @return the number of messages waiting for you in the queue
		  * @throw MmsConnectionException if the call fails.
		  * Note that this method only tries the API call once
          */
		unsigned int getMessageCount();

#ifdef MMSAPI_STUB
        /**
          * exitAndThrow
		  *
		  * call exitQ then throw ex param
		  *
		  * @param ex is thrown after exitQ is called	
          */
		void exitAndThrow(MmsConnectionException ex);
#endif

        /**
          * isTestReponse
		  *
		  * @return true if message is a responce to a test message	
          */
		bool isTestReponse(const std::string& message);

        /**
		  *
		  * getMessageTypeStr
		  * return the "message type identifier" substring (char[8 - 10]) in the message
		  *
		  * if message.length() < 33 this will return a empty string and log an error to Debug util
          */
		std::string getMessageTypeStr(const std::string& message);

        /**
          * getNextMessage
		  * 
		  * get the mext message in the queue. Will wait getTimeout()/10 seconds
		  * before we assume the call has failed
		  * 
		  * @return the next message in the queue
		  * @throw MmsConnectionException if there is no message in the queue
		  *or the call fails.
		  * Note that this method only tries the API call once
          */
		std::string getNextMessage();

	//I have created several XXXNoRetry methods because we found out after this class was
	//created that all API calls should be called mulitiple times. Hence I renamed the
	//orginal methods to XXXNoRetry and created a wrapper method with the retry
	//refer to the comments of the public (wrapper) methods for what the following funcitons do
		void exitQNoRetry();
		void pingNoRetry();
		void sendMessageNoRetry(const std::string& message, MmsApiWrapper::messageType type);
		void clearMessagesNoRetry();

    };
} // namespace TA_IRS_App

#endif // !defined(MMS_API_WRAPPER_H)