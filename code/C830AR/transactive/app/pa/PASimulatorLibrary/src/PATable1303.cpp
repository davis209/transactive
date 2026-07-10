/**
  * The source code in this file is the property of
  * Ripple Systems and is not for redistribution
  * in any form.
  *
  * Source:   $File: //depot/4669_T01271350/4669/transactive/app/pa/PASimulatorLibrary/src/PATable1303.cpp $
  * @author:  Robert van Hugten
  * @version: $Revision: #1 $
  *
  * Last modification: $DateTime: 2008/11/28 16:26:01 $
  * Last modified by:  $Author: builder $
  *
  *
  * Definition file for class PATable1303.
  */
#include "app/pa/PASimulatorLibrary/src/PATable1303.h"
#include "app/pa/PASimulatorLibrary/src/PASimUtility.h"

using namespace TA_App;

PATable1303::PATable1303(PATableRequestProcessor * parent)
: PASimTable(1303,parent)
{
    Byte * bp = (Byte *)m_tableArray;
    size_t sz = sizeof(Table1303DataArray);

    unsigned int dfi = 0; // DataFiller index
    size_t dfs = PASimUtility::DataFiller.size();

    for ( unsigned int bi=0 ; bi<sz ; bi++,dfi++ )
    {
        // reset the DataFiller index, if necessary
        if (dfi >= dfs)
        {
            dfi=0;
        }

        bp[bi] = PASimUtility::DataFiller[dfi];
    }

    PASimTable::setTableData(bp,sz,0);
}


PATable1303::~PATable1303()
{
}



UserReply * PATable1303::processUserRead(UserRequest * ur)
{
    return PASimTable::processUserRead(ur);
}


UserReply * PATable1303::processUserWrite(UserRequest * ur)
{
    return PASimTable::processUserWrite(ur);
}

void PATable1303::setupDependents()
{
    dependUpon(303);
}

void PATable1303::providerChanged(PASimTable * t, ChangeReasonCode reasonCode)
{
    //
    // @todo 
}

