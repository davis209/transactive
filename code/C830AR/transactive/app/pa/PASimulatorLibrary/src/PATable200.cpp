/**
  * The source code in this file is the property of
  * Ripple Systems and is not for redistribution
  * in any form.
  *
  * Source:   $File: //depot/4669_T01271350/4669/transactive/app/pa/PASimulatorLibrary/src/PATable200.cpp $
  * @author:  Robert van Hugten
  * @version: $Revision: #1 $
  *
  * Last modification: $DateTime: 2008/11/28 16:26:01 $
  * Last modified by:  $Author: builder $
  *
  *
  * Definition file for class PATable200.
  */
#include "app/pa/PASimulatorLibrary/src/PATable200.h"
#include "app/pa/PASimulatorLibrary/src/PASimUtility.h"

using namespace TA_App;

PATable200::PATable200(PATableRequestProcessor * parent)
: PASimTable(200,parent)
{
    Byte * bp = (Byte *)m_tableArray;
    size_t sz = sizeof(Table200DataArray);

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


PATable200::~PATable200()
{
}



UserReply * PATable200::processUserRead(UserRequest * ur)
{
    return PASimTable::processUserRead(ur);
}


UserReply * PATable200::processUserWrite(UserRequest * ur)
{
    if (ur->m_requestType == 'U')
    {
        UserReply * reply = new UserReply;
        reply->m_replyType = 'U';
        reply->m_tableNumber = getTableNumber();


        Byte * bp = (Byte *)m_tableArray;
        size_t sz = sizeof(Table200DataArray);

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

