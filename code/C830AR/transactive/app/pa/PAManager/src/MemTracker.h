/**
 * The source code in this file is the property of
 * Ripple Systems and is not for redistribution
 * in any form.
 *
 * Source: $File: //depot/C830AR_TIP/C830AR/transactive/app/pa/PAManager/src/MemTracker.h $
 * @author Jade Lee
 * @version $Revision: #2 $
 * Last modification: $DateTime: 2024/12/17 16:14:02 $
 * Last modified by: $Author: limin.zhu $
 *
 * Provides memory leak tracking facilities
 *
 */

#ifndef _MEMTRACKER_HEADER_
#define _MEMTRACKER_HEADER_

namespace MemTracker
{
    /**
     * Attaches hook to new handler
     *
     */
    void attachMemoryHook();
}

#endif //_MEMTRACKER_HEADER_