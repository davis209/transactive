/**
 * The source code in this file is the property of
 * Ripple Systems and is not for redistribution
 * in any form.
 *
 * Source: $File: //depot/C830AR_TIP/C830AR/transactive/app/pa/PAManager/src/TrainLiveStatusDlg.h $
 * @author Jade Lee
 * @version $Revision: #2 $
 * Last modification: $DateTime: 2024/12/17 16:14:02 $
 * Last modified by: $Author: limin.zhu $
 *
 * TrainLiveStatusDlg handles GUI component of live train broadcast status dialog
 *
 */

#if !defined(AFX_TRAINLIVESTATUSDLG_H__8420358B_D5E4_4D3F_B59C_AF38445C948D__INCLUDED_)
#define AFX_TRAINLIVESTATUSDLG_H__8420358B_D5E4_4D3F_B59C_AF38445C948D__INCLUDED_

// **********  Added #8854
#include "app/pa/pamanager/src/ListCtrlSort.h"
// **********

#include "app/pa/PAManager/src/LiveStatusPage.h"
#include "bus/mfc_extensions/src/flashing_button/FlashButton.h"
#include "app/pa/PAManager/src/TrainLiveStatusModel.h"

class TrainLiveStatusModel;
class TrainRadioPttMonitor;

class TrainLiveStatusDlg
    :
    public LiveStatusPage
{
    friend class TrainLiveStatusDlgTestCases;

    // Construction

public:

    // jeffrey++ TD7956
    // TD16865
    TrainLiveStatusDlg(const std::string& broadcastId, bool overrideFlag,
                       std::map<unsigned int, std::string> trainIDToServiceNumber, CWnd* pParent = NULL);
    // TD16865
    // ++jeffrey TD7956

    virtual ~TrainLiveStatusDlg();

    /**
     * OnReportUpdate (extension of base class implementation)
     *
     * Called when report update windows message received
     *
     */
    virtual LRESULT OnReportUpdate(UINT wParam, LONG lParam);

    /**
     * OnTrainPttStateChange (extension of base class implementation)
     *
     * Called when the Train mic Press to Talk state has changed
     *
     */
    virtual LRESULT OnTrainPttStateChange(UINT wParam, LONG lParam);

    /**
     * getStatusModelPointer
     *
     * See base class definitions for more details
     *
     */
    virtual GenericStatusModel* getStatusModelPointer();

    // Overrides
    // ClassWizard generated virtual function overrides
    //{{AFX_VIRTUAL(TrainLiveStatusDlg)

protected:

    virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
    //}}AFX_VIRTUAL

protected:

    // Generated message map functions
    //{{AFX_MSG(TrainLiveStatusDlg)
    afx_msg void OnButtonBeginLiveAnnouncement();
    afx_msg void OnButtonContinueLiveAnnouncement();
    afx_msg void OnButtonEndLiveAnnouncement();
    afx_msg void OnRadioOverride();
    afx_msg void OnRadioNormal();
    virtual BOOL OnInitDialog();
    afx_msg void OnTrainHelp();
    afx_msg void OnDestroy();
    afx_msg void OnTrainStatusCloseBtn();
    afx_msg void OnTrainStatusRetryFailuresBtn();
    afx_msg void OnTrainStatusRetryAllBtn();
    //}}AFX_MSG
    DECLARE_MESSAGE_MAP()

protected:

    /**
     * getFailuresListControl
     *
     * @see LiveStatusPage::getFailuresListControl base class definition for more details
     *
     */
    // song toan++
    // TD 10316
    CListCtrlSort& getFailuresListControl() {
        return m_failuresList;
    }

    // ++song toan
    // TD 10316

    /**
     * getFailuresListControl
     *
     * @see LiveStatusPage::getFailuresListControl base class definition for more details
     *
     */
    // song toan++
    // TD 10316
    CListCtrlSort& getProgressListControl() {
        return m_progressList;
    }

    // ++song toan
    // TD 10316

    /**
     * destroyStatusModel
     *
     * @see LiveStatusPage::destroyStatusModel base class definition for more details
     *
     */
    virtual void    destroyStatusModel();

    /**
     * getStatusTextListBox
     *
     * @see GenericStatusPage::getStatusTextListBox base class definition for more details
     *
     */
    virtual CListBox& getStatusTextListBox() {
        return m_announceStatusControl;
    }

    /**
     * getRetryAllButton (base class interface implementation)
     *
     * @see LiveStatusPage::getRetryAllButton for more details
     *
     */
    virtual CButton& getRetryAllButton() {
        return m_retryAllButton;
    }

    /**
     * getRetryFailedButton (base class interface implementation)
     *
     * @see LiveStatusPage::getRetryFailedButton for more details
     *
     */
    virtual CButton& getRetryFailedButton() {
        return m_retryFailuresButton;
    }

    /**
     * onBroadcastStateChanged (extension of base class implementation)
     *
     * @see LiveStatusPage::onBroadcastStateChanged for more details
     *
     */
    virtual void onBroadcastStateChanged(const TA_Base_Bus::IPAAgentCorbaDef::EBroadcastState& newState, bool initialisation);

    // TD16865
    std::map<unsigned int, std::string>& getTrainIdAndServiceNumber() {
        return m_trainIdServiceNum;
    }

    // TD16865

private:

    /**
     * setContinueButtonEnabledState
     *
     * Updates the enabled state of the continue button - special function required
     * for this to handle the colouring of the continue button properly
     * If the button is already in the designed state, nothing is done
     *
     * @param enableContinueButton true to enable, false otherwise
     *
     */
    void    setContinueButtonEnabledState(bool enableContinueButton);

    /**
     * updateContinueButtonState
     *
     * Updates the state of the continue button (initialising flashing timer)
     *
     * @param bFlashRed true to make the button flash red, false to restore to system default colour
     *
     */
    void updateContinueButtonState(bool bFlashRed);

    /**
     * processContinueRequiredStateChange
     *
     * Processes a change in the 'continue required' state of a train
     * It is assumed the calling function will not thrash this function - and
     * only call it on state changes (since it provides user feedback on each call)
     *
     */
    void processContinueRequiredStateChange(const TrainLiveStatusModel::TrainContinueState& state);

    /**
     * resetContinueRequiredButton
     *
     * Called when we want to reset the continue required button to default (disabled) state
     *
     */
    void resetContinueRequiredButton();

    /**
     * updateButtonStatesAndFeedbackArea
     *
     * Updates the enabled states of buttons on this dialog, and
     *  checks the current internal state to determine if the user requires
     *  feedback to be displayed in the status feedback area
     *
     * @param newState the current state of the broadcast
     *
     */
    void updateButtonStatesAndFeedbackArea(const TA_Base_Bus::IPAAgentCorbaDef::EBroadcastState& newState);

    // Dialog Data
    //{{AFX_DATA(TrainLiveStatusDlg)
    enum { IDD = IDD_TRAIN_BROADCAST_LIVE_STATUS };
    FlashButton	m_continueButton;
    CListBox	m_announceStatusControl;

    // ********** Replaced #8854
    //	CListCtrl	m_progressList;
    //	CListCtrl	m_failuresList;
    // **********
    CListCtrlSort	m_progressList;
    CListCtrlSort	m_failuresList;

    CButton	m_endButton;
    CButton	m_beginButton;
    CButton	m_retryFailuresButton;
    CButton	m_retryAllButton;
    CButton	m_closeButton;
    //}}AFX_DATA

    TrainLiveStatusModel*   m_trainStatusModel;

    // Tracks last status string we sent to status window (to avoid duplicates)
    TA_Base_Core::PrimitiveWrapper<std::string> m_lastStatusString;

    // Maintains array of train IDs requiring 'continue broadcast' confirmation
    std::deque<int>    m_continueRequiredQueue;

    TrainRadioPttMonitor* m_trainRadioPttMonitor;

    std::string m_openingBroadcastId;

    static const std::string s_pressBeginToStart;
    static const std::string s_pressPttToTalk;
    static const std::string s_broadcastComplete;
    static const std::string s_callRequiredOnTrain;
    static const std::string s_callConfirmedOnTrain;
    static const std::string s_operatorPressedPtt;
    static const std::string s_operatorReleasedPtt;
    // jeffrey++ TD7956
    bool m_override;
    // ++jeffrey TD7956

    // TD16865
    std::map<unsigned int, std::string> m_trainIdServiceNum;
    // TD16865
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_TRAINLIVESTATUSDLG_H__8420358B_D5E4_4D3F_B59C_AF38445C948D__INCLUDED_)
