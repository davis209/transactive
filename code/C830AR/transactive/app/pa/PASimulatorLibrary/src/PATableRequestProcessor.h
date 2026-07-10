#ifndef PA_TABLE_REQUEST_PROCESSOR_H
#define PA_TABLE_REQUEST_PROCESSOR_H

/**
  * The source code in this file is the property of
  * Ripple Systems and is not for redistribution
  * in any form.
  *
  * Source:   $File: //depot/4669_T01271350/4669/transactive/app/pa/PASimulatorLibrary/src/PATableRequestProcessor.h $
  * @author:  Robert van Hugten
  * @version: $Revision: #1 $
  *
  * Last modification: $DateTime: 2008/11/28 16:26:01 $
  * Last modified by:  $Author: builder $
  *
  * PATableRequestProcessor is a thread which holds a queue of incoming packets. The
  * main method of the class loops, checking the queue for packets to be processed.
  */

#include <queue>
#include <map>

#include "core/synchronisation/src/NonReEntrantThreadLockable.h"
#include "core/threads/src/Thread.h"

#include "app/pa/PASimulatorLibrary/src/PASimPackets.h"
#include "app/pa/PASimulatorLibrary/src/PASimTypes.h"


namespace TA_App
{
    class PACommsObserver;
	class PASimTable;
    class PATable560;
    class PATableInvalid;

    class PATableRequestProcessor : public virtual TA_Core::Thread
    {

    public:

        /**
          * Preferred constructor
          *
          * @param owner The parent
          */
        PATableRequestProcessor( TA_App::PACommsObserver* owner );


        /**
          * Destructor
          */
        virtual ~PATableRequestProcessor();


        //////////////////////////////////////////////////////////////////////////////
        //                                                                          //
        //                       TA_Core::Thread methods                            //
        //                                                                          //
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
          * The terminate method must be implemented in any class that inherits
          * from TA_Core::Thread.
          *
          */
        void terminate();



        //////////////////////////////////////////////////////////////////////////////
        //                                                                          //
        //                        PATableRequestProcessor methods                   //
        //                                                                          //
        //////////////////////////////////////////////////////////////////////////////

        /**
          * addMessageRequest
          *
          * Called by PACommsObserver when a packet arrives on its socket.  It
          * adds the packet to the table request processor's queue.
          *
          * @param request the packet from the ISCS to add to the queue.
          */
        void addMessageRequest( ISCSRequest * request );

        /**
          * addUserRequest
          *
          * This method allows users (via PASimulatorCommandObserver) to add
          * requests to the queue.
          *
          * @param packetData pointer to the packet to send. Ownership of the
          *                   packet is transferred.
          */
        void addUserRequest( UserRequest * requestData );

        /**
          * isLocal
          *
          * This method checks if the instance of the SwitchProcessor is local or
          * attached to an ObservedSocket.
          *
          * @return the value of the data member m_localProcessor
          */
        bool isLocal();

        /**
          * sendReadReply
          *
          * Used by concrete table classes to send a reply packet to a
          * read request
          */
        void sendReadReply(ISCSReply * packet);

        /**
          * sendWriteReply
          *
          * Used by concrete table classes to send a reply packet to a
          * write request
          */
        void sendWriteReply(ISCSReply * packet);

        /**
          * retrieves the pointer to the table with the given number.
          *
          * @param the number of the table to attempt to fetch
          * @return the pointer to the Table if one exists, nil otherwise
          */
        PASimTable * getTable(int tNumber);

        /**
          * Called in the PASimTable constructor, adds the table to the TableMap
          *
          * @param the table to add to the map
          */
        void addTable(PASimTable * t);

        /**
          * Adds the given table pointer to the list of dependents of the
          * numbered table. If the requested numbered table does not exist,
          * nothing is done.
          *
          * @param t the table that is requesting dependency
          * @param tableNumber the number of the table that is going to be
          *        depended upon
          */
        void dependUponTable(PASimTable * t, int tableNumber);

    private:
        /**
          * isValidRequest
          *
          * This method validates the packet that has just been pulled from
          * the packet queue. This method does NOT check to see what type of
          * packet it has.
          * 
          * This method is now mostly moot, as verification and validation is
          * done elsewhere first
          *
          * @param request The packet to be validated.
          * @return true if the packet is valid, false otherwise.
          *
          */
        bool isValidRequest( const ISCSRequest * request );

        /**
          * isValidUserRequest
          *
          * Similar to isValidRequest but operates on UserRequests.
          * This method validates the UserRequest "packet" that has just been
          * pulled from the userRequests queue.
          * 
          * @param request The request to be validated.
          * @return true if the request is valid, false otherwise.
          *
          */
        bool isValidUserRequest( const UserRequest * request );

        /**
          * sendPacket
          * 
          * Sends the packet to either the required socket or std::cout.
          *
          * @param packet The packet to be sent.
          *
          */
        void sendPacket( const ByteVector & packet);


        /**
          * The requests that come from the ISCS are stored in a fifo
          */
        std::queue< ISCSRequest * > ISCSRequestQueue;

        /** A queue of incoming table requests.  Each request is an array
          * of Bytes (a.k.a. unsigned chars)
          */
        std::queue< ISCSRequest * > m_incomingRequests;

        /**
          *
          */
        volatile bool m_doThreadTerminate;

        /**
          *
          */
        volatile bool m_threadIsRunning;

        /** The CommsObserver that owns this instance of the processor
          */
        TA_App::PACommsObserver* m_parent;

        /**
          * Flag that indicates if the switch processor is bound to an observed
          * socket, or is a local instance
          */
        bool m_localProcessor;

        /**
          * typedef for a map of tables, keyed by table number
          */
        typedef std::map< int, PASimTable * > PASimTableMap;

        /**
          * A map of tables, keyed by table number
          */
        PASimTableMap m_tables;

        /**
          * Special table needed for flagging normal poll table write requests
          */
        PATable560 * m_table560;

        /**
          * Special table used for requests for tables we haven't implemented
          */
        PASimTable * m_invalidTable;

        /**
          * Lock for a ThreadGuard to only allow one thread at a time to
          * access the tables. I have yet to see exactly how they work.
          */
        TA_Core::NonReEntrantThreadLockable m_tableLock;

        typedef std::queue< UserRequest * > UserRequestQueue;

        UserRequestQueue m_userRequests;
    };
}
#endif // ifndef (PA_TABLE_REQUEST_PROCESSOR_H)
