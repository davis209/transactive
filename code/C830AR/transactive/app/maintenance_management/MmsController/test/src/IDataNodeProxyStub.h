/**
  * The source code in this file is the property of
  * Ripple Systems and is not for redistribution
  * in any form.
  *
  * Source:   $File: //depot/4669_T01271350/4669/transactive/app/maintenance_management/MmsController/test/src/IDataNodeProxyStub.h $
  * @author:  Ripple
  * @version: $Revision: #1 $
  *
  * Last modification: $DateTime: 2008/11/28 16:26:01 $
  * Last modified by:  $Author: builder $
  *
  */
#pragma once

#include <map>
#include <string>

namespace TA_Base_Core
{
    struct CorbaName;
}

namespace TA_Base_Bus
{
    class IDataNodeProxy
    {
    public:
        IDataNodeProxy(const TA_Base_Core::CorbaName& corbaName);

        virtual bool getInhibitMms();
        virtual void setInhibitMms(bool inhibit, const std::string& sessionId);
    
        static void setInhibitState(const std::string entityName, bool inhibit);
        static bool getInhibitState(const std::string entityName);
        
        static void setThrowExceptionOnNextSet() { s_throwExceptionOnNextSet = true; }
        static void setThrowExceptionOnNextGet() { s_throwExceptionOnNextGet = true; }
        
        std::string m_corbaObjectName;

        IDataNodeProxy* operator->() { return this; }

    private:

        // Map of CorbaName getObjectName strings to inhibited states
        typedef std::map<std::string, bool> InhibitMap;
        static InhibitMap s_inhibitMap;

        static bool    s_throwExceptionOnNextSet;
        static bool    s_throwExceptionOnNextGet;
    };

    typedef IDataNodeProxy DataNodeProxySmartPtr;
}
