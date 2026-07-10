/**
  * The source code in this file is the property of
  * Ripple Systems and is not for redistribution
  * in any form.
  *
  * Source:   $File: //depot/4669_T01271350/4669/transactive/app/pa/PAManager/test/src/DvaListCtrlModelTestCases.cpp $
  * @author:  Ripple
  * @version: $Revision: #1 $
  *
  * Last modification: $DateTime: 2008/11/28 16:26:01 $
  * Last modified by:  $Author: builder $
  *
  */
#pragma warning (disable : 4786 4290)

#include "StdAfx.h"

#include "DvaListCtrlModelTestCases.h"
#include "common.h"
#include "app/pa/PAManager/src/DvaListCtrlModel.h"

#include "bus/pa/cachedmaps/src/CachedMaps.h"
#include "bus/pa/cachedmaps/src/PaStationDvaMessageCachedMap.h"

#include "core/utilities/src/DebugUtil.h"
#include "core/utilities/src/TaAssert.h"

using namespace TA_IRS_Bus;

enum 
{
    KEY_A = 1,
    KEY_B,
    KEY_C
};

static const DWORD ADDRESS_A = 0x1;
static const DWORD ADDRESS_B = 0x2;
static const DWORD ADDRESS_C = 0x3;

/////////////////////////////////////////////////
//
// fetchLatestRecords Tests
//
/////////////////////////////////////////////////
void DvaListCtrlModelTestCases::testFetchLatestRecords()
{
    DvaListCtrlModel model;
    
    Keys keys;
    keys.push_back(28585);      // This one should came out as a null entry
    for (unsigned int i = 0; i < getMessages().size(); i ++)
    {
        keys.push_back(getMessages()[i].m_pKey);
    }
    keys.push_back(28586);      // This one should came out as a null entry
    
    std::vector<StationDvaMessageRecord*> records = model.fetchLatestRecords(keys);
    
    // Both start and end records should be null
    Assert(NULL == records.front());
    Assert(NULL == records.back());

    // Confirm all the returned records match those that we dumped into the cached map
    for (i = 0; i < records.size(); i ++)
    {
        if (0 == records[i])
        {
            continue;
        }

        Assert(*(records[i]) == getMessages()[i - 1]);

        // Clean up (client code responsibility to delete)
        delete records[i];
        records[i] = 0;
    }

    keys.clear();
    Assert(0 == model.fetchLatestRecords(keys).size());
}


/////////////////////////////////////////////////
//
// getMessageData Tests
//
/////////////////////////////////////////////////
void DvaListCtrlModelTestCases::testGetMessageData()
{
    DvaListCtrlModel model;
    
    model.updateMapping(KEY_A, ADDRESS_A);
    model.updateMapping(KEY_A, ADDRESS_B);
        
    model.updateMapping(KEY_B, ADDRESS_C);
    model.updateMapping(KEY_B, ADDRESS_A);
    model.updateMapping(KEY_B, ADDRESS_B);
    // Should be able to add in the same mapping twice, and have it appended
    // - because sometimes need to add in duplicates in a list control (ie
    // selected messages list)
    model.updateMapping(KEY_B, ADDRESS_B);
    
    std::vector<DWORD> result = model.getMessageData(KEY_A);
    Assert(2 == result.size());
    Assert(ADDRESS_A == result[0] && ADDRESS_B == result[1]);

    result = model.getMessageData(KEY_B);
    Assert(4 == result.size());

    Assert(ADDRESS_C == result[0] && ADDRESS_A == result[1] && ADDRESS_B == result[2] && ADDRESS_B == result[3]);

    try
    {   
        // Should be no data remaining (so exception when attempt to fetch)
        model.getMessageData(KEY_C);
        Assert(false);
    }
    catch (...)
    {
        Assert(true);
    }
}


/////////////////////////////////////////////////
//
// clearMapping Tests
//
/////////////////////////////////////////////////
void DvaListCtrlModelTestCases::testClearMapping()
{
    DvaListCtrlModel model;

    // Confirm that not allowed to clear mapping of a non existent element
    try
    {
        model.clearMapping(KEY_A, ADDRESS_A);
        Assert(false);
    }
    catch (...)
    {
        Assert(true);
    }

    model.updateMapping(KEY_A, ADDRESS_A);

    std::multimap<unsigned long, DWORD> old = model.m_controlMap;

    // Try to clear invalid mappings again
    try
    {
        model.clearMapping(KEY_A, ADDRESS_B);
        Assert(false);
    }
    catch (...)
    {
        Assert(true);
    }
    try
    {
        model.clearMapping(KEY_B, ADDRESS_A);
        Assert(false);
    }
    catch (...)
    {
        Assert(true);
    }
    
    // Confirm the control map hasn't changed after attempting to remove invalid item
    Assert(old == model.m_controlMap);
    
    // Add in remaining mappings
    model.updateMapping(KEY_A, ADDRESS_B);
    model.updateMapping(KEY_A, ADDRESS_C);

    Assert(3 == model.getMessageData(KEY_A).size());
    model.clearMapping(KEY_A, ADDRESS_C);
    Assert(2 == model.getMessageData(KEY_A).size());
    
    model.updateMapping(KEY_B, ADDRESS_A);
    model.updateMapping(KEY_B, ADDRESS_B);
    model.updateMapping(KEY_C, ADDRESS_A);
    model.updateMapping(KEY_C, ADDRESS_B);
    
    // Clear out all of a's mappings
    model.clearMapping(KEY_A, ADDRESS_B);
    model.clearMapping(KEY_A, ADDRESS_A);
    try
    {
        // Shouldn't be any data available now
        model.getMessageData(KEY_A);
        Assert(false);
    }
    catch (...)
    {
        Assert(true);
    }
  
    // Others should remain unaffected
    Assert(2 == model.getMessageData(KEY_B).size());
    Assert(2 == model.getMessageData(KEY_C).size());

    // Clear in order they were added
    model.clearMapping(KEY_B, ADDRESS_A);
    model.clearMapping(KEY_B, ADDRESS_B);

    try
    {
        // Shouldn't be any data available now
        model.getMessageData(KEY_B);
        Assert(false);
    }
    catch (...)
    {
        Assert(true);
    }
    Assert(2 == model.getMessageData(KEY_C).size());

    // Now add in a set of duplicate mappings, confirm clear OK
    model.updateMapping(KEY_B, ADDRESS_A);
    model.updateMapping(KEY_B, ADDRESS_B);
    model.updateMapping(KEY_B, ADDRESS_A);
    model.updateMapping(KEY_B, ADDRESS_A);
    Assert(4 == model.getMessageData(KEY_B).size());
    
    model.clearMapping(KEY_B, ADDRESS_A);
    Assert(3 == model.getMessageData(KEY_B).size());
    model.clearMapping(KEY_B, ADDRESS_A);
    Assert(2 == model.getMessageData(KEY_B).size());
    model.clearMapping(KEY_B, ADDRESS_A);
    Assert(1 == model.getMessageData(KEY_B).size());
    model.clearMapping(KEY_B, ADDRESS_B);

    try
    {
        // Should all be clear now - no remaining mappings
        model.getMessageData(KEY_B);
        Assert(false);
    }
    catch (...)
    {
        Assert(true);
    }
    Assert(2 == model.getMessageData(KEY_C).size());
}


/////////////////////////////////////////////////
//
// doesMappingExist Tests
//
/////////////////////////////////////////////////
void DvaListCtrlModelTestCases::testDoesMappingExist()
{
    DvaListCtrlModel model;
    Assert(!model.doesMappingExist(KEY_A));

    model.updateMapping(KEY_A, ADDRESS_A);
    Assert(model.doesMappingExist(KEY_A));

    Assert(!model.doesMappingExist(KEY_B));
    model.updateMapping(KEY_B, ADDRESS_A);

    model.updateMapping(KEY_A, ADDRESS_B);
    model.clearMapping(KEY_A, ADDRESS_A);
    Assert(model.doesMappingExist(KEY_A));
    Assert(model.doesMappingExist(KEY_B));

    model.clearMapping(KEY_A, ADDRESS_B);
    Assert(!model.doesMappingExist(KEY_A));
    Assert(model.doesMappingExist(KEY_B));

    model.clearMapping(KEY_B, ADDRESS_A);
    Assert(!model.doesMappingExist(KEY_B));
}


std::vector<StationDvaMessageRecord> DvaListCtrlModelTestCases::getMessages()
{
    std::vector<StationDvaMessageRecord> result;
    StationDvaMessageRecords temp = CachedMaps::getInstance()->getPaStationDvaMessageCachedMap().getAllRecords();

    for (StationDvaMessageRecordsIt itr = temp.begin(); itr != temp.end(); itr ++)
    {
        result.push_back(*itr);
    }
    return result;
}


void DvaListCtrlModelTestCases::setUpSuite( const std::string & inFile ) 
{
    TA_App::PaTest::setToValidLocation();

    PaStationDvaMessageCachedMap& cachedMap = CachedMaps::getInstance()->getPaStationDvaMessageCachedMap();
    
    // This in the station DVA Message cached map with some suitable data
    cachedMap.clearKeyToRecordMap();

    StationDvaMessageRecord record = createRecord(1, 1, "MessageA", 'N');
    cachedMap.updateRecordMapWithItem(record.m_pKey, record);
    record = createRecord(1, 1, "MessageB", 'N');
    cachedMap.updateRecordMapWithItem(record.m_pKey, record);
    record = createRecord(1, 1, "MessageC", 'N');
    cachedMap.updateRecordMapWithItem(record.m_pKey, record);
    record = createRecord(1, 1, "MessageD", 'N');
    cachedMap.updateRecordMapWithItem(record.m_pKey, record);
    record = createRecord(1, 1, "MessageA", 'E');
    cachedMap.updateRecordMapWithItem(record.m_pKey, record);
    record = createRecord(1, 1, "MessageB", 'E');
    cachedMap.updateRecordMapWithItem(record.m_pKey, record);
    record = createRecord(1, 1, "MessageC", 'E');
    cachedMap.updateRecordMapWithItem(record.m_pKey, record);
    record = createRecord(1, 1, "MessageD", 'E');
    cachedMap.updateRecordMapWithItem(record.m_pKey, record);

    cachedMap.flagAsInitialised();
}


StationDvaMessageRecord DvaListCtrlModelTestCases::createRecord(unsigned long locationKey, 
                                                                unsigned long id,
                                                                std::string label,
                                                                char type)
{
    static unsigned long primaryKey = 1;

    StationDvaMessageRecord result;
    result.m_pKey = (primaryKey ++ );
    result.m_locationKey = locationKey;
    result.m_id = id;
    result.m_label = label;
    result.m_type = type;

    return result;
}


void DvaListCtrlModelTestCases::tearDownSuite() 
{
    TA_App::PaTest::restoreLocation();
}


void DvaListCtrlModelTestCases::setUp( const std::string & INFile , const std::string & OUTFile )
{
}

TestSuite * DvaListCtrlModelTestCases::suite()
{
    TestSuite * testSuite = new TestSuite( "DvaListCtrlModelTestCases" );  

// Use temporary hashdef to add tests
#define ADD_UNIT_TEST(function) \
    testSuite->addTest( new TestCaller< DvaListCtrlModelTestCases >(\
                        #function,\
                        &DvaListCtrlModelTestCases::##function,this ) );

    //
    // fetchLatestRecord tests
    //
    ADD_UNIT_TEST(testFetchLatestRecords);

    //
    // getMessageData tests
    //
    ADD_UNIT_TEST(testGetMessageData);

    //
    // clearMapping tests
    //
    ADD_UNIT_TEST(testClearMapping);

    //
    // doesMappingExist tests
    //
    ADD_UNIT_TEST(testDoesMappingExist);

#undef ADD_UNIT_TEST

	return testSuite;
}