/**
  * The source code in this file is the property of
  * Ripple Systems and is not for redistribution
  * in any form.
  *
  * Source:   $File: //depot/4669_T01271350/4669/transactive/app/maintenance_management/MmsController/test/src/AuditSenderStub.h $
  * @author:  Ripple
  * @version: $Revision: #1 $
  *
  * Last modification: $DateTime: 2008/11/28 16:26:01 $
  * Last modified by:  $Author: builder $
  *
  */
#pragma once

#include <string>

#include "core/message/src/NameValuePair.h"
#include "core/message/types/MessageTypes.h"

namespace TA_Base_Core
{
    class AuditMessageSender
    {
    public:
        /// Verifies that the input message type is the last message that was
        /// sent by the audit message sender
        static bool verifyLastAuditMessage(const type_info& type_info);
        /// Clears internal buffer of audit messages 
        /// (verifyLastAuditMessage will then return false)
        static void clearAuditMessages();
        
        std::string sendAuditMessage( const MessageType& messageType,
							   unsigned long entityKey,
							   const DescriptionParameters& description,
							   const std::string& details,
							   const std::string& sessionID,
							   const std::string& alarmID,
							   const std::string& incidentID,
							   const std::string& parentEventID,
							   const FilterData* filterableData = NULL,
                               const time_t& timeStamp = 0);
    private:

        static std::string s_lastAuditMessageType;
    };
}