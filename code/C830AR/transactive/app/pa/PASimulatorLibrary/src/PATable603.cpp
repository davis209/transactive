/**
  * The source code in this file is the property of
  * Ripple Systems and is not for redistribution
  * in any form.
  *
  * Source:   $File: //depot/4669_T01271350/4669/transactive/app/pa/PASimulatorLibrary/src/PATable603.cpp $
  * @author:  Robert van Hugten
  * @version: $Revision: #1 $
  *
  * Last modification: $DateTime: 2008/11/28 16:26:01 $
  * Last modified by:  $Author: builder $
  *
  *
  * Definition file for class PATable603.
  */
#include "app/pa/PASimulatorLibrary/src/PATable603.h"
#include "app/pa/PASimulatorLibrary/src/PASimUtility.h"

using namespace TA_App;

const unsigned char PATable603::no_message = 'X';
const unsigned char PATable603::emergency_message = 'E';
const unsigned char PATable603::normal_message = 'N';

PATable603::PATable603(PATableRequestProcessor * parent)
: PASimTable(603,parent)
{
    size_t sz = sizeof(Table603DataArray);
    unsigned int dfi = 0; // DataFiller index
    size_t dfs = PASimUtility::DataFiller.size();

    for ( int bi=0 ; bi<NBPREREC ; bi++ )
    {
        // reset the DataFiller index, if necessary
        if (dfi >= dfs)
        {
            dfi=0;
        }

        for ( int li=0 ; li<40 ; li++ )
        {
            m_tableArray[bi].label[li] = PASimUtility::DataFiller[dfi];
        }
        m_tableArray[bi].type='z';

        dfi++;
    }

    Byte * bp = (Byte *)m_tableArray;
    PASimTable::setTableData(bp,sz,0);
}


PATable603::~PATable603()
{ }



UserReply * PATable603::processUserRead(UserRequest * ur)
{
    return PASimTable::processUserRead(ur);
}


UserReply * PATable603::processUserWrite(UserRequest * ur)
{
	//
	// NOTE: The following direct use of strtok is considered by the author
	// to be a disgusting hack.
	// 
	// A slightly better solution is to put the ugliest stuff into PASimUtility
	// in a similar fashion to ATSSimUtility::tokenise() and detokenise() in
	//	   ../../../ats/ATSSimulatorLibrary/src/ATSSimUtility.h & cpp
	//
	// then use as per ATSSimulator::execShowTable et. al.
	//
	// It doesn't *look* any tidier but it is more reliable and flexible
	//
    if (ur->m_requestType == 'W')
    {
        UserReply * reply = new UserReply;
        reply->m_replyType = 'W';
        reply->m_tableNumber = getTableNumber();

        unsigned char label[256];
        char numstr[16];
        char wholestring[256];
        strncpy(reinterpret_cast<char *>(wholestring),ur->m_requestInfo.c_str(),255);
        wholestring[255] = 0;

        char * token = strtok(wholestring," ");
        
        unsigned int messageNumber = atoi(token);
        if ( (messageNumber>(NBPREREC+6)) || (messageNumber<7) )
        {
            sprintf(numstr,"%u",(NBPREREC+6));
            reply->m_replyData="ERROR: message number must be between 7 and ";
            (reply->m_replyData).append(numstr);
            return reply;
        }

        token = strtok(0," ");
        unsigned char type=token[0];

        if ( (type!='X') && (type!='E') && (type!='N') )
        {
            reply->m_replyData="ERROR: type must be one of [X|E|N]";
            return reply;
        }

        token = strtok(0,"!");
        if (strlen(token) > 1)
        {
            strncpy(reinterpret_cast<char *>(m_tableArray[messageNumber-7].label),reinterpret_cast<const char *>(token),40);
            m_tableArray[messageNumber-7].type=type;
            
            Byte * bp = (Byte *)m_tableArray;
            size_t sz = sizeof(Table603DataArray);
            PASimTable::setTableData(bp,sz,0);
            
            notifyDependents(user_modification);

            reply->m_replyData = "Table updated!";
            return reply;
        }


        else
        {
            reply->m_replyData="ERROR: incorrect number of parameters: <message#> <type> <label>";
            return reply;
        }
    }

    return PASimTable::processUserWrite(ur);
}

