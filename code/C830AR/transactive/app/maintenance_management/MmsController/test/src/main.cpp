/**
  * The source code in this file is the property of
  * Ripple Systems and is not for redistribution
  * in any form.
  *
  * Source:   $File: //depot/4669_T01271350/4669/transactive/app/maintenance_management/MmsController/test/src/main.cpp $
  * @author:  Ripple
  * @version: $Revision: #1 $
  *
  * Last modification: $DateTime: 2008/11/28 16:26:01 $
  * Last modified by:  $Author: builder $
  *
  */
#pragma warning(disable:4786 4284)

#include "StdAfx.h"

#include <iostream>
#include "core\utilities\src\DebugUtil.h"
#include "core\utilities\src\RunParams.h"

#include "MmsControllerModelTestCases.h"
#include "Common.h"

using TA_Base_Core::DebugUtil;

#define TA_ASSERT_FAIL EXCEPTION

int main( int argc, char **argv)
{
    // Must initialise AFX for the unit tests (in case needs to bring up Afx dialogs)
    HINSTANCE handle = NULL;
    handle = LoadLibrary(argv[0]);

    if (0 != handle)
    {
        // Set AfxGetInstanceHandle()
        AfxGetModuleState()->m_hCurrentInstanceHandle = handle;
        // Must set this to allow dialogs to start up (create dummy app)
        AfxGetModuleState()->m_pCurrentWinApp = &TA_IRS_App::MmsTest::g_theTestApp;
        AfxGetApp()->m_hInstance = handle;
        
        AfxSetResourceHandle(handle);
    } 

    // Set up some configuration information accessible to Mms Controller app
    TA_IRS_App::MmsTest::setToValidLocation();

    // Set the debug information so we don't get the debug to standard output.
	DebugUtil::getInstance().setLevel(DebugUtil::DebugDebug);
    DebugUtil::getInstance().setFile("./MmsControllerTest.log");
    
    TestRunner runner;
 
    MmsControllerModelTestCases testcase1("MmsControllerModelTestCases");
    
    runner.addSuite( testcase1.suite() );      // Adds all test methods to the framework
    
    try
    {
        // Run config file. Make sure it is in the cfg directory.
        runner.begin( "./cfg/config.ini" );
    }
    catch (...)
    {
		std::cout << "Caught an unhandled exception" << std::endl;
	}

    return 0;
}
