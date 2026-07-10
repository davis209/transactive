/**
 * The source code in this file is the property of
 * Ripple Systems and is not for redistribution
 * in any form.
 *
 * Source: $File: //depot/C830AR_TIP/C830AR/transactive/app/pa/PAManager/src/PAStatusNameChangeSubscriber.h $
 * @author Jade Lee
 * @version $Revision: #3 $
 * Last modification: $DateTime: 2025/01/20 18:40:08 $
 * Last modified by: $Author: limin.zhu $
 *
 * PaStatusNameChangeSubscriber bridges communications between various PA Manager components
 * and CORBA broadcasts - subscribes to any PA status name change notifications
 * Only a single model can attach to this subscriber at a time, and to ensure the client
 * model is kept up to date, must connect first, then update Status Name list (if we refresh
 * list before subscribing notifications, Status Name list may be out of sync)
 *
 */

#if !defined(PaStatusNameChangeSubscriber_3F7C474E_D4B5_4966_A962_1414544E17A6__INCLUDED_)
#define PaStatusNameChangeSubscriber_3F7C474E_D4B5_4966_A962_1414544E17A6__INCLUDED_

#include "core/synchronisation/src/NonReEntrantThreadLockable.h"
#include "core/threads/src/Thread.h"
#include "core/message/src/SpecialisedMessageSubscriber.h"
#include "core/message/IDL/src/CommsMessageCorbaDef.h"
#include "core/exceptions/src/PasExceptions.h"
#include "bus/pa_4669/pa_agent/IDL/src/IPAAgentCorbaDef.h"
#include "bus/pa_4669/pa_agent/IDL/src/IPAAgentCorbaDefDescribe.h"
#include "core/data_access_interface/src/PrimitiveWrapper.h"
#include "app/pa/PAManager/src/CachedConfig.h"
#include "bus/pa/messagesubscriber/src/GenericMessageSubscriber.h"

namespace TA_Base_Core
{
    class AuditMessageSender;
}

class PaProgressReportSubscriber;

class PaStatusNameChangeSubscriber
    :
    public TA_IRS_Bus::GenericMessageSubscriber<TA_Base_Bus::IPAAgentCorbaDef::CurrentBroadcastsUpdateData>
{
public:

    /**
     * @return the one and only singleton instance - needs to be a singleton to maintain
     *          common entry point for processing of status name updates
     *      This is not threadsafe
     */
    static PaStatusNameChangeSubscriber& getInstance();
    static void removeInstance();

    /**
     * PaStatusNameChangeSubscriber (constructor)
     *
     */
    PaStatusNameChangeSubscriber();

    virtual ~PaStatusNameChangeSubscriber();

    /**
     * getMessageType (interface implementation)
     *
     * @see GenericMessageSubscriber::getMessageType for more details
     *
     */
    virtual const TA_Base_Core::MessageType& getMessageType() const;

    /**
     * registerForBroadcastIdChanges
     *
     * Registers a listener class to receive all name changes asychronously
     *  Not properly broken into interface class at this stage (something to do later)
     *
     */
    //void registerForBroadcastIdChanges(PaProgressReportSubscriber& listener);

    /**
     * deregisterForBroadcastIdChanges
     *
     * DeRegisters a listener class to receive all name changes asychronously
     *  @see registerForBroadcastIdChanges
     *
     * @pre registerForBroadcastIdChanges has been called for corresponding class
     *
     */
    //void deregisterForBroadcastIdChanges(PaProgressReportSubscriber& listener);

protected:

    /**
     * updateMessageQueue (interface override)
     *
     * Custom implementation required to suppress unwanted status name changes
     *
     * @see GenericMessageSubscriber::updateMessageQueue for more details
     *
     */
    virtual void updateMessageQueue(TA_IRS_Bus::UpdateBuffer<TA_Base_Bus::IPAAgentCorbaDef::CurrentBroadcastsUpdateData>& msgQueue,
                                    const std::set<HWND>& windowHwnds,
                                    const TA_Base_Bus::IPAAgentCorbaDef::CurrentBroadcastsUpdateData& updateData) const;
    /**
     * getNotificationData (interface implementation)
     *
     * @see GenericMessageSubscriber::getNotificationData
     *
     */
    virtual TA_Base_Bus::IPAAgentCorbaDef::CurrentBroadcastsUpdateData getNotificationData(
        const TA_Base_Bus::IPAAgentCorbaDef::CurrentBroadcastsUpdateData& packetData)
    {
        return packetData;
    }

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
     * Entry point for processing of CurrentBroadcastsUpdateData objects received
     *
     * @param nameChangeData the newly received name change information
     *
     * @exception TransactiveException if there is an error (see what() for more details)
     *
     */
    void    onPreProcessMessage(const TA_Base_Bus::IPAAgentCorbaDef::CurrentBroadcastsUpdateData& nameChangeData);

    /**
     * onPostProcessMessage (interface implementation)
     *
     * @see GenericMessageSubscriber::onPostProcessMessage for more details
     *
     */
    virtual void onPostProcessMessage(const TA_Base_Bus::IPAAgentCorbaDef::CurrentBroadcastsUpdateData& updateData);

private:

    //mutable TA_Base_Core::NonReEntrantThreadLockable     m_listenerLock;
    //std::vector<PaProgressReportSubscriber*>        m_updateListeners;

    // Copy constructor not used so made private
    PaStatusNameChangeSubscriber(const PaStatusNameChangeSubscriber& thePaStatusNameChangeSubscriber);
    // Assignment not used so it is made private
    PaStatusNameChangeSubscriber& operator=(const PaStatusNameChangeSubscriber&);

    static PaStatusNameChangeSubscriber* m_statusNameSubscriber;
};

#endif // !defined(PaStatusNameChangeSubscriber_3F7C474E_D4B5_4966_A962_1414544E17A6__INCLUDED_)
