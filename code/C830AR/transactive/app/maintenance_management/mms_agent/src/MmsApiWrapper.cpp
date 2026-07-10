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

#include "bus/maintenance_management/mms_api/src/MmsApi.h"
#include "app/maintenance_management/mms_agent/src/MmsApiWrapper.h"
#include "core/utilities/src/TAAssert.h"
#include "core/utilities/src/RunParams.h"


#include <algorithm>

namespace TA_IRS_App
{
    using namespace TA_IRS_Bus;
    //Mintao++
    //TD13606
	const unsigned int MmsApiWrapper::NUM_TRIES = 1;
    //Mintao++
    //TD13606
	const double MmsApiWrapper::DEFAULT_PING_SECONDS = 1; //TD15841 Mintao++
    const int MmsApiWrapper::JORUNAL_FILE_ON_STATUS_CODE = 5;

	//init static instance member
	MmsApiWrapper* MmsApiWrapper::m_instance = 0;
	TA_Base_Core::ReEntrantThreadLockable MmsApiWrapper::m_lock;
	const short MmsApiWrapper::BUFFER_SIZE = 12000;

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


#ifndef MMSAPI_STUB
	void MmsApiWrapper::initConfiguration(TA_Base_Core::MmsAgentEntityDataPtr pData)
	{
 		FUNCTION_ENTRY("MmsApiWrapper");
		try
		{
			   //must set up the server address
			//get the server address from the entity
			std::string serverAddress = pData->getServerAddress();
			MmsApi::getInstance().updateServerAddress(serverAddress);
			LOG_GENERIC(SourceInfo, TA_Base_Core::DebugUtil::DebugDebug,
				"setting server address: %s.", serverAddress.c_str());
			//must set up the configuration
			//get it from the entity
			QueueConfig serverQueue;
			std::string svrname("MMS");
			serverQueue.name = svrname;
			serverQueue.queueid = pData->getServerQueueId();
			serverQueue.groupid = pData->getServerGroupId();
			MmsApi::getInstance().updateQueueConfig(serverQueue, true);
			LOG_GENERIC(SourceInfo, TA_Base_Core::DebugUtil::DebugDebug,
				"setting server name: %s, server queue id: %d, server group id: %d.", serverQueue.name.c_str(),serverQueue.queueid, serverQueue.groupid );
			
			QueueConfig clientQueue;
			std::string clientname("ISCS");
			clientQueue.name = clientname;
			clientQueue.queueid = pData->getClientQueueId();
			clientQueue.groupid = pData->getClientGroupId();
			MmsApi::getInstance().updateQueueConfig(clientQueue, false);
			LOG_GENERIC(SourceInfo, TA_Base_Core::DebugUtil::DebugDebug,
			"setting client name: %s, client queue id: %d, client group id: %d.", clientQueue.name.c_str(),clientQueue.queueid, clientQueue.groupid );
		}
		catch(...)
		{
			LOG_GENERIC(SourceInfo, TA_Base_Core::DebugUtil::DebugDebug,
				"sth wrong when trying to get the agent configuration in the ApiWrapper." );
		}
		FUNCTION_EXIT;
	}
#endif

	MmsApiWrapper::~MmsApiWrapper()
	{
		FUNCTION_ENTRY("~MmsApiWrapper");

		if(0 != m_instance)
		{
            MmsApi::removeInstance();
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
                // Just clear it, if success, break the retry loop.
				clearMessagesNoRetry();
                break;
// 				//now we check the server status again, if pinging server returns ok
// 				//we need to exit Q
// 				if(MmsApi::getInstance().pingServer(DEFAULT_PING_SECONDS) == true)
// 				{
// 					//before get Q, must check the journal status, if journal is on
// 					//that means the server was previously down, now back on line
// 					//need to exit Q first
// 					if (MmsApi::getInstance().isJournalOn() == true)
// 					{
// 						LOG_GENERIC(SourceInfo, TA_Base_Core::DebugUtil::DebugInfo,
// 							"server is now back on, exitQ now from clearMessage");
// 						exitQ();
// 				        throw MmsConnectionException( "ExitQ from the clearMessage!", 0 );
// 					}
// 					else//if journal is off, we can carry with our clear message
// 					{
// 						LOG_GENERIC(SourceInfo, TA_Base_Core::DebugUtil::DebugInfo,
// 							"Now try to get the message from the server queue!");
// 						clearMessagesNoRetry();
// 					}
// 				}
// 				else//if ping server false, that means server is down, throw exception to let the handler set server status to false
// 				{
//                     //If the agent is starts up or running control, try to get a message from the server.
//                     //This is a hack to facilitate the fail over of the MMS agent
// 					if((MmsApi::getInstance().isInitQ() == false) && (MmsApi::getInstance().isJournalOn() == false))
// 					{
// 						LOG_GENERIC(SourceInfo, TA_Base_Core::DebugUtil::DebugInfo,
// 							"just starting up or running control, now get message");
// 						clearMessagesNoRetry();
// 					}
// 					else
// 					{
// 						LOG_GENERIC(SourceInfo, TA_Base_Core::DebugUtil::DebugInfo,
// 							"server is not available, now stop get message!");
// 						throw MmsConnectionException("Sever is down!", 0);
// 					}
// 				}
			}
			catch(MmsConnectionException& ex )
			{
				throw ex;
			}
			catch(...)
			{
				LOG_GENERIC(SourceInfo, TA_Base_Core::DebugUtil::DebugInfo,
					"catch unknown exception at clear message!");
				throw MmsConnectionException("server is down", JORUNAL_FILE_ON_STATUS_CODE);
			}
		}

	}

	void MmsApiWrapper::clearMessagesNoRetry()
	{
		//Thread guard this method
		TA_Base_Core::ThreadGuard guard( m_lock );

		FUNCTION_ENTRY("clearMessagesNoRetry");
		try
		{
			getNextMessage();
		}
		catch(MmsConnectionException& ex )
		{
			LOG_GENERIC(SourceInfo, TA_Base_Core::DebugUtil::DebugDebug,
				"catch MmsConnectionException at clearMessagesNoRetry, now re-throw");
			throw ex;
		}
		catch(...)
		{
			LOG_GENERIC(SourceInfo, TA_Base_Core::DebugUtil::DebugInfo,
				"catch unknown exception at clearMessagesNoRetry!");
			throw MmsConnectionException("server is down", JORUNAL_FILE_ON_STATUS_CODE);
		}
		FUNCTION_EXIT;
	}

#ifndef MMSAPI_STUB
	bool MmsApiWrapper::pingServer()
    {
		FUNCTION_ENTRY("pingServer");
		FUNCTION_EXIT;
       return (MmsApi::getInstance().pingServer(DEFAULT_PING_SECONDS));
    }
#endif

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
			catch(MmsConnectionException& ex)
			{
				LOG_GENERIC(SourceInfo, TA_Base_Core::DebugUtil::DebugDebug,
					"get mmsconnection exception, now re-throw!");
				throw ex;
			}
		}

	}

	void MmsApiWrapper::pingNoRetry()
	{
		//Thread guard this method
		TA_Base_Core::ThreadGuard guard( m_lock );

		FUNCTION_ENTRY("ping");

		try //try and send a empty test message
		{
			int result = MmsApi::getInstance().put_Q("ISCS", "MMS", "", "TST", 0);			//if the put_Q failed get the error message and throw it 
			if( 1 != result )
			{
				std::string errorMsg = "getErrorMsg() or getErrorCode() threw";
				long errorCode = 0;
				try
				{
					errorCode = MmsApi::getInstance().getErrorCode();
					errorMsg = MmsApi::getInstance().getErrorMsg();
				}
				catch(...)
				{
					//do nothing, errorMsg has been initialized to report this failure
					//
				LOG_GENERIC(SourceInfo, TA_Base_Core::DebugUtil::DebugDebug,
					"get last error or error message got exception");
					throw MmsConnectionException("Get last Error code or message exception", 0);
				}
				
			}
		}
		catch(MmsConnectionException& ex) //the put_Q call threw so exit and throw
		{
			throw ex;
		}
		catch(...) //the put_Q call threw so exit and throw
		{
			std::string errorMsg = "put_Q() API call at ping threw an exception";
			long errorCode = 0;
			LOG_GENERIC(SourceInfo, TA_Base_Core::DebugUtil::DebugDebug,
				"put_Q() API call throw an exception");
			throw MmsConnectionException(errorMsg.c_str(), errorCode);
		}
		
		//if here the put_Q call succeded so try and get a responce. 
		//We do this in a loop because it is possible that we will receive one or more messages
		//before we get our responce. Note that the getNextMessage() call will throw if we stop getting messages
		//which is just what we want
		try //try and get an ack message
		{
		    std::string message;
		    do
		    {
			     message = getNextMessage();

		    } while ((!isTestReponse(message)) && (getMessageCount() != 0));
        }
		catch(MmsConnectionException& ex) //the put_Q call threw so exit and throw
		{
			throw ex;
		}
		catch(...) //the put_Q call threw so exit and throw
		{
			std::string errorMsg = "get_Q() API call at ping throw an exception at wrapper PING()";
			long errorCode = 0;
			LOG_GENERIC(SourceInfo, TA_Base_Core::DebugUtil::DebugDebug,
				"put_Q() API call throw an exception");
			throw MmsConnectionException(errorMsg.c_str(), errorCode);
		}
		
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
			catch(MmsConnectionException& ex)
			{
				if(NUM_TRIES - 1 == i)
				{
					LOG_GENERIC(SourceInfo, TA_Base_Core::DebugUtil::DebugDebug,
						"sendMessageNoRetry() throw %d times. sendMessage() is now going to throw", NUM_TRIES);
					throw ex;
				}
			}
		}

		FUNCTION_EXIT;

	}

	void MmsApiWrapper::sendMessageNoRetry(const std::string& message, MmsApiWrapper::messageType type)
	{
		FUNCTION_ENTRY("sendMessage");
		//Thread guard this method
		TA_Base_Core::ThreadGuard guard( m_lock );
		//before put Q, must check the journal status, if journal is on
		//that means the server is already down
// 		if (MmsApi::getInstance().isJournalOn() == true)
// 		{
// 			//now we check the server status again, if pinging server returns ok
// 			//we need to exit Q
// 			if(MmsApi::getInstance().pingServer(DEFAULT_PING_SECONDS) == true)
// 			{
// 				LOG_GENERIC(SourceInfo, TA_Base_Core::DebugUtil::DebugInfo,
// 					"server is now back on, exitQ now at sendMessage");
//                 //this will make sure we exit from the journal file
// 				exitQ();
// 			}
// 			else//if the server still down, do nothing
// 			{
// 				LOG_GENERIC(SourceInfo, TA_Base_Core::DebugUtil::DebugInfo,
// 					"server is not available, now going to put the message to the journal file");
// 			}
// 		}

        //TD15841
        //Mintao++
        //only send the job request when the server is on-line,i.e. the journal file is not on
        if (type == JOB_REQUEST /*|| type == ALARM*/ )	// TD18549 for alarm type message, no need to test server or journal file, just send and put_Q, BEA will do the recoveralbe
        {
            //to save some time, ping for 1 second should be enough
            if ( (MmsApi::getInstance().isJournalOn() == true) || (MmsApi::getInstance().pingServer(DEFAULT_PING_SECONDS) == false) )
            {
				throw MmsConnectionException("Journal is on Or server is offline, job request will not be sent!", JORUNAL_FILE_ON_STATUS_CODE);				
				LOG_GENERIC(SourceInfo, TA_Base_Core::DebugUtil::DebugInfo,
					"xxs test: MQ server is not available, so do not send the alarm message");
            }
        }
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
		
		std::string errorMsg = "getErrorMsg() or getErrorCode() threw";
		long errorCode = 0;
        int result = 0;
		try //try and send the message
		{
			//copy the message because the api wants a non-const char*
			std::string msgCopy = message;

			LOG_GENERIC(SourceInfo, TA_Base_Core::DebugUtil::DebugInfo,
				"the next call is put_Q(\"ISCS\", \"MMS\", <message>, %s ), where <message> is:\n%s", typeStr.c_str(), message.c_str());

			result = MmsApi::getInstance().put_Q("ISCS", "MMS", const_cast<char*>(msgCopy.c_str()), const_cast<char*>(typeStr.c_str()), msgCopy.length() );
        }
		catch(MmsConnectionException& ex) 
		{
			LOG_GENERIC(SourceInfo, TA_Base_Core::DebugUtil::DebugInfo,
				"throw at the send message with unexpect error: %s with code(%d)", errorMsg.c_str(), errorCode );
		}
		catch(...) //the put_Q call threw so exit and throw
		{
			std::string errorMsg = "put_Q() API call threw an exception";
			long errorCode = 0;
			LOG_GENERIC(SourceInfo, TA_Base_Core::DebugUtil::DebugDebug,
				"put_Q() API call throw an exception");
		}

        //if the call succeded return the message 
        if( 1 == result )
        {
            LOG_GENERIC(SourceInfo, TA_Base_Core::DebugUtil::DebugInfo,
                "successfully sent message of type %s",  typeStr.c_str());
            return;
            FUNCTION_EXIT;
        }
        //else the put_Q failed so get the error message and check whether it is put in the journal file
        else if(JORUNAL_FILE_ON_STATUS_CODE == result)
        {
            LOG_GENERIC(SourceInfo, TA_Base_Core::DebugUtil::DebugInfo,
                "journal is on, message should be at the journal");
            throw MmsConnectionException("journal is on!", JORUNAL_FILE_ON_STATUS_CODE);				
        }
        try
        {
            errorCode = MmsApi::getInstance().getErrorCode();
            errorMsg = MmsApi::getInstance().getErrorMsg();
        }
        catch(...)
        {
            LOG_GENERIC(SourceInfo, TA_Base_Core::DebugUtil::DebugDebug,
                "get last error or error message got exception");
            throw MmsConnectionException("Get last Error code or message exception", 0);
        }
        throw MmsConnectionException(errorMsg, errorCode);
	}

	unsigned int MmsApiWrapper::getMessageCount()
	{
		//Thread guard this method
		TA_Base_Core::ThreadGuard guard( m_lock );

		FUNCTION_ENTRY("getMessageCount");

		std::string errorMsg = "getErrorMsg() or getErrorCode() threw";
		long errorCode = 0;
        int result = 0;
		try
		{
			result = MmsApi::getInstance().getPendingMsgs("ISCS");

			//if the call succeded return the message 
			if( -1 != result )
			{
				LOG_GENERIC(SourceInfo, TA_Base_Core::DebugUtil::DebugDebug,
					"There are %d messages on the queue",  result);
				FUNCTION_EXIT;
				return result;
			}
			//else the getPendingMsgs failed so get the error message and throw it
			try
			{
				errorCode = MmsApi::getInstance().getErrorCode();
				errorMsg = MmsApi::getInstance().getErrorMsg();
			}
			catch(...)
			{
				//do nothing, errorMsg has been initialized to report this failure
				LOG_GENERIC(SourceInfo, TA_Base_Core::DebugUtil::DebugDebug,
					"get last error or error message got exception");
				throw MmsConnectionException("Get last Error code or message exception", 0);				
			}
			throw MmsConnectionException(errorMsg, errorCode);
		}
		catch(MmsConnectionException& ) 
		{
			//just rethrow this so that the following catch does not
			//overwrite the memebers of this exception
				LOG_GENERIC(SourceInfo, TA_Base_Core::DebugUtil::DebugDebug,
					"getMessageCount got error:%s(%d)", errorMsg.c_str(), errorCode);
			//currently do not try to re-throw the exception
			//throw ex;
		}
		catch(...) //the getPendingMsgs threw so throw
		{
			std::string errorMsg = "getPendingMsgs() API call throw an exception";
			long errorCode = 0;
			LOG_GENERIC(SourceInfo, TA_Base_Core::DebugUtil::DebugDebug,
				"getPendingMsgs() API call throw an exception");
			throw MmsConnectionException(errorMsg.c_str(), errorCode);
		}

        LOG_GENERIC(SourceInfo, TA_Base_Core::DebugUtil::DebugDebug,
			"There are %i messages on the queue",  result);
		FUNCTION_EXIT;
        return result;
	}

	std::string MmsApiWrapper::getNextMessage()
	{
		//Thread guard this method
		TA_Base_Core::ThreadGuard guard( m_lock );

		FUNCTION_ENTRY("getNextMessage");

		std::string errorMsg = "getErrorMsg() or getErrorCode() threw";
		long errorCode = 0;

		try
		{
			char message[BUFFER_SIZE];
			int result = MmsApi::getInstance().get_Q("ISCS", message, BUFFER_SIZE, m_timeOutTenths );

			//if the call succeded exitQ and return the message 
            if( MmsApi::BEA_TRUE == result )
			{
				LOG_GENERIC(SourceInfo, TA_Base_Core::DebugUtil::DebugInfo,
					"getNextMessage() successfully retrieved a message of type \"%s\".", getMessageTypeStr(message).c_str() );
				FUNCTION_EXIT;
				return message;
			}
			//else the get_Q failed so get the error message and throw it

			try
			{
				errorCode = MmsApi::getInstance().getErrorCode();
				errorMsg = MmsApi::getInstance().getErrorMsg();
			}
			catch(...)
			{
				//do nothing, errorMsg has been initialized to report this failure
				LOG_GENERIC(SourceInfo, TA_Base_Core::DebugUtil::DebugDebug,
					"get last error or error message got exception");
				throw MmsConnectionException("Get last Error code or message exception", 0);				
			}
			throw MmsConnectionException(errorMsg, errorCode);
		}
		catch(MmsConnectionException& ex) 
		{
			//just rethrow this so that the following catch does not
			//overwrite the memebers of this exception
			LOG_GENERIC(SourceInfo, TA_Base_Core::DebugUtil::DebugInfo,
				"getNextMessage got error:%s (%d)", errorMsg.c_str(), errorCode);
			throw ex;
		}
		catch(...) //the getPendingMsgs threw so throw
		{
			std::string errorMsg = "get_Q() API call throw an exception";
			long errorCode = 0;
			LOG_GENERIC(SourceInfo, TA_Base_Core::DebugUtil::DebugDebug,
				"get_Q() API call throw an exception");
			throw MmsConnectionException(errorMsg.c_str(), errorCode);
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
			catch(MmsConnectionException& ex)
			{
				if(NUM_TRIES - 1 == i)
				{
					LOG_GENERIC(SourceInfo, TA_Base_Core::DebugUtil::DebugDebug,
						"exitQNoRetry() throw exceptions %d times", NUM_TRIES);
					//throw ex;
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
            if(MmsApi::BEA_TRUE == MmsApi::getInstance().exit_Q())
			{
				return;
			}
			//else the exit_Q() call failed so try and get the error code for logging
			long errorCode = 0;
			std::string errorMsg = "unknown";

			try
			{
				errorCode = MmsApi::getInstance().getErrorCode();
				errorMsg = MmsApi::getInstance().getErrorMsg();
			
				LOG_GENERIC(SourceInfo, TA_Base_Core::DebugUtil::DebugError,
					"exit_Q() call failed with getErrorMsg() = %s, getErrorCode() = %li", errorMsg.c_str(), errorCode);
			}
			catch(...) //getErrorMsg() or getErrorCode() threw
			{
				LOG_EXCEPTION_CATCH(SourceInfo, "Unknown API Exception", 
					"exit_Q() call failed ( return != 1 ) and getErrorMsg() or getErrorCode() threw");
				throw MmsConnectionException( errorMsg, errorCode);
			}
			throw MmsConnectionException(errorMsg, errorCode);
		}
		catch(MmsConnectionException& ex)
		{
			throw ex;
		}
		catch(...) //exit_Q() API call threw an exception
		{
			LOG_EXCEPTION_CATCH(SourceInfo, "Unknown API Exception at exit_Q", "exit_Q() API call threw an exception");
			throw MmsConnectionException( "unknown", 0 );
		}		
	}

    /**
      * isConnectedQ
	  *
	  * determine whether connected the queue
      */
	bool MmsApiWrapper::isConnectedQ()
	{
		return MmsApi::getInstance().isInitQ();
	}

    bool MmsApiWrapper::tryTurnOffJournal()
    {
        bool bTurnSuccess = false;
        //now we check the server status again, if pinging server returns ok
        //we need to exit Q
        if(MmsApi::getInstance().pingServer(DEFAULT_PING_SECONDS))
        {
            LOG_GENERIC(SourceInfo, TA_Base_Core::DebugUtil::DebugInfo,
                "server is now back on, exitQ now at sendMessage");
            //this will make sure we exit from the journal file

            try
            {
                if(MmsApi::BEA_TRUE == MmsApi::getInstance().exit_Q())
                {
                    bTurnSuccess = true;
                }
                else
                {
                    LOG_GENERIC( SourceInfo, TA_Base_Core::DebugUtil::DebugWarn, 
                        "Server is connected but exit_Q failed" );
                }
            }
            catch (std::exception& expWhat)
            {
                LOG_EXCEPTION_CATCH( SourceInfo, "std::exception", expWhat.what() );
            }
            catch (...)
            {
                LOG_EXCEPTION_CATCH( SourceInfo, "Unknown", "Unknown" );
            }
		}

        return bTurnSuccess;
    }
    
    bool MmsApiWrapper::isJournalOn()
    {
        return MmsApi::getInstance().isJournalOn();
    }
} //namespace TA_IRS_App



