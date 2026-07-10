/**
 * The source code in this file is the property of
 * Ripple Systems and is not for redistribution
 * in any form.
 *
 * Source: $File: //depot/C830AR_TIP/C830AR/transactive/app/pa/PAManager/src/PAManagerModel.cpp $
 * @author Jade Lee
 * @version $Revision: #3 $
 * Last modification: $DateTime: 2025/02/10 22:34:14 $
 * Last modified by: $Author: limin.zhu $
 *
 * PaManagerModel controls the shared business logic for the PA Manager
 */

#include "app/pa/PAManager/src/StdAfx.h"

#include "app/pa/PAManager/src/PaManagerModel.h"
#include "app/pa/PAManager/src/CachedConfig.h"
#include "app/pa/PAManager/src/PaRightsManager.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#include <algorithm>

PaManagerModel::PaManagerModel()
    :
    m_statusNameChangeSubscriber(PaStatusNameChangeSubscriber::getInstance())
{
}

PaManagerModel::~PaManagerModel()
{
}

std::vector<std::string> PaManagerModel::getStationDvaBroadcastZones()
{
    // Whether at the OCC or a station, all incoming messages we want have StationDva
    // (so use this as the filter)
    return PaManagerModel::getDvaBroadcastZones(CachedConfig::getInstance()->getAgentObject(),
                                                TA_Base_Bus::IPAAgentCorbaDef::StationDva,
                                                USE_RIGHTS_CHECKING);
}

std::vector<std::string> PaManagerModel::getTrainDvaBroadcastZones()
{
    return PaManagerModel::getDvaBroadcastZones(CachedConfig::getInstance()->getAgentObject(),
                                                TA_Base_Bus::IPAAgentCorbaDef::TrainDva,
                                                USE_RIGHTS_CHECKING);
}

std::vector<std::string> PaManagerModel::getDvaBroadcastZones(PAAgentNamedObject& paAgent,
                                                              const TA_Base_Bus::IPAAgentCorbaDef::EBroadcastType& filter,
                                                              ERightsCheckingMode filterByRightToView)
{
    LOG_SCOPE("PaManagerModel::getDvaBroadcastZones");
    CWaitCursor hourGlassCursor;

    std::vector<std::string>    result;
    TA_Base_Bus::IPAAgentCorbaDef::BroadcastIdTypeSeq* broadcastIds = NULL;

    try
    {
        CORBA_CALL_RETURN(broadcastIds, paAgent, getBroadcastIdTypes, ());
    }
    catch (const TA_Base_Core::ObjectResolutionException&)
    {
        throw;
    }
    catch (const CORBA::Exception&)
    {
        // Some unspecific error occurred with CORBA servant, just means an empty array
        // (reduced functionality error)
        PaErrorHandler::displayError(PaErrorHandler::PA_AGENT_FAIL);
        return result;
    }

    TA_ASSERT(broadcastIds != NULL, "broadcastIds NULL");

    for (unsigned int i = 0; i < broadcastIds->length(); i++)
    {
        const TA_Base_Bus::IPAAgentCorbaDef::BroadcastIdType& item = (*broadcastIds)[i];

        if (filter == item.broadcastType)
        {
            bool placeInOutput = false;

            // We can only add those items we have a right to display - unless rights checking has been override
            if (IGNORE_RIGHTS_CHECKING == filterByRightToView)
            {
                placeInOutput = true;
            }
            else if (PaRightsManager::getInstance().hasRightToDisplayBroadcastWithSessionId(
                std::string(item.sessionId.in())))
            {
                // Else not ignoring rights checking, only place in output if appropriate rights present
                placeInOutput = true;
            }

            if (placeInOutput)
            {
                result.push_back(std::string(item.broadcastId.in()));
            }
        }
    }

    // Clean up
    delete broadcastIds;
    broadcastIds = 0;

    return result;
}

void PaManagerModel::initializeStatusNameSubscriber(HWND hWnd)
{
    m_statusNameChangeSubscriber.addListenerWindow(hWnd);
}

void PaManagerModel::removeStatusNameSubscriber(HWND hWnd)
{
    if (::IsWindow(hWnd))
        m_statusNameChangeSubscriber.removeListenerWindow(hWnd);
}

TA_Base_Bus::IPAAgentCorbaDef::EBroadcastType PaManagerModel::collectStatusNameChange(DWORD id, HWND hWnd, std::string& from, std::string& to)
{
    const TA_Base_Bus::IPAAgentCorbaDef::CurrentBroadcastsUpdateData nameChange =
        m_statusNameChangeSubscriber.getMessageData(id, hWnd);

    from = std::string(nameChange.fromBroadcastId);
    to = std::string(nameChange.toBroadcastId);
    return nameChange.broadcastType;
}

int PaManagerModel::terminateAndRemoveBroadcastIds(const std::vector<std::string>& broadcastIds)
{
    return terminateAndRemoveBroadcastIds(broadcastIds, CachedConfig::getInstance()->getAgentObject());
}

int PaManagerModel::terminateAndRemoveBroadcastIds(const std::vector<std::string>& broadcastIds,
                                                   PAAgentNamedObject& paAgent,
                                                   EBroadcastOrigin origin)
{
    LOG_SCOPE("PaManagerModel::terminateAndRemoveBroadcastIds");

    CWaitCursor hourGlassCursor;

    int numberOfNonRemovals = 0;

    // Progressively go through and kill each specified broadcast id
    for (int i = 0; i < broadcastIds.size(); i++)
    {
        try
        {
            bool ignoreRightsChecking = false;

            // TD #3152
            // There is a subset of broadcast types that can be cleared without
            // regard for operator priveleges (because they were created locally, and
            // are effectively internal to the system - transparent to user) - don't
            // want such broadcasts to pop up errors saying insufficient priveleges
            switch (origin)
            {
                case BO_RECORDING_BROADCAST:    // FALLTHROUGH
                case BO_LIVE_BROADCAST:         // FALLTHROUGH
                case BO_PLAYBACK_BROADCAST:
                    ignoreRightsChecking = true;
                    break;
            };

            // Technically if we don't have a right to clear a broadcast, we are
            // still allowed to terminate it (just not remove it) - but it makes
            // sense for this function to do neither if that privelege is not present

            // TES #703: Cannot terminate broadcasts that are currently in progress
            bool allowedToClearBroadcast = PaRightsManager::getInstance().hasRightToClearBroadcast(
                broadcastIds[i].c_str(), paAgent) &&
                !PaRightsManager::getInstance().isBroadcastInProgress(
                    broadcastIds[i].c_str(), paAgent);

            if (ignoreRightsChecking || allowedToClearBroadcast)
            {
                LOG_DEBUG("terminateAndRemoveBroadcastIds(): IPAAgentCorbaDef::terminateBroadcast: %s", nvps(broadcastIds[i]));
                CORBA_CALL(paAgent, terminateBroadcast, (broadcastIds[i].c_str(), CachedConfig::getInstance()->getSessionId().c_str()));

                LOG_DEBUG("terminateAndRemoveBroadcastIds(): IPAAgentCorbaDef::removeBroadcast: %s", nvps(broadcastIds[i]));
                CORBA_CALL(paAgent, removeBroadcast, (broadcastIds[i].c_str(), CachedConfig::getInstance()->getSessionId().c_str()));
            }
            else
            {
                PaErrorHandler::displayError(PaErrorHandler::INSUFFICIENT_RIGHTS);
            }
        }
        catch (const TA_Base_Bus::IPAAgentCorbaDef::BroadcastIdNotFound&)
        {
            // This broadcast ID already terminated, ignore exception & continue
            // (this is not an error)
        }
        catch (const TA_Base_Core::BadParameterException&)
        {
            // The removeBroadcast failed, which indicates terminateBroadcast
            // didn't work correctly (perhaps another user initiated
            // the broadcast again) - we just don't remove it from the list

            // There is a known issue where broadcast does not complete immediately
            // so it's quite likely for this error to occur (at this point in time)
            numberOfNonRemovals++;
        }
        catch (const TA_Base_Core::AccessDeniedException&)
        {
            // No privilege to remove this one
            numberOfNonRemovals++;
        }
    }

    return numberOfNonRemovals;
}

bool PaManagerModel::confirmStatusNameExists(const std::string& broadcastId)
{
    try
    {
        CWaitCursor hourGlassCursor;

        TA_Base_Bus::IPAAgentCorbaDef::BroadcastConfig_var var = CachedConfig::getInstance()->getAgentObject()->getBroadcastConfig(broadcastId.c_str());

        return true;
    }
    catch (const TA_Base_Bus::IPAAgentCorbaDef::BroadcastIdNotFound&)
    {
        PaErrorHandler::displayError(PaErrorHandler::BROADCAST_ID_NOT_FOUND);
        return false;
    }
    catch (...)
    {
        PaErrorHandler::displayError(PaErrorHandler::UNSPECIFIED_ERROR);
        return false;
    }
}
