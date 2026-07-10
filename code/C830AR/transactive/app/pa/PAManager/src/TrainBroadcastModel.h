/**
 * The source code in this file is the property of
 * Ripple Systems and is not for redistribution
 * in any form.
 *
 * Source: $File: //depot/C830AR_TIP/C830AR/transactive/app/pa/PAManager/src/TrainBroadcastModel.h $
 * @author Jade Lee
 * @version $Revision: #2 $
 * Last modification: $DateTime: 2024/12/17 16:14:02 $
 * Last modified by: $Author: limin.zhu $
 *
 * TrainBroadcastModel models behaviour for the train broadcast GUI tab of the PA Manager GUI
 */

#if !defined(AFX_TrainBroadcastModel_H__4EBD5CE7_C4D0_43BC_A835_518CA9141975__INCLUDED_)
#define AFX_TrainBroadcastModel_H__4EBD5CE7_C4D0_43BC_A835_518CA9141975__INCLUDED_

#include "app/pa/PAManager/src/CachedConfig.h"
#include "app/pa/PAManager/src/GenericBroadcastModel.h"
#include "app/pa/PAManager/src/PAProgressReportSubscriber.h"
#include "app/pa/PAManager/src/TrainListUpdateContainer.h"

#include "bus/pa/cachedmaps/src/CachedMapDefinitions.h"
#include "bus/pa/cachedmaps/src/CachedMaps.h"
#include "bus/trains/TrainSelectionComponent/src/ITrainSelectorGUI.h"
#include "bus/trains/TrainSelectionComponent/src/TrainSelector.h"

#include "core/data_access_interface/src/PrimitiveWrapper.h"
#include "core/synchronisation/src/NonReEntrantThreadLockable.h"
#include "core/synchronisation/src/ThreadGuard.h"

#include <string>

class ITrainBroadcastPage;

class TrainBroadcastModel
    :
    public GenericBroadcastModel,
    public TA_IRS_Bus::ITrainSelectorGUI
{
public:

    /**
     * TrainBroadcastModel (constructor)
     *
     * @param pageView the page (view) we're associated with
     *
     * @param initialStationMode initial mode which affects what type of stations
     *          are expected to be displayed in attached view
     *
     */
    TrainBroadcastModel(ITrainBroadcastPage& pageView,
                        TA_IRS_Bus::LocationCache::EStationType initialStationMode);

    /**
     * ~TrainBroadcastModel (destructor)
     *
     */
    virtual ~TrainBroadcastModel();

    /**
     * updateInboundList (implemented from ITrainSelectorGUI)
     *
     * Updates the list of inbound trains for this GUI
     *
     * @param action        the action to take (see EAction definition).
     * @param trainDetails  the new details of the train if this is an Add
     *                      or Update.
     *
     */
    virtual void updateInboundList(
        TA_IRS_Bus::ITrainSelectorGUI::EAction action,
        const TA_IRS_Bus::TrainInformationTypes::TrainDetails& trainDetails);

    /**
     * updateOutboundList (implemented from ITrainSelectorGUI)
     *
     * Updates the list of outbound trains for this GUI
     *
     * @param action        the action to take (see EAction definition).
     * @param trainDetails  the new details of the train if this is an Add
     *                      or Update.
     *
     */
    virtual void updateOutboundList(
        TA_IRS_Bus::ITrainSelectorGUI::EAction action,
        const TA_IRS_Bus::TrainInformationTypes::TrainDetails& trainDetails);

    /**
     * updateSelectionCombo (implemented from ITrainSelectorGUI)
     *
     * Called when a selection group is added or deleted. The GUI should
     * update its selection combo with the list of train selection list
     * names.
     *
     * @param trainListNames  List of names of train selection lists that
     *                        are available.
     *
     */
    virtual void updateSelectionCombo(const std::vector<std::string>& trainListNames);

    /**
     * getPageHwnd
     *
     * @pre the associated page has a valid handle (isn't destroyed / non existent)
     *
     * @return the hWnd of the page this model is attached to
     *
     */
    HWND   getPageHwnd() const;

    /**
     * enableUpdateListener
     *
     * Has this model begin listening out for updates to the train states / groups
     *     The listener must be shut down before the associated page window is destroyed
     *
     * @pre the page attached to this model currently has a valid window (internal check)
     *
     * @param bEnable true to begin listening, false to shutdown the listener
     *
     */
    void   enableUpdateListener(bool bEnable);

    /**
     * fetchUpdatedInboundTrainData
     *
     * Should be called by associated page when it receives update waiting notifications
     *
     * This is a passthrough function, see TrainListUpdateContainer::fetchUpdate
     * for possible exceptions
     *
     */
    TrainListUpdateContainer::BufferedItem fetchUpdatedInboundTrainData();

    /**
     * fetchUpdatedOutboundTrainData
     *
     * Should be called by associated page when it receives update waiting notifications
     *
     * This is a passthrough function, see TrainListUpdateContainer::fetchUpdate
     * for possible exceptions
     *
     */
    TrainListUpdateContainer::BufferedItem fetchUpdatedOutboundTrainData();

    /**
     * fetchDynamicGroupList
     *
     * @return the list of items that should be present in the dynamic groups combo
     *
     */
    std::vector<std::string>   fetchDynamicGroupList();

    /**
     * executeBroadcast
     *
     * Executes the broadcast - taking the parameters from this model
     *
     * @param trainCoverage list of train IDs this broadcast is being sent to
     *
     */
    void    executeBroadcast(const TA_Base_Bus::IPAAgentCorbaDef::TrainCoverage& trainCoverage);

    /**
     * setDuration
     *
     * Sets the starting time (as date)
     *
     * @param start time in seconds from 1970 epoch
     *
     */
    virtual void setDuration(time_t start);

    /**
     * setSelectedMessage
     *
     * @param id the primary key of the train DVA message, -1 to unset
     *
     */
    virtual void setSelectedMessage(int id);

    /**
     * setPeriod
     *
     * Sets the stopping time (as date)
     *
     * @param start time in seconds from 1970 epoch
     *
     */
    virtual void setPeriod(time_t start);

    /**
     * setCyclicalEnabled
     *
     */
    void setCyclicalEnabled(bool enabled);

    /**
     * isDuplicateDynamicGroupName
     *
     * @param selectionName the name of the dynamic group
     *
     * @return true if the specified dynamic group name already exists
     *
     * @exception PasConnectionException if the train subscriber is not connected
     *                      or cannot be contacted
     *
     */
    bool isDuplicateDynamicGroupName(const std::string& selectionName) const;

    /**
     * getSelectedTrainsFromGroup
     *
     * Retrieves the train selection set of specified group
     *
     * @param selectionName the name of the dynamic group
     *
     * @return an array of train IDs representing the selection for this group
     *
     * @exception PasConnectionException if the train subscriber is not connected
     *                      or cannot be contacted
     *
     * @exception InvalidSelectionList
     *          thrown if no selection list with that name exists.
     *
     */
    std::vector<unsigned int> getSelectedTrainsFromGroup(const std::string& selectionName) const;

    /**
     * attemptTrainSubscriberConnection
     *
     * Attempts to connect the train subscriber (instance it)
     *
     * @pre the subscriber isn't already connected, that is, !isTrainSubscriberConnected()
     *
     */
    void    attemptTrainSubscriberConnection();

    /**
     * isTrainSubscriberConnected
     *
     * @return true if the train subscriber object is connected
     *
     */
    bool    isTrainSubscriberConnected() const;

    /**
     * switchTrainListing
     *
     * Switches the train listing mode (change what appears in train listing)
     *
     * @param newMode mode to switch to
     *
     * @exception PasConnectionException if the train subscriber is not connected
     *
     * @exception TransactiveException if some sort of CORBA connection error
     *
     */
    void    switchTrainListingMode(TA_IRS_Bus::LocationCache::EStationType newMode);

    /**
     * getCurrentListingMode (simple accessor)
     *
     * @return current listing mode user has defined
     *
     */
    TA_IRS_Bus::LocationCache::EStationType getCurrentListingMode() const {
        return m_currentStationMode;
    }

    /**
     * getMessages
     *
     * @param filterKeyword the string to use to filter the result
     *
     * @return all the train DVA messages (to put in message selection list) -
     *      after filtering using keyword
     *
     */
    TA_IRS_Bus::TrainDvaMessageRecords getMessages(const std::string& filterKeyword);

    /**
     * unsetBroadcastType
     *
     * Unsets the broadcast set for this model (so no longer valid)
     *
     */
    void unsetBroadcastType();

    /**
     * setBroadcastType
     *
     * Sets the type of broadcast to be sent to PA Agent
     *
     * @param broadcastType new type of broadcast
     *
     */
    void setBroadcastType(TA_Base_Bus::IPAAgentCorbaDef::EBroadcastType broadcastType);

    /**
     * testCanObtainLock
     *
     * Obtains the m_lock, and returns true
     *
     * @pre this thread doesn't already have the lock
     *
     */
    bool testCanObtainLock();

    /**
     * getTrainSelector (simple accessor)
     */
    TA_IRS_Bus::TrainSelector* getTrainSelector() {
        return m_trainSelectionSubscriber;
    }

    /**
     * setHasOverrideOption (simple set function)
     */
    void    setHasOverrideOption(bool hasOption) {
        m_hasOverrideOption = hasOption;
    }

    /**
     * getHasOverrideOption
     *
     * @see LiveStatusModel::getHasOverrideOption base class for details
     *
     */
    virtual bool getHasOverrideOption();

    void setSelectedStaticGroup(const std::string& groupName);

    virtual void updateFallbackStatus(bool inFallback);
    virtual bool hasRadioGroupSupport();
    virtual void updateRadioGroupCombo(const std::vector<std::string>& trainListNames);
    virtual TA_IRS_Bus::TrainInformationTypes::TrainDetailsList getSelectedTrainData() const;

protected:

private:

    /**
     * executeDva
     *
     * executes a DVA play request using the current parameters
     *
     * @exception BadParameterException if there is something wrong with the parameters
     *              and the operation failed
     *
     * @exception AccessDeniedException (from PA Agent - insufficient rights)
     *
     * @exception InvalidPasConfigurationException if the library version couldn't be identified
     *
     */
    void    executeDva();

    /**
     * executeLive
     *
     * executes a live play request using the current parameters
     *
     * @exception BadParameterException if there is something wrong with the parameters
     *              and the operation failed
     *
     * @exception AccessDeniedException (from PA Agent - insufficient rights)
     *
     */
    void executeLive();

    /**
     * getSessionId
     *
     * @return the session ID of the parent GUI
     *
     */
    const std::string getSessionId() const;

    // Maintains the current station mode
    TA_IRS_Bus::LocationCache::EStationType m_currentStationMode;

    TrainListUpdateContainer m_inboundUpdateBuffer;
    TrainListUpdateContainer m_outboundUpdateBuffer;

    TA_Base_Core::NonReEntrantThreadLockable m_dynamicGroupListLock;
    std::vector<std::string> m_dynamicGroupList;

    // Page associated with this model (to receive data update notifications)
    ITrainBroadcastPage&    m_trainBroadcastPage;

    TA_IRS_Bus::TrainSelector*  m_trainSelectionSubscriber;

    time_t                                      m_period;
    time_t                                      m_duration;
    bool                                        m_cyclicalEnabled;

    TA_Base_Core::NonReEntrantThreadLockable         m_lock;

    TA_Base_Bus::IPAAgentCorbaDef::TrainCoverage     m_trainCoverage;

    TA_Base_Core::PrimitiveWrapper<int>  m_selectedMessage;
    TA_Base_Core::PrimitiveWrapper<TA_Base_Bus::IPAAgentCorbaDef::EBroadcastType>    m_broadcastType;

    // True if model is currnetly in override mode
    bool    m_hasOverrideOption;

    std::string m_selectedStaticGroup;
};

#endif // !defined(AFX_TrainBroadcastModel_H__4EBD5CE7_C4D0_43BC_A835_518CA9141975__INCLUDED_)
