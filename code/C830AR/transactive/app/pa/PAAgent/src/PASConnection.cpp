/**
 * The source code in this file is the property of
 * Ripple Systems and is not for redistribution
 * in any form.
 *
 * Source : $File: //depot/C830AR_TIP/C830AR/transactive/app/pa/PAAgent/src/PASConnection.cpp $
 * @author  A. Ivankovic
 * @version $Revision: #11 $
 *
 * Last modification : $DateTime: 2025/09/09 12:36:11 $
 * Last modified by : $Author: limin.zhu $
 *
 */

#include "pch.h"
#include "PASConnection.h"
#include "PasHelpers.h"
#include "PasTableReadCounter.h"
#include "core/utilities/src/DebugUtilEx.h"
#include "core/utilities/src/StdUtil.h"
#include "core/utilities/src/TAAssert.h"
#include "core/utilities/src/StaticObject.h"
#include "core/utilities/src/stdutil/wait.h"
#include "core/exceptions/src/TcpSocketException.h"
#include "core/threads/src/Thread.h"
#include "core/message/src/MessagePublicationManager.h"
#include "core/message/types/PAAgentAudit_MessageTypes.h"
#include "core/message/src/AuditMessageSender.h"
#include "core/data_access_interface/src/LocationAccessFactoryEx.h"

#include <sstream>
#include <iomanip>
#include <chrono>

#if !defined( WIN32 )
#include <unistd.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <netinet/in_systm.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>

 //
 // Headers not available under FreeBSD.
 //
#if !defined( FREEBSD )
    //#include <stropts.h>
#endif // !defined( FREEBSD )

#else // defined( WIN32 )
#include <winsock2.h>
#include <windows.h>
#include <io.h>
#endif //  defined( WIN32 )

using namespace std::literals;
using namespace TA_Base_Core;
using std::chrono::steady_clock;
using std::chrono::seconds;

namespace
{
    const size_t PAS_WRITE_REQUEST_HEADER_SIZE                       = 5;
    const size_t PAS_WRITE_REQUEST_HEADER_TYPE_OFFSET                = 0;
    const size_t PAS_WRITE_REQUEST_HEADER_TABLE_NUMBER_OFFSET        = 1;
    const size_t PAS_WRITE_REQUEST_HEADER_TABLE_DATA_SIZE_OFFSET     = 3;

    const size_t PAS_WRITE_REPLY_PACKET_SIZE                         = 4;
    const size_t PAS_WRITE_REPLY_PACKET_TYPE_OFFSET                  = 0;
    const size_t PAS_WRITE_REPLY_PACKET_TABLE_NUMBER_OFFSET          = 1;
    const size_t PAS_WRITE_REPLY_PACKET_RETURN_VALUE_OFFSET          = 3;

    const size_t PAS_READ_REQUEST_PACKET_SIZE                        = 3;
    const size_t PAS_READ_REQUEST_PACKET_TYPE_OFFSET                 = 0;
    const size_t PAS_READ_REQUEST_PACKET_TABLE_NUMBER_OFFSET         = 1;

    const size_t PAS_READ_REPLY_HEADER_SIZE                          = 5;
    const size_t PAS_READ_REPLY_TABLE_100_DEPOT_HEADER_SIZE          = 7;
    const size_t PAS_READ_REPLY_HEADER_TYPE_OFFSET                   = 0;
    const size_t PAS_READ_REPLY_HEADER_TABLE_NUMBER_OFFSET           = 1;
    const size_t PAS_READ_REPLY_HEADER_TABLE_SIZE_OFFSET             = 3;

    const size_t SLEEP_TIME_IN_MSEC                                  = 10;
    const size_t SOCKET_FAILOVER_STALL_TIME_IN_MSEC                  = 0;

    int MAX_SOCKET_READ_TRIES                                        = 3;
}

//Maochun Sun++
//TD13454
//const size_t NON_BLOCKING_CONNECTION_PAUSE_IN_MSEC           = 3000;
//++Maochun Sun
//TD13454

namespace TA_IRS_App
{
    PASConnection* PASConnection::getInstance()
    {
        return &StaticObject<PASConnection>::value();
    }

    void PASConnection::removeInstance()
    {
        StaticObject<PASConnection>::destroy();
    }

    //const size_t PASConnection::MAX_READ_BUFFER_SIZE = 64000;

    PASConnection::PASConnection()
        : m_isUsingPrimaryConnection(true),
        m_socketOptions(),
        m_isEnabled(false),
        m_currentRetry(0),
        m_terminate(false)
        //Maochun Sun++
        //TD10396
        , m_isStillConnectedWithHW(true)
        //++Maochun Sun
        //TD10396
        //TD18607 marc_bugfix
        , m_PrimaryConnectionLost(false)
        , m_SecondaryConnectionLost(false)
        //TD18607

    {
        CachedConfig::getInstance()->attachObserver(this, CachedConfig::CONFIG_PRIMARY_FEPC_ADDRESS);
        CachedConfig::getInstance()->attachObserver(this, CachedConfig::CONFIG_SECONDARY_FEPC_ADDRESS);
        CachedConfig::getInstance()->attachObserver(this, CachedConfig::CONFIG_PRIMARY_FEPC_PORT);
        CachedConfig::getInstance()->attachObserver(this, CachedConfig::CONFIG_SECONDARY_FEPC_PORT);
        CachedConfig::getInstance()->attachObserver(this, CachedConfig::CONFIG_SOCKET_TIMEOUT);
    }

    PASConnection::PASConnection(const TA_Base_Core::SocketOptions& options)
        : m_isUsingPrimaryConnection(true),
        m_socketOptions(options),
        m_isEnabled(false),
        m_currentRetry(0)
        //Maochun Sun++
        //TD10396
        , m_isStillConnectedWithHW(true)
        //++Maochun Sun
        //TD10396
        //TD18607 marc_bugfix
        , m_PrimaryConnectionLost(false)
        , m_SecondaryConnectionLost(false)
        //TD18607
    {
        CachedConfig::getInstance()->attachObserver(this, CachedConfig::CONFIG_PRIMARY_FEPC_ADDRESS);
        CachedConfig::getInstance()->attachObserver(this, CachedConfig::CONFIG_SECONDARY_FEPC_ADDRESS);
        CachedConfig::getInstance()->attachObserver(this, CachedConfig::CONFIG_PRIMARY_FEPC_PORT);
        CachedConfig::getInstance()->attachObserver(this, CachedConfig::CONFIG_SECONDARY_FEPC_PORT);
        CachedConfig::getInstance()->attachObserver(this, CachedConfig::CONFIG_SOCKET_TIMEOUT);
    }

    PASConnection::~PASConnection()
    {
        CachedConfig::getInstance()->detachObserver(this);

        close();
    }

    void PASConnection::close()
    {
        LOG_SCOPE("PASConnection::close");
        ThreadGuard guard(m_lock);
        m_socket.reset();
        PasTableReadCounter::instance().reset();
    }

    int PASConnection::writeTable(int tableNumber, const std::uint8_t* writeBuffer, unsigned long bufferSize)
        //throw (TA_Base_Core::PasConnectionException)
    {
        LOG_SCOPE(boost::format("PASConnection::writeTable[%d]") % tableNumber);

        TA_ASSERT(bufferSize, "bufferSize is 0");
        TA_ASSERT(bufferSize <= MAX_PAS_BUFFER_SIZE, "bufferSize too big");
        TA_ASSERT(writeBuffer, "writeBuffer is NULL");

        if (!m_isEnabled)
        {
            handleUnexpectedSocketErrorsAndThrow("Connection disabled");
        }

        ThreadGuard guard(m_lock);

        // Create the header for the write request packet

        PasHelpers::instance().set8bit(m_internalBuffer, PAS_WRITE_REQUEST_HEADER_TYPE_OFFSET, 'W');
        PasHelpers::instance().set16bit(m_internalBuffer, PAS_WRITE_REQUEST_HEADER_TABLE_NUMBER_OFFSET, tableNumber);
        PasHelpers::instance().set16bit(m_internalBuffer, PAS_WRITE_REQUEST_HEADER_TABLE_DATA_SIZE_OFFSET, bufferSize);

        // Note that the external writeBuffer is locked by the calling class.  This used to
        // be locked here to minimise holdups, but this would randomly cause a deadlock.
        // The buffer lock is held for the entire duration of this call to be safe.

        ::memcpy(m_internalBuffer + PAS_WRITE_REQUEST_HEADER_SIZE, writeBuffer, bufferSize);

        //
        // Send the write packet
        //

#if !defined (PAS_DEMO_CODE)
        write(m_internalBuffer, bufferSize + PAS_WRITE_REQUEST_HEADER_SIZE);
#endif
        //
        // Receive the write packet reply
        //

        std::uint8_t replyPacket[PAS_WRITE_REPLY_PACKET_SIZE]{0};
        unsigned int bytesRead = 0;

#if defined (PAS_DEMO_CODE)
        bytesRead = PAS_WRITE_REPLY_PACKET_SIZE;
        PasHelpers::instance().set8bit(replyPacket, PAS_WRITE_REPLY_PACKET_TYPE_OFFSET, 'W');
        PasHelpers::instance().set16bit(replyPacket, PAS_WRITE_REPLY_PACKET_TABLE_NUMBER_OFFSET, tableNumber);
        PasHelpers::instance().set8bit(replyPacket, PAS_WRITE_REPLY_PACKET_RETURN_VALUE_OFFSET, 0);
#else
        read(replyPacket, &bytesRead, PAS_WRITE_REPLY_PACKET_SIZE, PAS_WRITE_REPLY_PACKET_SIZE);
#endif

        if (bytesRead != PAS_WRITE_REPLY_PACKET_SIZE)
        {
            std::stringstream err;
            err << "Failed to read the complete reply packet (Bytes read = "
                << bytesRead << ", expected = " << PAS_WRITE_REPLY_PACKET_SIZE << ")";
            handleUnexpectedSocketErrorsAndThrow(err.str());
        }

        char replyMode = PasHelpers::instance().get8bit(replyPacket, PAS_WRITE_REPLY_PACKET_TYPE_OFFSET);
        int replyTableNumber = PasHelpers::instance().get16bit(replyPacket, PAS_WRITE_REPLY_PACKET_TABLE_NUMBER_OFFSET);
        int replyReturnValue = PasHelpers::instance().get8bit(replyPacket, PAS_WRITE_REPLY_PACKET_RETURN_VALUE_OFFSET);

        if (replyMode != 'W' || replyTableNumber != tableNumber)
        {
            handleUnexpectedSocketErrorsAndThrow("Unexpected data received");
        }

        return replyReturnValue;
    }

    void PASConnection::readTable(int tableNumber, std::uint8_t* readBuffer, unsigned long bufferSize)
        //throw (TA_Base_Core::PasConnectionException)
    {
        LOG_SCOPE(boost::format("PASConnection::readTable[%d]") % tableNumber);

        TA_ASSERT(bufferSize, "bufferSize is 0");
        TA_ASSERT(bufferSize <= MAX_PAS_BUFFER_SIZE, "bufferSize too big");
        TA_ASSERT(readBuffer, "readBuffer is NULL");

        if (!m_isEnabled)
        {
            handleUnexpectedSocketErrorsAndThrow("Connection disabled");
        }

        ThreadGuard guard(m_lock);

        //
        // Send the read packet
        //

        // Create the write request packet
        std::uint8_t requestPacket[PAS_READ_REQUEST_PACKET_SIZE]{0};

        PasHelpers::instance().set8bit(requestPacket, PAS_READ_REQUEST_PACKET_TYPE_OFFSET, 'R');
        PasHelpers::instance().set16bit(requestPacket, PAS_READ_REQUEST_PACKET_TABLE_NUMBER_OFFSET, tableNumber);

#if !defined (PAS_DEMO_CODE)
        write(requestPacket, PAS_READ_REQUEST_PACKET_SIZE);
#endif

        //
        // Receive the read packet reply header
        //

        std::uint8_t replyPacket[PAS_READ_REPLY_HEADER_SIZE]{0};
        unsigned int bytesRead = 0;

#if defined (PAS_DEMO_CODE)
        bytesRead = PAS_READ_REPLY_HEADER_SIZE;
        PasHelpers::instance().set8bit(replyPacket, PAS_READ_REPLY_HEADER_TYPE_OFFSET, 'R');
        PasHelpers::instance().set16bit(replyPacket, PAS_READ_REPLY_HEADER_TABLE_NUMBER_OFFSET, tableNumber);
        PasHelpers::instance().set16bit(replyPacket, PAS_READ_REPLY_HEADER_TABLE_SIZE_OFFSET, bufferSize);
#else
        read(replyPacket, &bytesRead, PAS_READ_REPLY_HEADER_SIZE, PAS_READ_REPLY_HEADER_SIZE);
#endif

        if (bytesRead != PAS_READ_REPLY_HEADER_SIZE)
        {
            std::stringstream err;
            err << "Failed to read the complete reply packet (Bytes read = "
                << bytesRead << ", expected = " << PAS_READ_REPLY_HEADER_SIZE << ")";
            handleUnexpectedSocketErrorsAndThrow(err.str());
        }

        char replyMode = PasHelpers::instance().get8bit(replyPacket, PAS_READ_REPLY_HEADER_TYPE_OFFSET);
        int replyTableNumber = PasHelpers::instance().get16bit(replyPacket, PAS_READ_REPLY_HEADER_TABLE_NUMBER_OFFSET);
        std::uint16_t replySize = PasHelpers::instance().get16bit(replyPacket, PAS_READ_REPLY_HEADER_TABLE_SIZE_OFFSET);

        if (replySize == 0)
        {
            handleUnexpectedSocketErrorsAndThrow("Unexpected data received");
        }

        //
        // Receive the read packet reply body
        //

#if defined (PAS_DEMO_CODE)
        bytesRead = bufferSize;

        for (int i = 0; i < bufferSize; ++i)
        {
            m_internalBuffer[i] = rand();
        }

#else
        read(m_internalBuffer, &bytesRead, MAX_PAS_BUFFER_SIZE, replySize);
#endif

        if (replyMode != 'R' || replyTableNumber != tableNumber)
        {
            handleUnexpectedSocketErrorsAndThrow("Unexpected data received");
        }

        if (replySize != bufferSize)
        {
            handleUnexpectedSocketErrorsAndThrow("Unexpected data received");
        }

        if (bytesRead != bufferSize)
        {
            std::stringstream err;
            err << "Failed to read the complete reply packet (Bytes read = "
                << bytesRead << ", expected = " << bufferSize << ")";
            handleUnexpectedSocketErrorsAndThrow(err.str());
        }

        // Good to go ...

        // Note that the external readBuffer is locked by the calling class.  This used to
        // be locked here to minimise holdups, but this would randomly cause a deadlock.
        // The buffer lock is held for the entire duration of this call to be safe.

        memcpy(readBuffer, m_internalBuffer, bufferSize);
    }

    void PASConnection::readTable100Depot(std::uint8_t* readBuffer, unsigned long bufferSize)
        //throw (TA_Base_Core::PasConnectionException)
    {
        LOG_SCOPE("PASConnection::readTable100Depot");

        TA_ASSERT(bufferSize, "bufferSize is 0");
        TA_ASSERT(bufferSize <= MAX_PAS_BUFFER_SIZE, "bufferSize too big");
        TA_ASSERT(readBuffer, "readBuffer is NULL");

        if (!m_isEnabled)
        {
            handleUnexpectedSocketErrorsAndThrow("Connection disabled");
        }

        ThreadGuard guard(m_lock);

        //
        // Send the read packet
        //

        // Create the write request packet
        std::uint8_t requestPacket[PAS_READ_REQUEST_PACKET_SIZE];

        PasHelpers::instance().set8bit(requestPacket, PAS_READ_REQUEST_PACKET_TYPE_OFFSET, 'R');
        PasHelpers::instance().set16bit(requestPacket, PAS_READ_REQUEST_PACKET_TABLE_NUMBER_OFFSET, 100);

#if !defined (PAS_DEMO_CODE)
        write(requestPacket, PAS_READ_REQUEST_PACKET_SIZE);
#endif

        //
        // Receive the read packet reply header
        //

        std::uint8_t replyPacket[PAS_READ_REPLY_TABLE_100_DEPOT_HEADER_SIZE]{0};
        unsigned int bytesRead = 0;

#if defined (PAS_DEMO_CODE)
        bytesRead = PAS_READ_REPLY_HEADER_SIZE;
        PasHelpers::instance().set8bit(replyPacket, PAS_READ_REPLY_HEADER_TYPE_OFFSET, 'R');
        PasHelpers::instance().set16bit(replyPacket, PAS_READ_REPLY_HEADER_TABLE_NUMBER_OFFSET, tableNumber);
        PasHelpers::instance().set16bit(replyPacket, PAS_READ_REPLY_HEADER_TABLE_SIZE_OFFSET, bufferSize);
#else
        read(replyPacket, &bytesRead, PAS_READ_REPLY_TABLE_100_DEPOT_HEADER_SIZE, PAS_READ_REPLY_TABLE_100_DEPOT_HEADER_SIZE);
#endif

        if (bytesRead != PAS_READ_REPLY_TABLE_100_DEPOT_HEADER_SIZE)
        {
            std::stringstream err;
            err << "Failed to read the complete reply packet (Bytes read = "
                << bytesRead << ", expected = " << PAS_READ_REPLY_TABLE_100_DEPOT_HEADER_SIZE << ")";
            handleUnexpectedSocketErrorsAndThrow(err.str());
        }

        char replyMode = PasHelpers::instance().get8bit(replyPacket, PAS_READ_REPLY_HEADER_TYPE_OFFSET);
        int replyTableNumber = PasHelpers::instance().get16bit(replyPacket, PAS_READ_REPLY_HEADER_TABLE_NUMBER_OFFSET);
        auto replySize = PasHelpers::instance().get32bit(replyPacket, PAS_READ_REPLY_HEADER_TABLE_SIZE_OFFSET);

        if (replySize == 0)
        {
            handleUnexpectedSocketErrorsAndThrow("Unexpected data received");
        }

        if (replySize != bufferSize)
        {
            std::stringstream err;
            err << "Failed to read the header reply packet (Table Size = "
                << replySize << ", expected = " << bufferSize << ")";
            handleUnexpectedSocketErrorsAndThrow(err.str());
        }

        //
        // Receive the read packet reply body
        //

#if defined (PAS_DEMO_CODE)
        bytesRead = bufferSize;

        for (int i = 0; i < bufferSize; ++i)
        {
            m_internalBuffer[i] = rand();
        }

#else
        read(m_internalBuffer, &bytesRead, MAX_PAS_BUFFER_SIZE, replySize);
#endif

        if (replyMode != 'R' || replyTableNumber != 100)
        {
            handleUnexpectedSocketErrorsAndThrow("Unexpected data received");
        }

        if (bytesRead != bufferSize)
        {
            std::stringstream err;
            err << "Failed to read the complete reply packet (Bytes read = "
                << bytesRead << ", expected = " << bufferSize << ")";
            handleUnexpectedSocketErrorsAndThrow(err.str());
        }

        // Good to go ...

        // Note that the external readBuffer is locked by the calling class.  This used to
        // be locked here to minimise holdups, but this would randomly cause a deadlock.
        // The buffer lock is held for the entire duration of this call to be safe.

        memcpy(readBuffer, m_internalBuffer, bufferSize);
    }

    void PASConnection::ensureConnected() //throw(TA_Base_Core::PasConnectionException)
    {
        LOG_SCOPE("PASConnection::ensureConnected");

        ThreadGuard guard(m_lock);

        // If not enabled, don't bother to connect.
        if (!m_isEnabled)
        {
            TA_THROW(PasConnectionException("PASConnection instance not enabled, possibly in monitor mode.", UNABLE_TO_CONNECT));
        }

        // Connect to the socket if its not currently connected.
        if (!m_socket)
        {
            LOG_ERROR("Creating a new TcpSocket on %s FEPC", m_isUsingPrimaryConnection ? "primary" : "secondary");

            try
            {
                unsigned long genericTimeoutInMSecs = CachedConfig::getInstance()->getSocketTimeoutInSecs() * 1000;
                MAX_SOCKET_READ_TRIES = CachedConfig::getInstance()->getRetries();

                if (m_isUsingPrimaryConnection)
                {
                    m_socket = std::make_shared<TcpSocket>(CachedConfig::getInstance()->getPrimaryFEPCAddress(),
                                                           CachedConfig::getInstance()->getPrimaryFEPCPort(),
                                                           false,  // Blocking
                                                           genericTimeoutInMSecs,
                                                           genericTimeoutInMSecs,
                                                           genericTimeoutInMSecs);
                }
                else
                {
                    m_socket = std::make_shared<TcpSocket>(CachedConfig::getInstance()->getSecondaryFEPCAddress(),
                                                           CachedConfig::getInstance()->getSecondaryFEPCPort(),
                                                           false,  // Blocking
                                                           genericTimeoutInMSecs,
                                                           genericTimeoutInMSecs,
                                                           genericTimeoutInMSecs);
                }
            }
            catch (TcpSocketException& e)
            {
                // Switch over to the other connection for the next time
                m_isUsingPrimaryConnection = !m_isUsingPrimaryConnection;
                TA_THROW(PasConnectionException(e.what(), UNABLE_TO_CONNECT));
            }

            // apply any options before connecting
            m_socketOptions.apply(m_socket->getSocketId());

            connect();
        }
        else if (!m_socket->stillConnected())
        {
            connect();
        }

        // If its still not connected throw an exception.
        if (!m_socket->stillConnected())
        {
            close();
            TA_THROW(PasConnectionException("Failed to connect to device.", UNABLE_TO_CONNECT));
        }
    }

    int PASConnection::write(const std::uint8_t* buffer, const int length)
        //throw(TA_Base_Core::PasConnectionException)
    {
        LOG_SCOPE("PASConnection::write");

        ensureConnected();

        try
        {
            PasHelpers::instance().hexDumpDebug("Writing: ", buffer, length);
            return m_socket->write(reinterpret_cast<const char*>(buffer), length);
        }
        catch (TcpSocketException& e)
        {
            handleUnexpectedSocketErrorsAndThrow(e.what());
        }

        // Never gets here
        return 0;
    }

    int PASConnection::read(std::uint8_t* buffer, unsigned int* totalBytesRead, const unsigned int bufferLength, unsigned int maxBytesToRead)
        //throw(TA_Base_Core::PasConnectionException)
    {
        LOG_SCOPE("PASConnection::read");

        *totalBytesRead = 0;

        if (!isConnected())
        {
            handleUnexpectedSocketErrorsAndThrow("Socket not connected when attempting a read.");
        }

        int readErrorCode = 0;
        size_t readTries = 0;
        unsigned int bytesReadInOneIteration = 0;

        while (!m_terminate && readTries < MAX_SOCKET_READ_TRIES)
        {
#if 0
            while (!m_terminate && !m_socket->isReadRequired())
            {
                Thread::sleep(SLEEP_TIME_IN_MSEC);
            }
#else
            auto res = st::wait_for(CachedConfig::getInstance()->getSocketTimeoutInSecs() * 1000, 10, [=]
            {
                return m_socket->isReadRequired() ||
                    !m_socket->stillConnected() ||
                    m_terminate
                    ;
            });

            if (m_terminate)
            {
                handleUnexpectedSocketErrorsAndThrow("terminated");
            }

            if (!m_socket->stillConnected())
            {
                handleUnexpectedSocketErrorsAndThrow("disconnected");
            }

            if (!res)
            {
                ++readTries;
                LOG_DEBUG("read(): can not read, readTries=%d", readTries);
                continue;
            }
#endif

            try
            {
                readErrorCode = m_socket->read(reinterpret_cast<char*>(buffer + *totalBytesRead),
                                               &bytesReadInOneIteration,
                                               (maxBytesToRead - *totalBytesRead));
            }
            catch (TcpSocketException& e)
            {
                handleUnexpectedSocketErrorsAndThrow(e.what());
            }

            if (readErrorCode != 0)
            {
                // Any low level socket read failures force a closure of the socket.
                // close();
                handleUnexpectedSocketErrorsAndThrow(str(boost::format("Read error (error code = %d)") % readErrorCode));
            }

            if (bytesReadInOneIteration == 0)
            {
                // Failed to read any bytes so retry
                ++readTries;
                LOG_DEBUG("read(): 0 bytes read, readTries=%d", readTries);
                continue;
            }

            *totalBytesRead += bytesReadInOneIteration;

            if (*totalBytesRead >= maxBytesToRead)
            {
                // Done
                PasHelpers::instance().hexDumpDebug("Read: ", buffer, *totalBytesRead);
                return 0;
            }

            PasHelpers::instance().hexDumpDebug("Partial Read: ", buffer + *totalBytesRead - bytesReadInOneIteration, bytesReadInOneIteration);
        }

        if (*totalBytesRead)
        {
            PasHelpers::instance().hexDumpDebug("Read: ", buffer, *totalBytesRead);
        }

        if (readTries >= MAX_SOCKET_READ_TRIES)
        {
            //Bohong++, move following code into a try block, due to the exception thown out in m_socket->getClientName().
            //However if any socket expections, we should get an exception thrown out when call m_socket->read() but not here.
            //Suspect there is something handled not properly in socket libarary, so we handle it here 1st.
            try
            {
                // Send an audit message stating that the PA Server was not responding
                auto sender = TA_Base_Core::MessagePublicationManager::getInstance().getAuditMessageSenderPtr(TA_Base_Core::PAAgentAudit::Context);

                TA_Base_Core::DescriptionParameters params;
                TA_Base_Core::NameValuePair param1("IPAddress", m_socket->getClientName());
                TA_Base_Core::NameValuePair param2("Port", m_socket->getServiceName());
                params.push_back(&param1);
                params.push_back(&param2);

                sender->sendAuditMessage(TA_Base_Core::PAAgentAudit::PaFEPCNotResponding,
                                         //           CachedConfig::getInstance()->getAgentKey(),
                                         CachedConfig::getInstance()->getMonitorProcessKey(),
                                         params,
                                         "N/A",
                                         "",
                                         "",
                                         "",
                                         "");

                // Throw an exception
                std::stringstream stringStream;
                stringStream << "PAS FEPC at is not reponding at "
                    << m_socket->getClientName()
                    << ":" << m_socket->getServiceName();
                handleUnexpectedSocketErrorsAndThrow(stringStream.str().c_str());
            }
            catch (...)
            {
                std::stringstream stringStream;
                stringStream << "PASConnection unknown expection caught ";
                handleUnexpectedSocketErrorsAndThrow(stringStream.str().c_str());
            }

            //++Bohong
        }

        return readErrorCode;
    }

    void PASConnection::handleUnexpectedSocketErrorsAndThrow(const std::string& error)
        //throw(TA_Base_Core::PasConnectionException)
    {
        LOG_SCOPE("PASConnection::handleUnexpectedSocketErrorsAndThrow");

        // Put in place mid IT 19 to limit required changes to just the PA Agent instead
        // of changing the TcpSocket class.  Was intending to add the following code to
        // TcpSocket:1320
        //
        //             if( 0 == lvStatus )
        //             {
        //                 LOG (SourceInfo, TA_Base_Core::DebugUtil::GenericLog,
        //                      TA_Base_Core::DebugUtil::DebugInfo,
        //                      "No data to be read from socket => socket closed" );
        //
        //   >>>>>>>>>>    m_hasDetectedDisconnection = true;
        //                 return false;
        //             }

        // Closing the socket is the simplest approach put ugly.
        close();

        // Prefer to be able to flush the connection and continue

        TA_THROW(PasConnectionException(error.c_str(), DISCONNECTED));
    }

    void PASConnection::connect() //throw(TA_Base_Core::PasConnectionException)
    {
        LOG_SCOPE("PASConnection::connect");

        unsigned long maxRetries = CachedConfig::getInstance()->getRetries();
        unsigned long retry = 0;
        std::string lastWhat("");

        for (int connection = 0; connection < 2; ++connection)
        {
            // Caters for primary and secondary connection

            for (unsigned long retry = 0; retry <= maxRetries; ++retry)
            {
                try
                {
                    if (retry > 0)
                    {
                        LOG_ERROR("Retrying PAS connection (%d of %d) on %s FEPC", retry, maxRetries,
                                  m_isUsingPrimaryConnection ? "primary" : "secondary");
                    }

                    m_socket->connect();

                    // For some reason in UNIX, it was noted that the connect would
                    // return okay but it is still not connected yet.  After isConnected()
                    // returns false and we call connect() again, it indicates that the
                    // connection is still in progress.
                    //
                    // We have inserted this slight pause to hopefully remove this
                    // hicup.

                    //Maochun Sun++
                    //TD13454
                    //Thread::sleep(NON_BLOCKING_CONNECTION_PAUSE_IN_MSEC);
                    std::uint8_t replyBuffer[17];
                    std::uint8_t requestBuffer[3];

                    PasHelpers::instance().set8bit(requestBuffer, PAS_READ_REQUEST_PACKET_TYPE_OFFSET, 'R');
                    PasHelpers::instance().set16bit(requestBuffer, PAS_READ_REQUEST_PACKET_TABLE_NUMBER_OFFSET, 302);

                    PasHelpers::instance().hexDumpDebug("Writing: ", requestBuffer, 3);
                    m_socket->write(reinterpret_cast<const char*>(requestBuffer), 3);

                    unsigned int actualSize = 17;
                    m_socket->read(reinterpret_cast<char*>(replyBuffer), &actualSize, 17);
                    PasHelpers::instance().hexDumpDebug("Read: ", replyBuffer, actualSize);

                    //++Maochun Sun
                    //TD13454

                    if (isConnected())
                    {
                        // Success therefore return after closing any connection alarms
                        // liuyu++ TD4657
                        // PasHelpers::instance().closePASCommsFailureAlarm();
                        // normalize previously raised alarm
                        /*
                                            if (m_isUsingPrimaryConnection)
                                            {
                                                PasHelpers::instance().closePASCommsFailureAlarm(
                                                    CachedConfig::getInstance()->getPrimaryFEPCAddress(),
                                                    CachedConfig::getInstance()->getPrimaryFEPCPort() );
                                            }
                                            else
                                            {
                                                PasHelpers::instance().closePASCommsFailureAlarm(
                                                    CachedConfig::getInstance()->getSecondaryFEPCAddress(),
                                                    CachedConfig::getInstance()->getSecondaryFEPCPort() );
                                            }
                        */
                        //TD18607   marc_bugfix make both variables false so that next time failover occurs, alarm will only be
                        // raised if both connections have been tried
                        PasHelpers::instance().closePASCommsFailureAlarm();
                        m_PrimaryConnectionLost = false;
                        m_SecondaryConnectionLost = false;
                        //TD18607

                        //Maochun Sun++
                        //TD10396
                        m_isStillConnectedWithHW = true;
                        //++Maochun Sun
                        //TD10396

                        // ++liuyu TD4657
                        return;
                    }

                    // In the current version of the sockets library, it is still possible to return
                    // from m_socket->connect() when not connected.
                    lastWhat = "Unable to connect";
                    // keep retrying
                }
                catch (TcpSocketException& e)
                {
                    // continue to retry
                    lastWhat = e.what();
                }

                //Maochun Sun++
                //TD13454
                catch (TA_Base_Core::PasConnectionException& e)
                {
                    // continue to retry
                    lastWhat = e.what();
                }

                //++Maochun Sun
                //TD13454
            } // for loop for retries

            failoverProcess();

            // now try again for the other connection
            // ??? Maybe we process the new connection next time???
        } // for loop for connections

        // by this point we have failed maxRetry times x 2 connections
        close();

        //Maochun Sun++
        //TD10396
        m_isStillConnectedWithHW = false;
        //++Maochun Sun
        //TD10396

        TA_THROW(PasConnectionException(lastWhat.c_str(), UNABLE_TO_CONNECT));
    }

    void PASConnection::failoverProcess() //throw(TA_Base_Core::PasConnectionException)
    {
        LOG_SCOPE("PASConnection::failoverProcess");

        close();
        Thread::sleep(SOCKET_FAILOVER_STALL_TIME_IN_MSEC);

        // Raise a (unique) alarm first
        //TD18607 marc_bugfix
        /*
            if (m_isUsingPrimaryConnection)
            {
                PasHelpers::instance().raisePASCommsFailureAlarm(
                    CachedConfig::getInstance()->getPrimaryFEPCAddress(),
                    CachedConfig::getInstance()->getPrimaryFEPCPort() );
            }
            else
            {
                PasHelpers::instance().raisePASCommsFailureAlarm(
                    CachedConfig::getInstance()->getSecondaryFEPCAddress(),
                    CachedConfig::getInstance()->getSecondaryFEPCPort() );
            }
        */
        if (m_isUsingPrimaryConnection)
        {
            m_PrimaryConnectionLost = true;
        }
        else
        {
            m_SecondaryConnectionLost  = true;
        }

        if (m_PrimaryConnectionLost && m_SecondaryConnectionLost)
        {
            PasHelpers::instance().raisePASCommsFailureAlarm();
        }

        //TD18607

        // Swap connections
        m_isUsingPrimaryConnection = !m_isUsingPrimaryConnection;

        LOG_ERROR("Creating a new TcpSocket on %s FEPC", m_isUsingPrimaryConnection ? "primary" : "secondary");

        try
        {
            unsigned long genericTimeoutInMSecs = CachedConfig::getInstance()->getSocketTimeoutInSecs() * 1000;

            if (m_isUsingPrimaryConnection)
            {
                m_socket = std::make_shared<TcpSocket>(CachedConfig::getInstance()->getPrimaryFEPCAddress(),
                                                       CachedConfig::getInstance()->getPrimaryFEPCPort(),
                                                       false,  // Blocking
                                                       genericTimeoutInMSecs,
                                                       genericTimeoutInMSecs,
                                                       genericTimeoutInMSecs);
            }
            else
            {
                m_socket = std::make_shared<TcpSocket>(CachedConfig::getInstance()->getSecondaryFEPCAddress(),
                                                       CachedConfig::getInstance()->getSecondaryFEPCPort(),
                                                       false,  // Blocking
                                                       genericTimeoutInMSecs,
                                                       genericTimeoutInMSecs,
                                                       genericTimeoutInMSecs);
            }

            // apply any options before connecting
            m_socketOptions.apply(m_socket->getSocketId());
        }
        catch (TcpSocketException& e)
        {
            TA_THROW(PasConnectionException(e.what(), UNABLE_TO_CONNECT));
        }
    }

    ///////////////////////////////////////////////////////////////////////////
    //
    //                     ICachedConfigObserver methods
    //
    ///////////////////////////////////////////////////////////////////////////

    void PASConnection::processCachedConfigUpdate(CachedConfig::ECachedConfigItemKey key)
    {
        ThreadGuard guard(m_lock);

        switch (key)
        {
            case CachedConfig::CONFIG_PRIMARY_FEPC_ADDRESS:
            case CachedConfig::CONFIG_PRIMARY_FEPC_PORT:
                if (m_isUsingPrimaryConnection)
                {
                    // Change affects the primary connection which we are currently using
                    // We shall force a disconnection so that a new one is made on the next read
                    close();
                }

                break;

            case CachedConfig::CONFIG_SECONDARY_FEPC_ADDRESS:
            case CachedConfig::CONFIG_SECONDARY_FEPC_PORT:
                if (!m_isUsingPrimaryConnection)
                {
                    // Change affects the secondary connection which we are currently using
                    // We shall force a disconnection so that a new one is made on the next read
                    close();
                }

                break;

            case CachedConfig::CONFIG_SOCKET_TIMEOUT:
                // Force a close to recreate the socket with the new timeout parameter
                close();
                break;

            default:
                break;
        }
    }
}
