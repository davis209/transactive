/**
 * The source code in this file is the property of
 * Ripple Systems and is not for redistribution
 * in any form.
 *
 * Source:   $File: //depot/C830AR_TIP/C830AR/transactive/app/pa/PAAgent/src/ConnectToPAFEPC.cpp $
 * @author:  Ripple
 * @version: $Revision: #4 $
 *
 * Last modification: $DateTime: 2025/01/06 12:26:20 $
 * Last modified by:  $Author: limin.zhu $
 *
 */

#include "pch.h"
#include "ConnectToPAFEPC.h"
#include "PasTableManager.h"
#include "Table100.h"
#include "Table200.h"
#include "Table1200.h"
#include "Table201.h"
#include "Table1201.h"
#include "Table202.h"
#include "Table204.h"
#include "Table1204.h"
#include "Table301.h"
#include "Table1301.h"
#include "Table302.h"
#include "Table303.h"
#include "Table1303.h"
#include "Table350.h"
#include "Table550.h"
#include "Table560.h"
#include "Table570.h"
#include "Table590.h"
#include "Table603.h"
#include "Table604.h"
#include "Table610.h"
#include "Table1610.h"
#include "Table630.h"
#include "core/utilities/src/DebugUtilEx.h"

using namespace TA_Base_Core;

namespace TA_IRS_App
{
    ConnectToPAFEPC::ConnectToPAFEPC(TA_IRS_Bus::Scheduler& socketScheduler,
                                     TA_IRS_Bus::Scheduler& processScheduler,
                                     PasTableManager& pasTableManager,
                                     PASConnection& pasConnection)
        : TA_IRS_Bus::IEvent(ACE_OS::gettimeofday() + ACE_Time_Value(1))
        , m_socketScheduler(socketScheduler)
        , m_processScheduler(processScheduler)
        , m_pasTableManager(pasTableManager)
        , m_pasConnection(pasConnection)
    {
    }

    ConnectToPAFEPC::ConnectToPAFEPC(ACE_Time_Value expiryTime,
                                     TA_IRS_Bus::Scheduler& socketScheduler,
                                     TA_IRS_Bus::Scheduler& processScheduler,
                                     PasTableManager& pasTableManager,
                                     PASConnection& pasConnection)
        : TA_IRS_Bus::IEvent(expiryTime)
        , m_socketScheduler(socketScheduler)
        , m_processScheduler(processScheduler)
        , m_pasTableManager(pasTableManager)
        , m_pasConnection(pasConnection)
    {
    }

    ConnectToPAFEPC::~ConnectToPAFEPC()
    {
    }

    void ConnectToPAFEPC::consume()
    {
        LOG_SCOPE("ConnectToPAFEPC::consume");
        LOG_DEBUG("Consuming ConnectToPAFEPC event.");

        // Remove existing events.
        m_socketScheduler.clear();

        // liuyu++ #243 terminate error
        if (m_pasConnection.getTerminate() == true)
        {
            return;
        }

        // ++liuyu #243 terminate error

        m_pasConnection.ensureConnected();

        // Schedule the fast polling events.
        m_socketScheduler.post(new ReadTable302(m_pasTableManager.getTable302(),
                                                m_socketScheduler,
                                                m_processScheduler,
                                                m_pasTableManager));

        // Schedule non recurring events.
        m_socketScheduler.post(new ReadTable100(m_pasTableManager.getTable100(),
                                                m_pasTableManager.getTable560(),
                                                m_socketScheduler,
                                                m_processScheduler,
                                                m_pasTableManager));

        m_socketScheduler.post(new ReadTable202(m_pasTableManager.getTable202(),
                                                m_pasTableManager.getTable560(),
                                                m_socketScheduler,
                                                m_processScheduler,
                                                m_pasTableManager));

        m_socketScheduler.post(new ReadTable550(m_pasTableManager.getTable550(),
                                                m_pasTableManager.getTable560(),
                                                m_socketScheduler,
                                                m_processScheduler,
                                                m_pasTableManager));

        m_socketScheduler.post(new ReadTable570(m_pasTableManager.getTable570(),
                                                m_pasTableManager.getTable560(),
                                                m_socketScheduler,
                                                m_processScheduler,
                                                m_pasTableManager));

        m_socketScheduler.post(new ReadTable590(m_pasTableManager.getTable590(),
                                                m_pasTableManager.getTable560(),
                                                m_socketScheduler,
                                                m_processScheduler,
                                                m_pasTableManager));

        m_socketScheduler.post(new ReadTable603(m_pasTableManager.getTable603(),
                                                m_pasTableManager.getTable560(),
                                                m_socketScheduler,
                                                m_processScheduler,
                                                m_pasTableManager));

        m_socketScheduler.post(new ReadTable604(m_pasTableManager.getTable604(),
                                                m_pasTableManager.getTable560(),
                                                m_socketScheduler,
                                                m_processScheduler,
                                                m_pasTableManager));
    }

    void ConnectToPAFEPC::repostAllWriteablePASTableEvents()
    {
        LOG_SCOPE("ConnectToPAFEPC::repostAllWriteablePASTableEvents");
        std::vector<AbstractWriteablePASTable*> writeableTables =
            m_pasTableManager.getAllWriteableTables();

        // repost all the table write events
        for (std::vector<AbstractWriteablePASTable*>::iterator itr = writeableTables.begin();
             itr != writeableTables.end(); itr++)
        {
            AbstractWriteablePASTable* writeableTable = *itr;

            writeableTable->repostCurrentWriteEvents(m_socketScheduler);
        }
    }

    ConnectToMasterPAFEPC::ConnectToMasterPAFEPC(TA_IRS_Bus::Scheduler& socketScheduler,
                                                 TA_IRS_Bus::Scheduler& processScheduler,
                                                 PasTableManager& pasTableManager,
                                                 PASConnection& pasConnection)
        : ConnectToPAFEPC(socketScheduler,
                          processScheduler,
                          pasTableManager,
                          pasConnection)
    {
    }

    ConnectToMasterPAFEPC::ConnectToMasterPAFEPC(ACE_Time_Value expiryTime,
                                                 TA_IRS_Bus::Scheduler& socketScheduler,
                                                 TA_IRS_Bus::Scheduler& processScheduler,
                                                 PasTableManager& pasTableManager,
                                                 PASConnection& pasConnection)
        : ConnectToPAFEPC(expiryTime,
                          socketScheduler,
                          processScheduler,
                          pasTableManager,
                          pasConnection)

    {
    }

    ConnectToMasterPAFEPC::~ConnectToMasterPAFEPC()
    {
    }

    void ConnectToMasterPAFEPC::consume()
    {
        LOG_SCOPE("ConnectToMasterPAFEPC::consume");

        try
        {
            ConnectToPAFEPC::consume();

            // liuyu++ #243 terminate error
            if (m_pasConnection.getTerminate() == true)
            {
                return;
            }

            // ++liuyu #243 terminate error

            // Schedule the fast polling events.
            // TD 17013
            m_pasTableManager.getTable1200().setInitalThreadValue(true);
            // TD 17013

            m_socketScheduler.post(new ReadTable1200(m_pasTableManager.getTable1200(),
                                                     m_socketScheduler,
                                                     m_processScheduler,
                                                     m_pasTableManager));

            m_socketScheduler.post(new ReadTable560(m_pasTableManager.getTable560(),
                                                    m_socketScheduler,
                                                    m_processScheduler,
                                                    m_pasTableManager));

            // Schedule an initial read
            m_socketScheduler.post(new ReadTable1204(m_pasTableManager.getTable1204(),
                                                     m_pasTableManager.getTable560(),
                                                     m_socketScheduler,
                                                     m_processScheduler,
                                                     m_pasTableManager));

            //        m_socketScheduler.post( new ReadTable610( m_pasTableManager.getTable610(),
            //                                                        m_pasTableManager.getTable560(),
            //                                                        m_socketScheduler,
            //                                                        m_processScheduler,
            //                                                        m_pasTableManager ) );
            m_socketScheduler.post(new ReadTable1610(m_pasTableManager.getTable1610(),
                                                     m_pasTableManager.getTable560(),
                                                     m_socketScheduler,
                                                     m_processScheduler,
                                                     m_pasTableManager));

            repostAllWriteablePASTableEvents();
        }
        catch (TA_Base_Core::PasConnectionException&)
        {
            m_socketScheduler.post(new ConnectToMasterPAFEPC(ACE_OS::gettimeofday() + ACE_Time_Value(10, 0),
                                                             m_socketScheduler,
                                                             m_processScheduler,
                                                             m_pasTableManager,
                                                             m_pasConnection));
        }

        delete this;
    }

    void ConnectToMasterPAFEPC::cancel()
    {
        LOG_SCOPE("ConnectToMasterPAFEPC::cancel");
        delete this;
    }

    ConnectToStationPAFEPC::ConnectToStationPAFEPC(TA_IRS_Bus::Scheduler& socketScheduler,
                                                   TA_IRS_Bus::Scheduler& processScheduler,
                                                   PasTableManager& pasTableManager,
                                                   PASConnection& pasConnection)
        : ConnectToPAFEPC(socketScheduler,
                          processScheduler,
                          pasTableManager,
                          pasConnection)
    {
    }

    ConnectToStationPAFEPC::ConnectToStationPAFEPC(ACE_Time_Value expiryTime,
                                                   TA_IRS_Bus::Scheduler& socketScheduler,
                                                   TA_IRS_Bus::Scheduler& processScheduler,
                                                   PasTableManager& pasTableManager,
                                                   PASConnection& pasConnection)
        : ConnectToPAFEPC(expiryTime,
                          socketScheduler,
                          processScheduler,
                          pasTableManager,
                          pasConnection)

    {
    }

    ConnectToStationPAFEPC::~ConnectToStationPAFEPC()
    {
    }

    void ConnectToStationPAFEPC::consume()
    {
        LOG_SCOPE("ConnectToStationPAFEPC::consume");

        try
        {
            ConnectToPAFEPC::consume();

            // liuyu++ #243 terminate error
            if (m_pasConnection.getTerminate() == true)
            {
                return;
            }

            // ++liuyu #243 terminate error
            // Schedule the fast polling events.
            // TD 17013
            m_pasTableManager.getTable200().setInitalThreadValue(true);
            // TD 17013

            m_socketScheduler.post(new ReadTable200(m_pasTableManager.getTable200(),
                                                    m_socketScheduler,
                                                    m_processScheduler,
                                                    m_pasTableManager));

            m_socketScheduler.post(new ReadTable560(m_pasTableManager.getTable560(),
                                                    m_socketScheduler,
                                                    m_processScheduler,
                                                    m_pasTableManager));

            // Schedule an initial read
            m_socketScheduler.post(new ReadTable204(m_pasTableManager.getTable204(),
                                                    m_pasTableManager.getTable560(),
                                                    m_socketScheduler,
                                                    m_processScheduler,
                                                    m_pasTableManager));

            m_socketScheduler.post(new ReadTable610(m_pasTableManager.getTable610(),
                                                    m_pasTableManager.getTable560(),
                                                    m_socketScheduler,
                                                    m_processScheduler,
                                                    m_pasTableManager));

            repostAllWriteablePASTableEvents();
        }
        catch (TA_Base_Core::PasConnectionException&)
        {
            LOG_DEBUG("Rescheduling ConnectToPAFEPC event.");
            m_socketScheduler.post(new ConnectToStationPAFEPC(ACE_OS::gettimeofday() + ACE_Time_Value(10, 0),
                                                              m_socketScheduler,
                                                              m_processScheduler,
                                                              m_pasTableManager,
                                                              m_pasConnection));
        }

        delete this;
    }

    void ConnectToStationPAFEPC::cancel()
    {
        LOG_DEBUG("Cancelling ConnectToPAFEPC event.");
        delete this;
    }
}
