/**
 * The source code in this file is the property of
 * Ripple Systems and is not for redistribution
 * in any form.
 *
 * Source: $File: //depot/C830AR_TIP/C830AR/transactive/app/pa/PAManager/src/StationDvaStatusDlg.h $
 * @author Jade Lee
 * @version $Revision: #3 $
 * Last modification: $DateTime: 2025/01/20 18:40:08 $
 * Last modified by: $Author: limin.zhu $
 *
 * StationDvaStatusDlg handles GUI component of DVA station broadcast status dialog
 */

#if !defined(AFX_STATIONSTATUSDLG_H__1B97A351_62B5_41E4_94D5_450D78023FC3__INCLUDED_)
#define AFX_STATIONSTATUSDLG_H__1B97A351_62B5_41E4_94D5_450D78023FC3__INCLUDED_

// **********  Added #8854
#include "app/pa/pamanager/src/ListCtrlSort.h"
// **********

#include "core/naming/src/NamedObject.h"
#include "core/synchronisation/src/NonReEntrantThreadLockable.h"
#include "core/synchronisation/src/ThreadGuard.h"
#include "bus/pa_4669/pa_agent/IDL/src/IPAAgentCorbaDef.h"
#include "bus/pa_4669/pa_agent/IDL/src/IPAAgentCorbaDefDescribe.h"
#include "app/pa/PAManager/src/DvaListCtrl.h"
#include "app/pa/PAManager/src/DvaStatusPage.h"

class StationDvaStatusModel;
class DvaMessageChangeSubscriber;

class StationDvaStatusDlg : public DvaStatusPage
{
    // Construction

public:

    typedef std::list<unsigned long>  MessageIdentifiers;

    /**
     * StationDvaStatusDlg (constructor)
     *
     * @param broadcastId broadcast ID for this status dialog (aka Status Name)
     *
     * @param dvaMessageChangeSubscriber reference to the subscriber listening out
     *          for DVA message udpates
     *
     * @param isOcc true if at OCC
     *
     */
    StationDvaStatusDlg(
        // song toan++
        // TD 10316
        bool isOcc,
        // ++song toan
        // TD 10316
        const std::string& broadcastId,
        DvaMessageChangeSubscriber& dvaMessageChangeSubscriber,
        CWnd* pParent = NULL);   // standard constructor

    virtual ~StationDvaStatusDlg();

    /**
     * getStationDvaStatusModel
     *
     * Returns the model attached to this dialog
     *
     */
    StationDvaStatusModel& getStationDvaStatusModel();

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
     * Extends functionality of base class implementation (see that for more details)
     *
     */
    void    onCurrentStatusNameDeleted();

    // Overrides
    // ClassWizard generated virtual function overrides
    //{{AFX_VIRTUAL(StationDvaStatusDlg)

protected:

    virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
    //}}AFX_VIRTUAL

    // Implementation

protected:

    // Returns list control showing broadcast failures
    CListCtrlSort& getFailuresListControl()
    {
        return m_failuresLst;
    }

    // Returns list control showing broadcast (success/progress)
    CListCtrlSort& getProgressListControl()
    {
        return m_progressLst;
    }

    /**
     * getNewBroadcastIdTextBoxRef (simple accessor)
     *
     */
    virtual CEdit& getNewBroadcastIdTextBoxRef()
    {
        return m_newBroadcastIdTb;
    }

    /**
     * getCurrentBroadcastIdTextBoxRef (simple accessor)
     *
     */
    virtual CEdit& getCurrentBroadcastIdTextBoxRef()
    {
        return m_currentBroadcastIdTb;
    }

    // Generated message map functions
    //{{AFX_MSG(StationDvaStatusDlg)
    afx_msg LRESULT OnDvaMessageUpdate(UINT wParam, LONG lParam);
    virtual BOOL OnInitDialog();
    afx_msg void OnButtonClose();
    afx_msg void OnButtonHelp();
    afx_msg void OnButtonRetryFailures();
    afx_msg void OnButtonRetryAll();
    afx_msg void OnButtonAbort();
    afx_msg void OnChangeNewStatusName();
    afx_msg void OnDestroy();
    afx_msg void OnTimer(UINT nIDEvent);
    //}}AFX_MSG
    DECLARE_MESSAGE_MAP()

    /**
     * destroyStatusModel
     *
     * @see DvaStatusPage::destroyStatusModel base class definition for more details
     *
     */
    virtual void    destroyStatusModel();

    /**
     * getRetryAllButton (base class implementation, see for more details)
     */
    virtual CButton& getRetryAllButton()
    {
        return m_retryAllButton;
    }

    /**
     * getRetryFailedButton (base class implementation, see for more details)
     */
    virtual CButton& getRetryFailedButton()
    {
        return m_retryFailuresButton;
    }

    /**
     * getAbortButton (base class implementation, see for more details)
     */
    virtual CButton& getAbortButton()
    {
        return m_abortButton;
    }

    /**
     * getStatusTextListBox
     *
     * @see GenericStatusPage::getStatusTextListBox base class definition for more details
     *
     */
    virtual CListBox& getStatusTextListBox()
    {
        return m_statusAreaText;
    }

    // TD16865
    std::map<unsigned int, std::string>& getTrainIdAndServiceNumber();
    // TD16865

private:

    /**
     * fillOutBroadcastConfiguration
     *
     * Fills out the entries present in the DVA Broadcast Configuration area
     * The function queries the model (which will query the agent) to get
     * the relevant data, also updates selected messages list
     *
     * @return messages
     */
    void    fillOutBroadcastConfiguration();

    // Dialog Data
    //{{AFX_DATA(StationDvaStatusDlg)
    enum { IDD = IDD_STATION_BROADCAST_DVA_STATUS };
    CButton	m_retryFailuresButton;
    CButton	m_retryAllButton;
    CEdit	m_currentBroadcastIdTb;
    CEdit	m_newBroadcastIdTb;

    // ********** Replaced #8854
    //	CListCtrl	m_progressLst;
    //	CListCtrl	m_failuresLst;
    // **********
    CListCtrlSort	m_progressLst;
    CListCtrlSort	m_failuresLst;

    DvaListCtrl	m_messagesToBroadcastLst;
    CEdit	m_stopTb;
    CEdit	m_startTb;
    CEdit	m_priorityTb;
    CEdit	m_periodTb;
    CEdit	m_dwellTb;
    CEdit	m_chimeTb;
    CButton m_closeButton;
    CButton m_abortButton;
    CString	m_chimeText;
    int		m_dwellSecs;
    int		m_periodSecs;
    // jeffrey++ TES712
    //int		m_priority;
    // ++jeffrey TES712
    CString	m_startTimeText;
    CString	m_endTimeText;
    CListBox m_statusAreaText;
    //}}AFX_DATA

    StationDvaStatusModel* m_stationStatusModel;

    std::string m_openingBroadcastId;
    // jeffrey++ TD10377
    bool broadcastStateUnknownflag;	// indicate the link status of agent, true = disconnected.
    // ++jeffrey TD10377
    // Only Station Dva Status dialogs need respond to ad-hoc message changes
    DvaMessageChangeSubscriber& m_dvaMessageChangeSubscriber;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_STATIONSTATUSDLG_H__1B97A351_62B5_41E4_94D5_450D78023FC3__INCLUDED_)
