/**
  * The source code in this file is the property of
  * Ripple Systems and is not for redistribution
  * in any form.
  *
  * Source:   $File: //depot/4669_T01271350/4669/transactive/app/pa/PASimulatorLibrary/src/PATable1200.cpp $
  * @author:  Robert van Hugten
  * @version: $Revision: #1 $
  *
  * Last modification: $DateTime: 2008/11/28 16:26:01 $
  * Last modified by:  $Author: builder $
  *
  *
  * Definition file for class PATable1200.
  */
#include "app/pa/PASimulatorLibrary/src/PATable1200.h"
#include "app/pa/PASimulatorLibrary/src/PASimUtility.h"

using namespace TA_App;

PATable1200::PATable1200(PATableRequestProcessor * parent)
: PASimTable(1200,parent)
{
    Byte * bp = (Byte *)m_tableArray;
    size_t sz = sizeof(Table1200DataArray);

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


PATable1200::~PATable1200()
{
}



UserReply * PATable1200::processUserRead(UserRequest * ur)
{
    return PASimTable::processUserRead(ur);
}


UserReply * PATable1200::processUserWrite(UserRequest * ur)
{
    if (ur->m_requestType == 'U')
    {
        UserReply * reply = new UserReply;
        reply->m_replyType = 'U';
        reply->m_tableNumber = getTableNumber();


        Byte * bp = (Byte *)m_tableArray;
        size_t sz = sizeof(Table1200DataArray);

        for ( unsigned int bi=0 ; bi<sz ; bi++ )
        {
            bp[bi] = rand();
        }

        PASimTable::setTableData(bp,sz,0);
        
        reply->m_replyData = "Updated Table";

        notifyDependents(user_modification);

        return reply;
    }

    return PASimTable::processUserWrite(ur);
}

