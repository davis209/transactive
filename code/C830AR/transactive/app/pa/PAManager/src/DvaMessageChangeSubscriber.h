/**
 * The source code in this file is the property of
 * Ripple Systems and is not for redistribution
 * in any form.
 *
 * Source: $File: //depot/C830AR_TIP/C830AR/transactive/app/pa/PAManager/src/DvaMessageChangeSubscriber.h $
 * @author Jade Lee
 * @version $Revision: #3 $
 * Last modification: $DateTime: 2025/01/20 18:40:08 $
 * Last modified by: $Author: limin.zhu $
 *
 * DvaMessageChangeSubscriber subscriber to listen out for changes to
 * DVA message changes
 *
 */

#if !defined(DvaMessageChangeSubscriber_3F7C474E_D4B5_4966_A962_1414544E17A6__INCLUDED_)
#define DvaMessageChangeSubscriber_3F7C474E_D4B5_4966_A962_1414544E17A6__INCLUDED_

#include "bus/pa/messagesubscriber/src/GenericMessageSubscriber.h"
#include "bus/pa_4669/pa_agent/IDL/src/IPAAgentCorbaDef.h"
#include "bus/pa_4669/pa_agent/IDL/src/IPAAgentCorbaDefDescribe.h"
#include "bus/pa/cachedmaps/src/ICachedMapObserver.h"

class DvaMessageChangeSubscriber
    :
    public TA_IRS_Bus::GenericMessageSubscriber
    <TA_Base_Bus::IPAAgentCorbaDef::DvaMessagesUpdateData, DvaMessageUpdateData >,
    public TA_IRS_Bus::ICachedMapObserver
{
    friend class DvaMessageChangeSubscriberTestCases;

public:

    typedef TA_Base_Bus::IPAAgentCorbaDef::DvaMessagesUpdateData DvaMessagesUpdateData;

    /**
     * DvaMessageChangeSubscriber(constructor)
     */
    DvaMessageChangeSubscriber();

    /**
     * DvaMessageChangeSubscriber (destructor)
     */
    virtual ~DvaMessageChangeSubscriber();

    /**
     * getMessageType (interface implementation)
     *
     * @see GenericMessageSubscriber::getMessageType for more details
     *
     */
    virtual const TA_Base_Core::MessageType& getMessageType() const;

    /**
     * onCachedMapChange (interface implementation)
     *
     * @see ICachedMapObserver::onCachedMapChange for more details
     *
     */
    virtual void onCachedMapChanged(unsigned long pKey, bool changesMade);

    /**
     * registerWatchOnTypeChange
     *
     * Registers a type change that the subscriber should watch out
     *  for - and upon receipt of an update notification from this subscriber
     *      (not the cached maps) matching the input parameters, resets watch
     *
     * @pre hasTypeChangeWatchBeenCleared is true
     *
     * @param pKey the primary key of the update to watch out for
     *
     * @param type the new type to watch out for
     *
     */
    void registerWatchOnTypeChange(unsigned long pKey,
                                   unsigned char type);

    /**
     * registerWatchOnLabelChange
     *
     * Registers a label change that the subscriber should watch out
     *  for - and upon receipt of an update notification from this subscriber
     *      (not the cached maps) matching the input parameters, resets watch
     *
     * @pre hasLabelChangeWatchBeenCleared is true
     *
     * @param pKey the primary key of the update to watch out for
     *
     * @param label the new type to watch out for
     *
     */
    void registerWatchOnLabelChange(unsigned long pKey,
                                    const std::string& label);

    /**
     * clearAllChangeWatches
     *
     * Clears all registered watches
     *
     */
    void clearAllChangeWatches();

    /**
     * clearTypeChangeWatch
     *
     * Clears the type change watch, @see registerWatchOnTypeChange
     *
     */
    void clearTypeChangeWatch();

    /**
     * clearLabelChangeWatch
     *
     * Clears the label change watch, @see registerWatchOnLabelChange
     *
     */
    void clearLabelChangeWatch();

    /**
     * hasTypeChangeWatchBeenCleared
     *
     * @return true if the type change watch has been cleared
     *
     */
    bool hasTypeChangeWatchBeenCleared();

    /**
     * hasLabelChangeWatchBeenCleared
     *
     * @return true if the label change watch has been cleared
     *
     */
    bool hasLabelChangeWatchBeenCleared();

protected:

    /**
     * updateMessageQueue (interface override)
     *
     * Custom implementation required to handle insertion in message queue
     *
     * @see GenericMessageSubscriber::updateMessageQueue for more details
     *
     */
    virtual void updateMessageQueue(TA_IRS_Bus::UpdateBuffer<DvaMessageUpdateData>& msgQueue,
                                    const std::set<HWND>& windowHwnds,
                                    const DvaMessageUpdateData& updateData) const;

    /**
     * getNotificationData (interface implementation)
     *
     * @see GenericMessageSubscriber::getNotificationData
     *
     */
    virtual DvaMessageUpdateData getNotificationData(
        const DvaMessagesUpdateData& packetData);

    /**
     * onSubscriptionSuccessful (interface implementation)
     *
     * @see GenericMessageSubscriber::onSubscriptionSuccessful for more details
     *
     */
    virtual void onSubscriptionSuccessful();

    /**
     * onPreProcessMessage (interface implementation)
     *
     * @see GenericMessageSubscriber::onPreProcessMessage for more details
     *
     */
    virtual void onPreProcessMessage(const DvaMessagesUpdateData& updateData);

    /**
     * onPostProcessMessage (interface implementation)
     *
     * @see GenericMessageSubscriber::onPostProcessMessage for more details
     *
     */
    virtual void onPostProcessMessage(const DvaMessagesUpdateData& updateData);

private:

    enum EWatchType
    {
        CHANGE_TYPE,
        CHANGE_LABEL
    };

    /**
     * displayForeignUpdateWarning
     *
     * Displays a warning to the operator telling them that a DVA Message
     *  update has been received
     *
     * @param changesDetected set to true if the incoming DVA update
     *      actually caused a change in the cached maps
     *
     */
    void displayForeignUpdateWarning(bool changesDetected);

    /**
     * clearExpectedConfigUpdate
     *
     * NOTE: No locking performed: reponsibility of client function to lock
     * Clears the specified key from m_expectedConfigUpdates
     *
     * @param pKey pointer to key to clear, or null to specify remove all keys
     *
     * @param type the type associated with the key (we only remove the
     *      first matching type / key pair - if the type doesn't match, it's not removed)
     *
     */
    void clearExpectedConfigUpdate(EWatchType type, const unsigned long* pKey = 0);

    /**
     * addExpectedConfigUpdate
     *
     * NOTE: No locking performed: reponsibility of client function to lock
     * Appends an item to the expected config update queue
     *
     * @param type the type to associated with the expected config update (for management purposes)
     *
     * @param pKey the associated key
     *
     */
    void addExpectedConfigUpdate(EWatchType type, unsigned long pKey);

    /**
     * clearFirstMatchingExpectedConfigUpdateKey [locking]
     *
     * Clears the first key matching 'pKey' from the config update buffer
     *
     * @return true if there was a match for 'pKey' and an item was cleared from buffer
     *          or false if no matching item found (ie not an expected update)
     *
     */
    bool clearFirstMatchingExpectedConfigUpdateKey(unsigned long pKey);

    /**
     * testForReceiptOfFlaggedDvaChange
     *
     * Tests the input packet to see if it corresponds to a type or label change watch
     *  set earlier (if a match found, that watch is cleared)
     *
     * @param item the item which should be tested against all registered watches
     *
     * @param changesDetected set to true if the incoming DVA update
     *      actually caused a change in the cached maps
     *
     */
    void testForReceiptOfFlaggedDvaChange(const TA_Base_Bus::IPAAgentCorbaDef::StationDvaMessageRecord& item, bool changesDetected);

    /**
     * resetCachedMapDvaMessageChangeObserverRegistration
     *
     * Resets observer registration with the Station Dva Message Cached Map
     *
     * @param subscribe true to register ourself as an observer with cached map
     *              (if not done already), false to deregister (if not already)
     *
     */
    void    resetCachedMapDvaMessageChangeObserverRegistration(bool subscribe);

    // Structure defining ad hoc label change values - treat as a struct
    template <class T> class TMessageChange
    {
    public:

        unsigned long messageKey;
        T changedData;
    };

    /// Defines dva message / type association
    typedef TMessageChange<std::string>   LabelChange;
    /// Defines a dva message / label association
    typedef TMessageChange<char>          TypeChange;

    // Lock on accesses to m_receiveTypeWatch or m_receiveLabelWatch
    TA_Base_Core::NonReEntrantThreadLockable         m_receiveCheckLock;
    TA_Base_Core::PrimitiveWrapper<TypeChange>       m_receiveTypeWatch;
    TA_Base_Core::PrimitiveWrapper<LabelChange>      m_receiveLabelWatch;

    /// Array of 'Config Editor' style updates that we are expecting to receive in response
    /// to changes operator has made locally
    typedef std::pair<EWatchType, unsigned long> ExpectedConfigUpdate;
    typedef std::vector<ExpectedConfigUpdate> ExpectedConfigUpdates;
    ExpectedConfigUpdates m_expectedConfigUpdates;

    bool    m_subscribedToCachedMapChanges;
};

#endif // !defined(DvaMessageChangeSubscriber_3F7C474E_D4B5_4966_A962_1414544E17A6__INCLUDED_)
