/**
  * The source code in this file is the property of
  * Ripple Systems and is not for redistribution
  * in any form.
  *
  * Source:   $File: //depot/4669_T01271350/4669/transactive/app/pa/PASimulatorLibrary/src/PATable604.cpp $
  * @author:  Robert van Hugten
  * @version: $Revision: #1 $
  *
  * Last modification: $DateTime: 2008/11/28 16:26:01 $
  * Last modified by:  $Author: builder $
  *
  *
  * Definition file for class PATable604.
  */
#include "app/pa/PASimulatorLibrary/src/PATable604.h"

using namespace TA_App;

const unsigned char PATable604::no_message = 'X';
const unsigned char PATable604::emergency_message = 'E';
const unsigned char PATable604::normal_message = 'N';

PATable604::PATable604(PATableRequestProcessor * parent)
: PASimTable(604,parent)
{
    for ( int bi=0 ; bi<NBADHOC ; bi++ )
    {
        m_tableArray[bi] = 'X';
    }

    Byte * bp = (Byte *)m_tableArray;
    PASimTable::setTableData(bp,NBADHOC,0);
}


PATable604::~PATable604()
{
}



UserReply * PATable604::processUserRead(UserRequest * ur)
{
    return PASimTable::processUserRead(ur);
}


UserReply * PATable604::processUserWrite(UserRequest * ur)
{
    return PASimTable::processUserWrite(ur);
}

