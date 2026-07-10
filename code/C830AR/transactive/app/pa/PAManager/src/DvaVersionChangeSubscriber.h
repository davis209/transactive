/**
 * The source code in this file is the property of
 * Ripple Systems and is not for redistribution
 * in any form.
 *
 * Source: $File: //depot/C830AR_TIP/C830AR/transactive/app/pa/PAManager/src/DvaVersionChangeSubscriber.h $
 * @author Jade Lee
 * @version $Revision: #3 $
 * Last modification: $DateTime: 2025/01/20 18:40:08 $
 * Last modified by: $Author: limin.zhu $
 *
 * DvaVersionChangeSubscriber subscriber to listen out for changes to
 * DVA version changes (for the DVA Versions tab)
 *
 */

#if !defined(DvaVersionChangeSubscriber_3F7C474E_D4B5_4966_A962_1414544E17A6__INCLUDED_)
#define DvaVersionChangeSubscriber_3F7C474E_D4B5_4966_A962_1414544E17A6__INCLUDED_

#include "bus/pa/messagesubscriber/src/GenericMessageSubscriber.h"
#include "bus/pa_4669/pa_agent/IDL/src/IPAAgentCorbaDef.h"
#include "bus/pa_4669/pa_agent/IDL/src/IPAAgentCorbaDefDescribe.h"

class DvaVersionChangeSubscriber
    :
    public TA_IRS_Bus::GenericMessageSubscriber<TA_Base_Bus::IPAAgentCorbaDef::DvaVersionRecords>
{
public:

    /**
     * DvaVersionChangeSubscriber(constructor)
     */
    DvaVersionChangeSubscriber();

    /**
     * getMessageType (interface implementation)
     *
     * @see GenericMessageSubscriber::getMessageType for more details
     *
     */
    virtual const TA_Base_Core::MessageType& getMessageType() const;

protected:

    /**
     * getNotificationData (interface implementation)
     *
     * @see GenericMessageSubscriber::getNotificationData
     *
     */
    virtual TA_Base_Bus::IPAAgentCorbaDef::DvaVersionRecords getNotificationData(
        const TA_Base_Bus::IPAAgentCorbaDef::DvaVersionRecords& packetData)
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
     * @see GenericMessageSubscriber::onPreProcessMessage for more details
     *
     */
    virtual void onPreProcessMessage(const TA_Base_Bus::IPAAgentCorbaDef::DvaVersionRecords& updateData);

    /**
     * onPostProcessMessage (interface implementation)
     *
     * @see GenericMessageSubscriber::onPostProcessMessage for more details
     *
     */
    virtual void onPostProcessMessage(const TA_Base_Bus::IPAAgentCorbaDef::DvaVersionRecords& updateData);
};

#endif // !defined(DvaVersionChangeSubscriber_3F7C474E_D4B5_4966_A962_1414544E17A6__INCLUDED_)
