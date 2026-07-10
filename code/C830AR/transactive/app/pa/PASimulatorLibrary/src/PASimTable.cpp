/**
  * The source code in this file is the property of 
  * Ripple Systems and is not for redistribution
  * in any form.
  *
  * Source:   $File: //depot/4669_T01271350/4669/transactive/app/pa/PASimulatorLibrary/src/PASimTable.cpp $
  * @author:  Robert van Hugten
  * @version: $Revision: #1 $
  *
  * Last modification: $DateTime: 2008/11/28 16:26:01 $
  * Last modified by:  $Author: builder $
  * 
  * Definition file for PASimTable class
  */
#ifdef WIN32
#pragma warning (disable : 4786)
#endif // #ifdef WIN32

#include "app/pa/PASimulatorLibrary/src/PASimTable.h"
#include "app/pa/PASimulatorLibrary/src/PASimUtility.h"
#include "app/pa/PASimulatorLibrary/src/PATableRequestProcessor.h"
#include <stdexcept>
#include <iostream>

using namespace TA_App;

PASimTable::PASimTable(int tableNumber, PATableRequestProcessor * parent)
: m_tableNumber(tableNumber)
, m_parent(parent)
, m_dirty(false)
, m_writeReturnValue(PAS_ERR_OK)
// , m_dependents(0)
{
    parent->addTable(this);
}


PASimTable::~PASimTable()
{
    // all data members are by value, and the m_tableData stores bytes by
    // value, so there isn't any explicit destructing to be done here.
}


short PASimTable::getTableNumber() const
{
    return m_tableNumber;
}


int PASimTable::getTableSize() const
{
    return m_tableData.size();
}


PATableRequestProcessor * PASimTable::getParent() const
{
    return m_parent;
}

bool PASimTable::isDirty() const
{
    return m_dirty;
}


void PASimTable::processReadRequest(ISCSRequest * request)
{
    // Create and populate a Read Request Reply Packet
    //
    ISCSReadReply * reply = new ISCSReadReply;
    reply->m_tableNumber = getTableNumber();
    reply->m_tableSize = getTableSize();
    reply->m_tableData.resize(reply->m_tableSize);
    std::copy(m_tableData.begin(),m_tableData.end(),reply->m_tableData.begin());

    // send it.
    //
    m_parent->sendReadReply(reply);
}


void PASimTable::processWriteRequest(ISCSRequest * request)
{
    // assimilate the data.
    // 
    if (m_tableData.size() != request->m_tableData.size())
    {
        size_t ours = m_tableData.size();
        size_t theirs = request->m_tableData.size();

        std::cerr << "WARNING: received table data different size (ours="
                  << ours << ", theirs="
                  << theirs << ")" << std::endl;
        std::cerr << "RESIZING to match foreign data size" << std::endl;
        m_tableData.resize(request->m_tableData.size());
    }

    std::copy(request->m_tableData.begin(), request->m_tableData.end(), m_tableData.begin());
    
    // Create and populate a Write Request Reply Packet
    ISCSWriteReply * reply = new ISCSWriteReply;
    reply->m_tableNumber = getTableNumber();
    reply->m_returnValue = m_writeReturnValue;

    // reset the write reply return value
    m_writeReturnValue = PAS_ERR_OK;

    m_parent->sendWriteReply(reply);

    notifyDependents(iscs_write_request);
}


Byte PASimTable::getByte(unsigned int index) const
{
    if (index>m_tableData.size())
    {
        throw std::out_of_range("index out of range");
    }

    return m_tableData[index];
}


unsigned short PASimTable::getUShort(unsigned int index) const
{
    if ( index > (m_tableData.size()-1) )
    {
        throw std::out_of_range("index out of range");
    }

    const Byte * bval = &(m_tableData[index]);
    unsigned short * pval = (unsigned short *)((void *)bval);
    return *pval;
}


unsigned long PASimTable::getULong(unsigned int index) const
{
    if ( index > (m_tableData.size()-3) )
    {
        throw std::out_of_range("index out of range");
    }

    return 0;
}


const ByteVector & PASimTable::getByteVector() const
{
    return m_tableData;
}


void PASimTable::setByte(Byte value, unsigned int index)
{
    if (index>m_tableData.size())
    {
        throw std::out_of_range("index out of range");
    }

    m_tableData[index]=value;
    // throw std::logic_error("PASimTable::setTableData(Byte) not implemented");
}


void PASimTable::setUShort(unsigned short value, unsigned int index)
{
    if (index>m_tableData.size())
    {
        throw std::out_of_range("index out of range");
    }

	setByte(index,   (value>>8));
    setByte(index+1, (value&0x00ff));

    // throw std::logic_error("PASimTable::setTableData(unsigned short) not implemented");
}


void PASimTable::setULong(unsigned long value, unsigned int index)
{
    if (index>m_tableData.size())
    {
        throw std::out_of_range("index out of range");
    }

	setUShort(index,   (value>>16));
    setUShort(index+2, (value&0x0000ffff));
    // throw std::logic_error("PASimTable::setTableData(unsigned long) not implemented");
}


void PASimTable::setTableData(void * address, int bytes, unsigned int index)
{
    if (bytes<0)
    {
        bytes = m_tableData.size();
    }

    if ( (bytes+index) > m_tableData.size() )
    {
		m_tableData.resize(bytes+index);
        // throw std::out_of_range("too many bytes requested to copy");
    }

    int b;
    unsigned int i;
    ByteArray ba;
    for ( b=0, i=index, ba=(ByteArray)address ;
          b < bytes ;
          ++i, ++b )
    {
        m_tableData[i] = ba[i];
    }
}


void PASimTable::setWriteReturnValue(Byte value)
{
    m_writeReturnValue = value;
}

UserReply * PASimTable::processUserRead(UserRequest * ur)
{
    UserReply * reply = new UserReply;

    reply->m_replyType = 'R';
    reply->m_tableNumber = getTableNumber();
    PASimUtility::data2string(m_tableData, reply->m_replyData);

    return reply;
}


UserReply * PASimTable::processUserWrite(UserRequest * ur)
{
    UserReply * reply = new UserReply;

    reply->m_replyType = 'R';
    reply->m_tableNumber = getTableNumber();

    if (ur->m_requestType == 'U')
    {
        // out of here now.
        reply->m_replyData = "Updating not valid for this table";
        return reply;
    }


    // unless otherwise overridden, we suppose that the data is in the form :
    // <table#> <Word|Byte> <index> <value>
    unsigned int tnum=0;
    char type[16];
    unsigned int index=0;
    unsigned int value=0;
    int numtokens=sscanf(ur->m_requestInfo.c_str(),"%u %s %u %u",tnum,type,index,value);

    if (numtokens==4)
    {
        std::string s(type);
        if ( (type=="word") || (type=="w") )
        {
            setUShort(value,index);
        }
        else if ( (type=="byte") || (type=="b") )
        {
			setByte(value,index);
        }
        else
        {
            reply->m_replyData = "ERROR: value type must be one of [byte|word|b|w]";
        }

    }
    else
    {
        reply->m_replyData = "ERROR: incorrect number of parameters : <table#> <byte|word> <index> <value>";
    }


    notifyDependents(user_modification);

    return reply;
}


void PASimTable::toStream(std::ostream& os) const
{
    std::string s;
    PASimUtility::data2string(m_tableData, s);

    os << s.c_str() << std::endl;
}

std::ostream &operator<< (std::ostream &os, const PASimTable &table)
{
    table.toStream(os);
    return os;
}

void PASimTable::addDependent(PASimTable * t)
{
    m_dependents.push_back(t);
}

void PASimTable::setupDependents()
{
}

void PASimTable::notifyDependents(ChangeReasonCode reasonCode)
{
    if (m_dependents.empty())
    {
        // shortcut out if nothing to do
        return;
    }
    
    for ( DependentTableContainer::iterator iter=m_dependents.begin() ;
          iter != m_dependents.end() ; iter++
        )
    {
        (*iter)->providerChanged(this, reasonCode);
    }
}

void PASimTable::providerChanged(PASimTable * t, ChangeReasonCode reasonCode)
{
}

void PASimTable::dependUpon(int tableNumber)
{
    m_parent->dependUponTable(this,tableNumber);
}
