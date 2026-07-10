/**
 * The source code in this file is the property of
 * Ripple Systems and is not for redistribution
 * in any form.
 *
 * Source: $File: //depot/C830AR_TIP/C830AR/transactive/app/pa/PAManager/src/TrainDvaStatusDlg.h $
 * @author Jade Lee
 * @version $Revision: #2 $
 * Last modification: $DateTime: 2024/12/17 16:14:02 $
 * Last modified by: $Author: limin.zhu $
 *
 * TrainDvaStatusDlg handles GUI component of DVA train broadcast status dialog
 */

#if !defined(AFX_TRAINDVASTATUSDLG_H__CE315E56_7CDB_43F0_A91C_08F75443737B__INCLUDED_)
#define AFX_TRAINDVASTATUSDLG_H__CE315E56_7CDB_43F0_A91C_08F75443737B__INCLUDED_

#include "app/pa/PAManager/src/DvaStatusPage.h"

// **********  Added #8854
#include "app/pa/pamanager/src/ListCtrlSort.h"
// **********

class TrainDvaStatusModel;
class CachedMapChangedWindowNotificationHandler;

class TrainDvaStatusDlg : public DvaStatusPage
{
public:

    /**
     * TrainDvaStatusDlg (constructor)
     */
    TrainDvaStatusDlg(const std::string& broadcastId,
                      CWnd* pParent = NULL);   // standard constructor
    // jeffrey++ TD7956
    // TD16865
    TrainDvaStatusDlg(const std::string& broadcastId, bool OverrideFlag,
                      std::map<unsigned int, std::string> trainIDToServiceNumber, CWnd* pParent = NULL);
    // TD16865
    // ++jeffrey TD7956

    virtual ~TrainDvaStatusDlg();

    // Overrides
    // ClassWizard generated virtual function overrides
    //{{AFX_VIRTUAL(TrainDvaStatusDlg)

protected:

    virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
    //}}AFX_VIRTUAL

    /**
     * getTrainDvaStatusModel
     *
     * Returns the model attached to this dialog
     *
     */
    TrainDvaStatusModel&    getTrainDvaStatusModel();

    /**
     * getStatusModelPointer
     *
     * @see DvaStatusPage::getStatusModelPointer base class definition for more details
     *
     */
    virtual GenericStatusModel* getStatusModelPointer();

    /**
     * onCurrentStatusNameDeleted
     *
     * Extends functionality of base class implementation
     *
     * @see DvaStatusPage::onCurrentStatusNameDeleted for more details
     *
     */
    virtual void    onCurrentStatusNameDeleted();

    // Implementation

protected:

    // Generated message map functions
    //{{AFX_MSG(TrainDvaStatusDlg)
    virtual BOOL OnInitDialog();
    afx_msg void OnButtonClose();
    afx_msg void OnButtonRetryFailures();
    afx_msg void OnButtonRetryAll();
    afx_msg void OnButtonAbort();
    afx_msg void OnChangeNewStatusName();
    afx_msg void OnRadioNormal();
    afx_msg void OnRadioOverride();
    afx_msg void OnDestroy();
    afx_msg LRESULT OnTrainDvaMessageUpdate(UINT wParam, LONG lParam);
    afx_msg void OnTrainHelp();
    //}}AFX_MSG
    DECLARE_MESSAGE_MAP()

protected:

    /**
     * destroyStatusModel
     *
     * @see DvaStatusPage::destroyStatusModel base class definition for more details
     *
     */
    virtual void    destroyStatusModel();

    /**
     * getFailuresListControl
     *
     * @see DvaStatusPage::getFailuresListControl base class definitions for more details
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
     * getStatusTextListBox
     *
     * @see GenericStatusPage::getStatusTextListBox base class definition for more details
     *
     */
    virtual CListBox& getStatusTextListBox() {
        throw std::exception("Not implemented");
    }

    /**
     * getFailuresListControl
     *
     * @see DvaStatusPage::getProgressListControl base class definitions for more details
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
     * getNewBroadcastIdTextBoxRef (simple accessor)
     *
     */
    virtual CEdit& getNewBroadcastIdTextBoxRef() {
        return m_newStatusNameControl;
    }

    /**
     * getCurrentBroadcastIdTextBoxRef (simple accessor)
     *
     */
    virtual CEdit& getCurrentBroadcastIdTextBoxRef() {
        return m_currentStatusNameControl;
    }

    /**
     * getRetryAllButton (base class implementation, see for more details)
     */
    virtual CButton& getRetryAllButton() {
        return m_retryAllButton;
    }

    /**
     * getRetryFailedButton (base class implementation, see for more details)
     */
    virtual CButton& getRetryFailedButton() {
        return m_retryFailuresButton;
    }

    /**
     * getAbortButton (base class implementation, see for more details)
     */
    virtual CButton& getAbortButton() {
        return m_abortButton;
    }

    // TD16865
    std::map<unsigned int, std::string>& getTrainIdAndServiceNumber() {
        return m_trainIdServiceNum;
    }

    // TD16865

private:

    /**
     * getMessageStringFromKey
     *
     * Returns a string (label) of the specified message
     * If there is some kind of error (key not recognized) and error
     * is logged and an error string is returned
     *
     * @param messageKey the key of the train message
     *
     * @return string representation of the message (the records label)
     *
     */
    std::string    getMessageStringFromKey(unsigned long messageKey);

    /**
     * fillOutBroadcastConfiguration
     *
     * Fills out the entries present in the DVA Broadcast Configuration area
     * The function queries the model (which will query the agent) to get
     * the relevant data
     *
     */
    void    fillOutBroadcastConfiguration();

    // Dialog Data
    //{{AFX_DATA(TrainDvaStatusDlg)
    enum { IDD = IDD_TRAIN_BROADCAST_DVA_STATUS };
    CButton	m_retryFailuresButton;
    CButton	m_retryAllButton;
    CButton	m_abortButton;

    // ********** Replaced #8854
    //	CListCtrl	m_progressList;
    //	CListCtrl	m_failuresList;
    // **********
    CListCtrlSort	m_progressList;
    CListCtrlSort	m_failuresList;

    CEdit	m_newStatusNameControl;
    CEdit	m_currentStatusNameControl;
    CString	m_startTime;
    CString	m_stopTime;
    CString	m_messageString;
    //}}AFX_DATA

    std::string m_openingBroadcastId;

    TrainDvaStatusModel*    m_trainStatusModel;

    CachedMapChangedWindowNotificationHandler* m_trainDvaUpdateNotificationHandler;

    unsigned long m_dvaMessageKey;
    // jeffrey++ TD7956
    bool m_override;
    // ++jeffrey TD7956

    // TD16865
    std::map<unsigned int, std::string> m_trainIdServiceNum;
    // TD16865
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_TRAINDVASTATUSDLG_H__CE315E56_7CDB_43F0_A91C_08F75443737B__INCLUDED_)
