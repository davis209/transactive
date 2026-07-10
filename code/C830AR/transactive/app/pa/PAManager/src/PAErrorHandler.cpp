/**
 * The source code in this file is the property of
 * Ripple Systems and is not for redistribution
 * in any form.
 *
 * Source: $File: //depot/C830AR_TIP/C830AR/transactive/app/pa/PAManager/src/PAErrorHandler.cpp $
 * @author Jade Lee
 * @version $Revision: #3 $
 * Last modification: $DateTime: 2025/01/20 18:40:08 $
 * Last modified by: $Author: limin.zhu $
 *
 * Provides main entry display handling function for PA Manager
 * typically (default behaviour) messages are sent to message queue
 * (if possible) and displayed after the code leaves its point of origin
 * to alleviate reentrantthreadlockable issues
 *
 */

#pragma warning(disable:4284)   // For std::set warnings

#include "app/pa/PAManager/src/stdafx.h"
#include "app/pa/PAManager/src/PaErrorHandler.h"
#include "core/synchronisation/src/ReEntrantThreadLockable.h"
#include "bus/generic_gui/src/TransactiveMessage.h" // TD14164
#include "core/utilities/src/stdutil/strings.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

TA_Base_Core::ReEntrantThreadLockable PaErrorHandler::s_lock;
std::set<DWORD> PaErrorHandler::s_queuedErrors;
HWND PaErrorHandler::s_primaryWindowHwnd = 0;

TA_Base_Core::ReEntrantThreadLockable PaErrorHandler::s_modalErrorDisplayCountLock;
int PaErrorHandler::s_modalErrorDisplayCount = 0;

char* PaErrorHandler::s_errorMessages[] =
{
    "Configuration data cannot be loaded.",  // UE-010001
    "Cannot load PA Manager.",               // UE-010002
    "Database error.",                       // UE-010007
    "Invalid command line.",                 // UE-010008
    "Multiple instances of the PA Manager.", // UE-010009
    "Unspecified errors.",                   // UE-010010
    "Invalid configuration data.",           // UE-010011
    "Initialisation error.",                 // UE-010012
    "Entity name not found.",                // UE-010013
    "Absence of Control Station.",           // UE-010014
    "Failure to contact PA Agent.",          // UE-610001
    "Failure to update ad-hoc message description.",     // UE-610002
    "Failure to update ad-hoc message type.",            //UE-610003
    "Failure to broadcast train DVA messages due to inconsistent train library version numbers.", // UE-610004
    "Invalid request parameters.",           // UE-610005
    "Broadcast not found.",                  // UE-610006
    "Broadcast must be terminated first.",   // UE-610007
    "Failure to contact PAS FEPC hardware.", // UE-610008
    "Status name is in use by another broadcast.",       // UE-610009
    "Invalid train update received.",                    // UE-N/A
    "Invalid DVA message update received.",              // UE-N/A
    "Priority Scheme update error.",                     // UE-N/A
    "Another operator has deleted this DVA broadcast.",  // UW-610007
    "Warning: One or more of the messages for this broadcast have been deleted.",    // UW-610008
    "Operator does not have the necessary rights to execute this request.",          // UE-610010
    "This is a deleted Ad-hoc message, and needs to have a recorded message before the name can be modified.",   // UW-610011
    "DVA message update received from external source.",                         // UI-610003
    "PA Manager may only interface with one schematic at a time.",               // UE-N/A
    "PA Manager cannot be run for a foreign station (can only function with schematics matching the terminals physical location).",    // UE-N/A
    "Please enter a valid name for the broadcast.",                              // UE-N/A
    "Cannot minimize window while an error message is active.",
    "The maximum number of broadcasts has been reached, please clear some broadcasts and try again.",    // UW-610012
    "You cannot run the PA Manager for the Overall schematic from a Station.",                           // UE-610012
    "Unable to communicate with schematic PA Manager was launched from, PA Manager will now close.",     // UE-610011
    "Cannot run PA Manager for the specified schematic, first close all PA Manager child windows, then try again.",  // UW-610013
    "Timeout waiting for Train reply.",                          // UE-N/A
    "Failure to contact Train Agent.",                           // UE-N/A
    "CORBA communications failure.",                             // UE-N/A
    "Failed to contact named object (object resolution error).", // UE-N/A
    "Train dynamic group update received from external source.\nThe current train selection has been updated.",  // UI-610004
    "PA Zone update received from external source.\nThe current zone selection has been updated.",               // UI-610005
    "PA Zone Group update received from external source.\nThe current zone selection has been updated.",         // UI-610006

    "Please enter a valid description for the Ad-hoc message.",  // UE-N/A
    "New DVA library Received",

    // Do not change this last message
    "Invalid error specification - error ID out of range.",   // Internal PAErrorHandler error
};

void PaErrorHandler::displayModalError(EErrorId errorId)
{
    // Check for out of range
    if (errorId >= ERROR_MAX)
    {
        TA_ASSERT(FALSE, "Invalid error display condition");
        errorId = ERROR_MAX_ID_SPECIFIED;
    }

    try
    {
        char* error = s_errorMessages[errorId];

        // Removed following code for now so that all messages are displayed whether
        // hidden or not.  In the future we may have different levels of messages,
        // the higher priority messages being shown and the lower priority ones hidden
        /*
        // If the primary window exists, but is hidden / iconic
        // - suppress displaying of all error messages
        bool existsAndIsHidden = false;

        if (IsWindow(s_primaryWindowHwnd))
        {
            if (!IsWindowVisible(s_primaryWindowHwnd) ||
                IsIconic(s_primaryWindowHwnd))
            {
                // Window hidden, or minimized icon
                existsAndIsHidden = true;
            }
        }

        if (existsAndIsHidden)
        {
            // Don't display if window hidden
            // Except for this one message (NO_EXTERNAL_PA_INTERACTION_ALLOWED)
            // Since NO_EXTERNAL_PA_INTERACTION_ALLOWED is only ever shown
            // while the window is hidden
            if (NO_EXTERNAL_PA_INTERACTION_ALLOWED != error)
            {
                return;
            }
        }
        */

        // TD14164 ++
        TA_Base_Bus::TransActiveMessage userMsg;
        CString description = error;
        userMsg << description;
        std::string errMsg = userMsg.constructMessage(IDS_UW_050018);
        /*internalDisplayMessageBox(error);*/
        internalDisplayMessageBox(errMsg.c_str());
        // ++ TD14164
    }
    catch (...)
    {
        // TD14164 ++
        TA_Base_Bus::TransActiveMessage userMsg;
        std::string errMsg = userMsg.constructMessage(IDS_UE_630017);
        /*internalDisplayMessageBox("Internal error displaying dialog");*/
        internalDisplayMessageBox(errMsg.c_str());
        // ++ TD14164
        return;
    }
}

void PaErrorHandler::internalDisplayMessageBox(const std::string& errorStr)
{
    // Ensure don't hold onto any locks while actually displaying the message
    // box, which is a blocking call (via AfxMessageBox)
    {
        TA_Base_Core::ThreadGuard sync(s_modalErrorDisplayCountLock);
        s_modalErrorDisplayCount++;
    }

    DWORD flags = MB_OK;

    {
        TA_Base_Core::ThreadGuard guard(s_lock);

        if (0 == s_primaryWindowHwnd)
        {
            // Prior to window creation, ensure all error messages displayed in foreground
            flags |= MB_TOPMOST;
        }
    }

    AfxMessageBox(errorStr.c_str(), flags);

    {
        TA_Base_Core::ThreadGuard sync(s_modalErrorDisplayCountLock);
        s_modalErrorDisplayCount--;
    }
}

void PaErrorHandler::displayError(EErrorId errorId)
{
    LOG_SCOPE("PaErrorHandler::displayError");

    TA_Base_Core::ThreadGuard guard(s_lock);

    std::set<DWORD>::iterator itr = s_queuedErrors.find(errorId);

    if (!s_queuedErrors.insert(std::set<DWORD>::value_type(errorId)).second)
    {
        // This message is already queued for display
        // Don't display it a second time (ie send out notification)
        return;
    }

    if (0 == s_primaryWindowHwnd || !::IsWindow(s_primaryWindowHwnd))
    {
        // Queue to display when m_primaryWindowHwnd is available
        return;
    }

    // Post out the error notification for window to process
    ::PostMessage(s_primaryWindowHwnd, WM_PA_ERROR_MESSAGE, errorId, 0);
    LOG_DEBUG("displayError(): PostMessage, WM_PA_ERROR_MESSAGE, %s", nvps(errorId));
}

bool PaErrorHandler::confirmReceiptOfRequest(DWORD id)
{
    TA_Base_Core::ThreadGuard guard(s_lock);

    std::set<DWORD>::iterator itr = s_queuedErrors.find(id);

    if (itr == s_queuedErrors.end())
    {
        // This id not found
        return false;
    }
    else
    {
        s_queuedErrors.erase(itr);
        return true;
    }
}

void PaErrorHandler::setPrimaryWindowHandle(HWND hWnd)
{
    if (0 != hWnd)
    {
        TA_ASSERT(::IsWindow(hWnd), "Invalid window handle");
    }

    TA_Base_Core::ThreadGuard guard(s_lock);

    // Make a copy of the errors - so we can't display same message twice.  If
    // keep getting same error message spammed from separate thread on startup, the
    // user ends up having to acknowledge all of them before application will proceed,
    // which is not what we want - occurs during batch DVA Message update.
    std::set<DWORD> copy = s_queuedErrors;

    // Display all un-acknowledged errors in queue whenever changing handle
    for (std::set<DWORD>::iterator itr = copy.begin();
         itr != copy.end();
         itr++)
    {
        // TES #748 - there are threading issues involved if we attempt to display
        // the remaining errors when resetting the window handle
        // (error displayed, then from message box thread the
        // main thread continues processing windows messages, and attempts to display
        // error again).  This caused a runtime error on exit..
        // Since this error message purging will only happen on
        // shutdown, instead of displaying the errors, just log them instead..
        if (0 != s_primaryWindowHwnd)
        {
            LOG_INFO("An error message was queued on shutdown, enum id = %d", int(*itr));
        }
        else
        {
            // Otherwise when the previous window handle is invalid, OK (safe) to
            // go ahead and display errors - currently this condition only
            // occurs on startup
            displayModalError(EErrorId(*itr));
        }
    }

    // Clear existing items in queue
    s_queuedErrors.clear();

    // Set the window handle AFTERWARDS, to ensure all errors receive foreground status
    s_primaryWindowHwnd = hWnd;
}

bool PaErrorHandler::isDisplayingModalError()
{
    TA_Base_Core::ThreadGuard sync(s_modalErrorDisplayCountLock);
    return s_modalErrorDisplayCount > 0;
}

void PaErrorHandler::defaultExceptionHandler(const CORBA::Exception& e, const char* detail, EErrorId errorId)
{
    std::ostringstream ostr;
    ostr << "Unhandled CORBA exception (" << detail << ")";

    LOG(SourceInfo, TA_Base_Core::DebugUtil::ExceptionCatch, ostr.str().c_str(), e._name());

    PaErrorHandler::displayError(errorId);
}

void PaErrorHandler::defaultExceptionHandler(const TA_Base_Core::TransactiveException& e, const char* detail, EErrorId errorId)
{
    std::ostringstream ostr;
    ostr << "Unhandled Transactive exception (" << detail << ")";

    LOG(SourceInfo, TA_Base_Core::DebugUtil::ExceptionCatch, ostr.str().c_str(), e.what());

    PaErrorHandler::displayError(errorId);
}

void PaErrorHandler::defaultExceptionHandler(const std::exception& e, const char* detail, EErrorId errorId)
{
    std::ostringstream ostr;
    ostr << "Unhandled std::exception (" << detail << ")";

    LOG(SourceInfo, TA_Base_Core::DebugUtil::ExceptionCatch, ostr.str().c_str(), e.what());

    PaErrorHandler::displayError(errorId);
}

void PaErrorHandler::defaultExceptionHandler(const char* detail, EErrorId errorId)
{
    std::ostringstream ostr;
    ostr << "Unhandled (unknown) exception (" << detail << ")";

    LOG(SourceInfo, TA_Base_Core::DebugUtil::ExceptionCatch, ostr.str().c_str(), "Unknown");

    PaErrorHandler::displayError(errorId);
}
