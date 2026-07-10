/**
 * The source code in this file is the property of Ripple Systems and is not for redistribution in
 * any form.
 *
 * Source:        $File: //depot/C830AR_TIP/C830AR/transactive/app/pa/PAAgent/src/DataPointWriteQueueProcessor.h $
 * @author:    HoaVu
 * @version:    $Revision: #2 $
 *
 * Last modification:    $DateTime: 2024/12/10 11:18:50 $
 * Last modified by:    $Author: limin.zhu $
 *
 * Description:
 *
 *    The DataPointWriteQueueProcessor object is responsible for processing write data commands to the PMS RTU
 *
 */

#pragma once
#include "core/threads/src/Thread.h"
#include "bus/scada/datapoint_library/src/DataPointWriteQueue.h"
#include "app/pa/PAAgent/src/Table570.h"
#include "bus/scada/common_library/src/EventProcessor.h"

class CachedConfig;

namespace TA_IRS_App
{
    class DataPointWriteQueueProcessor : public EventProcessor<TA_Base_Bus::DataPointWriteRequest>
    {
    private:

        DataPointWriteQueueProcessor(const DataPointWriteQueueProcessor& obj);
        DataPointWriteQueueProcessor& operator= (const DataPointWriteQueueProcessor& rhs);

        // operations

    public:

        DataPointWriteQueueProcessor(Table570& table570);

        virtual ~DataPointWriteQueueProcessor();

        // operations

    private:

        /**
         * processEvent
         *
         * Process the specified DataPoint write request
         *
         */
        virtual void processEvent(TA_Base_Bus::DataPointWriteRequest* newEvent);

        // attributes

    private:

        TA_Base_Core::NonReEntrantThreadLockable     m_lock;
        CachedConfig* m_cachedConfigInstance;
        /** The reference to Table 570 to reset fire alarm announcements. */
        Table570& m_table570;
    };
}
