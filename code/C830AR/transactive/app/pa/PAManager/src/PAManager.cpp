/**
 * The source code in this file is the property of
 * Ripple Systems and is not for redistribution
 * in any form.
 *
 * Source:   $File: //depot/C830AR_TIP/C830AR/transactive/app/pa/PAManager/src/PAManager.cpp $
 * @author:  Ripple
 * @version: $Revision: #4 $
 *
 * Last modification: $DateTime: 2025/02/14 12:19:11 $
 * Last modified by:  $Author: limin.zhu $
 *
 */
 // PAManager.cpp : Defines the class behaviors for the application.
 //

#include "stdafx.h"
#include "PAManager.h"
#include "PAManagerGUI.h"
#include "CachedConfig.h"
#include "PaRightsManager.h"
#include "RunParamListener.h"
#include "GraphworxComms.h"
#include "PaManagerDlg.h"
#include "PaStatusNameChangeSubscriber.h"
#include "bus/pa/CachedMaps/src/CachedMaps.h"
#include "bus/generic_gui/src/IGUIApplication.h"
#include "bus/generic_gui/src/AppLauncher.h"
#include "bus/application_types/src/apptypes.h"
#include "core/data_access_interface/pa_4669/src/PaAtsTriggeredBroadcastAccessFactory.h"
#include "core/data_access_interface/pa_4669/src/PaDvaMessageAccessFactory.h"
#include "core/data_access_interface/pa_4669/src/PaDvaMessageVersionAccessFactory.h"
#include "core/data_access_interface/pa_4669/src/PaTrainDvaMessageAccessFactory.h"
#include "core/data_access_interface/pa_4669/src/PaZoneAccessFactory.h"
#include "core/data_access_interface/pa_4669/src/PaZoneGroupAccessFactory.h"
#include "core/utilities/src/NamedScopeLogger.h"
#include "core/utilities/src/DebugUtilEx.h"
#include "core/utilities/src/DebugUtilOptions.h"
#include "core/utilities/src/GlobalInterceptors.h"
#include "core/utilities/src/stdutil/strings.h"
#include "core/utilities/src/StdUtil.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// PAManagerApp

using namespace TA_Base_Core;

BEGIN_MESSAGE_MAP(PAManagerApp, TA_Base_Bus::TransActiveWinApp)
    //{{AFX_MSG_MAP(PAManagerApp)
    // NOTE - the ClassWizard will add and remove mapping macros here.
    //    DO NOT EDIT what you see in these blocks of generated code!
    //}}AFX_MSG
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// PAManagerApp construction

PAManagerApp::PAManagerApp()
    : TA_Base_Bus::TransActiveWinApp(new PAManagerGUI(), "PA Manager")
{
    // Place all significant initialization in InitInstance
}

PAManagerApp::~PAManagerApp()
{
}

int PAManagerApp::ExitInstance()
{
    int result = TransActiveWinApp::ExitInstance();

    // Application de-registered, so may go and start up another instance (if restarting)
    if (PAManagerDlg::isApplicationRestartInProgress())
    {
        attemptRestart();
    }

    // Clean up all static instances that we may (or may not have) used
    // It is expected this will already have been called (before servants
    // de-activated etc) - call again just in case
    cleanupSingletons();

    return result;
}

void PAManagerApp::cleanupSingletons()
{
    PaStatusNameChangeSubscriber::removeInstance();
    TA_IRS_Bus::CachedMaps::removeInstance();
    PaRightsManager::removeInstance();
    CachedConfig::removeInstance();

    TA_Base_Core::PaAtsTriggeredBroadcastAccessFactory::removeInstance();
    TA_Base_Core::PaDvaMessageAccessFactory::removeInstance();
    TA_Base_Core::PaDvaMessageVersionAccessFactory::removeInstance();
    TA_Base_Core::PaTrainDvaMessageAccessFactory::removeInstance();
    TA_Base_Core::PaZoneAccessFactory::removeInstance();
    TA_Base_Core::PaZoneGroupAccessFactory::removeInstance();
}

void PAManagerApp::attemptRestart()
{
    const std::string commandLine = RunParamListener::getInstance().getCommandLine();

    LOG_INFO("Launching PA Manager with command line: %s", commandLine);

    try
    {
        // These default parameters taken from MainNavigator.gdf
        unsigned long posType = TA_Base_Bus::TA_GenericGui::POS_BOUNDED |
            TA_Base_Bus::TA_GenericGui::POS_RELATIVE |
            TA_Base_Bus::TA_GenericGui::POS_RESIZABLE;
        unsigned long alignType = TA_Base_Bus::TA_GenericGui::ALIGN_FIT;

        RECT* objectDim = 0;

        // Determine left position of the schematic the new instance is going to be running for
        int leftPos = PAManagerDlg::getAppplicationRestartTargetPosition();

        // Obtain dimensions we need to place PA Manager at
        RECT screenDim = TA_Base_Bus::AppLauncher::getInstance().getRect(TA_Base_Bus::TA_GenericGui::SCREEN_CURRENT,
                                                                         TA_Base_Bus::TA_GenericGui::AREA_SCHEMATIC,
                                                                         leftPos);

        TA_Base_Bus::AppLauncher::getInstance().launchApplication(PA_MANAGER_GUI_APPTYPE,
                                                                  commandLine,
                                                                  posType,
                                                                  alignType,
                                                                  objectDim,
                                                                  &screenDim);
    }
    catch (...)
    {
        LOG(SourceInfo,
            TA_Base_Core::DebugUtil::ExceptionCatch,
            "Unknown launching application",
            "Caught in PAManagerApp::attemptRestart");

        PaErrorHandler::displayError(PaErrorHandler::ERROR_SCHEMATIC_COMMS);
    }
}

/////////////////////////////////////////////////////////////////////////////
// The one and only PAManagerApp object

PAManagerApp theApp;
