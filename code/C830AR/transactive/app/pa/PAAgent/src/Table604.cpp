/**
 * The source code in this file is the property of
 * Ripple Systems and is not for redistribution
 * in any form.
 *
 * Source:   $File: //depot/C830AR_TIP/C830AR/transactive/app/pa/PAAgent/src/Table604.cpp $
 * @author:  Ripple
 * @version: $Revision: #5 $
 *
 * Last modification: $DateTime: 2025/06/24 11:25:13 $
 * Last modified by:  $Author: limin.zhu $
 *
 */

#include "pch.h"
#include "Table604.h"
#include "PasHelpers.h"
#include "PasTableManager.h"
#include "PASConnection.h"
#include "CachedConfig.h"
#include "core/utilities/src/TAAssert.h"
#include "core/utilities/src/DebugUtilEx.h"
#include "core/message/src/MessagePublicationManager.h"
#include "core/message/types/PAAgentComms_MessageTypes.h"
#include "core/message/src/CommsMessageSender.h"
#include <malloc.h>

using namespace TA_Base_Core;
using namespace TA_Base_Bus;

namespace TA_IRS_App
{
    Table604::Table604(TA_IRS_Bus::Scheduler& socketScheduler,
                       TA_IRS_Bus::Scheduler& processScheduler,
                       unsigned long locationKey,
                       IPasEventSource& eventSource)
        : AbstractWriteablePASTable(604)
        , m_minMessageId(0)
        , m_maxMessageId(0)
        , m_locationKey(locationKey)
        , m_isSynchronised(false)
        , m_socketScheduler(socketScheduler)
        , m_processScheduler(processScheduler)
        , m_pasEventSource(eventSource)
    {
        FUNCTION_ENTRY("Constructor");

        // Cached these throughout the life of the application to save accessing
        // the CachedConfig locks all the time

        if (CachedConfig::getInstance()->getIsMaster() || CachedConfig::getInstance()->getIsDepot())
        {
            // 1-6
            m_minMessageId = 1;
            m_maxMessageId = NBADHOC;
        }
        else
        {
            // Station 501-506
            m_minMessageId = NBADHOC + NBPREREC + 1;
            m_maxMessageId = NBADHOC + NBPREREC + NBADHOC;
        }

        FUNCTION_EXIT;
    }

    void Table604::processRead()
        //throw (TA_Base_Core::DatabaseException, TA_Base_Core::PasConnectionException)
    {
        LOG_SCOPE("Table604::processRead");

        // No raw data process required since data is simple enough to be interpreted
        // straight out of the buffer

        if (m_isSynchronised)
        {
            // Don't update the database if ISCS-PAS have not been synchronised
            // since startup.  This is especially used to prevent the database
            // write when synchronise() calls read().
            updateDatabaseAndBroadcastUpdates();
        }

        // Maochun Sun++
        // TD13413
        else
        {
            synchronise();
        }

        // ++Maochun Sun
        // TD13413
    }

    void Table604::updateDatabaseAndBroadcastUpdates()
        //throw (TA_Base_Core::DatabaseException, TA_Base_Core::PasConnectionException)
    {
        LOG_SCOPE("Table604::updateDatabaseAndBroadcastUpdates");

        // We check if there is any inconsistency between the new data and the
        // data in the CachedMaps

        TA_Base_Bus::IPAAgentCorbaDef::DvaMessagesUpdateData* disposableUpdateData = NULL;
        int numberOfChanges = 0;

        try
        {
            ThreadGuard guard(m_lock);

            for (int messageId = m_minMessageId; messageId <= m_maxMessageId; ++messageId)
            {
                unsigned long messageKey(0);
                TA_IRS_Bus::StationDvaMessageRecord record;
                char newAdhocType(0);

                try
                {
                    messageKey = TA_IRS_Bus::CachedMaps::getInstance()->getKeyFromStationDvaMessageIdAndLocation(messageId, m_locationKey);
                    record = TA_IRS_Bus::CachedMaps::getInstance()->getStationDvaMessageRecordFromKey(messageKey);
                }
                catch (TA_Base_Core::CachedMappingNotFoundException)
                {
                    // Create a new record

                    record.m_id = messageId;
                    record.m_label = "";
                    record.m_locationKey = m_locationKey;
                    record.m_pKey = TA_Base_Core::DatabaseKey::getInvalidKey();
                    record.m_tisMessageTag = TA_Base_Core::DatabaseKey::getInvalidKey();
                    record.m_tisLibrarySection = TA_Base_Core::DatabaseKey::getInvalidKey();
                    record.m_tisLibraryVersion = TA_Base_Core::DatabaseKey::getInvalidKey();
                    record.m_type = 0;
                    // We deliberately nullify record.m_type here so the code below will be called
                }

                if (getAdhocType(messageId) != record.m_type)
                {
                    char oldType = record.m_type;
                    std::string oldLabel = record.m_label;

                    record.m_type = getAdhocType(messageId);

                    if (record.m_type == 'X')
                    {
                        // Nullify the message label only if the message is deleted
                        record.m_label = "";
                    }

                    try
                    {
                        TA_IRS_Bus::CachedMaps::getInstance()->setStationDvaMessageRecord(record, true);
                    }
                    catch (...)
                    {
                        // Only expecting DataException or DataBaseException
                        // Failed to write the changes - see if we can process the rest

                        // ToDo: submit alarm
                        continue;
                    }

                    PasHelpers::instance().submitPaAgentDVAMessageUpdateAuditMessage(
                        oldType,
                        record.m_type,
                        oldLabel,
                        record.m_label,
                        record.m_pKey);

                    // Detected at least one change
                    ++numberOfChanges;

                    if (!disposableUpdateData)
                    {
                        // First time so we initialise with the max possible size
                        // expected and then later crop the sequence
                        disposableUpdateData = new TA_Base_Bus::IPAAgentCorbaDef::DvaMessagesUpdateData(NBPREREC);
                        disposableUpdateData->length(NBPREREC);
                    }

                    (*disposableUpdateData)[numberOfChanges - 1].m_pKey               = record.m_pKey;
                    (*disposableUpdateData)[numberOfChanges - 1].m_id                 = record.m_id;
                    (*disposableUpdateData)[numberOfChanges - 1].m_locationKey        = record.m_locationKey;
                    (*disposableUpdateData)[numberOfChanges - 1].m_label              = record.m_label.c_str();
                    (*disposableUpdateData)[numberOfChanges - 1].m_type               = record.m_type;
                    (*disposableUpdateData)[numberOfChanges - 1].m_tisMessageTag      = record.m_tisMessageTag;
                    (*disposableUpdateData)[numberOfChanges - 1].m_tisLibrarySection  = record.m_tisLibrarySection;
                    (*disposableUpdateData)[numberOfChanges - 1].m_tisLibraryVersion  = record.m_tisLibraryVersion;
                }
            } // for loop

            if (disposableUpdateData)
            {
                // Detected at least one change, so we broadcast it to all open PA Managers

                // Reset the length of the disposableUpdateData vector so it only contains
                // the actual number of changes.  This is for the client to know.
                disposableUpdateData->length(numberOfChanges);

                LOG_INFO("Sending DvaMessagesUpdate message to subscribers");

                CORBA::Any data;

                data <<= disposableUpdateData;

                m_paAgentCommsSender->sendCommsMessage(
                    PAAgentComms::DvaMessagesUpdate,                        // Context::Type
                    CachedConfig::getInstance()->getAgentKey(),             // EntityKey
                    data,                                                   // Data
                    CachedConfig::getInstance()->getAgentSubsystemKey(),    // Subsystem
                    CachedConfig::getInstance()->getAgentLocationKey(),     // LocationKey
                    NULL);
            }
        }
        // Cleanup in case of exceptions thrown
        catch (TA_Base_Core::DataException&)
        {
            // ToDo - make this impossible
            TA_ASSERT(false, "Should not get here");
        }
        catch (...)
        {
            // Expecting TA_Base_Core::DatabaseException and TA_Base_Core::PasConnectionException

            if (disposableUpdateData)
            {
                delete disposableUpdateData;
                disposableUpdateData = NULL;
            }

            // To be caught and be retried in the next update
            throw;
        }
    }

    void Table604::setTableDataAndWrite(unsigned int messageKey, char messageType)
        //throw ( TA_Base_Core::PasWriteErrorException, TA_Base_Core::PasConnectionException, TA_Base_Core::DatabaseException )
    {
        LOG_SCOPE("Table604::setTableDataAndWrite");

        int adhocOffset = PasHelpers::instance().getAdhocOffset(messageKey);

        if (adhocOffset == -1)
        {
            // Somehow not found
            return;
        }

        TA_ASSERT(adhocOffset < NBADHOC, "adhocOffset out of bounds");

        ThreadGuard guardWrite(m_lock);

        PasHelpers::instance().set8bit(m_buffer, adhocOffset, messageType);

        WriteTable604* event = new WriteTable604(*this,
                                                 m_socketScheduler,
                                                 m_processScheduler,
                                                 m_pasEventSource);
        addWriteEvent(*event);
        m_socketScheduler.post(event);
    }

    char Table604::getAdhocType(unsigned int messageId)
        //throw ( TA_Base_Core::PasConnectionException )
    {
        // Guarded in updateDatabaseAndBroadcastUpdates()

        unsigned int adhocOffset = messageId - m_minMessageId;

        TA_ASSERT(adhocOffset < NBADHOC, "adhocOffset out of bounds");

        ThreadGuard guard(m_lock);

        char messageType = PasHelpers::instance().get8bit(m_buffer, adhocOffset);

        // Verify type
        if (messageType != 'E' &&
            messageType != 'N' &&
            messageType != 'X')
        {
            // Force to the default normal type in case of error
            messageType = 'N';
        }

        return messageType;
    }

    void Table604::synchronise()
        //throw ( TA_Base_Core::PasWriteErrorException, TA_Base_Core::PasConnectionException, TA_Base_Core::DatabaseException );
    {
        for (int messageId = m_minMessageId; messageId <= m_maxMessageId; ++messageId)
        {
            unsigned long messageKey(0);
            TA_IRS_Bus::StationDvaMessageRecord record;

            try
            {
                messageKey = TA_IRS_Bus::CachedMaps::getInstance()->getKeyFromStationDvaMessageIdAndLocation(messageId, m_locationKey);
                record = TA_IRS_Bus::CachedMaps::getInstance()->getStationDvaMessageRecordFromKey(messageKey);
            }
            catch (TA_Base_Core::CachedMappingNotFoundException)
            {
                // ignore and process next message
                continue;
            }

            // Note getAdhocType will probably call read() and then processRead().
            // Note processRead() will not write to the database because it checks
            // the m_isSynchronised flag first.

            if (record.m_type != getAdhocType(messageId))
            {
                setTableDataAndWrite(messageKey, record.m_type);
            }
        }

        m_isSynchronised = true;
        return;
    }

    ReadTable604::ReadTable604(Table604& table,
                               Table560& table560,
                               TA_IRS_Bus::Scheduler& socketScheduler,
                               TA_IRS_Bus::Scheduler& processScheduler,
                               IPasEventSource& eventSource)
        : PASReadEvent(socketScheduler, processScheduler, eventSource)
        , m_table(table)
        , m_table560(table560)
    {
    }

    void ReadTable604::readTable()
    {
        LOG_SCOPE("ReadTable604::readTable");

        ThreadGuard guard(m_table.m_lock);

        PASConnection::getInstance()->readTable(m_table.TABLE_NUMBER,
                                                m_table.m_buffer,
                                                m_table.BUFFER_SIZE);

        m_processScheduler.post(new ProcessTable604(m_table));

        m_table560.resetFlags(this,
                              Table560::TABLE_560_TABLE_604_FLAG_A_BIT_OFFSET,
                              Table560::TABLE_560_TABLE_604_FLAG_B_BIT_OFFSET);
    }

    ProcessTable604::ProcessTable604(Table604& table)
        : m_table(table)
    {
    }

    void ProcessTable604::consume()
    {
        LOG_SCOPE("ProcessTable604::consume");

        m_table.processRead();
        PasTableReadCounter::instance().increase(m_table.TABLE_NUMBER);

        delete this;
    }

    void ProcessTable604::cancel()
    {
        LOG_SCOPE("ProcessTable604::cancel");
        delete this;
    }

    WriteTable604::WriteTable604(Table604& table,
                                 TA_IRS_Bus::Scheduler& socketScheduler,
                                 TA_IRS_Bus::Scheduler& processScheduler,
                                 IPasEventSource& eventSource)
        : PASWriteEvent(table, socketScheduler, processScheduler, eventSource)
        , m_table(table)
    {
    }

    WriteTable604::WriteTable604(ACE_Time_Value timeToExpire,
                                 Table604& table,
                                 TA_IRS_Bus::Scheduler& socketScheduler,
                                 TA_IRS_Bus::Scheduler& processScheduler,
                                 IPasEventSource& eventSource)
        : PASWriteEvent(timeToExpire, table, socketScheduler, processScheduler, eventSource)
        , m_table(table)
    {
    }

    void WriteTable604::writeTable()
    {
        LOG_SCOPE("WriteTable604::writeTable");

        ThreadGuard guard(m_table.m_lock);

        PASConnection::getInstance()->writeTable(m_table.TABLE_NUMBER,
                                                 m_table.m_buffer,
                                                 m_table.BUFFER_SIZE);
    }

    PASWriteEvent* WriteTable604::recreateEvent(ACE_Time_Value timeToExpire)
    {
        return new WriteTable604(timeToExpire,
                                 m_table,
                                 m_socketScheduler,
                                 m_processScheduler,
                                 m_pasEventSource);
    }
}
