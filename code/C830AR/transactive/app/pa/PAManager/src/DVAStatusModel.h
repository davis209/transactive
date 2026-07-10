/**
 * The source code in this file is the property of
 * Ripple Systems and is not for redistribution
 * in any form.
 *
 * Source: $File: //depot/C830AR_TIP/C830AR/transactive/app/pa/PAManager/src/DVAStatusModel.h $
 * @author Jade Lee
 * @version $Revision: #2 $
 * Last modification: $DateTime: 2024/12/17 16:14:02 $
 * Last modified by: $Author: limin.zhu $
 *
 * DvaStatusModel controls the shared business logic for any DVA status dialog screens
 * - All Dva Status screens have a status name that the user may change
 *
 */

#if !defined(DvaStatusModel_4EBD5CE7_C4D0_43BC_A835_518CA9141975__INCLUDED_)
#define DvaStatusModel_4EBD5CE7_C4D0_43BC_A835_518CA9141975__INCLUDED_

#include "app/pa/PAManager/src/GenericStatusModel.h"
#include "bus/pa/cachedmaps/src/CachedMaps.h"
#include "app/pa/PAManager/src/PAProgressReportSubscriber.h"
#include "app/pa/PAManager/src/PaStatusNameChangeSubscriber.h"

class DvaStatusModel : public GenericStatusModel
{
public:

    typedef std::list<unsigned long>  MessageIdentifiers;
    enum { STATUS_DVA_NAME_MAX = 40 };

    /**
     * DvaStatusModel(constructor)
     *
     */
    DvaStatusModel(GenericStatusPage& page,
                   const std::string& broadcastId,
                   const EModelSubType& subType);

    /**
     * validateStatusNameString
     *
     * This function uses a cached set of Status Names, call updateStatusNameArrayCache
     * to update this data to the latest available information from the PA Agent
     *
     * @param str the string we are validating
     *
     * @return true if the status name is valid (under max length and not a duplicate)
     *
     */
    bool    validateStatusNameString(const std::string& str);

    /**
     * synchronizeBroadcastId
     *
     * Synchronizes the local broadcast ID of this model with the new requested broadcast ID
     *  - ensuring the change is applied to the PA Agent also
     *
     * @param newBroadcastId the new broadcast ID Name
     *
     * @return true if the name change was successful, and the local broadcast ID name
     *          has been synchronized with the requested broadcast ID name, false
     *          if the broadcast ID already exists
     *
     * @exception BroadcastIdNotFound if the current broadcast ID is out of date - this means
     *                       the local model is out of sync with the agent (local model lagging)
     *
     * @exception AccessDeniedException (from PA Agent - insufficient rights)
     *
     * @exception TransactiveException communication error (with CORBA object)
     *
     */
    bool    synchronizeBroadcastId(const std::string& newBroadcastId);

    /**
     * deleteStatusName
     *
     * This function won't return until the operation is complete (status name is first stopped then deleted)
     * If the specified Status Name doesn't exist (ie already deleted), the function returns normally
     *
     * @param str the string identifier of the status name to delete
     *
     * @exception CORBA::Exception if there is a comms error
     *
     */
    void    deleteStatusName(const std::string& str);

    /**
     * updateStatusNameArrayCache
     *
     * Updates the local cache of Status Names
     *
     */
    void    updateStatusNameArrayCache();

    /**
     * onStatusNameChange
     *
     * Called when a Status Name change occurs, may or may not be relevant to this Status Name
     *
     * @param id the identifier of the Status Name change packet
     *
     * @param hWnd the handle of the window that received the changed notification
     *
     * @return true if the local Status Name of this model has been updated (as a result
     *          of the Status Name change)
     *
     * @exception ValueNotSetException if the specified name change object doesn't exist
     *
     * @exception TransactiveException if we're not connected to the subscriber anymore
     *
     */
    bool    onStatusNameChange(DWORD id, HWND hWnd);

    /**
     * initializeStatusNameSubscriber
     *
     * Initializes StatusName subscriber service, which is to send notifications to the
     * specified window
     *
     * @exception TransactiveException if the system is out of sync, and initialization failed
     *
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
     * isConnectedToWindow
     *
     * @return true if this model has been connected to its status name change subscriber
     *
     */
    bool    isConnectedToWindow(HWND hWnd);

private:

    // Cache of Status Names in the system
    std::vector<std::string> m_cachedStatusNames;

    PaStatusNameChangeSubscriber& m_statusNameChangeSubscriber;
};

#endif // !defined(DvaStatusModel_4EBD5CE7_C4D0_43BC_A835_518CA9141975__INCLUDED_)
