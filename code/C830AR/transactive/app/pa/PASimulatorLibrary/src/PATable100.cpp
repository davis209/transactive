/**
  * The source code in this file is the property of
  * Ripple Systems and is not for redistribution
  * in any form.
  *
  * Source:   $File: //depot/4669_T01271350/4669/transactive/app/pa/PASimulatorLibrary/src/PATable100.cpp $
  * @author:  Robert van Hugten
  * @version: $Revision: #1 $
  *
  * Last modification: $DateTime: 2008/11/28 16:26:01 $
  * Last modified by:  $Author: builder $
  *
  *
  * Definition file for class PATable100.
  */
#include "app/pa/PASimulatorLibrary/src/PATable100.h"
#include "app/pa/PASimulatorLibrary/src/PASimUtility.h"

using namespace TA_App;

PATable100::PATable100(PATableRequestProcessor * parent)
: PASimTable(100,parent)
{
    Byte * bp = (Byte *)m_tableArray;
    size_t sz = sizeof(Table100_3DDataArray);
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


PATable100::~PATable100()
{
}



UserReply * PATable100::processUserRead(UserRequest * ur)
{
    return PASimTable::processUserRead(ur);
}


UserReply * PATable100::processUserWrite(UserRequest * ur)
{
    if (ur->m_requestType == 'U')
    {
        UserReply * reply = new UserReply;
        reply->m_replyType = 'U';
        reply->m_tableNumber = getTableNumber();


        Byte * bp = (Byte *)m_tableArray;
        size_t sz = sizeof(Table100_3DDataArray);

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

