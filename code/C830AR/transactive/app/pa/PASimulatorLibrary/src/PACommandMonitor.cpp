/**
  * The source code in this file is the property of 
  * Ripple Systems and is not for redistribution
  * in any form.
  *
  * Source:   $File: //depot/4669_T01271350/4669/transactive/app/pa/PASimulatorLibrary/src/PACommandMonitor.cpp $
  * @author:  Robert van Hugten
  * @version: $Revision: #1 $
  *
  * Last modification: $DateTime: 2008/11/28 16:26:01 $
  * Last modified by:  $Author: builder $
  * 
  * Implementation file for the PACommandMonitor class, derived from the
  * CommandMonitor class. CommandMonitor provides functionality to constantly
  * listen for user input from the command line, and to process that input
  * and execute the appropriate commands.
  *
  * PACommandMonitor extends CommandMonitor by overriding createSimulationModule
  * to return an instance of the PASimulator class.
  */

#include <iostream>


#include "app/pa/PASimulatorLibrary/src/PACommandMonitor.h"
#include "app/pa/PASimulatorLibrary/src/PASimulator.h"
#include "bus/generic_simulator/src/GenericSimulator.h"


using namespace TA_App;

/* ----------------------------------------------------------------------------
 * Construction/Destruction
 * --------------------------------------------------------------------------*/
PACommandMonitor::PACommandMonitor(const std::string &desc, TA_Bus::GenericSimulator *simulator)
 : CommandMonitor(desc,simulator)
{

}

PACommandMonitor::~PACommandMonitor()
{
}

TA_Bus::ISimulationModule* PACommandMonitor::createSimulationModule
(   const std::string& type,
    const std::string& name,
    const std::string& parameters)
{
    if (type=="PA")
    {
        char    host[100];
        char    port[100];

        // Get the host and port 
		// @todo the host and port should be optional, as they default
		//       to localhost and 27000 respectively
        int numTokens = sscanf(parameters.c_str(), "%s %s", host, port);
        if (numTokens==2)
        {
            return new TA_App::PASimulator(name,host,port);
        }
        else
        {
            std::cout << "Incorrect parameters for PA simulator module." << name << std::endl;
            std::cout << "addmod <name> PA <host> <port>" << std::endl;

        }
    }

    return 0;
}
