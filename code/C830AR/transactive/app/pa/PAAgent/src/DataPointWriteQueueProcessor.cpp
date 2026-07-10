/**
 * The source code in this file is the property of Ripple Systems and is not for redistribution in
 * any form.
 *
 * Source:        $File: //depot/C830AR_TIP/C830AR/transactive/app/pa/PAAgent/src/DataPointWriteQueueProcessor.cpp $
 * @author:    HoaVu
 * @version:    $Revision: #5 $
 *
 * Last modification:    $DateTime: 2025/06/19 16:32:14 $
 * Last modified by:    $Author: bihui.luo $
 *
 * Description:
 *
 *    The DataPointWriteQueueProcessor object is responsible for processing write data commands to the PA Agent
 *
 */

#include "pch.h"
#include "Table570.h"
#include "CachedConfig.h"
#include "DataPointWriteQueueProcessor.h"
#include "CachedConfig.h"
#include "core/utilities/src/TAAssert.h"
#include "bus/scada/datapoint_library/src/DataPoint.h"
#include "core/utilities/src/DebugUtilEx.h"

namespace TA_IRS_App
{
    DataPointWriteQueueProcessor::DataPointWriteQueueProcessor(Table570& table570)
        : m_cachedConfigInstance(NULL),
        m_table570(table570)
    {
        m_cachedConfigInstance = CachedConfig::getInstance();
    }

    DataPointWriteQueueProcessor::~DataPointWriteQueueProcessor()
    {
    }

    void DataPointWriteQueueProcessor::processEvent(TA_Base_Bus::DataPointWriteRequest* newEvent)
    {
        LOG_SCOPE("DataPointWriteQueueProcessor::processEvent");

        // prevent multiple threads attempting to delete/create
        // simultaneously
        TA_Base_Core::ThreadGuard guard(m_lock);

        LOG_INFO("DataPoint write request found.");

        TA_Base_Bus::DataPoint* dp = newEvent->getDataPoint();

        // We are only expecting a value of false from the Fire Countdown DIO datapoint.

        try
        {
            //TD15174
            if (!CachedConfig::getInstance()->getIsMaster() )
            {
                // Pointer comparison should be okay.
                if (dp == m_cachedConfigInstance->getFireCountdownActiveOutputDP())
                {
                    if (true == newEvent->getValue().getBoolean())
                    {
                        m_table570.resetFireAlarmStatusAndWrite();
                    }
                }
            }

            //TD15174
        }
        catch (...)
        {
            // do nothing
        }
    }
}
