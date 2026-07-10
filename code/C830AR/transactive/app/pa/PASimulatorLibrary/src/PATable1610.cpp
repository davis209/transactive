/**
  * The source code in this file is the property of
  * Ripple Systems and is not for redistribution
  * in any form.
  *
  * Source:   $File: //depot/4669_T01271350/4669/transactive/app/pa/PASimulatorLibrary/src/PATable1610.cpp $
  * @author:  Robert van Hugten
  * @version: $Revision: #1 $
  *
  * Last modification: $DateTime: 2008/11/28 16:26:01 $
  * Last modified by:  $Author: builder $
  *
  *
  * Definition file for class PATable1610.
  */
#include "app/pa/PASimulatorLibrary/src/PATable1610.h"

using namespace TA_App;


PATable1610::PATable1610(PATableRequestProcessor * parent)
: PASimTable(1610,parent)
{
    //
    // THis is fantastic fun, mainly intended as a guide to accessing the
    // m_tableArray structure
    for (int stnidx=0 ; stnidx<MAXSTNID ; stnidx++)
    {
        //
        // for each byte in each 3-word (6-byte) element
        for (int vidx=0 ; vidx<6 ; vidx++ )
        {
            // 
            // for each adhoc dva message version id
            for (int adhocidx=0 ; adhocidx<NBADHOC ; adhocidx++ )
            {
                m_tableArray[stnidx].privateSet.adhoc[adhocidx][vidx] = 0;
                m_tableArray[stnidx].commonSet.adhoc[adhocidx][vidx] = 0;
            }
            m_tableArray[stnidx].privateSet.prerecorded[vidx] = 0;
            m_tableArray[stnidx].commonSet.prerecorded[vidx] = 0;
        }
    }

    Byte * bp = (Byte *)(m_tableArray[0].privateSet.adhoc[0]);
    PASimTable::setTableData(bp,sizeof(Table1610DataArray),0);
}


PATable1610::~PATable1610()
{
}



UserReply * PATable1610::processUserRead(UserRequest * ur)
{
    return PASimTable::processUserRead(ur);
}


UserReply * PATable1610::processUserWrite(UserRequest * ur)
{
    return 0;
}

void PATable1610::setupDependents()
{
    dependUpon(610);
}

void PATable1610::providerChanged(PASimTable * t, ChangeReasonCode reasonCode)
{
    //
    // @todo 
}

