/**
  * The source code in this file is the property of
  * Ripple Systems and is not for redistribution
  * in any form.
  *
  * Source:   $File: //depot/4669_T01271350/4669/transactive/app/pa/PASimulatorLibrary/src/PATable570.cpp $
  * @author:  Robert van Hugten
  * @version: $Revision: #1 $
  *
  * Last modification: $DateTime: 2008/11/28 16:26:01 $
  * Last modified by:  $Author: builder $
  *
  *
  * Definition file for class PATable570.
  */
#include "app/pa/PASimulatorLibrary/src/PATable570.h"
#include "app/pa/PASimulatorLibrary/src/PASimUtility.h"

using namespace TA_App;

PATable570::PATable570(PATableRequestProcessor * parent)
: PASimTable(570,parent)
, m_tableByte(0)
{
    PASimTable::setTableData(&m_tableByte,1,0);
}


PATable570::~PATable570()
{
}



UserReply * PATable570::processUserRead(UserRequest * ur)
{
    return PASimTable::processUserRead(ur);
}


UserReply * PATable570::processUserWrite(UserRequest * ur)
{
    return PASimTable::processUserWrite(ur);
}

