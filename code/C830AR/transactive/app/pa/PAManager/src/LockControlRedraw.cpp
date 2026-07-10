/**
 * The source code in this file is the property of
 * Ripple Systems and is not for redistribution
 * in any form.
 *
 * Source: $File: //depot/C830AR_TIP/C830AR/transactive/app/pa/PAManager/src/LockControlRedraw.cpp $
 * @author Jade Lee
 * @version $Revision: #2 $
 * Last modification: $DateTime: 2024/12/17 16:14:02 $
 * Last modified by: $Author: limin.zhu $
 *
 * Helper class to handle setting and resetting of redraw flag
 *     for controls (to reduce flicker, when making major changes)
 *
 */

#include "app/pa/PAManager/src/stdafx.h"
#include "app/pa/PAManager/src/LockControlRedraw.h"

#ifdef _DEBUG
    #define new DEBUG_NEW
    #undef THIS_FILE
    static char THIS_FILE[] = __FILE__;
#endif

LockControlRedraw::LockControlRedraw(CWnd& window)
    :
    m_window(window)
{
    // Turn off redraw
    m_window.SetRedraw(FALSE);
}

LockControlRedraw::~LockControlRedraw()
{
    // Restore the redraw state
    m_window.SetRedraw(TRUE);

    // Invalidate the window to force complete redraw
    m_window.InvalidateRect(0, TRUE);
}
