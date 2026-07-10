/**
  * The source code in this file is the property of
  * Ripple Systems and is not for redistribution
  * in any form.
  *
  * Source:   $File: //depot/4669_T01271350/4669/transactive/app/maintenance_management/mms_agent/src/main.cpp $
  * @author:  christiandw
  * @version: $Revision: #1 $
  *
  * Last modification: $DateTime: 2008/11/28 16:26:01 $
  * Last modified by:  $Author: builder $
  *
  * This file contains the main that starts the MMS Agent application
  *
  */

#include "core/utilities/src/DebugUtil.h"
#include "core/utilities/src/RunParams.h"
#include "core/exceptions/src/GenericAgentException.h"
#include "core/exceptions/src/DatabaseException.h"
#include "core/exceptions/src/DataException.h"

#include "app/maintenance_management/mms_agent/src/MmsAgent.h"

using TA_Base_Core::DebugUtil;
using TA_Base_Core::RunParams;
using TA_Base_Core::GenericAgentException;

//--entity-name=OccMmsAgent --operation-mode=Control --mgr-port=6666 --debug-level=DEBUG --debug-file=c:/transactive/log/occMmsAgent.log

int main( int argc, char* argv[] )
{
	FUNCTION_ENTRY("main");

	TA_IRS_App::MmsAgent* agent = 0;

    try
    {		
		agent = new TA_IRS_App::MmsAgent(argc, argv);
		agent->run();
		delete agent;
    }
    catch( const GenericAgentException& gae )
    {
        LOG( SourceInfo, DebugUtil::ExceptionCatch,
             "GenericAgentException", gae.what());
		if(0 != agent) delete agent;
    }
    catch( const TA_Base_Core::DatabaseException& dbe )
    {
        LOG( SourceInfo, DebugUtil::ExceptionCatch,
             "DatabaseException", dbe.what());
		if(0 != agent) delete agent;
    }
    catch( const TA_Base_Core::DataException& de )
    {
        LOG( SourceInfo, DebugUtil::ExceptionCatch,
             "DataException", de.what());
		if(0 != agent) delete agent;
    }
	catch (std::exception e)
	{
        LOG( SourceInfo, DebugUtil::ExceptionCatch,
             "generic exception class", e.what());
		if(0 != agent) delete agent;
	}
	catch (...)
	{
        LOG( SourceInfo, DebugUtil::ExceptionCatch,
             "exception of unknown type caught");
		if(0 != agent) delete agent;
	}

    FUNCTION_EXIT;

	return 0;
}
