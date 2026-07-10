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
  *
  */

#if !defined(AFX_JobRequestMgrGUI_H__2A63FC16_95CE_48E6_9464_A662B7478D49__INCLUDED_)
#define AFX_JobRequestMgrGUI_H__2A63FC16_95CE_48E6_9464_A662B7478D49__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "bus/generic_gui/src/AbstractDialogGUI.h"
#include "core\data_access_interface\entity_access\src\IEntityData.h"
#include "app/maintenance_management/job_request_gui/src/JobRequestMgrDlg.h"
#include "core\utilities\src\RunParams.h"


namespace TA_IRS_App
{
	class JobRequestMgrGUI : public TA_Base_Bus::AbstractDialogGUI, public TA_Base_Core::RunParamUser
	{
	public:



		/**
		*
		* CTOR
		*
		*/
		JobRequestMgrGUI();


		/**
		*
		* DTOR
		*
		*/
		virtual ~JobRequestMgrGUI();

		//TD18095, jianghp
		virtual void onInitGenericGUICompleted();

		/**
		* createApplicationObject
		*
		* This method must set the m_application member variable. It should create a new
		* instance of the dialog/document/property sheet and assign it to m_application
		*/
		virtual void createApplicationObject();

		/**
		* prepareForClose
		*
		* This method is used to prepare the GUI for closing. It should save the user settings
		* and perform any other tidying up. eg Stop any processing threads, ask the user if they
		* wish to save anything etc.
		* This should be called before any close is done on the GUI whether initiated by the
		* GUI itself or the Control Station. This means it should be the first call in the 
		* implementation of terminateGUI().
		*/
		virtual void prepareForClose();


		/**
		* saveAllUserSettings
		*
		* This will save all the application's user preferences if configured to do so.
		* This is provided in case the GUI needs to save user preferences at a predetermined
		* time. The settings are already saved when the application exits.
		*
		*/		
		virtual void saveAllUserSettings();


		/**
		* loadAllUserSettings
		*
		* This will load all the application's user preferences if configured to do so.
		* This should be called by the application itself in OnInit() or equivalent. 
		*
		*/
		virtual void loadAllUserSettings();


		/**
		* serverIsDown
		*
		* This is called when the server the workstation is using goes down. The GUI should
		* indicate to the user that the server is down. It should also then limit operations
		* so that the user can only view existing data. This must be implemented by the lowest
		* level class as it knows what to do when this occurs.
		*/
		virtual void serverIsDown();


		/**
		* serverIsUp
		*
		* This is called when the server the workstation is using starts functioning again.
		* The GUI should indicate to the user that the server is functioning. It should 
		* then allow the user to perform all valid operations again. This must be implemented by
		* the lowest level class as it knows what to do when this occurs.
		*/
		virtual void serverIsUp();


		/**
		* entityChanged
		*
		* This will be called by GenericGUI when it receives a callback indicating that the
		* GUI entity has been modified. GenericGUI will have invalidated the entity database
		* object and will tell the GUI which items were modified. The GUI just has to retrieve
		* the details out of it and update accordingly. This should be implemented by the
		* lowest level class as it knows exactly what to do with the updates.
		*
		* @param vector<string>& changes - This is a list of all attributes changed in the
		*                                  object.
		*
		*/
		virtual void entityChanged(const std::vector <std::string>& changes);


		/**
		* checkCommandLine
		*
		* This method throws if the current RunParams are valid not valid. 
		* The criteria is as follows:
		* The parameter/value pair must have one of the two following forms:
		*	--launch-condition=alarm:<location key>:<alarmId>
		*   --launch-condition=schematic:<entity name>
		*
		* Where:
		* <location key>:and <alarmId> must be only digits, i.e. convertible to uint.
		* <entity name> can be any non-empty, non-blank string
		*
		* @param errorMsg is a string reference that this method populates with a reason why the
		* runparams are not valid if this method returns false.
		*
		* Note that this method does NOT perform further validation of the values 
		* passed such as if <alarmId> is in fact the PKEY of an existing alarm at 
		* the specified location. Issues such as these are reflected by the behaviour of
		* the JobRequestMessage CTOR, etc.
		*
		* @throw GenericGUIException if param is not valid
		*/
		virtual void checkCommandLine();


		/**
		* checkEntity
		* 
		* This will check that the entity is valid and then store a local pointer to the
		* entity. This must be implemented in the lowest level class as this will be where
		* the GUI specific configuration details are known.
		*
		* @param IEntityData* guiEntity - A pointer to the gui entity to check and store.
		*
		* @exceptions GenericGUIException with the type set to ENTITY_CONFIGURATION_INVALID
		*             This should be thrown if the application checks the entity and it is
		*             not configured correctly for the GUI to run.
		*
		*/
		virtual void checkEntity(TA_Base_Core::IEntityData* guiEntity);


		/**
		* getApplicationType
		* 
		* This returns the application type for this GUI. GenericGUI will then save
		* this in the RunParams and will use it to pass to the Control Station. In the
		* existing system the application type would need to be hard-coded into here.
		* This method must be implemented by the lowest level class.
		*
		* @return unsigned long representing the application type
		*
		*/
		virtual unsigned long getApplicationType();

		/**
		* onRunParamChange
		*
		* called when a runparam we have registered for changes
		*
		* This method checks if the parameter change reflects new launch conditions.
		* If so it updates the message fields and updates the user.
		*
		* If the new parameter is invalid or the resolution of the new fields fails 
		* then all fields are cleared and the user is notified accordingly.
		*
		* If the parameter and value do not differ from the current launch conditions then
		* no action is taken
		*/
		virtual void onRunParamChange(const std::string& name, const std::string& value);

	private: //members

		//The dialog
		CJobRequestMgrDlg* m_dlg;

		//the name of the required runparam
		const static char* LAUNCH_PARAM;


		CJobRequestMgrDlg::launchConditionsStruct  m_launchConditions;

		

	private: //methods


		/**
		* setLaunchConditionsFromParams
		*
		* Sets the local launch param members to the values indicated by the current
		* runparam values.
		* The criteria for the run params is defined by the command line format:
		* The command line must have one of the two following forms:
		*	--launch-condition=alarm:<location key>:<alarmId>
		*   --launch-condition=schematic:<entity name>
		*
		* Where:
		* "alarm" indicates that the Job Request is associated with an alarm
		* "schematic" indicates that the Job Request is associated with a piece
		* of equipment in a schematic.
		* If "alarm":
		*	<alarmId> is the string UID of the associated alarm
		*	<location key> is the uint pkey of the location that the alarm is from
		* If "schematic":
		*	<entity name> is the entity name of the equipment datanode
		*
		* @throw BadRunParamException exception if any of the conditions described above
		* for the runparams are not set. If this method throws the m_client param is
		* guarenteed to be set to INVALID_PARAM
		*
		*/
		void setLaunchConditionsFromParams();

		unsigned int toUint(const std::string& str);

	};
} // end namespace TA_IRS_App
 
#endif // !defined(AFX_JobRequestMgrGUI_H__2A63FC16_95CE_48E6_9464_A662B7478D49__INCLUDED_)
