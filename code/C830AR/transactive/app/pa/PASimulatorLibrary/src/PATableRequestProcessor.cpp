/**
  * The source code in this file is the property of
  * Ripple Systems and is not for redistribution
  * in any form.
  *
  * Source:   $File: //depot/4669_T01271350/4669/transactive/app/pa/PASimulatorLibrary/src/PATableRequestProcessor.cpp $
  * @author:  Robert van Hugten
  * @version: $Revision: #1 $
  *
  * Last modification: $DateTime: 2008/11/28 16:26:01 $
  * Last modified by:  $Author: builder $
  *
  *
  * PATableRequestProcessor is a thread which holds a queue of incoming PA Table
  * Request "packets".  The main method of the class, run(), loops checking the
  * queue for packets to be processed.
  */

#ifdef WIN32
#pragma warning (disable : 4786)
#endif // #ifdef WIN32

// #include <iostream>
// #include <sstream>
// #include <iomanip>
// #include <mbstring.h>
// #include <functional>

#include <string>
#include "app/pa/PASimulatorLibrary/src/PATableRequestProcessor.h"
#include "app/pa/PASimulatorLibrary/src/PACommsObserver.h"
#include "app/pa/PASimulatorLibrary/src/PASimTable.h"
#include "app/pa/PASimulatorLibrary/src/PASimUtility.h"
#include "core/synchronisation/src/ThreadGuard.h"

#include "app/pa/PASimulatorLibrary/src/PATable100.h"
#include "app/pa/PASimulatorLibrary/src/PATable200.h"
#include "app/pa/PASimulatorLibrary/src/PATable1200.h"
#include "app/pa/PASimulatorLibrary/src/PATable201.h"
#include "app/pa/PASimulatorLibrary/src/PATable1201.h"
#include "app/pa/PASimulatorLibrary/src/PATable202.h"
#include "app/pa/PASimulatorLibrary/src/PATable204.h"
#include "app/pa/PASimulatorLibrary/src/PATable1204.h"
#include "app/pa/PASimulatorLibrary/src/PATable301.h"
#include "app/pa/PASimulatorLibrary/src/PATable1301.h"
#include "app/pa/PASimulatorLibrary/src/PATable302.h"
#include "app/pa/PASimulatorLibrary/src/PATable303.h"
#include "app/pa/PASimulatorLibrary/src/PATable1303.h"
#include "app/pa/PASimulatorLibrary/src/PATable350.h"
#include "app/pa/PASimulatorLibrary/src/PATable550.h"
#include "app/pa/PASimulatorLibrary/src/PATable560.h"
#include "app/pa/PASimulatorLibrary/src/PATable570.h"
#include "app/pa/PASimulatorLibrary/src/PATable590.h"
#include "app/pa/PASimulatorLibrary/src/PATable603.h"
#include "app/pa/PASimulatorLibrary/src/PATable604.h"
#include "app/pa/PASimulatorLibrary/src/PATable610.h"
#include "app/pa/PASimulatorLibrary/src/PATable1610.h"
#include "app/pa/PASimulatorLibrary/src/PATable630.h"
#include "app/pa/PASimulatorLibrary/src/PATableInvalid.h"
//#include "app/pa/PASimulatorLibrary/src/PATable.h"
#include "core/utilities/src/DebugUtil.h"



//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

using namespace TA_App;

PATableRequestProcessor::PATableRequestProcessor( TA_App::PACommsObserver* parent )
: m_doThreadTerminate(false)
, m_threadIsRunning(false)
, m_parent(parent)
, m_localProcessor(false)
{
    FUNCTION_ENTRY("PATableRequestProcessor::PATableRequestProcessor");

    // FastPoll Tables
    m_table560        = new PATable560(this);
    PASimTable * t200 = new PATable200(this);
    PASimTable * t1200 = new PATable1200(this);
    PASimTable * t302 = new PATable302(this);

    // Table560 "Normal" Poll Tables
    PASimTable * t100 = new PATable100(this);
    PASimTable * t204 = new PATable204(this);
    PASimTable * t303 = new PATable303(this);
    PASimTable * t1303 = new PATable1303(this);
    PASimTable * t550 = new PATable550(this);
    PASimTable * t570 = new PATable570(this);
    PASimTable * t603 = new PATable603(this);
    PASimTable * t604 = new PATable604(this);
    PASimTable * t610 = new PATable610(this);
    PASimTable * t1610 = new PATable1610(this);

    // on-demand poll tables
    PASimTable * t201 = new PATable201(this);
    PASimTable * t1201 = new PATable1201(this);
    PASimTable * t202 = new PATable202(this);
    PASimTable * t1204 = new PATable1204(this);
    PASimTable * t301 = new PATable301(this);
    PASimTable * t1301 = new PATable1301(this);
    PASimTable * t350 = new PATable350(this);
    PASimTable * t590 = new PATable590(this);
    PASimTable * t630 = new PATable630(this);

    // used for table requests for tables we haven't implemented
    m_invalidTable = new PATableInvalid(this);

    //
    // Once all the tables that are going to exist do, "initialise" them.
    // Can't do this stuff during construction since the method called is an
    // overridden virtual, which hence ain't necessarily accessible at that time
    //
    for ( PASimTableMap::iterator iter = m_tables.begin() ;
          iter != m_tables.end() ; iter++ )
    {
        ((*iter).second)->setupDependents();
    }

    start();

    FUNCTION_EXIT;
}

PATableRequestProcessor::~PATableRequestProcessor()
{

    FUNCTION_ENTRY("~PATableRequestProcessor");
    m_doThreadTerminate = true;
    while( true == m_threadIsRunning )
    {
        sleep(100);
    }
    FUNCTION_EXIT;
    terminateAndWait();

    // Sadly, the following doesn't work on queue's
    // m_userRequests.erase(m_userRequests.begin(), m_userRequests.end());
    //
    // but this does...
    while (!m_userRequests.empty())
    {
        UserRequest * r = m_userRequests.front();
        delete r;
        m_userRequests.pop();
    }

    //
    // m_incomingRequests are also on a std::queue
    //
    // m_incomingRequests.erase(m_incomingRequests.begin(), m_incomingRequests.end());
    //
    // however it is a queue of ByteVector, which self-destructs
    while (!m_userRequests.empty())
    {
        m_userRequests.pop();
    }

    // 
    // the contents of m_tables are pointers, so we must destroy them
    // explicitly, also
    // 
    // m_tables.erase(m_tables.begin(), m_tables.end());
    //
    for ( PASimTableMap::iterator iter = m_tables.begin() ;
          iter != m_tables.end() ;
          iter++
        )
    {
        PASimTable * t = (*iter).second;
        delete t;
    }
    //
    // done implicitly during destruction
    //
    // m_tables.erase();
}


//////////////////////////////////////////////////////////////////////////////
//                                                                          //
//                       TA_Core::Thread methods                            //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////

//
//run
//
void PATableRequestProcessor::run()
{
    FUNCTION_ENTRY("PATableRequestProcessor::run");
    m_threadIsRunning = true;

    while( ! m_doThreadTerminate )
    {
//         if (! m_incomingRequests.empty() )
//         {
//             std::cout << "Number of incoming table requests:" << m_incomingRequests.size() << std::endl;
//         }

        while ( ! m_incomingRequests.empty() )
        {
            ISCSRequest * request = m_incomingRequests.front();

            // Check that we have a valid packet
            if( isValidRequest(request) )
            {
                if ( request->m_requestType == 'R' )
                {
                    TA_Core::ThreadGuard rg(m_tableLock);
                    PASimTableMap::iterator found = m_tables.find(request->m_tableNumber);
                    if ( found != m_tables.end() )
                    {
                        ((*found).second)->processReadRequest(request);
                        LOG(SourceInfo, TA_Core::DebugUtil::GenericLog, TA_Core::DebugUtil::DebugDebug,
                            "Processed Read Request for table %d",request->m_tableNumber);
//                         std::cout << "Processed Read Request for table " << request->m_tableNumber << std::endl;
                    }
                    else
                    {
                        ISCSReadReply * reply = new ISCSReadReply;
                        reply->m_tableNumber = request->m_tableNumber;
                        reply->m_tableSize = 0;
                        reply->m_tableData.resize(0);
                        sendReadReply(reply);
                        // we haven't yet properly implemented the table in question
                        // Don't process it with the invalid table, as that
                        // sends a nonsense packet reply.  Which is to say that
                        // the packet is well-formed, but the Agent cannot do
                        // anything with table#32222
                        // m_invalidTable->processReadRequest(request);
                        LOG(SourceInfo, TA_Core::DebugUtil::GenericLog, TA_Core::DebugUtil::DebugDebug,
                           "Invalid Read Request for table %d: does not exist",request->m_tableNumber);
                    }

                    delete request;
                }
                else if ( request->m_requestType == 'W' )
                {
                    TA_Core::ThreadGuard wg(m_tableLock);
                    PASimTableMap::iterator found = m_tables.find(request->m_tableNumber);
                    if ( found != m_tables.end() )
                    {
                        PASimTable * t = (*found).second;
                        t->processWriteRequest(request);
                        // ((*found).second)->processWriteRequest(w);
                        LOG(SourceInfo, TA_Core::DebugUtil::GenericLog, TA_Core::DebugUtil::DebugDebug,
                            "Processed Write Request for table %d",request->m_tableNumber);
//                         std::cout << "Processed Write Request for table " << request->m_tableNumber << std::endl;
                    }
                    else
                    {
                        ISCSWriteReply * reply = new ISCSWriteReply;
                        reply->m_tableNumber = request->m_tableNumber;
                        reply->m_returnValue = PAS_ERR_GENERIC;
                        sendWriteReply(reply);
                        // we haven't yet properly implemented the table in question
                        // Don't process it with the invalid table, as that
                        // sends a nonsense packet reply.  Which is to say that
                        // the packet is well-formed, but the Agent cannot do
                        // anything with table#32222
                        // m_invalidTable->processWriteRequest(request);
                        LOG(SourceInfo, TA_Core::DebugUtil::GenericLog, TA_Core::DebugUtil::DebugDebug,
                            "Invalid Write Request for table %d: does not exist",request->m_tableNumber);
                    }

                    delete request;
                }
            }

//             size_t sizebefore=m_incomingRequests.size();
            // Remove the packet from the queue.
            m_incomingRequests.pop();
//             size_t sizeafter=m_incomingRequests.size();
//             std::string isempty; PASimUtility::bool2string(m_incomingRequests.empty(),isempty);
//             std::cout << "POPPED - Size before=" << sizebefore << "   size after=" << sizeafter << "  EMTPY=" << isempty << std::endl;
            if ( !m_incomingRequests.empty() )
            {
                std::cerr << "EEEK!! request queue not empty after pop(), size=" << m_incomingRequests.size() << std::endl;
            }
        }

        while ( ! m_userRequests.empty() )
        {
            UserRequest * request = m_userRequests.front();

            // Check that we have a valid packet
            if( isValidUserRequest(request) )
            {
                if ( request->m_requestType == 'R' )
                {
                    TA_Core::ThreadGuard rg(m_tableLock);
                    PASimTableMap::iterator found = m_tables.find(request->m_tableNumber);
                    if ( found != m_tables.end() )
                    {
                        PASimTable * t = ((*found).second);
                        UserReply * r = t->processUserRead(request);
                        LOG(SourceInfo, TA_Core::DebugUtil::GenericLog, TA_Core::DebugUtil::DebugDebug,
                            "Processed User Read for table %d",request->m_tableNumber);
                        std::cout << r->m_replyData << std::endl;
                        delete r;
                    }
                    else
                    {
                        // we haven't yet properly implemented the table in question
                        m_invalidTable->processUserRead(request);
                        LOG(SourceInfo, TA_Core::DebugUtil::GenericLog, TA_Core::DebugUtil::DebugDebug,
                            "Invalid User Read for table %d: does not exist",request->m_tableNumber);
                        std::cout << "Invalid User Read for table " << request->m_tableNumber << ": does not exist" << std::endl;
                    }
                }
                else if ( request->m_requestType == 'W' )
                {
                    TA_Core::ThreadGuard wg(m_tableLock);
                    PASimTableMap::iterator found = m_tables.find(request->m_tableNumber);
                    if ( found != m_tables.end() )
                    {
                        PASimTable * t = (*found).second;
                        UserReply * r = t->processUserWrite(request);
                        // ((*found).second)->processUserWrite(w);
                        LOG(SourceInfo, TA_Core::DebugUtil::GenericLog, TA_Core::DebugUtil::DebugDebug,
                            "Processed User Write for table %d",request->m_tableNumber);
                        std::cout << r->m_replyData << std::endl;
                        delete r;
                    }
                    else
                    {
                        // we haven't yet properly implemented the table in question
                        m_invalidTable->processUserWrite(request);
                        LOG(SourceInfo, TA_Core::DebugUtil::GenericLog, TA_Core::DebugUtil::DebugDebug,
                            "Invalid User Write for table %d: does not exist",request->m_tableNumber);
                        std::cout << "Invalid User Write for table " << request->m_tableNumber << ": does not exist" << std::endl;
                    }
                }
                else if ( request->m_requestType == 'U' )
                {
                    TA_Core::ThreadGuard wg(m_tableLock);
                    PASimTableMap::iterator found = m_tables.find(request->m_tableNumber);
                    if ( found != m_tables.end() )
                    {
                        PASimTable * t = (*found).second;
                        UserReply * r = t->processUserWrite(request);
                        // ((*found).second)->processUserWrite(w);
                        LOG(SourceInfo, TA_Core::DebugUtil::GenericLog, TA_Core::DebugUtil::DebugDebug,
                            "Processed Update for table %d",request->m_tableNumber);
                        std::cout << r->m_replyData << std::endl;
                        delete r;
                    }
                    else
                    {
                        // we haven't yet properly implemented the table in question
                        m_invalidTable->processUserWrite(request);
                        LOG(SourceInfo, TA_Core::DebugUtil::GenericLog, TA_Core::DebugUtil::DebugDebug,
                            "Invalid Update for table %d: does not exist",request->m_tableNumber);
                        std::cout << "Invalid Update for table " << request->m_tableNumber << ": does not exist" << std::endl;
                    }
                }
            }

            // Remove the packet from the queue.
            m_userRequests.pop();
            // and delete it
            delete request;
        }
        sleep(10);
    }
    m_threadIsRunning = false;
    FUNCTION_EXIT;

}


//
//terminate
//
void PATableRequestProcessor::terminate()
{
    FUNCTION_ENTRY("PATableRequestProcessor::terminate");
    FUNCTION_EXIT;

}


//////////////////////////////////////////////////////////////////////////////
//                                                                          //
//                        PATableRequestProcessor methods                   //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////
void PATableRequestProcessor::addMessageRequest( ISCSRequest * request )
{
    m_incomingRequests.push(request);
    LOG_GENERIC(SourceInfo, TA_Core::DebugUtil::DebugDebug,
        "PATableRequestProcessor::addMessageRequest(%d) successful",request->m_tableNumber);
}

//
//addUserRequest
//
void PATableRequestProcessor::addUserRequest( UserRequest * request )
{
    // FUNCTION_ENTRY("PATableRequestProcessor::addUserRequest");
    m_userRequests.push( request );
    LOG_GENERIC(SourceInfo, TA_Core::DebugUtil::DebugDebug,
        "PATableRequestProcessor::addUserRequest(%d) successful",request->m_tableNumber);
    // FUNCTION_EXIT;
}

//
//isLocal
//
bool PATableRequestProcessor::isLocal()
{
    return m_localProcessor;
}

PASimTable * PATableRequestProcessor::getTable(int tNumber)
{
    PASimTable * t = 0;

    PASimTableMap::iterator fnd = m_tables.find(tNumber);
    if (fnd != m_tables.end())
    {
        t = (*fnd).second;
    }

    return t;
}

void PATableRequestProcessor::sendReadReply(ISCSReply * reply)
{
    ByteVector packet;
    reply->toNetwork(packet);
    sendPacket(packet);
}


void PATableRequestProcessor::sendWriteReply(ISCSReply * reply)
{
    ByteVector packet;
    reply->toNetwork(packet);
    sendPacket(packet);
}


bool PATableRequestProcessor::isValidRequest( const ISCSRequest * request )
{
    // FUNCTION_ENTRY("PATableRequestProcessor::isValidPacket");

    return true;

    bool packetOK = true;

    std::string truthstr;
    PASimUtility::bool2string(packetOK,truthstr,true);
    LOG(SourceInfo, TA_Core::DebugUtil::GenericLog, TA_Core::DebugUtil::DebugDebug,
        "packet ok: %s",truthstr.c_str());

    return packetOK;

    // FUNCTION_EXIT;
}

void PATableRequestProcessor::addTable(PASimTable * t)
{
    int tNum = t->getTableNumber();
    PASimTableMap::iterator fnd = m_tables.find(tNum);
    //
    // only add the table if it hasn't been already.
    if (fnd == m_tables.end())
    {
        m_tables.insert(PASimTableMap::value_type(tNum,t));
        // LOG(SourceInfo, TA_Core::DebugUtil::GenericLog, TA_Core::DebugUtil::DebugDebug,
        LOG_GENERIC(SourceInfo, TA_Core::DebugUtil::DebugDebug,
            "PATableRequestProcessor::addTable(%d) successful",t->getTableNumber());
    }
    else
    {
        PASimTable * x = (*fnd).second;
        //
        // if the new pointer doesn't point to the same object as the first,
        // dump the new one.
        if ( x != t )
        {
            delete t;
        }

        LOG(SourceInfo, TA_Core::DebugUtil::GenericLog, TA_Core::DebugUtil::DebugDebug,
            "PATableRequestProcessor::addTable(%d) Table already exists",t->getTableNumber());
    }
}

void PATableRequestProcessor::dependUponTable(PASimTable * t, int tableNumber)
{
    PASimTableMap::iterator fnd = m_tables.find(tableNumber);

    if (fnd != m_tables.end())
    {
        ((*fnd).second)->addDependent(t);
    }

}

bool PATableRequestProcessor::isValidUserRequest( const UserRequest * request )
{
    return true;
}

//////////////////////////////////////////////////////////////////////////////////////
//                                                                                  //
//                       Packet Processing & Sending Methods                        //
//                                                                                  //
//////////////////////////////////////////////////////////////////////////////////////


//
//sendPacket
//
void PATableRequestProcessor::sendPacket( const ByteVector & packet )
{
//     FUNCTION_ENTRY("SwitchRequestProcessor::sendPacket");

    //if this is a local processor
    if( !m_localProcessor )
    {
        //send the packet out on the socket it arrived on.
        m_parent->getSocket()->write( (char*)&(packet[0]), packet.size() );
    }
/*        else
    {
        //sendPacket.push_back('\0');

        std::string temp( sendPacket.size() - 6, ' ' );
        std::copy( sendPacket.begin() + 3, sendPacket.begin() + (sendPacket.size() - 3), temp.begin() );
        std::cout << "@XX" << temp << "XX\n" <<  std::endl;
    } */


//     FUNCTION_EXIT;
}


