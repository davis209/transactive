/**
 * The source code in this file is the property of
 * Ripple Systems and is not for redistribution
 * in any form.
 *
 * Source:   $File: //depot/C830AR_TIP/C830AR/transactive/app/pa/PAAgent/src/Table610.cpp $
 * @author:  Ripple
 * @version: $Revision: #5 $
 *
 * Last modification: $DateTime: 2025/06/24 11:25:13 $
 * Last modified by:  $Author: limin.zhu $
 *
 */

#include "pch.h"
#include "Table610.h"
#include "PasHelpers.h"
#include "PasTableManager.h"
#include "PASConnection.h"
#include "CachedConfig.h"
#include "core/utilities/src/TAAssert.h"
#include "core/utilities/src/DebugUtilEx.h"
#include "core/message/src/MessagePublicationManager.h"
#include "core/message/types/PAAgentComms_MessageTypes.h"
#include "core/message/src/CommsMessageSender.h"

using namespace TA_Base_Core;
using namespace TA_Base_Bus;

const size_t TABLE_610_PRIVATE_ADHOC_1_OFFSET       = 0;
const size_t TABLE_610_PRIVATE_ADHOC_2_OFFSET       = 6;
const size_t TABLE_610_PRIVATE_ADHOC_3_OFFSET       = 12;
const size_t TABLE_610_PRIVATE_ADHOC_4_OFFSET       = 18;
const size_t TABLE_610_PRIVATE_ADHOC_5_OFFSET       = 24;
const size_t TABLE_610_PRIVATE_ADHOC_6_OFFSET       = 30;
const size_t TABLE_610_PRIVATE_PRERECORDED_OFFSET   = 36;

const size_t TABLE_610_PUBLIC_ADHOC_1_OFFSET        = 42;
const size_t TABLE_610_PUBLIC_ADHOC_2_OFFSET        = 48;
const size_t TABLE_610_PUBLIC_ADHOC_3_OFFSET        = 54;
const size_t TABLE_610_PUBLIC_ADHOC_4_OFFSET        = 60;
const size_t TABLE_610_PUBLIC_ADHOC_5_OFFSET        = 66;
const size_t TABLE_610_PUBLIC_ADHOC_6_OFFSET        = 72;
const size_t TABLE_610_PUBLIC_PRERECORDED_OFFSET    = 78;

namespace TA_IRS_App
{
    Table610::Table610()
        : AbstractPASTable(610)
    {
        FUNCTION_ENTRY("Constructor");
        FUNCTION_EXIT;
    }

    void Table610::processRead()
    {
        LOG_SCOPE("Table610::processRead");

        ThreadGuard guard(m_lock);

        // TA_IRS_Bus::DvaVersionRecord currentDvaVersionRecord;
        TA_IRS_Bus::DvaPrivateVersionRecord currentDvaVersionRecord;

        try
        {
            // currentDvaVersionRecord = TA_IRS_Bus::CachedMaps::getInstance()->
            //     getDvaVersionRecordFromLocationKey(CachedConfig::getInstance()->getAgentLocationKey());
            currentDvaVersionRecord = TA_IRS_Bus::CachedMaps::getInstance()->getDvaPrivateVersionRecordFromLocationKey(CachedConfig::getInstance()->getAgentLocationKey());
        }
        catch (TA_Base_Core::CachedMappingNotFoundException&)
        {
            // Record does not exist yet.  We will create one as soon as data arrives
            // The m_localDvaVersionRecord is initialised by the default constructor
        }

        bool dataChanged = false;

        dataChanged |= processOneVersionId(currentDvaVersionRecord.m_privateAdhoc1, TABLE_610_PRIVATE_ADHOC_1_OFFSET);
        dataChanged |= processOneVersionId(currentDvaVersionRecord.m_privateAdhoc2, TABLE_610_PRIVATE_ADHOC_2_OFFSET);
        dataChanged |= processOneVersionId(currentDvaVersionRecord.m_privateAdhoc3, TABLE_610_PRIVATE_ADHOC_3_OFFSET);
        dataChanged |= processOneVersionId(currentDvaVersionRecord.m_privateAdhoc4, TABLE_610_PRIVATE_ADHOC_4_OFFSET);
        dataChanged |= processOneVersionId(currentDvaVersionRecord.m_privateAdhoc5, TABLE_610_PRIVATE_ADHOC_5_OFFSET);
        dataChanged |= processOneVersionId(currentDvaVersionRecord.m_privateAdhoc6, TABLE_610_PRIVATE_ADHOC_6_OFFSET);
        dataChanged |= processOneVersionId(currentDvaVersionRecord.m_privatePreRecorded, TABLE_610_PRIVATE_PRERECORDED_OFFSET);

        /*
        dataChanged |= processOneVersionId(currentDvaVersionRecord.m_publicAdhoc1, TABLE_610_PUBLIC_ADHOC_1_OFFSET);
        dataChanged |= processOneVersionId(currentDvaVersionRecord.m_publicAdhoc2, TABLE_610_PUBLIC_ADHOC_2_OFFSET);
        dataChanged |= processOneVersionId(currentDvaVersionRecord.m_publicAdhoc3, TABLE_610_PUBLIC_ADHOC_3_OFFSET);
        dataChanged |= processOneVersionId(currentDvaVersionRecord.m_publicAdhoc4, TABLE_610_PUBLIC_ADHOC_4_OFFSET);
        dataChanged |= processOneVersionId(currentDvaVersionRecord.m_publicAdhoc5, TABLE_610_PUBLIC_ADHOC_5_OFFSET);
        dataChanged |= processOneVersionId(currentDvaVersionRecord.m_publicAdhoc6, TABLE_610_PUBLIC_ADHOC_6_OFFSET);
        dataChanged |= processOneVersionId(currentDvaVersionRecord.m_publicPreRecorded, TABLE_610_PUBLIC_PRERECORDED_OFFSET);
        */
        if (dataChanged)
        {
            updateDatabaseAndBroadcastUpdates(currentDvaVersionRecord);
        }
    }

    void Table610::updateDatabaseAndBroadcastUpdates(TA_IRS_Bus::DvaPrivateVersionRecord& currentDvaVersionRecord)
        //throw (TA_Base_Core::DatabaseException)
    {
        LOG_SCOPE("Table610::updateDatabaseAndBroadcastUpdates");

        try
        {
            TA_IRS_Bus::CachedMaps::getInstance()->setDvaMessagePrivateVersionRecord(currentDvaVersionRecord, true);
        }
        catch (TA_Base_Core::DataException&)
        {
            // ToDo - make this impossible
            TA_ASSERT(false, "Should not get here");
        }

        // Database exception will slip through
        // Theres no point in sending any update message until the dabase write is
        // successful

        // Broadcast update to Managers:

        TA_Base_Bus::IPAAgentCorbaDef::DvaPrivateVersionRecords* newDvaVersionRecords = new TA_Base_Bus::IPAAgentCorbaDef::DvaPrivateVersionRecords;
        newDvaVersionRecords->length(1);
        (*newDvaVersionRecords)[0].m_pKey          = currentDvaVersionRecord.m_pKey;
        (*newDvaVersionRecords)[0].m_locationKey   = currentDvaVersionRecord.m_locationKey;
        (*newDvaVersionRecords)[0].m_privateAdhoc1 = currentDvaVersionRecord.m_privateAdhoc1.c_str();
        (*newDvaVersionRecords)[0].m_privateAdhoc2 = currentDvaVersionRecord.m_privateAdhoc2.c_str();
        (*newDvaVersionRecords)[0].m_privateAdhoc3 = currentDvaVersionRecord.m_privateAdhoc3.c_str();
        (*newDvaVersionRecords)[0].m_privateAdhoc4 = currentDvaVersionRecord.m_privateAdhoc4.c_str();
        (*newDvaVersionRecords)[0].m_privateAdhoc5 = currentDvaVersionRecord.m_privateAdhoc5.c_str();
        (*newDvaVersionRecords)[0].m_privateAdhoc6 = currentDvaVersionRecord.m_privateAdhoc6.c_str();
        (*newDvaVersionRecords)[0].m_privatePreRecorded = currentDvaVersionRecord.m_privatePreRecorded.c_str();

        if (!CachedConfig::getInstance()->getIsMaster())
        {
            // We avoid sending updates if we are the Master PA Agent since this
            // is detailed in table 1610

            LOG_INFO("Sending DvaVersionsUpdate message to subscribers");

            CORBA::Any data;

            data <<= newDvaVersionRecords;

            m_paAgentCommsSender->sendCommsMessage(
                PAAgentComms::DvaVersionsUpdate,                        // Context::Type
                CachedConfig::getInstance()->getAgentKey(),             // EntityKey
                data,                                                   // Data
                CachedConfig::getInstance()->getAgentSubsystemKey(),    // Subsystem
                CachedConfig::getInstance()->getAgentLocationKey(),     // LocationKey
                NULL);
        }
    }

    bool Table610::processOneVersionId(std::string& dvaVersionId, unsigned long offset)
    {
        // Guarded in processRead()

        unsigned short day     =  PasHelpers::instance().get8bit(m_buffer, offset + 0);
        unsigned short month   =  PasHelpers::instance().get8bit(m_buffer, offset + 1);
        unsigned short year    =  PasHelpers::instance().get8bit(m_buffer, offset + 2);
        unsigned short hour    =  PasHelpers::instance().get8bit(m_buffer, offset + 3);
        unsigned short minute  =  PasHelpers::instance().get8bit(m_buffer, offset + 4);
        unsigned short version =  PasHelpers::instance().get8bit(m_buffer, offset + 5);

        std::ostringstream versionString;

        // jeffrey++ TD10338
        //versionString << day  << "/" << month  << "/" << year << " "
        //              << hour << ":" << minute << " "
        //              << version;
        versionString << day << "." << month << "." << year << "."
            << hour << "." << minute << "."
            << version;
        // ++jeffrey TD10338

        if (dvaVersionId != versionString.str())
        {
            // Data has been updated
            dvaVersionId = versionString.str();
            return true;
        }

        // No change
        return false;
    }

    ReadTable610::ReadTable610(Table610& table,
                               Table560& table560,
                               TA_IRS_Bus::Scheduler& socketScheduler,
                               TA_IRS_Bus::Scheduler& processScheduler,
                               IPasEventSource& eventSource)
        : PASReadEvent(socketScheduler, processScheduler, eventSource)
        , m_table(table)
        , m_table560(table560)
    {
    }

    void ReadTable610::readTable()
    {
        LOG_SCOPE("ReadTable610::readTable");

        ThreadGuard guard(m_table.m_lock);

        PASConnection::getInstance()->readTable(m_table.TABLE_NUMBER,
                                                m_table.m_buffer,
                                                m_table.BUFFER_SIZE);

        m_processScheduler.post(new ProcessTable610(m_table));

        m_table560.resetFlags(this,
                              Table560::TABLE_560_TABLE_610_FLAG_A_BIT_OFFSET,
                              Table560::TABLE_560_TABLE_610_FLAG_B_BIT_OFFSET);
    }

    ProcessTable610::ProcessTable610(Table610& table)
        : m_table(table)
    {
    }

    void ProcessTable610::consume()
    {
        LOG_SCOPE("ProcessTable610::consume");

        m_table.processRead();
        PasTableReadCounter::instance().increase(m_table.TABLE_NUMBER);

        delete this;
    }

    void ProcessTable610::cancel()
    {
        LOG_SCOPE("ProcessTable610::cancel");
        delete this;
    }
}
