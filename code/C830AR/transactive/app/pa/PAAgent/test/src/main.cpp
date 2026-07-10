/**
  * The source code in this file is the property of
  * Ripple Systems and is not for redistribution
  * in any form.
  *
  * Source:   $File: //depot/4669_T01271350/4669/transactive/app/pa/PAAgent/test/src/main.cpp $
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

#include "bus\pa\cachedmaps\test\src\GenericCachedMapTestCases.h"
#include "bus\pa\cachedmaps\test\src\GenericCachedMapBaseTestCases.h"
#include "bus\pa\cachedmaps\test\src\LockedPacketTestCases.h"
#include "Table201TestCases.h"

using TA_Base_Core::DebugUtil;

#define TA_ASSERT_FAIL EXCEPTION

int main( int argc, char **argv)
{
    // Set the debug information so we don't get the debug to standard output.
	DebugUtil::getInstance().setLevel(DebugUtil::DebugDebug);
    DebugUtil::getInstance().setFile("./PAManagerTest.log");
    
    TestRunner runner;
 
    LockedPacketTestCases testcase1("LockedPacketTestCases");
    TA_IRS_Bus::GenericCachedMapTestCases testcase2("GenericCachedMapTestCases");
    TA_IRS_Bus::GenericCachedMapBaseTestCases testcase3("GenericCachedMapBaseTestCases");
    Table201TestCases testcase4("Table201TestCases");

    // Adds all test methods to the framework
    runner.addSuite( testcase1.suite() );
    runner.addSuite( testcase2.suite() );
    runner.addSuite( testcase3.suite() );
    runner.addSuite( testcase4.suite() );

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
