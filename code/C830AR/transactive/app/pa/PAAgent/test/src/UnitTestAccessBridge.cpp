/**
  * The source code in this file is the property of
  * Ripple Systems and is not for redistribution
  * in any form.
  *
  * Source:   $File: //depot/4669_T01271350/4669/transactive/app/pa/PAAgent/test/src/UnitTestAccessBridge.cpp $
  * @author:  Ripple
  * @version: $Revision: #1 $
  *
  * Last modification: $DateTime: 2008/11/28 16:26:01 $
  * Last modified by:  $Author: builder $
  *
  */
#include "UnitTestAccessBridge.h"
#include "app/pa/PAAgent/src/AbstractPASTable.h"
#include "app/pa/PAAgent/src/AbstractWriteablePASTable.h"
#include "app/pa/PAAgent/src/PasConnection.h"
#include "app/pa/PAAgent/src/PasHelpers.h"

//////////
// Copied directly from PasConnection.cpp
const unsigned long PAS_WRITE_REQUEST_HEADER_SIZE                   = 5;
const unsigned long PAS_WRITE_REQUEST_HEADER_TYPE_OFFSET            = 0;
const unsigned long PAS_WRITE_REQUEST_HEADER_TABLE_NUMBER_OFFSET    = 1;
const unsigned long PAS_WRITE_REQUEST_HEADER_TABLE_DATA_SIZE_OFFSET = 3;
//////////

unsigned long UnitTestAccessBridge::getTableNumber(AbstractPASTable& table)
{
    return table.TABLE_NUMBER;
}

bool UnitTestAccessBridge::confirmTCPBufferDetails
( 
 const char type,
 const unsigned long tableNumber,
 const SocketBufferType& buf
)
{
    ta_uint8* buffer = PASConnection::getInstance()->m_internalBuffer;
    
    char foundType = PasHelpers::getInstance()->get8bit(buffer, PAS_WRITE_REQUEST_HEADER_TYPE_OFFSET);
    if (foundType != type)
    {
        return false;
    }

    unsigned long foundTableNumber = PasHelpers::getInstance()->get16bit(buffer, PAS_WRITE_REQUEST_HEADER_TABLE_NUMBER_OFFSET);    
    if (tableNumber != tableNumber)
    {
        return false;
    }

    unsigned long foundSize = PasHelpers::getInstance()->get16bit(buffer, PAS_WRITE_REQUEST_HEADER_TABLE_DATA_SIZE_OFFSET);
    if (foundSize != buf.size())
    {
        return false;
    }
    
    char* data = reinterpret_cast<char*>(buffer + PAS_WRITE_REQUEST_HEADER_SIZE);

    if (0 != memcmp(data, buf.begin(), buf.size()) )
    {
        return false;
    }

    // All checks succeeded
    return true;
}


TA_Base_Core::ReEntrantThreadLockable& UnitTestAccessBridge::getWriteLock(AbstractWritablePASTable& table)
{
    return table.m_lockForWriting;
}