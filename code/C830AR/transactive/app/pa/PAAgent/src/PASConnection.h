/**
 * The source code in this file is the property of
 * Ripple Systems and is not for redistribution
 * in any form.
 *
 * Source : $File: //depot/C830AR_TIP/C830AR/transactive/app/pa/PAAgent/src/PASConnection.h $
 * @author  A. Ivankovic
 * @version $Revision: #8 $
 *
 * Last modification : $DateTime: 2025/09/09 12:36:11 $
 * Last modified by : $Author: limin.zhu $
 *
 * A TCP/IP connection to a Modbus device.
 *
 * Note: In failure scenarios calls to the IModbus method implementations of this
 * class will block the calling thread for the TCP/IP connection time out period.
 * If this is an issue for the application, the Modbus communication threads need
 * to be isolated from the application code that needs to be responsive even
 * under failure scenarios.
 *
 */

#pragma once
#include "app/pa/PAAgent/src/CachedConfig.h"
#include "core/types/src/ta_types.h"
#include "core/synchronisation/src/ReEntrantThreadLockable.h"
#include "core/sockets/src/TcpSocket.h"
#include "core/sockets/src/SocketOptions.h"
#include "core/exceptions/src/PasExceptions.h"

 // const int MAX_PAS_BUFFER_SIZE = 64000;
const int MAX_PAS_BUFFER_SIZE = 204800;

namespace TA_IRS_App
{
    class PASConnection : public virtual ICachedConfigObserver
    {
    public:

        void setTerminate(bool terminate)
        {
            m_terminate = terminate;
        }

        // liuyu++ #243 terminate error
        bool getTerminate()
        {
            return m_terminate;
        }

        // ++liuyu #243 terminate error

        /**
         * getInstance
         *
         * Returns an instance of the class
         *
         * @return the pointer to this singleton instance
         *
         */
        static PASConnection* getInstance();

        /**
         * removeInstance
         *
         * Removes the instance of the class (if already created) and cleanup the members.  Primarily
         * used upon program termination (e.g. from main()) so that Purify does not consider this class
         * and its members to be leaks.
         *
         */
        static void removeInstance();

        /**
         * enable
         *
         * Enables this object for FEPC device communications. This object
         * will subsequently initiate a connection to the actual modbus device
         * as required to respond to communication method calls.
         */
        void enable()
        {
            m_isEnabled = true;
            // Restore 'Blocking read'
            setTerminate(false);
        }

        /**
         * disable
         *
         * Disables FEPC communications via the object. The current connection
         * to the Modbus device is disconnected. Any subsequent communication method
         * calls on this object will fail (until it is enabled again). The disable
         * method is useful in redundancy scenarios where a PASConnection instance is
         * being used as a "hot standby" but should not take up any FEPC device
         * connection resources.
         */
        void disable()
        {
            m_isEnabled = false;
            // Wake up any 'Blocking reads'
            setTerminate(true);

            // Threadguarded
            close();
        }

        /**
         * writeTable
         *
         * Writes a PAS table packet to the communication connection and returns when the
         * write confirmation packet is received.  This is a blocking call.
         *
         * @param tableNumber the number of the PAS table (used for header creation)
         * @param writeBuffer the raw PAS table data (without the packet header)
         * @param bufferSize  the size of the writeBuffer (also used for header creation)
         *
         * @return the return state of the operation.
         *
         */
        int writeTable(int tableNumber,
                       const std::uint8_t* writeBuffer,
                       unsigned long bufferSize);
        //throw (TA_Base_Core::PasConnectionException);

        /**
         * readTable
         *
         * First sends a read PAS table request packet returns when the PAS table packet
         * from the communication connection is received.  This is a blocking call.
         *
         * @param tableNumber the number of the PAS table (used for header creation)
         * @param readBuffer the raw PAS table data (without the packet header)
         * @param bufferSize  the size of the readBuffer (also used for header creation)
         *
         * @return the return state of the operation.
         *
         */
        void readTable(int tableNumber, std::uint8_t* readBuffer, unsigned long bufferSize);
        //throw (TA_Base_Core::PasConnectionException);

        void readTable100Depot(std::uint8_t* readBuffer, unsigned long bufferSize);

        bool isConnected()
        {
            return (m_socket && m_socket->stillConnected());
        }

        /**
         * connect
         *
         * Wraps the TcpSocket connection function with some exception
         * processing code and retry handling
         *
         */
        void connect(); //throw(TA_Base_Core::PasConnectionException);

        /**
         * ensureConnected
         *
         * Connects to the FEPC device if a connection is not current.
         *
         * @exception TcpSocketException Thrown if a TCP/IP connection problem is
         * encountered.
         */
        void ensureConnected(); //throw(TA_Base_Core::PasConnectionException);

        //Maochun Sun++
        //TD10396
        bool isStillConnectedWithHW()
        {
            return m_isStillConnectedWithHW;
        }

        //++Maochun Sun
        //TD10396

    protected:

        /**
         * close
         *
         * Closes the socket connection if open and destroys the socket object.
         * This method does not change the enable status.
         */
        void close();

        /**
         * handleUnexpectedSocketErrorsAndThrow()
         *
         * Closes the socket and throws the PasConnectionException
         */
        void handleUnexpectedSocketErrorsAndThrow(const std::string& error);
        //throw(TA_Base_Core::PasConnectionException)

        /**
         * failoverProcess
         *
         * Raises the failover alarm, destroys the current connection and switches to the backup
         * connection.
         *
         */
        void failoverProcess(); //throw(TA_Base_Core::PasConnectionException);

        /**
         * write
         *
         * Wraps the TcpSocket write function with some exception
         * processing code.
         *
         * @param buffer   The buffer to be written to the socket.
         * @param length   The length of the buffer to be written.
         * @return         The number of characters actually written.
         */
        int write(const std::uint8_t* buffer, const int length); //throw(TA_Base_Core::PasConnectionException);

        /**
         * read
         *
         * Wraps the TcpSocket read function with some exception
         * processing code.
         *
         * @param buffer           The buffer to be read from the socket.
         * @param bytesRead        The actual number of bytes read.
         * @param length           The maximum allowed length of the read buffer returned.
         * @param maxBytesToRead   The number of bytes read.
         * @return                 The number of characters actually written.
         */
        int read(std::uint8_t* buffer, unsigned int* totalBytesRead, const unsigned int bufferLength, unsigned int maxBytesToRead);
        //throw(TA_Base_Core::PasConnectionException);

    public:

        PASConnection(const PASConnection&) = delete;
        PASConnection& operator=(const PASConnection&) = delete;

        /**
         * PASConnection
         *
         * Constructor.
         *
         * @param isInitiallyEnabled   specifies whether the connection is initially enabled (startup
         *                             in CONTROL mode).
         */
        PASConnection();

        /**
         * PASConnection
         *
         * Constructor.
         *
         * @param isInitiallyEnabled   specifies whether the connection is initially enabled (startup
         *                             in CONTROL mode).
         * @param options              a socket options object.
         *
         */
        PASConnection(const TA_Base_Core::SocketOptions& options);

        /**
         * ~PASConnection
         *
         * Destructor.
         */
        virtual ~PASConnection();

    private:

        //TD18607 marc_bugfix
        bool m_PrimaryConnectionLost;
        bool m_SecondaryConnectionLost;
        //TD18607 marc_bugfix

    protected:

        bool                                        m_isUsingPrimaryConnection = true;
        bool                                        m_isEnabled = false;
        std::shared_ptr<TA_Base_Core::TcpSocket>    m_socket;
        TA_Base_Core::SocketOptions                 m_socketOptions;
        int                                         m_currentRetry = 0;

        TA_Base_Core::ReEntrantThreadLockable       m_lock;

        std::uint8_t                                m_internalBuffer[MAX_PAS_BUFFER_SIZE];
        bool                                        m_terminate = false;

        //Maochun Sun++
        //TD10396
        bool                                        m_isStillConnectedWithHW = false;
        //++Maochun Sun
        //TD10396

    public:

        ///////////////////////////////////////////////////////////////////////////
        //
        //                     ICachedConfigObserver methods
        //
        ///////////////////////////////////////////////////////////////////////////

        virtual void processCachedConfigUpdate(CachedConfig::ECachedConfigItemKey key);
    };
}
