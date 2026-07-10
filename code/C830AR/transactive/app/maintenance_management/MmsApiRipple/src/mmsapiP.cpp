/**
  * The source code in this file is the property of
  * Ripple Systems and is not for redistribution in any form.
  * 
  * Source:    $File: //depot/4669_T01271350/4669/transactive/app/maintenance_management/MmsApiRipple/src/mmsapiP.cpp $
  * @author:   Chris DeWolfe
  * @version:  $Revision: #1 $
  * @see:      9999-Q01-02-0016-GenericAgent_Usage_Guidelines.doc
  *
  * Last modification: $DateTime: 2008/11/28 16:26:01 $
  * Last mofified by:  $Author: builder $
  *
  * This library provides an alternative implimentation of the functions
  * provided by the MMSAPI provided by STE
  *
  */
#if !defined(int32)
	#define int32 long //unsafe but required due to mmsapiP.h content
#endif
//#include "cots/MmsApi/Solaris8/include/mmsapiP.h"
#include <string.h>
#include <string>

#include "core/utilities/src/DebugUtil.h"
#include "core/utilities/src/TAAssert.h"

#include "ParamReader.h"

static ParamReader reader;

int printAlarmMessage(const char* msg)
{
	const std::string message = msg; 
	std::string messageWithLineBreaks = "152:[";
	messageWithLineBreaks += message.substr(0,152);
	messageWithLineBreaks += "] 40:[";
	messageWithLineBreaks += message.substr(152,40);
	messageWithLineBreaks += "]";
		LOG_GENERIC(SourceInfo, TA_Base_Core::DebugUtil::DebugInfo,
			"API put_Q got Alarm message with following fields:\n%s. Total chars: %d", messageWithLineBreaks.c_str(), strlen(msg));
	
	if(reader.shouldFailAlarmMessage())
	{
		LOG_GENERIC(SourceInfo, TA_Base_Core::DebugUtil::DebugInfo,
			"API is configured to fail this call, will now return 0");
		return 0;

	}
	else
	{
		return 1;
	}

}
int printCounterMessage(const char* msg)
{
	std::string message = msg; 
	std::string messageWithLineBreaks = "40:[";
	messageWithLineBreaks += message.substr(0,40);
	messageWithLineBreaks += "]19:[";

	messageWithLineBreaks += message.substr(40,19);
	messageWithLineBreaks += "]40:[";

	messageWithLineBreaks += message.substr(59, 40);
	messageWithLineBreaks += "]10:[";

	messageWithLineBreaks += message.substr(99,10);
	messageWithLineBreaks += "]";

		LOG_GENERIC(SourceInfo, TA_Base_Core::DebugUtil::DebugInfo,
			"API put_Q got Counter message with following fields:\n%s. Total chars: %d", messageWithLineBreaks.c_str(), strlen(msg));

	if(reader.shouldFailPeriodic())
	{
		LOG_GENERIC(SourceInfo, TA_Base_Core::DebugUtil::DebugInfo,
			"API is configured to fail this call, will now return 0");
		return 0;

	}
	else
	{
		return 1;
	}
}
int printMeasurementMessage(const char* msg)
{
	std::string message = msg; 
	std::string messageWithLineBreaks = "40:[";
	messageWithLineBreaks += message.substr(0,40);
	messageWithLineBreaks += "]19:[";

	messageWithLineBreaks += message.substr(40,19);
	messageWithLineBreaks += "]40:[";

	messageWithLineBreaks += message.substr(59, 40);
	messageWithLineBreaks += "]10:[";

	messageWithLineBreaks += message.substr(99,10);
	messageWithLineBreaks += "]";

		LOG_GENERIC(SourceInfo, TA_Base_Core::DebugUtil::DebugInfo,
			"API put_Q got Measurement message with following fields:\n%s. Total chars: %d", messageWithLineBreaks.c_str(), strlen(msg));

	if(reader.shouldFailPeriodic())
	{
		LOG_GENERIC(SourceInfo, TA_Base_Core::DebugUtil::DebugInfo,
			"API is configured to fail this call, will now return 0");
		return 0;

	}
	else
	{
		return 1;
	}
}
int printJobReqMessage(const char* msg)
{
	std::string message = msg; 
	std::string messageWithLineBreaks = "40:[";
	messageWithLineBreaks += message.substr(0,40);
	messageWithLineBreaks += "]20:[";

	messageWithLineBreaks += message.substr(40,20);
	messageWithLineBreaks += "]256:[";

	messageWithLineBreaks += message.substr(60, 256);
	messageWithLineBreaks += "]19:[";

	messageWithLineBreaks += message.substr(316, 19);
	messageWithLineBreaks += "]";

	LOG_GENERIC(SourceInfo, TA_Base_Core::DebugUtil::DebugInfo,
		"API put_Q got Job Request message with following fields:\n%s. Total chars: %d", messageWithLineBreaks.c_str(), strlen(msg));

	if(reader.shouldFailJobRequest())
	{
		LOG_GENERIC(SourceInfo, TA_Base_Core::DebugUtil::DebugInfo,
			"API is configured to fail this call, will now return 0");
		return 0;

	}
	else
	{
		return 1;
	}
}
int printTestMessage(const char* msg)
{
	std::string message = msg; 
	TA_ASSERT(message.length() == 0, "test message body is not empty!");
	
	bool result = reader.shouldFailPing();
	std::string str = reader.toString(result);

	LOG_GENERIC(SourceInfo, TA_Base_Core::DebugUtil::DebugInfo,
		"API put_Q got Test message. shouldFailPing() says to return %s", str.c_str());

	return result == true ? 0 : 1;
}

int put_Q(char *q_name, char *target_q_name, 
		  char *msg, char *msgType, short msg_len /*=0*/ )
{
	int result = 0;
	if(0 == strcmp("ALA", msgType) )
	{
		result = printAlarmMessage(msg);
	}
	if(0 == strcmp("CPT", msgType) )
	{
		result = printCounterMessage(msg);
	}
	if(0 == strcmp("VAL", msgType) )
	{
		result = printMeasurementMessage(msg);
	}
	if(0 == strcmp("REQ", msgType) )
	{
		result = printJobReqMessage(msg);
	}
	if(0 == strcmp("TST", msgType) )
	{
		result = printTestMessage(msg);
	}

	return result;
}

int get_Q(char *q_name, char * msg, short len, int32 itimeout)
{
	//to test pinging this should fail the first call and 
	//return a valid test ACK on subsequent calls
	static int numberCalls = 0;
	if(numberCalls < 2) numberCalls++;

	strcpy(msg, "ISCSMMS ACK12328/03/1961/15:30:45TST");

	int value = (numberCalls > 1) ? 1 : 0;

	if(reader.shouldFailGetMessage())
	{
		LOG_GENERIC(SourceInfo, TA_Base_Core::DebugUtil::DebugInfo,
			"API is configured to fail this call, will now return 0");
		return 0;

	}
	else
	{
		return 1;
	}


}

int getPendingMsgs(char *q_name)
{
	static int numberMessages = 5;
	numberMessages--;

	if(numberMessages < 0) numberMessages = 5;

	LOG_GENERIC(SourceInfo, TA_Base_Core::DebugUtil::DebugInfo,
		"API getPendingMsgs( ) called. Will return %d", numberMessages);

	if(reader.shouldFailGetPending())
	{
		LOG_GENERIC(SourceInfo, TA_Base_Core::DebugUtil::DebugInfo,
			"API is configured to fail this call, will now return -1");
		return -1;

	}
	else
	{
		return numberMessages;
	}

}

int exit_Q()
{

	LOG_GENERIC(SourceInfo, TA_Base_Core::DebugUtil::DebugInfo,
		"API exit_Q( ) called. Will return 1");

	if(reader.shouldFailExit())
	{
		LOG_GENERIC(SourceInfo, TA_Base_Core::DebugUtil::DebugInfo,
			"API is configured to fail this call, will now return 0");
		return 0;

	}
	else
	{
		return 1;
	}
}

char* getErrorMsg()
{
	LOG_GENERIC(SourceInfo, TA_Base_Core::DebugUtil::DebugInfo,
		"API getErrorMsg( ) called.");

	return "Ripple Stub's error message";
}

int32 getErrorCode()

{

	LOG_GENERIC(SourceInfo, TA_Base_Core::DebugUtil::DebugInfo,
		"API getErrorCode( ) called.");

	return 678;
}
