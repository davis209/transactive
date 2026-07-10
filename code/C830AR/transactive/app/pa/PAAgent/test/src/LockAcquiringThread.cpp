/**
  * The source code in this file is the property of
  * Ripple Systems and is not for redistribution
  * in any form.
  *
  * Source:   $File: //depot/4669_T01271350/4669/transactive/app/pa/PAAgent/test/src/LockAcquiringThread.cpp $
  * @author:  Ripple
  * @version: $Revision: #1 $
  *
  * Last modification: $DateTime: 2008/11/28 16:26:01 $
  * Last modified by:  $Author: builder $
  *
  */
#include "LockAcquiringThread.h"
#include "core/utilities/src/TAAssert.h"


LockAcquiringThread::LockAcquiringThread(TA_Base_Core::IThreadLockable& lock, UINT timeout) :
m_lockAcquired(false),
m_completedAcquisition(false),
m_lock(0),
m_timeout(timeout)
{
    acquireLock(lock);
}


void LockAcquiringThread::waitForLockToBeAcquired()
{
    // Wait for lock to be acquired before returning
    while (!m_lockAcquired) { Sleep(1); }
}


void LockAcquiringThread::acquireLock(TA_Base_Core::IThreadLockable& lock)
{
    m_lockAcquired = false;
    m_lock = &lock;

    // Thread cleans itself up
    DWORD threadId = 0;
    HANDLE hThread = CreateThread(0, 0, threadLockAcquire, (void*)this, 0, &threadId);
    
    TA_ASSERT(NULL != hThread, "Unable to create thread");
    TA_ASSERT(0 != threadId, "Thread ID unknown");
}


DWORD WINAPI LockAcquiringThread::threadLockAcquire(void* val)
{
    // Static implementation that forwards to internal non-static implementation
    LockAcquiringThread* lat = (LockAcquiringThread*)(val);
    lat->threadLockAcquire();
    ExitThread(0);
    return 0;
}


void LockAcquiringThread::threadLockAcquire()
{
    // Reset initial state flags
    m_completedAcquisition = false;

    try
    {        
        TA_Base_Core::ThreadGuard guard (*m_lock); 
        m_lockAcquired = true;    
    
        // Sleep for the required amount of time
        Sleep(m_timeout);          

        // No longer waiting to acquire lock (need to set flag before releasing lock)
        m_completedAcquisition = true;
    }
    catch (...)
    {
        
    }
}

