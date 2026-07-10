/**
 * The source code in this file is the property of
 * Ripple Systems and is not for redistribution
 * in any form.
 *
 * Source: $File: //depot/C830AR_TIP/C830AR/transactive/app/pa/PAManager/src/PaRightsManager.h $
 * @author Sean Liew
 * @version $Revision: #4 $
 * Last modification: $DateTime: 2025/03/05 13:16:38 $
 * Last modified by: $Author: limin.zhu $
 *
 * Contains passthrough functions for querying the right to perform
 * various operations
 *
 */

#pragma once
#include "bus/pa_4669/pa_agent/IDL/src/IPAAgentCorbaDef.h"
#include "bus/pa_4669/pa_agent/IDL/src/IPAAgentCorbaDefDescribe.h"
#include "core/synchronisation/src/ReEntrantThreadLockable.h"
#include "core/data_access_interface/src/PrimitiveWrapper.h"
#include <boost/describe.hpp>
#include <set>
#include <vector>
#include <string>

interface IRightsChangeListener;

namespace TA_Base_Bus
{
    class RightsLibrary;
};

class PaRightsManager
{
public:

    /**
     * getInstance
     *
     * @return singleton instance of the rights manager
     *
     */
    static PaRightsManager& getInstance();

    /**
     * removeInstance
     *
     * Destroys the singleton instance of PaRights manager
     *
     */
    static void removeInstance();

    virtual ~PaRightsManager();

    /// Publically available rights - each time enum updated, must
    /// update PaRightsManager constructor to apply mappings
    enum EPublicRight
    {
        PA_BROADCAST_STATION_MUSIC,
        PA_BROADCAST_STATION_DVA,
        PA_MODIFY_ZONE_GROUPS,
        PA_MODIFY_ADHOC_MESSAGE,

        //Maochun Sun++
        //TD12798
        //PA_BROADCAST_TRAIN_DVA,
        TRAIN_PA_PRERECORDED_ANNOUNCEMENT,
        //++Maochun Sun
        //TD12798

        RADIO_MANAGE_TRAIN_SELECTIONS,

        // This define for local checking
        PUBLIC_RIGHT_MAX
    };

    BOOST_DESCRIBE_NESTED_ENUM(EPublicRight,
                               PA_BROADCAST_STATION_MUSIC,
                               PA_BROADCAST_STATION_DVA,
                               PA_MODIFY_ZONE_GROUPS,
                               PA_MODIFY_ADHOC_MESSAGE,
                               TRAIN_PA_PRERECORDED_ANNOUNCEMENT,
                               RADIO_MANAGE_TRAIN_SELECTIONS,
                               PUBLIC_RIGHT_MAX);

    /**
     * hasRights
     *
     * Public access to rights checking, passes on to internal implementation
     *
     */
    bool hasRight(EPublicRight right) const;

    /**
     * obtainReEntrantLock
     *
     * Obtains a thread re-entrant lock on all the local data information
     *
     */
    std::auto_ptr<TA_Base_Core::ThreadGuard> obtainReEntrantLock();

    /**
     * processStatusNameChangeRights
     *
     * This function does no locking (caller needs to do this)
     * Checks if the PAManager has the right to process the specified name change
     *  If PAManager does, the input broadcast IDs data will be updated to reflect
     *  the status name change (to keep locally created broadcast ID information up to date)
     *
     * @param nameChangeData the name change information to check
     *
     * @param agentObject agent used to determine session ID of broadcast
     *
     * @return true if processing should continue, false if the name change should
     *          be discarded (it doesn't concern us)
     *
     */
    bool processStatusNameChangeRights(
        const TA_Base_Bus::IPAAgentCorbaDef::CurrentBroadcastsUpdateData& nameChangeData,
        PAAgentNamedObject& agentObject) const;

    /**
     * hasRightToRetryBroadcast
     *
     * @return true if the operator has the right to retry the specified broadcast
     *
     * @param broadcastId the 'status name' of the broadcast to check
     *
     * @param agentObject agent used to determine session ID of broadcast
     *
     */
    bool hasRightToRetryBroadcast(const std::string& broadcastId, PAAgentNamedObject& agentObject);

    /**
     * hasRightToClearBroadcast
     *
     * @return true if we have a right to clear the input broadcast
     *
     * @param broadcastId the broadcast we're checking privileges for
     *
     * @param agentObject agent used to determine session ID of broadcast
     *
     */
    bool hasRightToClearBroadcast(const std::string& broadcastId, PAAgentNamedObject& agentObject) const;

    /**
     * isBroadcastInProgress
     *
     * @return true if the specified broadcast is still in progress
     *
     * @param broadcastId the broadcast we're checking
     *
     * @param agentObject agent used to determine session ID of broadcast
     *
     */
    bool isBroadcastInProgress(const std::string& broadcastId, PAAgentNamedObject& agentObject) const;

    /**
     * hasRightToBroadcastStationLive
     *
     * @return true if the operator has a right to make live station broadcast
     *
     */
    bool hasRightToBroadcastStationLive();

    /**
     * hasRightToBroadcastTrainLive
     *
     * @return true if the operator has a right to make live train broadcast
     *
     */
    bool hasRightToBroadcastTrainLive();

    /**
     * hasRightToRecordDvaMessage
     *
     * @return true if operator has right to record DVA (ad-hoc) messages
     *          from this terminal
     *
     */
    bool hasRightToRecordDvaMessage();

    /**
     * hasRightToPlaybackDvaMessage
     *
     * @return true if operator has right to play a DVA (ad-hoc) message
     *          from this terminal
     *
     */
    bool hasRightToPlaybackDvaMessage();

    /**
     * flagChangeInRights
     *
     * This is called whenever part of the using application has knowledge
     *  that there has been a change in rights - subsequently
     *  notifications are sent out to all registered rights change listeners
     *
     * @see registerRightsChangeListener
     *
     */
    void flagChangeInRights();

    /**
     * registerRightsChangeListener
     *
     * Registers a listener that is to receive notification of a rights change
     *
     * @pre listener is not already registered
     *
     */
    void registerRightsChangeListener(IRightsChangeListener& listener);

    /**
     * deRegisterRightsChangeListener
     *
     * Deregisters a listener that had been registered with registerRightsChangeListener
     *
     * @pre listener has been registered with registerRightsChangeListener
     *
     */
    void deRegisterRightsChangeListener(IRightsChangeListener& listener);

    /**
     * hasRightToDisplayBroadcast
     *
     * @return true if we have the right to view the input broadcast ID
     *
     * @param broadcastId the broadcast we're checking the rights to display for
     *
     * @param agentObject the agent to use in obtaining the broadcasts session ID
     *
     */
    bool hasRightToDisplayBroadcast(const std::string& broadcastId, PAAgentNamedObject& agentObject);

    /**
     * hasRightToDisplayBroadcastWithSessionId
     *
     * @param broadcastSessionId the session ID of the broadcast
     *
     * @return true if we have a right to display broadcasts with this session ID
     *
     */
    bool hasRightToDisplayBroadcastWithSessionId(const std::string& broadcastSessionId) const;

    /**
     * hasBroadcastIdBeenCreatedByThisSession
     *
     * @return true if the input broadcast ID has been created by the current session
     *
     * @param broadcastId the identifier of the broadcast
     *
     */
    bool hasBroadcastIdBeenCreatedByThisSession(const std::string& broadcastId, PAAgentNamedObject& agentObject) const;

private:

    /// Private use rights - each time enum updated, must
    /// update PaRightsManager constructor to apply mappings
    /// Private rights cane be indirectly accessed publicly by using
    /// the functions mentioned next to each enumerated value
    enum EPrivateRight
    {
        // Start off at last (MAX) public rights ID
        //Maochun Sun++
        //TD12798
        //PA_BROADCAST_TRAIN_LIVE = PUBLIC_RIGHT_MAX,        // see hasRightToBroadcastTrainLive
        TRAIN_PA_LIVE_ANNOUNCEMENT = PUBLIC_RIGHT_MAX,
        //Maochun Sun++
        //TD12798

        PA_RETRY_LOCAL_BROADCASTS,      // see hasRightToRetryBroadcast
        PA_RECORD_ADHOC_MESSAGE,        // see hasRightToRecordDvaMessage
        PA_BROADCAST_STATION_LIVE,      // see hasRightToBroadcastStationLive
        PA_DISPLAY_LOCAL_BROADCAST,     // see hasRightToDisplayBroadcast, hasRightToDisplayBroadcastWithSessionId
        PA_TERMINATE_LOCAL_BROADCASTS,  // see hasRightToClearBroadcast

        PA_RIGHT_MAX
    };

    BOOST_DESCRIBE_NESTED_ENUM(EPrivateRight,
                               TRAIN_PA_LIVE_ANNOUNCEMENT,
                               PA_RETRY_LOCAL_BROADCASTS,
                               PA_RECORD_ADHOC_MESSAGE,
                               PA_BROADCAST_STATION_LIVE,
                               PA_DISPLAY_LOCAL_BROADCAST,
                               PA_TERMINATE_LOCAL_BROADCASTS,
                               PA_RIGHT_MAX);

    /**
     * hasRights
     *
     * @param operation the operation we are checking the rights of
     *
     */
    bool hasRight(unsigned long right) const;

    /**
     * isPhysicallyLocatedAtOperatingLocation
     *
     * @return true if the PAManager's physical location corresponds to the
     *      location it is operating on (or running for)
     *      (ie a terminal may be running the PAManager for the neighbouring
     *          station, meaning this returns false)
     *
     */
    bool isPhysicallyLocatedAtOperatingLocation();

    /**
     * getResourceKey
     *
     * Caches the resource key once successfully obtained
     *
     * @return the resource key for this entity (for use
     *      with the rights manager
     *
     * @exception ValueNotSetException if the resource key could
     *              not be identified
     *
     */
    unsigned long getResourceKey() const;

    /**
     * convertRightIdentifier
     *
     * Converts the input locally defined (enum) right into the globally
     *  recognized right for system
     *
     * @param localRight the index of local right
     *
     * @return the identifier of global right (for use with RightsManager)
     *
     * @exception ValueNotSetException if no mapping has been set up
     *
     */
    unsigned long convertLocalRightIdentifier(unsigned long localRight) const;

    /**
     * debugConfirmRightsExist (purely for debugging purposes)
     *
     * returns true if all rights exist (all checks passed)
     *
     * @pre all rights exist, and there are no many to one relationships
     *
     */
    bool debugConfirmRightsExist();

    // Singleton
    PaRightsManager();

    // Lock to ensure any local data information that need to be check
    // in order to determine rights can be blocked from asynchronous
    // updates (requests) from other threads (must maintain as a ReEntrant!)
    mutable TA_Base_Core::ReEntrantThreadLockable    m_lock;

    // Maintains mapping to global rights identifiers (we use local enum identifiers
    // within the PA Manager code)
    std::map<unsigned long, unsigned long> m_localToGlobalRightsMap;

    // Cached copy for use in hasRight
    mutable TA_Base_Bus::RightsLibrary* m_rightsLibrary = nullptr;
    // Maintains cached copy from getResourceKey
    mutable TA_Base_Core::PrimitiveWrapper<unsigned long> m_resourceKey;

    std::set<IRightsChangeListener*> m_rightsChangeListeners;

    static PaRightsManager* s_me;
};
