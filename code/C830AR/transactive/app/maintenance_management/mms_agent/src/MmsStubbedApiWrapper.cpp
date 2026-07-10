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
  * MmsApiWrapper class implimentation. See header file for class descrption
  */

#if defined(_MSC_VER)
#pragma warning(disable:4786)
#endif // defined (_MSC_VER)




#if !defined(int32)
	#define int32 long //unsafe but required due to mmsapiP.h content
#endif

//TODO note that if you recieve new versions of the api this path will change.
//Also dont forget to set the project settings so that they point at the 
//correct API library. There is a stub version library for testing, but the 
//release code must be built against the actual lib implimentation - both
//Win and Solaris builds
//Note that STE have provided a header file for Windows and another for Solaris. I
//am using the one specified below for both
//
// As of 04 Aug I am running against my version of the lib see:
//app\maintenance_management\MmsApiRipple\Debug\MmsApiRipple.lib
//#include "cots/MmsApi/Solaris8/include/mmsapiP.h"

#include "app/maintenance_management/mms_agent/src/MmsApiWrapper.h"

#include "core/utilities/src/TAAssert.h"


#include <algorithm>

int put_Q(char *q_name, char *target_q_name, char *msg, char *msgType, short msg_len=0 );
int get_Q(char *q_name, char * msg, short len, int32 itimeout);
int getPendingMsgs(char *q_name);
int exit_Q();
char* getErrorMsg();
int32 getErrorCode();

namespace TA_IRS_App
{
    //Mintao++
    //TD13606
	const unsigned int MmsApiWrapper::NUM_TRIES = 1;
    //Mintao++
    //TD13606

	//init static instance member
	MmsApiWrapper* MmsApiWrapper::m_instance = 0;
	TA_Base_Core::ReEntrantThreadLockable MmsApiWrapper::m_lock;

	const short MmsApiWrapper::BUFFER_SIZE = 12000;
	//const short MmsApiWrapper::BUFFER_SIZE = 32767;

	MmsApiWrapper* MmsApiWrapper::getInstance()
	{

		//Thread guard this method
		TA_Base_Core::ThreadGuard guard( m_lock );

		FUNCTION_ENTRY("getInstance");		

		if(0 == m_instance)
		{
			m_instance = new MmsApiWrapper();
		}


		FUNCTION_EXIT;

		return m_instance;
	}

	MmsApiWrapper::MmsApiWrapper()
		:m_timeOutTenths(10)
	{
		FUNCTION_ENTRY("MmsApiWrapper");		

		FUNCTION_EXIT;
	}


	MmsApiWrapper::~MmsApiWrapper()
	{
		FUNCTION_ENTRY("~MmsApiWrapper");

		if(0 != m_instance)
		{
			delete m_instance;
		}

		FUNCTION_EXIT;
	}
	//wrapper for original clearMessages method
	void MmsApiWrapper::clearMessages()
	{
		//Thread guard this method
		TA_Base_Core::ThreadGuard guard( m_lock );

		for(int i = 0; i < NUM_TRIES; i++)
		{
			try
			{
				clearMessagesNoRetry();
				break;
			}
			catch(MmsConnectionException& )
			{
				if(NUM_TRIES - 1 == i) throw;
			}
		}

		exitQ();

	}

	void MmsApiWrapper::clearMessagesNoRetry()
	{
		//Thread guard this method
		TA_Base_Core::ThreadGuard guard( m_lock );

		FUNCTION_ENTRY("clearMessages");
		
		int cnt = 0;
		//if there are any messages get em'
		while( 0 < getMessageCount() )
		{
			//get the message and throw it away
			getNextMessage();
			cnt++;
		}

		if( cnt > 0 )
		{
			LOG_GENERIC(SourceInfo, TA_Base_Core::DebugUtil::DebugInfo,
				"clearMessages() cleared %d messages.", cnt);

		}

		FUNCTION_EXIT;
	}

	//wrapper for original ping method
	void MmsApiWrapper::ping()
	{
		//Thread guard this method
		TA_Base_Core::ThreadGuard guard( m_lock );

		for(int i = 0; i < NUM_TRIES; i++)
		{
			try
			{
				pingNoRetry();
				break;
			}
			catch(MmsConnectionException& )
			{
				if(NUM_TRIES - 1 == i) throw;
			}
		}

		exitQ();

	}

	void MmsApiWrapper::pingNoRetry()
	{
		//Thread guard this method
		TA_Base_Core::ThreadGuard guard( m_lock );

		FUNCTION_ENTRY("ping");

		try //try and send a empty test message
		{
			int result = put_Q("ISCS", "MMS", "", "TST" );

			//if the put_Q failed get the error message and throw it 
			if( 1 != result )
			{
				std::string errorMsg = "getErrorMsg() or getErrorCode() threw";
				long errorCode = 0;
				try
				{
					errorCode = getErrorCode();
					errorMsg = getErrorMsg();
				}
				catch(...)
				{
					//do nothing, errorMsg has been initialized to report this failure
				}
				
				exitAndThrow( MmsConnectionException( errorMsg, errorCode) );
			}
		}
		catch(...) //the put_Q call threw so exit and throw
		{
			std::string errorMsg = "put_Q() API call threw an exception";
			long errorCode = 0;
			exitAndThrow( MmsConnectionException( errorMsg, errorCode) );

		}
		
		//if here the put_Q call succeded so try and get a responce. 
		//We do this in a loop because it is possible that we will receive one or more messages
		//before we get our responce. Note that the getNextMessage() call will throw if we stop getting messages
		//which is just what we want
		std::string message;
		do
		{
			 message = getNextMessage();

		} while (! isTestReponse(message) );

		//if you make it out of the loop alive we got a responce to our test
		FUNCTION_EXIT;

	}

	bool MmsApiWrapper::isTestReponse(const std::string& message)
	{
		FUNCTION_ENTRY("isTestReponse");

		//A test response should be 36 char long
		if(message.length() != 36)
		{
			LOG_GENERIC(SourceInfo, TA_Base_Core::DebugUtil::DebugWarn,
				"isTestReponse() called with (message.length() == %i, expecting 36, returning false.", message.length());

			FUNCTION_EXIT;
			return false;
		}

		//A test response should have "ACK" substring starting at char 8 (zero indexed)
		std::string substring = getMessageTypeStr(message);

		if("ACK" != substring)
		{
			LOG_GENERIC(SourceInfo, TA_Base_Core::DebugUtil::DebugWarn,
				"isTestReponse() called with message of correct length but not containing expected ACK sub-string at proper location.");

			FUNCTION_EXIT;
			return false;
		}
		//else we have a valid response
		LOG_GENERIC(SourceInfo, TA_Base_Core::DebugUtil::DebugDebug,
			"isTestReponse() returning true");

		return true;
		FUNCTION_EXIT;		
	}

	std::string MmsApiWrapper::getMessageTypeStr(const std::string& message)
	{
		FUNCTION_ENTRY("getMessageTypeStr");

		//if message is not at least as long as a message header something is amiss, so'
		//log and return an empty string 
		if(message.length() < 33)
		{
			LOG_GENERIC(SourceInfo, TA_Base_Core::DebugUtil::DebugError,
				"Message is not at least 33 chars long.");

			FUNCTION_EXIT;
			return "";
		}

		std::string substring(message, 8, 3);

		FUNCTION_EXIT;

		return substring;
	
	}

	//wrapper for original sendMessage method
	void MmsApiWrapper::sendMessage(const std::string& message, MmsApiWrapper::messageType type)
	{
		//Thread guard this method
		TA_Base_Core::ThreadGuard guard( m_lock );

		FUNCTION_ENTRY("sendMessage");

		for(int i = 0; i < NUM_TRIES; i++)
		{
			try
			{
				sendMessageNoRetry(message, type);
				break;
			}
			catch(MmsConnectionException& )
			{
				if(NUM_TRIES - 1 == i)
				{
					LOG_GENERIC(SourceInfo, TA_Base_Core::DebugUtil::DebugDebug,
						"sendMessageNoRetry() threw %d times. sendMessage() is now going to throw", NUM_TRIES);
					throw;
				}
			}
		}

		exitQ();

		FUNCTION_EXIT;

	}

	void MmsApiWrapper::sendMessageNoRetry(const std::string& message, MmsApiWrapper::messageType type)
	{
		//Thread guard this method
		TA_Base_Core::ThreadGuard guard( m_lock );

		FUNCTION_ENTRY("sendMessage");

		//convert the message type to the appropriate string
		std::string typeStr;
		switch(type)
		{
			case ALARM :
			{
				typeStr = "ALA";
				break;
			}
			case COUNTER :
			{
				typeStr = "CPT";
				break;
			}
			case MEASUREMENT :
			{
				typeStr = "VAL";
				break;
			}
			case JOB_REQUEST :
			{
				typeStr = "REQ";
				break;
			}
			case TEST :
			{
				typeStr = "TST";
				break;
			}

			default:
			{
				TA_ASSERT(false, "bad messageType param");
			}
		}
		
		try //try and send the message
		{
			//copy the message because the api wants a non-const char*
			std::string msgCopy = message;

			LOG_GENERIC(SourceInfo, TA_Base_Core::DebugUtil::DebugInfo,
				"the next call is put_Q(\"ISCS\", \"MMS\", <message>, %s ), where <message> is:\n%s", typeStr.c_str(), message.c_str());

			int result = put_Q("ISCS", "MMS", const_cast<char*>(msgCopy.c_str()), const_cast<char*>(typeStr.c_str()) );

			//if the call succeded return the message 
			if( 1 == result )
			{

				LOG_GENERIC(SourceInfo, TA_Base_Core::DebugUtil::DebugInfo,
					"successfully sent message of type %s",  typeStr.c_str());
				return;
				FUNCTION_EXIT;
			}
			//else the put_Q failed so get the error message and throw it

			std::string errorMsg = "getErrorMsg() or getErrorCode() threw";
			long errorCode = 0;
			try
			{
				errorCode = getErrorCode();
				errorMsg = getErrorMsg();
			}
			catch(...)
			{
				//do nothing, errorMsg has been initialized to report this failure
			}
			
			exitAndThrow( MmsConnectionException( errorMsg, errorCode) );
		}
		catch(MmsConnectionException& ex) 
		{
			//just rethrow this so that the following catch does not
			//overwrite the memebers of this exception
			throw ex;
		}
		catch(...) //the put_Q call threw so exit and throw
		{
			std::string errorMsg = "put_Q() API call threw an exception";
			long errorCode = 0;
			exitAndThrow( MmsConnectionException( errorMsg, errorCode) );

		}
	}

	unsigned int MmsApiWrapper::getMessageCount()
	{
		//Thread guard this method
		TA_Base_Core::ThreadGuard guard( m_lock );

		FUNCTION_ENTRY("getMessageCount");

		try
		{
			int result = getPendingMsgs("ISCS");

			//if the call succeded return the message 
			if( -1 != result )
			{
				LOG_GENERIC(SourceInfo, TA_Base_Core::DebugUtil::DebugDebug,
					"There are %i messages on the queue",  result);
				FUNCTION_EXIT;
				return result;
			}
			//else the getPendingMsgs failed so get the error message and throw it

			std::string errorMsg = "getErrorMsg() or getErrorCode() threw";
			long errorCode = 0;
			try
			{
				errorCode = getErrorCode();
				errorMsg = getErrorMsg();
			}
			catch(...)
			{
				//do nothing, errorMsg has been initialized to report this failure
			}
			
			exitAndThrow( MmsConnectionException( errorMsg, errorCode) );
		}
		catch(MmsConnectionException& ex) 
		{
			//just rethrow this so that the following catch does not
			//overwrite the memebers of this exception
			throw ex;
		}
		catch(...) //the getPendingMsgs call threw so exit and throw
		{
			std::string errorMsg = "getPendingMsgs() API call threw an exception";
			long errorCode = 0;
			exitAndThrow( MmsConnectionException( errorMsg, errorCode) );
		}
	}

	std::string MmsApiWrapper::getNextMessage()
	{
		//Thread guard this method
		TA_Base_Core::ThreadGuard guard( m_lock );

		FUNCTION_ENTRY("getNextMessage");

		try
		{
			char message[BUFFER_SIZE];
			int result = get_Q("ISCS", message, BUFFER_SIZE, m_timeOutTenths );

			//if the call succeded exitQ and return the message 
			if( 1 == result )
			{
				LOG_GENERIC(SourceInfo, TA_Base_Core::DebugUtil::DebugInfo,
					"getNextMessage() successfully retrieved a message of type \"%s\".", getMessageTypeStr(message).c_str() );
				FUNCTION_EXIT;
				return message;
			}
			//else the get_Q failed so get the error message and throw it

			std::string errorMsg = "getErrorMsg() or getErrorCode() threw";
			long errorCode = 0;
			try
			{
				errorCode = getErrorCode();
				errorMsg = getErrorMsg();
			}
			catch(...)
			{
				//do nothing, errorMsg has been initialized to report this failure
			}
			
			exitAndThrow( MmsConnectionException( errorMsg, errorCode) );

		}
		catch(MmsConnectionException& ex) 
		{
			//just rethrow this so that the following catch does not
			//overwrite the memebers of this exception
			throw ex;
		}
		catch(...) //the get_Q call threw so exit and throw
		{
			std::string errorMsg = "get_Q() API call threw an exception";
			long errorCode = 0;
			exitAndThrow( MmsConnectionException( errorMsg, errorCode) );
		}
	}

	void MmsApiWrapper::exitQ()
	{
		FUNCTION_ENTRY("exitQ");

		for(int i = 0; i < NUM_TRIES; i++)
		{
			try
			{
				exitQNoRetry();
				break;
			}
			catch(MmsConnectionException& )
			{
				if(NUM_TRIES - 1 == i)
				{
					LOG_GENERIC(SourceInfo, TA_Base_Core::DebugUtil::DebugDebug,
						"exitQNoRetry() threw %d times. sendMessage() is now going to throw", NUM_TRIES);
					throw;
				}
			}
		}

		FUNCTION_EXIT;
	}

	void MmsApiWrapper::exitQNoRetry()
	{
		try
		{
			//if successfull then return
			if(1 == exit_Q())
			{
				return;
			}
			//else the exit_Q() call failed so try and get the error code for logging
			long errorCode = 0;
			std::string errorMsg = "unknown";

			try
			{
				errorCode = getErrorCode();
				errorMsg = getErrorMsg();
			
				LOG_GENERIC(SourceInfo, TA_Base_Core::DebugUtil::DebugError,
					"exit_Q() call failed with getErrorMsg() = %s, getErrorCode() = %li", errorMsg.c_str(), errorCode);
				
			}
			catch(...) //getErrorMsg() or getErrorCode() threw
			{
				LOG_EXCEPTION_CATCH(SourceInfo, "Unknown API Exception", 
					"exit_Q() call failed ( return != 1 ) and getErrorMsg() or getErrorCode() threw");
			}

			throw MmsConnectionException( errorMsg, errorCode);
		}
		catch(MmsConnectionException&)
		{
			throw;
		}
		catch(...) //exit_Q() API call threw an exception
		{
			LOG_EXCEPTION_CATCH(SourceInfo, "Unknown API Exception", "exit_Q() API call threw an exception");
			throw MmsConnectionException( "unknown", 0 );
		}		
	}

#ifdef MMSAPI_STUB
	void MmsApiWrapper::exitAndThrow(MmsConnectionException ex)
	{
		LOG_GENERIC(SourceInfo, TA_Base_Core::DebugUtil::DebugError,
			"MMS Connection failed with getErrorMsg() = %s, getErrorCode() = %li.\n Application will now call exit_Q and throw MmsConnectionException.", ex.getErrorMessage().c_str(), ex.getErrorCode() );

		exitQ();
		throw ex;
	}
#endif

    bool MmsApiWrapper::tryTurnOffJournal()
    {
        return true;
    }

    bool MmsApiWrapper::isJournalOn()
    {
        return false;
    }

	//end IEntity methods
} //namespace TA_IRS_App



