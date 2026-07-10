/**
  * The source code in this file is the property of
  * Ripple Systems and is not for redistribution
  * in any form.
  *
  * Source:   $File: //depot/4669_T01271350/4669/transactive/app/maintenance_management/MmsApiRipple/src/ParamReader.cpp $
  * @author:  Ripple
  * @version: $Revision: #1 $
  *
  * Last modification: $DateTime: 2008/11/28 16:26:01 $
  * Last modified by:  $Author: builder $
  *
  */
// ParamReader.cpp: implementation of the ParamReader class.
//
//////////////////////////////////////////////////////////////////////

#include "ParamReader.h"

#include "core/utilities/src/TAAssert.h"

#include "core/data_access_interface/entity_access/src/EntityAccessFactory.h"

#include "core/data_access_interface/entity_access/src/MmsAgentEntityData.h"

#include "core/data_access_interface/entity_access/src/IEntityData.h"

#include "core/data_access_interface/entity_access/src/MmsAlarmSubmitterEntityData.h"

#include "core/data_access_interface/entity_access/src/MmsJobRequestSubmitterEntityData.h"

#include "core/data_access_interface/entity_access/src/MmsPeriodicEntityData.h"

#include "core/data_access_interface/entity_access/src/MmsConnectiontEntityData.h"


//The Ping Period param in the connection entity is used to cause the
//stub to fail various calls depending on the params current value
const unsigned int ParamReader::FAIL_PING = 3;
const unsigned int ParamReader::FAIL_JOB_REQUEST = 8;
const unsigned int ParamReader::FAIL_ALARM_MESSAGE = 9;
const unsigned int ParamReader::FAIL_GET_MESSAGE = 10;
const unsigned int ParamReader::FAIL_PERIODIC_MESSAGE = 11;
const unsigned int ParamReader::FAIL_GET_PENDING = 12;
const unsigned int ParamReader::FAIL_EXIT = 13;

ParamReader::ParamReader()
	:isInit(false), m_periodicData(0), m_connectionData(0)
{

}

ParamReader::~ParamReader()
{

}

bool ParamReader::shouldFailPing()
{
	if(!isInit)
	{
		init();
	}

	m_connectionData->invalidate();

	if(FAIL_PING == m_connectionData->getPingPeriodSeconds())
	{
		return true;
	}

	return false;
}

bool ParamReader::shouldFailJobRequest()
{
	if(!isInit)
	{
		init();
	}

	m_connectionData->invalidate();

	if(FAIL_JOB_REQUEST == m_connectionData->getPingPeriodSeconds())
	{
		return true;
	}

	return false;
}

bool ParamReader::shouldFailAlarmMessage()
{
	if(!isInit)
	{
		init();
	}

	m_connectionData->invalidate();

	if(FAIL_ALARM_MESSAGE == m_connectionData->getPingPeriodSeconds())
	{
		return true;
	}

	return false;
}


bool ParamReader::shouldFailGetMessage()
{
	if(!isInit)
	{
		init();
	}

	m_connectionData->invalidate();

	if(FAIL_GET_MESSAGE == m_connectionData->getPingPeriodSeconds())
	{
		return true;
	}

	return false;
}

bool ParamReader::shouldFailPeriodic()
{
	if(!isInit)
	{
		init();
	}

	m_connectionData->invalidate();

	if(FAIL_PERIODIC_MESSAGE == m_connectionData->getPingPeriodSeconds())
	{
		return true;
	}

	return false;
}

bool ParamReader::shouldFailGetPending()
{
	if(!isInit)
	{
		init();
	}

	m_connectionData->invalidate();

	if(FAIL_GET_PENDING == m_connectionData->getPingPeriodSeconds())
	{
		return true;
	}

	return false;
}

bool ParamReader::shouldFailExit()
{
	if(!isInit)
	{
		init();
	}

	m_connectionData->invalidate();

	if(FAIL_EXIT == m_connectionData->getPingPeriodSeconds())
	{
		return true;
	}

	return false;
}

std::string ParamReader::toString(bool value)
{
	std::string	str = "true";
	if(false == value) str = "false";
	return str;
}

void ParamReader::init()
{
	

	TA_Base_Core::IEntityData * data = 0;
		
	// get TA_Base_Core::MmsPeriodicEntityData //////////////////////////
	data = TA_Base_Core::EntityAccessFactory::getInstance().getEntity("MmsPeriodicSubmitter_occ");

	m_periodicData = 
		dynamic_cast< TA_Base_Core::MmsPeriodicEntityData * > ( data );

	TA_ASSERT(0 != m_periodicData, "dynamic_cast failed");


	// get MmsConnectiontEntityData //////////////////////////
	data = TA_Base_Core::EntityAccessFactory::getInstance().getEntity("MmsConnection_occ");

	m_connectionData = 
		dynamic_cast< TA_Base_Core::MmsConnectiontEntityData * > ( data );

	TA_ASSERT(0 != m_connectionData, "dynamic_cast failed");

	isInit = true;

}
