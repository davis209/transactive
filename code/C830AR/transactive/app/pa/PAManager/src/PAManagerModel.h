/**
 * The source code in this file is the property of
 * Ripple Systems and is not for redistribution
 * in any form.
 *
 * Source: $File: //depot/C830AR_TIP/C830AR/transactive/app/pa/PAManager/src/PAManagerModel.h $
 * @author Jade Lee
 * @version $Revision: #3 $
 * Last modification: $DateTime: 2025/01/20 18:40:08 $
 * Last modified by: $Author: limin.zhu $
 *
 * PaManagerModel controls the shared business logic for the PA Manager
 */

#if !defined(PaManagerModel_4EBD5CE7_C4D0_43BC_A835_518CA9141975__INCLUDED_)
#define PaManagerModel_4EBD5CE7_C4D0_43BC_A835_518CA9141975__INCLUDED_

#include "app/pa/PAManager/src/PAProgressReportSubscriber.h"
#include "bus/pa_4669/pa_agent/IDL/src/IPAAgentCorbaDef.h"
#include "bus/pa_4669/pa_agent/IDL/src/IPAAgentCorbaDefDescribe.h"
#include "app/pa/PAManager/src/PAStatusNameChangeSubscriber.h"
#include "app/pa/PAManager/src/DvaMessageChangeSubscriber.h"

class PaManagerModel
{
public:

    enum ERightsCheckingMode
    {
        USE_RIGHTS_CHECKING,
        IGNORE_RIGHTS_CHECKING
    };

    /**
     * PaManagerModel (constructor)
     *
     *
     *
     */
    PaManagerModel();

    virtual ~PaManagerModel();

    /*
     * getDvaBroadcastZones
     *
     * Returns all the 'visible' broadcast zones - only the Dva broadcast zones
     * should be made visible to the user (live & music are hidden from the PA Manager GUI)
     * The returned array is context sensitive - that is, the stations will not receive
     * the OCC broadcast zones, and vice-versa.  Also any broadcast IDs that the
     * operator doesn't have the privilege to view (necessary rights) will not be
     * returned
     *
     * @param paAgent the reference to the agent used to perform request
     *
     * @param filter the type of broadcast zones to retrieve (ie train or station)
     *
     * @param filterByRightToView a flag defining whether to filter out all broadcast
     *              zones that we don't have a right to view.  Can also set to retrieve
     *              all broadcast zones, regardless of whether we have viewing rights or not.
     *
     * @return an array of all the zone names that match the filter criteria
     *
     * @exception ObjectResolutionException if there is an error retrieving the PA Agent
     *
     */
    static std::vector<std::string> getDvaBroadcastZones(PAAgentNamedObject& paAgent,
                                                         const TA_Base_Bus::IPAAgentCorbaDef::EBroadcastType& filter,
                                                         ERightsCheckingMode filterByRightToView = USE_RIGHTS_CHECKING);

    /*
     * getStationDvaBroadcastZones
     *
     * Passes on local pa agent as parameter to static implementation (see that for details)
     * To retrieve broadcast zones for the station
     *
     */
    std::vector<std::string>    getStationDvaBroadcastZones();

    /*
     * getTrainDvaBroadcastZones
     *
     * Passes on local pa agent as parameter to static implementation (see that for details)
     * To retrieve broadcast zones for the station
     *
     */
    std::vector<std::string>    getTrainDvaBroadcastZones();

    /**
     * initializeStatusNameSubscriber
     *
     * Connects to the Status Name subscriber, to we receive updates whenever
     * changes occur to status names
     *
     * @param hWnd the handle to the window to receive the changed notifications
     */
    void    initializeStatusNameSubscriber(HWND hWnd);

    /**
     * Unsubscribes the specified window from the status name subscriber
     *
     * @see initializeStatusNameSubscriber
     *
     */
    void    removeStatusNameSubscriber(HWND hWnd);

    /**
     * collectStatusNameChange
     *
     * Called when a Status Name change notification has been received by associated view
     *
     * @param id the identifier of the Status Name change packet
     *
     * @param hWnd the handle of the window that received the changed notification
     *
     * @param from [OUT] the name of the item being changed
     *
     * @param to [OUT] the name that the items being changed to
     *
     * @return the type of broadcast for this message
     *
     * @exception ValueNotSetException if the specified name change object doesn't exist
     *
     * @exception TransactiveException if we're not connected to the subscriber anymore
     *
     */
    TA_Base_Bus::IPAAgentCorbaDef::EBroadcastType collectStatusNameChange(DWORD id, HWND hWnd, std::string& from, std::string& to);

    /**
     * terminateAndRemoveBroadcastIds
     *
     * Terminates and removes all the broadcastIds specified in array
     *
     * @param broadcastIds array of broadcast Ids we want to have removed
     *
     * @return number of broadcasts that couldn't be removed, because they couldn't
     *          be terminated properly (ie another terminal activating them)
     *
     * @exception ObjectResolutionNotSet if there is a CORBA resolving conflict (comm error)
     *
     * @param origin defines the creation category (origin) of this broadcast - if available
     *
     */
    static int terminateAndRemoveBroadcastIds(const std::vector<std::string>& broadcastIds,
                                              PAAgentNamedObject& paAgent,
                                              EBroadcastOrigin origin = BO_UNKNOWN);

    /**
     * terminateAndRemoveBroadcastIds
     *
     * Passes through to static implementation, using local PA Agent as argument
     *
     */
    int terminateAndRemoveBroadcastIds(const std::vector<std::string>& broadcastIds);

    /// Simple accessor
    DvaMessageChangeSubscriber& getDvaMessageChangeSubscriber()
    {
        return m_dvaMessageChangeSubscriber;
    }

    /**
     * confirmStatusNameExists
     *
     * Confirms the input status name (broadcast ID) exists within the PA Agent
     * If the broadcast doesn't exist, an error dialog will be displayed
     *  (this function is only to double-check/confirm a status name exists)
     *
     * @param broadcastId the name of the status name to check
     *
     * @return true if the specified broadcast ID was found
     *
     */
    bool confirmStatusNameExists(const std::string& broadcastId);

private:

    // This should be the only instance, passed around to any parts of the app
    // that need this notifications service
    DvaMessageChangeSubscriber      m_dvaMessageChangeSubscriber;

    PaStatusNameChangeSubscriber& m_statusNameChangeSubscriber;
};

#endif // !defined(PaManagerModel_4EBD5CE7_C4D0_43BC_A835_518CA9141975__INCLUDED_)
