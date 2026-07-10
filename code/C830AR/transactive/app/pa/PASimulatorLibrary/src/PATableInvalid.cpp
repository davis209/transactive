/**
  * The source code in this file is the property of
  * Ripple Systems and is not for redistribution
  * in any form.
  *
  * Source:   $File: //depot/4669_T01271350/4669/transactive/app/pa/PASimulatorLibrary/src/PATableInvalid.cpp $
  * @author:  Robert van Hugten
  * @version: $Revision: #1 $
  *
  * Last modification: $DateTime: 2008/11/28 16:26:01 $
  * Last modified by:  $Author: builder $
  *
  *
  * Definition file for class PATableInvalid.
  */
#include "app/pa/PASimulatorLibrary/src/PATableInvalid.h"

using namespace TA_App;

PATableInvalid::PATableInvalid(PATableRequestProcessor * parent)
: PASimTable(32222,parent)
{
}


PATableInvalid::~PATableInvalid()
{
}


UserReply * PATableInvalid::processUserRead(UserRequest * ur)
{
    return 0;
}


UserReply * PATableInvalid::processUserWrite(UserRequest * ur)
{
    return 0;
}


