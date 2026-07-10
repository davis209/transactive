/**
  * The source code in this file is the property of
  * Ripple Systems and is not for redistribution
  * in any form.
  *
  * Source:   $File: //depot/4669_T01271350/4669/transactive/app/pa/PASimulatorLibrary/src/PATable202.cpp $
  * @author:  Robert van Hugten
  * @version: $Revision: #1 $
  *
  * Last modification: $DateTime: 2008/11/28 16:26:01 $
  * Last modified by:  $Author: builder $
  *
  *
  * Definition file for class PATable202.
  */
#include "app/pa/PASimulatorLibrary/src/PATable202.h"
#include "app/pa/PASimulatorLibrary/src/PASimUtility.h"

using namespace TA_App;

PATable202::PATable202(PATableRequestProcessor * parent)
: PASimTable(202,parent)
{
    Byte * bp = (Byte *)m_tableArray;
    size_t sz = sizeof(Table202DataArray);

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


PATable202::~PATable202()
{
}



UserReply * PATable202::processUserRead(UserRequest * ur)
{
    return PASimTable::processUserRead(ur);
}


UserReply * PATable202::processUserWrite(UserRequest * ur)
{
    return PASimTable::processUserWrite(ur);
}

void PATable202::setupDependents()
{
    dependUpon(201);
}

void PATable202::providerChanged(PASimTable * t, ChangeReasonCode reasonCode)
{
    //
    // @todo 
}

