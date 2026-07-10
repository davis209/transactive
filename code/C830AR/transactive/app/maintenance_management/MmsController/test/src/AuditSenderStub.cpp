/**
  * The source code in this file is the property of
  * Ripple Systems and is not for redistribution
  * in any form.
  *
  * Source:   $File: //depot/4669_T01271350/4669/transactive/app/maintenance_management/MmsController/test/src/AuditSenderStub.cpp $
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
    std::string AuditMessageSender::s_lastAuditMessageType;

    bool AuditMessageSender::verifyLastAuditMessage(const type_info& type_info)
    {
        return (stricmp(s_lastAuditMessageType.c_str(), type_info.raw_name()) == 0);
    }


    void AuditMessageSender::clearAuditMessages()
    {
        s_lastAuditMessageType = "";
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
                               const time_t& timeStamp)
    {
        s_lastAuditMessageType = typeid(messageType).raw_name();

        return s_lastAuditMessageType;
    }

}