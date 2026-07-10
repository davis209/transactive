/**
 * The source code in this file is the property of
 * Ripple Systems and is not for redistribution
 * in any form.
 *
 * Source:   $File: //depot/C830AR_TIP/C830AR/transactive/app/pa/PAAgent/src/AtsAgentSubscriber.cpp $
 * @author:  Ripple
 * @version: $Revision: #5 $
 *
 * Last modification: $DateTime: 2025/09/11 17:51:01 $
 * Last modified by:  $Author: limin.zhu $
 *
 */

#include "pch.h"
#include "CachedConfig.h"
#include "AtsAgentSubscriber.h"
#include "BroadcastManager.h"
#include "PasHelpers.h"
#include "bus/generic_agent/src/GenericAgent.h"
#include "bus/pa/cachedmaps/src/CachedMaps.h"
#include "core/utilities/src/DebugUtilEx.h"
#include "core/utilities/src/TAAssert.h"
#include "core/synchronisation/src/ThreadGuard.h"
#include "core/message/src/MessageSubscriptionManager.h"
#include "core/message/types/ATSAgentComms_MessageTypes.h"
#include "bus/ats/ats_agent/IDL/src/IAtsPasCorbaDefDescribe.h"
#include "core/utilities/src/stdutil/strings/enum_to_string.h"

using namespace TA_Base_Core;
using namespace TA_IRS_Bus;

namespace TA_IRS_App
{
    // ExceptionChecked
    AtsAgentSubscriber::AtsAgentSubscriber(BroadcastManager& theBroadcastManager)
        : m_broadcastManager(theBroadcastManager),
        m_locationKey(0)
    {
        FUNCTION_ENTRY("Constructor");
        TA_ASSERT(!CachedConfig::getInstance()->getIsMaster(), "AtsAgentSubscriber created for master");

        m_locationKey = CachedConfig::getInstance()->getAgentLocationKey();

        subscribe();

        FUNCTION_EXIT;
    }

    // ExceptionChecked
    AtsAgentSubscriber::~AtsAgentSubscriber()
    {
        FUNCTION_ENTRY("Destructor");

        unsubscribe();

        TA_Base_Core::ThreadGuard guard(m_lock);

        // Just to ensure that we don't start deleting until the messaging has finished
        // processing the last message.

        FUNCTION_EXIT;
    }

    void AtsAgentSubscriber::subscribe()
    {
        LOG_SCOPE("AtsAgentSubscriber::subscribe");
        FUNCTION_ENTRY("subscribe");

        if (false == CachedConfig::getInstance()->getIsLocalATSAgentConfigured())
        {
            // No local Ats Agent was found on startup.
            return;
        }

        // Subscribe for updates from the local Ats Agent
        MessageSubscriptionManager::getInstance().subscribeToCommsMessage(ATSAgentComms::AtsPasUpdate,                            //context
                                                                          this,                                                   //SpecialisedMessageSubscriber
                                                                          CachedConfig::getInstance()->getLocalATSAgentKey(),     //unsigned long entityKey
                                                                          0,                                                      //unsigned long subsystemKey
                                                                          CachedConfig::getInstance()->getAgentLocationKey());   //unsigned long regionKey

        FUNCTION_EXIT;
    }

    void AtsAgentSubscriber::unsubscribe()
    {
        LOG_SCOPE("AtsAgentSubscriber::unsubscribe");
        FUNCTION_ENTRY("unsubscribe");

        MessageSubscriptionManager::getInstance().unsubscribeToMessages(this);

        FUNCTION_EXIT;
    }

    // ExceptionChecked
    void AtsAgentSubscriber::receiveSpecialisedMessage(const CommsMessageCorbaDef& message)
    {
        LOG_SCOPE("AtsAgentSubscriber::receiveSpecialisedMessage");
        FUNCTION_ENTRY("receiveSpecialisedMessage");
        TA_Base_Core::ThreadGuard guard(m_lock);

        // TODO:: check operation mode, only Control Mode will process this msg
        try
        {
            TA_Base_Bus::GenericAgent::ensureControlMode();
        }
        catch (TA_Base_Core::OperationModeException&)
        {
            FUNCTION_EXIT;
            return;
        }

        try
        {
            const TA_IRS_Bus::IAtsPasCorbaDef::AtsPasMessageCorbaDef* messageData = 0;

            if ((message.messageState >>= messageData) == 0)
            {
                LOG_ERROR("Failed to interpret message");
                return;
            }

            processMessage(*messageData);
        }
        catch (...)
        {
            LOG_ERROR("receiveSpecialisedMessage(): Failed to process message");
        }

        FUNCTION_EXIT;
    }

    // ExceptionChecked
    void AtsAgentSubscriber::processMessage(const TA_IRS_Bus::IAtsPasCorbaDef::AtsPasMessageCorbaDef& messageData)
    {
        LOG_SCOPE("AtsAgentSubscriber::processMessage");
        FUNCTION_ENTRY("processMessage");

        int effectiveDestinationStationId(0); // Unallocated if invalid
        bool effectiveLastTrain(false);       // Not the last train if invalid

        if (messageData.trainInfo.valid)
        {
            effectiveDestinationStationId = messageData.trainInfo.destinationStationId;
            effectiveLastTrain = messageData.trainInfo.lastTrain;
        }

        std::stringstream msg;
        msg << "processMessage(): Processing Ats info"
            << " EventType=" << messageData.atsEventType << "(" << st::enum_to_string(messageData.atsEventType) << ")"
            << " StationId=" << messageData.stationId
            << " PlatformId=" << messageData.platformId
            << " Valid=" << messageData.trainInfo.valid
            << " DestinationStationId=" << messageData.trainInfo.destinationStationId
            << " LastTrain=" << messageData.trainInfo.lastTrain;

        LOG_INFO(msg);

        if (!messageData.trainInfo.valid)
        {
            std::stringstream msg;
            msg << "processMessage(): Ats info is not valid"
                << " EffectiveDestinationStationId=" << effectiveDestinationStationId
                << " EffectiveLastTrain=" << effectiveLastTrain;

            LOG_WARN(msg);
        }

        if (messageData.stationId != m_locationKey)
        {
            LOG_ERROR("processMessage(): Event triggered broadcast, location mismatched - ignored");
            return;
        }

        PaAtsTriggeredBroadcastRecord record = CachedMaps::getInstance()->getAtsTriggeredBroadcastRecordFromDetail(
            messageData.atsEventType,
            messageData.stationId,
            effectiveDestinationStationId,
            messageData.platformId,
            effectiveLastTrain);

        LOG_DEBUG("processMessage(): %s", nvps(record));

        if (record.m_pKey == TA_Base_Core::DatabaseKey::getInvalidKey())
        {
            // Not found - ignore
            LOG_ERROR("processMessage(): Event triggered broadcast not configured - ignored");
            return;
        }

        if (!record.m_isEnabled)
        {
            LOG_ERROR("processMessage(): Event triggered broadcast has been disabled - ignored");
            return;
        }

        PaZoneGroupRecord groupRecord;

        try
        {
            groupRecord = CachedMaps::getInstance()->getPaZoneGroupRecordFromKey(record.m_paZoneGroupKey);
            LOG_DEBUG("processMessage(): %s", nvps(groupRecord));
        }
        catch (TA_Base_Core::CachedMappingNotFoundException&)
        {
            // log the error
            LOG_ERROR("processMessage(): Event triggered Station broadcast failed - invalid zone group key %d", record.m_paZoneGroupKey);
            return;
        }

        // Verify validity of zone group
        if (!groupRecord.m_isEventGroup || (groupRecord.m_locationKey != m_locationKey))
        {
            // log the error
            LOG_ERROR("processMessage(): Event triggered Station broadcast failed - invalid zone group key %d", record.m_paZoneGroupKey);
            return;
        }

        // Create request:
        m_broadcastManager.broadcastEventTriggeredStationDvaForGroup(record.m_paDvaMessageKey,
                                                                     record.m_dvaMessageVersion.c_str(),
                                                                     false,
                                                                     false,
                                                                     0,
                                                                     0,
                                                                     BroadcastManager::ATS_AGENT_SESSION_ID.c_str(),
                                                                     groupRecord.m_label.c_str());
        FUNCTION_EXIT;
    }
}
