/**
 * The source code in this file is the property of
 * Ripple Systems and is not for redistribution
 * in any form.
 *
 * Source : $File: //depot/C830AR_TIP/C830AR/transactive/app/pa/PAAgent/src/CircularList.h $
 * @author  J. Welton
 * @version $Revision: #2 $
 * Last modification : $DateTime: 2024/12/10 11:18:50 $
 * Last modified by : $Author: limin.zhu $
 *
 *
 *    Template for a  queue object. Example
 *    implementations are DataPointWriteQueue and
 *    DataPointAlarmQueue.
 *
 *
 */

#pragma once
#include "core/synchronisation/src/ReEntrantThreadLockable.h"
#include "core/synchronisation/src/ThreadGuard.h"
#include "core/synchronisation/src/Semaphore.h"
#include <list>

template<class ITEM> class CircularList
{
public:

    CircularList<ITEM>() : m_semaphore(0)
    {
        //m_currentPosition = NULL;
        m_currentPosition = m_list.end();
    }

    ~CircularList<ITEM>()
    {
        //
        // Empty the list
        //
        TA_Base_Core::ThreadGuard synch(m_lock);

        // Data in the list is not owned by this class so we should not delete them.
        // The class that creates the ITEMS should be responsible for deleting the items.

        m_list.clear();
        //m_currentPosition = NULL;
        m_currentPosition = m_list.end();
        m_list.end();
    }

    bool isEmpty()
    {
        //
        // Synch this as the operation is unlikely to be atomic
        //
        TA_Base_Core::ThreadGuard guard(m_lock);
        return m_list.empty();
    }

    void clear()
    {
        //
        // Synch this as the operation is unlikely to be atomic
        //
        TA_Base_Core::ThreadGuard guard(m_lock);
        m_list.clear();
        // invalidate our position
        m_currentPosition = NULL;
    }

    unsigned int getSize()
    {
        //
        // Synch this as the operation is unlikely to be atomic
        //
        TA_Base_Core::ThreadGuard guard(m_lock);
        return m_list.size();
    }

    // Inserts at the current position i.e. getNext will have to traverse the entire collection
    // before reaching this new entry.
    void insert(ITEM newItem)
    {
        //
        // Synch this to prevent multiple inserts occurring
        // simultaneously or  insert/getNext's on the same
        // object
        //
        TA_Base_Core::ThreadGuard guard(m_lock);

        typename std::list<ITEM>::iterator it = m_list.begin();

        for (; it != m_list.end(); ++it)
        {
            if (*it == newItem)
            {
                return;
            }
        }

        // Not found - therefore add
        //if (m_currentPosition == NULL || m_currentPosition == m_list.end())
        if (m_currentPosition == m_list.end())
        {
            // new list or invalidated position
            m_list.push_front(newItem);
        }
        else
        {
            // We increment the current position first so that the insert is done
            // at the current position (not before)
            m_currentPosition++;

            // Set the new current position to this new element so that the next call
            // to getNext will return the item after this new element.
            m_currentPosition = m_list.insert(m_currentPosition, newItem);
        }

        m_semaphore.post();
    }

    void remove(ITEM newItem)
    {
        //
        // Synch this to prevent multiple inserts occurring
        // simultaneously or  insert/getNext's on the same
        // object
        //
        TA_Base_Core::ThreadGuard guard(m_lock);

        typename std::list<ITEM>::iterator it = m_list.begin();

        for (; it != m_list.end(); ++it)
        {
            if (*it == newItem)
            {
                if (m_currentPosition == it)
                {
                    if (m_currentPosition == m_list.begin())
                    {
                        // invalidate our position
                        //m_currentPosition = NULL;
                        m_currentPosition = m_list.end();
                    }
                    else
                    {
                        --m_currentPosition;
                    }
                }

                m_list.erase(it);
                break;
            }
        }
    }

    ITEM getNext()
    {
        if (isEmpty())
        {
            m_semaphore.wait();
        }

        TA_Base_Core::ThreadGuard synch(m_lock);

        //if (m_currentPosition == NULL || m_currentPosition == m_list.end())
        if (m_currentPosition == m_list.end())
        {
            // Invalidated iterator conditions
            return getFirst();
        }

        ++m_currentPosition;

        if (m_currentPosition == m_list.end())
        {
            // wrap-around condition
            return getFirst();
        }

        return *m_currentPosition;
    }

    void unBlockList()
    {
        m_semaphore.post();
    }

protected:

    // Only to be used by getNext()
    ITEM getFirst()
    {
        TA_Base_Core::ThreadGuard synch(m_lock);

        m_currentPosition = m_list.begin();

        if (m_currentPosition == m_list.end())
        {
            return NULL;
        }

        return *m_currentPosition;
    }

    std::list<ITEM>                             m_list;
    typename std::list<ITEM>::iterator          m_currentPosition;

    TA_Base_Core::ReEntrantThreadLockable           m_lock;
    TA_Base_Core::Semaphore                          m_semaphore;
};
