/**
 * The source code in this file is the property of
 * Ripple Systems and is not for redistribution
 * in any form.
 *
 * Source: $File: //depot/C830AR_TIP/C830AR/transactive/app/pa/PAManager/src/PaRightsManager.cpp $
 * @author Sean Liew
 * @version $Revision: #4 $
 * Last modification: $DateTime: 2025/02/10 22:34:14 $
 * Last modified by: $Author: limin.zhu $
 *
 * Contains passthrough functions for querying the right to perform
 * various operations
 *
 */

#pragma warning(disable:4284)   // For std::set warnings

#include "app/pa/PAManager/src/stdafx.h"
#include "app/pa/PAManager/src/PaRightsManager.h"
#include "core/synchronisation/src/ThreadGuard.h"
#include "bus/security/rights_library/src/RightsLibrary.h"
#include "core/data_access_interface/src/ResourceAccessFactory.h"
#include "core/utilities/src/DebugUtil.h"
#include "core/utilities/src/stdutil/strings.h"
#include "core/utilities/src/containers/Map.h"
#include "app/pa/PAManager/src/CachedConfig.h"
#include "bus/security/access_control/actions/src/AccessControlledActions.h"
#include "app/pa/PAManager/src/IRightsChangeListener.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

PaRightsManager* PaRightsManager::s_me = 0;

PaRightsManager::PaRightsManager()
{
    /////////////////////////START define right mappings
#define MAP_RIGHT(x) { m_localToGlobalRightsMap[x] = TA_Base_Bus::aca_##x; }

    MAP_RIGHT(PA_DISPLAY_LOCAL_BROADCAST);
    MAP_RIGHT(PA_TERMINATE_LOCAL_BROADCASTS);
    MAP_RIGHT(PA_BROADCAST_STATION_MUSIC);
    MAP_RIGHT(PA_BROADCAST_STATION_DVA);
    MAP_RIGHT(PA_BROADCAST_STATION_LIVE);
    MAP_RIGHT(PA_MODIFY_ZONE_GROUPS);
    MAP_RIGHT(PA_MODIFY_ADHOC_MESSAGE);
    MAP_RIGHT(PA_RECORD_ADHOC_MESSAGE);
    MAP_RIGHT(PA_RETRY_LOCAL_BROADCASTS);

    //Maochun Sun++
    //TD12798
    //MAP_RIGHT(PA_BROADCAST_TRAIN_LIVE);
    //MAP_RIGHT(PA_BROADCAST_TRAIN_DVA);
    MAP_RIGHT(TRAIN_PA_PRERECORDED_ANNOUNCEMENT);
    MAP_RIGHT(TRAIN_PA_LIVE_ANNOUNCEMENT);
    //++Maochun Sun
    //TD12798

    MAP_RIGHT(RADIO_MANAGE_TRAIN_SELECTIONS);

#undef MAP_RIGHT
    /////////////////////////END define right mappings

    // Confirm all the rights exist (debug)
    TA_ASSERT(debugConfirmRightsExist(), "Rights map not set correctly");
}

PaRightsManager::~PaRightsManager()
{
    try
    {
        delete m_rightsLibrary;
        m_rightsLibrary = 0;
    }
    catch (...)
    {
        LOG(SourceInfo, TA_Base_Core::DebugUtil::ExceptionCatch, "Unknown", "Caught in destructor");
    }
}

PaRightsManager& PaRightsManager::getInstance()
{
    if (0 == s_me)
    {
        s_me = new PaRightsManager();

        TA_ASSERT(s_me, "Unable to create");
    }

    return *s_me;
}

void PaRightsManager::removeInstance()
{
    if (s_me != NULL)
    {
        delete s_me;
        s_me = NULL;
    }
}

bool PaRightsManager::debugConfirmRightsExist()
{
    try
    {
        std::set<unsigned long> temp;

        for (int i = 0; i < PA_RIGHT_MAX; i++)
        {
            if (!temp.insert(convertLocalRightIdentifier(i)).second)
            {
                TA_ASSERT(false, "Multiple local rights mapped to same global right");
            }
        }
    }
    catch (const TA_Base_Core::ValueNotSetException&)
    {
        TA_ASSERT(false, "Local right not identified by map");
    }

    return true;
}

unsigned long PaRightsManager::convertLocalRightIdentifier(unsigned long localRight) const
{
    std::map<unsigned long, unsigned long>::const_iterator itr = m_localToGlobalRightsMap.find(localRight);

    if (itr == m_localToGlobalRightsMap.end())
    {
        TA_THROW(TA_Base_Core::ValueNotSetException());
    }

    return itr->second;
}

bool PaRightsManager::hasRight(EPublicRight right) const
{
    LOG_SCOPE(boost::format("PaRightsManager::hasRight[%s]") % st::enum_to_string_value(right));
    // Access internal implementation (non typesafe)

    auto result = hasRight((unsigned long)right);
    LOG_DEBUG("hasRight(): %s", nvps(right, result));

    return result;
}

bool PaRightsManager::hasRight(unsigned long right) const
{
    LOG_SCOPE(boost::format("PaRightsManager::hasRight[%d]") % right);

    if (0 == m_rightsLibrary)
    {
        // Postpone instancing of rights library until first request
        TA_Base_Bus::RightsLibraryFactory rlFactory;

        m_rightsLibrary = rlFactory.buildRightsLibrary(TA_Base_Bus::DEFAULT_RL, TA_Base_Bus::RightsLibraryFactory::MANAGER);
    }

    if (0 == m_rightsLibrary)
    {
        // Unable to create rights library, assume no rights by default
        return false;
    }

#ifdef _DEBUG

    if (CachedConfig::getInstance()->getSessionId().compare("debug") == 0)
    {
        static bool sRights[PA_RIGHT_MAX] = {true, true, true, true, true, true, true, true, true, true, true};

        /*
        std::string param = TA_Base_Core::RunParams::getInstance().get("rights");
        if (param.length() > 0 && param.find(",") != std::string::npos)
        {
            std::string indexStr = param.substr(0, param.find(",") );

            sRights[atoi( indexStr.c_str() )] = param.at(param.length() - 1) == '1';
        }
        */

        // If debug session ID defined, assume true (debug mode) - can change
        // the rights at runtime as required
        return sRights[right];
    }

#endif

    bool response = false;

    try
    {
        unsigned long resourceKey = getResourceKey();
        std::string reasonString;
        TA_Base_Bus::EDecisionModule decisionModule = TA_Base_Bus::UNDEFINED_DM;
        unsigned long globalRightIdentifier = convertLocalRightIdentifier(right);

        response = m_rightsLibrary->isActionPermittedOnResource(CachedConfig::getInstance()->getSessionId(),
                                                                resourceKey,
                                                                globalRightIdentifier,
                                                                reasonString,
                                                                decisionModule);
    }
    catch (const TA_Base_Core::RightsException&)
    {
    }
    catch (const TA_Base_Core::ValueNotSetException&)
    {
    }

    return response;
}

unsigned long PaRightsManager::getResourceKey() const
{
    LOG_SCOPE("PaRightsManager::getResourceKey");

    if (m_resourceKey.hasBeenSet())
    {
        // Return the cached version
        return m_resourceKey.getValue();
    }

    unsigned long entityKey = CachedConfig::getInstance()->getEntityKey();
    TA_Base_Core::IResource* resource = TA_Base_Core::ResourceAccessFactory::getInstance().getResourceFromEntity(entityKey);

    if (0 == resource)
    {
        TA_THROW(TA_Base_Core::ValueNotSetException());
    }

    m_resourceKey.setValue(resource->getKey());

    delete resource;

    return m_resourceKey.getValue();
}

std::auto_ptr<TA_Base_Core::ThreadGuard> PaRightsManager::obtainReEntrantLock()
{
    std::auto_ptr<TA_Base_Core::ThreadGuard> result(new TA_Base_Core::ThreadGuard(m_lock));
    return result;
}

bool PaRightsManager::hasRightToClearBroadcast(const std::string& broadcastId, PAAgentNamedObject& agentObject) const
{
    if (hasRight(PA_TERMINATE_LOCAL_BROADCASTS))
    {
        // We have the right to clear any broadcast we like
        return true;
    }

    // Otherwise we can only clear broadcasts created in this session
    return hasBroadcastIdBeenCreatedByThisSession(broadcastId, agentObject);
}

bool PaRightsManager::isBroadcastInProgress(const std::string& broadcastId, PAAgentNamedObject& agentObject) const
{
    LOG_SCOPE("PaRightsManager::isBroadcastInProgress");

    try
    {
        TA_Base_Bus::IPAAgentCorbaDef::ProgressReport_var progressReport;
        CORBA_CALL_RETURN(progressReport, agentObject, getProgressReport, (broadcastId.c_str()));

        // Broadcast is complete once enters pending removal state
        return (TA_Base_Bus::IPAAgentCorbaDef::PendingRemoval != progressReport->broadcastState);
    }
    catch (...)
    {
        // If there is any doubt, assume in progress
        return true;
    }
}

bool PaRightsManager::hasRightToRetryBroadcast(const std::string& broadcastId, PAAgentNamedObject& agentObject)
{
    if (hasRight(PA_RETRY_LOCAL_BROADCASTS))
    {
        // Have a right to retry any broadcast on the agent
        return true;
    }

    // Otherwise only those created in this session
    return hasBroadcastIdBeenCreatedByThisSession(broadcastId, agentObject);
}

bool PaRightsManager::hasRightToBroadcastStationLive()
{
    if (hasRight(PA_BROADCAST_STATION_LIVE))
    {
        // In addition to the right, the operator must be physically
        // located (defined by locationKey) at the station we're operating for
        if (isPhysicallyLocatedAtOperatingLocation())
        {
            return true;
        }
    }

    return false;
}

bool PaRightsManager::hasRightToPlaybackDvaMessage()
{
    // Operator must be physically located at the same station
    // to allow playing of DVA Messages
    if (isPhysicallyLocatedAtOperatingLocation())
    {
        return true;
    }
    else
    {
        return false;
    }
}

bool PaRightsManager::hasRightToRecordDvaMessage()
{
    if (hasRight(PA_RECORD_ADHOC_MESSAGE))
    {
        // In addition to the right, operator must be physically located at station
        if (isPhysicallyLocatedAtOperatingLocation())
        {
            return true;
        }
    }

    return false;
}

bool PaRightsManager::isPhysicallyLocatedAtOperatingLocation()
{
    return CachedConfig::getInstance()->getPhysicalLocationKey() == CachedConfig::getInstance()->getEntityLocationKey();
}

bool PaRightsManager::hasRightToBroadcastTrainLive()
{
    //Maochun Sun++
    //TD12798
    //if (hasRight(PA_BROADCAST_TRAIN_LIVE))
    if (hasRight(TRAIN_PA_LIVE_ANNOUNCEMENT))
        //++Maochun Sun
        //TD12798
    {
        // In addition to the right, the operator must be physically
        // located (defined by locationKey) at the station we're operating for
        if (isPhysicallyLocatedAtOperatingLocation())
        {
            return true;
        }
    }

    return false;
}

void PaRightsManager::registerRightsChangeListener(IRightsChangeListener& listener)
{
    TA_Base_Core::ThreadGuard sync(m_lock);

    if (!m_rightsChangeListeners.insert(&listener).second)
    {
        TA_ASSERT(FALSE, "Listener already registered");
    }
}

void PaRightsManager::deRegisterRightsChangeListener(IRightsChangeListener& listener)
{
    TA_Base_Core::ThreadGuard sync(m_lock);

    std::set<IRightsChangeListener*>::iterator itr = m_rightsChangeListeners.find(&listener);

    if (itr == m_rightsChangeListeners.end())
    {
        TA_ASSERT(FALSE, "Could not find listener to deRegister");
    }
    else
    {
        m_rightsChangeListeners.erase(itr);
    }
}

void PaRightsManager::flagChangeInRights()
{
    TA_Base_Core::ThreadGuard sync(m_lock);

    std::vector<unsigned long> globalRightIds;

    // For now, send out notification that all rights have changed
    for (int i = 0; i < PA_RIGHT_MAX; i++)
    {
        globalRightIds.push_back(convertLocalRightIdentifier(i));
    }

    for (std::set<IRightsChangeListener*>::iterator itr = m_rightsChangeListeners.begin();
         itr != m_rightsChangeListeners.end();
         itr++)
    {
        (*itr)->onRightsChanged(globalRightIds);
    }
}

bool PaRightsManager::processStatusNameChangeRights(
    const TA_Base_Bus::IPAAgentCorbaDef::CurrentBroadcastsUpdateData& nameChangeData,
    PAAgentNamedObject& agentObject) const
{
    return hasRightToDisplayBroadcastWithSessionId(nameChangeData.sessionId.in());
}

bool PaRightsManager::hasRightToDisplayBroadcast(const std::string& broadcastId, PAAgentNamedObject& agentObject)
{
    if (hasRight(PA_DISPLAY_LOCAL_BROADCAST))
    {
        // We're allowed to interact with all broadcasts, not just the
        // one within the session
        return true;
    }

    // Otherwise only allowed to view those broadcasts created in this session
    return hasBroadcastIdBeenCreatedByThisSession(broadcastId, agentObject);
}

bool PaRightsManager::hasRightToDisplayBroadcastWithSessionId(const std::string& broadcastSessionId) const
{
    TA_Base_Core::ThreadGuard sync(m_lock);

    if (hasRight(PA_DISPLAY_LOCAL_BROADCAST))
    {
        // We're allowed to interact with all broadcasts, not just the
        // one within the session
        return true;
    }

    // We're allowed to display all broadcasts that have a matching session ID
    return CachedConfig::getInstance()->getSessionId() == broadcastSessionId;
}

bool PaRightsManager::hasBroadcastIdBeenCreatedByThisSession(const std::string& broadcastId, PAAgentNamedObject& agentObject) const
{
    try
    {
        const TA_Base_Bus::IPAAgentCorbaDef::BroadcastConfig_var data = agentObject->getBroadcastConfig(broadcastId.c_str());

        // Return true if the broadcasts session ID matches our own
        return CachedConfig::getInstance()->getSessionId() == data->sessionId.in();
    }
    catch (...)
    {
        // If there is any doubt, assume cannot display
        return false;
    }
}

/*
The various rights are handled in the following way:

PA_DISPLAY_LOCAL_BROADCAST [DONE]
    "All currently executing broadcasts on the local PA Agent are displayed,
    otherwise only broadacasts initiated within this session are displayed"
        * OnGoing DVA Status list will only display all local if this privilege present
    ] As status name changes come through, checked against rights manager
    ] Any calls to collect all existing broadcast IDs automatically filters out
        those we don't have the right to view

PA_TERMINATE_LOCAL_BROADCASTS [DONE]
    In order to use the clear DVA for broadcasts local but not initiated within the operator session
        * The Clear DVA Button will be greyed out if no right to delete broadcast (cos we didn't spawn it)
        * The Clear All DVA Button will be greyed out if cannot delete any of the broadcasts, otherwise
                it will only delete the ones we're allowed to
    ] @see hasRightToClearBroadcast, hasRightToClearAny
    ] Secondary check whenever calling removeBroadcast (all requests checked) - displays
        error if user manages to get option to do this
    ] During clear all, only clears those we have rights to clear

PA_BROADCAST_STATION_MUSIC [DONE]
    * Required in order to select + execute the OCC background music
    * Required in order to select + execute station background music
    * Required to turn music OFF
    ] Handled (not allowed to cycle, and defaults to OFF) -
        but shouldn't be enabled in first place
    ] In a refresh function button is enabled or disabled as applicable
        @see StationBroadcastPage::refreshBroadcastButtons
    ] Just before any requests to PA Agent, a secondary test made [broadcastStationMusic]

PA_BROADCAST_STATION_LIVE [DONE] hasRightToBroadcastStationLive
    * Required for live broadcasts -
        operator must also be physically located at station defined by locationKey
    ] There is a single function to test if allowed to broadcast live (which
        tests for both conditions mentioned above), @see    hasRightToBroadcastStationLive
    ] In a refresh function button is enabled or disabled as applicable
        @see StationBroadcastPage::refreshBroadcastButtons
    ] Secondary check done just before making request to PA Agent [broadcastStationLive]

PA_BROADCAST_STATION_DVA [DONE]
    * Required for DVA broadcasts
    ] In a refresh function button is enabled or disabled as applicable
        @see StationBroadcastPage::refreshBroadcastButtons
    ] Secondary check done just before making a request to PA Agent [broadcastStationDva]

PA_MODIFY_ZONE_GROUPS [DONE]
    * Required to save zone groups (disable save button if not present)
    * Required to delete zone groups
    ] Save button disabled, secondary check on save button click, third check on code
        that actually saves the group [since operator prompted for name hence delay exists]
    ] Delete button disabled, secondary check on clicking (where deletion occurs)

PA_MODIFY_ADHOC_MESSAGE  [DONE]
    * Required for modifying the ad-hoc type or label
        [display message box upon attempt to edit label]
    ] If disabled, the list control doesn't display the checkboxes (@see refreshAdhocListStyle)
    ] Secondary check when attempting to call setAdhocType
    ] Secondary check when attemting to call setAdhocLabel
    ] If user attempts to edit a label when rights not present,
        displays error (can't edit) - @see OnBeginlabeleditStnAdhocMessagesLst

PA_RECORD_ADHOC_MESSAGE [DONE] @see hasRightToRecordDvaMessage()
    * Required for enabling the recording of ad-hoc messages (and associated buttons)
    ] Button disabled
    ] Button disabled in mini recording dialog (@see updateButtonStates)
    ] Secondary check in OnStnRecordBtn (to disable popping up dialog)
    ] Third stage check in OnStartRecording
    ] Final stage check just before PA Agent call

PA_RETRY_LOCAL_BROADCASTS [DONE]
    * Required for <any>retry of broadcasts that we didn't initiate in this session
        in the status dialogs - for train or dva
    ] Buttons disabled, @see hasRightToRetryBroadcast
    ] Secondary check (just before making PA request)
        on retryStationBroadcast and retryTrainBroadcast

PA_BROADCAST_TRAIN_LIVE [DONE] hasRightToBroadcastTrainLive
    * Required for live broadcasts on train (Live Broadcast button disable)
    ] Button enabled state updated based on mode (@see refreshBroadcastButtons)
    ] Secondary check just before making PA request, @see broadcastTrainLive

PA_BROADCAST_TRAIN_DVA [DONE]
    * Required for train DVA broadcasts
    ] Button enabled state updated based on mode (@see refreshBroadcastButtons)
    ] Secondary check just before making PA request, @see broadcastTrainDva

  [NULL] @see hasRightToPlaybackDvaMessage
    * Special case right - doesn't exist in rights manager, but restrictions nonetheless
        apply when an operator is interfacing with a foreign station
  ] Disables playback button
  ] Performs secondary check just before making PA Agent call
*/
