/**
 * The source code in this file is the property of
 * Ripple Systems and is not for redistribution
 * in any form.
 *
 * Source: $File: //depot/C830AR_TIP/C830AR/transactive/app/pa/PAManager/src/GenericStatusPage.h $
 * @author Jade Lee
 * @version $Revision: #3 $
 * Last modification: $DateTime: 2025/01/20 18:40:08 $
 * Last modified by: $Author: limin.zhu $
 *
 * GenericStatusPage provides some basic functionality that can be
 * shared by all Status Pages.  If the MFC based message mapping
 * (within the BEGIN_MESSAGE_MAP blocks) doesn't seem to be working
 * properly, it means someone didn't define the opening of the message
 * map BEGIN_MESSAGE_MAP(thisClass, baseClass) - if 'baseClass' skips
 * a class - then the skipped classes message map will not work
 * For example BEGIN_MESSAGE_MAP(DerivedStatusPage, CDialog) - all message
 * maps in the hierarchy will be ignored, except for DerivedStatusPage
 * (should go BEGIN_MESSAGE_MAP(DerivedStatusPage, GenericStatusPage)
 *
 */

#if !defined(AFX_GenericStatusPage_B3711608_2C1B_4371_9411_7B156F95E8B9__INCLUDED_)
#define AFX_GenericStatusPage_B3711608_2C1B_4371_9411_7B156F95E8B9__INCLUDED_

#include "bus/pa_4669/pa_agent/IDL/src/IPAAgentCorbaDef.h"
#include "bus/pa_4669/pa_agent/IDL/src/IPAAgentCorbaDefDescribe.h"
#include "core/data_access_interface/src/PrimitiveWrapper.h"
#include "app/pa/PAManager/src/IWindow.h"

// song toan++
// TD 10316
#include "app/pa/PAManager/src/ListCtrlSort.h"
// ++song toan
// TD 10316

class PAManagerDlg;
class GenericStatusModel;
class RightsChangedWindowNotificationHandler;

class GenericStatusPage : public CDialog, public IWindow
{
public:

    // song toan++
    // TD 10316
    GenericStatusPage(bool isOCC, UINT nIDTemplate, CWnd* pParentWnd = NULL);
    // ++song toan
    // TD 10316
    virtual ~GenericStatusPage();

    /**
     * updateReportLists
     *
     * updates the progress and failures report lists with information contained in 'pr'
     *
     * @param pr the structure containing the new information to display in report lists
     *
     * @exception TransactiveException if there was some error while performing the update
     *              - this means some of the items added to the lists are showing incorrect
     *              data (in the form of error codes, like UNKNOWN etc in the list)
     *
     */
    virtual void updateReportLists(const TA_Base_Bus::IPAAgentCorbaDef::BroadcastProgressUpdateData& pr);

    /**
     * getHWnd
     *
     * @return the handle associated with this page, NULL if unavailable
     *
     */
    virtual HWND    getHWnd()
    {
        return m_hWnd;
    }

    /**
     * DoDataExchange (passthrough)
     */
    virtual void    DoDataExchange(CDataExchange* pDX);

    /**
     * OnReportUpdate
     *
     * Called when report update windows message received
     *
     */
    virtual LRESULT OnReportUpdate(UINT wParam, LONG lParam);

protected:

    /**
     * OnInitDialog
     *
     * Called when window is first being initialised
     *
     */
    virtual BOOL OnInitDialog();

    // Generated message map functions
    //{{AFX_MSG(LiveStatusPage)
    afx_msg void OnDestroy();
    afx_msg LRESULT OnRightsChanged(UINT wParam, LONG lParam);
    //}}AFX_MSG

    DECLARE_MESSAGE_MAP()

    /**
     * OnCancel
     *
     * This method has been implemented to hide accidental calls when
     * the ESC key is pressed. It can be overriden with new behaviour if required.
     */
    virtual afx_msg void OnCancel() {}

    /**
     * updateRetryButtonState
     *
     * Called whenever there is a state change that may affect
     * the retry button enabled states (ie changes in broadcast state
     *  or the number of items in failed list - after progress report update)
     *
     */
    void updateRetryButtonState();

    /**
     * Called once each time the broadcast state changes
     *  (and on startup, when broadcast state invalid)
     *
     * @param newState the new state of the broadcast
     * @param initialisation set to true if this is an initialisation call (made on creation
     *          of the status window) - false if called in response to an updated progress report
     */
    virtual void onBroadcastStateChanged(const TA_Base_Bus::IPAAgentCorbaDef::EBroadcastState& newState, bool initialisation);

    /**
     * Called once each time the global broadcast state changes
     *  (and once off on startup, when state unset)
     *
     * @param state defines the new global state of the broadcast this status page is representing
     */
    virtual void onGlobalStateChanged(const TA_Base_Bus::IPAAgentCorbaDef::EGlobalState& state);

    /**
     * getFailuresListControl
     *
     * @return list control showing broadcast failures
     *
     */
    // song toan++
    // TD 10316
    virtual CListCtrlSort& getFailuresListControl() = 0;
    // ++song toan
    // TD 10316

    /**
     * getFailuresListControl
     *
     * @return list control showing broadcast (success/progress)
     *
     */
    // song toan++
    // TD 10316
    virtual CListCtrlSort& getProgressListControl() = 0;
    // ++song toan
    // TD 10316

    // TD16865
    virtual std::map<unsigned int, std::string>& getTrainIdAndServiceNumber() = 0;
    // TD16865

    /**
     * getStatusModel
     *
     * NOTE: Do not use this function in any constructors
     *      as it is unlikely to return a valid state (and throw exception)
     *
     * @return the status model attached to this page (view)
     *
     * @exception bad_cast if the status model hasn't been set yet
     *
     */
    GenericStatusModel& getStatusModel();

    /**
     * getStatusModelPointer
     *
     * @return pointer of status model attached to page, null if doesn't exist yet
     *
     */
    virtual GenericStatusModel* getStatusModelPointer() = 0;

    /**
     * destroyStatusModel
     *
     * Called to destroy the attached status model - there can be
     *  no further calls to GetStatusModel after this
     *
     */
    virtual void    destroyStatusModel() = 0;

    /**
     * getRetryAllButton
     *
     * @return the retry all button control for this page
     *
     */
    virtual CButton& getRetryAllButton() = 0;

    /**
     * getRetryFailedButton
     *
     * @return the retry failed button control for this page
     *
     */
    virtual CButton& getRetryFailedButton() = 0;

    /**
     * getCurrentBroadcastState (simple accessor)
     *
     * @return the broadcast state of the most recently processed progress report
     *
     * @exception ValueNotSetException if no progress reports have been processed
     *
     */
    TA_Base_Bus::IPAAgentCorbaDef::EBroadcastState getCurrentBroadcastState()
    {
        return m_currentBroadcastState.getValue();
    }

    /**
     * @return text suitable for display in the status area text window
     * @param state defines the current broadcast status
     */
    std::string getBroadcastStatusAreaText(const TA_Base_Bus::IPAAgentCorbaDef::EBroadcastState& state);

    /**
     * @return text suitable for display in the status are text window
     * @param state defines the current global state for the broadcast
     * @pre state is not set to GLOBAL_STATE_NO_FAILURE, this state is not supposed to display any text
     */
    std::string getGlobalStateStatusAreaText(const TA_Base_Bus::IPAAgentCorbaDef::EGlobalState& state);

    /**
     * getStatusTextListBox
     *
     * @return the edit control responsible for displaying the status text
     *
     */
    virtual CListBox& getStatusTextListBox() = 0;

    /**
     * addStringToStatusArea
     *
     * Adds a new string to the Status area edit box
     * automatically inserting newline character (if it doesn't already exist)
     *
     * @param newLine the new line to display in the status box
     *
     */
    void    addStringToStatusArea(const std::string& newLine);

private:

    // Store the last known states, to allow state change checking
    TA_Base_Core::PrimitiveWrapper<TA_Base_Bus::IPAAgentCorbaDef::EBroadcastState> m_currentBroadcastState;
    TA_Base_Core::PrimitiveWrapper<TA_Base_Bus::IPAAgentCorbaDef::EGlobalState> m_globalState;

    RightsChangedWindowNotificationHandler* m_rightsChangeHandler;
    // song toan++
    // TD 10316
    bool m_isOCC;   // true if at OCC
    // ++song toan
    // TD 10316
};

#endif // !defined(AFX_GenericStatusPage_B3711608_2C1B_4371_9411_7B156F95E8B9__INCLUDED_)
