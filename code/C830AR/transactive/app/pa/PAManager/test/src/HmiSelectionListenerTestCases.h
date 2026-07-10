/**
  * The source code in this file is the property of
  * Ripple Systems and is not for redistribution in any form.
  *
  * Source:   $File: //depot/4669_T01271350/4669/transactive/app/pa/PAManager/test/src/HmiSelectionListenerTestCases.h $
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
#include "WindowStub.h"
#include "bus/pa/cachedmaps/src/CachedMapDefinitions.h"

class HmiSelectionListener;

class HmiSelectionListenerTestCases : public TestCase
{

public:
	HmiSelectionListenerTestCases ( const std::string & name ) : TestCase ( name )
	{
	}

	virtual ~HmiSelectionListenerTestCases()
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
    
    void testConstructor();
    void testGetZoneList();    
    void testProcessRunParamChange_select_deselect();
    void testProcessRunParamChange_select_deselect_single();
    void testProcessRunParamChange_select_deselect_byStation();
    void testProcessRunParamChange_select_deselect_byArea();
    void testProcessRunParamChange_select_deselect_byZoneName();

    void testProcessZoneRefresh_singleCall();
    void testProcessZoneRefresh_multipleCalls();

    void testOnZoneSelectionChange();

    void testRefreshSelectionUsingZones();
    void testClearAllZonesFromSelection();
    void testAddAllZonesToSelection();
    void testGetTokenValue();
    void testModifyZoneSelection();

    void testGetSpecifiedZones();
    // These functions are fully tested by testGetSpecifiedZones
    //void testGetPaManagerSpecificZones();
    //void testCreateNewFilter();
    //void testLinkFilters();

private:

    enum ENotification
    {
        CHANGEEXPECTED,
        NOCHANGEEXPECTED,
        DOESNTMATTER
    };

    std::string getZoneName(int i, bool makeLowerCase = false);
    std::string getZoneName(const TA_IRS_Bus::PaZoneRecord& record);
    std::string getZoneNameFromKey(unsigned long key);

    void confirmAndClearWindowNotification(ENotification type = CHANGEEXPECTED);

    TA_IRS_Bus::PaZoneRecord createRecord(unsigned long locationKey, 
                                std::string stationName,
                                std::string area,
                                std::string shortName);

    HmiSelectionListener* m_hmiSelectionListener;
    WindowStub m_windowStub;

    // Custom set of zones we're using for these tests
    std::vector<TA_IRS_Bus::PaZoneRecord>   m_zones;
};


