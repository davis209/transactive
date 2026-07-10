/**
 * The source code in this file is the property of
 * Ripple Systems and is not for redistribution
 * in any form.
 *
 * Source:   $File: //depot/C830AR_TIP/C830AR/transactive/app/pa/PAAgent/src/StationPasTableManager.cpp $
 * @author:  Ripple
 * @version: $Revision: #4 $
 *
 * Last modification: $DateTime: 2025/01/06 12:26:20 $
 * Last modified by:  $Author: limin.zhu $
 *
 */

#include "pch.h"
#include "StationPasTableManager.h"
#include "core/utilities/src/DebugUtilEx.h"

using namespace TA_Base_Core;

namespace TA_IRS_App
{
    StationPasTableManager::StationPasTableManager(unsigned long locationKey)
        : PasTableManager(locationKey)

    {
        FUNCTION_ENTRY("Constructor");

        m_table201 = new Table201(m_socketScheduler, m_processScheduler, *this);
        m_table204 = new Table204();
        m_table301 = new Table301(m_socketScheduler, m_processScheduler, *this);
        m_table303 = new Table303(locationKey);
        m_table200 = new Table200(m_table202, m_table302, m_table303);
        m_table610 = new Table610();
        m_table560 = new Table560(m_socketScheduler,
                                  m_processScheduler,
                                  m_table100,
                                  m_table202,
                                  m_table204,
                                  NULL,
                                  m_table303,
                                  NULL,
                                  m_table550,
                                  m_table570,
                                  m_table590,
                                  m_table603,
                                  m_table604,
                                  m_table610,
                                  NULL,
                                  *this);

        FUNCTION_EXIT;
    }

    Table200& StationPasTableManager::getTable200()
    {
        return *(m_table200);
    }

    Table1200& StationPasTableManager::getTable1200()
    {
        TA_ASSERT(false, "Not MasterPasTable instance.");
        return *(m_table1200);
    }

    Table201& StationPasTableManager::getTable201()
    {
        return *(m_table201);
    }

    Table1201& StationPasTableManager::getTable1201()
    {
        TA_ASSERT(false, "Not MasterPasTable instance.");
        return *(m_table1201);
    }

    Table204& StationPasTableManager::getTable204()
    {
        return *(m_table204);
    }

    Table1204& StationPasTableManager::getTable1204()
    {
        TA_ASSERT(false, "Not MasterPasTable instance.");
        return *(m_table1204);
    }

    Table301& StationPasTableManager::getTable301()
    {
        return *(m_table301);
    }

    Table1301& StationPasTableManager::getTable1301()
    {
        TA_ASSERT(false, "Not MasterPasTable instance.");
        return *(m_table1301);
    }

    Table303& StationPasTableManager::getTable303()
    {
        return *(m_table303);
    }

    Table1303& StationPasTableManager::getTable1303()
    {
        TA_ASSERT(false, "Not MasterPasTable instance.");
        return *(m_table1303);
    }

    Table610& StationPasTableManager::getTable610()
    {
        return *(m_table610);
    }

    Table1610& StationPasTableManager::getTable1610()
    {
        TA_ASSERT(false, "Not MasterPasTable instance.");
        return *(m_table1610);
    }

    ConnectToPAFEPC* StationPasTableManager::getConnectEvent()
    {
        LOG_INFO("Returning STATION connect event.");
        return new ConnectToStationPAFEPC(m_socketScheduler,
                                          m_processScheduler,
                                          *this,
                                          *(PASConnection::getInstance()));
    }

    InvalidatePASDataPoints* StationPasTableManager::getInvalidateEvent()
    {
        LOG_INFO("Returning STATION invalidate event.");
        return new InvalidateStationPASDataPoints(m_processScheduler, *this);
    }

    std::vector<AbstractWriteablePASTable*> StationPasTableManager::getAllWriteableTables()
    {
        std::vector<AbstractWriteablePASTable*> writeableTables;

        writeableTables.push_back(m_table201);
        writeableTables.push_back(m_table301);
        writeableTables.push_back(m_table350);
        writeableTables.push_back(m_table560);
        writeableTables.push_back(m_table570);
        writeableTables.push_back(m_table604);
        writeableTables.push_back(m_table630);

        return writeableTables;
    }

    // ExceptionChecked
    StationPasTableManager::~StationPasTableManager()
    {
        FUNCTION_ENTRY("Destructor");
        FUNCTION_EXIT;
    }

    void StationPasTableManager::setMonitor()
    {
        PasTableManager::setMonitor();
    }

    void StationPasTableManager::setControl()
    {
        PasTableManager::setControl();
    }

    //wuzhongyi CL-19136
    //Table301 Messages Sequence Broadcast Request
    void StationPasTableManager::forceTerminateAllPA()
    {
        LOG_SCOPE("StationPasTableManager::forceTerminateAllPA");
        FUNCTION_ENTRY("forceTerminateAllPA");

        TA_Base_Bus::IPAAgentCorbaDef::ZoneCoverage nullZones;
        nullZones.length(0);

        for (int messageSeqId = 1; messageSeqId <= 4; messageSeqId++)
        {
            try
            {
                //can trigger exception
                getTable301().setTableDataAndWrite(messageSeqId, 0, 0, 0, 0, false, nullZones);
                LOG_INFO("Table301 setTableDataAndWrite messageSeqId=%d  announceId=0 announceId=0 periodInsecs=0 starttime=0 stoptime=0 isEventTrigger=false", messageSeqId);
            }
            catch (TA_Base_Core::TransactiveException& err)
            {
                LOG_ERROR("Table301 setTableDataAndWrite ERROR messageSeqId=%d announceId=0 periodInsecs=0 starttime=0 stoptime=0 isEventTrigger=false", messageSeqId);
                LOG_EXCEPTION_CATCH(SourceInfo, "TransactiveException", err.what());
            }
            catch (...)
            {
                LOG_ERROR("unknown exception when Table301 setTableDataAndWrite messageSeqId=%d", messageSeqId);
            }
        }

        FUNCTION_EXIT;
    }
}
