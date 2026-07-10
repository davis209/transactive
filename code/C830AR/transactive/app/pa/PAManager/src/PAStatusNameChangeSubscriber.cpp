/**
 * The source code in this file is the property of
 * Ripple Systems and is not for redistribution
 * in any form.
 *
 * Source: $File: //depot/C830AR_TIP/C830AR/transactive/app/pa/PAManager/src/PAStatusNameChangeSubscriber.cpp $
 * @author Jade Lee
 * @version $Revision: #3 $
 * Last modification: $DateTime: 2025/01/20 18:40:08 $
 * Last modified by: $Author: limin.zhu $
 *
 *
 * PaStatusNameChangeSubscriber bridges communications between various PA Manager components
 * and CORBA broadcasts - subscribes to any PA status name change notifications
 * Only a single model can attach to this subscriber at a time, and to ensure the client
 * model is kept up to date, must connect first, then update Status Name list (if we refresh
 * list before subscribing notifications, Status Name list may be out of sync)
 *
 */

#if defined(_MSC_VER)
#pragma warning(disable : 4786 4146 4018 4250 4290)
#endif // defined _MSC_VER

#include "app/pa/PAManager/src/StdAfx.h"
#include "core/utilities/src/DebugUtil.h"
#include "core/utilities/src/TAAssert.h"
#include "core/utilities/src/RunParams.h"
#include "core/synchronisation/src/ThreadGuard.h"
#include "core/message/src/MessagePublicationManager.h"
#include "core/message/types/PAAgentComms_MessageTypes.h"
#include "core/message/src/AuditMessageSender.h"
#include "core/message/src/MessageSubscriptionManager.h"
#include "core/message/types/PAAgentAudit_MessageTypes.h"
#include "core/message/types/ATSAgentComms_MessageTypes.h"
#include "bus/pa_4669/pa_agent/IDL/src/IPAAgentCorbaDef.h"
#include "bus/pa_4669/pa_agent/IDL/src/IPAAgentCorbaDefDescribe.h"
#include "app/pa/PAManager/src/CachedConfig.h"
#include "app/pa/PAManager/src/PaStatusNameChangeSubscriber.h"
#include "app/pa/PAManager/src/PaProgressReportSubscriber.h"
#include "core/exceptions/src/ValueNotSetException.h"
#include "app/pa/PAManager/src/PaRightsManager.h"
#include "app/pa/PAManager/src/CachedConfig.h"

#include <algorithm>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

using namespace TA_Base_Core;
using namespace TA_IRS_Bus;

PaStatusNameChangeSubscriber* PaStatusNameChangeSubscriber::m_statusNameSubscriber = 0;

PaStatusNameChangeSubscriber::PaStatusNameChangeSubscriber()
    :
    GenericMessageSubscriber<TA_Base_Bus::IPAAgentCorbaDef::CurrentBroadcastsUpdateData>
    (WM_PA_STN_STATUS_NAME_UPDATE, CachedConfig::getInstance()->getEntityLocationKey())
{
}

PaStatusNameChangeSubscriber::~PaStatusNameChangeSubscriber()
{
}

PaStatusNameChangeSubscriber& PaStatusNameChangeSubscriber::getInstance()
{
    if (0 == m_statusNameSubscriber)
    {
        m_statusNameSubscriber = new PaStatusNameChangeSubscriber();
    }

    return *m_statusNameSubscriber;
}

void PaStatusNameChangeSubscriber::removeInstance()
{
    if (0 != m_statusNameSubscriber)
    {
        delete m_statusNameSubscriber;
        m_statusNameSubscriber = 0;
    }
}

const TA_Base_Core::MessageType& PaStatusNameChangeSubscriber::getMessageType() const
{
    return TA_Base_Core::PAAgentComms::CurrentBroadcastsUpdate;
}

void PaStatusNameChangeSubscriber::updateMessageQueue(
    TA_IRS_Bus::UpdateBuffer<TA_Base_Bus::IPAAgentCorbaDef::CurrentBroadcastsUpdateData>& msgQueue,
    const std::set<HWND>& windowHwnds,
    const TA_Base_Bus::IPAAgentCorbaDef::CurrentBroadcastsUpdateData& updateData) const
{
    // Before processing the name change, check if we have the privileges required
    // to acknowledge this name change - if not, may ignore this update
    if (!PaRightsManager::getInstance().processStatusNameChangeRights(
        updateData,
        CachedConfig::getInstance()->getAgentObject()))
    {
        // Don't allow processing (discard message)
        return;
    }

    // Dispatch the notification to all register listeners
    /*
    TA_Base_Core::ThreadGuard guard(m_listenerLock);

    for (unsigned int i = 0; i < m_updateListeners.size(); i ++)
    {
        m_updateListeners[i]->onBroadcastIdChange(updateData.fromBroadcastId.in(), updateData.toBroadcastId.in());
    }
    */

    // Call base class implementation if not suppressed
    GenericMessageSubscriber<TA_Base_Bus::IPAAgentCorbaDef::CurrentBroadcastsUpdateData>::
        updateMessageQueue(msgQueue, windowHwnds, updateData);
}

/*
void PaStatusNameChangeSubscriber::registerForBroadcastIdChanges(PaProgressReportSubscriber& listener)
{
    TA_Base_Core::ThreadGuard guard(m_listenerLock);

    m_updateListeners.push_back(&listener);
}

void PaStatusNameChangeSubscriber::deregisterForBroadcastIdChanges(PaProgressReportSubscriber& listener)
{
    TA_Base_Core::ThreadGuard guard(m_listenerLock);

    std::vector<PaProgressReportSubscriber*>::iterator itr =
            std::find(m_updateListeners.begin(), m_updateListeners.end(), &listener);

    if (itr != m_updateListeners.end())
    {
        m_updateListeners.erase(itr);
    }
    else
    {
        ASSERT(false);
    }
}
*/

void PaStatusNameChangeSubscriber::onSubscriptionSuccessful()
{
    // DO NOTHING
    return;
}

void PaStatusNameChangeSubscriber::onPreProcessMessage(
    const TA_Base_Bus::IPAAgentCorbaDef::CurrentBroadcastsUpdateData& nameChangeData)
{
    // DO NOTHING
    return;
}

void PaStatusNameChangeSubscriber::onPostProcessMessage(
    const TA_Base_Bus::IPAAgentCorbaDef::CurrentBroadcastsUpdateData& updateData)
{
    // DO NOTHING
    return;
}
