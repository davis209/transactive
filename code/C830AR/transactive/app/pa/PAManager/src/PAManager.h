/**
 * The source code in this file is the property of
 * Ripple Systems and is not for redistribution
 * in any form.
 *
 * Source:   $File: //depot/C830AR_TIP/C830AR/transactive/app/pa/PAManager/src/PAManager.h $
 * @author:  Ripple
 * @version: $Revision: #2 $
 *
 * Last modification: $DateTime: 2024/12/17 16:14:02 $
 * Last modified by:  $Author: limin.zhu $
 *
 */
// TISManager.h : main header file for the TISMANAGER application
//

#if !defined(AFX_PAMANAGER_H__BDA71947_87B3_4D13_90C2_5E8F9311AB9D__INCLUDED_)
#define AFX_PAMANAGER_H__BDA71947_87B3_4D13_90C2_5E8F9311AB9D__INCLUDED_

#if _MSC_VER > 1000
    #pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
    #error include 'stdafx.h' before including this file for PCH
#endif

#include "app/pa/PAManager/src/resource.h"		// main symbols
#include "bus/generic_gui/src/TransActiveWinApp.h"
#include "bus/generic_gui/src/GenericGUI.h"

/////////////////////////////////////////////////////////////////////////////
// PAManagerApp:
// See TISManager.cpp for the implementation of this class
//

class PAManagerApp : public TA_Base_Bus::TransActiveWinApp
{
public:

    PAManagerApp();
    virtual ~PAManagerApp();

    /**
     * ExitInstance (extension of base class implementation)
     *
     * Performs some additional cleanup (singleton instances etc)
     *
     */
    virtual int ExitInstance();

    /**
     * cleanupSingletons
     *
     * Calls removeInstance on singletons in use by application
     * This can be called as many times as application wants to
     * (and in some cases, is known to be called twice when known
     * the singleton destruction can't be postponed, because of
     * late subscriber destruction issues)
     *
     */
    static void cleanupSingletons();

    /**
     * attemptRestart
     *
     * Attempts to re-launch PA Manager
     *
     */
    void attemptRestart();

    // Overrides
    // ClassWizard generated virtual function overrides
    //{{AFX_VIRTUAL(CTISManagerApp)

public:

    //}}AFX_VIRTUAL

    // Implementation

    //{{AFX_MSG(CTISManagerApp)
    // NOTE - the ClassWizard will add and remove member functions here.
    //    DO NOT EDIT what you see in these blocks of generated code !
    //}}AFX_MSG
    DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_PAMANAGER_H__BDA71947_87B3_4D13_90C2_5E8F9311AB9D__INCLUDED_)
