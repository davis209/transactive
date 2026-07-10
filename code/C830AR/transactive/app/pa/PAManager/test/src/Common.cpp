/**
  * The source code in this file is the property of
  * Ripple Systems and is not for redistribution
  * in any form.
  *
  * Source:   $File: //depot/4669_T01271350/4669/transactive/app/pa/PAManager/test/src/Common.cpp $
  * @author:  Ripple
  * @version: $Revision: #1 $
  *
  * Last modification: $DateTime: 2008/11/28 16:26:01 $
  * Last modified by:  $Author: builder $
  *
  */
#include "StdAfx.h"

#include "core/data_access_interface/src/PrimitiveWrapper.h"
#include "app/pa/PAManager/src/CachedConfig.h"
#include "core/utilities/src/RunParams.h"


namespace TA_IRS_App
{
    namespace PaTest
    {
        TA_Base_Core::PrimitiveWrapper<CachedConfig::ELocationType> g_locationSavePoint;
        TA_Base_Core::PrimitiveWrapper<unsigned long> g_locationKeySavePoint;

        void setToValidLocation()
        {
            if (!g_locationSavePoint.hasBeenSet())
            {
                try
                {
                    g_locationSavePoint.setValue(CachedConfig::getInstance()->getLocationType());
                }
                catch (...)
                {
                    g_locationSavePoint.setValue(CachedConfig::LOCATION_TYPE_INVALID);
                }

                try
                {
                    g_locationKeySavePoint.setValue(CachedConfig::getInstance()->getEntityLocationKey());
                }
                catch (...)
                {
                    g_locationKeySavePoint.setValue(0);
                }                
            }

            CachedConfig::getInstance()->setLocationType(CachedConfig::LOCATION_TYPE_OCC);   
            CachedConfig::getInstance()->setEntityLocationKey(1);
            
            // We may leave these parameter as set
            TA_Base_Core::RunParams::getInstance().set("LocationKey", "1");
            TA_Base_Core::RunParams::getInstance().set("NotifyHosts", "utopia:2323,utopia:2324");
            TA_Base_Core::RunParams::getInstance().set("DbConnection", "tra_int:it160_4:it160_4");

            // This is required to disable rights checking..
            TA_Base_Core::RunParams::getInstance().set("SessionId", "debug");
            
            static bool corbaNameSet = false;
            if (!corbaNameSet)
            {
                // Apply a dummy name to avoid tripping an assert when attempting to resolve the agent
                TA_Base_Core::CorbaName corbaName("AgentName", "ObjectName");
                CachedConfig::getInstance()->getAgentObject().setCorbaName(corbaName);            

                corbaNameSet = true;
            }
        }

        void restoreLocation()
        {
            // Restore state of cached config - if a valid save point exists
            if (g_locationSavePoint.hasBeenSet())
            {
                try
                {
                    CachedConfig::getInstance()->setLocationType(g_locationSavePoint.getValue());
                }
                catch (...) {}

                g_locationSavePoint.resetValue();
            }

            if (g_locationKeySavePoint.hasBeenSet())
            {
                try
                {
                    CachedConfig::getInstance()->setEntityLocationKey(g_locationKeySavePoint.getValue());
                }
                catch (...) {}

                g_locationKeySavePoint.resetValue();
            }
        }

    }   // end PaTest
}   // end TA_App