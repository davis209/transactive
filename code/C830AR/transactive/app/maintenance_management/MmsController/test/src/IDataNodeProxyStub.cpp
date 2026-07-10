/**
  * The source code in this file is the property of
  * Ripple Systems and is not for redistribution
  * in any form.
  *
  * Source:   $File: //depot/4669_T01271350/4669/transactive/app/maintenance_management/MmsController/test/src/IDataNodeProxyStub.cpp $
  * @author:  Ripple
  * @version: $Revision: #1 $
  *
  * Last modification: $DateTime: 2008/11/28 16:26:01 $
  * Last modified by:  $Author: builder $
  *
  */
#pragma warning ( disable : 4786 )

#include "IDataNodeProxyStub.h"
#include "core\naming\src\NamedObject.h"

namespace  TA_Base_Bus
{
    // Inhibit state is false by default
    const static bool sg_defaultProxyInhibit = false;
   
    IDataNodeProxy::InhibitMap  IDataNodeProxy::s_inhibitMap;
    bool    IDataNodeProxy::s_throwExceptionOnNextSet(false);
    bool    IDataNodeProxy::s_throwExceptionOnNextGet(false);

    IDataNodeProxy::IDataNodeProxy(const TA_Base_Core::CorbaName& corbaName)
    :
    m_corbaObjectName(corbaName.getObjectName())
    {
    }


    bool IDataNodeProxy::getInhibitMms()
    {
        return getInhibitState(m_corbaObjectName);        
    }


    void IDataNodeProxy::setInhibitMms(bool inhibit, const std::string& sessionId)
    {
        setInhibitState(m_corbaObjectName, inhibit);
    }

       
    void IDataNodeProxy::setInhibitState(const std::string entityName, bool inhibit)
    {
        if (s_throwExceptionOnNextSet)
        {
            s_throwExceptionOnNextSet = false;
            throw std::exception("Failed to set subsystem inhibit state");
        }

        // Update inhibit map entry
        s_inhibitMap[entityName] = inhibit;
    }


    bool IDataNodeProxy::getInhibitState(const std::string entityName)
    {
        if (s_throwExceptionOnNextGet)
        {
            s_throwExceptionOnNextGet = false;
            throw std::exception("Failed to obtain subsystem inhibit state");
        }

        // if not present in map, have sg_defaultProxyInhibit returned by default
        InhibitMap::iterator itr = s_inhibitMap.find(entityName);

        if (s_inhibitMap.end() == itr)
        {
            return sg_defaultProxyInhibit;
        }
        else
        {
            return itr->second;
        }
    }
}

