/**
  * The source code in this file is the property of
  * Ripple Systems and is not for redistribution
  * in any form.
  *
  * Source:   $File: //depot/4669_T01271350/4669/transactive/app/maintenance_management/MmsController/test/src/Common.h $
  * @author:  Ripple
  * @version: $Revision: #1 $
  *
  * Last modification: $DateTime: 2008/11/28 16:26:01 $
  * Last modified by:  $Author: builder $
  *
  */
namespace TA_IRS_App
{
    namespace MmsTest
    {
        void setToValidLocation();

        class CMmsTestWinApp : public CWinApp
        {
        public:
            CMmsTestWinApp();
        
            virtual int DoMessageBox( LPCTSTR lpszPrompt, UINT nType, UINT nIDPrompt );
        
            BOOL m_messageBoxDisplayed;
        };

        extern CMmsTestWinApp g_theTestApp;

    }   // end MmsTest
}   // end TA_IRS_App