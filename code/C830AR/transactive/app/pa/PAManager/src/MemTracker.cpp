/**
 * The source code in this file is the property of
 * Ripple Systems and is not for redistribution
 * in any form.
 *
 * Source: $File: //depot/C830AR_TIP/C830AR/transactive/app/pa/PAManager/src/MemTracker.cpp $
 * @author Jade Lee
 * @version $Revision: #2 $
 * Last modification: $DateTime: 2024/12/17 16:14:02 $
 * Last modified by: $Author: limin.zhu $
 *
 * Provides memory leak tracking facilities
 *
 */

#include "app/pa/PAManager/src/MemTracker.h"

#include <CrtDbg.h>

namespace MemTracker
{
    int __cdecl CustomAllocationHook(
        int      nAllocType,
        void*    pvData,
        size_t   nSize,
        int      nBlockUse,
        long     lRequest,
        const unsigned char* szFileName,
        int      nLine
    )
    {
        if ( nBlockUse == _CRT_BLOCK )
        {
            // Ignore internal C runtime library allocations
            return true;
        }

        /*
        switch (lRequest)
        {
        case 8410:
            int h;
            h = 2;
            break;
        default:
            break;
        };
        */

        /*
        char *operation[] = { "", "allocating", "re-allocating", "freeing" };
        char *blockType[] = { "Free", "Normal", "CRT", "Ignore", "Client" };

        if ( nBlockUse == _CRT_BLOCK )   // Ignore internal C runtime library allocations
            return TRUE;

        _ASSERT( ( nAllocType > 0 ) && ( nAllocType < 4 ) );
        _ASSERT( ( nBlockUse >= 0 ) && ( nBlockUse < 5 ) );

        fprintf( logFile,
                    "Memory operation in %s, line %d: %s a %d-byte '%s' block (# %ld)\n",
                    szFileName, nLine, operation[nAllocType], nSize,
                    blockType[nBlockUse], lRequest );
        if ( pvData != NULL )
            fprintf( logFile, " at %X", pvData );
        */

        return ( true );        // Allow the memory operation to proceed
    }

    void attachMemoryHook()
    {
        _CrtSetAllocHook( CustomAllocationHook );
    }

}