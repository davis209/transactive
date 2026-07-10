/**
  * The source code in this file is the property of 
  * Ripple Systems and is not for redistribution
  * in any form.
  *
  * Source:   $File: //depot/4669_T01271350/4669/transactive/app/pa/PASimulatorLibrary/src/PACommsObserver.cpp $
  * @author:  Robert van Hugten
  * @version: $Revision: #1 $
  *
  * Last modification: $DateTime: 2008/11/28 16:26:01 $
  * Last modified by:  $Author: builder $
  * 
  * 
  * CommsObserver allows the CommsHandler to delegate incoming socket connections.
  * Each new connection is passed to a new CommsObserver object, a vector of which
  * is held by the WattsSim singleton object.
  */

#ifdef WIN32
#pragma warning (disable : 4786)
#endif // #ifdef WIN32

#include <iostream>
#include <algorithm>

#include "app/pa/PASimulatorLibrary/src/PACommsObserver.h"
#include "app/pa/PASimulatorLibrary/src/PASimUtility.h"

#include "core/utilities/src/DebugUtil.h"

using namespace TA_App;

//////////////////////////////////////////////////////////////////////////////
//            Construction/Destruction
//////////////////////////////////////////////////////////////////////////////

PACommsObserver::PACommsObserver(TA_Core::TcpObservedSocket< TA_Core::TcpNonblockingSocket > * socket)
:            m_clientSocket(socket),
            m_stillConnected(true),
            m_requestProcessor(0)
{
    m_requestProcessor = new PATableRequestProcessor( this );
    m_clientSocket->addObserver(* this);
    
    m_clientSocket->start();
}

PACommsObserver::~PACommsObserver()
{
    FUNCTION_ENTRY("~PACommsObserver");
    if( 0 != m_clientSocket )
    {
        //terminate the thread the socket is holding.
        m_clientSocket->terminate();
        delete m_clientSocket;
    }

    delete m_requestProcessor;
    FUNCTION_EXIT;
}



//////////////////////////////////////////////////////////////////////////////
//                                                                          //
//                            ITcpObserver methods                          //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////
/**
  * processReceivedData
  *
  * Parses the received stream of data into zero or more valid table requests
  * and adds them to the message processor's queue.
  *
  * @param pData the tcp data stream
  * @param pSocket the ISocket that received the data
  */
void PACommsObserver::processReceivedData( std::vector<unsigned char> & pData, TA_Core::ISocket * pSocket )
{
    LOG( SourceInfo, TA_Core::DebugUtil::GenericLog,
            TA_Core::DebugUtil::DebugDebug, "Data received" );

    bool keep_processing = true;
    std::vector<unsigned char>::iterator iter=pData.begin();

    std::vector<unsigned char>::iterator first=pData.end(); // will be set properly by the first valid packet
    std::vector<unsigned char>::iterator last=pData.begin(); // will be set properly by the last valid packet
    int dumped_chars = 0;

    //
    // it'd be nice to use std::find to get the first packet, however there
    // isn't any reliable stl-friendly way to make sure that if we have
    // multiple R and W requests that they'll be handled in the order they
    // arrived. 
    //
    // As it happens, for a W request we need to scorch through the stream
    // decyphering the packet for the table size anyhow, so we'll just sift
    // through it all byte by byte.
    //

    // checking iter against pData.end() isn't effective.
    // while ( ( iter <= pData.end() ) || ( keep_processing ) )
    while ( keep_processing )
    {
        if ( ( *iter == 'R' ) || ( *iter == 'W' ) )
        {
            // adjust the iterator pointing to the first element that we'll
            // eventually erase from.
            if ( first == pData.end() )
            {
                first = iter;
            }

            ISCSRequest * request = 0;

            if ( *iter == 'R' )
            {
                request = processReceivedRead(iter);
            }
            else if ( *iter == 'W' )
            {
                request = processReceivedWrite(iter);
            }
            else
            {
                std::cerr << "Received invalid data" << std::endl;
            }

            last = iter;

            if (request!=0)
            {
                // send the request to the processor
                m_requestProcessor->addMessageRequest(request);
            }
        }
        else
        {
            keep_processing = false;
        }

//         try
//         {
//             // char h[64];
//             // sprintf(h,"pData.erase(%p,%p)  std::copy(%p,%p,%p) pData.size()=%d pData.capacity()=%d pData.max_size()=%d",first,last,last,pData.end(),first,pData.size(),pData.capacity(),pData.max_size());
//             // std::cout << h << std::endl;
// 
//             // Look i don't know what the heck is going on here, but using
//             // erase() by iterator is stuffed.  Well alright that's what is
//             // going on, but i dunno why.  The underlying problem is that
//             // pData.end() eventually becomes equal to the begin(), even if
//             // items have been push_back()ed by the TcpSockeObserver, so that
//             // in erase(), the call to copy causes a loop which only ends when
//             // the iterators go to infinity and beyond, ie. Memory Violation
//             //
//             // pData.erase( first, last );
//             //
//             // The following code is my attempt to overcome this by using
//             // other stuff which should do essentially the same thing.
//         }
//         catch (...)
//         {
//             throw;
//         }
    } // while ( keep_processing )

    pData.clear();
}

ISCSRequest * PACommsObserver::processReceivedRead(std::vector<unsigned char>::iterator & iter)
{
    ISCSReadRequest * r = new ISCSReadRequest;
    r->fromNetwork(iter);

    return r;
}

ISCSRequest * PACommsObserver::processReceivedWrite(std::vector<unsigned char>::iterator & iter)
{
    ISCSWriteRequest * w = new ISCSWriteRequest;
    w->fromNetwork(iter);

    return w;
}


//
//writeFailed
//
void PACommsObserver::writeFailed()
{
        //should probably throw some sort of exception.
    LOG( SourceInfo, TA_Core::DebugUtil::GenericLog, TA_Core::DebugUtil::DebugDebug, "Write failed!" );
}


//
//connectionLost
//
bool PACommsObserver::connectionLost()
{
    m_stillConnected = false;
    // Do NOT call removeObserver. Its not threadsafe, and besides you don't
    // want to stop observing just because a connection is lost.
    // m_clientSocket->removeObserver ( * this );


    return true;
    //the TcpObservedSocket run() method checks this value to see 
    //if it should try to reconnect.
    //set the socket pointer to null.
}



//
//connectionEstablished
//
void PACommsObserver::connectionEstablished()
{
    m_stillConnected = true;
}


PATableRequestProcessor * PACommsObserver::getRequestProcessor()
{
    return m_requestProcessor;
}
