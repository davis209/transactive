/**
  * The source code in this file is the property of
  * Ripple Systems and is not for redistribution
  * in any form.
  *
  * Source:   $File: //depot/4669_T01271350/4669/transactive/app/maintenance_management/MmsController/test/src/MmsInhibitAgentNamedObjectStub.h $
  * @author:  Ripple
  * @version: $Revision: #1 $
  *
  * Last modification: $DateTime: 2008/11/28 16:26:01 $
  * Last modified by:  $Author: builder $
  *
  */
#pragma once

#include <string>

namespace TA_IRS_App
{
    class MmsInhibitAgentNamedObject
    {
    public:
        MmsInhibitAgentNamedObject();

        MmsInhibitAgentNamedObject* operator->() { return this; }

        bool getGlobalMmsInhibit();
        void setGlobalMmsInhibit(bool inhibit, const std::string& sessionId);
        
        void setThrowExceptionOnNextGlobalGet();
        void setThrowExceptionOnNextGlobalSet();
      
        void setEntityName(const std::string& name) {}

    private:
        
        bool m_globalMmsInhibit;
        bool m_throwExceptionOnNextGlobalGet, m_throwExceptionOnNextGlobalSet;
    };

}