/**
 * The source code in this file is the property of
 * Ripple Systems and is not for redistribution
 * in any form.
 *
 * Source: $File: //depot/C830AR_TIP/C830AR/transactive/app/pa/PAManager/src/DvaVersionChangeSubscriber.cpp $
 * @author Jade Lee
 * @version $Revision: #2 $
 * Last modification: $DateTime: 2024/12/17 16:14:02 $
 * Last modified by: $Author: limin.zhu $
 *
 * DvaVersionChangeSubscriber subscriber to listen out for changes to
 * DVA version changes (for the DVA Versions tab)
 *
 */

#include "app/pa/PAManager/src/StdAfx.h"
#include "app/pa/PAManager/src/DvaVersionChangeSubscriber.h"
#include "app/pa/PAManager/src/CachedConfig.h"

#ifdef _DEBUG
    #define new DEBUG_NEW
    #undef THIS_FILE
    static char THIS_FILE[] = __FILE__;
#endif

using namespace TA_IRS_Bus;

DvaVersionChangeSubscriber::DvaVersionChangeSubscriber()
    :
    GenericMessageSubscriber<TA_Base_Bus::IPAAgentCorbaDef::DvaVersionRecords>
    (WM_PA_DVA_VERSION_UPDATE, CachedConfig::getInstance()->getEntityLocationKey())
{
}

const TA_Base_Core::MessageType& DvaVersionChangeSubscriber::getMessageType() const
{
    return TA_Base_Core::PAAgentComms::DvaVersionsUpdate;
}

void DvaVersionChangeSubscriber::onSubscriptionSuccessful()
{
    // DO NOTHING
    return;
}

void DvaVersionChangeSubscriber::onPreProcessMessage(const TA_Base_Bus::IPAAgentCorbaDef::DvaVersionRecords& updateData)
{
    // DO NOTHING
    return;
}

void DvaVersionChangeSubscriber::onPostProcessMessage(const TA_Base_Bus::IPAAgentCorbaDef::DvaVersionRecords& updateData)
{
    // DO NOTHING
    return;
}
