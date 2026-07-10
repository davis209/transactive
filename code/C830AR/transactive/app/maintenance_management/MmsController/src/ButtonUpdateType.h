/**
  * The source code in this file is the property of
  * Ripple Systems and is not for redistribution
  * in any form.
  *
  * Source:   $File: //depot/4669_T01271350/4669/transactive/app/maintenance_management/MmsController/src/ButtonUpdateType.h $
  * @author:  Ripple
  * @version: $Revision: #1 $
  *
  * Last modification: $DateTime: 2008/11/28 16:26:01 $
  * Last modified by:  $Author: builder $
  *
  */

#ifndef _BUTTONUPDATETYPES_H_
#define _BUTTONUPDATETYPES_H_

namespace TA_IRS_App
{	
	enum EButtonUpdateType
	{
		ALL,
        APPLY_BUTTON_ONLY,
		REMOVE_APPLY_BUTTONS_ONLY,
		ADD_APPLY_BUTTONS_ONLY,
		DISABLE_ADD_BUTTON,
		DISABLE_REMOVE_BUTTON,
		REMOVE_BUTTON_ONLY,
		ADD_BUTTON_ONLY
	};
	
}

#endif //_BUTTONUPDATETYPES_H_