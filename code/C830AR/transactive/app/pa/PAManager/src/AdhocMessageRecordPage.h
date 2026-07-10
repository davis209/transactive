/**
 * The source code in this file is the property of
 * Ripple Systems and is not for redistribution
 * in any form.
 *
 * Source: $File: //depot/C830AR_TIP/C830AR/transactive/app/pa/PAManager/src/AdhocMessageRecordPage.h $
 * @author Jade Lee
 * @version $Revision: #2 $
 * Last modification: $DateTime: 2024/12/17 16:14:02 $
 * Last modified by: $Author: limin.zhu $
 *
 * Provides a dialog interface for recording of ad-hoc
 *     messages.  While the dialog is active, it is possible
 *     for external sources to modify the type / label of the
 *     ad-hoc message being recorded.  No special actions are
 *     taken when this occurs (ignored, and changes applied when
 *     user closes dialog regardless)
 *
 */

#if !defined(AFX_ADHOCMESSAGERECORDPAGE_H__C1D937B1_69A0_4AC9_995C_28AA8183DB80__INCLUDED_)
#define AFX_ADHOCMESSAGERECORDPAGE_H__C1D937B1_69A0_4AC9_995C_28AA8183DB80__INCLUDED_

#if _MSC_VER > 1000
    #pragma once
#endif // _MSC_VER > 1000

#include "app/pa/PAManager/src/AdhocMessageRecordModel.h"
#include "bus/pa/cachedmaps/src/CachedMapDefinitions.h"
#include "app/pa/PAManager/src/IWindow.h"
#include "core/data_access_interface/src/PrimitiveWrapper.h"

class OperatorTerminalDvaPlaybackModel;
class RightsChangedWindowNotificationHandler;

class AdhocMessageRecordPage : public CDialog, public IWindow
{
    // Construction

public:

    /**
     * AdhocMessageRecordPage (constructor)
     *
     * @param source the existing adhoc message that we're recording over
     *
     * @param dvaPlaybackObject (optional) the client may provide an instance
     *          of a dva message playback entity (if null we construct one ourselves)
     *
     */
    AdhocMessageRecordPage(const TA_IRS_Bus::StationDvaMessageRecord& source,
                           OperatorTerminalDvaPlaybackModel* dvaPlaybackObject = 0,
                           CWnd* pParent = NULL);   // standard constructor

    virtual ~AdhocMessageRecordPage();

    /**
     * getMessageDescription
     *
     * @return the contents of the ad hoc message description edit control
     *
     */
    const std::string getMessageDescription();

    /**
     * getMessageType
     *
     * @return the message type descriptor for this adhoc message
     *
     */
    unsigned char getMessageType();

    /**
     * getHWnd (inteface implementation)
     *
     * @see IWindow::getHWnd for more details
     *
     */
    virtual HWND        getHWnd() {
        return m_hWnd;
    }

    /**
     * hasMessageTypeOrLabelBeenModified
     *
     * @return true if the operator has modified either the type
     *      or label fields in the dialog (any text modifications,
     *      or a type change - which may have been invoked by 'Delete' operation)
     *
     */
    bool    hasMessageTypeOrLabelBeenModified();

    // Overrides
    // ClassWizard generated virtual function overrides
    //{{AFX_VIRTUAL(AdhocMessageRecordPage)

protected:

    virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
    //}}AFX_VIRTUAL

    // Implementation

protected:

    // Generated message map functions
    //{{AFX_MSG(AdhocMessageRecordPage)
    afx_msg void OnDelete();
    afx_msg void OnHelp();
    afx_msg void OnListen();
    afx_msg void OnStartRecording();
    afx_msg LRESULT OnProgressReportUpdate(UINT wParam, LONG lParam);
    afx_msg LRESULT OnRightsChanged(UINT wParam, LONG lParam);
    afx_msg void OnDestroy();
    afx_msg void OnChangeEditDescription();
    afx_msg void OnCheckType();
    afx_msg void OnStop();
    afx_msg void OnTimer( UINT nIDEvent );

    virtual BOOL OnInitDialog();
    virtual void OnOK();
    virtual void OnCancel();
    //}}AFX_MSG
    DECLARE_MESSAGE_MAP()

private:

    /**
     * refreshDescriptionTypeEnabledStates
     *
     * Refreshes the enabled state of the description and type controls on this
     *  page, dependent on the current deleted states + rights
     *
     */
    void refreshDescriptionTypeEnabledStates();

    /**
     * isTimingMessage
     *
     * @return true if class is actively timing a message duration
     *
     * @see setMessageDurationCheckpoint
     *
     */
    bool isTimingMessage();

    /**
     * stopDvaPlaybackIfActive
     *
     * If this dialog is currently playing back a DVA message, it is stopped
     *
     */
    void stopDvaPlaybackIfActive();

    /**
     * abortRecordingIfActive
     *
     * If this dialog is currently recordinga DVA message, recording is aborted (terminated)
     *
     */
    void abortRecordingIfActive();

    /**
     * setIsDeleted
     *
     * Sets new deleted state of the contained ad-hoc message
     *
     * @param bDeleted true to indicate user just deleted this message
     *          or false to indicate user just finished recording
     *          over a deleted message (an undelete operation)
     *
     */
    void setIsDeleted(bool bDeleted);

    /**
     * updateButtonStates
     *
     * Called each time there is a state change in dialog, so that
     *  it may be updated as appropriate to reflect the changes
     *  (namely the enabled states of buttons)
     *
     */
    void updateButtonStates();

    /**
     * updateTimeField
     *
     * Passthrough function to updateTimeField using the time value
     *  from the local member variable estimated duration value
     *
     */
    void    updateTimeField();

    /**
     * updateTimeFieldWithEstimate
     *
     * Refreshes the time field using the estimated elapsed time of the active recording / playback
     *
     * @pre isTimingMessage() is true (we're estimating the time elapsed for an active message)
     *
     */
    void    updateTimeFieldWithEstimate();

    /**
     * updateTimeField
     *
     * Updates the time field (readonly edit) to display proper recorded
     *  length of this ad-hoc message (in the format "00m:00s")
     *
     * @param estimatedDuration details of estimated duration, unset
     *          state allowed (if duration unknown)
     *
     */
    void    updateTimeField(TA_Base_Core::PrimitiveWrapper<time_t>& estimatedDuration);

    /**
     * setMessageDurationCheckpoint
     *
     * Used to time duration of adhoc message (since hardware can't provide
     * this information, we must estimate it)
     *
     * @pre there has not been two consecutive calls to the function with the
     *          same input parameter value (ie must call with false after true)
     *
     * @param startTiming true to begin timing procedure, false to complete
     *          (and in doing so refresh duration field)
     *
     * @see isTimingMessage
     *
     */
    void    setMessageDurationCheckpoint(bool startTiming);

    /**
     * getEstimatedDuration
     *
     * @return the time in milliseconds since the last call to
     *      setMessageDurationCheckpoint(true)
     *
     * @pre isTiming() returns true
     *
     */
    time_t getEstimatedDurationMilli();

    /**
     * onRecordingComplete
     *
     * Called once off when recording is complete (to refresh state of dialog)
     *
     */
    void    onRecordingComplete();

    /**
     * addStringToStatusWindow
     *
     * Adds a new string for display in the status feedback window
     *
     * @param str the string to display in status window
     *
     */
    void addStringToStatusWindow(const std::string& str);

    /**
     * Cleans up memory associated with icon bitmaps
     */
    void destroyIcons();

    // Dialog Data
    //{{AFX_DATA(AdhocMessageRecordPage)
    enum { IDD = IDD_ADHOC_RECORD };
    CButton	m_stopButton;
    CButton	m_typeControl;
    CEdit	m_descriptionControl;
    CButton	m_okButton;
    CButton	m_deleteButton;
    CButton	m_listenButton;
    CButton	m_startRecordingButton;
    CListBox	m_recordStatus;
    CString	m_adhocDescription;
    BOOL	m_adhocType;
    CString	m_time;
    //}}AFX_DATA

    AdhocMessageRecordModel m_recordingModel;

    // Store references to all icons used by this dialog
    std::map<DWORD, HANDLE> m_hIcons;

    std::auto_ptr<OperatorTerminalDvaPlaybackModel>   m_dvaPlaybackObject;

    // These track duration of recording (determined via playback or recording)
    TA_Base_Core::PrimitiveWrapper<time_t>   m_startTime;
    // Estimated length of recorded message, in milliseconds
    TA_Base_Core::PrimitiveWrapper<time_t>   m_estimatedDuration;

    // Tracks whether this is a special deleted message
    bool    m_isDeleted;

    // Keep track of whether operator has made any modifications
    bool    m_hasMessageTypeOrLabelBeenModified;

    UINT    m_timeFieldTimer;

    RightsChangedWindowNotificationHandler* m_rightsChangeHandler;

    static const std::string s_pressPttPrompt;
    static const std::string s_releasePttPrompt;
    static const std::string s_recordingComplete;
    static const std::string s_recordingAborted;

    // jeffrey++ TES736
    bool isPlayBack;
    // ++jeffrey TES736
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_ADHOCMESSAGERECORDPAGE_H__C1D937B1_69A0_4AC9_995C_28AA8183DB80__INCLUDED_)
