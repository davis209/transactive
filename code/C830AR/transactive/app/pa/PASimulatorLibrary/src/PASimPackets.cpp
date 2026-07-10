/**
  * The source code in this file is the property of
  * Ripple Systems and is not for redistribution
  * in any form.
  *
  * Source:   $File: //depot/4669_T01271350/4669/transactive/app/pa/PASimulatorLibrary/src/PASimPackets.cpp $
  * @author:  Robert van Hugten
  * @version: $Revision: #1 $
  *
  * Last modification: $DateTime: 2008/11/28 16:26:01 $
  * Last modified by:  $Author: builder $
  *
  *
  * Definition file for structs in PASimTypes.h
  */

// for ntohs,ntohl,htonl,htons
#if !defined( WIN32 )
#include <arpa/inet.h>
#else
#include <winsock.h>
#endif // !WIN32

#include "app/pa/PASimulatorLibrary/src/PASimPackets.h"

#include <algorithm>
#include <iostream>

using namespace TA_App;

ISCSPacket::ISCSPacket(unsigned char type)
: m_requestType(type)
, m_tableNumber(0)
, m_tableSize(0)
, m_returnValue(PAS_ERR_OK)
, m_tableData(0)
{
}

ISCSPacket::~ISCSPacket()
{
}

ISCSRequest::ISCSRequest(unsigned char type)
:ISCSPacket(type)
{
}

ISCSReadRequest::ISCSReadRequest()
:ISCSRequest('R')
{
}

union TheseTwoBytes
{
    unsigned char two_uchars[2];
    unsigned short one_ushort;
};

void ISCSReadRequest::fromNetwork(std::vector<unsigned char>::iterator & iter)
{
    // UNUSED variable
    // unsigned char type = *iter;

    // 
    // NOTE
    // On Solaris, the following code snippet causes a core dump on the
    // second line:
    // 
    //     unsigned short * networkShortP = (unsigned short *)(&(iter[1]));
    //     unsigned short networkShort = *networkShortP;
    //
    // as does replacing the first line with the following.
    //
    //     unsigned short * networkShortP = (unsigned short *)((void *)((&(*iter))+1));
    //
    // There does not appear to be any good reason for such.  Even gdb is able
    // to "print *networkShortP" correctly in both cases, and dumping memory
    // with "x/2b networkShortP" is also good.
    //
    // Our fix is to use a union hack, which ought to do the same thing, just
    // one byte at a time.
    //
    TheseTwoBytes r_two_bytes;
    r_two_bytes.two_uchars[0] = iter[1];
    r_two_bytes.two_uchars[1] = iter[2];

    unsigned short networkShort = r_two_bytes.one_ushort; 
    m_tableNumber = ntohs(networkShort);

    iter+=SIZEOF_ReadRequestPacket;
}

ISCSWriteRequest::ISCSWriteRequest()
:ISCSRequest('W')
{
}

void ISCSWriteRequest::fromNetwork(std::vector<unsigned char>::iterator & iter)
{
    // Unused Variable
    // unsigned char type = *iter;

    //
    // See the NOTE in ISCSReadRequest::fromNetwork for why the roundabout
    // approach is required for accessing stuff from the iterator.
    TheseTwoBytes w_two_bytes;

    w_two_bytes.two_uchars[0] = iter[1];
    w_two_bytes.two_uchars[1] = iter[2];

    unsigned short networkShort = w_two_bytes.one_ushort; 
    // table number
    m_tableNumber = ntohs(networkShort);

    w_two_bytes.two_uchars[0] = iter[3];
    w_two_bytes.two_uchars[1] = iter[4];
    networkShort = w_two_bytes.one_ushort; 
    // table size
    m_tableSize = ntohs(networkShort);

    // must ensure just enough storage in the table
    m_tableData.resize(m_tableSize);
    std::copy(iter+5,iter+m_tableSize+5,m_tableData.begin());
    // unsigned char * data = iter+5;
    // memcpy(&(m_tableData[0]),data,m_tableSize);

    iter += (m_tableSize+5);
}

ISCSReply::ISCSReply(unsigned char type)
:ISCSPacket(type)
{
}

ISCSReadReply::ISCSReadReply()
:ISCSReply('R')
{
}

void ISCSReadReply::toNetwork(std::vector<unsigned char> & packet)
{
    packet.push_back(m_requestType);
    unsigned short networkShort = htons(m_tableNumber);
    unsigned char * pktShrtPtr = (unsigned char *)((void *)(&networkShort));
    packet.push_back(*pktShrtPtr);
    packet.push_back(*(pktShrtPtr+1));

    networkShort=htons(m_tableSize);
    pktShrtPtr = (unsigned char *)((void *)(&networkShort));
    packet.push_back(*pktShrtPtr);
    packet.push_back(*(pktShrtPtr+1));

    for ( ByteVector::iterator iter = m_tableData.begin() ;
          iter != m_tableData.end() ;
          iter++
        )
    {
        packet.push_back(*iter);
    }
}


ISCSWriteReply::ISCSWriteReply()
:ISCSReply('W')
{
}

void ISCSWriteReply::toNetwork(std::vector<unsigned char> & packet)
{
    packet.push_back(m_requestType);
    unsigned short networkShort = htons(m_tableNumber);
    unsigned char * pktShrtPtr = (unsigned char *)((void *)(&networkShort));
    packet.push_back(*pktShrtPtr);
    packet.push_back(*(pktShrtPtr+1));

    packet.push_back(m_returnValue);
}


