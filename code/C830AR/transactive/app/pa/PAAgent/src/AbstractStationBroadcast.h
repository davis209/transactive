/**
 * The source code in this file is the property of
 * Ripple Systems and is not for redistribution
 * in any form.
 *
 * Source:   $File: //depot/C830AR_TIP/C830AR/transactive/app/pa/PAAgent/src/AbstractStationBroadcast.h $
 * @author:  Ripple
 * @version: $Revision: #5 $
 *
 * Last modification: $DateTime: 2025/01/22 17:56:54 $
 * Last modified by:  $Author: limin.zhu $
 *
 */

#pragma once
#include "bus/pa_4669/pa_agent/IDL/src/IPAAgentCorbaDef.h"
#include "bus/pa_4669/pa_agent/IDL/src/IPAAgentCorbaDefDescribe.h"
#include "app/pa/PAAgent/src/AbstractBroadcast.h"
#include "app/pa/PAAgent/src/PasHelpers.h"

namespace TA_IRS_App
{
    class AbstractStationBroadcast : public AbstractBroadcast
    {
    public:

        AbstractStationBroadcast(TA_Base_Bus::IPAAgentCorbaDef::BroadcastConfig* broadcastConfigP, const std::string& sessionId);
        // throw CorruptInternalStateDataException

        void retry(const TA_Base_Bus::IPAAgentCorbaDef::ZoneCoverage& zones, const std::string& sessionId);
        // throw(TA_Base_Core::BadParameterException);

        TA_Base_Bus::IPAAgentCorbaDef::BroadcastParameterSeq* getBroadcastParametersCopy() override;
        void setBroadcastParameter(const TA_Base_Bus::IPAAgentCorbaDef::BroadcastParameter& parameter) override;

        virtual ~AbstractStationBroadcast();

        TA_Base_Bus::IPAAgentCorbaDef::ZoneCoverage getEffectiveZoneCoverage(const TA_Base_Bus::IPAAgentCorbaDef::ZoneCoverage& zones);

        virtual TA_Base_Bus::IPAAgentCorbaDef::ZoneCoverage getZoneCoverage() = 0;

        static TA_Base_Bus::IPAAgentCorbaDef::CoverageIndices calculateEffectiveCoverageIndices(
            const TA_Base_Bus::IPAAgentCorbaDef::ZoneCoverage& allZones,
            const TA_Base_Bus::IPAAgentCorbaDef::ZoneCoverage& newZones);
        //throw(TA_Base_Core::BadParameterException)

        // Agent-synchronised BroadcastParameter
        CORBA::Octet getAnnounceId();
        void setAnnounceId(CORBA::Octet announceId, bool sendSynchronisation = true);

        // Agent-synchronised BroadcastParameter
        bool getWasExecutedSuccessfully();
        void setWasExecutedSuccessfully(bool value, bool sendSynchronisation = true);

        void updateFinalProgressReport() override;

        // TD17456
        virtual bool isAllFailureZones();
        // TD17456

    private:

        AbstractStationBroadcast(const AbstractStationBroadcast& theAbstractStationBroadcast);

    protected:

        CORBA::Octet                                    m_announceId;
        bool                                            m_wasExecutedSuccessfully;
    };
}
