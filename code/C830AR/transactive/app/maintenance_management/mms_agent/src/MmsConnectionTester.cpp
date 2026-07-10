
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
  * Implimentation of MmsConnectionTester
  *
  */

#pragma warning(disable:4503)

#include <ace/OS.h>

#include "app/maintenance_management/mms_agent/src/MmsConnectionTester.h"
#include "app/maintenance_management/mms_agent/src/MmsApiWrapper.h"
#include "app/maintenance_management/mms_agent/src/MmsMessageUtil.h"
#include "app/maintenance_management/mms_agent/src/MmsServerState.h"

#include "app/maintenance_management/mms_agent/src/MmsConnectionException.h"
#include "core/utilities/src/TAAssert.h"
#include "bus/maintenance_management/mmsCorbaDef/idl/src/IMmsConnectionCorbaDef.h"

namespace TA_IRS_App
{

	MmsConnectionTester::MmsConnectionTester()
		: m_pingPeriodSec(0), 
		  m_messageCheckPeriodMin(0), m_stopRequest(true)
	{
		FUNCTION_ENTRY("MmsConnectionTester");

		time(&m_lastPingTime);
		time(&m_lastMessageCheckTime);

		FUNCTION_EXIT;
	}



	MmsConnectionTester::~MmsConnectionTester()
	{
		FUNCTION_ENTRY("~MmsConnectionTester");

		FUNCTION_EXIT;
	}

	void MmsConnectionTester::run()
	{
		FUNCTION_ENTRY("checkForMessages");
		m_stopRequest = false;

		//chenlei++
		//init MmsServerState here
		//moved from MmsConnection::setToControlMode() function
		MmsServerState::getInstance()->initMmsServerState();

        long lClock = 0;
        long lElapse = 0;

        const unsigned int unTestSleepTime = 1000;
        const unsigned int unTurnOffInterval = 200;
        const unsigned int unContinue = unTestSleepTime - unTurnOffInterval;
        bool bJournalFailed = false;

		while(m_stopRequest == false)
        {
            bJournalFailed = false;

            if ( MmsApiWrapper::getInstance()->isJournalOn() )
            {
                if ( !tryTurnOffJournal() )
                {
                    bJournalFailed = true;

                    lElapse = ACE_OS::gettimeofday().msec() - lClock;
                    if (static_cast<unsigned int>(lElapse) < unTestSleepTime)
                    {
                        sleep(unTurnOffInterval);
                        if (static_cast<unsigned int>(lElapse) < unContinue)
                        {
                            LOG_GENERIC(SourceInfo, TA_Base_Core::DebugUtil::DebugInfo, "tryTurnOffJournal failed" );
                            continue;
                        }
                    }
                }
            }

            // AZ++ TES 4673
            try
            {
			    //see if any messages are waiting to be received
			    checkForMessages();

			    //test the connection to the server if time a test is due
			    connectionTest();
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

            lClock = ACE_OS::gettimeofday().msec();
			//have a one second nap
            if ( !bJournalFailed )
            {
			    sleep(unTestSleepTime);
            }

            LOG_GENERIC(SourceInfo, TA_Base_Core::DebugUtil::DebugInfo, "ConnectionTest done" );
		}

	}

	void MmsConnectionTester::setPingPeriodSec(unsigned int seconds)
	{
		FUNCTION_ENTRY("setPingPeriodSec");

		m_pingPeriodSec = seconds;

		if(0 == m_pingPeriodSec)
		{
			LOG_GENERIC(SourceInfo, TA_Base_Core::DebugUtil::DebugWarn,
				"Pinging MMS server is currently disabled");
		}
		else
		{
			LOG_GENERIC(SourceInfo, TA_Base_Core::DebugUtil::DebugInfo,
				"Now pinging the MMS Server every %ui seconds", seconds);
		}

		FUNCTION_EXIT;
	}

	void MmsConnectionTester::setMsgCheckPeriodSec(unsigned int seconds)
	{
		FUNCTION_ENTRY("setMsgCheckPeriodSec");

		m_messageCheckPeriodMin = seconds;

		if(0 == m_messageCheckPeriodMin)
		{
			LOG_GENERIC(SourceInfo, TA_Base_Core::DebugUtil::DebugWarn,
				"Checking for incomming messages is currently disabled");
		}
		else
		{
			LOG_GENERIC(SourceInfo, TA_Base_Core::DebugUtil::DebugInfo,
				"Now checking for incomming messages every %ui seconds", seconds);
		}

		FUNCTION_EXIT;
	}

	void MmsConnectionTester::connectionTest()
	{
		//removed this cause I am sick of the logging
		//FUNCTION_ENTRY("connectionTest");

		time_t now;
		time(&now);
		time_t diff = now - m_lastPingTime;

		//if a test of the server connection is due...
		if(diff > static_cast<long>(m_pingPeriodSec))
		{
			if(0 != m_pingPeriodSec)
			{
				try
				{
					LOG_GENERIC(SourceInfo, TA_Base_Core::DebugUtil::DebugInfo,
						"next call is MmsApiWrapper::ping()");

					MmsApiWrapper::getInstance()->ping();

					LOG_GENERIC(SourceInfo, TA_Base_Core::DebugUtil::DebugInfo,
						"Successfully pinged MMS Server");

					MmsServerState::getInstance()->reportMmsServerSucess();

				}
				catch(MmsConnectionException& ex)
				{
					TA_Base_Bus::MmsConnectionFailure failure(ex.getErrorMessage().c_str(), ex.getErrorCode());
                    MmsApiWrapper::getInstance()->exitQ();
                    MmsServerState::getInstance()->reportMmsServerFailure(failure);
				}
			}
			m_lastPingTime = now;
		}		

		//FUNCTION_EXIT;
	}

	void MmsConnectionTester::checkForMessages()
	{
		//removed this cause I am sick of the logging
		//FUNCTION_ENTRY("checkForMessages");

	
		time_t now;
		time(&now);
		time_t diff = now - m_lastMessageCheckTime;

		//if a test of the server connection is due...
		if(diff > static_cast<long>(m_messageCheckPeriodMin))
		{
			if(0 != m_messageCheckPeriodMin)
			{
				try
				{
					LOG_GENERIC(SourceInfo, TA_Base_Core::DebugUtil::DebugInfo,
						"next call is MmsApiWrapper::clearMessages()");

					MmsApiWrapper::getInstance()->clearMessages();

					LOG_GENERIC(SourceInfo, TA_Base_Core::DebugUtil::DebugInfo,
						"Successfully cleared messages from MMS Server");

					MmsServerState::getInstance()->reportMmsServerSucess();
				}
				catch(MmsConnectionException& ex)
				{
					TA_Base_Bus::MmsConnectionFailure failure(ex.getErrorMessage().c_str(), ex.getErrorCode());
                    MmsApiWrapper::getInstance()->exitQ();
                    MmsServerState::getInstance()->reportMmsServerFailure(failure);
				}
			}
			m_lastMessageCheckTime = now;
		}		

		//FUNCTION_EXIT;
	}

	void MmsConnectionTester::terminate()
	{
		FUNCTION_ENTRY("terminate");

		m_stopRequest = true;


		//TD18394 
		//xinsong++ teminate the MQ connection
#ifndef MMSAPI_STUB
		if ( MmsApiWrapper::getInstance()->isConnectedQ() )
		{
			MmsApiWrapper::getInstance()->exitQ();
		}
#endif

		FUNCTION_EXIT;
	}

	void MmsConnectionTester::initConfiguration(TA_Base_Core::MmsConnectiontEntityDataPtr pData)
	{
		FUNCTION_ENTRY("initConfiguration");

		setPingPeriodSec( pData->getPingPeriodSeconds() );
		setMsgCheckPeriodSec( pData->getMessageCheckSeconds() );

		FUNCTION_EXIT;
	}
    
    bool MmsConnectionTester::tryTurnOffJournal()
    {
        return MmsApiWrapper::getInstance()->tryTurnOffJournal();
    }
}

