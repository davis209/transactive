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

#if !defined(AFX_JOBREQUESTMGRDLG_H__B1FC56F1_91F3_4680_9193_F5B9A34FE350__INCLUDED_)
#define AFX_JOBREQUESTMGRDLG_H__B1FC56F1_91F3_4680_9193_F5B9A34FE350__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "bus/generic_gui/src/TransActiveDialog.h"
#include "bus/maintenance_management/mmsCorbaDef/idl/src/IMmsJobRequestSubmitterCorbaDef.h"
#include "core/naming/src/NamedObject.h"

#include "resource.h"


namespace TA_IRS_App
{
	class CJobRequestMgrDlg : public TA_Base_Bus::TransActiveDialog
	{
		public:

		//The Job Request Manager is launched from either a alarm or a schematic symbol as
		//indicated by the runparams.
		enum clientType { FROM_ALARM, FROM_SCHEMATIC, INVALID_PARAMS };

		struct launchConditionsStruct
		{
			//A Job Request is submitted from either a alarm or a schematic symbol.
			//and this classes behaviour is dictated by which it was - so we store it.
			clientType launchedFrom;

			//The following parameters are used to store the launch conditions that
			//are parsed from the parameter value. Note that only the alarm params or the
			//equipment param are relavent at any given time:

			//The name of the equpment we are raising the job request on.
			std::string equipmentName; 

			//Where the alarm was raised
			unsigned int alarmLocationKey;

			//The UID string of the alarm
			std::string alarmId;
		};


		CJobRequestMgrDlg(TA_Base_Bus::IGUIAccess& genericGUICallback,
			const launchConditionsStruct lc, CWnd* pParent = NULL);

		virtual ~CJobRequestMgrDlg();

		void init();
		/**
		*
		* updates the dialog based on the new launch conditions. All
		* prior changes by user will be lost. 
		* This method generates a windows event to notify. Hence
		* it uses the Windows message Q to ensure thread saftey
		*/
		void updateLaunchConditions(const launchConditionsStruct lc);

	// Dialog Data
		//{{AFX_DATA(CJobRequestMgrDlg)
	enum { IDD = IDD_MMS_DIALOG };
	CButton	m_submitBtn;
		CDateTimeCtrl	m_targetTimeCtr;
		CDateTimeCtrl	m_targetDateCtr;
		CButton	m_dateEnableBtn;
		CEdit	m_alarmMessageCtr;
		CEdit	m_requestorCtr;
		CEdit	m_equipmentCtr;
		CEdit	m_reportCtr;		
		CString	m_assetId;
		CString	m_requestor;
		CString	m_problem;
		CString	m_alarmMessage;
		CTime	m_compDate;
		CTime	m_compTime;
		BOOL	m_dateEnabled;
	//}}AFX_DATA

		// ClassWizard generated virtual function overrides
		//{{AFX_VIRTUAL(CJobRequestMgrDlg)
		protected:
		virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
		//}}AFX_VIRTUAL

	// Implementation
	protected:

		/**
		 * OnCancel
		 *
		 * This method has been implemented to hide accidental calls when
		 * the ESC key is pressed. It can be overriden with new behaviour if required.
		 */
		virtual afx_msg void OnCancel();

		/**
		 * OnLaunchConditionsChange
		 *
		 * This method updates the dialog based on the current launch conditions
		 * If it can not access remote processes required to get related data
		 * a messagebox is displayed and the application is terminated on
		 * acknowledgement
		 */
		afx_msg LRESULT OnLaunchConditionsChange(WPARAM wParam, LPARAM lParam);

		// Generated message map functions
		//{{AFX_MSG(CJobRequestMgrDlg)
		virtual BOOL OnInitDialog();
		afx_msg void OnSubmit();
	afx_msg void OnDateEnableClick();
	afx_msg void OnChangeDate(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnChangeTime(NMHDR* pNMHDR, LRESULT* pResult);
	//}}AFX_MSG
		DECLARE_MESSAGE_MAP()

	private: //data members:

		// The object exposed by the MMS Agent for Job Request submission:
		TA_Base_Core::NamedObject<TA_Base_Bus::IMmsJobRequestSubmitterCorbaDef,
				TA_Base_Bus::IMmsJobRequestSubmitterCorbaDef_ptr,
				TA_Base_Bus::IMmsJobRequestSubmitterCorbaDef_var> m_submitter;


		//The Job Request we are sending
		TA_Base_Bus::JobRequestDetailCorbaDef* m_jobRequest;


		//we use the pending for thread saftey - see OnLaunchConditionsChange()
		launchConditionsStruct m_launchConditions, m_pendingLaunchConditions;


	private: //methods

		/**
		 * getGuiKey
		 *
		 * Get the pkey for the JobRequestManager entity
		 */
		unsigned int getGuiKey();

		/**
		 * initData
		 *
		 * Use the state of the m_jobRequest member to set the values
		 * of the local data members and set various constraints on controls,
		 * specifically, set the min and max on the date control and the max
		 * chars for the report field. All other control limitations are static
		 *
		 */
		void initData();

		/**
		 * initFromAlarm
		 *
		 * Update the data with the alarm data per the current launch
		 * conditions
		 *
		 * @throws TA_Base_Bus::DependencyFailure if the data can not be retrieved
		 */
		void initFromAlarm();

		/**
		 * initFromSchematic
		 *
		 * Update the data with the equipment data per the current launch
		 * conditions
		 *
		 * @throws TA_Base_Bus::DependencyFailure if the data can not be retrieved
		 */
		void initFromSchematic();

		/**
		 * reportFailureAndTerminate
		 * This method does the following:
		 * 1) A messagebox is displayed with the a message:
		 *	<displayStr>
		 *	Please contact your system administrator.
		 *  The application will now terminate.
		 *
		 * 2) A JobRequestFailure event is logged with <eventStr> as
		 * the description
		 *
		 * 3) Terminate the app
		 * Note that event submission fails silently
		 */
		void reportFailureAndTerminate(const std::string& displayStr,
			const std::string& eventStr);


		/**
		 * reportSubmissionDenial
		 * This method does the following:
		 * 1) A messagebox is displayed with the a message:
		 *	<displayStr>
		 *	The Job Request could not be submitted.
		 *
		 * 2) A JobRequestFailure event is logged with <eventStr> as
		 * the description
		 *
		 * 3) if terminate param == true close the app.
		 *
		 * See SubmissionDeniedException for preconfigured descriptions
		 * Note that event submission fails silently
		 */
		void reportSubmissionDenial(const std::string& displayStr,
		const std::string& eventStr, bool terminate = true);

		/**
		 * LogSuccessEvent
		 * submits an JobRequestSubmitted event recording that the Job Request has been
		 * successfully submitted.
		 * Note that event submission fails silently
		 */
		void LogSuccessEvent();

        /**
        * getSessionId
        *
        * This will return the current sessionId. This should be called each time the sessionId
        * is needed and it will ensure the application is always using the current one.
        *
        * @return string The current sessionId.
		*
		*@throw TA_Base_Bus::DependencyFailure if the session ID can not be retrieved or is empty
        */
		std::string getSessionId();

		/**
		* buildDate
		* @returns a time_t by combining the date from the m_compDate member
		* and the m_compTime member. Note that these members may not represent the
		* current values set in the controls
		*/
		time_t buildDate();

		//precondition control data variables have been updated
		//
		//return true if date is not enabled or if it is enabled
		//and is set to a value later than now, 
		//else set new min limit to now on control and return false
		bool validateDate();

	};
} //end namespace TA_IRS_App

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_JOBREQUESTMGRDLG_H__B1FC56F1_91F3_4680_9193_F5B9A34FE350__INCLUDED_)
