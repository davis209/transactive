/**
 * The source code in this file is the property of
 * Ripple Systems and is not for redistribution
 * in any form.
 *
 * Source:   $File: //depot/C830AR_TIP/C830AR/transactive/app/pa/PAAgent/src/ForcedDataPointSynchroniser.cpp $
 * @author:  Ripple
 * @version: $Revision: #2 $
 *
 * Last modification: $DateTime: 2024/12/10 11:18:50 $
 * Last modified by:  $Author: limin.zhu $
 *
 */

#if defined(_MSC_VER)
    #pragma warning(disable : 4786 4146 4018 4250 4290)
#endif // defined _MSC_VER

#include "core/message/src/MessagePublicationManager.h"
#include "core/message/src/StateUpdateMessageSender.h"
#include "core/message/types/PAAgentStateUpdate_MessageTypes.h"
#include "bus/scada/DataPointCorbaDef_Impl/src/DataPoint.h"
#include "bus/scada/DataPointCorbaDef_Impl/src/AgentStateUpdateQueueSingletonManager.h"
#include "app/pa/PAAgent/src/ForcedDataPointSynchroniser.h"
#include "app/pa/PAAgent/src/CachedConfig.h"

using namespace TA_Base_Core;

namespace TA_IRS_App
{
    // ExceptionChecked
    ForcedDataPointSynchroniser::ForcedDataPointSynchroniser()
        : m_agentStateUpdateQ(NULL),
          m_isRunning(false),
          m_stateUpdateMessageSender(NULL),
          m_cachedConfigInstance(NULL),
          m_agentSetControl(false)
    {
        FUNCTION_ENTRY("Constructor");

        m_cachedConfigInstance = CachedConfig::getInstance();

        // create AgentStateUpdateQueue for synchronisation
        m_agentStateUpdateQ = TA_Base_Bus::AgentStateUpdateQueueSingletonManager::getInstance();
        m_agentStateUpdateQ->createAgentStateUpdateQueue(true,   // dpInternalStateToBeSynchronised
                                                         true,    // dpForcedStateToBeSynchronised
                                                         false);  // dpFieldStateToBeSynchronised

        // get a handle to StateUpdateMessageSender
        m_stateUpdateMessageSender =
            MessagePublicationManager::getInstance().getStateUpdateMessageSender(PAAgentStateUpdate::PAAgentDataPointUpdate);

        FUNCTION_EXIT;
    }

    // ExceptionChecked
    ForcedDataPointSynchroniser::~ForcedDataPointSynchroniser()
    {
        FUNCTION_ENTRY("Destructor");
        TA_ASSERT(!m_isRunning, "Deleting a running thread");

        // clean up handle to the AgentStateUpdateQueue
        if (0 != m_agentStateUpdateQ)
        {
            m_agentStateUpdateQ->removeInstance(m_agentStateUpdateQ);
            m_agentStateUpdateQ = 0;
        }

        // clean up comms message sender
        if (m_stateUpdateMessageSender != NULL)
        {
            delete m_stateUpdateMessageSender;
            m_stateUpdateMessageSender = NULL;
        }

        FUNCTION_EXIT;
    }

    void ForcedDataPointSynchroniser::setControlMode(bool inControl)
    {
        m_agentSetControl = inControl;
    }

    // ExceptionChecked
    void ForcedDataPointSynchroniser::run()
    {
        FUNCTION_ENTRY("run");

        m_isRunning = true;

        while (m_isRunning)
        {
            try
            {
                processDataPointStateManuallyUpdate();
            }
            catch (...)
            {
                // Not expecting any failures...
                LOG_EXCEPTION_CATCH(SourceInfo, "Unknown exception", "Unknown reason");
            }
        }

        FUNCTION_EXIT;
    }

    // ExceptionChecked
    void ForcedDataPointSynchroniser::terminate()
    {
        FUNCTION_ENTRY("terminate");

        m_isRunning = false;
        m_agentStateUpdateQ->unblockAgentStateUpdateQueue();

        FUNCTION_EXIT;
    }

    void ForcedDataPointSynchroniser::processDataPointStateManuallyUpdate()
    {
        // check the AgentStateUpdateQueue for any data to be sent to the
        // monitor agent
        TA_Base_Bus::DataPointStateManuallyUpdate * newEvent = m_agentStateUpdateQ->getNextDataPointInternalStateForSynchronisation();

        // if the agent is running in Control mode
        if ((true == m_agentSetControl) && (newEvent != NULL))
        {
            TA_Base_Bus::DataPointCorbaDef::DataPointStateChangeType dpEventType = newEvent->getStateChangeType();

            // if the new event is for internal state or value/timestamp/quality
            CORBA::Any data;

            if (TA_Base_Bus::DataPointCorbaDef::dpValueStatus == dpEventType)
            {
                TA_Base_Bus::DataPointCorbaDef::ClientDataPointState * dpState = 0;
                dpState = new TA_Base_Bus::DataPointCorbaDef::ClientDataPointState();
                newEvent->getDataPointState(*dpState);
                data <<= dpState;
            }
            else
            {
                TA_Base_Bus::DataPointCorbaDef::DataPointInternalState * dpInternalState = 0;
                dpInternalState = new TA_Base_Bus::DataPointCorbaDef::DataPointInternalState();
                newEvent->getDataPointInternalState(*dpInternalState);
                data <<= dpInternalState;
            }

            m_stateUpdateMessageSender->sendStateUpdateMessage(
                PAAgentStateUpdate::PAAgentDataPointUpdate,
                newEvent->getEntityKey(),
                CachedConfig::getInstance()->getAgentName(),
                data);

            delete newEvent;
            newEvent = 0;
        }
    }

} // namespace TA_IRS_App
