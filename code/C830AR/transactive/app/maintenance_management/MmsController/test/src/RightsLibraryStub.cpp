/**
  * The source code in this file is the property of
  * Ripple Systems and is not for redistribution
  * in any form.
  *
  * Source:   $File: //depot/4669_T01271350/4669/transactive/app/maintenance_management/MmsController/test/src/RightsLibraryStub.cpp $
  * @author:  Ripple
  * @version: $Revision: #1 $
  *
  * Last modification: $DateTime: 2008/11/28 16:26:01 $
  * Last modified by:  $Author: builder $
  *
  */
#include "RightsLibraryStub.h"

namespace TA_Base_Bus
{
    std::map<unsigned long, bool> RightsLibrary::s_overridenRights;

    bool RightsLibrary::isActionPermittedOnResource(
					const std::string& sessionId, unsigned long resourceKey,
					unsigned long actionKey, std::string& reasonString,
					EDecisionModule& decisionModule )
    {
        std::map<unsigned long, bool>::iterator itr = s_overridenRights.find(actionKey);

        if (s_overridenRights.end() == itr)
        {
            // Return true by default (when no override set)
            return true;
        }
        else
        {
            return itr->second;
        }
    }
}
