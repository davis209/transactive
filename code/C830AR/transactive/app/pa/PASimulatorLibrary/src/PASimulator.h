#ifndef PA_SIMULATOR_H
#define PA_SIMULATOR_H

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

/**
  * The source code in this file is the property of 
  * Ripple Systems and is not for redistribution
  * in any form.
  *
  * Source:   $File: //depot/4669_T01271350/4669/transactive/app/pa/PASimulatorLibrary/src/PASimulator.h $
  * @author:  Robert van Hugten
  * @version: $Revision: #1 $
  *
  * Last modification: $DateTime: 2008/11/28 16:26:01 $
  * Last modified by:  $Author: builder $
  * 
  * Declaration file for the PASimulator class
  */

#include <vector>

#include "bus/generic_simulator/src/ISimulationModule.h"
#include "app/pa/PASimulatorLibrary/src/PATableRequestProcessor.h"
#include "app/pa/PASimulatorLibrary/src/PACommsObserver.h"

#include "core/threads/src/Thread.h"
#include "core/sockets/src/TcpSocket.h"
#include "core/sockets/src/TcpObservedSocket.h"
#include "core/sockets/src/ITcpSocketObserver.h"
#include "core/sockets/src/TcpServerSocket.h"
#include "core/sockets/src/TcpNonblockingSocket.h"


namespace TA_App
{

    /**
      * PASimulator implements ISimulationModule, for use with the generic simulator.
      */
    class PASimulator
    : public virtual TA_Core::Thread
    , public TA_Bus::ISimulationModule
    {

    public:

        //Construction & Destruction
        PASimulator(const std::string &name,const char* host,const char* port);
        virtual ~PASimulator();


        //////////////////////////////////////////////////////////////////////////////
        //                                                                            //
        //                                Thread methods                                //
        //                                                                            //
        //////////////////////////////////////////////////////////////////////////////

        /**
          * run
          * 
          * The run method must be implemented in any class that inherits from TA_Core::Thread.
            *
          */
        void run();


        /**
          * terminate
          * 
          * The terminate method must be implemented in any class that inherits from TA_Core::Thread.
          *
          */
        void terminate();


        //////////////////////////////////////////////////////////////////////////////
        //                                                                            //
        //                            ISimulationModule methods                        //
        //                                                                            //
        //////////////////////////////////////////////////////////////////////////////

        /**
          * startSim
          * 
          * startSim is called whenever the "start" command is issued by the simulater user.
          * Any initialisation that needs to be taken care of before the simulation starts 
          * for the first time is done in this method.
          *
          */
        void startSim();


        /**
          * stoptSim
          * 
          * stopSim is called whenever the "stop" command is issued by the simulater user.
          * Any processing that needs to be taken care of as the simulation finishes 
          * is done in this method.
          *
          */
        void stopSim();


        /**
          * restartSim
          * 
          * restartSim is called whenever the "start" command is issued by the simulater user.
          * Any initialisation that needs to be taken care of before the simulation restarts 
          * is done in this method.
          *
          */
        void restartSim();


        /**
          * destroySim
          * 
          * startSim is called whenever the "quit" command is issued by the simulater user.
          * Any processing that needs to be taken care of before the simulator is destroyed
          * should be completed in this method.
          *
          */
        void destroySim();    


        /**
          * getName
          * 
          * returns the name that the simulator module was instantiated with.
          *
          * @return std::string Name
          *
          */
        std::string getName();


        /**
          * getSimDescription
          * 
          * returns the description of the simulation module, for help messages.
          *
          * @return std::string simulatorDescription
          *
          */
        std::string getSimDescription();


        /**
          * getCommandProcessor
          * 
          * returns a pointer to the derived PASimulatorCommandProcessor
          *
          */
        TA_Bus::ISimulationCommandProcessor& getCommandProcessor();


        //////////////////////////////////////////////////////////////////////////////
        //                                                                          //
        //                                Execute methods                           //
        //                                                                          //
        //////////////////////////////////////////////////////////////////////////////

        /**
          * execShowTable
          * 
          * forms a request from the supplied string, and sends it.  This method
          * uses no error checking of the packet, so a response is never
          * guaranteed.  It does however allow the user to test responses to a
          * wide variety of packets
          *
          * @param params The parameters passed that were given with the command
          *
          * @return bool successful True if the command is executed sucessfully.
          *
          */
        bool execShowTable( const std::string& params );

        bool execWriteTable( unsigned short int tableNumber, const std::string& params );

        bool execUpdateTable( const std::string& params );
        /**
          * execStatus
          * 
          * outputs the current status of the VirtualVideoSwitch.  This method is 
          * executed when 
          *
          * @return bool successful True if the command is executed sucessfully.
          *
          */
        bool execStatus();

        /**
          * execSwitch
          * 
          * executes the switch command passed in by the user.  This command will
          * be reflected in the VirtualVideoSwitch, and hence all other attached 
          * agents
          *
          * @return bool successful True if the command is executed sucessfully.
          *
          */
//         bool execSwitch( const std::string& params );

        //////////////////////////////////////////////////////////////////////////////
        //                                                                          //
        //                        Simulator public methods                          //
        //                                                                          //
        //////////////////////////////////////////////////////////////////////////////

    

        
    private:

        //////////////////////////////////////////////////////////////////////////////
        //                                                                            //
        //                         Simulator private methods                            //
        //                                                                            //
        //////////////////////////////////////////////////////////////////////////////

        /**
          * getCommsObserverVector
          * 
          * returns the vector of CommsObserver objects held by this class.  Each CommsObserver
          * monitors a connected client socket.
          *
          * @return std::vector< CommsObserver* >& commsObservers The vector of CommsObserver objects
          *                    held by the Simulator
          *
          */
        std::vector< TA_App::PACommsObserver* >& getCommsObserverVector();


        /**
          * addCommsObserver
          * 
          * adds the passed in CommsObserver to the vector of CommsObserver objects held by the 
          * simulator
          *
          * @param CommsObserver* com A pointer to the CommsObserver object to be added
          *
          */
        void addCommsObserver( TA_App::PACommsObserver* com); 


        /**
          * removeCommsObserver
          * 
          * removes the passed in CommsObserver from the vector of CommsObserver objects held by the 
          * simulator
          *
          * @param com A pointer to the CommsObserver object to be removed
          *
          */
        void removeCommsObserver( TA_App::PACommsObserver* com );


        // The name of this SimulationModule
        std::string m_name;

        // The PASimulationCommandProcessor
        TA_Bus::ISimulationCommandProcessor *m_commandProcessor;

        volatile bool m_threadIsRunning;
        volatile bool m_doThreadTerminate;

        // indicates whether the socket is currently connected
        volatile bool m_isConnected;

        // The port used by the socket. Defaults to 27000 (see ISCS-COM SICD
        // Vol3 (PA) Appendix A A.1.1)
        std::string m_socketPort;

        // The host used by the socket. Defaults to "localhost"
        std::string m_socketHost;

        // a vector which holds an observed socket for each connection that
        // is accepted.
        std::vector< TA_App::PACommsObserver* > m_commsObservers;
        
        /**
          * There is a requestProcessor (ie. thread) for each commsObserver,
          * ie. for each socket that connects. By having an "active"
          * requestProcessor, the user can select which agent's tables
          * will be affected by their commands.
          */
        PATableRequestProcessor* m_activeRequestProcessor;

    };

}//close namespace

#endif // PA_SIMULATOR_H
