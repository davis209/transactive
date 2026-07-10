/**
  * The source code in this file is the property of
  * Ripple Systems and is not for redistribution
  * in any form.
  *
  * Source:   $File: //depot/4669_T01271350/4669/transactive/app/pa/PAAgent/test/src/LockAcquiringThread.h $
  * @author:  Ripple
  * @version: $Revision: #1 $
  *
  * Last modification: $DateTime: 2008/11/28 16:26:01 $
  * Last modified by:  $Author: builder $
  *
  */
#pragma once

#include <windows.h>
#include "core/synchronisation/src/NonReEntrantThreadLockable.h"

/**
 * This class creates a new thread upon constructor that attempts to acquire the 
 *  designated input thread
 */
class LockAcquiringThread
{
public:

    LockAcquiringThread(TA_Base_Core::IThreadLockable& lock, UINT timeout = (-1));

    // For multithreading operations
    static DWORD WINAPI threadLockAcquire(void* val);

    // Internal implementation
    void threadLockAcquire();
    
    bool hasCompletedAcquisition() { return m_completedAcquisition; }

    void waitForLockToBeAcquired();

private:

    UINT m_timeout;
    volatile bool m_lockAcquired;
    volatile bool m_completedAcquisition;
    TA_Base_Core::IThreadLockable* m_lock;

    void acquireLock(TA_Base_Core::IThreadLockable& lock);
};