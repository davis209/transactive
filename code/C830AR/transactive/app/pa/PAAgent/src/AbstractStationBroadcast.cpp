/**
 * The source code in this file is the property of
 * Ripple Systems and is not for redistribution
 * in any form.
 *
 * Source : $File: //depot/C830AR_TIP/C830AR/transactive/app/pa/PAAgent/src/AbstractStationBroadcast.cpp $
 * @author  A. Parker
 * @version $Revision: #6 $
 * Last modification : $DateTime: 2025/01/22 17:56:54 $
 * Last modified by : $Author: limin.zhu $
 *
 *
 */

#include "pch.h"
#include "AbstractStationBroadcast.h"
#include "core/utilities/src/DebugUtilEx.h"
#include "core/exceptions/src/ThrowException.h"
#include "core/utilities/src/TAAssert.h"
#include "core/utilities/src/MakeNameValuePairsString.h"

using namespace TA_Base_Core;

namespace TA_IRS_App
{
    // Thread checked
    AbstractStationBroadcast::AbstractStationBroadcast(TA_Base_Bus::IPAAgentCorbaDef::BroadcastConfig* broadcastConfigP, const std::string& sessionId)
        : AbstractBroadcast(broadcastConfigP, sessionId),
        m_announceId(0),
        m_wasExecutedSuccessfully(false)
    {
        LOG_SCOPE("AbstractStationBroadcast");
        FUNCTION_ENTRY("AbstractStationBroadcast");

        switch (broadcastConfigP->broadcastType)
        {
            case TA_Base_Bus::IPAAgentCorbaDef::StationMusic:
            {
                TA_Base_Bus::IPAAgentCorbaDef::ZoneCoverage nullZones;
                nullZones.length(0);
                m_progressReportSentry = new ProgressReportSentry(broadcastConfigP->broadcastId, nullZones);
            }
            break;

            case TA_Base_Bus::IPAAgentCorbaDef::StationLive:
                m_progressReportSentry = new ProgressReportSentry(broadcastConfigP->broadcastId,
                                                                  broadcastConfigP->theBroadcastConfigSpecifics.theStationLiveConfig().zones);
                break;

            case TA_Base_Bus::IPAAgentCorbaDef::StationDva:
                m_progressReportSentry = new ProgressReportSentry(broadcastConfigP->broadcastId,
                                                                  broadcastConfigP->theBroadcastConfigSpecifics.theStationDvaConfig().zones);
                break;

            case TA_Base_Bus::IPAAgentCorbaDef::StationRecording:
            {
                TA_Base_Bus::IPAAgentCorbaDef::ZoneCoverage nullZones;
                nullZones.length(0);
                m_progressReportSentry = new ProgressReportSentry(broadcastConfigP->broadcastId, nullZones);
            }
            break;

            case TA_Base_Bus::IPAAgentCorbaDef::TrainLive:
            case TA_Base_Bus::IPAAgentCorbaDef::TrainDva:
            default:
                TA_THROW(CorruptInternalStateDataException("Unexpected broadcastType"));
        } // switch

        FUNCTION_EXIT;
    }

    // Thread checked
    AbstractStationBroadcast::~AbstractStationBroadcast()
    {
        FUNCTION_ENTRY("~AbstractStationBroadcast");

        delete m_progressReportSentry;
        m_progressReportSentry = NULL;

        FUNCTION_EXIT;
    }

    TA_Base_Bus::IPAAgentCorbaDef::CoverageIndices AbstractStationBroadcast::calculateEffectiveCoverageIndices(const TA_Base_Bus::IPAAgentCorbaDef::ZoneCoverage& allZones, const TA_Base_Bus::IPAAgentCorbaDef::ZoneCoverage& newZones)
        //throw(TA_Base_Core::BadParameterException)
    {
        FUNCTION_ENTRY("calculateEffectiveCoverageIndices");

        // ThreadGuard guard(m_lock);

        TA_Base_Bus::IPAAgentCorbaDef::CoverageIndices indices;
        indices.length(newZones.length());

        // Locate matching zones
        int index = 0;

        for (; index < newZones.length(); ++index)
        {
            int j = 0;

            for (; j < allZones.length(); ++j)
            {
                if (newZones[index] == allZones[j])
                {
                    // A matched zone
                    indices[index] = j;
                    break; // j loop
                }
            } // for j

            if (j == allZones.length())
            {
                // log it and throw the exception
                // the calling method should catch this and log an audit message in one point
                LOG_ERROR("Station broadcast failed - The specified zone is not configured for this broadcast");

                FUNCTION_EXIT;
                TA_THROW(BadParameterException("The specified zone is not configured for this broadcast"));
            }
        } // for index

        FUNCTION_EXIT;
        return indices;
    }

    TA_Base_Bus::IPAAgentCorbaDef::ZoneCoverage AbstractStationBroadcast::getEffectiveZoneCoverage(const TA_Base_Bus::IPAAgentCorbaDef::ZoneCoverage& allZones)
    {
        FUNCTION_ENTRY("getEffectiveZoneCoverage");

        ThreadGuard guard(m_lock);

        TA_Base_Bus::IPAAgentCorbaDef::ZoneCoverage effectiveZoneCoverage;
        effectiveZoneCoverage.length(m_effectiveCoverageIndices.length());

        for (int index = 0; index < m_effectiveCoverageIndices.length(); ++index)
        {
            effectiveZoneCoverage[index] = allZones[m_effectiveCoverageIndices[index]];
        }

        FUNCTION_EXIT;
        return effectiveZoneCoverage;
    }

    void AbstractStationBroadcast::retry(const TA_Base_Bus::IPAAgentCorbaDef::ZoneCoverage& zones, const std::string& sessionId)
        //throw(TA_Base_Core::BadParameterException)
    {
        LOG_SCOPE("AbstractStationBroadcast::retry");
        FUNCTION_ENTRY("retry");

        if (getBroadcastState() != TA_Base_Bus::IPAAgentCorbaDef::PendingRemoval)
        {
            // log it and throw the exception
            // the calling method should catch this and log an audit message in one point
            LOG_ERROR("Station broadcast retry failed - Broadcast must be terminated first");

            FUNCTION_EXIT;
            TA_THROW(BadParameterException("Broadcast must be terminated first"));
        }

        ThreadGuard guard(m_lock);

        TA_Base_Bus::IPAAgentCorbaDef::CoverageIndices coverageIndices = calculateEffectiveCoverageIndices(getZoneCoverage(), zones);

        // Reset selected zones
        for (int index = 0; index < coverageIndices.length(); ++index)
        {
            m_progressReportSentry->resetZoneOrTrain(coverageIndices[index]);
        }

        // Update the list of effective zones to broadcast with.  This time only contains
        // the ones specified
        setEffectiveCoverageIndices(coverageIndices);

        // Reset sessionId
        setSessionId(sessionId);

        // Revalidate broadcast in case parameters are no longer valid
        validateBroadcastConfig();

        // Reset broadcast
        setBroadcastState(TA_Base_Bus::IPAAgentCorbaDef::Execution);
        m_progressReportSentry->resetGlobalState();

        FUNCTION_EXIT;
    }

    // Thread checked in parent
    void AbstractStationBroadcast::updateFinalProgressReport()
    {
        LOG_SCOPE("AbstractStationBroadcast::updateFinalProgressReport");
        FUNCTION_ENTRY("updateFinalProgressReport");

        AbstractBroadcast::updateFinalProgressReport();

        // go through each zone and get the ones with failed states
        // all zones that did not fail will be set to completed int the parent class
        TA_Base_Bus::IPAAgentCorbaDef::ZoneCoverage failedZones;
        TA_Base_Bus::IPAAgentCorbaDef::ZoneCoverage allZones = getZoneCoverage();
        std::map<int, std::string> failedReasons; //TD10320
        int failedIndex = 0;

        for (int indexIndex = 0; indexIndex < m_effectiveCoverageIndices.length(); ++indexIndex)
        {
            TA_Base_Bus::IPAAgentCorbaDef::ELocalState localState = m_progressReportSentry->getLocalState(m_effectiveCoverageIndices[indexIndex]);

            if (localState != TA_Base_Bus::IPAAgentCorbaDef::LOCAL_STATE_COMPLETED)
            {
                unsigned int blah = m_effectiveCoverageIndices[indexIndex];
                unsigned int zone = allZones[m_effectiveCoverageIndices[indexIndex]];

                // this is a failed zone
                failedZones.length(failedZones.length() + 1);
                failedZones[failedIndex] = allZones[m_effectiveCoverageIndices[indexIndex]];
                std::string reason = "";//TD10320

                switch (localState)
                {
                    case TA_Base_Bus::IPAAgentCorbaDef::LOCAL_STATE_IN_PROGRESS:
                        reason = "In progress";
                        break;

                    case TA_Base_Bus::IPAAgentCorbaDef::LOCAL_STATE_IN_PROGRESS_DWELL_TIME:
                        reason = "In progress dwell time";
                        break;

                    case TA_Base_Bus::IPAAgentCorbaDef::LOCAL_STATE_CONNECTING:
                        reason = "Connecting";
                        break;

                    case TA_Base_Bus::IPAAgentCorbaDef::LOCAL_STATE_TRAIN_READY:
                        reason = "Train ready";
                        break;

                    case TA_Base_Bus::IPAAgentCorbaDef::LOCAL_STATE_CONTINUE_LIVE_ANNOUNCEMENT_REQUIRED:
                        reason = "Continue live announcement required";
                        break;

                    case TA_Base_Bus::IPAAgentCorbaDef::LOCAL_STATE_COMPLETED:
                        reason = "Completed";
                        break;

                    case TA_Base_Bus::IPAAgentCorbaDef::LOCAL_STATE_ABORTED:
                        reason = "Aborted";
                        break;

                    case TA_Base_Bus::IPAAgentCorbaDef::LOCAL_STATE_FAILED_CYCLE:
                        reason = "Failed cycle";
                        break;

                    case TA_Base_Bus::IPAAgentCorbaDef::LOCAL_STATE_FAILED_ZONE:
                        reason = "Failed zone";
                        break;

                    case TA_Base_Bus::IPAAgentCorbaDef::LOCAL_STATE_FAILED_OVERRIDDEN:
                        reason = "Overridden";
                        break;

                    case TA_Base_Bus::IPAAgentCorbaDef::LOCAL_STATE_FAILED_TRAIN:
                        reason = "Failed train";
                        break;

                    case TA_Base_Bus::IPAAgentCorbaDef::LOCAL_STATE_FAILED_CONNECTION_RESET:
                        reason = "Connection reset";
                        break;

                    case TA_Base_Bus::IPAAgentCorbaDef::LOCAL_STATE_FAILED_INTERRUPTED:
                        reason = "Failed interrupted";
                        break;

                    case TA_Base_Bus::IPAAgentCorbaDef::LOCAL_STATE_FAILED_VERSION_CONFLICT:
                        reason = "Version conflict";
                        break;

                    case TA_Base_Bus::IPAAgentCorbaDef::LOCAL_STATE_FAILED_INVALID_REQUEST:
                        reason = "Invalid request";
                        break;

                    case TA_Base_Bus::IPAAgentCorbaDef::LOCAL_STATE_FAILED_BUSY:
                        reason = "Busy";
                        break;

                    case TA_Base_Bus::IPAAgentCorbaDef::LOCAL_STATE_FAILED_BUSY_WITH_OVERRIDE_OPTION:
                        reason = "Busy with override option";
                        break;

                    case TA_Base_Bus::IPAAgentCorbaDef::LOCAL_STATE_FAILED_CONTINUE_LIVE_ANNOUNCEMENT_TIMEOUT:
                        reason = "TIMS continue request not received";
                        break;

                    case TA_Base_Bus::IPAAgentCorbaDef::LOCAL_STATE_FAILED_NO_PERMISSION:
                        reason = "No duty for this train at this location";
                        break;

                    case TA_Base_Bus::IPAAgentCorbaDef::LOCAL_STATE_TRAIN_FIRST_CYCLE_CANCELLED:
                        reason = "Train first cycle cancelled";
                        break;

                    case TA_Base_Bus::IPAAgentCorbaDef::LOCAL_STATE_FAILED_BAD_COMMAND:
                        reason = "Failed bad command";
                        break;

                    case TA_Base_Bus::IPAAgentCorbaDef::LOCAL_STATE_FAILED_DIFFERENT_ORIGIN:
                        reason = "Failed different origin";
                        break;

                    case TA_Base_Bus::IPAAgentCorbaDef::LOCAL_STATE_FAILED_DIFFERENT_VERSION:
                        reason = "Failed different version";
                        break;

                    case TA_Base_Bus::IPAAgentCorbaDef::LOCAL_STATE_FAILED_MPU_CHANGEOVER:
                        reason = "Failed mpu changeover";
                        break;
                }

                failedReasons[failedIndex] = reason;//TD10320
                failedIndex++;
            }
        }

        PasHelpers::instance().submitPaStationBroadcastResultAuditMessage(
            getBroadcastId(),
            failedZones,
            (failedZones.length() > 0),
            failedReasons,//TD10320
            getSessionId());

        FUNCTION_EXIT;
    }

    // TD17456
    bool AbstractStationBroadcast::isAllFailureZones()
    {
        FUNCTION_ENTRY("isAllFailureZones");

        TA_ASSERT(false, "AbstractStationBroadcast::isAllFailureZones() must be called from the derived classes");

        FUNCTION_EXIT;
        return true;
    }

    ////////////////////////
    //      INLINES
    ////////////////////////

    CORBA::Octet AbstractStationBroadcast::getAnnounceId()
    {
        TA_Base_Core::ThreadGuard guard(m_lockForBroadcastParameters);
        return m_announceId;
    }

    void AbstractStationBroadcast::setAnnounceId(CORBA::Octet announceId, bool sendSynchronisation)
    {
        TA_Base_Core::ThreadGuard guard(m_lockForBroadcastParameters);

        LOG_DEBUG_IF(m_announceId != announceId, "setAnnounceId(): %d => %d", (int)m_announceId, (int)announceId);

        m_announceId = announceId;

        if (sendSynchronisation)
        {
            CORBA::Any data;
            data <<= CORBA::Any::from_octet(announceId);

            PasHelpers::instance().sendPAAgentBroadcastParameterUpdate
            (TA_Base_Bus::IPAAgentCorbaDef::BROADCAST_PARAMETER_STATION_ANNOUNCE_ID, data, getBroadcastId().c_str());
        }
    }

    void AbstractStationBroadcast::setWasExecutedSuccessfully(bool value, bool sendSynchronisation)
    {
        TA_Base_Core::ThreadGuard guard(m_lockForBroadcastParameters);

        LOG_DEBUG_IF(m_wasExecutedSuccessfully != value, "setWasExecutedSuccessfully(): %d => %d", m_wasExecutedSuccessfully, value);
        m_wasExecutedSuccessfully = value;

        if (sendSynchronisation)
        {
            CORBA::Any data;
            data <<= CORBA::Any::from_boolean(value);

            PasHelpers::instance().sendPAAgentBroadcastParameterUpdate
            (TA_Base_Bus::IPAAgentCorbaDef::BROADCAST_PARAMETER_WAS_EXECUTED_SUCCESSFULLY, data, getBroadcastId().c_str());
        }
    }

    bool AbstractStationBroadcast::getWasExecutedSuccessfully()
    {
        TA_Base_Core::ThreadGuard guard(m_lockForBroadcastParameters);

        return m_wasExecutedSuccessfully;
    }

    TA_Base_Bus::IPAAgentCorbaDef::BroadcastParameterSeq* AbstractStationBroadcast::getBroadcastParametersCopy()
    {
        TA_Base_Bus::IPAAgentCorbaDef::BroadcastParameterSeq* parameters = AbstractBroadcast::getBroadcastParametersCopy();

        int originalLength = parameters->length();
        parameters->length(originalLength + 2);

        CORBA::Any data1;
        data1 <<= CORBA::Any::from_octet(getAnnounceId());
        (*parameters)[originalLength].broadcastId = getBroadcastId().c_str();
        (*parameters)[originalLength].parameter   = TA_Base_Bus::IPAAgentCorbaDef::BROADCAST_PARAMETER_STATION_ANNOUNCE_ID;
        (*parameters)[originalLength].data        = data1;

        CORBA::Any data2;
        data2 <<= CORBA::Any::from_boolean(getWasExecutedSuccessfully());
        (*parameters)[originalLength + 1].broadcastId = getBroadcastId().c_str();
        (*parameters)[originalLength + 1].parameter   = TA_Base_Bus::IPAAgentCorbaDef::BROADCAST_PARAMETER_WAS_EXECUTED_SUCCESSFULLY;
        (*parameters)[originalLength + 1].data        = data2;

        return parameters;
    }

    void AbstractStationBroadcast::setBroadcastParameter(const TA_Base_Bus::IPAAgentCorbaDef::BroadcastParameter& parameter)
    {
        if (parameter.parameter == TA_Base_Bus::IPAAgentCorbaDef::BROADCAST_PARAMETER_STATION_ANNOUNCE_ID)
        {
            CORBA::Octet updateData = 0;

            if (0 != (parameter.data >>= CORBA::Any::to_octet(updateData)))
            {
                setAnnounceId(updateData, false);
                LOG_INFO("BroadcastParameter update received: AnnounceId=%d", updateData);
            }
        }
        else if (parameter.parameter == TA_Base_Bus::IPAAgentCorbaDef::BROADCAST_PARAMETER_WAS_EXECUTED_SUCCESSFULLY)
        {
            bool updateData = 0;

            if (0 != (parameter.data >>= CORBA::Any::to_boolean(updateData)))
            {
                setWasExecutedSuccessfully(updateData, false);
                LOG_INFO("BroadcastParameter update received: WasExecutedSuccessfully=%s", updateData ? "true" : "false");
            }
        }
        else
        {
            AbstractBroadcast::setBroadcastParameter(parameter);
        }
    }
}
