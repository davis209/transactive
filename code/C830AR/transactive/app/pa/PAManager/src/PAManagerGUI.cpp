/**
 * The source code in this file is the property of
 * Ripple Systems and is not for redistribution
 * in any form.
 *
 * Source:   $File: //depot/C830AR_TIP/C830AR/transactive/app/pa/PAManager/src/PAManagerGUI.cpp $
 * @author:  Ripple
 * @version: $Revision: #6 $
 *
 * Last modification: $DateTime: 2025/03/05 13:16:38 $
 * Last modified by:  $Author: limin.zhu $
 *
 */
// PAManagerGUI.cpp : implementation file
//

#if defined(_MSC_VER)
#pragma warning(disable : 4786 4146 4018 4250 4290 4284)
#endif // defined _MSC_VER

#include "app/pa/PAManager/src/stdafx.h"

#include "app/pa/PAManager/src/PAManagerGUI.h"
#include "app/pa/PAManager/src/CachedConfig.h"
#include "app/pa/PAManager/src/CachedMapLoader.h"
#include "app/pa/PAManager/src/GraphworxComms.h"
#include "app/pa/PAManager/src/MemTracker.h"
#include "app/pa/PAManager/src/PAmanager.h"
#include "app/pa/PAManager/src/PAManagerDlg.h"
#include "app/pa/PAManager/src/RunParamListener.h"

#include "bus/application_types/src/apptypes.h"
#include "bus/pa/cachedmaps/src/CachedMaps.h"
#include "bus/pa/cachedmaps/src/PaStationDvaMessageCachedMap.h"
#include "bus/pa/cachedmaps/src/PaTrainDvaMessageCachedMap.h"
#include "bus/user_settings/src/SettingsMgr.h"

#include "core/configuration_updates/src/OnlineUpdateListener.h"
#include "core/data_access_interface/src/LocationAccessFactory.h"
#include "core/data_access_interface/entity_access/src/TrainAgentEntityData.h"
#include "core/data_access_interface/entity_access/src/IConsole.h"
#include "core/data_access_interface/entity_access/src/ConsoleAccessFactory.h"
#include "core/exceptions/src/GenericGUIException.h"
#include "core/exceptions/src/UserSettingsException.h"
#include "core/utilities/src/DebugUtil.h"
#include "core/utilities/src/DebugUtilOptions.h"
#include "core/utilities/src/RunParams.h"
#include "core/utilities/src/GlobalInterceptors.h"
#include "core/utilities/src/stdutil/strings.h"
#include "core/utilities/src/StdUtil.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

using namespace TA_Base_Core;
using TA_Base_Bus::SettingsMgr;
using namespace TA_Base_Bus;

/////////////////////////////////////////////////////////////////////////////
// PAManagerGUI

PAManagerGUI::PAManagerGUI()
    : AbstractDialogGUI(IDD_MAIN_DLG),
    m_PAManagerDlg(NULL),
    m_commandLineInvalid(false)
{
#ifdef _DEBUG
    MemTracker::attachMemoryHook();
#endif

    NamedScopeLoggerConfig::enable();

    {
        // ignore logs

        static const auto s_excludes =
        {
            "transactivedialog.cpp",
        };

        LOG_DEBUG("PAManagerApp(): will ignore log in files:\n%s", st::join("\n", s_excludes));

        DebugUtilOptions::enable_output_interceptor();
        GlobalInterceptors::add("DebugUtil::Impl::logMessage::ignore", [](const char* file)
        {
            return st::any_of_iequals(s_excludes, st::filename(file));
        });
    }
}

void PAManagerGUI::createApplicationObject()
{
    if (!m_PAManagerDlg)
    {
        if (m_commandLineInvalid)
        {
            // Create a dummy dialog, with empty DoModal()
            // implementation to allow rapid shutdown - this
            // is only done as no way to exit GenericGUI quickly+silently
            // when the command line fails
            m_PAManagerDlg = new NullDialog();
        }
        else
        {
            m_PAManagerDlg = new PAManagerDlg(*this);
            CachedConfig::getInstance()->setMainWnd(m_PAManagerDlg);
        }

        setApplicationWnd(*m_PAManagerDlg);
    }
}

PAManagerGUI::~PAManagerGUI()
{
    try
    {
        if (m_PAManagerDlg)
        {
            delete m_PAManagerDlg;
            m_PAManagerDlg = NULL;
        }
    }
    catch (...)
    {
        LOG(SourceInfo, TA_Base_Core::DebugUtil::ExceptionCatch, "Unknown", "Caught in destructor");
    }
}

unsigned long PAManagerGUI::getApplicationType()
{
    FUNCTION_ENTRY("getApplicationType");
    FUNCTION_EXIT;
    return PA_MANAGER_GUI_APPTYPE;
}

void PAManagerGUI::checkEntity(TA_Base_Core::IEntityData* guiEntity)
{
    LOG_SCOPE("PAManagerGUI::checkEntity");

    if (m_commandLineInvalid)
    {
        return;
    }

    FUNCTION_ENTRY("checkEntity");

    setGuiEntity(*guiEntity);

    // Note that the following can throw InvalidPAConfigurationException
    // which will be handled by the GenericGUI framework

    // This information is set up in checkCommandLine
    unsigned long locationKey = CachedConfig::getInstance()->getEntityLocationKey();

    CachedConfig::getInstance()->setEntityKey(guiEntity->getKey());
    CachedConfig::getInstance()->setEntityName(guiEntity->getName());
    CachedConfig::getInstance()->setEntitySubsystemKey(guiEntity->getSubsystem());
    CachedConfig::getInstance()->setEntityTypeKey(guiEntity->getTypeKey());

    CachedConfig::getInstance()->setSessionId(RunParams::getInstance().get(RPARAM_SESSIONID).c_str());

    std::string entityName;

    if (locationKey == LocationAccessFactory::getInstance().getOccLocationKey())
    {
        entityName = MASTER_PA_AGENT_ENTITY_TYPE_NAME;
    }
    else if (locationKey == LocationAccessFactory::getInstance().getBoccLocationKeyOptional().value_or(0))
    {
        entityName = MASTER_PA_AGENT_ENTITY_TYPE_NAME;
    }
    else
    {
        entityName = STATION_PA_AGENT_ENTITY_TYPE_NAME;
    }

    // Not used currently
    //    CachedConfig::getInstance()->setLocalPAAgentKey(DEMO_PA_AGENT_KEY);
    std::vector<CorbaName> allNames = EntityAccessFactory::getInstance().getCorbaNamesOfTypeAtLocation(entityName, locationKey, true);
    TA_ASSERT(allNames.size() == 1, "There can be only one CORBA NAME at specified location");
    CachedConfig::getInstance()->setLocalPAAgentCorbaName(allNames[0]);

    {
        TA_Base_Core::IEntityDataList entityArray = EntityAccessFactory::getInstance().getEntitiesOfTypeAtLocation(entityName, locationKey);
        TA_ASSERT(entityArray.size() == 1, "There can be only one Entity NAME at specified location");

        CachedConfig::getInstance()->setLocalPAAgentEntityName(entityArray[0]->getName());
        TA_IRS_Bus::deleteAllItemsInArray(entityArray);
    }

    {
        TA_Base_Core::IEntityDataList entityArray = EntityAccessFactory::getInstance().getEntitiesOfTypeAtLocation(TrainAgentEntityData::getStaticType(), locationKey);
        TA_ASSERT(entityArray.size() == 1, "There can be only one Entity NAME at specified location");

        CachedConfig::getInstance()->setLocalTrainAgentEntityName(entityArray[0]->getName());
        TA_IRS_Bus::deleteAllItemsInArray(entityArray);
    }

    try
    {
        // Perform initial refresh of cached maps
        CachedMapLoader::loadAllCachedMaps();

        // Tell the online update listener we want to automatically register
        // any new keys as they come through
        TA_Base_Core::OnlineUpdateListener::getInstance().setListenOutForNewKeys(true);

        // Begin listening for out for updates for subset of maps
        // PA Zone and PA Zone Group updates handled
        TA_IRS_Bus::CachedMaps::getInstance()->getPaZoneCachedMap().startListening();
        TA_IRS_Bus::CachedMaps::getInstance()->getPaZoneGroupCachedMap().startListening();

        // Pa Station DVA Message updates listened out for
        TA_IRS_Bus::CachedMaps::getInstance()->getPaStationDvaMessageCachedMap().startListening();
        // Pa Train DVA Messages listened out for
        TA_IRS_Bus::CachedMaps::getInstance()->getPaTrainDvaMessageCachedMap().startListening();

        // Dva versions are handled via PA Agent (don't listen out for them)
    }
    catch (const TA_Base_Core::TransactiveException& ex)
    {
        // Error reading data from database
        LOG(SourceInfo, TA_Base_Core::DebugUtil::ExceptionCatch, "Database initial read error", ex.what());
        throw;
    }
    catch (const std::exception& se)
    {
        LOG(SourceInfo, TA_Base_Core::DebugUtil::ExceptionCatch, "Internal error reading database", se.what());
        throw;
    }

    FUNCTION_EXIT;
}

void PAManagerGUI::checkCommandLine()
{
    LOG_SCOPE("PAManagerGUI::checkCommandLine");
    FUNCTION_ENTRY("checkCommandLine");

    // On startup - deactivate all the graphworx displays (until startup complete)
    TA_App::GraphworxComms::getInstance().resetGraphworxDisplaysToInactive();

    // Must collect location data from database - to set up initial location parameters
    TA_IRS_Bus::CachedMaps::getInstance()->refreshLocationKeyToNameMap();

    ////////////////////////////////////////////
    // Perform early setup of location key information, to allow checking of command line
    // Note there are three locations:  The physical location
    // key is hardwired, and will always be the same for a given machine
    // The virtual location key depends on which schematic we were launched from

    // The LocationKey runparam (RPARAM_LOCATIONKEY) doesn't represent
    // either of these (represents non-schematic related location), and hence shouldn't be used.
    // The physical location can be taken from the console
    std::string sessionId = TA_Base_Core::RunParams::getInstance().get(RPARAM_SESSIONID);

    try
    {
        std::auto_ptr<TA_Base_Core::IConsole> console(TA_Base_Core::ConsoleAccessFactory::getInstance().getConsoleFromSession(sessionId));

        TA_ASSERT(0 != console.get(), "Should have thrown exception");

        unsigned long physicalLocationKey = console->getLocation();
        CachedConfig::getInstance()->setPhysicalLocationKey(physicalLocationKey);
    }
    catch (...)
    {
        if (stricmp(sessionId.c_str(), "debug") != 0)
        {
            LOG_ERROR("Unable to retrieve console from sessionID");

            // Unable to determine physical location
            TA_THROW(TA_Base_Core::GenericGUIException(TA_Base_Core::GenericGUIException::COMMAND_LINE_INVALID));
        }
        else
        {
            // In debug mode, go for a physical location defined by locationkey runparam
            unsigned long physicalLocationKey = atol(TA_Base_Core::RunParams::getInstance().get(RPARAM_LOCATIONKEY).c_str());
            CachedConfig::getInstance()->setPhysicalLocationKey(physicalLocationKey);
        }
    }

    std::string commandLineLocationName = RunParamListener::getInstance().findVirtualLocation();

    unsigned long locationKey = -1;

    try
    {
        // The command line location name (passed in from schematics) identifies what location
        // the schematics is running for (or the 'virtual' as opposed to physical location)
        if (boost::iequals(commandLineLocationName, "LIN"))
        {
            // Special keyword exists for the 'Overall' schematic, as it is not a proper location
            locationKey = LocationAccessFactory::getInstance().getOccLocationKey();
        }
        else
        {
            locationKey = TA_IRS_Bus::CachedMaps::getInstance()->getLocationKeyFromName(commandLineLocationName);
        }

        if (LocationAccessFactory::getInstance().getOccLocationKey() == CachedConfig::getInstance()->getPhysicalLocationKey())
        {
            // Whenever we're at the OCC terminal, we leave our physical location as OCC (so we
            // continue to communicate with the OCC agent - NOT the stations agent)
            locationKey = CachedConfig::getInstance()->getPhysicalLocationKey();
        }
        else if (LocationAccessFactory::getInstance().getBoccLocationKeyOptional().value_or(0) == CachedConfig::getInstance()->getPhysicalLocationKey())
        {
            // Whenever we're at the BOCC terminal, we leave our physical location as BOCC (so we
            // continue to communicate with the BOCC agent - NOT the stations agent)
            locationKey = CachedConfig::getInstance()->getPhysicalLocationKey();
        }
        else
        {
            // If we're not at the OCC, must be station or depot
            // The station / depot can operate for any other location - except for overall
            // (that is, they can't communicate with the OCC agent)
            if (LocationAccessFactory::getInstance().getOccLocationKey() == locationKey)
            {
                // A station cannot operate on the overall schematic
                PaErrorHandler::displayModalError(PaErrorHandler::STATION_CANNOT_RUN_FOR_OVERALL);
                m_commandLineInvalid = true;
                return;
            }

            if (LocationAccessFactory::getInstance().getBoccLocationKeyOptional().value_or(0) == locationKey)
            {
                // A station cannot operate on the overall schematic
                PaErrorHandler::displayModalError(PaErrorHandler::STATION_CANNOT_RUN_FOR_OVERALL);
                m_commandLineInvalid = true;
                return;
            }
        }

        // We may need to run PA Manager for a foreign station - in which case locationKey
        // won't match physicalLocationKey (locationKey defines station we're operating for)
        // - any restrictions in rights due to a mismatched location key need to be
        // implemented in the PaRightsManager class
        CachedConfig::getInstance()->setEntityLocationKey(locationKey);
    }
    catch (const TA_Base_Core::CachedMappingNotFoundException&)
    {
        std::ostringstream reason;
        reason << "Could not find location key for location specified in command line: " << commandLineLocationName;

        LOG_ERROR(reason);

        // Couldn't find the specified location in the database - must be an invalid command
        TA_THROW(TA_Base_Core::GenericGUIException(TA_Base_Core::GenericGUIException::COMMAND_LINE_INVALID));
    }

    TA_ASSERT(-1 != locationKey, "Invalid location key value");

    // Determine special location flags:
    if (locationKey == LocationAccessFactory::getInstance().getOccLocationKey())
    {
        CachedConfig::getInstance()->setLocationType(CachedConfig::LOCATION_TYPE_OCC);
        TA_IRS_Bus::CachedMaps::getInstance()->setOccDepotFlags(true, false);
    }
    else if (locationKey == LocationAccessFactory::getInstance().getBoccLocationKeyOptional().value_or(0))
    {
        CachedConfig::getInstance()->setLocationType(CachedConfig::LOCATION_TYPE_OCC);
        TA_IRS_Bus::CachedMaps::getInstance()->setOccDepotFlags(true, false);
    }
    else
    {
        if (locationKey == LocationAccessFactory::getInstance().getDepotLocationKey())
        {
            CachedConfig::getInstance()->setLocationType(CachedConfig::LOCATION_TYPE_DEPOT);
            TA_IRS_Bus::CachedMaps::getInstance()->setOccDepotFlags(false, true);
        }
        else
        {
            // Station
            CachedConfig::getInstance()->setLocationType(CachedConfig::LOCATION_TYPE_STATION);
            TA_IRS_Bus::CachedMaps::getInstance()->setOccDepotFlags(false, false);
        }
    }

    // The command line should have specified an initial runparam
    // that can be used to identify this instance
    if (!RunParamListener::getInstance().retrieveInitialRunParamIdentity())
    {
        // This is required since when we throw an exception from here
        // All the core threading functionality is killed off - and
        // cleanupSingletons will have issued if called after this..
        PAManagerApp::cleanupSingletons();

        // TD #2593: Removed as per request (so only single error displayed)
        //TA_THROW(TA_Base_Core::GenericGUIException(
        //            TA_Base_Core::GenericGUIException::COMMAND_LINE_INVALID));

        // Display our error in a modal dialog (the only feedback user will get before shutdown)
        PaErrorHandler::displayModalError(PaErrorHandler::NO_EXTERNAL_PA_INTERACTION_ALLOWED);

        m_commandLineInvalid = true;
    }

    FUNCTION_EXIT;
}

void PAManagerGUI::entityChanged(const std::vector<std::string>& changes)
{
    FUNCTION_ENTRY("entityChanged");

    // TODO: Check the changes passed and pass any relevant information on.

    FUNCTION_EXIT;
}

void PAManagerGUI::serverIsDown()
{
    FUNCTION_ENTRY("serverIsDown");

    // TODO: Get the GUI to update to indicate this.

    FUNCTION_EXIT;
}

void PAManagerGUI::serverIsUp()
{
    FUNCTION_ENTRY("serverIsUp");

    // TODO: Get the GUI to update to indicate this.

    FUNCTION_EXIT;
}

void PAManagerGUI::loadAllUserSettings()
{
    FUNCTION_ENTRY("loadAllUserSettings");

    // First check if we should be loading the user settings.
    if (RunParams::getInstance().get(RPARAM_USERPREFERENCES).empty())
    {
        setWindowPosition();
        FUNCTION_EXIT;
        return;
    }

    loadPositionSettings();

    FUNCTION_EXIT;
}

void PAManagerGUI::saveAllUserSettings()
{
    // First check if we should be saving the user settings.
    if (RunParams::getInstance().get(RPARAM_USERPREFERENCES).empty())
    {
        return;
    }

    savePositionSettings();
}

void PAManagerGUI::prepareForClose()
{
    //TODO: This provides a standard method for each GUI to use to clean itself
    //up ready to exit. This will be called whenever the application exits whether
    //it is closed by the Control Station or by the user. This should perform
    //tasks such as saving user settings, stopping any threads and asking the user
    //if they want to save their settings. When this method is completed the GUI
    //should be in a safe state to be closed.

    try
    {
        saveAllUserSettings();
    }
    catch (TA_Base_Core::UserSettingsException&)
    {
        // Handle here with appropriate errror message
    }

    // TODO: Perform any shutdown tasks here
}

//TD18095, jianghp
void PAManagerGUI::onInitGenericGUICompleted()
{
    LOG_SCOPE("PAManagerGUI::onInitGenericGUICompleted");
    CWaitCursor cur;
    PAManagerDlg* pDlg = dynamic_cast<PAManagerDlg*>(getApplicationWnd());
    TA_ASSERT(pDlg != NULL, "the main dialog is NULL!");
    pDlg->LockWindowUpdate();
    pDlg->init();
    pDlg->UnlockWindowUpdate();
    pDlg->UpdateWindow();
    cur.Restore();
}

//TES230++ //added implementation in setWindowPosition
// Code taken from STISManager ---
// TES #720
// jeffrey++ TES640
// changeFocus() is removed by TES720. It caused PA Manager always get focus (show up) when
// selecting zone at schematic. Resuming it back and can't reproduce TES720 any more.
//void PAManagerGUI::changeFocus(const TA_Base_Core::EFocusType focus)
//{
//	// We only want to handle the change of focus
//	// if the application is not currently hidden,
//	// ie the operator is still making PID selections,
//	// and has not pressed the 'Launch STIS Manager' button
//
//	if( AbstractGUIApplication::getApplicationWnd() == NULL )
//    {
//        // Too early
//        return;
//    }
//
//    PAManagerDlg* dlg = dynamic_cast<PAManagerDlg*>(AbstractGUIApplication::getApplicationWnd());
//    TA_ASSERT(dlg, "dlg is not a PAManagerDlg");
//    if ( dlg->wantToShow() )
//    {
//	    AbstractGUIApplication::changeFocus( focus );
//
//    }
//}
// ++jeffrey TES640
//++TES230

/////////////////////////////////////////////////////////////////////////////
// PAManagerGUI message handlers
