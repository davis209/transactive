/**
  * The source code in this file is the property of 
  * Ripple Systems and is not for redistribution
  * in any form.
  *
  * Source:   $File: //depot/4669_T01271350/4669/transactive/app/maintenance_management/job_request_gui/src/AuditMessageUtility.h $
  * @author:  
  * @version: $Revision: #1 $
  *
  * Last modification: $DateTime: 2008/11/28 16:26:01 $
  * Last modified by:  $Author: builder $
  * 
  * Provides support for logging messages in the MmsAudit context
  */
#if !defined(AFX_AUDITMESSAGEUTILITY_H__EE8A5E03_48A5_47E6_B232_47D67A619541__INCLUDED_)
#define AFX_AUDITMESSAGEUTILITY_H__EE8A5E03_48A5_47E6_B232_47D67A619541__INCLUDED_

#include <string>

#include "core\message\types\MmsAudit_MessageTypes.h"
#include "core/message/src/MessagePublicationManager.h"
#include "core/message/src/AuditMessageSender.h"
#include "bus/maintenance_management/mmsCorbaDef/idl/src/IMmsJobRequestSubmitterCorbaDef.h"

namespace TA_IRS_App
{
	class AuditMessageUtility  
	{
	public:

		/**
		*
		* getInstance
		*
		* @return the one and only instance of this singleton class
		*
		*/
		static AuditMessageUtility* getInstance();

		/**
		*
		* submitJobRequestFailureMessage
		*
		* submits a TA_Base_Core::MmsAudit::JobRequestFailure message
		*
		* @param eventDesc: the decription for the event. Note that this message type
		* has only one parameter and this param is used to populate its value.
		@param jobRequest is the Job Request that failed. Note that this
		* could be in any state including null
		*
		* This method fails silently if the message submission fails
		* precondition: if setJobRequestGuiKey() has not been called then
		* this method may fail under certain conditions
		*/
		void submitJobRequestFailureMessage(
			const std::string& eventDesc, const TA_Base_Bus::JobRequestDetailCorbaDef* jobRequest);
		
		/**
		*
		* submitJobRequestDenialMessage
		*
		* submits a TA_Base_Core::MmsAudit::JobRequestDenial message
		*
		* @param eventDesc: the decription for the event. Note that this message type
		* has only one parameter and this param is used to populate its value.
		* @param jobRequest is the Job Request that failed. Note that this
		* could be in any state including null
		*
		* This method fails silently if the message submission fails
		* precondition: if setJobRequestGuiKey() has not been called then
		* this method may fail under certain conditions
		*/
		void submitJobRequestDenialMessage(
			const std::string& eventDesc, const TA_Base_Bus::JobRequestDetailCorbaDef* jobRequest);

		/**
		*
		* submitJobRequestSubmittedMessage
		*
		* submits a TA_Base_Core::MmsAudit::JobRequestSubmitted message
		*
		* @param jobRequest is the Job Request that failed. Note that this
		* could be in any state including null
		*
		* This method fails silently if the message submission fails
		* precondition: if setJobRequestGuiKey() has not been called then
		* this method may fail under certain conditions
		*
		*/
		void submitJobRequestSubmittedMessage(const TA_Base_Bus::JobRequestDetailCorbaDef* jobRequest);
		
		/**
		 *	DTOR. 
		 * Deletes the instance of this singleton
		 *
		 */		
		virtual ~AuditMessageUtility();

		/**
		 *	setJobRequestGuiKey. 
		 * set the pkey of the job request gui
		 *
		 */	
		void setJobRequestGuiKey(unsigned int guiKey);

	private: //data members:

		unsigned int m_jobRequestGuiKey;

		static AuditMessageUtility* m_instance;

		TA_Base_Core::AuditMessageSender* m_auditMsgSender;

	private: //methods:

		/**
		 *	CTOR is private. This class is a singleton
		 *
		 */
		AuditMessageUtility();

		//not implimented
		AuditMessageUtility(const AuditMessageUtility&);

		//not implimented
		AuditMessageUtility& operator=(const AuditMessageUtility&);

		/**
		 *	sends the message or fails silently
		 * Uses the entity key from jobRequest->getEntityKeyForSubmission()
		 * unless the call fails or returns 0, in which case it uses the key
		 * for the Job Request Manager
		 */
		void submitJobRequestAuditMessage(const TA_Base_Core::MessageType& type,
			const TA_Base_Core::DescriptionParameters& descParams, const TA_Base_Bus::JobRequestDetailCorbaDef* jobRequest);
		
	};

} // end namespace TA_IRS_App

#endif // !defined(AFX_AUDITMESSAGEUTILITY_H__EE8A5E03_48A5_47E6_B232_47D67A619541__INCLUDED_)
