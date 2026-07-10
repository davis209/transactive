/**
 * The source code in this file is the property of
 * Ripple Systems and is not for redistribution
 * in any form.
 *
 * Source:   $File: //depot/C830AR_TIP/C830AR/transactive/app/pa/PAAgent/src/AbstractPASTable.h $
 * @author:  Ripple
 * @version: $Revision: #7 $
 *
 * Last modification: $DateTime: 2025/06/24 11:25:13 $
 * Last modified by:  $Author: limin.zhu $
 *
 */

#pragma once
#include "PasTableReadCounter.h"
#include "bus/pa/cachedmaps/src/CachedMaps.h"
#include "bus/pa_4669/pa_agent/IDL/src/IPAAgentCorbaDef.h"
#include "core/synchronisation/src/NonReEntrantThreadLockable.h"
#include "core/synchronisation/src/ThreadGuard.h"
#include "core/exceptions/src/ThrowException.h"
#include <boost/describe.hpp>
#include <cstdint>

namespace TA_Base_Core
{
    class CommsMessageSender;
    using CommsMessageSenderPtr = std::shared_ptr<CommsMessageSender>;
}

/**
 * Super class for all Table classes to inherit.
 */
class AbstractPASTable
{
public:

    /** Destructs an instance of this class. */
    virtual ~AbstractPASTable()
    {
        // ++ AZ bug774 (TD13071)
        delete[] m_buffer;
        m_buffer = nullptr;
        // ++ AZ
    };

    enum EPasWriteErrors
    {
        PAS_ERR_OK                      = 0,
        PAS_ERR_GENERIC                 = 1,
        PAS_ERR_BUSY_MESSAGE_SEQUENCE   = 2,
        PAS_ERR_BUSY_SOURCE             = 3,
        PAS_ERR_BUSY_RECORDING          = 4,
        PAS_ERR_BUSY_BROADCASTING       = 5,
        PAS_ERR_INVALID_ANNOUNCE        = 6,
        PAS_ERR_NO_FLAG                 = 7,
        PAS_ERR_NO_FIRE                 = 8
    };

protected:

    /**
     * Constructs an instance of this class.
     * @param tableNumber The table number of the table the instance will store.
     * @param bufferSize  The size of the buffer for this table.
     */
    AbstractPASTable(unsigned long tableNumber);

    static void init();
    static bool is_depot();
    static bool is_station();
    static bool is_station_or_depot();

    static bool is_live(TA_Base_Bus::IPAAgentCorbaDef::ESource source);
    static bool is_live(CORBA::Octet source);
    static bool is_bgm(TA_Base_Bus::IPAAgentCorbaDef::ESource source);
    static bool is_bgm(CORBA::Octet source);
    static bool is_dva(TA_Base_Bus::IPAAgentCorbaDef::ESource source);
    static bool is_dva(CORBA::Octet source);

private:

    AbstractPASTable(const AbstractPASTable& theAbstractPASTable) = delete;

protected:

    static size_t get_buffer_size(int tableNumber);

    /** The table number of the PAS table this instance will represent. */
    const size_t TABLE_NUMBER = 0;

    /** The buffer size. */
    unsigned long BUFFER_SIZE = 0;

    /** Thread lock to protect the buffer. */
    TA_Base_Core::ReEntrantThreadLockable m_lock;

    /** The buffer that will contain all the data from the PAS. */
    std::uint8_t* m_buffer = nullptr;

    TA_Base_Core::CommsMessageSenderPtr m_paAgentCommsSender;

public:

    static size_t MAXSTNID;
    static size_t MAXZONEID;
    static size_t MAXMODEID;
    static size_t MAXSRCID;
    static size_t MAXMSGSEQ;
    static size_t NBPREREC;
    static size_t NBADHOC;
};

BOOST_DESCRIBE_ENUM(AbstractPASTable::EPasWriteErrors,
                    PAS_ERR_OK,
                    PAS_ERR_GENERIC,
                    PAS_ERR_BUSY_MESSAGE_SEQUENCE,
                    PAS_ERR_BUSY_SOURCE,
                    PAS_ERR_BUSY_RECORDING,
                    PAS_ERR_BUSY_BROADCASTING,
                    PAS_ERR_INVALID_ANNOUNCE,
                    PAS_ERR_NO_FLAG,
                    PAS_ERR_NO_FIRE);
