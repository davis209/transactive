#ifndef PA_COMMS_OBSERVER_H
#define PA_COMMS_OBSERVER_H
/**
  * The source code in this file is the property of 
  * Ripple Systems and is not for redistribution
  * in any form.
  *
  * Source:   $File: //depot/4669_T01271350/4669/transactive/app/pa/PASimulatorLibrary/src/PACommsObserver.h $
  * @author:  Robert van Hugten
  * @version: $Revision: #1 $
  *
  * Last modification: $DateTime: 2008/11/28 16:26:01 $
  * Last modified by:  $Author: builder $
  * 
  */

#ifdef WIN32
#pragma warning (disable : 4786)
#endif // #ifdef WIN32

#include "app/pa/PASimulatorLibrary/src/PATableRequestProcessor.h"

#include "core/sockets/src/TcpObservedSocket.h"
#include "core/sockets/src/ITcpSocketObserver.h"
#include "core/sockets/src/TcpServerSocket.h"
#include "core/sockets/src/TcpNonblockingSocket.h"

namespace TA_App
{

    /**
      * CommsObserver holds a member observed socket, and acts as an interface for objects
      * to access that socket.
      */
    class PACommsObserver : public virtual TA_Core::ITcpSocketObserver
    {
    public:

        //Construction & Destruction
        
        //we have no need for a default constructor.
        PACommsObserver(TA_Core::TcpObservedSocket< TA_Core::TcpNonblockingSocket > * socket);
        PACommsObserver();
        
        virtual ~PACommsObserver();
        
        
        //////////////////////////////////////////////////////////////////////////////
        //                                                                            //
        //                            ITcpObserver methods                            //
        //                                                                            //
        //////////////////////////////////////////////////////////////////////////////


        /**
          * processReceivedData
          * 
          * This method must be implemented to inherit ITcpSocketObserver.  Any time a 
          * packet is received on the specified socket, this method is called.
          *
          * @param pData the data contained in the received packet
          *
          * @param TA_Core::ISocket* pSocket a pointer to the socket the packet was received on
          *
          */
        void processReceivedData( std::vector<unsigned  char> & pData, TA_Core::ISocket * pSocket );

        ISCSRequest * processReceivedRead(std::vector<unsigned char>::iterator & iter);

        ISCSRequest * processReceivedWrite(std::vector<unsigned char>::iterator & iter);


        /**
          * writeFailed
          * 
          * This method must be implemented to inherit ITcpSocketObserver.  Any time a 
          * request to write to the observed socket fails, this method is called.  It allows
          * the user to include any cleanup they require after a failed write.
          *
          */
        void writeFailed();


        /**
          * connectionLost
          * 
          * This method must be implemented to inherit ITcpSocketObserver.  This method is called
          * whenever the observed socket loses it's connection to the remote socket.
          *
          * @return bool continueToObserve returns true if the socket should continue to be observed
          *            and returns false if the socket should no longer be observed.
          *
          */
        bool connectionLost();


        /**
          * connectionEstablished
          *
          * This method must be implemented to inherit ITcpSocketObserver. it is called after a
          * connection is successfully established.
          *
          */
        void connectionEstablished();


        //////////////////////////////////////////////////////////////////////////////
        //                                                                            //
        //                            CommsObserver methods                            //
        //                                                                            //
        //////////////////////////////////////////////////////////////////////////////

        
        /**
          * getSocket
          *
          * returns a pointer to the socket being observed.
          *
          * @return TA_Core::TcpObservedSocket<TA_Core::TcpNonblockingSocket> socket the observed socket.
          *
          */
        TA_Core::TcpObservedSocket<TA_Core::TcpNonblockingSocket>* getSocket() { return m_clientSocket; }


        /**
          * stillConnected
          *
          * indicates if the observed socket is still connected to it's remote host.
          *
          * @return bool true if the socket is still connected, false otherwise.
          *
          */
        bool stillConnected() { return m_stillConnected; }


        /** accessor for the object that is processing requests for this comms
          * observer
          *
          * @return the value of the data member m_requestProcessor
          */
        PATableRequestProcessor * getRequestProcessor();


    private:
        // the observed socket
        TA_Core::TcpObservedSocket<TA_Core::TcpNonblockingSocket>* m_clientSocket;

        // indicates if the observed socket is still connected.    
        bool m_stillConnected;

        // Request processor to handle incoming packets.    
        PATableRequestProcessor * m_requestProcessor;
    };

}//close namespace
#endif // ifndef(PA_COMMS_OBSERVER_H)

