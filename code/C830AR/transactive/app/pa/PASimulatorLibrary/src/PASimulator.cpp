
/**
  * The source code in this file is the property of 
  * Ripple Systems and is not for redistribution
  * in any form.
  *
  * Source:   $File: //depot/4669_T01271350/4669/transactive/app/pa/PASimulatorLibrary/src/PASimulator.cpp $
  * @author:  Robert van Hugten
  * @version: $Revision: #1 $
  *
  * Last modification: $DateTime: 2008/11/28 16:26:01 $
  * Last modified by:  $Author: builder $
  * 
  * 
  * Definition of the PASimulator class.
  */

#ifdef WIN32
#pragma warning (disable : 4786)
#endif // #ifdef WIN32

#include <iostream>
#include <sstream>
#include <iomanip>

#include "app/pa/PASimulatorLibrary/src/PASimulator.h"
#include "app/pa/PASimulatorLibrary/src/PATableRequestProcessor.h"
#include "app/pa/PASimulatorLibrary/src/PASimulatorCommandProcessor.h"
#include "app/pa/PASimulatorLibrary/src/PASimUtility.h"

#include "core/utilities/src/DebugUtil.h"
#include "core/utilities/src/RunParams.h"


using namespace TA_App;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

PASimulator::PASimulator(const std::string &name,const char* host,const char* port)
: m_name(name)
, m_threadIsRunning(false)
, m_doThreadTerminate(false)
, m_socketPort(port)
, m_socketHost(host)
, m_commandProcessor(0)
, m_activeRequestProcessor(0)

{
    m_commandProcessor = new PASimulatorCommandProcessor(this);
}

PASimulator::~PASimulator()
{
    stopSim();

}

//
// run()
//
// @todo something more useful in the catch(...) blocks
void PASimulator::run()
{

    FUNCTION_ENTRY("PASimulator::run");
    std::cout << "PA Simulator thread has started" << std::endl;

    // the run method is really the guts of it all
    m_threadIsRunning = true;

    TA_Core::TcpServerSocket<TA_Core::TcpObservedSocket<TA_Core::TcpNonblockingSocket> >* serverSocket = 0;
    try
    {
        serverSocket = new TA_Core::TcpServerSocket<TA_Core::TcpObservedSocket< TA_Core::TcpNonblockingSocket > >( m_socketHost, m_socketPort );
        serverSocket->bind();
        serverSocket->listen();
        std::cout   << "PA Simulator listening on "
                    << m_socketHost << ":" << m_socketPort << std::endl;
    }
    catch ( ... )
    {
        std::cerr << "PA Simulator listener thread caught exception during the construction of socket at address " << m_socketHost << ":" 
                  << m_socketPort << ", and will be terminated." << std::endl;
        m_doThreadTerminate = true;
    }

    // single-threaded connection-at-a-time
    while(!m_doThreadTerminate)
    {
        try
        {
            
            TA_Core::TcpObservedSocket< TA_Core::TcpNonblockingSocket >* acceptSocket = serverSocket->accept(TA_Core::SERVER_NON_BLOCKING_ACCEPT);

            
            if ( 0 != acceptSocket )
            {
        /*        TA_Core::TcpObservedSocket< TA_Core::TcpNonblockingSocket >* clientSocket = 
                    new TA_Core::TcpObservedSocket< TA_Core::TcpNonblockingSocket >(acceptSocket->getSocketId(), 
                                                        acceptSocket->getSocketHandle()); */


                m_isConnected = true;
            
                TA_App::PACommsObserver* com = new TA_App::PACommsObserver(acceptSocket);
                
                addCommsObserver(com);

                LOG( SourceInfo, TA_Core::DebugUtil::GenericLog, TA_Core::DebugUtil::DebugDebug, "New Connection accepted on socket." );
            }

            //we should check to see if all of our Observed Sockets are still connected.
            std::vector< TA_App::PACommsObserver * >::iterator it = m_commsObservers.begin();

            while(it != m_commsObservers.end())
            {
                if((*it)->stillConnected() == false)

                {
                    removeCommsObserver(*it);
                    delete *it;
                    //This removes the comms observer from the vector, and removes the socket
                    //from the vector.
                                        
                    it = m_commsObservers.begin();
                }
                else
                {
                    it++;
                }
            }
        }
        catch ( ... )
        {
            m_threadIsRunning = false;
            throw;
        }

        sleep(200);
    }

    m_threadIsRunning = false;

    FUNCTION_EXIT;

}

//
// terminate()
//
void PASimulator::terminate()
{
    LOG_GENERIC(SourceInfo, TA_Core::DebugUtil::DebugDebug, "PASimulator::terminate()");
}

//////////////////////////////////////////////////////////////////
//                                                                //
//                    ISimulationModule stuff                        //
//                                                                //
//////////////////////////////////////////////////////////////////

//
// startSim()
//
void PASimulator::startSim()
{
    m_doThreadTerminate = false;
    start();
    while( false == m_threadIsRunning )
    {
        sleep(100);
    }
}

//
// stopSim()
//
void PASimulator::stopSim()
{
    m_doThreadTerminate = true;
    
    while( m_threadIsRunning )
    {
        sleep(200);
    }

    std::cout << getSimDescription() << "=" << getName() << " stopped." << std::endl;
    //delete m_commandProcessor;

    // it should already be dead but this is a good cleanup
    terminateAndWait();
}

//
// restartSim()
//
void PASimulator::restartSim()
{
    stopSim();
    startSim();
}

//
// destroySim()
//
void PASimulator::destroySim()
{
    delete this;
}

//
// getName()
//
std::string PASimulator::getName()
{
    return m_name;
}

//
// getSimDescription()
//
std::string PASimulator::getSimDescription()
{
    return std::string( "PA simulator" );
}

//
// getCommandProcessor()
//
TA_Bus::ISimulationCommandProcessor& PASimulator::getCommandProcessor()
{
    return *m_commandProcessor;
}


//////////////////////////////////////////////////////////////////////////////
//                                                                          //
//                                Execute methods                           //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////


bool PASimulator::execUpdateTable( const std::string & params )
{
    //we assume that the entire parameter string is the one number

    if (m_activeRequestProcessor != 0)
    {
        UserRequest * request = new UserRequest;
        request->m_requestType='U';
        request->m_tableNumber=atoi(params.c_str());
        request->m_requestInfo="";

        m_activeRequestProcessor->addUserRequest(request);

        return true;
    }

    return false;
}


bool PASimulator::execShowTable( const std::string & params )
{
    //we assume that the entire parameter string is the one number

    if (m_activeRequestProcessor != 0)
    {
        UserRequest * request = new UserRequest;
        request->m_requestType='R';
        request->m_tableNumber=atoi(params.c_str());
        request->m_requestInfo="";

        m_activeRequestProcessor->addUserRequest(request);

        return true;
    }

    return false;
}


bool PASimulator::execWriteTable( unsigned short int tableNumber, const std::string & params )
{
    //we assume that the entire parameter string is the one number

    if (m_activeRequestProcessor != 0)
    {
        UserRequest * request = new UserRequest;
        request->m_requestType='W';
        request->m_tableNumber=tableNumber;
        request->m_requestInfo=params;

        m_activeRequestProcessor->addUserRequest(request);

        return true;
    }

    return false;
}


//
//execStatus
//
bool PASimulator::execStatus()
{
    int numConnections = m_commsObservers.size();
    std::cout << std::endl;
    std::cout << "PA Simulator Status. Number of Connections=" << numConnections << std::endl << std::endl;
    if (numConnections < 1) return true;

    std::cout << "+-----------+-----------+---------------------" << std::endl;
    std::cout << "| socket id | connected | client name         " << std::endl;
    std::cout << "+-----------+-----------+---------------------" << std::endl;
    std::vector< TA_App::PACommsObserver* >::iterator iter = m_commsObservers.begin();
    for ( ; iter != m_commsObservers.end() ; iter++ )
    {
        TA_Core::TcpObservedSocket<TA_Core::TcpNonblockingSocket> * socket = (*iter)->getSocket();
        char buffStr[16];
        sprintf(buffStr, "%.7d", socket->getSocketId());
        std::string socketIdStr=buffStr;
        std::string connectedStr;
        PASimUtility::bool2string(socket->stillConnected(), connectedStr);
        std::string cname = socket->getClientName();
        std::cout << "|  " << socketIdStr.c_str() << "  |   " << connectedStr.c_str() << "   | " << cname.c_str() << std::endl;

    }

    return true;

}

//
//execSwitch
//
// bool PASimulator::execSwitch( const std::string& params)
// {
// 
//     //we are only interested in the first word.  In fact, the first 5 characters.
//     std::string input = params.substr( 0, 5 );
// 
//     // the Date/Time display flag is the first param
//     int tmpIndex = params.find(" ", 0);
//     std::string output = params.substr( tmpIndex+1, 5 ); 
// 
//     tmpIndex = params.find(" ", tmpIndex+1);
//     std::string override = params.substr( tmpIndex+1, 1);
// 
//     std::string packet("RQIO");
//     packet += input;
//     packet += output;
//     packet += override;
// 
//     std::cout << "packet to be queued: " << packet << std::endl;
// 
//     execSendPacket( packet );
// 
//     return true;
// 
// }

//////////////////////////////////////////////////////////////////////////////
//                                                                          //
//                                Simulator methods                         //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////


//
//getCommsObserverVector
//
std::vector< PACommsObserver * >& PASimulator::getCommsObserverVector()
{ 
    return m_commsObservers; 
}

//
//addCommsObserver
//
// @param com the CommsObserver to add
//
void PASimulator::addCommsObserver(PACommsObserver* com)
{

    std::cout << "New ISCS connected to " << m_name << " simulator" << std::endl;
    LOG_GENERIC( SourceInfo, TA_Core::DebugUtil::DebugDebug, "New ISCS connected to %s simulator", m_name.c_str());
    m_commsObservers.push_back(com);

    m_activeRequestProcessor = com->getRequestProcessor();
}

//
// removeCommsObserver removes the given 
//
// @param com the CommsObserver to remove
//
void PASimulator::removeCommsObserver(PACommsObserver* com)
{
    std::cout << "ISCS disconnected from " << m_name << " simulator" << std::endl;
    LOG_GENERIC( SourceInfo, TA_Core::DebugUtil::DebugDebug, "ISCS disconnected from %s simulator", m_name.c_str());

    std::vector< PACommsObserver * >::iterator it = std::find(m_commsObservers.begin(), m_commsObservers.end(), com);
    if( it != m_commsObservers.end() )
    {
        //remove the CommsObserver
        m_commsObservers.erase(it);
    }

    //TODO if "com" was the active processor, replace it with another.
    if (com->getRequestProcessor() == m_activeRequestProcessor)
    {
        m_activeRequestProcessor=0;
    }
}


