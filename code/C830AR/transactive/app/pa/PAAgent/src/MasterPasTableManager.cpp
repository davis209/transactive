/**
 * The source code in this file is the property of
 * Ripple Systems and is not for redistribution
 * in any form.
 *
 * Source:   $File: //depot/C830AR_TIP/C830AR/transactive/app/pa/PAAgent/src/MasterPasTableManager.cpp $
 * @author:  Ripple
 * @version: $Revision: #4 $
 *
 * Last modification: $DateTime: 2025/01/06 12:26:20 $
 * Last modified by:  $Author: limin.zhu $
 *
 */

#include "pch.h"
#include "MasterPasTableManager.h"
#include "core/utilities/src/DebugUtilEx.h"

using namespace TA_Base_Core;

namespace TA_IRS_App
{
    // ExceptionChecked
    MasterPasTableManager::MasterPasTableManager(unsigned long locationKey)
        : PasTableManager(locationKey)
    {
        FUNCTION_ENTRY("Constructor");

        m_table1201 = new Table1201(m_socketScheduler, m_processScheduler, *this);
        m_table1204 = new Table1204();
        m_table1301 = new Table1301(m_socketScheduler, m_processScheduler, *this);
        m_table1303 = new Table1303(locationKey);
        m_table1200 = new Table1200(m_table202, m_table302, m_table1303);
        m_table1610 = new Table1610();
        m_table560  = new Table560(m_socketScheduler,
                                   m_processScheduler,
                                   m_table100,
                                   m_table202,
                                   NULL,
                                   m_table1204,
                                   NULL,
                                   m_table1303,
                                   m_table550,
                                   m_table570,
                                   m_table590,
                                   m_table603,
                                   m_table604,
                                   NULL,
                                   m_table1610,
                                   *this);

        FUNCTION_EXIT;
    }

    Table200& MasterPasTableManager::getTable200()
    {
        TA_ASSERT(false, "Not StationPasTable instance.");
        return *(m_table200);
    }

    Table1200& MasterPasTableManager::getTable1200()
    {
        return *(m_table1200);
    }

    Table201& MasterPasTableManager::getTable201()
    {
        TA_ASSERT(false, "Not StationPasTable instance.");
        return *(m_table201);
    }

    Table1201& MasterPasTableManager::getTable1201()
    {
        return *(m_table1201);
    }

    Table204& MasterPasTableManager::getTable204()
    {
        TA_ASSERT(false, "Not StationPasTable instance.");
        return *(m_table204);
    }

    Table1204& MasterPasTableManager::getTable1204()
    {
        return *(m_table1204);
    }

    Table301& MasterPasTableManager::getTable301()
    {
        TA_ASSERT(false, "Not StationPasTable instance.");
        return *(m_table301);
    }

    Table1301& MasterPasTableManager::getTable1301()
    {
        return *(m_table1301);
    }

    Table303& MasterPasTableManager::getTable303()
    {
        TA_ASSERT(false, "Not StationPasTable instance.");
        return *(m_table303);
    }

    Table1303& MasterPasTableManager::getTable1303()
    {
        return *(m_table1303);
    }

    Table610& MasterPasTableManager::getTable610()
    {
        TA_ASSERT(false, "Not StationPasTable instance.");
        return *(m_table610);
    }

    Table1610& MasterPasTableManager::getTable1610()
    {
        return *(m_table1610);
    }

    ConnectToPAFEPC* MasterPasTableManager::getConnectEvent()
    {
        LOG_INFO("Returning MASTER connect event.");
        return new ConnectToMasterPAFEPC(m_socketScheduler,
                                         m_processScheduler,
                                         *this,
                                         *(PASConnection::getInstance()));
    }

    InvalidatePASDataPoints* MasterPasTableManager::getInvalidateEvent()
    {
        LOG_INFO("Returning MASTER invalidate event.");
        return new InvalidateMasterPASDataPoints(m_processScheduler, *this);
    }

    std::vector<AbstractWriteablePASTable*> MasterPasTableManager::getAllWriteableTables()
    {
        std::vector<AbstractWriteablePASTable*> writeableTables;

        writeableTables.push_back(m_table1201);
        writeableTables.push_back(m_table1301);
        writeableTables.push_back(m_table350);
        writeableTables.push_back(m_table560);
        writeableTables.push_back(m_table570);
        writeableTables.push_back(m_table604);
        writeableTables.push_back(m_table630);

        return writeableTables;
    }

    // ExceptionChecked
    MasterPasTableManager::~MasterPasTableManager()
    {
        FUNCTION_ENTRY("Destructor");

        FUNCTION_EXIT;
    }

    void MasterPasTableManager::setMonitor()
    {
        PasTableManager::setMonitor();
    }

    void MasterPasTableManager::setControl()
    {
        PasTableManager::setControl();
    }

    //wuzhongyi CL-19136
    //Table1301 Global Messages Sequence Broadcast Request
    void MasterPasTableManager::forceTerminateAllPA()
    {
        LOG_SCOPE("MasterPasTableManager::forceTerminateAllPA");
        FUNCTION_ENTRY("forceTerminateAllPA");

        TA_Base_Bus::IPAAgentCorbaDef::ZoneCoverage nullZones;
        nullZones.length(0);

        for (int messageSeqId = 1; messageSeqId <= 4; messageSeqId++)
        {
            //can trigger exception
            try
            {
                getTable1301().setTableDataAndWrite(messageSeqId, 0, 0, 0, 0, false, nullZones);
                LOG_INFO("Table1301 setTableDataAndWrite messageSeqId=%d  announceId=0 periodInsecs=0 starttime=0 stoptime=0 isEventTrigger=false", messageSeqId);
            }
            catch (TA_Base_Core::TransactiveException& err)
            {
                LOG_ERROR("Table1301 setTableDataAndWrite ERROR messageSeqId=%d  announceId=0 periodInsecs=0 starttime=0 stoptime=0 isEventTrigger=false", messageSeqId);
                LOG_EXCEPTION_CATCH(SourceInfo, "TransactiveException", err.what());
            }
            catch (...)
            {
                LOG_ERROR("unknown exception when Table1301 setTableDataAndWrite messageSeqId=%d", messageSeqId);
            }
        }

        FUNCTION_EXIT;
    }
}
