/**
 * The source code in this file is the property of
 * Ripple Systems and is not for redistribution
 * in any form.
 *
 * Source:   $File: //depot/C830AR_TIP/C830AR/transactive/app/pa/PAAgent/src/Table200.cpp $
 * @author:  Ripple
 * @version: $Revision: #10 $
 *
 * Last modification: $DateTime: 2025/06/24 11:25:13 $
 * Last modified by:  $Author: limin.zhu $
 *
 */

#include "pch.h"
#include "PASConnection.h"
#include "Table200.h"
#include "PasHelpers.h"
#include "PasTableManager.h"
#include "Table202.h"
#include "Table302.h"
#include "Table303.h"
#include "bus/scada/datapoint_library/src/DataPoint.h"
#include "bus/pa_4669/pa_agent/IDL/src/IPAAgentCorbaDefDescribe.h"
#include "core/data_access_interface/src/LocationAccessFactoryEx.h"
#include "core/utilities/src/TAAssert.h"
#include "core/utilities/src/DebugUtilEx.h"
#include "core/utilities/src/MakeNameValuePairsString.h"
#include <time.h>

using namespace boost::describe::operators;
using namespace TA_Base_Core;
using namespace TA_IRS_Bus;

const size_t TABLE_590_SOURCE_ID_SUB_OFFSET = 0;
const size_t TABLE_590_STATUS_AND_PRIORITY_SUB_OFFSET = 1;

namespace TA_IRS_App
{
    Table200::Table200(Table202* table202, Table302* table302, Table303* table303)
        : AbstractPASTable(200),
        m_cachedMapsInstance(NULL),
        m_readWait(),
        m_initialThread(true)// TD17013
    {
        FUNCTION_ENTRY("Constructor");

        m_cachedMapsInstance = CachedMaps::getInstance();

        m_zoneStatuses.resize(MAXZONEID);

        m_table202 = table202;
        m_table302 = table302;
        m_table303 = table303;

        // jeffrey++ Bug457
        m_readIndex = 0;
        // ++jeffrey Bug457
        FUNCTION_EXIT;
    }

    Table200::~Table200()
    {
        FUNCTION_ENTRY("Destructor");
        FUNCTION_EXIT;
    }

    // TD 17013
    void Table200::setInitalThreadValue(bool initialThreadValue)
    {
        m_initialThread = initialThreadValue;
    }

    // TD 17013

    // TD 16749
    void Table200::processRead()
    {
        LOG_SCOPE("Table200::processRead");
        ThreadGuard guard(m_lock);

        for (int zone = 0; zone < MAXZONEID; ++zone)
        {
            // Zone data is expected to be in the following format
            //
            // Bit: 15 14 13 12 11 10 09 08 07  06 05 04 03 02 01 00
            //      |_____________________|  |  |__________________|
            //                |              |            |
            //           Source Id        Status       Priority
            //

            LOG_SCOPE(boost::format("ZONE[%d]") % (zone + 1));

            std::uint16_t zoneData = PasHelpers::instance().get16bit(m_buffer, zone * 2);

            auto sourceId = static_cast<TA_Base_Bus::IPAAgentCorbaDef::ESource>(zoneData >> 8);
            bool status = (zoneData & (0x0080)) != 0;
            CORBA::Octet priority = static_cast<CORBA::Octet>(zoneData & (0x007f));

            auto zoneStatus = ZoneStatus{sourceId, status, priority};
            auto& oldZoneStatus = m_zoneStatuses[zone];
            LOG_DEBUG_IF(zoneStatus != oldZoneStatus, "%s", nvps(oldZoneStatus, zoneStatus));

            // The datapoints will only update if the DataPointState has changed
            // (with the exception of just a timestamp change).  Therefore to
            // simplify the code, we update the datapoint regardless.

            // Update broadcast icon datapoints
            if (m_zoneStatuses[zone].m_sourceId != sourceId || m_initialThread) // TD17013
            {
                m_zoneStatuses[zone].m_sourceId = sourceId;

                TA_Base_Bus::DataPoint* liveDp   = m_cachedMapsInstance->getLiveDataPointFromZoneId(zone + 1);
                TA_Base_Bus::DataPoint* cyclicDp = m_cachedMapsInstance->getCyclicDataPointFromZoneId(zone + 1);
                TA_Base_Bus::DataPoint* musicDp  = m_cachedMapsInstance->getMusicDataPointFromZoneId(zone + 1);

                LOG_ERROR_IF_NOT(liveDp, "processRead(): can not find live datapoitn for zone id %d", zone + 1);
                LOG_ERROR_IF_NOT(cyclicDp, "processRead(): can not find cyclic datapoitn for zone id %d", zone + 1);
                LOG_ERROR_IF_NOT(musicDp, "processRead(): can not find music datapoitn for zone id %d", zone + 1);

                // toan++
                // TD-10381
                //
                // Fixed 23/06/2005: Set good quality to all broadcast datapoints
                //                   Except the fix mentioned above, there's no change
                //                   in the logic. The code was only made shorter by
                //                   moving out the duplicated parts to the end.
                //
                //                   Note: if source id is an unexpected value -> no update.
                //                   Don't know if this is correct. It is the original
                //                   logic.

                // determine the state of each broadcast
                bool bLive = is_live(sourceId);
                bool bMusic = is_bgm(sourceId);
                bool bDva = is_dva(sourceId);
                bool bCyclic = false;

                if (bDva)
                {
                    CORBA::Octet announcementId = m_table202->getAnnounceId(sourceId);
                    unsigned int sequenceId = m_table302->getSequenceId(announcementId);
                    bCyclic = m_table303->isMessageSequenceCyclic(sequenceId);
                }

                // bool bRecognisedSourceId = true;    // false if value of source id is unexpected
                bool bRecognisedSourceId = (0 == sourceId || bLive || bMusic || bDva);

                LOG_DEBUG_IF(bRecognisedSourceId, "processRead(): %s", nvps(bLive, bMusic, bCyclic));

                if (bRecognisedSourceId)
                {
                    // update the music, live and cyclic broadcast datapoints
                    if (cyclicDp)
                    {
                        TA_Base_Bus::DpValue dpValue(cyclicDp->getDataPointDataType(),
                                                     cyclicDp->getEnumLabelsMap(),
                                                     cyclicDp->getBooleanLabels());
                        dpValue.setBoolean(bCyclic);
                        TA_Base_Bus::DataPointState newState(dpValue, time(NULL), TA_Base_Bus::QUALITY_GOOD_NO_SPECIFIC_REASON);
                        cyclicDp->updateFieldState(newState);
                    }

                    if (liveDp)
                    {
                        TA_Base_Bus::DpValue dpValue(liveDp->getDataPointDataType(),
                                                     liveDp->getEnumLabelsMap(),
                                                     liveDp->getBooleanLabels());
                        dpValue.setBoolean(bLive);
                        TA_Base_Bus::DataPointState newState(dpValue, time(NULL), TA_Base_Bus::QUALITY_GOOD_NO_SPECIFIC_REASON);
                        liveDp->updateFieldState(newState);
                    }

                    if (musicDp)
                    {
                        TA_Base_Bus::DpValue dpValue(musicDp->getDataPointDataType(),
                                                     musicDp->getEnumLabelsMap(),
                                                     musicDp->getBooleanLabels());
                        dpValue.setBoolean(bMusic);
                        TA_Base_Bus::DataPointState newState(dpValue, time(NULL), TA_Base_Bus::QUALITY_GOOD_NO_SPECIFIC_REASON);
                        musicDp->updateFieldState(newState);
                    }
                }

                // ++toan
                // TD-10381
            }

            // update status datapoint
            if (m_zoneStatuses[zone].m_status != status || m_initialThread) // TD17013
            {
                LOG_DEBUG("processRead(): %s", nvps(status));
                m_zoneStatuses[zone].m_status = status;
                TA_Base_Bus::DataPoint* statusDp = m_cachedMapsInstance->getStatusDataPointFromZoneId(zone + 1);
                LOG_ERROR_IF_NOT(statusDp, "processRead(): can not find status datapoitn for zone id %d", zone + 1);

                if (statusDp)
                {
                    // PA Zone datapoint exists, so we can update it
                    TA_Base_Bus::DpValue dpValue(statusDp->getDataPointDataType(),
                                                 statusDp->getEnumLabelsMap(),
                                                 statusDp->getBooleanLabels());

                    dpValue.setBoolean(status);

                    // set up DataPointState with the default timestamp and quality
                    TA_Base_Bus::DataPointState newState(dpValue, time(NULL), TA_Base_Bus::QUALITY_GOOD_NO_SPECIFIC_REASON);

                    statusDp->updateFieldState(newState);
                }
            }

            // update priority datapoint
            if (m_zoneStatuses[zone].m_priority != priority || m_initialThread) // TD17013
            {
                LOG_DEBUG("processRead(): %s", nvps(priority));
                m_zoneStatuses[zone].m_priority = priority;
                TA_Base_Bus::DataPoint* priorityDp = m_cachedMapsInstance->getPriorityDataPointFromZoneId(zone + 1);

                if (priorityDp)
                {
                    // PA Zone datapoint exists, so we can update it
                    TA_Base_Bus::DpValue dpValue(priorityDp->getDataPointDataType(),
                                                 priorityDp->getEnumLabelsMap(),
                                                 priorityDp->getBooleanLabels());

                    dpValue.setFloat(priority, priorityDp->getPrecision());

                    // set up DataPointState with the default timestamp and quality
                    TA_Base_Bus::DataPointState newState(dpValue, time(NULL), TA_Base_Bus::QUALITY_GOOD_NO_SPECIFIC_REASON);

                    priorityDp->updateFieldState(newState);
                }
            }
        }

        // jeffrey++ Bug457
        m_readIndex++;
        // ++jeffrey Bug457
        m_readWait.broadcast();

        m_initialThread = false;// TD17013
    }

    // TD 16749

    // ++ Bug456
    void Table200::readTimedWait()
    {
        m_readWait.timedWait(CachedConfig::getInstance()->getFastPollRateInMSecs());
    }

    // ++ Bug456

    ZoneStatus Table200::getZoneStatus(unsigned int zoneId)
        //throw ( TA_Base_Core::PasConnectionException )
    {
        TA_ASSERT(zoneId > 0 && zoneId <= MAXZONEID, "zoneId is out of range");

        // ++ Bug456
        //m_readWait.timedWait( CachedConfig::getInstance()->getFastPollRateInMSecs() );
        // ++ Bug456

        ThreadGuard guard(m_lock);

        return m_zoneStatuses[zoneId - 1];
    }

    void invalidateDataPoint(TA_Base_Bus::DataPoint& datapoint)
    {
        LOG_SCOPE("invalidateDataPoint");
        TA_Base_Bus::DpValue dpValue = datapoint.getCurrentValue();
        TA_Base_Bus::DataPointState newState(dpValue, time(NULL), TA_Base_Bus::QUALITY_BAD_COMM_FAILURE);
        datapoint.updateFieldState(newState);
    }

    void Table200::invalidateAllDataPoints()
    {
        LOG_SCOPE("Table200::invalidateAllDataPoints");

        for (int zone = 0; zone < MAXZONEID; ++zone)
        {
            try
            {
                TA_Base_Bus::DataPoint* liveDp   = m_cachedMapsInstance->getLiveDataPointFromZoneId(zone + 1);

                if (liveDp)
                {
                    invalidateDataPoint(*liveDp);
                }

                TA_Base_Bus::DataPoint* musicDp  = m_cachedMapsInstance->getMusicDataPointFromZoneId(zone + 1);

                if (musicDp)
                {
                    invalidateDataPoint(*musicDp);
                }

                TA_Base_Bus::DataPoint* cyclicDp = m_cachedMapsInstance->getCyclicDataPointFromZoneId(zone + 1);

                if (cyclicDp)
                {
                    invalidateDataPoint(*cyclicDp);
                }

                TA_Base_Bus::DataPoint* statusDp = m_cachedMapsInstance->getStatusDataPointFromZoneId(zone + 1);

                if (statusDp)
                {
                    invalidateDataPoint(*statusDp);
                }

                TA_Base_Bus::DataPoint* priorityDp = m_cachedMapsInstance->getPriorityDataPointFromZoneId(zone + 1);

                if (priorityDp)
                {
                    invalidateDataPoint(*priorityDp);
                }
            }
            catch (...)
            {
                // Carry on
            }
        }
    }

    ReadTable200::ReadTable200(Table200& table,
                               TA_IRS_Bus::Scheduler& socketScheduler,
                               TA_IRS_Bus::Scheduler& processScheduler,
                               IPasEventSource& eventSource)
        : PASReadEvent(socketScheduler, processScheduler, eventSource)
        , m_table(table)
    {
    }

    ReadTable200::ReadTable200(ACE_Time_Value expiryTime,
                               Table200& table,
                               TA_IRS_Bus::Scheduler& socketScheduler,
                               TA_IRS_Bus::Scheduler& processScheduler,
                               IPasEventSource& eventSource)
        : PASReadEvent(expiryTime, socketScheduler, processScheduler, eventSource)
        , m_table(table)
    {
    }

    void ReadTable200::readTable()
    {
        LOG_SCOPE("ReadTable200::readTable");
        ThreadGuard guard(m_table.m_lock);

        PASConnection::getInstance()->readTable(m_table.TABLE_NUMBER,
                                                m_table.m_buffer,
                                                m_table.BUFFER_SIZE);

        m_socketScheduler.post(new ReadTable200(getNextTimeToExpire(),
                                                m_table,
                                                m_socketScheduler,
                                                m_processScheduler,
                                                m_pasEventSource));

        m_processScheduler.post(new ProcessTable200(m_table));
    }

    ProcessTable200::ProcessTable200(Table200& table)
        : m_table(table)
    {
    }

    void ProcessTable200::consume()
    {
        LOG_SCOPE("ProcessTable200::consume");
        m_table.processRead();
        PasTableReadCounter::instance().increase(m_table.TABLE_NUMBER);

        delete this;
    }

    void ProcessTable200::cancel()
    {
        LOG_SCOPE("ProcessTable200::cancel");
        delete this;
    }

    InvalidateTable200::InvalidateTable200(Table200& table)
        : m_table(table)
    {
    }

    void InvalidateTable200::consume()
    {
        LOG_SCOPE("InvalidateTable200::consume");

        m_table.invalidateAllDataPoints();

        delete this;
    }

    void InvalidateTable200::cancel()
    {
        LOG_SCOPE("InvalidateTable200::cancel");
        delete this;
    }
}
