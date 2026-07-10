/**
 * The source code in this file is the property of
 * Ripple Systems and is not for redistribution
 * in any form.
 *
 * Source: $File: //depot/C830AR_TIP/C830AR/transactive/app/pa/PAManager/src/LockControlRedraw.h $
 * @author Jade Lee
 * @version $Revision: #2 $
 * Last modification: $DateTime: 2024/12/17 16:14:02 $
 * Last modified by: $Author: limin.zhu $
 *
 * Helper class to handle setting and resetting of redraw flag
 *     for controls (to reduce flicker, when making major changes)
 *
 */

#ifndef _LockControlRedraw_HEADER_
#define _LockControlRedraw_HEADER_

class LockControlRedraw
{
public:

    LockControlRedraw(CWnd& window);
    virtual ~LockControlRedraw();

private:

    CWnd&   m_window;

    // There is no such thing as GetRedraw..
    //BOOL    m_oldRedrawState;
};

#endif // _LockControlRedraw_HEADER_