/**
  * The source code in this file is the property of
  * Ripple Systems and is not for redistribution
  * in any form.
  *
  * Source:   $File: //depot/4669_T01271350/4669/transactive/app/pa/PASimulatorLibrary/src/PATable610.cpp $
  * @author:  Robert van Hugten
  * @version: $Revision: #1 $
  *
  * Last modification: $DateTime: 2008/11/28 16:26:01 $
  * Last modified by:  $Author: builder $
  *
  *
  * Definition file for class PATable610.
  */
#include "app/pa/PASimulatorLibrary/src/PATable610.h"

using namespace TA_App;


PATable610::PATable610(PATableRequestProcessor * parent)
: PASimTable(610,parent)
{
    //
    // THis is fantastic fun, mainly intended as a guide to accessing the
    // m_tableArray structure
    for (int vidx=0 ; vidx<6 ; vidx++ )
    {
        for (int adhocidx=0 ; adhocidx<NBADHOC ; adhocidx++ )
        {
            m_tableArray.privateSet.adhoc[adhocidx][vidx] = 0;
            m_tableArray.commonSet.adhoc[adhocidx][vidx] = 0;
        }
        m_tableArray.privateSet.prerecorded[vidx] = 0;
        m_tableArray.commonSet.prerecorded[vidx] = 0;
    }

    Byte * bp = (Byte *)(m_tableArray.privateSet.adhoc[0]);
    PASimTable::setTableData(bp,sizeof(Table610DataArray),0);
}


PATable610::~PATable610()
{
}



UserReply * PATable610::processUserRead(UserRequest * ur)
{
    return PASimTable::processUserRead(ur);
}


UserReply * PATable610::processUserWrite(UserRequest * ur)
{
    return PASimTable::processUserWrite(ur);
}

void PATable610::setupDependents()
{
    dependUpon(630);
    dependUpon(1610);
}

void PATable610::providerChanged(PASimTable * t, ChangeReasonCode reasonCode)
{
    //
    // @todo 
}

