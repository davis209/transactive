/**
 * The source code in this file is the property of
 * Ripple Systems and is not for redistribution
 * in any form.
 *
 * Source: $File: //depot/C830AR_TIP/C830AR/transactive/app/pa/PAManager/src/TrainListUpdateContainer.h $
 * @author Jade Lee
 * @version $Revision: #2 $
 * Last modification: $DateTime: 2024/12/17 16:14:02 $
 * Last modified by: $Author: limin.zhu $
 *
 * TrainListUpdateContainer manages intermediary storage (buffering) of train list
 *     updates received asychronously - to allow orderly processing by an MFC window
 *
 */
#if !defined(AFX_TrainListUpdateContainer_H__4EBD5CE7_C4D0_43BC_A835_518CA9141975__INCLUDED_)
#define AFX_TrainListUpdateContainer_H__4EBD5CE7_C4D0_43BC_A835_518CA9141975__INCLUDED_

#include "bus/trains/TrainSelectionComponent/src/ITrainSelectorGUI.h"

#include "core/exceptions/src/TransactiveException.h"
#include "core/synchronisation/src/NonReEntrantThreadLockable.h"
#include "core/synchronisation/src/ThreadGuard.h"

#include <deque>

class TrainListUpdateContainer
{
public:

    // Container returned to client code
    struct BufferedItem
    {
        TA_IRS_Bus::ITrainSelectorGUI::EAction action;
        TA_IRS_Bus::TrainInformationTypes::TrainDetails trainDetails;
    };

    /**
     * TrainListUpdateContainer (constructor)
     *
     * @param notificationType the type of notification to be send out to windows when
     *          a new update is buffered (to identify this container when processing notification)
     *
     */
    TrainListUpdateContainer(DWORD notificationType);
    virtual ~TrainListUpdateContainer();

    /**
     * bufferUpdate
     *
     * Adds a new update to the buffer, sending out notification to the specified window that
     * new information is available
     *
     * @param hWnd the window that is sent out a notification that new buffered data is available
     *
     * @param action [data to be buffered]
     *
     * @param trainDetails [data to be buffered]
     */
    void    bufferUpdate(HWND hWnd,
                         TA_IRS_Bus::ITrainSelectorGUI::EAction action,
                         const TA_IRS_Bus::TrainInformationTypes::TrainDetails& trainDetails);

    /**
     * fetchUpdate
     *
     * Retrieves (and flushes) the oldest buffered update
     *
     * @param hWnd the hWnd collecting the update - must match window that received the
     *              update waiting notification (sent out in bufferUpdate)
     *
     * @exception ValueNotSetException if there is no data in buffer
     *
     * @exception TransactiveException if the input hWnd doesn't match the expected hWnd stored in buffer
     *
     * @return the oldest item in the buffer
     *
     */
    BufferedItem    fetchUpdate(const HWND& hWnd);

private:

    // Copy constructor not used so made private
    TrainListUpdateContainer( const TrainListUpdateContainer&);
    // Assignment not used so it is made private
    TrainListUpdateContainer& operator=(const TrainListUpdateContainer&);

    struct BufferedItemEx
    {
        BufferedItem data;
        // Provided for internal checking
        HWND    associatedHwnd;
    };

    std::deque<BufferedItemEx>    m_buffer;

    TA_Base_Core::NonReEntrantThreadLockable m_lock;

    // Stores the notification that uniquely identifies this container from client code
    DWORD   m_notificationValue;
};

#endif // !defined(AFX_TrainListUpdateContainer_H__4EBD5CE7_C4D0_43BC_A835_518CA9141975__INCLUDED_)
