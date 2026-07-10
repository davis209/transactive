/**
  * The source code in this file is the property of
  * Ripple Systems and is not for redistribution
  * in any form.
  *
  * Source:   $File: //depot/4669_T01271350/4669/transactive/app/pa/PASimulator/src/main.cpp $
  * @author:  Ripple
  * @version: $Revision: #1 $
  *
  * Last modification: $DateTime: 2008/11/28 16:26:01 $
  * Last modified by:  $Author: builder $
  *
  */
#ifdef WIN32
#pragma warning (disable : 4786)
#endif // #ifdef WIN32

//System Includes
#include <iostream>

//TransActive Includes
#include "core/utilities/src/DebugUtil.h"
#include "core/process_management/src/UtilityInitialiser.h"
#include "core/utilities/src/RunParams.h"

//Threads to be started
#include "bus/generic_simulator/src/GenericSimulator.h"
#include "app/pa/PASimulatorLibrary/src/PACommandMonitor.h"

using TA_Core::DebugUtil;
using TA_Core::UtilityInitialiser;

int main(int argc, char* argv[])
{

	if( true == (TA_Core::RunParams::getInstance().parseCmdLine(argc, argv)) )
	{
			
		DebugUtil::getInstance().setLevel(DebugUtil::DebugDebug);

		DebugUtil::getInstance().setFile("./PASimulator.log");

		//Check the command line for host/port info.  If there is no command, set defaults.
		if( false == TA_Core::RunParams::getInstance().isSet("Host") )
		{
			TA_Core::RunParams::getInstance().set("Host", "localhost");
		}

		if( false == TA_Core::RunParams::getInstance().isSet("Port") )
		{
			TA_Core::RunParams::getInstance().set("Port", "27000");
		}
	}
	TA_Core::RunParams::getInstance().set(RPARAM_DEBUGFILEMAXSIZE, "9999999999999999");
//	DebugUtil::getInstance().setLevel(DebugUtil::DebugDebug);
//	DebugUtil::getInstance().setFile( "debug.txt" );

	srand(0);		//seed the random number generator

	TA_Bus::GenericSimulator* simulator = new TA_Bus::GenericSimulator();

	TA_App::PACommandMonitor* monitor = new TA_App::PACommandMonitor("PA Simulator v0.1", simulator);
	monitor->monitorUserInput();

	//once the command monitor returns (user has typed exit), kill the simulator 
	std::cout << "Shutting down simulator..." << std::endl;
	delete simulator;
	std::cout << "done." << std::endl;

	return 0;
}

