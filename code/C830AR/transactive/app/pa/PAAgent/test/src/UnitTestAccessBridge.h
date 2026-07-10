/**
  * The source code in this file is the property of
  * Ripple Systems and is not for redistribution
  * in any form.
  *
  * Source:   $File: //depot/4669_T01271350/4669/transactive/app/pa/PAAgent/test/src/UnitTestAccessBridge.h $
  * @author:  Ripple
  * @version: $Revision: #1 $
  *
  * Last modification: $DateTime: 2008/11/28 16:26:01 $
  * Last modified by:  $Author: builder $
  *
  */
#pragma once

class AbstractPASTable;
class AbstractWritablePASTable;

#include <vector>
#include <TestCase.h>
#include <TestFramework.h>
#include <windows.h>

#include "core/synchronisation/src/NonReEntrantThreadLockable.h"
#include "core/synchronisation/src/ReEntrantThreadLockable.h"

/// Provides a bridge between unit tests and access to private / protected members and
/// methods of classes (so no need for multiple declarations of friend classes in source)
class UnitTestAccessBridge
{
public:
    typedef std::vector<unsigned __int8> SocketBufferType;

    static unsigned long getTableNumber(AbstractPASTable& table);

    static bool confirmTCPBufferDetails
    ( 
     const char type,
     const unsigned long tableNumber,
     const SocketBufferType& buf
    );

    static TA_Base_Core::ReEntrantThreadLockable& getWriteLock(AbstractWritablePASTable& table);
};