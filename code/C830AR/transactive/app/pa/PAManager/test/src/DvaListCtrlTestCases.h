/**
  * The source code in this file is the property of
  * Ripple Systems and is not for redistribution in any form.
  *
  * Source:   $File: //depot/4669_T01271350/4669/transactive/app/pa/PAManager/test/src/DvaListCtrlTestCases.h $
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
#include "bus/pa/cachedmaps/src/CachedMaps.h"

class DvaListCtrl;
class WindowStub;

class DvaListCtrlTestCases : public TestCase
{

public:
	DvaListCtrlTestCases ( const std::string & name ) 
    : 
    TestCase ( name ),
    m_listCtrlWindowStub(0),
    m_dvaListCtrl(0)
	{
	}

	virtual ~DvaListCtrlTestCases()
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
    void tearDown( const std::string & INFile );


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

    static int __cdecl deleteCheckHook(int      nAllocType,
                                        void   * pvData,
                                        size_t   nSize,
                                        int      nBlockUse,
                                        long     lRequest,
                                        const unsigned char * szFileName,
                                        int      nLine);       
    static std::set<void*>   s_expectedDeletions;

protected:

    void testRefreshContents();
    void testProcessDvaMessageUpdates_uniqueItems();
    void testProcessDvaMessageUpdates_duplicateItems();
    
    void testDoesMessageKeyExist();
    // Tested within testDoesMessageKeyExist
    //void testDoAllMessageKeysExist();
    
    void testRemoveSingleDvaMessage();
    void testGetSelectedMessageKey();
    void testGetRecordFromIndex();
    void testCollectMatchingKeys_returnvalue();
    void testCollectMatchingKeys_listOutputs();
    void testCollectMatchingKeys_typeLabelOutputs();
    void testSortAlphabeticallyAscending();
    void testFindItemIndices();
    void testDeleteAllItems();
    void testDeleteItem();
    // tested in testDeleteItem
    //void testDeleteItemData();  
    // Tested in testDeleteAllItems
    //void testCleanupItemData();
    void testSetItemData_replaceInvalid();
    void testSetItemData_addReplace();
    void testSetItemData_duplicateKeys();
    void testFindSortedIdInsertionPoint();
    // tested in processDvaMessageUpdates (passes through to this function)
    //void testRefreshItemsFromCachedMaps();
    // tested in testSetItemData (internalCompleteItemUpdate is just an internal helper function)
    //void testInternalCompleteItemUpdate();

private:

    // helper function for use by testSortAlphabeticallyAscending
    void confirmSortsAlphabetically(std::vector <std::pair<int, std::string> > ordered);

    // Removes all shared elements that exist between setA and B
    // @return true if both sets emptied (fully) as a result of this function
    bool destroyAllMatches(Keys& setA, Keys& setB);
    
    // Returns list of keys present in the m_dvaListCtrl    
    Keys getKeysInListCtrl();
    
    // Returns list of keys in local messages array (as defined by getMessages())
    // within the specified range
    Keys getKeysInRange(UINT lower, UINT upper);
    DvaMessageUpdateData getUpdatesInRange(UINT lower, UINT upper);

    // Confirm records present in list control match data in cached map
    void confirmListCtrlContentsMatchCachedMap(bool confirmMatch = true);


    TA_IRS_Bus::StationDvaMessageRecord createRecord(unsigned long locationKey, 
                                                                unsigned long id,
                                                                std::string label,
                                                                char type);

    std::vector<TA_IRS_Bus::StationDvaMessageRecord> getMessages();

    WindowStub* m_listCtrlWindowStub;
    DvaListCtrl* m_dvaListCtrl;
};



