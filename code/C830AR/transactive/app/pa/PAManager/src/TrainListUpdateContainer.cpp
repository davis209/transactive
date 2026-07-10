/**
 * The source code in this file is the property of
 * Ripple Systems and is not for redistribution
 * in any form.
 *
 * Source: $File: //depot/C830AR_TIP/C830AR/transactive/app/pa/PAManager/src/TrainListUpdateContainer.cpp $
 * @author Jade Lee
 * @version $Revision: #2 $
 * Last modification: $DateTime: 2024/12/17 16:14:02 $
 * Last modified by: $Author: limin.zhu $
 *
 * TrainListUpdateContainer manages intermediary storage (buffering) of train list
 *     updates received asychronously - to allow orderly processing by an MFC window
 *
 */

#include "app/pa/PAManager/src/StdAfx.h"

#include "app/pa/PAManager/src/TrainListUpdateContainer.h"

#include "bus/trains/TrainCommonLibrary/src/TrainInformationTypes.h"

#include "core/exceptions/src/ValueNotSetException.h"

#ifdef _DEBUG
    #define new DEBUG_NEW
    #undef THIS_FILE
    static char THIS_FILE[] = __FILE__;
#endif

TrainListUpdateContainer::TrainListUpdateContainer(DWORD notificationType)
    :
    m_notificationValue(notificationType)
{
    FUNCTION_ENTRY( "TrainListUpdateContainer" );
    FUNCTION_EXIT;
}

TrainListUpdateContainer::~TrainListUpdateContainer()
{
    FUNCTION_ENTRY( "~TrainListUpdateContainer" );
    FUNCTION_EXIT;
}

void TrainListUpdateContainer::bufferUpdate
(
    HWND hWnd,
    TA_IRS_Bus::ITrainSelectorGUI::EAction action,
    const TA_IRS_Bus::TrainInformationTypes::TrainDetails& trainDetails
)
{
    FUNCTION_ENTRY( "bufferUpdate" );

    // Cache the data, and send notification the window afterwards
    TA_Base_Core::ThreadGuard sync(m_lock);

    BufferedItemEx bi;
    bi.data.action = action;
    bi.data.trainDetails = trainDetails;
    bi.associatedHwnd = hWnd;

    // Add item to back of queue for processing
    m_buffer.push_back(bi);

    // Finally post the update to the window (after everything ready)
    ::PostMessage(hWnd, m_notificationValue, 0, 0);

    FUNCTION_EXIT;
}

TrainListUpdateContainer::BufferedItem TrainListUpdateContainer::fetchUpdate(const HWND& hWnd)
{
    FUNCTION_ENTRY( "fetchUpdate" );

    TA_Base_Core::ThreadGuard sync(m_lock);

    try
    {
        BufferedItemEx result = m_buffer.front();

        if (result.associatedHwnd != hWnd)
        {
            // Mismatched request (requesting window is not the one that
            // we posted the update to)
            TA_THROW(TA_Base_Core::TransactiveException("Mismatched fetch request"));
        }

        m_buffer.pop_front();

        FUNCTION_EXIT;
        return result.data;
    }
    catch (const TA_Base_Core::TransactiveException&)
    {
        FUNCTION_EXIT;
        throw;
    }
    catch (const std::exception&)
    {
        TA_THROW(TA_Base_Core::ValueNotSetException());
    }
}
