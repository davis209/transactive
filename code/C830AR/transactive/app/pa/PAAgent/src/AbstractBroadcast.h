/**
 * The source code in this file is the property of
 * Ripple Systems and is not for redistribution
 * in any form.
 *
 * Source:   $File: //depot/C830AR_TIP/C830AR/transactive/app/pa/PAAgent/src/AbstractBroadcast.h $
 * @author:  Ripple
 * @version: $Revision: #5 $
 *
 * Last modification: $DateTime: 2025/01/22 17:56:54 $
 * Last modified by:  $Author: limin.zhu $
 *
 */

#pragma once
#include <string>
#include "core/utilities/src/TAAssert.h"
#include "core/synchronisation/src/ReEntrantThreadLockable.h"
#include "core/exceptions/src/PasExceptions.h"
#include "bus/pa_4669/pa_agent/IDL/src/IPAAgentCorbaDef.h"
#include "bus/pa_4669/pa_agent/IDL/src/IPAAgentCorbaDefDescribe.h"
#include "app/pa/PAAgent/src/ProgressReportSentry.h"
#include "app/pa/PAAgent/src/PasHelpers.h"
#include "core/utilities/src/TAAssert.h"
#include "core/utilities/src/DebugUtilEx.h"

namespace TA_IRS_App
{
    class AbstractBroadcast
    {
    public:

        AbstractBroadcast(TA_Base_Bus::IPAAgentCorbaDef::BroadcastConfig* broadcastConfigP, const std::string& sessionId);
        // throws CorruptInternalStateDataException

        void setProgressReport(const TA_Base_Bus::IPAAgentCorbaDef::ProgressReport& progressReport)
        {
            TA_ASSERT(m_progressReportSentry, "m_progressReportSentry is NULL");
            m_progressReportSentry->setProgressReport(progressReport);
        }

        virtual ~AbstractBroadcast();

        virtual void validateBroadcastConfig() = 0;

        virtual void operatorAbort();
        virtual void execute() = 0;
        virtual void terminate() = 0;
        void updateRuntimeState();  // calls runtimeSpecifics()

        virtual TA_Base_Bus::IPAAgentCorbaDef::BroadcastConfig* getBroadcastConfigCopy();

        virtual TA_Base_Bus::IPAAgentCorbaDef::BroadcastParameterSeq* getBroadcastParametersCopy();
        virtual void setBroadcastParameter(const TA_Base_Bus::IPAAgentCorbaDef::BroadcastParameter& parameter);
        // throws CorruptInternalStateDataException

        std::string getBroadcastId();
        void setBroadcastId(const std::string& broadcastId);

        TA_Base_Bus::IPAAgentCorbaDef::EBroadcastType getBroadcastType();

        bool getIsEventTriggered();

        time_t getLastUpdate();

        ///////////////////////////////////////////////////////////////////////////
        // Delegation to ProgressReportSentry
        ///////////////////////////////////////////////////////////////////////////

        TA_Base_Bus::IPAAgentCorbaDef::EBroadcastState getBroadcastState()
        {
            TA_ASSERT(m_progressReportSentry, "m_progressReportSentry is NULL");
            return m_progressReportSentry->getBroadcastState();
        }

        void setBroadcastState(TA_Base_Bus::IPAAgentCorbaDef::EBroadcastState state)
        {
            TA_ASSERT(m_progressReportSentry, "m_progressReportSentry is NULL");
            m_progressReportSentry->setBroadcastState(state);
            resetRetries();
        }

        TA_Base_Bus::IPAAgentCorbaDef::ProgressReport* getProgressReportCopy()
        {
            TA_ASSERT(m_progressReportSentry, "m_progressReportSentry is NULL");
            return m_progressReportSentry->getProgressReportCopy();
        }

        bool getHasProgressReportChanged()
        {
            TA_ASSERT(m_progressReportSentry, "m_progressReportSentry is NULL");
            return m_progressReportSentry->getHasProgressReportChanged();
        }

        void setHasProgressReportChanged(bool changed)
        {
            TA_ASSERT(m_progressReportSentry, "m_progressReportSentry is NULL");
            m_progressReportSentry->setHasProgressReportChanged(changed);
        }

        // Agent-synchronised BroadcastParameter
        std::string getSessionId();
        void setSessionId(const std::string& sessionId, bool sendSynchronisation = true);

        // Agent-synchronised BroadcastParameter
        bool getIsTaggedForRemoval();
        void setIsTaggedForRemoval(bool isTaggedForRemoval, bool sendSynchronisation = true);

        // Agent-synchronised BroadcastParameter
        TA_Base_Bus::IPAAgentCorbaDef::CoverageIndices getEffectiveCoverageIndices();
        void setEffectiveCoverageIndices(const TA_Base_Bus::IPAAgentCorbaDef::CoverageIndices& indicies, bool sendSynchronisation = true);

        void resetRetries();
        void determineWhetherToRetryOrAbort(TA_Base_Bus::IPAAgentCorbaDef::EGlobalState fault);

        virtual void updateFinalProgressReport();

    protected:

        virtual void runtimeSpecifics() = 0;

        void abnormalTermination(TA_Base_Bus::IPAAgentCorbaDef::EGlobalState fault);
        void abnormalContinuation(TA_Base_Bus::IPAAgentCorbaDef::EGlobalState fault);
        // TD17456
        void noneValidZoneTermination(TA_Base_Bus::IPAAgentCorbaDef::EGlobalState fault);
        // TD17456

    private:

        AbstractBroadcast(const AbstractBroadcast& theAbstractBroadcast);

    protected:

        TA_Base_Bus::IPAAgentCorbaDef::BroadcastConfig_var           m_broadcastConfigV;
        ProgressReportSentry* m_progressReportSentry;

        // Usual lock for runtime operations
        TA_Base_Core::ReEntrantThreadLockable                        m_lock;

        // Special lock to limit locked time for accesses to global broadcast information
        // frequently accessed by AbstractCircularList e.g. announcementIds, broadcastIds
        // broadcastTypes ...
        TA_Base_Core::ReEntrantThreadLockable                        m_lockForBroadcastParameters;

        time_t                                                  m_lastUpdate;
        bool                                                    m_hasProgressReportChanged;

        bool                                                    m_isTaggedForRemoval;

        TA_Base_Bus::IPAAgentCorbaDef::CoverageIndices               m_effectiveCoverageIndices;

        int                                                     m_retries;

        static const int                                        MAX_NON_FATAL_RETRIES;
    };

    ////////////////////////
    //      INLINES
    ////////////////////////

    inline std::string AbstractBroadcast::getSessionId()
    {
        TA_Base_Core::ThreadGuard guard(m_lockForBroadcastParameters);

        // For Solaris
        std::string retVal(m_broadcastConfigV->sessionId);
        return retVal;
    }

    inline void AbstractBroadcast::setSessionId(const std::string& sessionId, bool sendSynchronisation)
    {
        TA_Base_Core::ThreadGuard guard(m_lockForBroadcastParameters);

        // Note CORBA::String_members will automatically free and reallocate memory for const char* assignment
        m_broadcastConfigV->sessionId = sessionId.c_str();

        if (sendSynchronisation)
        {
            CORBA::Any data;
            data <<= CORBA::Any::from_string(sessionId.c_str(), 0);

            PasHelpers::instance().sendPAAgentBroadcastParameterUpdate(TA_Base_Bus::IPAAgentCorbaDef::BROADCAST_PARAMETER_SESSION_ID, data, getBroadcastId().c_str());
        }
    }

    inline bool AbstractBroadcast::getIsTaggedForRemoval()
    {
        TA_Base_Core::ThreadGuard guard(m_lockForBroadcastParameters);

        return m_isTaggedForRemoval;
    }

    inline void AbstractBroadcast::setIsTaggedForRemoval(bool isTaggedForRemoval, bool sendSynchronisation)
    {
        TA_Base_Core::ThreadGuard guard(m_lockForBroadcastParameters);

        m_isTaggedForRemoval = isTaggedForRemoval;

        if (sendSynchronisation)
        {
            CORBA::Any data;
            data <<= CORBA::Any::from_boolean(isTaggedForRemoval);

            PasHelpers::instance().sendPAAgentBroadcastParameterUpdate(TA_Base_Bus::IPAAgentCorbaDef::BROADCAST_PARAMETER_IS_TAGGED_FOR_REMOVAL, data, getBroadcastId().c_str());
        }
    }

    inline TA_Base_Bus::IPAAgentCorbaDef::CoverageIndices AbstractBroadcast::getEffectiveCoverageIndices()
    {
        TA_Base_Core::ThreadGuard guard(m_lockForBroadcastParameters);

        return m_effectiveCoverageIndices;
    }

    inline void AbstractBroadcast::setEffectiveCoverageIndices(const TA_Base_Bus::IPAAgentCorbaDef::CoverageIndices& indicies, bool sendSynchronisation)
    {
        TA_Base_Core::ThreadGuard guard(m_lockForBroadcastParameters);

        m_effectiveCoverageIndices = indicies;
        LOG_DEBUG("setEffectiveCoverageIndices(): %s", nvps(indicies));

        if (sendSynchronisation)
        {
            CORBA::Any data;
            data <<= indicies;

            PasHelpers::instance().sendPAAgentBroadcastParameterUpdate(TA_Base_Bus::IPAAgentCorbaDef::BROADCAST_PARAMETER_EFFECTIVE_COVERAGE_INDICES, data, getBroadcastId().c_str());
        }
    }

    inline TA_Base_Bus::IPAAgentCorbaDef::BroadcastParameterSeq* AbstractBroadcast::getBroadcastParametersCopy()
    {
        TA_Base_Bus::IPAAgentCorbaDef::BroadcastParameterSeq* parameters =
            new TA_Base_Bus::IPAAgentCorbaDef::BroadcastParameterSeq;

        parameters->length(3);

        CORBA::Any data1;
        data1 <<= CORBA::Any::from_string(getSessionId().c_str(), 0);
        (*parameters)[0].broadcastId = getBroadcastId().c_str();
        (*parameters)[0].parameter   = TA_Base_Bus::IPAAgentCorbaDef::BROADCAST_PARAMETER_SESSION_ID;
        (*parameters)[0].data        = data1;

        CORBA::Any data2;
        data2 <<= CORBA::Any::from_boolean(getIsTaggedForRemoval());
        (*parameters)[0].broadcastId = getBroadcastId().c_str();
        (*parameters)[0].parameter   = TA_Base_Bus::IPAAgentCorbaDef::BROADCAST_PARAMETER_IS_TAGGED_FOR_REMOVAL;
        (*parameters)[0].data        = data2;

        CORBA::Any data3;
        data3 <<= getEffectiveCoverageIndices();
        (*parameters)[0].broadcastId = getBroadcastId().c_str();
        (*parameters)[0].parameter   = TA_Base_Bus::IPAAgentCorbaDef::BROADCAST_PARAMETER_EFFECTIVE_COVERAGE_INDICES;
        (*parameters)[0].data        = data3;

        return parameters;
    }

    inline void AbstractBroadcast::setBroadcastParameter(const TA_Base_Bus::IPAAgentCorbaDef::BroadcastParameter& parameter)
    {
        if (parameter.parameter == TA_Base_Bus::IPAAgentCorbaDef::BROADCAST_PARAMETER_SESSION_ID)
        {
            const char* updateData = NULL;

            if (0 != (parameter.data >>= CORBA::Any::to_string(updateData, 0)))
            {
                setSessionId(updateData, false);
                LOG_INFO("BroadcastParameter update received: SessionId=%s", updateData);
            }
        }
        else if (parameter.parameter == TA_Base_Bus::IPAAgentCorbaDef::BROADCAST_PARAMETER_IS_TAGGED_FOR_REMOVAL)
        {
            bool updateData = 0;

            if (0 != (parameter.data >>= CORBA::Any::to_boolean(updateData)))
            {
                setIsTaggedForRemoval(updateData, false);
                LOG_INFO("BroadcastParameter update received: IsTaggedForRemoval=%s", updateData ? "true" : "false");
            }
        }
        else if (parameter.parameter == TA_Base_Bus::IPAAgentCorbaDef::BROADCAST_PARAMETER_EFFECTIVE_COVERAGE_INDICES)
        {
            TA_Base_Bus::IPAAgentCorbaDef::CoverageIndices* updateData = 0;

            if (0 != (parameter.data >>= updateData))
            {
                setEffectiveCoverageIndices(*updateData, false);
                LOG_INFO("BroadcastParameter update received: EffectiveCoverageIndices");
            }
        }
        else
        {
            // Should never get here
            TA_THROW(TA_Base_Core::CorruptInternalStateDataException("Unrecognised BroadcastParameter update received"));
        }
    }
}
