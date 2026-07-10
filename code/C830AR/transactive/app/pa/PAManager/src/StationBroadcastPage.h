/**
 * The source code in this file is the property of
 * Ripple Systems and is not for redistribution
 * in any form.
 *
 * Source: $File: //depot/C830AR_TIP/C830AR/transactive/app/pa/PAManager/src/StationBroadcastPage.h $
 * @author Sean Liew
 * @version $Revision: #2 $
 * Last modification: $DateTime: 2024/12/17 16:14:02 $
 * Last modified by: $Author: limin.zhu $
 *
 */

#if !defined(AFX_STATIONBROADCASTPAGE_H__B3711608_2C1B_4371_9411_7B156F95E8B9__INCLUDED_)
#define AFX_STATIONBROADCASTPAGE_H__B3711608_2C1B_4371_9411_7B156F95E8B9__INCLUDED_

#if _MSC_VER > 1000
    #pragma once
#endif // _MSC_VER > 1000
// StationBroadcastPage.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// StationBroadcastPage dialog

#include "app/pa/PAManager/src/CachedConfig.h"
#include "app/pa/PAManager/src/DvaListCtrl.h"
#include "app/pa/PAManager/src/IStationBroadcastPage.h"
#include "app/pa/PAManager/src/IWindow.h"
#include "app/pa/PAManager/src/PaZoneListCtrl.h"

#include "bus/mfc_extensions/src/coloured_controls/ColourCombo.h"
#include "bus/mfc_extensions/src/focus_management/XorListSelectionController.h"

#include "core/data_access_interface/src/PrimitiveWrapper.h"

class PAManagerDlg;
class DvaMessageChangeSubscriber;
class CachedMapChangedWindowNotificationHandler;
class OperatorTerminalDvaPlaybackModel;
class StationBroadcastModel;
class HmiSelectionListener;
class RightsChangedWindowNotificationHandler;

class StationBroadcastPage
    :
    public CPropertyPage,
    public virtual IStationBroadcastPage,
    public IWindow
{
    //DECLARE_DYNCREATE(StationBroadcastPage)

    // Construction

public:

    StationBroadcastPage(DvaMessageChangeSubscriber& adhocSubscriber, PAManagerDlg* paManagerDlg = NULL);
    ~StationBroadcastPage();

    void executeBroadcast();

    void setHMIZones(const Keys& keys);
    virtual void updateNormalMessageList(TA_IRS_Bus::StationDvaMessageRecordPointers& records);
    virtual void updateEmergencyMessageList(TA_IRS_Bus::StationDvaMessageRecordPointers& records);
    virtual void updateAdhocMessageList(TA_IRS_Bus::StationDvaMessageRecordPointers& records);
    virtual void updateSelectedMessagesList(TA_IRS_Bus::StationDvaMessageRecordPointers& records);

    virtual void updateExecuteButtonEnabledState();

    /**
     * setTimeStartDtp
     *
     * Sets all the cyclical date start controls
     * so they correspond collectively to the input parameter time
     *
     * @param time the new time to update the control values with
     *
     */
    virtual void setTimeStartDtp(const CTime& time);

    /**
     * setTimeStopDtp
     *
     * Sets all the cyclical date stop controls
     * so they correspond collectively to the input parameter time
     *
     * @param time the new time to update the control values with
     *
     */
    virtual void setTimeStopDtp(const CTime& time);

    /**
     * postDialogCreate
     *
     * This function should be called once this property page has been initialized
     * and added to the tab control.  Some of the GetClientRect functions *do not* work
     * properly when put in OnInitDialog, driving the need for this function.  It should
     * be called before the page is to become visible (as view related setup is performed)
     *
     */
    void postDialogCreate();

    /**
     * displayDvaStatusDialog - Interface implementation
     *
     * @see IStationBroadcastPage::displayDvaStatusDialog for more details
     *
     */
    virtual void    displayDvaStatusDialog(const std::string& broadcastId);

    /**
     * displayLiveStatusDialog - Interface implementation
     *
     * @see IStationBroadcastPage::displayLiveStatusDialog for more details
     *
     */
    virtual void    displayLiveStatusDialog(const std::string& broadcastId);

    /// Simple accessor
    DvaMessageChangeSubscriber& getDvaMessageChangeSubscriber() {
        return m_dvaMessageChangeSubscriber;
    }

    // TD 15349

    /**
     * selectMessageNameInLists
     * This function is to iterate all the current message lists in the page
     * and select the first message that matches the given name. Return the status of selection.
     *
     * @param std::string: Message name to be selected from the current lists.
     *
     * @return bool: Status of selection. True if found and selected, otherwise False
     *
     */
    bool selectMessageNameInLists(const std::string messageName);
    // TD 15349

    // Overrides
    // ClassWizard generate virtual function overrides
    //{{AFX_VIRTUAL(StationBroadcastPage)

public:

    virtual BOOL OnSetActive();
    virtual BOOL OnKillActive();

protected:

    virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
    //}}AFX_VIRTUAL

    virtual CWnd*       getWindow() {
        return this;
    }

    virtual HWND        getHWnd() {
        return m_hWnd;
    }

    // Implementation

protected:

    // Generated message map functions
    //{{AFX_MSG(StationBroadcastPage)
    virtual BOOL OnInitDialog();
    afx_msg void OnCyclicCb();
    afx_msg void OnMultiCb();
    afx_msg void OnDvaBtn();
    afx_msg void OnLiveBtn();
    afx_msg void OnChangeNormalFilterTb();
    afx_msg void OnChangeEmergencyFilterTb();
    afx_msg void OnItemchangedSelectedMessagesLst(NMHDR* pNMHDR, LRESULT* pResult);
    afx_msg void OnClearAllBtn();
    afx_msg void OnChimeCb();
    afx_msg void OnStisCb();
    afx_msg LRESULT OnDvaMessageUpdate(UINT wParam, LONG lParam);
    afx_msg LRESULT OnProgressReportUpdate(UINT wParam, LONG lParam);
    afx_msg LRESULT OnDvaTypeChangeNotification(UINT wParam, LONG lParam);
    afx_msg LRESULT OnDvaLabelChangeNotification(UINT wParam, LONG lParam);
    afx_msg LRESULT OnZoneGroupUpdate(UINT wParam, LONG lParam);
    afx_msg LRESULT OnZoneUpdate(UINT wParam, LONG lParam);
    afx_msg LRESULT OnHMIZoneSelectionChange(UINT wParam, LONG lParam);
    afx_msg void OnEndlabeleditAdhocMessagesLst(NMHDR* pNMHDR, LRESULT* pResult);
    afx_msg void OnDatetimechangeStopDtp(NMHDR* pNMHDR, LRESULT* pResult);
    afx_msg void OnDatetimechangeStartDtp(NMHDR* pNMHDR, LRESULT* pResult);
    afx_msg void OnClearOneBtn();
    afx_msg void OnDblclkNormalMessagesLst(NMHDR* pNMHDR, LRESULT* pResult);
    afx_msg void OnDblclkEmergencyMessagesLst(NMHDR* pNMHDR, LRESULT* pResult);
    afx_msg void OnDblclkAdhocMessagesLst(NMHDR* pNMHDR, LRESULT* pResult);
    afx_msg void OnDatetimechangeStnDwellDtp(NMHDR* pNMHDR, LRESULT* pResult);
    afx_msg void OnDatetimechangeStnIntervalDtp(NMHDR* pNMHDR, LRESULT* pResult);
    afx_msg void OnMouseMove(UINT nFlags, CPoint point);
    afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
    afx_msg void OnPaint();
    afx_msg void OnBeginlabeleditStnAdhocMessagesLst(NMHDR* pNMHDR, LRESULT* pResult);
    afx_msg void OnItemchangingStnAdhocMessagesLst(NMHDR* pNMHDR, LRESULT* pResult);
    afx_msg void OnZoneGroupSaveBtn();
    afx_msg void OnZoneGroupDeleteBtn();
    afx_msg void OnItemchangedStnZoneSelectionLst(NMHDR* pNMHDR, LRESULT* pResult);
    afx_msg void OnSelchangeStnZoneGroupCombo();
    afx_msg void OnItemchangedStnNormalMessagesLst(NMHDR* pNMHDR, LRESULT* pResult);
    afx_msg void OnItemchangedStnEmergencyMessagesLst(NMHDR* pNMHDR, LRESULT* pResult);
    afx_msg void OnItemchangedStnAdhocMessagesLst(NMHDR* pNMHDR, LRESULT* pResult);
    afx_msg void OnStnMusicBtn();
    afx_msg void OnOccMusicBtn();
    afx_msg void OnStnPlayBtn();
    afx_msg void OnStnRecordBtn();
    afx_msg void OnStnStopBtn();
    afx_msg void OnDestroy();
    afx_msg void OnItemchangingStnZoneSelectionLst(NMHDR* pNMHDR, LRESULT* pResult);
    afx_msg LRESULT OnRightsChanged(UINT wParam, LONG lParam);
    afx_msg LRESULT OnStatusNameChange(UINT wParam, LONG lParam);
    afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
    //}}AFX_MSG
    DECLARE_MESSAGE_MAP()

protected:

    /**
     * processStartStopTimeChange
     *
     * Called each time there is a change in the start and stop time controls
     *
     */
    void    processStartStopTimeChange();

private:

    /**
     * Called whenever there is a Status Name change for a Station Music broadcast
     *  This will affect the state of our Station Music pushbuttons (must synchronise
     *  pushbutton state between PA Managers)
     * @param nameChange struct defining the details of the status name change
     */
    void onStationMusicStatusNameChange(const TA_Base_Bus::IPAAgentCorbaDef::CurrentBroadcastsUpdateData& nameChange);

    /**
     * onBroadcastModeChange
     *
     * This is called whenever this a change by the operator that affects the
     *      current 'broadcast mode'.  This in turn refreshes the priority information
     *      displayed to the operator in the zone list control.  The types of things
     *      that affect the current broadcast mode include what kind of message types
     *      are in the selected messages list and the type of broadcast (dva, live, etc)
     *
     */
    void onBroadcastModeChange();

    /**
     * changeAdhocListTitle
     *
     * Changes the title (first header element) of the adhoc list control
     *  to the specified string
     *
     * @param title the new title for the adhoc list
     *
     */
    void changeAdhocListTitle(std::string title);

    /**
     * updateStisCheckboxEnabled
     *
     * Using the most up to date states of other checkboxes (cyclic, multi),
     *  updates the enable state of the Stis checkbox
     *  Should be called each time the selected messages list changes (since
     *      its enabled state depends on contents of that list control)
     *
     */
    void updateStisCheckboxEnabled();

    /**
     * userInitiatedAdhocTypeChange
     *
     * Called when the user requests a change to the type of an adhoc message
     *
     * @param key the primary key of the adhoc message
     *
     * @param newState the new adhoc type value
     *
     */
    void userInitiatedAdhocTypeChange(unsigned long key, unsigned char newState);

    /**
     * userInitiatedAdhocLabelChange
     *
     * Called when the user requests a change to the label of an adhoc message
     *
     * @param key the primary key of the adhoc message
     *
     * @param newLabel the new adhoc label
     *
     */
    void userInitiatedAdhocLabelChange(unsigned long key, const std::string& newLabel);

    /**
     * onDvaPlaybackStopped
     *
     * Called immediately after a DVA Playback has completed (to update
     *  associated button states)
     *
     * @param bPlayStarted set to true if just initiated a playback,
     *          false to indicate just stopped the playback (with stop btn)
     *
     */
    void onDvaPlayStateChanged(bool bPlayStarted);

    /**
     * destroyCachedMapObservers
     *
     * Destroys all our cached map observers, must be called from
     * within OnDestroy (not destructor)
     *
     * @pre the current window handle is a valid window
     *
     */
    void destroyCachedMapObservers();

    /**
     * createCachedMapObservers
     *
     * Creates a set of (various) cached map observers for this
     * window (so we can receive updates of cached map changes)
     * Must call this in OnInitDialog (once window exists)
     *
     * @pre the current window handle is a valid window
     *
     */
    void createCachedMapObservers();

    /**
     * onZoneListOrZoneComboSelectionChange
     *
     * Called whenever there is a change in selection for the zone list
     * or zone group combo or the zone list area changes enabled state
     * (to update buttons associated with these lists)
     *
     */
    void onZoneListOrZoneComboSelectionChange();

    void addToSelectedMessagesList(const DvaListCtrl& theControl);
    void processDwell();
    void processPeriod();
    void enableCyclicArea(bool isEnabled);
    void enableMultiArea(bool isEnabled);
    void enableDvaArea(bool isEnabled);

    PAManagerDlg*               m_paManagerDlg;
    StationBroadcastModel*      m_stationBroadcastModel;

    void fillOutAdhocList();
    void fillOutNormalList();
    void fillOutEmergencyList();

    /**
     * setupDateTimeEntryControls
     *
     * Sets up the date / time entry controls wtih suitable format / default values
     *
     */
    void    setupDateTimeEntryControls();

    /**
     * getTimeStartDtp
     *
     * @return the start time corresponding to the collection of cyclical start time
     *          date / time controls
     *
     */
    CTime getTimeStartDtp();

    /**
     * getTimeStopDtp
     *
     * @return the stop time corresponding to the collection of cyclical stop time
     *          date / time controls
     *
     */
    CTime getTimeStopDtp();

    /**
     * setupPaModeButtons
     *
     * Sets up the PA mode selection buttons (called on startup)
     *
     */
    void    setupPaModeButtons();

    /**
     * setupAdhocBitmaps
     *
     * Sets up the bitmaps used in the Ad-hoc recording area (play/stop etc)
     *
     */
    void    setupAdhocBitmaps();

    /**
     * processPAModeChange
     *
     * Processes changes made to the PA mode selection buttons - called when button
     * that controls the PA mode changes state
     *
     * @param modifiedButton a pointer to the button that changed state, null to clear selection
     *
     */
    void    processPAModeChange(CButton* modifiedButton);

    enum eAdhocWindowFlags
    {
        ADHOC_AREA_ENABLE = 1,
        ADHOC_LABEL_ENABLE = 2,
        ADHOC_TYPE_ENABLE = 4
    };

    /**
     * EnableAdhocWindow
     *
     * Special function for aggregating ad hoc window enable requests, from different
     *  locations in code - only when all requests satisfy enable will the window be enabled
     *
     * @param bEnable true to enable the window, any single request with false will disable the window
     *
     * @param bitFlags flags identifying the source of the request
     *
     */
    void    EnableAdhocWindow(bool bEnable, eAdhocWindowFlags bitFlags);

    /**
     * processDvaMessageUpdates
     *
     * Processes a DVA Message update packet (updating the view as appropriate)
     *
     * @param udpatedKeys the keys of the messages that have been updated
     *
     */
    void    processDvaMessageUpdates(const DvaMessageUpdateData& updatedKeys);

    /**
     * createZoneGroupWithSelection
     *
     * Creates a new zone group at our current location with specified label
     *
     * @param label new label for the zone group
     *
     * @param existingKey a key value if we want to overwrite an existing group
     *
     * @param selectedZoneKeys the set of zone keys to create group with
     *
     * @exception DatabaseException if there is a database error.
     *
     * @exception DataException if there is an internal error interpreting the input parameters
     *
     * @exception DataConfigurationException if there is an internal error interpreting the input parameters
     *
     */
    void createZoneGroupWithSelection(
        const std::string& name,
        TA_Base_Core::PrimitiveWrapper<unsigned long> existingKey,
        std::list<unsigned long>& selectedZoneKeys);

    /**
     * getSelectedZones
     *
     * Using the current selection of the zone list, determine the keys
     *  of all currently selected zones
     *
     * @return a list of primary keys representing the selected zones
     *
     */
    Keys getSelectedZones();

    /**
     * getSelectedZoneGroupName
     *
     * Gets the currently selected zone from the zone group combo.
     *
     * @return The selected zone group name, empty string if none.
     *
     */
    std::string getSelectedZoneGroupName();

    /**
     * refreshZoneList
     *
     * Refreshes the contents of the zone listing (when zone group sel changes etc)
     *
     */
    void refreshZoneList();

    /**
     * refreshZoneComboData
     *
     * Refreshes the contents of the zone combobox (when zone groups added/removed etc)
     *
     */
    void refreshZoneComboData();

    /**
     * destroyIcons
     *
     * Destroys all memory associated with any loaded icons
     *  in the m_hIconBitmaps map
     *
     */
    void destroyIcons();

    /**
     * loadIcon (simple helper function)
     *
     * @param resourceId the resource ID of the icon to load
     *
     * @param rc the target (resize) rectangle to use when loading
     *
     * @return handle to icon (may use existing handle if
     *          the icon has already been loaded - cached locally)
     */
    HICON loadIcon(DWORD resourceId, CRect rc = CRect(0, 0, 0, 0));

    /**
     * updatePlayStopRecordButtons
     *
     * Updates the enabled state of the Play, Stop and Record
     *  buttons - however user may disable this function with freezePlayStopRecordUpdate
     *
     * @see freezePlayStopRecordUpdate
     *
     * @param isDvaAreaEnabled set to true if the DVA Area is enabled
     *          (if false, buttons are always disabled)
     *
     */
    void updatePlayStopRecordButtons(bool isDvaEnabled);

    /**
     * getSelectedDvaMessageKey
     *
     * @return the currently selected Station DVA Message primary key
     *      (it can come from any of the three DVA lists), or
     *      TA_Base_Core::DatabaseKey::getInvalidKey() if
     *      there is no current selection
     *
     */
    unsigned long getSelectedDvaMessageKey();

    /**
     * refreshBroadcastButtonEnabledState
     *
     * Called on startup, and whenever the rights associated with these buttons changes
     *  (to update their enabled state)
     *
     */
    void refreshBroadcastButtonEnabledState();

    /**
     * enterDefaultBroadcastMode
     *
     * Called whenever we want to enter any valid broadcast state (dva, live or music)
     *      this is called on startup, and whenever enabled state of buttons change that
     *      cause the current selection to become invalid
     *
     */
    void enterDefaultBroadcastMode();

    /**
     * refreshAdhocStyle
     *
     * Refreshes the style of the ad-hoc list based on current user privilege settings
     *  (call when change in user priveleges for adhoc modification rights, and on startup)
     *
     */
    void refreshAdhocListStyle();

    /**
     * refreshAdhocDisabledState
     *
     * Refreshes the enabled state of the adhco control on the dialog
     *  Should be called whenever there are changes that may affect the enabled state
     *
     */
    void refreshAdhocDisabledState();

    /**
     * updateSelectedListButtons
     *
     * Called whenever there is a change made to the selected messages
     *  list (either selection changed, or items added/removed)
     *
     * This updates the state of the buttons attached to the selected
     *  messages list
     *
     * @param isDvaAreaEnabled true if the DVA Area of this page is currently enabled
     *
     */
    void updateSelectedListButtons(bool isDvaAreaEnabled);

    /**
     * freezePlayStopRecordUpdate
     *
     * Stops the updatePlayStopRecordButtons function from doing anything
     * Put in purely as an efficiency measure when making large updates to
     *  the DVA Message lists
     *
     * @param freeze true to stop updatePlayStopRecordButtons from processing, false to
     *      restore (when restoring state, updatePlayStopRecordButtons is called)
     *
     * @pre freeze parameter is not set to the present frozen state (for safety),
     *          --ie not called with same value twice consecutively
     *
     * @see updatePlayStopRecordButtons
     *
     */
    void freezePlayStopRecordUpdate(bool freeze);

    // TD 15349

    /**
     * findAndSelectMessageNameInList
     *
     * This function is to find the message given the name in the given list.
     * If found, select the message in the list.
     *
     * @param std::string: The message's name to be selected
     * @param DvaList: The DVA list to be search for the message
     *
     * @return bool: Return True if the message is found and selected, otherwise False.
     *
     */
    bool findAndSelectMessageNameInList(const std::string messageName, DvaListCtrl& DvaList);
    // TD 15349

    typedef TA_Base_Bus::ColourCombo ColourCombo;

    // Dialog Data
    //{{AFX_DATA(StationBroadcastPage)
    enum { IDD = IDD_STATION_BROADCAST };
    CButton	m_clearOneBtn;
    CButton	m_clearAllBtn;
    CButton	m_stnMusicButton;
    CButton m_occMusicButton;
    CButton	m_deleteZoneGroupButton;
    CButton	m_saveZoneGroupButton;
    ColourCombo	m_zoneCombo;
    CDateTimeCtrl	m_cyclicalStartDate;
    CDateTimeCtrl	m_cyclicalStopDate;
    CDateTimeCtrl	m_multiBroadcastDwellTimeSeconds;
    CDateTimeCtrl	m_cyclicalInterval;

    CDateTimeCtrl	m_cyclicalStopTime;
    CDateTimeCtrl	m_cyclicalStartTime;

    CListCtrl	m_testList;
    PaZoneListCtrl	m_zoneLst;
    DvaListCtrl	m_adhocLst;
    CButton	m_stisCb;
    CButton	m_chimeCb;
    CButton	m_stopBtn;
    CEdit	m_emergencyFilterTb;
    CEdit	m_normalFilterTb;
    DvaListCtrl	m_selectedLst;
    CButton	m_recordBtn;
    CButton	m_playBtn;
    DvaListCtrl	m_normalLst;
    DvaListCtrl	m_emergencyLst;
    CButton	m_liveBtn;
    CButton	m_dvaBtn;
    CButton	m_cyclicCb;
    CButton	m_multiCb;
    //}}AFX_DATA

    DvaMessageChangeSubscriber&   m_dvaMessageChangeSubscriber;
    DWORD   m_adhocWindowDisabled;

    /// True while this property page is active
    bool    m_pageActive;

    Keys m_hmiZones;

    static const std::string    s_enterNonZeroLengthNameMessage;
    static const std::string    s_zoneGroupNameAlreadyExistsMessage;
    static const std::string    s_defaultZoneGroupName;
    static const std::string    s_enterZoneGroupNamePrompt;
    static const std::string    s_enterZoneGroupNameWindowTitle;
    static const std::string    s_zoneGroupNameSystemReservedMessage;

    std::vector<CachedMapChangedWindowNotificationHandler*> m_cachedMapObservers;

    TA_Base_Bus::XorListSelectionController  m_dvaMessageListSelController;

    std::map<DWORD, HANDLE>  m_hIconBitmaps;

    OperatorTerminalDvaPlaybackModel* m_dvaPlaybackObject;

    HmiSelectionListener*   m_hmiZoneSelChangeListener;

    bool    m_freezePlayStopRecordButtonUpdate;

    bool    m_adhocStyleChangeInProgress;       // TD #3152
    // song toan++
    // TD10316
    int    m_LocationType;
    // ++song toan
    // TD10316
    RightsChangedWindowNotificationHandler* m_rightsChangeHandler;

    CFont   m_underlineFont;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_STATIONBROADCASTPAGE_H__B3711608_2C1B_4371_9411_7B156F95E8B9__INCLUDED_)
