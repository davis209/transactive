/**
  * The source code in this file is the property of
  * Ripple Systems and is not for redistribution
  * in any form.
  *
  * Source:   $File: //depot/4669_T01271350/4669/transactive/app/pa/PASimulatorLibrary/src/PATable560.cpp $
  * @author:  Robert van Hugten
  * @version: $Revision: #1 $
  *
  * Last modification: $DateTime: 2008/11/28 16:26:01 $
  * Last modified by:  $Author: builder $
  *
  *
  * Definition file for class PATable560.
  */
#include "app/pa/PASimulatorLibrary/src/PATable560.h"
#include "core/utilities/src/DebugUtil.h"
#include <stdexcept>
//
// for ntohs,ntohl,htonl,htons
#if !defined( WIN32 )
#include <arpa/inet.h>
#else
#include <winsock.h>
#endif // !WIN32

using namespace TA_App;

PATable560::PATable560(PATableRequestProcessor * parent)
: PASimTable(560,parent)
{
    size_t sz = sizeof(Table560DataUnion);

    for ( unsigned int bi=0 ; bi<sz ; bi++ )
    {
        m_dataUnion.ucArray[bi] = 0;
    }


    Byte * bp = (Byte *)(m_dataUnion.ucArray);
    PASimTable::setTableData(bp,sz,0);
}


PATable560::~PATable560()
{
}


void PATable560::flagWrite(int tableNumber)
{
    LOG_GENERIC(SourceInfo, TA_Core::DebugUtil::DebugDebug,
        "PATable560::flagWrite(%d)",tableNumber);

    switch (tableNumber)
    {
        case 100:
            setFlag(FLAG_100_A|FLAG_100_B);
            break;
        case 202:
            setFlag(FLAG_202_A|FLAG_202_B);
            break;
        case 204:
            setFlag(FLAG_204_A|FLAG_204_B);
            break;
        case 1204:
            setFlag(FLAG_1204_A|FLAG_1204_B);
            break;
        case 303:
            setFlag(FLAG_303_A|FLAG_303_B);
            break;
        case 1303:
            setFlag(FLAG_1303_A|FLAG_1303_B);
            break;
        case 550:
            setFlag(FLAG_550_A|FLAG_550_B);
            break;
        case 570:
            setFlag(FLAG_570_A|FLAG_570_B);
            break;
        case 590:
            setFlag(FLAG_590_A|FLAG_590_B);
            break;
        case 603:
            setFlag(FLAG_603_A|FLAG_603_B);
            break;
        case 604:
            setFlag(FLAG_604_A|FLAG_604_B);
            break;
        case 610:
            setFlag(FLAG_610_A|FLAG_610_B);
            break;
        case 1610:
            setFlag(FLAG_1610_A|FLAG_1610_B);
            break;
        default:
            throw std::logic_error("No flag for specified table in Table560");
            break;
    }


    Table560DataUnion localUnion;
    localUnion.longArray[0] = htonl(m_dataUnion.longArray[0]);
    localUnion.longArray[1] = htonl(m_dataUnion.longArray[1]);
    
    // Byte * bp = (Byte *)(m_dataUnion.ucArray);
    Byte * bp = (Byte *)(localUnion.ucArray);
    PASimTable::setTableData(bp,sizeof(Table560DataUnion),0);
}

/*
 * fortunately for us, only 10 tables' are flagged in the data, so that only
 * 20 bits are needing set, so the unsigned long fits them all.
 */
void PATable560::setFlag(unsigned long int flagPattern, unsigned long int highLongWord)
{
    m_dataUnion.longArray[0] |= flagPattern;
    m_dataUnion.longArray[1] |= highLongWord;
}

void PATable560::clearFlag(unsigned long int flagPattern, unsigned long int highLongWord)
{
    m_dataUnion.longArray[0] &= (~flagPattern);
    m_dataUnion.longArray[1] &= (~highLongWord);
}

UserReply * PATable560::processUserRead(UserRequest * ur)
{
    LOG_GENERIC(SourceInfo, TA_Core::DebugUtil::DebugDebug,
        "PATable560::processUserRead(%d)",ur->m_tableNumber);
    return PASimTable::processUserRead(ur);
}


UserReply * PATable560::processUserWrite(UserRequest * ur)
{
    LOG_GENERIC(SourceInfo, TA_Core::DebugUtil::DebugDebug,
        "PATable560::processUserWrite(%d)",ur->m_tableNumber);
    return PASimTable::processUserWrite(ur);
}


void PATable560::processWriteRequest(ISCSRequest * request)
{
    LOG_GENERIC(SourceInfo, TA_Core::DebugUtil::DebugDebug,
        "PATable560::processWriteRequest(%d)",request->m_tableNumber);
    //
    // initial sanity check
    if (sizeof(Table560DataUnion) != request->m_tableData.size())
    {
        //
        // These two variable declarations are superfluous, but don't really
        // cause too many extra machine instructions and are very handy for
        // debugging.
        size_t ours = sizeof(Table560DataUnion);
        size_t theirs = request->m_tableData.size();

        // std::cerr << "WARNING: received table data different size (ours="
        //           << ours << ", theirs=" << theirs << ")" << std::endl;
        LOG_GENERIC(SourceInfo, TA_Core::DebugUtil::DebugDebug,
                    "WARNING: received table data different size (ours=%d, theirs=%d)",ours,theirs);
    }

    //
    // much slipping through the arrived data, comparing the arrived data
    // with ours.  In the arrived data, a "01" should correspond to a "11"
    // in our data, which we should set to "00" to acknowledge that we've seen
    // the ISCS's acknowledgment of the table being flagged.
    //
    // Additionally, we jump up and down and move it all around if the pattern
    // found is inconsistent with our recollection of affairs.  There are many
    // ways that could occur:
    //
    //  Arrived Ours        Action
    //  -------+-----------+-------
    //  00      00          OK. Leave.
    //  00      11          Leave our flags set, ISCS should pick up soon.
    //  01      00          Error, send PAS_ERR_NO_FLAG (see SICD A.22.6)
    //  01      11          OK. clear flag as expected
    //  10      00          Error, send PAS_ERR_GENERIC
    //  10      11          Error, send PAS_ERR_GENERIC
    //  11      00          Error, send PAS_ERR_GENERIC
    //  11      11          OK. Leave flags set, ISCS should pick up soon.
    //  XX      01 10       Our error. Clear flag and carry on.
    //
    // ALSO NOTE: we only deal with the Least Significant 4 Bytes.
    //
    unsigned long * lp = (unsigned long *)(&((request->m_tableData)[4]));
    unsigned long arr_long = *lp; // which hopefully does a bitwise copy

    unsigned long our_long = m_dataUnion.longArray[1];
    unsigned long new_long = our_long;
    
    unsigned long flag_a = 0x80000000;
    unsigned long flag_b = 0x40000000;
    unsigned long flag_x = 0xC0000000;
    unsigned long mask   = 0xFFFFFFFF;

    unsigned long arrived = arr_long & flag_x;
    unsigned long ours    = our_long & flag_x;

    //
    // once the flag_x has been shifted past the right end of the long word,
    // it should be empty and hence zero.
    while ( flag_x > 0 )
    {
        if ((ours==flag_a) || (ours==flag_b))
        {
            // clearFlag(flag_b);
            mask= ~flag_x;
            new_long = new_long & mask;
        }
        else if (arrived==flag_b)
        {
            if (ours==0)
            {
                setWriteReturnValue(PAS_ERR_NO_FLAG);
            }
            else
            {
                // clearFlag(flag_b);
                mask= ~flag_x; // our_long flag still has both bits set
                new_long = new_long & mask;
            }
        }
        else if (arrived==flag_a)
        {
            setWriteReturnValue(PAS_ERR_GENERIC);
        }
        else if (arrived==flag_x)
        {
            if ( ours == 0 )
            {
                setWriteReturnValue(PAS_ERR_GENERIC);
            }
        }

        flag_a >>= 2;
        flag_b >>= 2;
        flag_x >>= 2;
        arrived = arr_long & flag_x;
        ours    = our_long & flag_x;
    }

    m_dataUnion.longArray[1] = new_long;

    //
    // the [super|base]class sends the write reply
    PASimTable::processWriteRequest(request);

    //
    // Still need to set the base data correctly, for read requests
    Table560DataUnion localUnion;
    localUnion.longArray[0] = htonl(m_dataUnion.longArray[0]);
    localUnion.longArray[1] = htonl(m_dataUnion.longArray[1]);
    
    // Byte * bp = (Byte *)(m_dataUnion.ucArray);
    Byte * bp = (Byte *)(localUnion.ucArray);
    PASimTable::setTableData(bp,sizeof(Table560DataUnion),0);
}

void PATable560::setupDependents()
{
    dependUpon(100);
    dependUpon(202);
    dependUpon(204);
    dependUpon(1204);
    dependUpon(303);
    dependUpon(1303);
    dependUpon(550);
    dependUpon(570);
    dependUpon(590);
    dependUpon(603);
    dependUpon(604);
    dependUpon(610);
    dependUpon(1610);
}

void PATable560::providerChanged(PASimTable * t, ChangeReasonCode reasonCode)
{
    //
    // we're only really interested in one reason.
    if (reasonCode == user_modification)
    {
        flagWrite(t->getTableNumber());
    }
}

