/**
  * The source code in this file is the property of
  * Ripple Systems and is not for redistribution
  * in any form.
  *
  * Source:   $File: //depot/4669_T01271350/4669/transactive/app/pa/PAManager/test/src/main.cpp $
  * @author:  Ripple
  * @version: $Revision: #1 $
  *
  * Last modification: $DateTime: 2008/11/28 16:26:01 $
  * Last modified by:  $Author: builder $
  *
  */
#pragma warning(disable:4786 4284)

#include <iostream>
#include "core\utilities\src\DebugUtil.h"
#include "core\utilities\src\RunParams.h"

#include "StdAfx.h"
#include "PAManagerTestCases.h"
#include "RunParamListenerTestCases.h"
#include "DvaMessageChangeSubscriberTestCases.h"
#include "HmiSelectionListenerTestCases.h"
#include "DvaListCtrlModelTestCases.h"
#include "DvaListCtrlTestCases.h"
#include "TrainLiveStatusModelTestCases.h"
#include "TrainLiveStatusDlgTestCases.h"
#include "TrainListCtrlTestCases.h"

#include "bus\pa\common\test\src\UpdateBufferTestCases.h"
#include "bus\pa\common\test\src\ElementTestCases.h"
#include "bus\pa\messagesubscriber\test\src\GenericMessageSubscriberTestCases.h"

using TA_Base_Core::DebugUtil;

#define TA_ASSERT_FAIL EXCEPTION

int main( int argc, char **argv)
{
    // Must initialise AFX for the PA Manager unit tests (in case needs to bring up Afx dialogs)
    HINSTANCE handle = NULL;
    handle = LoadLibrary(argv[0]);

    std::auto_ptr<CWinApp> winApp(new CWinApp("DummyApp"));

    if (0 != handle)
    {
        // Set AfxGetInstanceHandle()
        AfxGetModuleState()->m_hCurrentInstanceHandle = handle;
        // Must set this to allow dialogs to start up (create dummy app)
        AfxGetModuleState()->m_pCurrentWinApp = winApp.get();
        AfxGetApp()->m_hInstance = handle;
        
        AfxSetResourceHandle(handle);
    } 

    // Set the debug information so we don't get the debug to standard output.
	DebugUtil::getInstance().setLevel(DebugUtil::DebugDebug);
    DebugUtil::getInstance().setFile("./PAManagerTest.log");
    
    TestRunner runner;
 
    //PaManagerTestCases testcase1("PAManagerTestCases");
    UpdateBufferTestCases testcase2("UpdateBufferTestCases");
    ElementTestCases testcase3("ElementTestCases");
    RunParamListenerTestCases testcase4("RunParamListenerTestCases");
    DvaMessageChangeSubscriberTestCases testcase5("DvaMessageChangeSubscriberTestCases");
    GenericMessageSubscriberTestCases testcase6("GenericMessageSubscriberTestCases");
    HmiSelectionListenerTestCases testcase7("HmiSelectionListenerTestCases");
    DvaListCtrlModelTestCases testcase8("DvaListCtrlModelTestCases");
    DvaListCtrlTestCases testcase9("DvaListCtrlTestCases");
    TrainLiveStatusModelTestCases testcase10("TrainLiveStatusModelTestCases");
    TrainLiveStatusDlgTestCases testcase11("TrainLiveStatusDlgTestCases");
    TrainListCtrlTestCases testcase12("TrainListCtrlTestCases");

    //runner.addSuite( testcase1.suite() );      // Adds all test methods to the framework
    runner.addSuite( testcase2.suite() );
    runner.addSuite( testcase3.suite() );
    runner.addSuite( testcase4.suite() );
    runner.addSuite( testcase5.suite() );
    runner.addSuite( testcase6.suite() );
    runner.addSuite( testcase7.suite() );
    runner.addSuite( testcase8.suite() );
    runner.addSuite( testcase9.suite() );
    runner.addSuite( testcase10.suite() );
    runner.addSuite( testcase11.suite() );
    runner.addSuite( testcase12.suite() );

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
