/**
 * The source code in this file is the property of
 * Ripple Systems and is not for redistribution
 * in any form.
 *
 * Source:   $File: //depot/C830AR_TIP/C830AR/transactive/app/pa/PAManager/src/ChangeBroadcastId.cpp $
 * @author:  Ripple
 * @version: $Revision: #2 $
 *
 * Last modification: $DateTime: 2024/12/17 16:14:02 $
 * Last modified by:  $Author: limin.zhu $
 *
 */
#include "StdAfx.h"
#include "app/pa/PAManager/src/ChangeBroadcastId.h"
#include "app/pa/PAManager/src/DvaStatusModel.h"

ChangeBroadcastId::ChangeBroadcastId(DvaStatusModel& model, std::string newBroadcastId)
    :
    m_dvaStatusModel(model),
    m_newBroadcastId(newBroadcastId),
    m_broadcastIdChangedSuccessfully(false)
{
}

bool ChangeBroadcastId::shouldCancel()
{
    if (hasSuccessfullyChangedBroadcast())
    {
        // Broadcast ID has already been changed
        return true;
    }

    // If broadcast hasn't been changed - we attempt to delete the target broadcast
    // (to make space for the name), and then synchronize using model
    m_dvaStatusModel.deleteStatusName(m_newBroadcastId);

    if (m_dvaStatusModel.synchronizeBroadcastId(m_newBroadcastId))
    {
        // If synchronization successful, then change has been completed
        m_broadcastIdChangedSuccessfully = true;
    }

    return hasSuccessfullyChangedBroadcast();
}

bool ChangeBroadcastId::hasSuccessfullyChangedBroadcast()
{
    return m_broadcastIdChangedSuccessfully;
}