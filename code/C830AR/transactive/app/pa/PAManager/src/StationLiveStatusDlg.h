#if defined(_MSC_VER)
#pragma warning(disable : 4786 4146 4018 4250 4290)
#endif // defined _MSC_VER

/**
 * The source code in this file is the property of
 * Ripple Systems and is not for redistribution
 * in any form.
 *
 * Source:   $File: //depot/C830AR_TIP/C830AR/transactive/app/pa/PAManager/src/StationLiveStatusDlg.h $
 * @author:
 * @version: $Revision: #3 $
 *
 * Last modification: $DateTime: 2025/01/20 18:40:08 $
 * Last modified by:  $Author: limin.zhu $
 *
 * <description>
 */

#if !defined(AFX_STATIONLIVEDLG_H__1B97A351_62B5_41E4_94D5_450D78023FC3__INCLUDED_)
#define AFX_STATIONLIVEDLG_H__1B97A351_62B5_41E4_94D5_450D78023FC3__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// StationLiveStatusDlg.h : header file
//

// **********  Added #8854
#include "app/pa/pamanager/src/ListCtrlSort.h"
// **********

#include "core/naming/src/NamedObject.h"
#include "core/synchronisation/src/NonReEntrantThreadLockable.h"
#include "core/synchronisation/src/ThreadGuard.h"
#include "bus/pa_4669/pa_agent/IDL/src/IPAAgentCorbaDef.h"
#include "bus/pa_4669/pa_agent/IDL/src/IPAAgentCorbaDefDescribe.h"
#include "app/pa/PAManager/src/LiveStatusPage.h"

class GenericStatusModel;
class StationLiveStatusModel;

class StationLiveStatusDlg : public LiveStatusPage
{
    // Construction

public:

    StationLiveStatusDlg(
        // song toan++
        // TD 10316
        bool isOCC,
        // ++song toan
        // TD 10316
        const std::string& broadcastId,
        CWnd* pParent = NULL);   // standard constructor

    virtual ~StationLiveStatusDlg();

    /**
     * getStationLiveStatusModel
     *
     * Returns the model attached to this dialog
     *
     * @exception bad_cast if the status model hasn't been set yet
     *
     */
    StationLiveStatusModel& getStationLiveStatusModel();

    /**
     * getStatusModelPointer
     *
     * @see LiveStatusPage::getStatusModelPointer base class definitions for more details
     *
     */
    virtual GenericStatusModel* getStatusModelPointer();

    // Overrides
    // ClassWizard generated virtual function overrides
    //{{AFX_VIRTUAL(StationLiveStatusDlg)

protected:

    virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
    //}}AFX_VIRTUAL

    // Implementation

protected:

    /**
     * getFailuresListControl
     *
     * @see LiveStatusPage::getFailuresListControl base class definition for more details
     *
     */
    // song toan++
    // TD 10316
    CListCtrlSort& getFailuresListControl()
    {
        return m_failuresLst;
    }

    // song toan++
    // TD 10316

    /**
     * getFailuresListControl
     *
     * @see LiveStatusPage::getProgressListControl base class definition for more details
     *
     */
    // song toan++
    // TD 10316
    CListCtrlSort& getProgressListControl()
    {
        return m_progressLst;
    }

    // song toan++
    // TD 10316

    /**
     * getStatusTextListBox
     *
     * @see LiveStatusPage::getStatusTextListBox base class definition for more details
     *
     */
    virtual CListBox& getStatusTextListBox()
    {
        return m_announceStatusControl;
    }

    /**
     * destroyStatusModel
     *
     * @see LiveStatusPage::destroyStatusModel base class definition for more details
     *
     */
    virtual void    destroyStatusModel();

    /**
     * getRetryAllButton (base class interface implementation)
     *
     * @see LiveStatusPage::getRetryAllButton base class definition for more details
     *
     */
    virtual CButton& getRetryAllButton()
    {
        return m_retryAllButton;
    }

    /**
     * getRetryFailedButton (base class interface implementation)
     *
     * @see LiveStatusPage::getRetryFailedButton base class definition for more details
     *
     */
    virtual CButton& getRetryFailedButton()
    {
        return m_retryFailuresButton;
    }

    /**
     * onBroadcastStateChanged (base class interface implementation)
     *
     * @see LiveStatusPage::onBroadcastStateChanged base class definition for more details
     *
     */
    void onBroadcastStateChanged(const TA_Base_Bus::IPAAgentCorbaDef::EBroadcastState& newState, bool initialisation);

    // TD16865
    std::map<unsigned int, std::string>& getTrainIdAndServiceNumber();
    // TD16865

    // Generated message map functions
    //{{AFX_MSG(StationLiveStatusDlg)
    virtual BOOL OnInitDialog();
    afx_msg void OnButtonClose();
    afx_msg void OnButtonHelp();
    afx_msg void OnButtonRetryFailures();
    afx_msg void OnButtonRetryAll();
    afx_msg void OnDestroy();
    afx_msg void OnTimer(UINT nIDEvent);
    //}}AFX_MSG
    DECLARE_MESSAGE_MAP()

private:

    // Dialog Data
    //{{AFX_DATA(StationLiveStatusDlg)
    enum { IDD = IDD_STATION_BROADCAST_LIVE_STATUS };
    CListBox	m_announceStatusControl;
    CButton	m_retryFailuresButton;
    CButton	m_retryAllButton;

    // ********** Replaced #8854
    //	CListCtrl	m_progressLst;
    //	CListCtrl	m_failuresLst;
    // **********
    CListCtrlSort	m_progressLst;
    CListCtrlSort	m_failuresLst;

    CButton m_closeButton;
    //}}AFX_DATA

    // Tracks last status string we sent to status window (to avoid duplicates)
    TA_Base_Core::PrimitiveWrapper<std::string> m_lastStatusString;

    StationLiveStatusModel* m_stationStatusModel;

    std::string m_openingBroadcastId;

    static const std::string s_pressPttPrompt;
    static const std::string s_releasePttPrompt;
    static const std::string s_broadcastComplete;

    // jeffrey++ TD10377
    bool broadcastStateUnknownflag;	// indicate the link status of agent, true = disconnected.
    // ++jeffrey TD10377
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_STATIONLIVEDLG_H__1B97A351_62B5_41E4_94D5_450D78023FC3__INCLUDED_)
