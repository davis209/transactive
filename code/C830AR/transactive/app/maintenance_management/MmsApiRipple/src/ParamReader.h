/**
  * The source code in this file is the property of
  * Ripple Systems and is not for redistribution
  * in any form.
  *
  * Source:   $File: //depot/4669_T01271350/4669/transactive/app/maintenance_management/MmsApiRipple/src/ParamReader.h $
  * @author:  Ripple
  * @version: $Revision: #1 $
  *
  * Last modification: $DateTime: 2008/11/28 16:26:01 $
  * Last modified by:  $Author: builder $
  *
  */
// ParamReader.h: interface for the ParamReader class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_PARAMREADER_H__64AF2DB6_3F87_4809_B30A_D7889C348567__INCLUDED_)
#define AFX_PARAMREADER_H__64AF2DB6_3F87_4809_B30A_D7889C348567__INCLUDED_

#include <string>

namespace TA_Base_Core
{
	class MmsPeriodicEntityData;
	class MmsConnectiontEntityData;
};

class ParamReader  
{
public:
	void init();
	std::string toString(bool value);

	bool shouldFailPing();
	bool shouldFailJobRequest();
	bool shouldFailAlarmMessage();
	bool shouldFailGetMessage();
	bool shouldFailPeriodic();
	bool shouldFailGetPending();
	bool shouldFailExit();

	ParamReader();
	virtual ~ParamReader();

private:
	TA_Base_Core::MmsPeriodicEntityData* m_periodicData;
	TA_Base_Core::MmsConnectiontEntityData* m_connectionData;

//The Ping Period param in the connection entity is used to cause the
//stub to fail various calls depending on the params current value
	static const unsigned int FAIL_PING;
	static const unsigned int FAIL_JOB_REQUEST;
	static const unsigned int FAIL_ALARM_MESSAGE;
	static const unsigned int FAIL_GET_MESSAGE;
	static const unsigned int FAIL_PERIODIC_MESSAGE;
	static const unsigned int FAIL_GET_PENDING;
	static const unsigned int FAIL_EXIT;
	bool isInit;

};

#endif // !defined(AFX_PARAMREADER_H__64AF2DB6_3F87_4809_B30A_D7889C348567__INCLUDED_)
