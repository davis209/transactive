/**
  * The source code in this file is the property of 
  * Ripple Systems and is not for redistribution
  * in any form.
  *
  * Source:   $File$
  * @author:  
  * @version: $Revision$
  *
  * Last modification: $DateTime$
  * Last modified by:  $Author$
  * 
  * Implimentation of JobRequestMgrGUI class
  *
  */

#include "stdafx.h"
#include <sstream>

#include "app/maintenance_management/job_request_gui/src/JobRequestMgrGUI.h"
#include "app/maintenance_management/job_request_gui/src/JobRequestMgrDlg.h"
#include "app/maintenance_management/job_request_gui/src/BadRunParamException.h"

#include "bus/application_types/src/apptypes.h"
#include "bus/user_settings/src/SettingsMgr.h"

#include "core/exceptions/src/GenericGUIException.h"
#include "core/exceptions/src/UserSettingsException.h"
#include "core/utilities/src/DebugUtil.h"

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
	const char* JobRequestMgrGUI::LAUNCH_PARAM = "LaunchCondition";

	JobRequestMgrGUI::JobRequestMgrGUI() : AbstractDialogGUI( IDD_MMS_DIALOG ),
    m_dlg( NULL )
	{
		m_launchConditions.launchedFrom = CJobRequestMgrDlg::INVALID_PARAMS;
	}


	JobRequestMgrGUI::~JobRequestMgrGUI()
	{

	}


	void JobRequestMgrGUI::createApplicationObject()
	{
		try
		{
			//This method validates the params and sets the local launch
			//condition members to appropriate values, even if the call throws
			setLaunchConditionsFromParams();
		}
		catch(TA_IRS_App::BadRunParamException)
		{
			//already logged
		}

		m_dlg = new CJobRequestMgrDlg(*this, m_launchConditions);
		setApplicationWnd(*m_dlg);

		RunParams::getInstance().registerRunParamUser(this, LAUNCH_PARAM);	
	}


	void JobRequestMgrGUI::prepareForClose()
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


	void JobRequestMgrGUI::saveAllUserSettings()
	{
		// First check if we should be saving the user settings.
		if ( RunParams::getInstance().get(RPARAM_USERPREFERENCES).empty() )
		{
			return;
		}

		if (getUserSetting() == NULL)
		{
			TA_Base_Bus::SettingsMgr* userSettings = new SettingsMgr(getSessionId(), getApplicationType());

			if (userSettings == NULL)
			{
				LOG_GENERIC(SourceInfo, DebugUtil::DebugWarn, "new on SettingsMgr returned NULL. Cannot load user preferences");
			}

			setUserSetting( *userSettings );            
		}
    
		savePositionSettings();
	}


	void JobRequestMgrGUI::loadAllUserSettings()
	{
		// First check if we should be loading the user settings.
		if ( RunParams::getInstance().get(RPARAM_USERPREFERENCES).empty() )
		{
		   setWindowPosition();
		   LOG( SourceInfo, DebugUtil::FunctionExit, "loadAllUserSettings" );            
		   return;
		}

		if (getUserSetting() == NULL)
		{
			TA_Base_Bus::SettingsMgr* userSettings = new SettingsMgr(getSessionId(), getApplicationType());

			if (userSettings == NULL)
			{
				LOG_GENERIC(SourceInfo, DebugUtil::DebugWarn, "new on SettingsMgr returned NULL. Cannot load user preferences");
			}

			setUserSetting( *userSettings );            
		}

		loadPositionSettings();
	}


	void JobRequestMgrGUI::serverIsDown()
	{
		// TODO: This method is used to ensure the GUI can handle when the server goes
		//up or down. The GUI should be updated to indicate the network status.
		//If the server has gone down it should enter some sort of read-only mode and
		//if the server comes back up it can restore to the correct operating mode.
	}


	void JobRequestMgrGUI::serverIsUp()
	{
		// TODO: This method is used to ensure the GUI can handle when the server goes
		//up or down. The GUI should be updated to indicate the network status.
		//If the server has gone down it should enter some sort of read-only mode and
		//if the server comes back up it can restore to the correct operating mode.
	}


	void JobRequestMgrGUI::entityChanged(const std::vector<std::string>& changes)
	{
		//TODO: This will be called by GenericGUI when it receives a callback
		//indicating that the GUI entity has been modified. GenericGUI will have
		//invalidated the entity database object and will tell the GUI which
		//items were modified. This method just has to retrieve the details out
		//of it and update accordingly. If the details in it are invalid then this
		//should print a warning message to the user and continue to use the old settings (ie do not update the GUI).
	}


	void JobRequestMgrGUI::checkCommandLine()
	{  
		FUNCTION_ENTRY("checkCommandLine()");

		try
		{
			//This method validates the params
			setLaunchConditionsFromParams();
		}
		catch(TA_IRS_App::BadRunParamException& ex)
		{
			std::string errorMsg = ex.what();
			TA_THROW(TA_Base_Core::GenericGUIException(errorMsg.c_str(),TA_Base_Core::GenericGUIException::COMMAND_LINE_INVALID));
		}

		FUNCTION_EXIT;
	}


	void JobRequestMgrGUI::checkEntity(TA_Base_Core::IEntityData* guiEntity)
	{
		setGuiEntity(*guiEntity);
	}


	unsigned long JobRequestMgrGUI::getApplicationType()
	{
		//return a value from apptypes.h
		return JOB_REQUEST_MANAGER;
	}


	void JobRequestMgrGUI::onRunParamChange(const std::string& name, const std::string& value)
	{
		FUNCTION_ENTRY ("onRunParamChange");

		//if the parameter change is not related to new launch conditions return
		if (name != LAUNCH_PARAM) 
		{
			FUNCTION_EXIT;
			return;
		}

		try
		{
			//This method validates the params and sets the local launch
			//condition members to appropriate values, even if the call throws
			setLaunchConditionsFromParams();
		}
		catch(TA_IRS_App::BadRunParamException)
		{
			//already logged
		}

		/**
		*
		* updates the dialog based on the new launch conditions. All
		* prior changes by user will be lost. 
		* This method generates a windows event to notify. Hence
		* it uses the message Q to ensure thread saftey
		*/
		m_dlg->updateLaunchConditions(m_launchConditions);

		FUNCTION_EXIT;
	}


	void JobRequestMgrGUI::setLaunchConditionsFromParams()
	{
		FUNCTION_ENTRY("setLaunchConditionsFromParams");
		
		//This is all wrapped in a try so that we can be sure to set m_client = INVALID_PARAM
		//if something is wrong with the parameters
		try
		{
			//if the required param is not set throw
			if ( !( TA_Base_Core::RunParams::getInstance().isSet(LAUNCH_PARAM)) )
			{
				std::ostringstream  details;
				details << "The required runparam " << LAUNCH_PARAM << " is not set.";
				std::string str = details.str();
				TA_THROW(TA_IRS_App::BadRunParamException(str.c_str() ) );
			}

			//Get the paramValue
			std::string paramValue = TA_Base_Core::RunParams::getInstance().get(LAUNCH_PARAM);

			//throw if the param value is empty
			if ( paramValue.empty()) 
			{
				std::ostringstream  details;
				details << "The required runparam " << LAUNCH_PARAM << " value is blank.";
				std::string str = details.str();
				TA_THROW(TA_IRS_App::BadRunParamException(str.c_str() ) );
			}

			//if the param value starts with "alarm"
			if(0 == paramValue.find("alarm"))
			{
				m_launchConditions.launchedFrom = CJobRequestMgrDlg::FROM_ALARM;
			}
			//else if the param value starts with "schematic"
			else if(0 == paramValue.find("schematic"))
			{
				m_launchConditions.launchedFrom = CJobRequestMgrDlg::FROM_SCHEMATIC;
			}
			//else the runparam value does not have the required format
			else
			{
				std::ostringstream  details;
				details <<  "The required runparam " << LAUNCH_PARAM << " value "
                    << "must start with \"alarm\" or \"schematic\".";
				std::string str = details.str();
				TA_THROW(TA_IRS_App::BadRunParamException(str.c_str() ) );
			}

			if(m_launchConditions.launchedFrom == CJobRequestMgrDlg::FROM_ALARM)
			{
				//Use the same error message if any of the param value is invalid:
				std::string str = "When launching from an alarm the runparam must have the form:/n";
				str += "alarm:<location key>:<alarmId>, where <location key> and <alarmId> are uint";

				//the param value must have the form"
				//alarm:<location key>:<alarmId>

				//so find the first colon
				std::string::size_type locFirstCol = paramValue.find(":");

				if(std::string::npos == locFirstCol)
				{
					TA_THROW(TA_IRS_App::BadRunParamException(str.c_str() ) );
				}

				//find the second colon
				std::string::size_type locSecondCol = paramValue.find(":", locFirstCol + 1);

				if(std::string::npos == locSecondCol)
				{
					TA_THROW(TA_IRS_App::BadRunParamException(str.c_str() ) );
				}

				//get the location key:
				std::string locKeyStr(&paramValue[locFirstCol + 1], &paramValue[locSecondCol]);

				//convert to location key to uint
				try
				{
					m_launchConditions.alarmLocationKey = toUint(locKeyStr);
				}
				catch(...)
				{
					TA_THROW(TA_IRS_App::BadRunParamException(str.c_str() ) );				
				}

				//get the alarmId:
// 				m_launchConditions.alarmId = std::string(&paramValue[locSecondCol+ 1], paramValue.end());
 				m_launchConditions.alarmId = std::string(paramValue.begin() + (locSecondCol+ 1), paramValue.end());
				

			}
			else if(m_launchConditions.launchedFrom == CJobRequestMgrDlg::FROM_SCHEMATIC)
			{
				//Use the same error message if any of the param value is invalid:
				std::string str = "When launching from a schematic the runparam must have the form:/n";
				str += "--launch-condition=schematic:<entity name>, where <entity name> can be any non-empty string";

				//the param value must have the form"
				//schematic:<entity name>

				//so find the first colon
				std::string::size_type locFirstCol = paramValue.find(":");

				if(std::string::npos == locFirstCol)
				{
					TA_THROW(TA_IRS_App::BadRunParamException(str.c_str() ) );
				}

				//get the entity name:
				std::string entityNameStr(paramValue.begin() + (locFirstCol + 1), paramValue.end());

				//throw if the string is empty or blank
				if(entityNameStr.empty())
				{
					TA_THROW(TA_IRS_App::BadRunParamException(str.c_str() ) );				
				}
				if(std::string::npos == entityNameStr.find_first_not_of(" ") )
				{
					TA_THROW(TA_IRS_App::BadRunParamException(str.c_str() ) );				
				}

				m_launchConditions.equipmentName = entityNameStr;
			}
			else
			{
				TA_ASSERT(false, "This code should not be reachable");
			}
		}
		catch(TA_IRS_App::BadRunParamException& ex)
		{
			m_launchConditions.launchedFrom = CJobRequestMgrDlg::INVALID_PARAMS;
			throw ex;
		}
		catch(...)
		{
			m_launchConditions.launchedFrom = CJobRequestMgrDlg::INVALID_PARAMS;
			std::string errorMsg = "Unknown exception occurred while parsing runparams.";
			TA_THROW(TA_IRS_App::BadRunParamException(errorMsg.c_str() ) );
		}
		FUNCTION_EXIT;
	}


	unsigned int JobRequestMgrGUI::toUint(const std::string& str)
	{
		FUNCTION_ENTRY("toUint");

		//see if it is empty
		TA_ASSERT( !str.empty(), "string is empty");

		//see if it is all digits:
		std::string::size_type loc = str.find_first_not_of("0123456789");
		TA_ASSERT( std::string::npos == loc, "non-digit in string");

		std::istringstream strm(str);
		unsigned int value;
		strm >> value;
		return value;

		FUNCTION_EXIT;
	}


	//TD18095, jianghp
	void JobRequestMgrGUI::onInitGenericGUICompleted()
	{
		CJobRequestMgrDlg * pDlg = dynamic_cast<CJobRequestMgrDlg * >(getApplicationWnd());
		TA_ASSERT(pDlg != NULL, "the main dialog is NULL!");
		//Hongzhi,CL20343,TES-SDV-LBD-289 Submit job request settings not correct
		//during the lock status, the window status change will be no use, so the interface will be confuse when unlock
//		pDlg->LockWindowUpdate();
		pDlg->init();
//		pDlg->UnlockWindowUpdate();
		if ( ::IsWindow(pDlg->GetSafeHwnd()) )
		{
			pDlg->UpdateWindow();
		}
	};
} //end namespace TA_IRS_App
