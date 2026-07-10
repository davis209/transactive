/**
 * The source code in this file is the property of
 * Ripple Systems and is not for redistribution
 * in any form.
 *
 * Source: $File: //depot/C830AR_TIP/C830AR/transactive/app/pa/PAManager/src/PAErrorHandler.h $
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

#pragma once
#include <boost/describe.hpp>
#include <set>

class PaErrorHandler
{
public:

    // PaErrorHandler::displayError(PaErrorHandler::INVALID_REQUEST_PARAMS);
    enum EErrorId
    {
        CONFIG_DATA_CANNOT_BE_LOADED,   // Configuration data cannot be loaded	UE-010001
        CANNOT_LOAD_PA_MANAGER,         // Cannot load PA Manager	UE-010002
        DATABASE_ERROR,                 // Database error	UE-010007
        INVALID_COMMAND_LINE,           // Invalid command line	UE-010008
        MULTIPLE_INSTANCES_OF_MANAGER,  // Multiple instances of the PA Manager	UE-010009
        UNSPECIFIED_ERROR,              // Unspecified errors	UE-010010
        INVALID_CONFIG_DATA,            // Invalid configuration data	UE-010011
        INIT_ERROR,                     // Initialisation error	UE-010012
        ENTITY_NAME_NOT_FOUND,          // Entity name not found	UE-010013
        NO_CONTROL_STATION,             // Absence of Control Station	UE-010014
        PA_AGENT_FAIL,                  // Failure to contact PA Agent	UE-610001
        AD_HOC_UPDATE_LABEL_FAIL,       // Failure to update ad-hoc message description	UE-610002
        AD_HOC_UPDATE_TYPE_FAIL,          // Failure to update ad-hoc message type	UE-610003
        BROADCAST_TRAIN_VERSION_FAILURE,// Failure to broadcast train DVA messages due to inconsistent train library version numbers.	UE-610004
        INVALID_REQUEST_PARAMS,         // Invalid request parameters.	UE-610005
        BROADCAST_ID_NOT_FOUND,         // Broadcast not found	UE-610006
        BROADCAST_STILL_ACTIVE,         // Broadcast must be terminated first.	UE-610007
        FEPC_HARDWARE_FAILURE,          // Failure to contact PAS FEPC hardware	UE-610008
        STATUS_NAME_IN_USE,              // Status name is in use by another broadcast.	UE-610009

        // Newly added messages
        INVALID_TRAIN_DATA,
        DVA_MESSAGE_UPDATE_FAILURE,
        PRIORITY_SCHEME_FAILURE,        // Error receiving priority scheme update

        THIS_DVA_BROADCAST_DELETED,     // While DVA Broadcast tab open, it was deleted by another operator
        DVA_MESSAGES_NO_LONGER_EXIST,   // When opening an existing DVA broadcast, found some of the messages no longer exist
        INSUFFICIENT_RIGHTS,            // User does not have privileges required to do something
        ADHOC_IS_DELETED,               // Attempting to edit label of a deleted adhoc message
        DVA_MESSAGE_UPDATE_RECEIVED_EXTERNALLY,   // Notification of external ad-hoc message change received
        ONLY_SINGLE_SCHEMATIC_ALLOWED,
        NO_EXTERNAL_PA_INTERACTION_ALLOWED,
        INVALID_BROADCAST_NAME,         // Invalid string for a broadcast ID
        CANNOT_MINIMIZE_PA_MANAGER_WHILE_MSGBOX_ACTIVE,
        MAXIMUM_BROADCAST_COUNT_REACHED,    // Too many active broadcasts, unable to create another
        STATION_CANNOT_RUN_FOR_OVERALL,
        ERROR_SCHEMATIC_COMMS,      // Error identifying or communicating with schematic we've been launched from
        ERROR_AUTO_SHUTDOWN,              // Occurs when going through an automatic shutdown procedure, but modal dialogs open
        TRAIN_AGENT_TIMEOUT,
        TRAIN_AGENT_FAIL,
        CORBA_COMM_FAIL,
        OBJECT_RESOLUTION_EXCEPTION,
        EXTERNAL_TRAIN_DYNAMIC_GROUP_UPDATE_RECEIVED,   // TD #3315
        EXTERNAL_PA_ZONE_UPDATE_RECEIVED,               // TD #3315 (for consistency)
        EXTERNAL_PA_ZONE_GROUP_UPDATE_RECEIVED,         // TD #3315 (for consistency)

        EMPTY_DESCRIPTION_NOT_ALLOWED,
        NEW_DVA_LIBRARY,

        /// Do not add anything after this point
        ERROR_MAX_ID_SPECIFIED,
        ERROR_MAX
    };

    /**
     * displayModalError
     *
     * Displays an error in a modal frame.  Typically user should not call
     *  this directly, unless they know for a fact the current point in code
     *  will be OK if blocked (if the current point in code is acknowledgement
     *  of a network message, it's typically not safe to display a modal error dialog)
     * Since the modal error dialog doesn't ever require a yes/no answer
     *  there is no need to have it pop up at the instance where the problem
     *  occurred, this function shouldn't need to be used (ever), and always
     *  safer to use the default implementation (displayError)
     *
     * @param errorId the identifier of the error message to display
     *
     */
    static void displayModalError(EErrorId errorId);

    /**
     * displayError
     *
     * default error display handler for PA Agent.  The error will be displayed
     *  immediately if there is a primary window attached to error handler,
     *  otherwise the error will be posted to said window (assumed the window
     *  will receive the notification and display the error at that point)
     *
     * @param errorId the identifier of the error message to display
     *
     */
    static void displayError(EErrorId errorId);

    /**
     * confirmReceiptOfRequest
     *
     * Called to confirm a notification sent out to the primary window
     * has been received (for internal checking purposes).
     *
     * @return false if the error handler doesn't recognize the input
     *          (which means the error has already been displayed to user)
     *
     */
    static bool confirmReceiptOfRequest(DWORD id);

    /**
     * setPrimaryWindowHandle
     *
     * Sets the primary error dialog notification window, 0 to unset
     *  the window (in which case errors displayed immediately)
     * Whenever set, if there are errors remaining in queue, they
     *  will be displayed (immediately), to ensure they're not lost
     *
     * @pre handle is a valid window (if non zero)
     *
     */
    static void setPrimaryWindowHandle(HWND hWnd);

    /**
     * isDisplayingModalError
     *
     * @return true if displayModalError function is actively
     *  displaying an AfxMessageBox (from any thread) - use
     *  reference counting to handle nested or concurrent requests
     *
     */
    static bool isDisplayingModalError();

    /**
     * Various exception handler functions
     */
    static void defaultExceptionHandler(const CORBA::Exception& e, const char* detail, EErrorId errorId);
    static void defaultExceptionHandler(const TA_Base_Core::TransactiveException& e, const char* detail, EErrorId errorId);
    static void defaultExceptionHandler(const std::exception& e, const char* detail, EErrorId errorId);
    static void defaultExceptionHandler(const char* detail, EErrorId errorId);

private:

    /**
     * internalDisplayMessageBox
     *
     * Pops up a message box
     *
     * @param errorStr the string to display in the message box
     *
     */
    static void internalDisplayMessageBox(const std::string& errorStr);

    static TA_Base_Core::ReEntrantThreadLockable s_lock;
    static std::set<DWORD> s_queuedErrors;
    static HWND    s_primaryWindowHwnd;

    static TA_Base_Core::ReEntrantThreadLockable s_modalErrorDisplayCountLock;
    static int    s_modalErrorDisplayCount;

    static char* s_errorMessages[];
};

/// Defines a default top level exception handler - application should not rely on this,
/// it is intended to provide last line of defense against unhandled exceptions
/// (to stop app crashing)
#define STANDARD_PA_CATCH(details, errorId)\
    catch (const CORBA::Exception& e)\
    {\
        PaErrorHandler::defaultExceptionHandler(e, details, errorId);\
    }\
    catch (const TA_Base_Core::TransactiveException& e)\
    {\
        PaErrorHandler::defaultExceptionHandler(e, details, errorId);\
    }\
    catch (const std::exception& e)\
    {\
        PaErrorHandler::defaultExceptionHandler(e, details, errorId);\
    }\
    catch (...)\
    {\
        PaErrorHandler::defaultExceptionHandler(details, errorId);\
    }

BOOST_DESCRIBE_ENUM(PaErrorHandler::EErrorId, CONFIG_DATA_CANNOT_BE_LOADED,
                    CANNOT_LOAD_PA_MANAGER,
                    DATABASE_ERROR,
                    INVALID_COMMAND_LINE,
                    MULTIPLE_INSTANCES_OF_MANAGER,
                    UNSPECIFIED_ERROR,
                    INVALID_CONFIG_DATA,
                    INIT_ERROR,
                    ENTITY_NAME_NOT_FOUND,
                    NO_CONTROL_STATION,
                    PA_AGENT_FAIL,
                    AD_HOC_UPDATE_LABEL_FAIL,
                    AD_HOC_UPDATE_TYPE_FAIL,
                    BROADCAST_TRAIN_VERSION_FAILURE,
                    INVALID_REQUEST_PARAMS,
                    BROADCAST_ID_NOT_FOUND,
                    BROADCAST_STILL_ACTIVE,
                    FEPC_HARDWARE_FAILURE,
                    STATUS_NAME_IN_USE,
                    INVALID_TRAIN_DATA,
                    DVA_MESSAGE_UPDATE_FAILURE,
                    PRIORITY_SCHEME_FAILURE,
                    THIS_DVA_BROADCAST_DELETED,
                    DVA_MESSAGES_NO_LONGER_EXIST,
                    INSUFFICIENT_RIGHTS,
                    ADHOC_IS_DELETED,
                    DVA_MESSAGE_UPDATE_RECEIVED_EXTERNALLY,
                    ONLY_SINGLE_SCHEMATIC_ALLOWED,
                    NO_EXTERNAL_PA_INTERACTION_ALLOWED,
                    INVALID_BROADCAST_NAME,
                    CANNOT_MINIMIZE_PA_MANAGER_WHILE_MSGBOX_ACTIVE,
                    MAXIMUM_BROADCAST_COUNT_REACHED,
                    STATION_CANNOT_RUN_FOR_OVERALL,
                    ERROR_SCHEMATIC_COMMS,
                    ERROR_AUTO_SHUTDOWN,
                    TRAIN_AGENT_TIMEOUT,
                    TRAIN_AGENT_FAIL,
                    CORBA_COMM_FAIL,
                    OBJECT_RESOLUTION_EXCEPTION,
                    EXTERNAL_TRAIN_DYNAMIC_GROUP_UPDATE_RECEIVED,
                    EXTERNAL_PA_ZONE_UPDATE_RECEIVED,
                    EXTERNAL_PA_ZONE_GROUP_UPDATE_RECEIVED,
                    EMPTY_DESCRIPTION_NOT_ALLOWED,
                    NEW_DVA_LIBRARY,
                    ERROR_MAX_ID_SPECIFIED,
                    ERROR_MAX);
