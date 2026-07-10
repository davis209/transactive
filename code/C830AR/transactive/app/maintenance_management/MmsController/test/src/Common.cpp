/**
  * The source code in this file is the property of
  * Ripple Systems and is not for redistribution
  * in any form.
  *
  * Source:   $File: //depot/4669_T01271350/4669/transactive/app/maintenance_management/MmsController/test/src/Common.cpp $
  * @author:  Ripple
  * @version: $Revision: #1 $
  *
  * Last modification: $DateTime: 2008/11/28 16:26:01 $
  * Last modified by:  $Author: builder $
  *
  */
#include "StdAfx.h"

#include "Common.h"
#include "app\maintenance_management\MmsController\src\CachedConfig.h"
#include "core/data_access_interface/src/PrimitiveWrapper.h"
#include "core/utilities/src/RunParams.h"

namespace TA_IRS_App
{
    namespace MmsTest
    {    
        CMmsTestWinApp g_theTestApp;

        void setToValidLocation()
        {            
            // We may leave these parameters as set
            TA_Base_Core::RunParams::getInstance().set("LocationKey", "1");
            TA_Base_Core::RunParams::getInstance().set("NotifyHosts", "utopia:2323,utopia:2324");
            TA_Base_Core::RunParams::getInstance().set("DbConnection", "tra_int:it160_4:it160_4");

            CachedConfig::getInstance().setEntityLocationKey(1);
            CachedConfig::getInstance().setEntityKey(1);

            // This is required to disable rights checking..
            TA_Base_Core::RunParams::getInstance().set("SessionId", "debug");            
            TA_IRS_App::CachedConfig::getInstance().setSessionId("unittest_session");

            TA_IRS_App::CachedConfig::getInstance().setMmsInhibitAgentEntityName("OccMmsAgent");
            TA_IRS_App::CachedConfig::getInstance().setMmsInhibitAgentEntityKey(1);
        }


        CMmsTestWinApp::CMmsTestWinApp()
        :
        CWinApp("CMmsTestWinApp"),
        m_messageBoxDisplayed(FALSE)
        {
        }

        
        int CMmsTestWinApp::DoMessageBox( LPCTSTR lpszPrompt, UINT nType, UINT nIDPrompt )
        {           
            m_messageBoxDisplayed = TRUE;
            return IDOK;
        }

    }   // end MmsTest
}   // end TA_IRS_App