/**
  * The source code in this file is the property of
  * Ripple Systems and is not for redistribution in any form.
  *
  * Source:   $File: //depot/4669_T01271350/4669/transactive/app/pa/PAManager/test/src/DvaListCtrlModelTestCases.h $
  * @author:  Jade Lee
  * @version: $Revision: #1 $
  *
  * Last modification: $DateTime: 2008/11/28 16:26:01 $
  * Last modified by:  $Author: builder $
  *
  * <description>
  *
  */

#pragma once

#include <map>
#include <set>
#include <TestCase.h>
#include <TestFramework.h>
#include <windows.h>

#include "bus/pa/cachedmaps/src/CachedMapDefinitions.h"


class DvaListCtrlModelTestCases : public TestCase
{

public:
	DvaListCtrlModelTestCases ( const std::string & name ) : TestCase ( name )
	{
	}

	virtual ~DvaListCtrlModelTestCases()
	{
	}

  /**
    * suite
    * 
    * This adds all the test methods to the suite of tests and returns it.
    *
    * @return TestSuite* A collection of all tests that can be run.
    *
    */
    TestSuite * suite();

  /**
    * setUp
    * 
    * Initialises variables before each method is run.
    *
    */
    void setUp( const std::string & INFile , const std::string & OUTFile );


  /**
    * tearDown
    * 
    * Destroys variables after each method is run.
    *
    */
    void tearDown( const std::string & INFile ) {}


  /**
    * setUpSuite
    * 
    * Sets up variables before a suite of tests are run.
    *
    */
    virtual void setUpSuite( const std::string & inFile );


  /**
    * setUpSuite
    * 
    * Destroys variables after a suite of tests have been run.
    *
    */
    void tearDownSuite();

protected:
    
    void testFetchLatestRecords();
    // tested in testGetMessageData
    //void testUpdateMapping();
    void testGetMessageData();
    void testClearMapping();
    void testDoesMappingExist();


private:

    TA_IRS_Bus::StationDvaMessageRecord createRecord(unsigned long locationKey, 
                                                                unsigned long id,
                                                                std::string label,
                                                                char type);

    std::vector<TA_IRS_Bus::StationDvaMessageRecord> getMessages();
};


