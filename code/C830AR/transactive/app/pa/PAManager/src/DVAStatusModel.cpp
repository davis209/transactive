/**
 * The source code in this file is the property of
 * Ripple Systems and is not for redistribution
 * in any form.
 *
 * Source: $File: //depot/C830AR_TIP/C830AR/transactive/app/pa/PAManager/src/DVAStatusModel.cpp $
 * @author Jade Lee
 * @version $Revision: #3 $
 * Last modification: $DateTime: 2025/02/10 22:34:14 $
 * Last modified by: $Author: limin.zhu $
 *
 * DvaStatusModel controls the shared business logic for any DVA status dialog screens
 * - All Dva Status screens have a status name that the user may change
 *
 */

#include "app/pa/PAManager/src/StdAfx.h"
#include "app/pa/PAManager/src/DvaStatusModel.h"
#include "app/pa/PAManager/src/PaManagerModel.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

DvaStatusModel::DvaStatusModel(GenericStatusPage& page,
                               const std::string& broadcastId,
                               const EModelSubType& subType)
    :
    GenericStatusModel(page, broadcastId, subType),
    m_statusNameChangeSubscriber(PaStatusNameChangeSubscriber::getInstance())
{
}

bool DvaStatusModel::validateStatusNameString(const std::string& str)
{
    // song toan++
    // TD 730
    //
    // string length will not be limited by STATUS_DVA_NAME_MAX anymore
    if (0 == str.length())
        // ++song toan
        // TD 730
    {
        return false;
    }

    // String is same as original, return true early (otherwise will detect as duplicate)
    if (str == getBroadcastId())
    {
        return true;
    }

    if (std::find(m_cachedStatusNames.begin(), m_cachedStatusNames.end(), str)
        != m_cachedStatusNames.end())
    {
        // A duplicate string was found in the cache
        return false;
    }

    // All checks passed, string valid
    return true;
}

bool DvaStatusModel::synchronizeBroadcastId(const std::string& newBroadcastId)
{
    LOG_SCOPE("DvaStatusModel::synchronizeBroadcastId");

    // If the strings are the same, no synchronization required (already in sync)
    if (getBroadcastId() == newBroadcastId)
    {
        return true;
    }

    try
    {
        CWaitCursor hourGlassCursor;

        LOG_DEBUG("synchronizeBroadcastId(): IPAAgentCorbaDef::changeBroadcastId: %s", nvps(getBroadcastId(), newBroadcastId));

        CORBA_CALL(CachedConfig::getInstance()->getAgentObject(), changeBroadcastId, (
            getBroadcastId().c_str(),
            newBroadcastId.c_str(),
            CachedConfig::getInstance()->getSessionId().c_str()));

        // In the absence of an exception being raised the operation must have
        // been successful - now update our cached name to reflect what it's been changed to
        setBroadcastId(newBroadcastId);

        return true;
    }
    catch (const TA_Base_Core::BadParameterException&)
    {
        // Some other thread in the system has used this name before we could
        // (taken the target name, with their own name change / addition)
        return false;
    }
    catch (const TA_Base_Bus::IPAAgentCorbaDef::BroadcastIdNotFound&)
    {
        // Pass this one up, synchronization error
        throw;
    }
}

void DvaStatusModel::deleteStatusName(const std::string& str)
{
    LOG_SCOPE("DvaStatusModel::deleteStatusName");

    std::vector<std::string> broadcastIds;
    broadcastIds.push_back(str);

    // Allow exceptions to propogate..
    int numRemovals = PaManagerModel::terminateAndRemoveBroadcastIds(
        broadcastIds,
        CachedConfig::getInstance()->getAgentObject());

    // If numRemovals == 0, perhaps status name was already removed..
}

void DvaStatusModel::updateStatusNameArrayCache()
{
    LOG_SCOPE("DvaStatusModel::updateStatusNameArrayCache");

    try
    {
        TA_Base_Bus::IPAAgentCorbaDef::EBroadcastType broadcastFilter;

        // Choose appropriate filter to determine status names
        if (isTrainModel())
        {
            broadcastFilter = TA_Base_Bus::IPAAgentCorbaDef::TrainDva;
        }
        else
        {
            broadcastFilter = TA_Base_Bus::IPAAgentCorbaDef::StationDva;
        }

        // Updates for Defect #3152 - when caching existing status names, ignore the rights checking filter
        // because if we don't have right to view a broadcast, we still can't overwrite it..
        m_cachedStatusNames = PaManagerModel::getDvaBroadcastZones(
            CachedConfig::getInstance()->getAgentObject(),
            broadcastFilter,
            PaManagerModel::IGNORE_RIGHTS_CHECKING);
    }
    catch (const TA_Base_Core::ObjectResolutionException&)
    {
        // The status name cache will remain empty, meaning the user
        // will be given no feedback if the string is a duplicate of existing
        // PA zone - but no major issue, because this will all be double checked
        // When user attempts to apply the changes (by closing the dialog)
        PaErrorHandler::displayError(PaErrorHandler::PA_AGENT_FAIL);
    }
    catch (const CORBA::Exception&)
    {
        // Comms error
        PaErrorHandler::displayError(PaErrorHandler::PA_AGENT_FAIL);
    }
}

bool DvaStatusModel::onStatusNameChange(DWORD id, HWND hWnd)
{
    if (0 == getBroadcastId().length())
    {
        // This is a deleted broadcast - do not respond to status name change
        // TestDirector Defect #3097
        return false;
    }

    const TA_Base_Bus::IPAAgentCorbaDef::CurrentBroadcastsUpdateData nameChange =
        m_statusNameChangeSubscriber.getMessageData(id, hWnd);

    // We should only inspect/respond to name changes for DVA messages
    if (nameChange.broadcastType != TA_Base_Bus::IPAAgentCorbaDef::TrainDva &&
        nameChange.broadcastType != TA_Base_Bus::IPAAgentCorbaDef::StationDva)
    {
        // Not a DVA status name, can safely ignore
        // This name change didn't affect us (return false)
        return false;
    }

    // The only name change that can affect us, is one that changes our current
    // internal name to something else (either removal or rename)
    if (std::string(nameChange.fromBroadcastId) == getBroadcastId())
    {
        // Name has changed to something else - must ensure we renew subscription
        // to continue getting progress report notifications
        setBroadcastId(std::string(nameChange.toBroadcastId));

        // Re-subscribe now that broadcast ID has changed, to ensure get latest updates
        // (from the new broadcast ID - the one we've changed to)
        refreshSubscription();

        return true;
    }
    else
    {
        // This name change didn't affect us
        return false;
    }
}

void DvaStatusModel::initializeStatusNameSubscriber(HWND hWnd)
{
    // Initialize the connection
    m_statusNameChangeSubscriber.addListenerWindow(hWnd);

    // Perform an initial storage of the Status Name array - this is not updated
    // at runtime, as it's expected there will be infrequent updates to Status Name array
    // But it does serve to help ensure things are in sync
    updateStatusNameArrayCache();

    std::string str = getBroadcastId();

    // Ensure the current broadcast ID exists in cache - if not, we cannot open
    // the dialog (out of sync) - if exists in cache, it means that we're
    // going to be able to track all subsequent updates through the status name subscriber
    if (std::find(m_cachedStatusNames.begin(), m_cachedStatusNames.end(), str)
        == m_cachedStatusNames.end())
    {
        TA_THROW(TA_Base_Core::TransactiveException("StatusDvaStatusModel: Out of sync with PA Agent"));
    }
}

void DvaStatusModel::removeStatusNameSubscriber(HWND hWnd)
{
    m_statusNameChangeSubscriber.removeListenerWindow(hWnd);
}

bool DvaStatusModel::isConnectedToWindow(HWND hWnd)
{
    return m_statusNameChangeSubscriber.isConnectedListenerWindow(hWnd);
}
