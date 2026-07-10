/**
  * The source code in this file is the property of 
  * Ripple Systems and is not for redistribution
  * in any form.
  *
  * Source:   $File: //depot/4669_T01271350/4669/transactive/app/pa/PASimulatorLibrary/src/PASimulatorCommandProcessor.cpp $
  * @author:  Robert van Hugten
  * @version: $Revision: #1 $
  *
  * Last modification: $DateTime: 2008/11/28 16:26:01 $
  * Last modified by:  $Author: builder $
  * 
  * 
  * PASimulator implements ISimulationModule, for use with the generic simulator.
  */

#include <iostream>
#include "app/pa/PASimulatorLibrary/src/PASimulatorCommandProcessor.h"

namespace TA_App
{

    std::string PASimulatorCommandProcessor::commands[NUM_COMMANDS] =
    {
        "showTable",
        "update",
        "set603",
        "set604",
        "set610",
        "set1610",
        "status"
    };

    std::string PASimulatorCommandProcessor::commandUsage[NUM_COMMANDS] =
    {
        "showTable <table number>",
        "update <table number>",
        "set603 <message#> <type> <label>",
        "set604 <message#> <type>",
        "set610 <private|common> <[1-6]>",
        "set1610 <private|common> <[1-6]> <station id>",
        "status"
    };


    //////////////////////////////////////////////////////////////////////
    // Construction/Destruction
    //////////////////////////////////////////////////////////////////////

    PASimulatorCommandProcessor::PASimulatorCommandProcessor(PASimulator* parentSimulator)
    :        m_parentSimulator(parentSimulator),
            m_errMsg("")
    {

    }

    PASimulatorCommandProcessor::~PASimulatorCommandProcessor()
    {

    }

        //////////////////////////////////////////////////////////////////////////////
        //                                                                            //
        //                    ISimulationCommandProcessor methods                        //
        //                                                                            //
        //////////////////////////////////////////////////////////////////////////////

    //
    //hasCommandName
    //
    bool PASimulatorCommandProcessor::hasCommandName( const std::string &name )
    {
        if (name == "help")
        {
            return true;
        }

        return hasCommand(name);
    }


    //
    //usageString
    //
    std::string PASimulatorCommandProcessor::usageString(const std::string &name)
    {
        return cmdUsage(name);
    }


    //
    //errorMsg
    //
    std::string PASimulatorCommandProcessor::errorMsg()
    {
        return m_errMsg;
    }


    //
    //execute
    //
    bool PASimulatorCommandProcessor::execute(const std::string &name, const std::string &params)
    {
        m_errMsg = "";

        if(name == "help" )
        {
            for (unsigned int i = 0; i < NUM_COMMANDS; i++)
            {
                std::cout << commandUsage[i] << std::endl;
            }
            return true;
        }

        else if(name == "showTable")
        {
            return m_parentSimulator->execShowTable( params );
        }
        
        else if (name == "set603")
        {
            return m_parentSimulator->execWriteTable(603, params );
        }
        
        else if (name == "set604")
        {
            return m_parentSimulator->execWriteTable(604, params );
        }
        
        else if (name == "set610")
        {
            return m_parentSimulator->execWriteTable(610, params );
        }
        
        else if (name == "set1610")
        {
            return m_parentSimulator->execWriteTable(1610, params );
        }
        
        else if(name == "update")
        {
            return m_parentSimulator->execUpdateTable( params );
        }
        
        else if(name == "status")
        {
            return m_parentSimulator->execStatus();
        }

        return true;
    }


        //////////////////////////////////////////////////////////////////////////////
        //                                                                            //
        //                PASimulatorCommandProcessor specific methods                //
        //                                                                            //
        //////////////////////////////////////////////////////////////////////////////


    //
    //hasCommand
    //
    bool PASimulatorCommandProcessor::hasCommand( const std::string &name )
    {
        for (unsigned int i = 0; i < NUM_COMMANDS; i++)
        {
            if (commands[i] == name)
            {
                return true;
            }
        }
        return false;
    }


    //
    //cmdUsage
    //
    std::string PASimulatorCommandProcessor::cmdUsage(const std::string& name)
    {
        for (unsigned int i = 0; i < NUM_COMMANDS; i++)
        {
            if (commands[i] == name)
            {
                return commandUsage[i];
            }
        }
        return "";
    }

}//close namespace
