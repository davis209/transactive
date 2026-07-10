/**
  * The source code in this file is the property of
  * Ripple Systems and is not for redistribution
  * in any form.
  *
  * Source:   $File: //depot/4669_T01271350/4669/transactive/app/pa/PAManager/test/src/PAManagerTestCases.cpp $
  * @author:  Ripple
  * @version: $Revision: #1 $
  *
  * Last modification: $DateTime: 2008/11/28 16:26:01 $
  * Last modified by:  $Author: builder $
  *
  */
#include "PAManagerTestCases.h"

/////////////////////////////////////////////////
//
// Test Suite Methods
//
/////////////////////////////////////////////////

void PaManagerTestCases::setUp( const std::string & INFile , const std::string & OUTFile )
{
}


void PaManagerTestCases::tearDown( const std::string & INFile )
{
}


void PaManagerTestCases::setUpSuite( const std::string & inFile )
{
}


void PaManagerTestCases::tearDownSuite()
{
}


TestSuite * PaManagerTestCases::suite()
{
    TestSuite * testSuite = new TestSuite( "PaManagerTestCases" );  

	return testSuite;
}
