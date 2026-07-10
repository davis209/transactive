/**
  * The source code in this file is the property of
  * Ripple Systems and is not for redistribution
  * in any form.
  *
  * Source:   $File: //depot/4669_T01271350/4669/transactive/app/maintenance_management/MmsController/test/src/ScadaProxyFactoryStub.cpp $
  * @author:  Ripple
  * @version: $Revision: #1 $
  *
  * Last modification: $DateTime: 2008/11/28 16:26:01 $
  * Last modified by:  $Author: builder $
  *
  */
#pragma warning ( disable : 4786 )

#include "ScadaProxyFactoryStub.h"
#include "IDataNodeProxyStub.h"

namespace  TA_Base_Bus
{
    ScadaProxyFactory* ScadaProxyFactory::s_factory = 0;

    ScadaProxyFactory& ScadaProxyFactory::getInstance()
    {
        if (0 == s_factory)
        {
            s_factory = new ScadaProxyFactory();
        }
        return *s_factory;
    }


    void ScadaProxyFactory::destroyInstance()
    {
        if (0 != s_factory)
        {
            delete s_factory;
            s_factory = 0;
        }
    }


    ScadaProxyFactory::~ScadaProxyFactory()
    {
    }


    DataNodeProxySmartPtr ScadaProxyFactory::createDataNodeProxy(const TA_Base_Core::CorbaName& corbaName, TA_Base_Bus::IEntityUpdateEventProcessor& ue)
    {
        IDataNodeProxy newProxy(corbaName);            
        return newProxy;
    }
}
