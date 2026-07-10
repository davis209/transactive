/**
  * The source code in this file is the property of
  * Ripple Systems and is not for redistribution
  * in any form.
  *
  * Source:   $File: //depot/4669_T01271350/4669/transactive/app/pa/PAAgent/test/src/Table201TestCases.cpp $
  * @author:  Ripple
  * @version: $Revision: #1 $
  *
  * Last modification: $DateTime: 2008/11/28 16:26:01 $
  * Last modified by:  $Author: builder $
  *
  */
#pragma warning (disable : 4786 4290)

#include "Table201TestCases.h"
#include "UnitTestAccessBridge.h"
#include "LockAcquiringThread.h"

#include "app\pa\PAAgent\src\Table201.h"
#include "app\pa\PAAgent\src\PASTableFactory.h"
#include "app\pa\PAAgent\src\PasHelpers.h"
#include "app\pa\PAAgent\src\CachedConfig.h"
#include "bus\pa\cachedmaps\src\PaZoneCachedMap.h"
#include "bus\pa\cachedmaps\src\CachedMaps.h"
#include "app\pa\PAAgent\src\PasConnection.h"

#include "core/utilities/src/RunParams.h"

using namespace TA_IRS_Bus;

/////
// Copied from Table201.cpp
const unsigned long TABLE_201_SOURCE_ID_OFFSET = 0;
const unsigned long TABLE_201_ANNOUNCE_ID_OFFSET = 1;
const unsigned long TABLE_201_LOCAL_COVERAGE_OFFSET = 2;
/////

Table201TestCases::Table201TestCases ( const std::string & name ) 
: 
TestCase ( name )
{
}


/////////////////////////////////////////////////
//
// constructor Tests
//
/////////////////////////////////////////////////
void Table201TestCases::testConstructor()
{
    Table201* table201 = PASTableFactory::getInstance()->getTable201();

    Assert(0 != table201);

    Assert(201 == UnitTestAccessBridge::getTableNumber(*table201));
}


/////////////////////////////////////////////////
//
// setTableDataAndWrite Tests
//
/////////////////////////////////////////////////
void Table201TestCases::testSetTableDataAndWrite_dataVerification()
{
    Table201* table201 = PASTableFactory::getInstance()->getTable201();

    Assert(0 != table201);
    
    enum { ZONE_KEY_MIN = 1, ZONE_KEY_MAX = 32 };

    for (unsigned char sourceId = 1; sourceId < 32; sourceId += 10)
    {
        for (unsigned char announceId = 1; announceId < 32; announceId += 10)
        {
            for (unsigned short zone = ZONE_KEY_MIN; zone < ZONE_KEY_MAX; zone ++)
            {
                // Perform length permutations of 0, 1, 2, then full length
                for (unsigned short zoneLength = 0; zoneLength < ZONE_KEY_MAX - zone; zoneLength ++)
                {
                    if (zoneLength > 2)
                    {
                        // Do this to make the unit test finish in reasonable time
                        zoneLength = (ZONE_KEY_MAX - zone) - 1;
                    }

                    TA_IRS_Bus::IPAAgentCorbaDef::ZoneCoverage zones;
                    zones.length(zoneLength);

                    for (UINT i = 0; i < zoneLength; i ++)
                    {
                        zones[i] = zone + i;
                        if (zones[i] >= ZONE_KEY_MAX)
                        {
                            zones[i] = ZONE_KEY_MIN;
                        }
                    }
                        
                    table201->setTableDataAndWrite(sourceId, announceId, zones);
                    
                    // Read data back from connection and verify corresponds to expected
                    UnitTestAccessBridge::SocketBufferType expectedData(6);   // 8 + 8 + 32 bits gives 48 bit = 6 bytes
                    PasHelpers::getInstance()->set8bit(expectedData.begin(), TABLE_201_SOURCE_ID_OFFSET, sourceId);
                    PasHelpers::getInstance()->set8bit(expectedData.begin(), TABLE_201_ANNOUNCE_ID_OFFSET, announceId);
                    ta_uint32 localCoverageBitmap;
                    PasHelpers::getInstance()->coverageToLocalCoverageBitmap(zones, localCoverageBitmap);
                    PasHelpers::getInstance()->set32bit(expectedData.begin(), TABLE_201_LOCAL_COVERAGE_OFFSET, localCoverageBitmap);
                    
                    Assert(UnitTestAccessBridge::confirmTCPBufferDetails('W', 201, expectedData));
                }
            }
        }
    }
}


void Table201TestCases::testSetTableDataAndWrite_lockVerification()
{
    Table201* table201 = PASTableFactory::getInstance()->getTable201();
    TA_Bus::IPAAgentCorbaDef::ZoneCoverage zones;   // Dummy zones

    Assert(0 != table201);

    enum { TIMEOUT = 1000 };

    // Confirm the table locks for writing correctly
    // Create a separate thread that will acquire the lock for TIMEOUT
    LockAcquiringThread lock(UnitTestAccessBridge::getWriteLock(*table201), TIMEOUT);
    lock.waitForLockToBeAcquired();   

    UINT calls = 0; // Tracks number of calls made

    while (!lock.hasCompletedAcquisition())
    {
        // Attempt the write, it should block until TIMEOUT reached and
        // subsequent completion of the 'lock' object
        table201->setTableDataAndWrite(1, 1, zones);
        calls ++;
        
        // Upon failure this means the write function was able to complete
        // more than once while the other thread held lock, not blocking correctly
        Assert(calls <= 1);        
    }        
}


//////////////////////////////////////////////////
// Unit test framework (implementation) functions
//////////////////////////////////////////////////
void Table201TestCases::setUpSuite( const std::string & inFile ) 
{
    // Table only used for slave
    CachedConfig::getInstance()->setIsMaster(false);
    // Set up a dummy location
    TA_Base_Core::RunParams::getInstance().set( RPARAM_LOCATIONKEY, "1" );
    TA_Base_Core::RunParams::getInstance().set( RPARAM_NOTIFYHOSTS, "utopia:2323,utopia:2324");

    PaZoneCachedMap& zoneCachedMap = CachedMaps::getInstance()->getPaZoneCachedMap();
    zoneCachedMap.clearKeyToRecordMap();
    for (unsigned int i = 1; i < 32; i ++)
    {   
        // Create 32 records, whose pKey corresponds to zone ID (for purposes of table pKey to id conversion)
        PaZoneRecord zone = createRecord(i, "OCC", "Public", "ZoneA");   
        zone.m_pKey = i;         
        zoneCachedMap.updateRecordMapWithItem(zone.m_pKey, zone);
    }
    zoneCachedMap.flagAsInitialised();

    PASConnection::getInstance()->enable();
}   


void Table201TestCases::tearDownSuite() 
{
}


void Table201TestCases::setUp( const std::string & INFile , const std::string & OUTFile )
{    
}


void Table201TestCases::tearDown( const std::string & INFile ) 
{
}


TestSuite * Table201TestCases::suite()
{
    TestSuite * testSuite = new TestSuite( "Table201TestCases" );  
    
// Use temporary hashdef to add tests
#define ADD_UNIT_TEST(function) \
    testSuite->addTest( new TestCaller< Table201TestCases >(\
                        #function,\
                        &Table201TestCases::##function,this ) );

    //
    // constructor tests
    //
    ADD_UNIT_TEST(testConstructor);

    //
    // setTableDataAndWrite tests
    //
    ADD_UNIT_TEST(testSetTableDataAndWrite_dataVerification);
    ADD_UNIT_TEST(testSetTableDataAndWrite_lockVerification);

#undef ADD_UNIT_TEST

	return testSuite;
}

PaZoneRecord Table201TestCases::createRecord(unsigned long locationKey, 
    std::string stationName,
    std::string area,
    std::string shortName)
{
    static unsigned long currentKey = 1;
    PaZoneRecord record;
    
    record.m_locationKey = locationKey;
    record.m_locationName = stationName;
    record.m_area = area;
    record.m_zoneShortName = shortName;
    
    record.m_id = 1;
    record.m_pKey = (currentKey ++);
    std::ostringstream label;
    label << "Zone " << record.m_pKey;
    record.m_label = label.str(); 

    // Must set to a valid state otherwise filtered out by HMI Selection class
    record.m_entityKey = 1;
    return record;
}
