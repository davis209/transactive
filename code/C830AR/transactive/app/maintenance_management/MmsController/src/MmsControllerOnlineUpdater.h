/**
  * The source code in this file is the property of
  * Ripple Systems and is not for redistribution
  * in any form.
  *
  * Source:   $File: //depot/4669_T01271350/4669/transactive/app/maintenance_management/MmsController/src/MmsControllerOnlineUpdater.h $
  * @author:  Ripple
  * @version: $Revision: #1 $
  *
  * Last modification: $DateTime: 2008/11/28 16:26:01 $
  * Last modified by:  $Author: builder $
  *
  */
// MmsControllerOnlineUpdater.h: interface for the MmsControllerOnlineUpdater class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_MMSCONTROLLERONLINEUPDATER_H__62C9702D_ED9C_47B6_9620_3857BA541A22__INCLUDED_)
#define AFX_MMSCONTROLLERONLINEUPDATER_H__62C9702D_ED9C_47B6_9620_3857BA541A22__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <memory>
#include  "app/maintenance_management/MmsController/src/InhibitResult.h"
#include "core/data_access_interface/entity_access/IDL/src/EntityStatusUpdateDataCorbaDef.h"
#include "core/data_access_interface/entity_access/src/EntityStatusData.h"
#include "core/message/src/SpecialisedMessageSubscriber.h"
#include "core/message/IDL/src/CommsMessageCorbaDef.h"
#include "core/message/src/CommsMessageSender.h"


namespace TA_Base_Core 
{ 
    class IEntityData;
}


namespace TA_IRS_App
{
	class MmsControllerModel;

	class MmsControllerOnlineUpdater : 
			public TA_Base_Core::SpecialisedMessageSubscriber<TA_Base_Core::CommsMessageCorbaDef>
	{
	public:
		MmsControllerOnlineUpdater();
		virtual ~MmsControllerOnlineUpdater();

		virtual void registerCallback(MmsControllerModel*  mmsModelCallBack);
		virtual void deregister();

		void receiveSpecialisedMessage(const TA_Base_Core::CommsMessageCorbaDef& message);
		void sendMmsStateUpdateMessage(const TA_Base_Core::EntityStatusData& statusData, EInhibitResult result);

	private:
		void subscribeToEntityMMSInhibitUpdateMsg();
		void unsubscribeToEntityMMSInhibitUpdateMsg();	
		
		std::auto_ptr< TA_Base_Core::CommsMessageSender> m_commsMsgSender;
		MmsControllerModel* m_callback;

 		TA_Base_Core::EntityStatusData convertEntityStatusCorbaToEntityStatusData(const TA_Base_Core::EntityStatusUpdateCorbaDef& entityStatusCorba);
		


	};
	
}
#endif // !defined(AFX_MMSCONTROLLERONLINEUPDATER_H__62C9702D_ED9C_47B6_9620_3857BA541A22__INCLUDED_)
