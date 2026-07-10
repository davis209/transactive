/**
 * The source code in this file is the property of
 * Ripple Systems and is not for redistribution
 * in any form.
 *
 * Source:   $File: //depot/C830AR_TIP/C830AR/transactive/app/pa/PAAgent/src/AtsAgentSubscriber.h $
 * @author:  Ripple
 * @version: $Revision: #2 $
 *
 * Last modification: $DateTime: 2024/12/10 11:18:50 $
 * Last modified by:  $Author: limin.zhu $
 *
 */

/**
 * Obtains the ATSTrainMessage ptr from the CORBA::Any.
 * Converts the Fixed block IDs to ECS Zone IDs.  Only ECS Zones inside the
 * station's ECS Zone Decision Scope are mapped.
 * Adds the equivalent ATSTrainMessage_var references to the StationDecisionEngine
 * and StationDataPointUpdater class maps.  The reference count mechanism is
 * apparently thread-safe.
 */

#pragma once
#include <string>
#include "core/synchronisation/src/NonReEntrantThreadLockable.h"
#include "core/threads/src/Thread.h"
#include "core/message/src/SpecialisedMessageSubscriber.h"
#include "core/message/IDL/src/CommsMessageCorbaDef.h"
#include "bus/ats/ats_agent/IDL/src/IAtsPasCorbaDef.h"

namespace TA_IRS_App
{
    class BroadcastManager;

    class AtsAgentSubscriber : public TA_Base_Core::SpecialisedMessageSubscriber<TA_Base_Core::CommsMessageCorbaDef>
    {
    public:

        AtsAgentSubscriber(BroadcastManager& broadcastManager);
        virtual ~AtsAgentSubscriber();
        virtual void receiveSpecialisedMessage(const TA_Base_Core::CommsMessageCorbaDef& message);

    private:

        AtsAgentSubscriber(const AtsAgentSubscriber& theAtsAgentSubscriber);

        void subscribe();

        void unsubscribe();

        void processMessage(const TA_IRS_Bus::IAtsPasCorbaDef::AtsPasMessageCorbaDef& message);

        TA_Base_Core::NonReEntrantThreadLockable    m_lock;  // Mainly used to syncronise destruction
        BroadcastManager& m_broadcastManager;
        unsigned long                               m_locationKey;
    };
}
