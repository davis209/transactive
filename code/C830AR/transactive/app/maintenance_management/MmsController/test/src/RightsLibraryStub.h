/**
  * The source code in this file is the property of
  * Ripple Systems and is not for redistribution
  * in any form.
  *
  * Source:   $File: //depot/4669_T01271350/4669/transactive/app/maintenance_management/MmsController/test/src/RightsLibraryStub.h $
  * @author:  Ripple
  * @version: $Revision: #1 $
  *
  * Last modification: $DateTime: 2008/11/28 16:26:01 $
  * Last modified by:  $Author: builder $
  *
  */
#pragma once

#include <string>
#include <map>
#include "RightsLibraryStub.h"

namespace TA_Base_Bus
{
	enum EDecisionModule
	{
		RIGHTS_DM,
		MUTEX_DM ,
		UNDEFINED_DM
	};

    class RightsLibrary
    {
    public:
        virtual bool isActionPermittedOnResource(
					const std::string& sessionId, unsigned long resourceKey,
					unsigned long actionKey, std::string& reasonString,
					EDecisionModule& decisionModule );

        static void disableRight(unsigned long right) { s_overridenRights[right] = false; }
        static void restoreRights() { s_overridenRights.clear(); }

    private:

        static std::map<unsigned long, bool> s_overridenRights;
    };
}