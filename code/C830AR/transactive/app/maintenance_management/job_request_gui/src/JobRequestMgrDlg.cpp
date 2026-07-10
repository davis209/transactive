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

#include "app/maintenance_management/job_request_gui/src/stdafx.h"
#include "app/maintenance_management/job_request_gui/src/JobRequestMgr.h"
#include "app/maintenance_management/job_request_gui/src/resource.h"
#include "app/maintenance_management/job_request_gui/src/JobRequestMgrDlg.h"


#include "app/maintenance_management/job_request_gui/src/AuditMessageUtility.h"

#include "bus/generic_gui/src/IGUIAccess.h"

#include "bus\application_types\src\apptypes.h"

#include "bus/generic_gui/src/TransactiveMessage.h" // TD14164 
#include "core\utilities\src\RunParams.h"
#include "core\utilities\src\DebugUtil.h"

#include "bus\user_settings\src\SettingsMgr.h"
#include "core\exceptions\src\UserSettingsException.h"

#include "core/utilities/src/TAAssert.h"
#include "core/data_access_interface/entity_access/src/IEntityData.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

using TA_Base_Core::RunParams;
using TA_Base_Core::DebugUtil;
using TA_Base_Bus::SettingsMgr;
using TA_Base_Bus::TransActiveDialog;



namespace TA_IRS_App
{

	CJobRequestMgrDlg::CJobRequestMgrDlg(TA_Base_Bus::IGUIAccess& genericGUICallback, 
		const launchConditionsStruct lc, CWnd* pParent /*=NULL*/)
		: TransActiveDialog(genericGUICallback, CJobRequestMgrDlg::IDD, pParent),
		m_jobRequest(0)
	{
		FUNCTION_ENTRY("CJobRequestMgrDlg");

		 m_launchConditions = lc;
		 m_pendingLaunchConditions = m_launchConditions;


		//{{AFX_DATA_INIT(CJobRequestMgrDlg)
		m_assetId = _T("");
		m_requestor = _T("");
		m_problem = _T("");
		m_alarmMessage = _T("");
		m_compDate = 0;
		m_compTime = 0;
		m_dateEnabled = FALSE;
	//}}AFX_DATA_INIT
		


		FUNCTION_EXIT;
	}

	CJobRequestMgrDlg::~CJobRequestMgrDlg()
	{
		FUNCTION_ENTRY("~CJobRequestMgrDlg");
		if(0 != m_jobRequest)
		{
			delete m_jobRequest;
		}

		FUNCTION_EXIT;
	}

	void CJobRequestMgrDlg::init()
	{//TD18095, jianghp, to fix the performance of showing manager application
		try
		{
			//resolve the submitter object
            m_submitter.setEntityName("MmsJobRequestSubmitter_occ",false);
            if( m_submitter->_non_existent() ) throw "bugger";

            AuditMessageUtility::getInstance()->setJobRequestGuiKey(getGuiKey());
		}
		catch(...)
		{
			reportFailureAndTerminate("OCC MMS Agent could not be contacted",
				"The OCC MMS Agent could not be contacted");
			return;
		}
		
		//Manually call the code for launch condition updates
		//so that the parameters are read from the launch conditions
		OnLaunchConditionsChange(0, 0);
	}

	void CJobRequestMgrDlg::DoDataExchange(CDataExchange* pDX)
	{
		TransActiveDialog::DoDataExchange(pDX);
		//{{AFX_DATA_MAP(CJobRequestMgrDlg)
		DDX_Control(pDX, IDSUBMIT, m_submitBtn);
		DDX_Control(pDX, IDC_DATETIMEPICKER2, m_targetTimeCtr);
		DDX_Control(pDX, IDC_DATETIMEPICKER1, m_targetDateCtr);
		DDX_Control(pDX, IDC_CHECK1, m_dateEnableBtn);
		DDX_Control(pDX, IDC_EDIT5, m_alarmMessageCtr);
		DDX_Control(pDX, IDC_EDIT2, m_requestorCtr);
		DDX_Control(pDX, IDC_EDIT1, m_equipmentCtr);
		DDX_Control(pDX, IDC_EDIT4, m_reportCtr);
		DDX_Text(pDX, IDC_EDIT1, m_assetId);
		DDX_Text(pDX, IDC_EDIT2, m_requestor);
		DDX_Text(pDX, IDC_EDIT4, m_problem);
		DDX_Text(pDX, IDC_EDIT5, m_alarmMessage);
		DDX_DateTimeCtrl(pDX, IDC_DATETIMEPICKER1, m_compDate);
		DDX_DateTimeCtrl(pDX, IDC_DATETIMEPICKER2, m_compTime);
		DDX_Check(pDX, IDC_CHECK1, m_dateEnabled);
	//}}AFX_DATA_MAP
	}

	BEGIN_MESSAGE_MAP(CJobRequestMgrDlg, TransActiveDialog)
		//{{AFX_MSG_MAP(CJobRequestMgrDlg)
		ON_BN_CLICKED(IDSUBMIT, OnSubmit)
		ON_BN_CLICKED(IDC_CHECK1, OnDateEnableClick)
		ON_NOTIFY(DTN_DATETIMECHANGE, IDC_DATETIMEPICKER1, OnChangeDate)
		ON_NOTIFY(DTN_DATETIMECHANGE, IDC_DATETIMEPICKER2, OnChangeTime)
	//}}AFX_MSG_MAP
		ON_MESSAGE(LAUNCH_CONDITIONS_UPDATE, OnLaunchConditionsChange)
	END_MESSAGE_MAP()

	/////////////////////////////////////////////////////////////////////////////
	// CJobRequestMgrDlg message handlers

	BOOL CJobRequestMgrDlg::OnInitDialog()
	{
		FUNCTION_ENTRY("OnInitDialog");

		// If necessary this code can go in OnInitDialog before the call to TransActiveDialog::OnInitDialog()
		TA_Base_Bus::ResizingProperties properties;

		CRect rect;
		GetWindowRect( &rect );
		properties.canMaximise = false;
		properties.maxHeight = rect.Height();
		properties.maxWidth = rect.Width();
		properties.minWidth =rect.Width();
		properties.minHeight = rect.Height();
		setResizingProperties(properties);


		TransActiveDialog::OnInitDialog();

		//set static limits for controls:
		m_alarmMessageCtr.SetLimitText(152);
		m_requestorCtr.SetLimitText(20);
		m_equipmentCtr.SetLimitText(40);

		//force this to a top most window. I tried setting the style in the
		//dialog template but that seemed to have no effect...?
		SetWindowPos(&wndTopMost, 0,0,0,0, SWP_NOMOVE | SWP_NOSIZE);

		m_reportCtr.SetFocus();

		FUNCTION_EXIT;
		return FALSE;  // return TRUE  unless you set the focus to a control
	}

	void CJobRequestMgrDlg::OnCancel() 
	{
		FUNCTION_ENTRY("OnCancel");
		TransActiveDialog::DestroyWindow();
	}

	/**
	* buildDate
	* @returns a time_t by combining the date from the m_compDate member
	* and the m_compTime member. Note that these members may not represent the
	* current values set in the controls
	*/
	time_t CJobRequestMgrDlg::buildDate()
	{
		FUNCTION_ENTRY("CJobRequestMgrDlg");
		int nYear = m_compDate.GetYear();
		int nMonth = m_compDate.GetMonth();
		int nDay = m_compDate.GetDay();
		int nHour = m_compTime.GetHour();
		int nMin = m_compTime.GetMinute();
		int nSec = m_compTime.GetSecond();

        // ++TD13664 : AZ 21/11/05
		int nDST = -1; // automatically set Daylight Saving Time
        // TD13664++

        CTime combined( nYear, nMonth, nDay, nHour, nMin, nSec, nDST );
		return combined.GetTime();
	}

	//precondition control data variables have been updated
	//
	//return true if date is not enabled or if it is enabled
	//and is set to a value later than now, 
	//else set new min limit to now on control and return false
	bool CJobRequestMgrDlg::validateDate()
	{

		//don't care what the date is set to if it ain't enabled
		if(!m_dateEnabled) return true;

		time_t dateEntered = buildDate();
		time_t now;
		time(&now);
		if(now < dateEntered) return true;

		//bugger - bad date
		//set the min value for the date and time control.
		CTime earliest(now);

		m_targetDateCtr.SetRange( &earliest,0 );
		m_targetTimeCtr.SetRange( &earliest,0 );

		return false;
	}

	void CJobRequestMgrDlg::OnSubmit()
	{
		FUNCTION_ENTRY("OnSubmit");
		//push the data from the GUI to the members 

		UpdateData(TRUE);

		//return true if date is not enabled or if it is enabled
		//and is set to a value later than now, 
		//else set new min limit to now on control and return false
		if(!validateDate())
		{
			// TD14164 ++
			/*std::string str =
			_T("\nThe Job Request cannot be submitted with this Target Completion Date.\nPlease select a date and time that has not already passed. ");
			MessageBox( str.c_str(), _T("Invalid Target Completion Date")
			, MB_ICONERROR | MB_OK );*/
			TA_Base_Bus::TransActiveMessage userMsg;
			userMsg.showMsgBox(IDS_UE_190004);
			// TD14164 ++

			FUNCTION_EXIT;
			return;
		}

		try
		{
			//Set the Job Request fields to the specified values
			m_jobRequest->assetId = (const char*)m_assetId;
			m_jobRequest->targetDate = buildDate();
			m_jobRequest->targetDateEnabled = m_dateEnabled != 0;
			m_jobRequest->problemReport = (const char*)m_problem;

			//try and submit
			CORBA_CALL( m_submitter,
			            submitJobRequest,
			            ( *m_jobRequest, getSessionId().c_str() ) );

			//if it did not throw the submission was a success 
			LogSuccessEvent();
			//CDialog::OnOK();	
			
			Sleep(1000); // Allow the event to be submitted to Event Agent
			std::string str = _T("Job Request submitted sucessfully.");
			MessageBox( str.c_str(), _T("Submission Successful") , MB_ICONINFORMATION | MB_OK );
			
			TransActiveDialog::DestroyWindow(); // TD18982 - exit from a modeless dialog, use destroywindow()
		
		}
		catch(TA_Base_Core::OperationModeException& ex)
		{
			std::string displayStr = "MMS Agent in transitional state, please retry.";
			reportFailureAndTerminate(displayStr, ex.reason.in());
		}
		catch(TA_Base_Bus::InvalidArg& ex)
		{
			std::string displayStr = "failure while constructing the Job Request.";
			reportFailureAndTerminate(displayStr, ex.reason.in());
		}
		catch(TA_Base_Bus::DependencyFailure& ex)
		{
			reportFailureAndTerminate(ex.reason.in(), ex.reason.in() );
		}
		catch(TA_Base_Bus::SubmissionDenied& ex)
		{
			reportSubmissionDenial(ex.reason.in(), ex.reason.in(), (ex.rights == false) );
		}
		catch(...)
		{
			std::string displayStr = "MMS Agent communication failure.";
			reportFailureAndTerminate(displayStr, displayStr);
		}
		
		FUNCTION_EXIT;
		
	}



	void CJobRequestMgrDlg::updateLaunchConditions(const launchConditionsStruct lc)
	{
		FUNCTION_ENTRY("updateLaunchConditions");
		
		m_pendingLaunchConditions = lc;

		if(::IsWindow(m_hWnd))
		{
			//send an message to tell the dlg 
			PostMessage(LAUNCH_CONDITIONS_UPDATE, 0, 0);
		}
		FUNCTION_EXIT;

	}

	LRESULT CJobRequestMgrDlg::OnLaunchConditionsChange(WPARAM wParam, LPARAM lParam)
	{
		FUNCTION_ENTRY("OnLaunchConditionsChange");
		m_launchConditions = m_pendingLaunchConditions;


		try
		{
			switch(m_launchConditions.launchedFrom)
			{
				case FROM_ALARM:
				{
					initFromAlarm();
					break;
				}
				case FROM_SCHEMATIC:
				{
					initFromSchematic();
					break;
				}
				case INVALID_PARAMS:
				{
					reportFailureAndTerminate("An invalid runparam value has been received", 
						"An invalid runparam value has been received");
					break;
				}
				default:
				{
					TA_ASSERT(false, "Launch condition struct is bad.");
				}

			}
		}
		catch(TA_Base_Bus::DependencyFailure& ex)
		{
			reportFailureAndTerminate(ex.reason.in(), ex.reason.in());
		}

		if(::IsWindow(m_hWnd))
		{
			//push the data from the members to the GUI
			UpdateData(FALSE);
			OnDateEnableClick(); //set the enable/disable state A/R 
		}

		FUNCTION_EXIT;
		return S_OK;
	}

	unsigned int CJobRequestMgrDlg::getGuiKey()
	{
		FUNCTION_ENTRY("CJobRequestMgrDlg");
		unsigned int key = 0;

		try
		{
			TA_Base_Core::IEntityData* pData = getGenericGUICallbackObject().getGUIEntity();
			key =  pData->getKey();
		}
		catch(...)
		{
			//TODO
		}
		FUNCTION_EXIT;
		return key;

	}

	std::string CJobRequestMgrDlg::getSessionId()
	{
		FUNCTION_ENTRY("getSessionId");
		std::string sessionId;
		try
		{
			sessionId = getGenericGUICallbackObject().getSessionId();
			if( sessionId.empty() ) throw "bugger";
		
		}
		catch(...)
		{
			throw TA_Base_Bus::DependencyFailure( "Could not retrieve the session ID." );
		}
		FUNCTION_EXIT;

		return sessionId;
	}


	void CJobRequestMgrDlg::initFromAlarm()
	{
		FUNCTION_ENTRY("initFromAlarm");
		m_jobRequest = 0;
		try
		{
			CORBA_CALL_RETURN( m_jobRequest,
			                   m_submitter,
			                   getDataFromAlarm,
			                   ( m_launchConditions.alarmId.c_str(), m_launchConditions.alarmLocationKey, getSessionId().c_str() ) );
		}
		catch(TA_Base_Bus::DependencyFailure& ex)
		{
			throw TA_Base_Bus::DependencyFailure( ex.reason.in()  );
		}
		catch(TA_Base_Bus::InvalidArg& ex)
		{
			throw TA_Base_Bus::DependencyFailure( ex.reason.in()  );
		}
		catch(...)
		{
			throw TA_Base_Bus::DependencyFailure( "The retrieval of the alarm data failed"  );
		}

		initData();
		FUNCTION_EXIT;
	}

	void CJobRequestMgrDlg::initFromSchematic()
	{
		FUNCTION_ENTRY("initFromSchematic");
		m_jobRequest = 0;
		try
		{
			CORBA_CALL_RETURN( m_jobRequest,
			                   m_submitter,
			                   getDataFromSymbol,
			                   ( m_launchConditions.equipmentName.c_str(), getSessionId().c_str() ) );
		}
		catch(TA_Base_Bus::DependencyFailure& ex)
		{
			throw TA_Base_Bus::DependencyFailure( ex.reason.in()  );
		}
		catch(TA_Base_Bus::InvalidArg& ex)
		{
			throw TA_Base_Bus::DependencyFailure( ex.reason.in()  );
		}
		catch(...)
		{
			throw TA_Base_Bus::DependencyFailure( "The retrieval of the equipment data failed"  );
		}

		initData();
		FUNCTION_EXIT;
	}

	void CJobRequestMgrDlg::initData()
	{
		FUNCTION_ENTRY("initData");
		m_assetId = m_jobRequest->assetId;
		m_requestor = m_jobRequest->requestor;
		m_problem = m_jobRequest->problemReport;
		m_alarmMessage = m_jobRequest->alarmDescField;
		m_compDate = m_jobRequest->targetDate;
		m_compTime = m_jobRequest->targetDate;
		m_dateEnabled = m_jobRequest->targetDateEnabled;		

		//set the min value for the date and time control.
		time_t now;
		time(&now);
		CTime earliest(now);

		m_targetDateCtr.SetRange( &earliest,0 );
		m_targetTimeCtr.SetRange( &earliest,0 );

		//set the max number of chars for the report
		int alarmMessageLen = m_alarmMessage.GetLength();
		int maxProb = 256;
		if(0 != alarmMessageLen)
		{
			maxProb = 256 - alarmMessageLen - 2;
		}

		m_reportCtr.SetLimitText( maxProb );
		FUNCTION_EXIT;
	}

	void CJobRequestMgrDlg::reportFailureAndTerminate(const std::string& displayStr,
		const std::string& eventStr)
	{
		FUNCTION_ENTRY("reportFailureAndTerminate");

		// TD14164 ++
		/*std::string str = _T("The Job Request could not be submitted due to the following failure:\n");
		str += displayStr;
		MessageBox( str.c_str(), _T("Submission Failure")
		, MB_ICONERROR | MB_OK );*/
		TA_Base_Bus::TransActiveMessage userMsg;
		userMsg << displayStr;
		userMsg.showMsgBox(IDS_UE_213000);
		// TD14164 ++

		AuditMessageUtility::getInstance()->submitJobRequestFailureMessage(eventStr, m_jobRequest);
		
		//OnCancel();		// if the first init() failed, will crash because it will still execute "m_reportCtr.SetFocus();"
		if (::IsWindow(GetSafeHwnd()))
		{
			PostMessage(WM_CLOSE);
		}

		FUNCTION_EXIT;
	}

	void CJobRequestMgrDlg::reportSubmissionDenial(const std::string& displayStr,
		const std::string& eventStr, bool terminate /*= true*/)
	{
		FUNCTION_ENTRY("reportSubmissionDenial");

		// TD14164 ++
		/*std::string str = _T("The Job Request submission was denied due to the following reason:\n");
		str += displayStr;
		MessageBox( str.c_str(), _T("Submission Denial")
		, MB_ICONERROR | MB_OK );*/
		TA_Base_Bus::TransActiveMessage userMsg;
		userMsg << displayStr;
		userMsg.showMsgBox(IDS_UW_213001);
		// TD14164 ++

		AuditMessageUtility::getInstance()->submitJobRequestDenialMessage(eventStr, m_jobRequest);

		if(terminate) OnCancel();

		FUNCTION_EXIT;

	}

	void CJobRequestMgrDlg::LogSuccessEvent()
	{
		FUNCTION_ENTRY("LogSuccessEvent");
		AuditMessageUtility::getInstance()->submitJobRequestSubmittedMessage(m_jobRequest);
		FUNCTION_EXIT;
	}

	void CJobRequestMgrDlg::OnDateEnableClick() 
	{
		FUNCTION_ENTRY("CJobRequestMgrDlg");
		//The date/time controls should be enabled only if this is checked
		if(1 == m_dateEnableBtn.GetCheck()) //1 == checked
		{
			m_targetTimeCtr.EnableWindow(TRUE);
			m_targetDateCtr.EnableWindow(TRUE);
		}
		else //is unchecked
		{
			m_targetTimeCtr.EnableWindow(FALSE);
			m_targetDateCtr.EnableWindow(FALSE);
		}
		FUNCTION_EXIT;		
	}

	void CJobRequestMgrDlg::OnChangeDate(NMHDR* pNMHDR, LRESULT* pResult) 
	{
		FUNCTION_ENTRY("OnChangeDate");
		// we set the other control to the same value as this one so that 
		//the validation works
		CTime t;
		m_targetDateCtr.GetTime(t);
		m_targetTimeCtr.SetTime(&t);
		
		*pResult = 0;
		FUNCTION_EXIT;
	}

	void CJobRequestMgrDlg::OnChangeTime(NMHDR* pNMHDR, LRESULT* pResult) 
	{
		FUNCTION_ENTRY("OnChangeTime");
		// we set the other control to the same value as this one so that 
		//the validation works
		CTime t;
		m_targetTimeCtr.GetTime(t);
		m_targetDateCtr.SetTime(&t);
		
		*pResult = 0;
		FUNCTION_EXIT;
	}


} //end namespace TA_IRS_App








