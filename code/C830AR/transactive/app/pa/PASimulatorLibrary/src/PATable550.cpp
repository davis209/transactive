/**
  * The source code in this file is the property of
  * Ripple Systems and is not for redistribution
  * in any form.
  *
  * Source:   $File: //depot/4669_T01271350/4669/transactive/app/pa/PASimulatorLibrary/src/PATable550.cpp $
  * @author:  Robert van Hugten
  * @version: $Revision: #1 $
  *
  * Last modification: $DateTime: 2008/11/28 16:26:01 $
  * Last modified by:  $Author: builder $
  *
  *
  * Definition file for class PATable550.
  */
#include "app/pa/PASimulatorLibrary/src/PATable550.h"
#include "app/pa/PASimulatorLibrary/src/PASimUtility.h"

using namespace TA_App;

PATable550::PATable550(PATableRequestProcessor * parent)
: PASimTable(550,parent)
{
    Byte * bp = (Byte *)m_tableArray;
    size_t sz = sizeof(Table550DataArray);

    for ( unsigned int bi=0 ; bi<sz ; bi++ )
    {
        bp[bi] = 0;
    }

    PASimTable::setTableData(bp,sz,0);
}


PATable550::~PATable550()
{
}



UserReply * PATable550::processUserRead(UserRequest * ur)
{
    return PASimTable::processUserRead(ur);
}


UserReply * PATable550::processUserWrite(UserRequest * ur)
{
    return PASimTable::processUserWrite(ur);
}

