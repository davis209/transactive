/**
  * The source code in this file is the property of
  * Ripple Systems and is not for redistribution
  * in any form.
  *
  * Source:   $File: //depot/4669_T01271350/4669/transactive/app/maintenance_management/MmsController/test/src/RightsLibraryFactoryStub.h $
  * @author:  Ripple
  * @version: $Revision: #1 $
  *
  * Last modification: $DateTime: 2008/11/28 16:26:01 $
  * Last modified by:  $Author: builder $
  *
  */
#pragma once

#include "RightsLibraryStub.h"
#include "core\data_access_interface\src\IResource.h"
#include "core\data_access_interface\src\ResourceAccessFactory.h"

namespace TA_Base_Bus
{
    class RightsLibraryFactory
    {
    public:
   
        RightsLibrary* buildRightsLibrary();
    };
}