/**
 * The source code in this file is the property of
 * Ripple Systems and is not for redistribution
 * in any form.
 *
 * Source:   $File: //depot/C830AR_TIP/C830AR/transactive/app/pa/PAManager/src/PAManagerDlg.h $
 * @author:  Ripple
 * @version: $Revision: #3 $
 *
 * Last modification: $DateTime: 2025/01/20 18:40:08 $
 * Last modified by:  $Author: limin.zhu $
 *
 */
#if !defined(AFX_PAMANAGERDLG_H__D55DB830_1342_4D9E_BA4C_C554E25FC74B__INCLUDED_)
#define AFX_PAMANAGERDLG_H__D55DB830_1342_4D9E_BA4C_C554E25FC74B__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// PAManagerDlg.h : header file
//

#include "bus/generic_gui/src/TransActiveDialog.h"
#include "core/data_access_interface/src/PrimitiveWrapper.h"
#include "bus/pa_4669/pa_agent/IDL/src/IPAAgentCorbaDef.h"
#include "bus/pa_4669/pa_agent/IDL/src/IPAAgentCorbaDefDescribe.h"
#include "app/pa/PAManager/src/IRunParamChangedListener.h"
#include "app/pa/PAManager/src/IRestartApplicationCallback.h"

class TAPropertySheet;
class DvaVersionsPage;
class StationBroadcastPage;
class TrainBroadcastPage;
class PaManagerModel;
class TrainDvaVersionsPage;
class RightsChangedWindowNotificationHandler;

// liuyu++ TD8839
class CSplashScreenEx;
// ++liuyu TD8839

/////////////////////////////////////////////////////////////////////////////
// PAManagerDlg dialog

class PAManagerDlg
    :
    public TA_Base_Bus::TransActiveDialog, public IRunParamChangedListener, public IRestartApplicationCallback
{
    // Construction

public:

    //PAManagerDlg(CWnd* pParent = NULL);   // standard constructor
    PAManagerDlg(TA_Base_Bus::IGUIAccess& controlClass);	// standard constructor

    virtual ~PAManagerDlg();

    void init();
    /**
     * enableExecuteButton
     *
     * Updates the state of the execute button, only active pages should enable it
     *
     * @param isEnable the new state of the execute button
     *
     */
    void enableExecuteButton(bool isEnabled);

    // Overrides
    // ClassWizard generated virtual function overrides
    //{{AFX_VIRTUAL(PAManagerDlg)

public:

    virtual BOOL DestroyWindow();

protected:

    virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
    virtual BOOL OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult);

    //}}AFX_VIRTUAL

    // Implementation

public:

    void ModifyWindowPos();  //zhongjie++
    /**
     * processRunParamChange (interface implementation)
     *
     * @see IRunParamChangedListener::processRunParamChange for more details
     *
     * @pre the input identifier (name) is a recognized command
     *
     */
    virtual void processRunParamChange(const std::string& name, const std::string& value);

    bool wantToShow()
    {
        return m_wantToShow;
    };

    /**
     * restartApplication (interface implementation)
     *
     * @see IRestartApplicationCallback::restartApplication for more details
     *
     */
    virtual void restartApplication(int leftPos);

    /**
     * @return true if the application is attempting to restart itself
     *
     */
    static bool isApplicationRestartInProgress();

    /**
     * @return the left pixel location defining the startup postiion for the PA Manager when
     *          it is restarted
     *
     * @pre isApplicationRestartInProgress is true
     *
     */
    static int getAppplicationRestartTargetPosition();

    //Maochun Sun++
    //TD14509
    std::string getSchematicId()
    {
        return m_schematicId;
    }

    //++Maochun Sun
    //TD14509

    // TD 15349

    /**
     * getSelectedStationMessage
     *
     * This function is to get the message name that is to be pre-selected in the station broadcast.
     * Empty string will be returned if there is no selected message.
     *
     * @return std::string: Message name for station broadcast supplied via the Runparam.
     * Empty string if no pre-selected message name
     */

    const std::string getSelectedStationMessage();

    /**
     * getSelectedTrainMessage
     *
     * This function is to get the message name that is to be pre-selected in the train broadcast.
     * Empty string will be returned if there is no selected message.
     *
     * @return std::string: Message name for train broadcast supplied via the Runparam.
     * Empty string if no pre-selected message name
     */

    const std::string getSelectedTrainMessage();
    // TD 15349

    // TD17372
    static const std::string MANAGER_LAUNCHING_BROADCAST_ID;
    // TD17372

protected:

    /**
     * OnSetWindowPosition (virtual extension)
     *
     * This method is called whenever GenericGui gets a new window position
     * from control station
     *
     */
    virtual afx_msg LRESULT OnSetWindowPosition(WPARAM wParam, LPARAM lParam);

    // Generated message map functions
    //{{AFX_MSG(PAManagerDlg)
    virtual BOOL OnInitDialog();
    afx_msg void OnPaint();
    afx_msg void OnCloseBtn();
    afx_msg void OnExecuteBtn();
    afx_msg void OnHelpBtn();
    afx_msg LRESULT OnStatusNameChange(UINT wParam, LONG lParam);
    afx_msg LRESULT OnDisplayModeChange(UINT wParam, LONG lParam);
    afx_msg LRESULT OnErrorMessage(UINT wParam, LONG lParam);
    afx_msg void OnSelchangeStatusCb();
    afx_msg void OnRestoreStatusBtn();
    afx_msg void OnKillStatusBtn();
    afx_msg void OnKillAllStatusBtn();
    afx_msg void OnDestroy();
    afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
    afx_msg void OnSize(UINT nType, int cx, int cy);
    afx_msg void OnAboutbutton();
    afx_msg void OnMove(int x, int y);
    afx_msg void OnActivateApp(BOOL bActive, DWORD hTask);
    afx_msg LRESULT OnRightsChanged(UINT wParam, LONG lParam);
    afx_msg LRESULT OnAttemptRestart(UINT wParam, LONG lParam);
    afx_msg LRESULT OnActivateSchematic(UINT wParam, LONG lParam);
    afx_msg void OnKillAllGoingDVA();
    //}}AFX_MSG
    DECLARE_MESSAGE_MAP()

    /**
     * Intercept all messages to allow provision of top level exception handler
     *
     */
    virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);

private:

    /**
     * offsetWindowPos
     *
     * Shifts the input window along by the specified offsets
     *
     * @param wnd the window to move
     *
     * @param x amount to move entire window in x direction
     *
     * @param y amount to move entire window in y direction
     *
     */
    void offsetWindowPos(CWnd& wnd, long x, long y);

    /**
     * terminateAndRemoveBroadcastInCombo
     *
     * terminates, then removes, the broadcast ID specified in the local status name
     * combo box
     *
     * @param nItem the index into the combo box defining the broadcast Id to remove
     *
     */

    void terminateAndRemoveBroadcastInCombo(int nItem);

    /**
     * Called when the user changes active tab
     *
     */
    void onTabChanged();

    /**
     * layoutMainTab
     *
     * Performs initialization operations on the main viewing tab area
     *
     */
    void    layoutMainTab();

    /**
     * updateComboBoxOnNameChange
     *
     * Updates the combo box after a name change notification received
     *
     * @param id the identifier of the name change that took place
     *
     * @return true if the name change was propogated successfully to the combo box
     *
     */
    bool    updateComboBoxOnNameChange(UINT id);

    /**
     * refreshStatusNameCombo
     *
     * Refreshes contents of the status name combo - updated dynamically as
     * subscriber notifications received (giving updated information for this list)
     *
     * @param bInitialization true if we are to initial subscription connection in model
     *                          (called once off on startup)
     */
    void    refreshStatusNameCombo(bool bInitialization);

    /**
     * refreshControlPositions
     *
     * Called each time page changes size, to ensure tab control centred in page
     *
     */
    void    refreshControlPositions();

    /**
     * getActiveTabPage
     *
     * @return active property page tab
     * (NOTE: m_mainTab->GetActivePage is NOT always sufficient)
     *
     */
    CPropertyPage* getActiveTabPage();

    /**
     * Attempts to set the schematic into an active state
     *  If fails, application must be automatically closed (for safety reasons)
     *
     */
    void activateSchematic();

    // Model for this dialog view
    PaManagerModel* m_managerModel;

    TAPropertySheet* m_mainTab;

    DvaVersionsPage* m_dvaVersionsPage;
    TrainDvaVersionsPage* m_trainDvaVersionsPage;
    StationBroadcastPage* m_stationPage;
    TrainBroadcastPage* m_trainPage;

    // Dialog Data
    //{{AFX_DATA(PAManagerDlg)
    enum { IDD = IDD_MAIN_DLG };
    CButton	m_aboutButton;
    CButton	m_helpButton;
    CButton	m_executeButton;
    CButton	m_closeButton;
    CButton	m_killAllStatusButton;
    CButton	m_killStatusButton;
    CButton	m_restoreStatusButton;
    CComboBox	m_statusNameList;
    CButton m_killAllGoingDVA;
    //}}AFX_DATA

    // Indicates whether the GUI is meant to be visible at the moment
    bool m_wantToShow;
    bool m_initialDisplay;

    static TA_Base_Core::PrimitiveWrapper<int>  s_restartTargetPosition;

    bool    m_overridingOnMove;
    RightsChangedWindowNotificationHandler* m_rightsChangeHandler;

    TA_Base_Core::PrimitiveWrapper<TA_Base_Bus::IPAAgentCorbaDef::EBroadcastType> m_currentStatusNameBroadcastType;

    // liuyu++ TD8839
    CSplashScreenEx* m_pSplash;
    // ++liuyu TD8839
    //Maochun Sun++
    //TD14509
    std::string m_schematicId;
    //++Maochun Sun
    //TD14509
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.
#endif // !defined(AFX_PAMANAGERDLG_H__D55DB830_1342_4D9E_BA4C_C554E25FC74B__INCLUDED_)
