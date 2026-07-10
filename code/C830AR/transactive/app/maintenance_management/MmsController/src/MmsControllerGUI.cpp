/**
  * The source code in this file is the property of 
  * Ripple Systems and is not for redistribution
  * in any form.
  *
  * Source: $File: //depot/4669_T01271350/4669/transactive/app/maintenance_management/MmsController/src/MmsControllerGUI.cpp $
  * @author Darren Sampson
  * @version $Revision: #1 $
  *
  * Last modification: $DateTime: 2008/11/28 16:26:01 $
  * Last modified by: $Author: builder $
  * 
  * Implements required functionality of GenericGUI framework, performs
  *  initialisation of singletons
  *
  */

#pragma warning ( disable : 4250 4786 4284 )

#include "stdafx.h"

#include "app/maintenance_management/MmsController/src/CachedConfig.h"
#include "app/maintenance_management/MmsController/src/MmsControllerGUI.h"
#include "app/maintenance_management/MmsController/src/MmsControllerDlg.h"
#include "app/maintenance_management/MmsController/src/resource.h"

#include "bus/application_types/src/apptypes.h"
#include "bus/user_settings/src/SettingsMgr.h"

#include "core/data_access_interface/entity_access/src/MmsConnectiontEntityData.h"
#include "core/data_access_interface/entity_access/src/IConsole.h"
#include "core/data_access_interface/entity_access/src/ConsoleAccessFactory.h"
#include "core/data_access_interface/entity_access/src/EntityAccessFactory.h"
#include "core/exceptions/src/DataException.h"
#include "core/exceptions/src/GenericGUIException.h"
#include "core/exceptions/src/UserSettingsException.h"
#include "core/utilities/src/DebugUtil.h"
#include "core/utilities/src/RunParams.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

using TA_Base_Core::RunParams;
using TA_Base_Core::DebugUtil;
using TA_Base_Bus::SettingsMgr;

namespace TA_IRS_App
{

	CMmsControllerGUI::CMmsControllerGUI()
    :
    AbstractDialogGUI( IDD_MMSCONTROLLER_DIALOG ),
    m_applicationWnd( NULL ),
    m_myUserSettings( NULL )
	{   
	}


	CMmsControllerGUI::~CMmsControllerGUI()
	{
	}


	void CMmsControllerGUI::createApplicationObject()
	{	
		//TD18095, jianghp, to fix the performance of showing manager application
		m_applicationWnd = new TA_IRS_App::CMmsControllerDlg(*this);
		setApplicationWnd(*m_applicationWnd);		
	}


	unsigned long CMmsControllerGUI::getApplicationType()
	{		
		return MMS_CONTROLLER_GUI_APPTYPE;
	}


	void CMmsControllerGUI::checkEntity(TA_Base_Core::IEntityData* guiEntity)
	{
        LOG( SourceInfo, DebugUtil::FunctionEntry, "checkEntity" );
    
    	setGuiEntity(*guiEntity);
    
        // Note that the following can throw InvalidPAConfigurationException
        // which will be handled by the GenericGUI framework
    
        // This information is set up in checkCommandLine
        std::string sessionId = TA_Base_Core::RunParams::getInstance().get(RPARAM_SESSIONID);
        	
        try
        {
            std::auto_ptr<TA_Base_Core::IConsole> console(
                    TA_Base_Core::ConsoleAccessFactory::getInstance().getConsoleFromSession(sessionId));
        	
			if (0 == console.get())
			{
				LOG_GENERIC( SourceInfo, TA_Base_Core::DebugUtil::DebugError, "Console is NULL, should have thrown exception" );
                TA_THROW(TA_Base_Core::TransactiveException("Console is NULL"));
			}
            
            unsigned long physicalLocationKey = console->getLocation(); 
            CachedConfig::getInstance().setPhysicalLocationKey(physicalLocationKey);
        }
        catch (...)
        {
            LOG_GENERIC(SourceInfo, TA_Base_Core::DebugUtil::DebugError, "%s", "Unable to retrieve console from sessionID" );
    
            // Unable to determine physical location
            TA_THROW(TA_Base_Core::GenericGUIException(TA_Base_Core::GenericGUIException::COMMAND_LINE_INVALID));
        }
        
        unsigned long locationKey = atol(RunParams::getInstance().get(RPARAM_LOCATIONKEY).c_str());    

        CachedConfig::getInstance().setEntityKey(guiEntity->getKey());
        CachedConfig::getInstance().setEntityName(guiEntity->getName());
        CachedConfig::getInstance().setEntityTypeKey(guiEntity->getTypeKey());
        CachedConfig::getInstance().setEntityLocationKey(locationKey);
        CachedConfig::getInstance().setEntitySubsystemKey(guiEntity->getSubsystem());
        CachedConfig::getInstance().setSessionId(sessionId.c_str());

        /*
        The IMmsConnectionCorbaDef interface inherits from IMmsGlobalInhibitCorbaDef. 
        MmsConnection implements IMmsConnectionCorbaDef . 
        And last but not least, MmsConnectiontEntityData provides the data support for MmsConnection. 
         - there is only one entity (not location specific)
        */
        TA_Base_Core::IEntityDataList entityArray = 
            TA_Base_Core::EntityAccessFactory::getInstance().getEntitiesOfType( 
                    TA_Base_Core::MmsConnectiontEntityData::getStaticType());

        if (entityArray.size() != 1)
        {
            // Throw a data exception, connection entity not set up in database
            TA_THROW(TA_Base_Core::DataException("There can be only one Entity NAME",
                                                TA_Base_Core::DataException::NOT_UNIQUE,
                                                "NAME"));
        }
    
        CachedConfig::getInstance().setMmsInhibitAgentEntityName(entityArray[0]->getName());
        CachedConfig::getInstance().setMmsInhibitAgentEntityKey(entityArray[0]->getKey());

        for (TA_Base_Core::IEntityDataList::iterator it = entityArray.begin(); it != entityArray.end(); ++it)
        {
            delete (*it);
        }
            
        LOG( SourceInfo, DebugUtil::FunctionExit, "checkEntity" );
	}


	void CMmsControllerGUI::checkCommandLine()
	{  		
	}


	void CMmsControllerGUI::entityChanged(const std::vector<std::string>& changes)
	{
		//TODO: This will be called by GenericGUI when it receives a callback
		//indicating that the GUI entity has been modified. GenericGUI will have
		//invalidated the entity database object and will tell the GUI which
		//items were modified. This method just has to retrieve the details out
		//of it and update accordingly. If the details in it are invalid then this
		//should print a warning message to the user and continue to use the old settings (ie do not update the GUI).
	}


	void CMmsControllerGUI::serverIsDown()
	{
		// TODO: This method is used to ensure the GUI can handle when the server goes
		//up or down. The GUI should be updated to indicate the network status.
		//If the server has gone down it should enter some sort of read-only mode and
		//if the server comes back up it can restore to the correct operating mode.
	}


	void CMmsControllerGUI::serverIsUp()
	{
		// TODO: This method is used to ensure the GUI can handle when the server goes
		//up or down. The GUI should be updated to indicate the network status.
		//If the server has gone down it should enter some sort of read-only mode and
		//if the server comes back up it can restore to the correct operating mode.
	}


	void CMmsControllerGUI::loadAllUserSettings()
	{
		// First check if we should be loading the user settings.
		if ( RunParams::getInstance().get(RPARAM_USERPREFERENCES).empty() )
		{
		   setWindowPosition();
		   return;
		}

		if (m_myUserSettings == NULL)
		{
			m_myUserSettings = new SettingsMgr(getSessionId(), getApplicationType() );
			setUserSetting(*m_myUserSettings);
		}

		loadPositionSettings();
	}


	void CMmsControllerGUI::saveAllUserSettings()
	{
		// First check if we should be saving the user settings.
		if ( RunParams::getInstance().get(RPARAM_USERPREFERENCES).empty() )
		{
			return;
		}

		if (m_myUserSettings == NULL)
		{
			m_myUserSettings = new SettingsMgr(getSessionId(), getApplicationType() );
			setUserSetting(*m_myUserSettings);
		}
    
		savePositionSettings();
	}


	void CMmsControllerGUI::prepareForClose()
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
		catch ( TA_Base_Core::UserSettingsException&)
		{
			// Handle here with appropriate errror message
		}
		// TODO: Perform any shutdown tasks here
	}

    
    //TD18095, jianghp
	void CMmsControllerGUI::onInitGenericGUICompleted()
	{
		CWaitCursor cur;
		CMmsControllerDlg * pDlg = dynamic_cast<CMmsControllerDlg * >(getApplicationWnd());
		if (NULL == pDlg)
		{
			LOG_GENERIC( SourceInfo, TA_Base_Core::DebugUtil::DebugError, "the main dialog is NULL!" );
			return;
		}
		pDlg->LockWindowUpdate();
		pDlg->init();
		pDlg->UnlockWindowUpdate();
		pDlg->UpdateWindow();
	};
} // TA_IRS_App
