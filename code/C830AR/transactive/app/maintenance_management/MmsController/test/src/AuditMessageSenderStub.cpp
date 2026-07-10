/**
  * The source code in this file is the property of
  * Ripple Systems and is not for redistribution
  * in any form.
  *
  * Source:   $File: //depot/4669_T01271350/4669/transactive/app/maintenance_management/MmsController/test/src/AuditMessageSenderStub.cpp $
  * @author:  Ripple
  * @version: $Revision: #1 $
  *
  * Last modification: $DateTime: 2008/11/28 16:26:01 $
  * Last modified by:  $Author: builder $
  *
  */
#include "AuditMessageSenderStub.h"

namespace TA_Base_Core
{
    const MessageType* AuditMessageSender::s_lastAuditMessageType = 0;

    bool AuditMessageSender::verifyLastAuditMessage(const MessageType& messageType)
    {
        if (s_lastAuditMessageType->getChannel() == messageType.getChannel() &&
            s_lastAuditMessageType->getContext() == messageType.getContext() )
        {
            // matching message (can't use absolute address!)
            return true;
        }
        else 
        {
            return false;
        }        
    }


    void AuditMessageSender::clearAuditMessages()
    {
        s_lastAuditMessageType = 0;
    }


    std::string AuditMessageSender::sendAuditMessage( const MessageType& messageType,
							   unsigned long entityKey,
							   const DescriptionParameters& description,
							   const std::string& details,
							   const std::string& sessionID,
							   const std::string& alarmID,
							   const std::string& incidentID,
							   const std::string& parentEventID,
                               const FilterData* filterableData,
                               const time_t& timeStamp,
                               int)
    {
        s_lastAuditMessageType = &messageType;

        return "";
    }

}