/**
  * The source code in this file is the property of
  * Ripple Systems and is not for redistribution
  * in any form.
  *
  * Source:   $File: //depot/4669_T01271350/4669/transactive/app/pa/PASimulatorLibrary/src/PATable590.cpp $
  * @author:  Robert van Hugten
  * @version: $Revision: #1 $
  *
  * Last modification: $DateTime: 2008/11/28 16:26:01 $
  * Last modified by:  $Author: builder $
  *
  *
  * Definition file for class PATable590.
  */
#include "app/pa/PASimulatorLibrary/src/PATable590.h"
#include "app/pa/PASimulatorLibrary/src/PASimUtility.h"

using namespace TA_App;

PATable590::PATable590(PATableRequestProcessor * parent)
: PASimTable(590,parent)
{
    m_tableShort=0;
    Byte ba[2]; ba[0]=0; ba[1]=0;
    Byte * bp = ba;
    PASimTable::setTableData(ba,2,0);
}


PATable590::~PATable590()
{
}



UserReply * PATable590::processUserRead(UserRequest * ur)
{
    return PASimTable::processUserRead(ur);
}


UserReply * PATable590::processUserWrite(UserRequest * ur)
{
    return PASimTable::processUserWrite(ur);
}

