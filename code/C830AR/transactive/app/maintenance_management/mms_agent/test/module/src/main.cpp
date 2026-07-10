/**
  * The source code in this file is the property of 
  * Ripple Systems and is not for redistribution
  * in any form.
  *
  * Source:   $File: //depot/4669_T01271350/4669/transactive/app/maintenance_management/mms_agent/test/module/src/main.cpp $
  * @author:  C. DeWolfe
  * @version: $Revision: #1 $
  *
  * Last modification: $DateTime: 2008/11/28 16:26:01 $
  * Last modified by:  $Author: builder $
  * 
  * This is the main program file for module testing of the MMS Agent 
  *
  */

#ifdef WIN32
#pragma warning(disable:4786)
#pragma warning(disable:4146)
#pragma warning(disable:4018)
#pragma warning(disable:4250)
#endif // WIN32

#include <iostream>

#include "core/utilities/src/DebugUtil.h"
#include "core/utilities/src/RunParams.h"

#include "MmsConnectionModuleTest.h"
#include "MmsJobRequestSubmitterTest.h"
#include "MmsAlarmSubmitterTest.h"
#include "Util.h"

using TA_Base_Core::DebugUtil;
using TA_Base_Core::RunParams;

void displayUsage(const char* appName)
{
	std::cout << std::endl
		  << "Usage: " << appName << " with the following parameters: "
		  << std::endl
		  << "\t--notify-hosts=<hostname(s)>" << std::endl
		  << "\t--db-connection=<user:pass:service>" << std::endl
		  << "\t--session-id=<your session id must be able to raise Job Requests>" << std::endl
		  << "\t--data-node=<entity name of datanode you will raise JR on>" << std::endl
		  << "\t--auto-alarm-uid=<UID of alarm that is configured MMS auto>" << std::endl
		  << "\t--auto-alarm-loc=<Location key of alarm that is configured MMS auto>" << std::endl
		  << "\t--semi-alarm-uid=<UID of alarm that is configured MMS semi>" << std::endl
		  << "\t--semi-alarm-loc=<Location key of alarm that is configured MMS semi>" << std::endl
		  << "where <hostname(s)> is a list of notification "
		  << "service hosts (optionally with ports)." << std::endl
		  << "eg:" << std::endl
		  << appName << " --NotifyHosts=jupiter:1234,jupiter:4321 --DbConnection=transact:its:its --DataNode=DBG.HV.22KV.HB201"
		  <<  " --AutoAlarmUid=4bc88046-2f18-4318-b1de-7051abf7f0d3 --AutoAlarmLoc=10 --SemiAlarmUid=5bc88046-2f18-4318-b1de-7051abf7f0d3 --SemiAlarmLoc=10"
		  << std::endl;

}

//--notify-hosts=scada1:6667,scada1:6668 --db-connection=scada:IT191_1:IT191_1 --session-id=--session-id=2e66a60a-3ae0-419f-b8c8-68c6aaec4100 --data-node=DBG.HV.22KV.HB201 --auto-alarm-uid=94c0b3d3-fc86-452c-a79a-a7e59744a1f2 --auto-alarm-loc=14 --semi-alarm-uid=8ac67011-a962-44c8-8030-e0f0ab1a1252 --semi-alarm-loc=14
int main( int argc, char **argv)
{
    // Set the debug information so we don't get the debug to standard output.
	DebugUtil::getInstance().setLevel( DebugUtil::DebugDebug );
    DebugUtil::getInstance().setFile( "./MMSAgentModuleTest.log" );
    DebugUtil::getInstance().setMaxFiles( 10 );

	// Get the parse parameters - used for some tests e.g. using Notification service 
    RunParams::getInstance().set(RPARAM_LOCATIONKEY, "1");


		// Added by Nous
	if(argc > 1)
	{
		RunParams::getInstance().parseCmdLine( argc, argv );
	}
	else
	{

		RunParams::getInstance().set(RPARAM_NOTIFYHOSTS, "localhost:1234,localhost:4321");
		RunParams::getInstance().set(RPARAM_DBCONNECTION, "tra_dev2:unittest:unittest");
		RunParams::getInstance().set(RPARAM_SESSIONID, "1");
		RunParams::getInstance().set(Util::DATA_NODE_ENTITY_NAME.c_str(), "DBG.HV.22KV.HB201");
		RunParams::getInstance().set(Util::AUTO_ALARM_UID.c_str(), "4bc88046-2f18-4318-b1de-7051abf7f0d3");
		RunParams::getInstance().set(Util::AUTO_ALARM_LOC.c_str(), "10");
		RunParams::getInstance().set(Util::SEMI_ALARM_UID.c_str(), "5bc88046-2f18-4318-b1de-7051abf7f0d3");
		RunParams::getInstance().set(Util::SEMI_ALARM_LOC.c_str(), "10");
	}


    //
	// Parse args for correctness
	//
	if ( ! RunParams::getInstance().isSet(RPARAM_NOTIFYHOSTS) ) 
	{
		std::cout << std::endl << "Notification Host not specified." << std::endl;
		displayUsage(argv[0]);
		return 0;
	}

	if ( ! RunParams::getInstance().isSet(RPARAM_DBCONNECTION) ) 
	{
		std::cout << std::endl << "Database connection not specified." << std::endl;
		displayUsage(argv[0]);
		return 0;
	}

	if ( ! RunParams::getInstance().isSet(RPARAM_SESSIONID) ) 
	{
		std::cout << std::endl << "Session not specified." << std::endl;
		displayUsage(argv[0]);
		return 0;
	}

	if ( ! RunParams::getInstance().isSet(Util::DATA_NODE_ENTITY_NAME.c_str()) ) 
	{
		std::cout << std::endl << "DataNode entity name not specified." << std::endl;
		displayUsage(argv[0]);
		return 0;
	}

	if ( ! RunParams::getInstance().isSet(Util::AUTO_ALARM_UID.c_str()) ) 
	{
		std::cout << std::endl << "UID of alarm that is configured MMS auto was not specified." << std::endl;
		displayUsage(argv[0]);
		return 0;
	}

	if ( ! RunParams::getInstance().isSet(Util::AUTO_ALARM_LOC.c_str()) ) 
	{
		std::cout << std::endl << "Location of alarm that is configured MMS auto was not specified." << std::endl;
		displayUsage(argv[0]);
		return 0;
	}

	if ( ! RunParams::getInstance().isSet(Util::SEMI_ALARM_UID.c_str()) ) 
	{
		std::cout << std::endl << "UID of alarm that is configured MMS semi was not specified." << std::endl;
		displayUsage(argv[0]);
		return 0;
	}

	if ( ! RunParams::getInstance().isSet(Util::SEMI_ALARM_LOC.c_str()) ) 
	{
		std::cout << std::endl << "Location of alarm that is configured MMS semi was not specified." << std::endl;
		displayUsage(argv[0]);
		return 0;
	}
    
    try
    {

        // Set up all the test classes.
        TestRunner runner;

        MmsConnectionModuleTest testcase1("MmsConnection Module Test");
        runner.addSuite( testcase1.suite() );      // Adds all test methods to the framework

	    MmsJobRequestSubmitterTest testcase2("MmsJobRequestSubmitter Module Test");
	    runner.addSuite( testcase2.suite() );

	    MmsAlarmSubmitterTest testcase3("MmsAlarmSubmitter Module Test");
	    runner.addSuite( testcase3.suite() );

        // Run config file. Make sure it is in the cfg directory.
        runner.begin( "./cfg/config.ini" );
    }
    catch ( TA_Base_Core::TransactiveException& ex )
    {
        std::cout << "Transactive Exception: " << ex.what() << std::endl;
    }
    catch (...)
    {
		std::cout << "Caught an unhandled exception" << std::endl;
	}

    return 0;
}


