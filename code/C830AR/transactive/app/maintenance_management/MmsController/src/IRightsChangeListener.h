/**
  * The source code in this file is the property of 
  * Ripple Systems and is not for redistribution
  * in any form.
  *
  * Source: $File: //depot/4669_T01271350/4669/transactive/app/maintenance_management/MmsController/src/IRightsChangeListener.h $
  * @author Jade Lee
  * @version $Revision: #1 $
  *
  * Last modification: $DateTime: 2008/11/28 16:26:01 $
  * Last modified by: $Author: builder $
  * 
  * Simple interface for receipt of rights changed notifications
  *
  */

namespace TA_IRS_App
{

    interface IRightsChangeListener
    {
    public:
    
        /**
         * onRightsChanged
         *
         * Called whenever there is a change in rights.
         *
         */
        virtual void onRightsChanged() = 0;
    };

} // end namespace TA_IRS_App