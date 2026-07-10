/**
  * The source code in this file is the property of
  * Ripple Systems and is not for redistribution
  * in any form.
  *
  * Source:   $File: //depot/4669_T01271350/4669/transactive/app/maintenance_management/MmsController/test/src/MmsInhibitAgentNamedObjectStub.cpp $
  * @author:  Ripple
  * @version: $Revision: #1 $
  *
  * Last modification: $DateTime: 2008/11/28 16:26:01 $
  * Last modified by:  $Author: builder $
  *
  */
#include "MmsInhibitAgentNamedObjectStub.h"

namespace TA_IRS_App
{
    MmsInhibitAgentNamedObject::MmsInhibitAgentNamedObject()
    :
    m_throwExceptionOnNextGlobalGet(false),
    m_globalMmsInhibit(false)
    {
    }
    

    void MmsInhibitAgentNamedObject::setThrowExceptionOnNextGlobalGet()
    {
        m_throwExceptionOnNextGlobalGet = true;
    }


    void MmsInhibitAgentNamedObject::setThrowExceptionOnNextGlobalSet()
    {
        m_throwExceptionOnNextGlobalSet = true;
    }


    bool MmsInhibitAgentNamedObject::getGlobalMmsInhibit()
    {
        if (m_throwExceptionOnNextGlobalGet)
        {
            m_throwExceptionOnNextGlobalGet = false;
            throw std::exception("Failure to retrieve MMS Inhibit");
        }
        return m_globalMmsInhibit;
    }


    void MmsInhibitAgentNamedObject::setGlobalMmsInhibit(bool inhibit, const std::string& sessionId)
    {
        if (m_throwExceptionOnNextGlobalSet)
        {
            m_throwExceptionOnNextGlobalSet = false;
            throw std::exception("Failure to set MMS Inhibit");
        }

        m_globalMmsInhibit = inhibit;
    }

}