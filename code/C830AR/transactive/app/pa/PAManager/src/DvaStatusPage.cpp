/**
 * The source code in this file is the property of
 * Ripple Systems and is not for redistribution
 * in any form.
 *
 * Source: $File: //depot/C830AR_TIP/C830AR/transactive/app/pa/PAManager/src/DvaStatusPage.cpp $
 * @author Jade Lee
 * @version $Revision: #3 $
 * Last modification: $DateTime: 2025/02/10 22:34:14 $
 * Last modified by: $Author: limin.zhu $
 *
 * DvaStatusPage controls shared view functionality for station pages (views)
 */

#include "app/pa/PAManager/src/StdAfx.h"
#include "app/pa/PAManager/src/DvaStatusPage.h"
#include "app/pa/PAManager/src/DvaStatusModel.h"
#include "app/pa/PAManager/src/DlgStatusNameWarning.h"
#include "app/pa/PAManager/src/CachedConfig.h"
#include "app/pa/PAManager/src/PaLayoutHelper.h"
#include "app/pa/PAManager/src/AsyncCancelDialog.h"
#include "app/pa/pamanager/src/AsyncCancelDialog.h"
#include "app/pa/pamanager/src/ChangeBroadcastId.h"
#include "app/pa/PAManager/src/PaRightsManager.h"
#include "bus/generic_gui/src/TransActiveMessage.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

static const DWORD STATUS_NAME_INVALID_TEXT_COLOUR = 0x000000ff;

// UW-610006
const std::string DvaStatusPage::s_pleaseWaitForStatusNameUpdate =
"This broadcast has been deleted or renamed by another operator.\nPlease wait for the system to update, then try again.";

// UW-610008
const std::string DvaStatusPage::s_errorCompletingStatusNameChange =
"There was an error contacting the PA Agent, do you want to cancel the status name change and close this dialog?";

DvaStatusPage::DvaStatusPage(bool isOCC, UINT nIDTemplate, CWnd* pParentWnd)
    :
    GenericStatusPage(isOCC, nIDTemplate, pParentWnd),
    m_statusNameValid(true),
    m_dialogInvalid(false)
{
}

DvaStatusModel& DvaStatusPage::getDvaStatusModel()
{
    return castPointerToReference<DvaStatusModel>(getStatusModelPointer());
}

BEGIN_MESSAGE_MAP(DvaStatusPage, GenericStatusPage)
    //{{AFX_MSG_MAP(DvaStatusPage)
    ON_MESSAGE(WM_PA_STN_STATUS_NAME_UPDATE, OnStatusNameChange)
    ON_WM_CTLCOLOR()
    ON_WM_DESTROY()
    //}}AFX_MSG_MAP
END_MESSAGE_MAP()

BOOL DvaStatusPage::OnInitDialog()
{
    GenericStatusPage::OnInitDialog();

    // Set up the status name cache - for rapid
    // checking for duplicates as new name entered
    setupStatusNames();

    return TRUE;
}

void DvaStatusPage::setupStatusNames()
{
    // Initialize subscription to Status Name changes, so we can track them
    // We need to do this before setting the names, to confirm all subsequent updates are tracked
    // Also because the station status model needs to be connected to the window before
    // it can validate the new broadcast ID string
    try
    {
        getDvaStatusModel().initializeStatusNameSubscriber(m_hWnd);
    }
    catch (const TA_Base_Core::TransactiveException&)
    {
        // Critically out of sync if this function fails, refuse to open dialog
        PaErrorHandler::displayError(PaErrorHandler::INIT_ERROR);

        // Close dialog
        CDialog::OnCancel();
        return;
    }

    // Initially the new and current will be equivalent, until user modifies new broadcast id
    getCurrentBroadcastIdTextBoxRef().SetWindowText(getDvaStatusModel().getBroadcastId().c_str());
    getNewBroadcastIdTextBoxRef().SetWindowText(getDvaStatusModel().getBroadcastId().c_str());

    // song toan++
    // TD730
    //
    // We remove the next statement because string length will not be limited
    // by STATUS_DVA_NAME_MAX anymore

    // Enforce limit on entry
    // getNewBroadcastIdTextBoxRef().SetLimitText(DvaStatusModel::STATUS_DVA_NAME_MAX);

    // ++song toan
    // TD730
}

bool DvaStatusPage::tryUpdateStatusNameOnClose()
{
    // If the dialog state is no longer valid (meaning no valid operations can
    // be performed), then immediately close the dialog
    if (m_dialogInvalid)
    {
        return true;
    }

    // We no longer care if the status name already exists -
    // just prompt the user if they want to overwrite existing status name
    /*
    if (!m_statusNameValid)
    {
        // User attempting to close when knowingly entered invalid name
        // Perform a quick check after refreshing Status Names
        getDvaStatusModel().updateStatusNameArrayCache();

        // This function performs actual checking (with updated Status Names)
        OnChangeNewStatusName();

        if (!m_statusNameValid)
        {
            // Still not valid - prompt user
            return displayDuplicateNameOnCloseWindowWarning();
        }
    }
    */

    // If the broadcast ID has changed, attempt to perform name change
    // If the name change fails, inform user (popup window) - and abort close operation
    CString str;
    getNewBroadcastIdTextBoxRef().GetWindowText(str);

    if (str.GetLength() == 0)
    {
        // Invalid name entered by operator, prompt them to try again - don't
        // allow closure of dialog
        PaErrorHandler::displayError(PaErrorHandler::INVALID_BROADCAST_NAME);
        return false;
    }

    bool askUserIfWantToForceClose = false;

    try
    {
        // Confirm the text in dialog, str
        // is synchronized with PA Agent (through name change request)
        if (!getDvaStatusModel().synchronizeBroadcastId(str.GetBuffer(0)))
        {
            // Refresh status name cache (just to update - so user can visibly see conflict)
            refreshStatusNameCache();

            // Status name update failed - because name already exists
            if (!promptUserForStatusNameOverwrite(str.GetBuffer(0)))
            {
                // User didn't want to overwrite the existing status name - leave dialog open
                // so they may enter another name
                return false;
            }

            // Otherwise user wants overwrite - attempt to delete the existing status name,
            // then attempt to synchronize again
            // TD #3098 -  continue trying to sychronize until successful,
            // or user aborts (this can take up to 10 seconds to succeed) - this is
            // somewhat redundant after TES #703 - broadcast has to be terminated to overwrite,
            // which means there should never be a delay..
            enum { RETRY_INTERVAL = 500 };      // Retry the overwrite every 500 ms
            // Create a model that attempts to sychronize the specified models
            // broadcast ID using the new broadcast ID
            ChangeBroadcastId overWriteModel(getDvaStatusModel(), str.GetBuffer(0));

            // Display our cancel dialog (only way to abort the synchronisation)
            AsyncCancelDialog dlg(this, RETRY_INTERVAL, &overWriteModel);
            dlg.DoModal();

            if (!overWriteModel.hasSuccessfullyChangedBroadcast())
            {
                // The cancel dialog was closed before synchronization (meaning user clicked cancel)
                // Abort the close operation in response to user clicking cancel
                return false;
            }
        }
    }
    catch (const TA_Base_Bus::IPAAgentCorbaDef::BroadcastIdNotFound&)
    {
        // This means someone else must have changed the name or deleted the
        // current broadcast ID - and we have not yet received notification of this
        // We must wait to receive notification

        //TD14164 yezh++
        //AfxMessageBox(s_pleaseWaitForStatusNameUpdate.c_str());
        TA_Base_Bus::TransActiveMessage userMsg;
        userMsg.showMsgBox(IDS_UW_610006);
        //TD14164 ++yezh

        // Don't close this dialog (return early)
        return false;
    }
    catch (const TA_Base_Core::AccessDeniedException&)
    {
        // Invalid rights to change name
        PaErrorHandler::displayError(PaErrorHandler::INSUFFICIENT_RIGHTS);
        askUserIfWantToForceClose = true;
    }
    catch (const CORBA::Exception&)
    {
        // Communication error
        PaErrorHandler::displayError(PaErrorHandler::PA_AGENT_FAIL);
        askUserIfWantToForceClose = true;
    }
    catch (const TA_Base_Core::TransactiveException&)
    {
        // Communication error
        PaErrorHandler::displayError(PaErrorHandler::PA_AGENT_FAIL);
        askUserIfWantToForceClose = true;
    }

    if (askUserIfWantToForceClose)
    {
        // Don't close dialog, unless user wants to forcibly close
        //TD14164 yezh++
        TA_Base_Bus::TransActiveMessage userMsg;

        if (IDYES == /*MessageBox(s_errorCompletingStatusNameChange.c_str(),
                                "PA Manager",
                                MB_YESNO)*/userMsg.showMsgBox(IDS_UW_610008))
            //TD14164 ++yezh
        {
            // Cancel the changes, close dialog
            return true;
        }
        else
        {
            // Leave dialog open so user may try again
            return false;
        }
    }

    // Status name changed OK, may close the dialog
    return true;
}

bool DvaStatusPage::promptUserForStatusNameOverwrite(const std::string& str)
{
    // TD14164 ++
    /*static const CString title("Status Name Overwrite");
    CString confirmationString;*/
    // ++ TD14164

    if (!PaRightsManager::getInstance().hasRightToClearBroadcast(str.c_str(), CachedConfig::getInstance()->getAgentObject()))
    {
        // Cannot attempt overwrite, insufficient priveleges to the target broadcast
        // TD14164 ++
        /*confirmationString.Format("The Status Name '%s' already exists, and the current Operator\ndoes not have the necessary rights to clear this existing Status Name.",
                                   str.c_str());
        AfxMessageBox(confirmationString);*/
        TA_Base_Bus::TransActiveMessage userMsg;
        userMsg << str;
        userMsg.showMsgBox(IDS_UE_610013);
        // ++ TD14164
        return false;
    }
    else if (PaRightsManager::getInstance().isBroadcastInProgress(str.c_str(), CachedConfig::getInstance()->getAgentObject()))
    {
        // TES #703: Cannot attempt overwrite, other broadcast is still in progress
        // TD14164 ++
        /*confirmationString.Format("The Status Name '%s' already exists and is currently in progress, it cannot be overwritten.",
                                   str.c_str());
        AfxMessageBox(confirmationString);*/
        TA_Base_Bus::TransActiveMessage userMsg;
        userMsg << str;
        userMsg.showMsgBox(IDS_UE_610014);
        // ++ TD14164
        return false;
    }
    else
    {
        // TD14164 ++
        /*confirmationString.Format("The Status Name '%s' already exists, would you like to overwrite it?",
                                    str.c_str());
        if (IDYES == MessageBox(confirmationString, title, MB_YESNO))*/
        TA_Base_Bus::TransActiveMessage userMsg;
        userMsg << str;
        UINT selectedButton = userMsg.showMsgBox(IDS_UW_610021);

        if (IDYES == selectedButton)
            // ++ TD14164
        {
            return true;
        }
        else
        {
            return false;
        }
    }
}

void DvaStatusPage::refreshStatusNameCache()
{
    // Synchronization failed, user must handle this - change Status Name
    getDvaStatusModel().updateStatusNameArrayCache();

    // Manually invoke change function, to keep dialog up to date
    // now that status name array has been updated
    OnChangeNewStatusName();
}

bool DvaStatusPage::displayDuplicateNameOnCloseWindowWarning()
{
    // Allow the user to acknowledge the error, but be given the option
    // to close window anyway if they want
    CDlgStatusNameWarning warningDialog(GetParent());

    if (IDCANCEL == warningDialog.DoModal())
    {
        // User wants to abort name changes, force closure of dialog
        return true;
    }
    else
    {
        return false;
    }
}

void DvaStatusPage::attemptOperation(const EPAAgentOperation op)
{
    // It should be noted that all operations act on the internal
    // broadcast ID - the New Status Name isn't, nor does it need to be
    // acknowledged by the PA Agent (makes no difference)

    try
    {
        switch (op)
        {
            case RETRY_FAILURES:
                getDvaStatusModel().retryFailedBroadcast();
                break;

            case RETRY_ALL:
                getDvaStatusModel().retryAllBroadcast();
                break;

            case TERMINATE_BROADCAST:
                // Terminate the broadcast currently contained by model
                getDvaStatusModel().terminateBroadcast();
                break;

            default:
                TA_ASSERT(FALSE, "Unknown OP");
                break;
        }
    }
    catch (const TA_Base_Bus::IPAAgentCorbaDef::BroadcastIdNotFound&)
    {
        // Broadcast ID not found because app must be slightly lagging / out of sync with PA Agent
        // Expect a notification to come shortly from PA Agent, bringing us up to date..
        //AfxMessageBox(s_pleaseWaitForStatusNameUpdate.c_str());
        //TD14164 yezh++
        TA_Base_Bus::TransActiveMessage userMsg;
        userMsg.showMsgBox(IDS_UW_610006);
        //TD14164 ++yezh
    }
    catch (const TA_Base_Core::AccessDeniedException&)
    {
        PaErrorHandler::displayError(PaErrorHandler::INSUFFICIENT_RIGHTS);
    }
    catch (const TA_Base_Core::ObjectResolutionException&)
    {
        // PA Agent is down, resulting in operation being aborted
        PaErrorHandler::displayError(PaErrorHandler::PA_AGENT_FAIL);
    }
    catch (const TA_Base_Core::BadParameterException&)
    {
        // Some unspecified error occurred, unable to carry out request
        PaErrorHandler::displayError(PaErrorHandler::INVALID_REQUEST_PARAMS);
    }
    catch (const TA_Base_Core::TransactiveException&)
    {
        PaErrorHandler::displayError(PaErrorHandler::UNSPECIFIED_ERROR);
    }
    catch (const CORBA::Exception&)
    {
        PaErrorHandler::displayError(PaErrorHandler::UNSPECIFIED_ERROR);
    }
}

void DvaStatusPage::OnChangeNewStatusName()
{
    bool validBefore = m_statusNameValid;

    // Update the state of status name, checked as CEdit box drawn
    m_statusNameValid = isStatusNameValid();

    if (validBefore != m_statusNameValid)
    {
        // If there is a state change of validity, must
        // invalidate entire text entry window to ensure
        // the text colour updates
        getNewBroadcastIdTextBoxRef().Invalidate();
    }
}

bool DvaStatusPage::isStatusNameValid()
{
    CString str;

    getNewBroadcastIdTextBoxRef().GetWindowText(str);

    // Check if there is a duplicate status name in the database
    bool nameValid = getDvaStatusModel().validateStatusNameString(str.GetBuffer(0));

    return nameValid;
}

HBRUSH DvaStatusPage::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
    HBRUSH hbr = GenericStatusPage::OnCtlColor(pDC, pWnd, nCtlColor);

    // Override as required
    if (getNewBroadcastIdTextBoxRef() == *pWnd)
    {
        // If about to draw the Status Name textbox, may need to change text colour
        if (!m_statusNameValid)
        {
            pDC->SetTextColor(STATUS_NAME_INVALID_TEXT_COLOUR);
        }
    }

    return hbr;
}

void DvaStatusPage::refreshCurrentStatusName()
{
    TA_ASSERT(getStatusModelPointer() != NULL, "getStatusModelPointer NULL");

    // Fetch the internal broadcast ID from the model
    std::string str = getDvaStatusModel().getBroadcastId();

    if (0 == str.length())
    {
        // This is a special case - means another user deleted this broadcast ID
        onCurrentStatusNameDeleted();
        // Return early (leave the current status name there for reference)
        return;
    }

    // If the current Status Name matches the New Status Name, then keep
    // The two edit boxes in sync (update both)
    CString currentStatusName, newStatusName;
    getCurrentBroadcastIdTextBoxRef().GetWindowText(currentStatusName);
    getNewBroadcastIdTextBoxRef().GetWindowText(newStatusName);

    // Lock names if currently matching
    bool lockNames = (currentStatusName == newStatusName);

    // That becomes our current name
    getCurrentBroadcastIdTextBoxRef().SetWindowText(str.c_str());

    // If the names are locked we update the New Status Name edit control as well
    // (otherwise it remains at whatever user currently has it at)
    if (lockNames)
    {
        // This will automatically call OnChangeStatusNameTb..
        getNewBroadcastIdTextBoxRef().SetWindowText(str.c_str());
    }
}

LRESULT DvaStatusPage::OnStatusNameChange(UINT wParam, LONG lParam)
{
    try
    {
        if (getDvaStatusModel().onStatusNameChange(wParam, m_hWnd))
        {
            // Must be careful here - if the subscriber isn't attached
            // to the window, this notification can be processed but not propogated
            // to the dialog just yet
            if (getDvaStatusModel().isConnectedToWindow(m_hWnd))
            {
                // upon change occuring, update the details in dialog (if connected)
                refreshCurrentStatusName();
            }
        }
    }

    STANDARD_PA_CATCH("DvaStatusPage::OnStatusNameChange", PaErrorHandler::UNSPECIFIED_ERROR);

    return 0;
}

void DvaStatusPage::onCurrentStatusNameDeleted()
{
    // Flag this dialog as being invalid
    m_dialogInvalid = true;

    PaErrorHandler::displayError(PaErrorHandler::THIS_DVA_BROADCAST_DELETED);
}

void DvaStatusPage::onBroadcastStateChanged(const TA_Base_Bus::IPAAgentCorbaDef::EBroadcastState& newState, bool initialisation)
{
    // Must retain default behaviour
    GenericStatusPage::onBroadcastStateChanged(newState, initialisation);

    BOOL broadcastComplete = (TA_Base_Bus::IPAAgentCorbaDef::PendingRemoval == newState);

    // Enable abort button while broadcast incomplete
    getAbortButton().EnableWindow(!broadcastComplete);
}

void DvaStatusPage::OnDestroy()
{
    getDvaStatusModel().removeStatusNameSubscriber(m_hWnd);

    GenericStatusPage::OnDestroy();
}
