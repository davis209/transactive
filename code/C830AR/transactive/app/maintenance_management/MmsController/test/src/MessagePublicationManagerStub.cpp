/**
  * The source code in this file is the property of
  * Ripple Systems and is not for redistribution
  * in any form.
  *
  * Source:   $File: //depot/4669_T01271350/4669/transactive/app/maintenance_management/MmsController/test/src/MessagePublicationManagerStub.cpp $
  * @author:  Ripple
  * @version: $Revision: #1 $
  *
  * Last modification: $DateTime: 2008/11/28 16:26:01 $
  * Last modified by:  $Author: builder $
  *
  */
#include "MessagePublicationManagerStub.h"

namespace TA_Base_Core
{
    MessagePublicationManager& MessagePublicationManager::getInstance(int) 
    { 
        static MessagePublicationManager sInstance; 
        return sInstance; 
    }
        
    AuditMessageSender* MessagePublicationManager::getAuditMessageSender(const MessageContext& messageContext, int)
    {
        return new AuditMessageSender();        
    }

}
