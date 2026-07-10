/**
 * The source code in this file is the property of
 * Ripple Systems and is not for redistribution
 * in any form.
 *
 * Source: $File: //depot/C830AR_TIP/C830AR/transactive/app/pa/PAManager/src/DvaStatusPage.h $
 * @author Jade Lee
 * @version $Revision: #2 $
 * Last modification: $DateTime: 2024/12/17 16:14:02 $
 * Last modified by: $Author: limin.zhu $
 *
 * DvaStatusPage controls shared view functionality for DVA status pages (views)
 */

#if !defined(AFX_DvaStatusPage_B3711608_2C1B_4371_9411_7B156F95E8B9__INCLUDED_)
#define AFX_DvaStatusPage_B3711608_2C1B_4371_9411_7B156F95E8B9__INCLUDED_

#include "app/pa/PAManager/src/GenericStatusPage.h"

class DvaStatusModel;

class DvaStatusPage : public GenericStatusPage
{
public:

    /**
     * DvaStatusPage (constructor)
     *
     */

    DvaStatusPage(bool isOCC, UINT nIDTemplate, CWnd* pParentWnd = NULL);

    /**
     * onCurrentStatusNameDeleted
     *
     * Called when the status name for this broadcast is deleted by another operator
     * it means this dialog can no longer continue
     *
     */
    virtual void    onCurrentStatusNameDeleted();

    /**
     * OnInitDialog
     *
     * Calls on inititalisation of dialog window
     *
     */
    virtual BOOL    OnInitDialog();

    // Required to allow use of MFC message map macros in source file
    DECLARE_MESSAGE_MAP()

protected:

    enum EPAAgentOperation
    {
        RETRY_FAILURES,
        RETRY_ALL,
        TERMINATE_BROADCAST
    };

    /**
     * attemptOperation
     *
     * Performs the specified agent operation, the operations are centralized
     * in this function to maintain shared exception handling (because all
     * functions are handled in the same manner)
     *
     * @param op the type of operation requested
     *
     */
    void    attemptOperation(const EPAAgentOperation op);

    /**
     * tryUpdateStatusNameOnClose
     *
     * Attempts to update the status name currently entered in the page, via the
     *  finalise calls to the associated model
     *
     * @return true if the dialog should be closed, false if should be left open,
     *           it is expected that usually when status name update failed, dialog
     *           should be left open (ie will return false)
     *
     */
    bool    tryUpdateStatusNameOnClose();

    /**
     * OnChangeNewStatusName
     *
     * Should be called each time the user modifies the new status name text field
     *
     */
    virtual void    OnChangeNewStatusName();

    /**
     * OnCtlColor (windows called function)
     *
     */
    virtual HBRUSH  OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);

    /**
     * OnStatusNameChange
     *
     * Called to indicate a Status Name change notification has been processed by model
     *  and is waiting for collection (to update on window)
     *
     */
    virtual LRESULT OnStatusNameChange(UINT wParam, LONG lParam);

    /**
     * getNewBroadcastIdTextBoxRef
     *
     * @return reference to the CEdit control used to enter new broadcast ID (Status Names)
     *
     */
    virtual CEdit&  getNewBroadcastIdTextBoxRef() = 0;

    /**
     * getCurrentBroadcastIdTextBoxRef
     *
     * @return reference to the CEdit control that displays the current (read-only) status name
     *
     */
    virtual CEdit&  getCurrentBroadcastIdTextBoxRef() = 0;

    /**
     * onBroadcastStateChanged (extension of base class implementation, see for details)
     *
     */
    virtual void onBroadcastStateChanged(const TA_Base_Bus::IPAAgentCorbaDef::EBroadcastState& newState, bool initialisation);

    /**
     * getAbortButton
     *
     * @return the abort button control for this page
     *
     */
    virtual CButton& getAbortButton() = 0;

    // AFX Override
    virtual void OnDestroy();

private:

    /**
     * promptUserForStatusNameOverwrite
     *
     * Prompts the user if they want to overwrite the specified status name
     *      with a messagebox
     *
     * @param str the Status Name to be overwritten
     *
     * @return true if the user wants to overwrite the target name
     *
     */
    bool    promptUserForStatusNameOverwrite(const std::string& str);

    /**
     * refreshStatusNameCache
     *
     * Refreshes all the cached status names - which helps in identifying duplicate
     * status names (to assist user)
     *
     */
    void    refreshStatusNameCache();

    /**
     * refreshCurrentStatusName
     *
     * Called to prompt the page to reload the current status name from the model
     * and update the text in the control showing this string
     *
     */
    void    refreshCurrentStatusName();

    /**
     * isStatusNameValid
     *
     * @return true if the status name field has a valid, non duplicate name
     *
     */
    bool isStatusNameValid();

    /**
     * getDvaStatusModel
     *
     * Returns the model attached to this dialog
     *
     * @exception bad_cast if the status model hasn't been set yet
     *
     */
    DvaStatusModel&  getDvaStatusModel();

    /**
     * displayDuplicateNameOnCloseWindowWarning
     *
     * Displays a warning about the New Status name being a duplicate of an
     * existing name in system - user able to choose to ignore this (abort name change
     * and close anyway), or to hit OK and be returned to dialog..
     *
     * @return true if the user wants to ignore the warning and close anyway, false otherwise
     *
     */
    bool    displayDuplicateNameOnCloseWindowWarning();

    /**
     * setupStatusNames
     *
     * Initialization helper function
     *
     */
    void    setupStatusNames();

    const static std::string s_pleaseWaitForStatusNameUpdate;
    const static std::string s_errorCompletingStatusNameChange;

    // Maintains current valid state of the new Status Name (whether or not a duplicate..)
    bool    m_statusNameValid;

    /// Flag to state whether this dialog is operational or not
    bool    m_dialogInvalid;
};

#endif // !defined(AFX_DvaStatusPage_B3711608_2C1B_4371_9411_7B156F95E8B9__INCLUDED_)
