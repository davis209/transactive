/**
  * The source code in this file is the property of
  * Ripple Systems and is not for redistribution
  * in any form.
  *
  * Source:   $File: //depot/4669_T01271350/4669/transactive/app/pa/PAManager/test/src/Common.h $
  * @author:  Ripple
  * @version: $Revision: #1 $
  *
  * Last modification: $DateTime: 2008/11/28 16:26:01 $
  * Last modified by:  $Author: builder $
  *
  */
namespace TA_App
{
    namespace PaTest
    {

        // Not supposed to call twice consecutively, but if it is, the save point
        // (as used in restoreLocation) stays at the value it's first set to
        void setToValidLocation();

        // Called to restore location that was present before calling setToValidLocation
        void restoreLocation();

    }   // end PaTest
}   // end TA_App