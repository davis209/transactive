/**
  * The source code in this file is the property of
  * Ripple Systems and is not for redistribution
  * in any form.
  *
  * Source:   $File: //depot/4669_T01271350/4669/transactive/app/maintenance_management/MmsController/test/src/MessagePublicationManagerStub.h $
  * @author:  Ripple
  * @version: $Revision: #1 $
  *
  * Last modification: $DateTime: 2008/11/28 16:26:01 $
  * Last modified by:  $Author: builder $
  *
  */
#pragma once

#include "AuditMessageSenderStub.h"
#include "core/message/types/MmsAudit_MessageTypes.h"

namespace TA_Base_Core
{
    class MessagePublicationManager
    {
    public:

        // Add the dummy parameter to stop multiply defined symbol linker error
        static MessagePublicationManager& getInstance(int = 0);
        
        AuditMessageSender* getAuditMessageSender(const MessageContext& messageContext, int = 0);
    };
}
