/**
  * The source code in this file is the property of
  * Ripple Systems and is not for redistribution
  * in any form.
  *
  * Source:   $File: //depot/4669_T01271350/4669/transactive/app/pa/PAManager/test/src/HmiSelectionListenerTestCases.cpp $
  * @author:  Ripple
  * @version: $Revision: #1 $
  *
  * Last modification: $DateTime: 2008/11/28 16:26:01 $
  * Last modified by:  $Author: builder $
  *
  */
#pragma warning (disable : 4786 4290)

#include "StdAfx.h"
#include "HmiSelectionListenerTestCases.h"
#include "common.h"
#include "app/pa/PAManager/src/HmiSelectionListener.h"
#include "core/utilities/src/DebugUtil.h"
#include "core/utilities/src/TaAssert.h"
#include "bus/pa/cachedmaps/src/CachedMaps.h"
#include "bus/pa/cachedmaps/src/PaZoneCachedMap.h"
#include "bus/pa/cachedmaps/src/GenericCachedMap.h"

#include <algorithm>

using namespace TA_Base_Bus;

/////////////////////////////////////////////////
//
// constructor Tests
//
/////////////////////////////////////////////////
void HmiSelectionListenerTestCases::testConstructor()
{
    // Cannot create more than 1 listener (because runparams
    // doesn't support multiple connections)
    //HmiSelectionListener listener(m_windowStub.getHWnd());
    
    Assert(m_windowStub.getHWnd() == m_hmiSelectionListener->m_notificationHwnd);
}


/////////////////////////////////////////////////
//
// getZoneList Tests
//
/////////////////////////////////////////////////
void HmiSelectionListenerTestCases::testGetZoneList()
{
    // Select all zones
    m_hmiSelectionListener->processRunParamChange("Select", "???");
    const std::list<unsigned long> zones = m_hmiSelectionListener->getZoneList();
    
    Assert(zones.size() == m_hmiSelectionListener->m_zoneList.size());

    for (std::list<unsigned long>::const_iterator itr = zones.begin(); 
            itr != zones.end();
            itr ++)
    {
        // Confirm every item returned in list exists in the set
        Assert(m_hmiSelectionListener->m_zoneList.end() !=
                m_hmiSelectionListener->m_zoneList.find(*itr));
    }
}


/////////////////////////////////////////////////
//
// processRunParamChange Tests
//
/////////////////////////////////////////////////
void HmiSelectionListenerTestCases::testProcessRunParamChange_select_deselect()
{
    // Test selection / deselection of entire set
    m_hmiSelectionListener->processRunParamChange("Select", "???");    
    confirmAndClearWindowNotification();
    Assert(m_zones.size() == m_hmiSelectionListener->getZoneList().size());

    m_hmiSelectionListener->processRunParamChange("Deselect", "???");    
    confirmAndClearWindowNotification();
    Assert(0 == m_hmiSelectionListener->getZoneList().size());
}


void HmiSelectionListenerTestCases::testProcessRunParamChange_select_deselect_byZoneName()
{
    // Do a batch selection by zone name
    std::string byZone = "???.PA.???." + m_zones[0].m_zoneShortName;
    m_hmiSelectionListener->processRunParamChange("Select", byZone);    
    confirmAndClearWindowNotification();

    for (unsigned int i = 0; i < m_zones.size(); i ++)
    {
        unsigned long key = m_zones[i].m_pKey;
        
        if (m_zones[i].m_zoneShortName == m_zones[0].m_zoneShortName)
        {
            // Confirm that for all matching areas, key is in the selected zone list
            Assert(m_hmiSelectionListener->m_zoneList.end() != 
                    m_hmiSelectionListener->m_zoneList.find(key));
        }
        else
        {
            // This zone shouldn't be in result
            Assert(m_hmiSelectionListener->m_zoneList.end() == 
                    m_hmiSelectionListener->m_zoneList.find(key));
        }
    }    

    m_hmiSelectionListener->processRunParamChange("Deselect", byZone);    
    confirmAndClearWindowNotification();
    Assert(0 == m_hmiSelectionListener->getZoneList().size());
}


void HmiSelectionListenerTestCases::testProcessRunParamChange_select_deselect_byStation()
{
    // Do a batch selection by zone name
    for (int iteration = 0; iteration < 3; iteration ++)
    {
        const std::string stationName = "DBG";
        std::string byStation;
 
        switch (iteration)
        {
        case 0:
            byStation = stationName + ".PA.???.???";
            break;
        case 1:
            byStation = stationName + ".???";
            break;
        default:
            byStation = stationName;
            break;
        }

        m_hmiSelectionListener->processRunParamChange("Select", byStation);    
        confirmAndClearWindowNotification();
    
        for (unsigned int i = 0; i < m_zones.size(); i ++)
        {
            unsigned long key = m_zones[i].m_pKey;
            
            if (m_zones[i].m_locationName == stationName)
            {
                // Confirm that for all matching areas, key is in the selected zone list
                Assert(m_hmiSelectionListener->m_zoneList.end() != 
                        m_hmiSelectionListener->m_zoneList.find(key));
            }
            else
            {
                // This zone shouldn't be in result
                Assert(m_hmiSelectionListener->m_zoneList.end() == 
                        m_hmiSelectionListener->m_zoneList.find(key));
            }
        }    
    
        m_hmiSelectionListener->processRunParamChange("Deselect", byStation);    
        confirmAndClearWindowNotification();
        Assert(0 == m_hmiSelectionListener->getZoneList().size());
   }
}


void HmiSelectionListenerTestCases::testProcessRunParamChange_select_deselect_byArea()
{
    // Do a batch selection by area
    std::string byArea = "???.PA." + m_zones[0].m_area + ".???";
    m_hmiSelectionListener->processRunParamChange("Select", byArea);    
    confirmAndClearWindowNotification();

    for (unsigned int i = 0; i < m_zones.size(); i ++)
    {
        unsigned long key = m_zones[i].m_pKey;
        
        if (m_zones[i].m_area == m_zones[0].m_area)
        {
            // Confirm that for all matching areas, key is in the selected zone list
            Assert(m_hmiSelectionListener->m_zoneList.end() != 
                    m_hmiSelectionListener->m_zoneList.find(key));
        }
        else
        {
            // This zone shouldn't be in result
            Assert(m_hmiSelectionListener->m_zoneList.end() == 
                    m_hmiSelectionListener->m_zoneList.find(key));
        }
    }
    m_hmiSelectionListener->processRunParamChange("Deselect", byArea);    
    confirmAndClearWindowNotification();
    Assert(0 == m_hmiSelectionListener->getZoneList().size());
}


void HmiSelectionListenerTestCases::testProcessRunParamChange_select_deselect_single()
{
    // Selects and deselects a single item multiple times, confirming remains in sync
    srand(20);

    std::set<unsigned long> zoneKeys;

    for (unsigned int i = 0; i < 1000; i ++)
    {
        int zoneId = rand() % m_zones.size();
        unsigned long zoneKey = m_zones[zoneId].m_pKey;
        std::string zoneName = getZoneName(zoneId);

        std::set<unsigned long> oldKeys = zoneKeys;

        if (rand() % 2 == 0)
        {
            m_hmiSelectionListener->processRunParamChange("Select", zoneName);    
            zoneKeys.insert(zoneKey);        
        }
        else
        {
            m_hmiSelectionListener->processRunParamChange("Deselect", zoneName);               
            std::set<unsigned long>::iterator itr = zoneKeys.find(zoneKey);
            if (itr != zoneKeys.end())
            {
                zoneKeys.erase(itr);
            }
        }

        // Only get a notification if selection changed
        if (oldKeys != zoneKeys)
        {
            confirmAndClearWindowNotification();
        }

        // Confirm sending through dud parameters is OK
        m_hmiSelectionListener->processRunParamChange("Select", getZoneName(-1));
        m_hmiSelectionListener->processRunParamChange("Deselect", getZoneName(-1));
        confirmAndClearWindowNotification(DOESNTMATTER);
        
        // Confirm selection matches at each stage
        Assert(zoneKeys == m_hmiSelectionListener->m_zoneList);
    }
}

/////////////////////////////////////////////////
//
// processZoneRefresh Tests
//
/////////////////////////////////////////////////
void HmiSelectionListenerTestCases::testProcessZoneRefresh_singleCall()
{
    // Test the zone refresh function - which accepts multiple requests (and strings them together)
    // The format is "count,zone,zone,zone..."
    // NOTE: SELECT is the only expected operation at this stage    
    
    // Send through dud count value - should be disregarded (no expections thrown)
    Assert(!m_hmiSelectionListener->processZoneRefresh("x," + getZoneName(0), HmiSelectionListener::SELECT));    

    std::set<unsigned long> zoneKeys;

    // This should select single zone
    zoneKeys.insert(m_zones[0].m_pKey);
    Assert(m_hmiSelectionListener->processZoneRefresh("1," + getZoneNameFromKey(*zoneKeys.begin()), HmiSelectionListener::SELECT));    
    Assert(zoneKeys == m_hmiSelectionListener->m_zoneList);
    
    zoneKeys.clear();
    // Reset key content (the zone refresh should clear all existing / old keys)
    zoneKeys.insert(m_zones[1].m_pKey);
    zoneKeys.insert(m_zones[2].m_pKey);
    zoneKeys.insert(m_zones[5].m_pKey);
    
    std::ostringstream command;
    command << zoneKeys.size();
    for (std::set<unsigned long>::iterator itr = zoneKeys.begin(); itr != zoneKeys.end(); itr ++)
    {
        command << "," << getZoneNameFromKey(*itr);
    }
    
    // Should select all the zones in set
    Assert(m_hmiSelectionListener->processZoneRefresh(command.str(), HmiSelectionListener::SELECT));    
    Assert(zoneKeys == m_hmiSelectionListener->m_zoneList);
}


void HmiSelectionListenerTestCases::testProcessZoneRefresh_multipleCalls()
{
    // Because the zone refresh may receive a bunch of related runparam changes
    // need to test it correctly links them together
    srand(10);
    for (int iteration = 0; iteration < 10; iteration ++)
    {
        // Set up a random set of zones
        std::set<unsigned long> zoneKeys;
        zoneKeys.insert(m_zones[0].m_pKey);     // Always have one zone
        for (unsigned int i = 1; i < m_zones.size(); i ++)
        {
            if (rand() % 2 == 0)
            {
                continue;
            }
            zoneKeys.insert(m_zones[i].m_pKey);
        }
    
        // Ensure selection clear on each iteration
        m_hmiSelectionListener->processRunParamChange("Deselect", "???");
        unsigned int progress = 0;
        do
        {   
            // Define a split point, defining the number of zones to send through for this processZoneRefresh call
            unsigned int splitPoint = 1 + rand() % 5;

            if (splitPoint > zoneKeys.size() - progress)
            {
                splitPoint = zoneKeys.size() - progress;
            }
            
            std::ostringstream command;
            // Always send through the total count
            command << zoneKeys.size();

            unsigned int count = 0;
            for (std::set<unsigned long>::iterator itr = zoneKeys.begin(); itr != zoneKeys.end(); itr ++)
            {
                // Add in all zones between progress <= counter <= progress + splitPoint
                if (count >= progress && count < (progress + splitPoint) )
                {
                    command << "," << getZoneNameFromKey(*itr);
                }
                count ++;                
            }
        
            progress += splitPoint;

            bool updated = m_hmiSelectionListener->processZoneRefresh(command.str(), HmiSelectionListener::SELECT);    
            
            if (progress < zoneKeys.size())
            { 
                // Until all zones finally get through, zone list should remain inactive
                Assert(!updated);
                Assert(0 == m_hmiSelectionListener->m_zoneList.size());
            }
            else
            {   
                // All changes should be through now once all zone labels have been sent through
                Assert(updated);
                Assert(zoneKeys == m_hmiSelectionListener->m_zoneList);
            }            
        }
        while (progress < zoneKeys.size());
    }
}


/////////////////////////////////////////////////
//
// onZoneSelectionChange Tests
//
/////////////////////////////////////////////////
void HmiSelectionListenerTestCases::testOnZoneSelectionChange()
{
    // Add in all zones initially
    m_hmiSelectionListener->onZoneSelectionChange("???", HmiSelectionListener::SELECT);
    
    // Confirm able to remove zones one by one
    while (m_hmiSelectionListener->m_zoneList.size() > 0)
    {
        unsigned long zoneKey = *m_hmiSelectionListener->m_zoneList.begin();       
		TA_IRS_Bus::PaZoneRecord zone = CachedMaps::getInstance()->getPaZoneCachedMap().getCachedDbRecordFromKey(zoneKey);
                
        std::string zoneName = getZoneName(zone);

        m_hmiSelectionListener->onZoneSelectionChange(zoneName, HmiSelectionListener::DESELECT);
        
        // Confirm this zone no longer exists in selection..
        Assert(m_hmiSelectionListener->m_zoneList.end() == 
                m_hmiSelectionListener->m_zoneList.find(zoneKey));

        // Confirm removing it for a second time won't work
        std::set<unsigned long> originalSel = m_hmiSelectionListener->m_zoneList;
        m_hmiSelectionListener->onZoneSelectionChange(zoneName, HmiSelectionListener::DESELECT);
        // Confirm no notification received, and change in selection
        Assert(originalSel == m_hmiSelectionListener->m_zoneList);    

        // Confirm removing a dud item doesn't work
        m_hmiSelectionListener->onZoneSelectionChange(getZoneName(-1), HmiSelectionListener::DESELECT);
        // Confirm no notification received, and change in selection
        Assert(originalSel == m_hmiSelectionListener->m_zoneList);    
    }
}


/////////////////////////////////////////////////
//
// refreshSelectionUsingZones Tests
//
/////////////////////////////////////////////////
void HmiSelectionListenerTestCases::testRefreshSelectionUsingZones()
{
    srand(10);

    for (int iteration = 0; iteration < 10; iteration ++)
    {
        // Set up a random set of zones
        std::set<unsigned long> zoneKeys;
        std::set<std::string> zoneNames;
        for (unsigned int i = 1; i < m_zones.size(); i ++)
        {
            if (rand() % 2 == 0)
            {
                continue;
            }
            zoneKeys.insert(m_zones[i].m_pKey);
            zoneNames.insert(getZoneName(i, true));
        }
        
        // This function should clear any current selection automatically
        m_hmiSelectionListener->refreshSelectionUsingZones(zoneNames, HmiSelectionListener::SELECT);
        Assert(zoneKeys == m_hmiSelectionListener->m_zoneList);
    }
}


/////////////////////////////////////////////////
//
// clearAllZonesFromSelection Tests
//
/////////////////////////////////////////////////
void HmiSelectionListenerTestCases::testClearAllZonesFromSelection()
{
    m_hmiSelectionListener->processRunParamChange("Select", "???");
    Assert(m_zones.size() == m_hmiSelectionListener->getZoneList().size());
    
    m_hmiSelectionListener->clearAllZonesFromSelection();

    Assert(0 == m_hmiSelectionListener->getZoneList().size());
}


/////////////////////////////////////////////////
//
// addAllZonesToSelection Tests
//
/////////////////////////////////////////////////
void HmiSelectionListenerTestCases::testAddAllZonesToSelection()
{
    Assert(0 == m_hmiSelectionListener->getZoneList().size());
    m_hmiSelectionListener->addAllZonesToSelection();
    Assert(m_zones.size() == m_hmiSelectionListener->getZoneList().size());
}


/////////////////////////////////////////////////
//
// getTokenValue Tests
//
/////////////////////////////////////////////////
void HmiSelectionListenerTestCases::testGetTokenValue()
{
    std::vector<std::string> tokens;
    tokens.push_back("1");
    tokens.push_back("2");
    tokens.push_back("3");
    Assert(tokens[2] == m_hmiSelectionListener->getTokenValue(tokens, PaZoneRecord::ETokenIdentifier(2)) );

    try
    {
        m_hmiSelectionListener->getTokenValue(tokens, PaZoneRecord::ETokenIdentifier(3));
    }
    catch (const TA_Base_Core::InvalidPasConfigurationException&)
    {
        Assert(true);
    }
}


/////////////////////////////////////////////////
//
// modifyZoneSelection Tests
//
/////////////////////////////////////////////////
void HmiSelectionListenerTestCases::testModifyZoneSelection()
{
    std::set<unsigned long> globalSelection;
    
    srand(20);
    for (int iteration = 0; iteration < 10; iteration ++)
    {
        HmiSelectionListener::EOperation op = rand() % 2 == 0 ? 
                                                HmiSelectionListener::SELECT : 
                                                HmiSelectionListener::DESELECT;        

        // Set up a random set of zones
        std::vector<unsigned long> zoneKeys;
        for (unsigned int i = 1; i < m_zones.size(); i ++)
        {
            if (rand() % 2 == 0)
            {
                continue;
            }
            zoneKeys.push_back(m_zones[i].m_pKey);

            if (HmiSelectionListener::SELECT == op)
            {
                globalSelection.insert(m_zones[i].m_pKey);
            }
            else if (HmiSelectionListener::DESELECT == op)
            {
                // Remove from global selection object
                std::set<unsigned long>::iterator itr = globalSelection.find(m_zones[i].m_pKey);
                if (itr != globalSelection.end())
                {
                    globalSelection.erase(itr);
                }
            }
        }
        
        // This function should clear any current selection automatically
        m_hmiSelectionListener->modifyZoneSelection(zoneKeys, op);

        Assert(globalSelection == m_hmiSelectionListener->m_zoneList);
    }
}


/////////////////////////////////////////////////
//
// getSpecifiedZones Tests
//
/////////////////////////////////////////////////
void HmiSelectionListenerTestCases::testGetSpecifiedZones()
{
    std::vector<unsigned long> zoneIds = m_hmiSelectionListener->getSpecifiedZones(GenericCachedMapBase::getAllTokens("DBG.PA.???.???", "."));

    for (unsigned int i = 0; i < m_zones.size(); i ++)
    {
        unsigned long key = m_zones[i].m_pKey;
        if (m_zones[i].m_locationName == "DBG")
        {
            Assert(std::find(zoneIds.begin(), zoneIds.end(), key) != zoneIds.end());
        }
        else
        {
            Assert(std::find(zoneIds.begin(), zoneIds.end(), key) == zoneIds.end());        
        }
    }

    zoneIds = m_hmiSelectionListener->getSpecifiedZones(GenericCachedMapBase::getAllTokens("???.PA.nonpublic.???", "."));

    for (i = 0; i < m_zones.size(); i ++)
    {
        unsigned long key = m_zones[i].m_pKey;
        if (stricmp(m_zones[i].m_area.c_str(), "nonpublic") == 0)
        {
            Assert(std::find(zoneIds.begin(), zoneIds.end(), key) != zoneIds.end());
        }
        else
        {
            Assert(std::find(zoneIds.begin(), zoneIds.end(), key) == zoneIds.end());        
        }
    }

    zoneIds = m_hmiSelectionListener->getSpecifiedZones(GenericCachedMapBase::getAllTokens("???.PA.???.ZoneA", "."));

    for (i = 0; i < m_zones.size(); i ++)
    {
        unsigned long key = m_zones[i].m_pKey;
        if (stricmp(m_zones[i].m_zoneShortName.c_str(), "ZoneA") == 0)
        {
            Assert(std::find(zoneIds.begin(), zoneIds.end(), key) != zoneIds.end());
        }
        else
        {
            Assert(std::find(zoneIds.begin(), zoneIds.end(), key) == zoneIds.end());        
        }
    }

    zoneIds = m_hmiSelectionListener->getSpecifiedZones(GenericCachedMapBase::getAllTokens(getZoneName(5), "."));

    for (i = 0; i < m_zones.size(); i ++)
    {
        unsigned long key = m_zones[i].m_pKey;
        if (getZoneName(i) == getZoneName(5))
        {
            Assert(std::find(zoneIds.begin(), zoneIds.end(), key) != zoneIds.end());
        }
        else
        {
            Assert(std::find(zoneIds.begin(), zoneIds.end(), key) == zoneIds.end());        
        }
    }
}


std::string HmiSelectionListenerTestCases::getZoneName(int i, bool makeLowerCase)
{
    if (i < 0)
    {
        return "OCC.PA.Public.InvalidName";
    }
    std::string result = getZoneName(m_zones[i]);
    if (makeLowerCase)
    {
        std::transform(result.begin(), result.end(), result.begin(), tolower);
    }
    return result;
}


std::string HmiSelectionListenerTestCases::getZoneNameFromKey(unsigned long key)
{
    for (unsigned int i = 0; i < m_zones.size(); i ++)
    {
        if (m_zones[i].m_pKey == key)
        {
            return getZoneName(m_zones[i]);
        }
    }
    throw new TA_Base_Core::TransactiveException("getZoneNameFromKey: Could not find the specified zone");
}


std::string HmiSelectionListenerTestCases::getZoneName(const PaZoneRecord& record)
{
    std::ostringstream result;
    result << record.m_locationName << ".PA." << record.m_area << "." << record.m_zoneShortName;
    return result.str();
}


void HmiSelectionListenerTestCases::confirmAndClearWindowNotification(ENotification type)
{
    const WindowStub::MessageQueue data = m_windowStub.getReceived();
        
    // Confirm window got notification of new selection
    if (CHANGEEXPECTED == type)
    {
        Assert(1 == data.size());
        Assert(WM_PA_STN_HMI_ZONE_SEL_CHANGE == data[0].message);
    }
    else if (NOCHANGEEXPECTED == type)
    {
        Assert(0 == data.size());
    }

    m_windowStub.reset();
}


void HmiSelectionListenerTestCases::setUpSuite( const std::string & inFile ) 
{
    TA_App::PaTest::setToValidLocation();

    m_hmiSelectionListener = new HmiSelectionListener(m_windowStub.getHWnd());

    // Create some custom zones for these unit tests
    PaZoneCachedMap& zoneMap = CachedMaps::getInstance()->getPaZoneCachedMap();
    zoneMap.clearKeyToRecordMap();

    m_zones.push_back(createRecord(1, "OCC", "Public", "ZoneA"));
    m_zones.push_back(createRecord(1, "OCC", "Public", "ZoneB"));
    m_zones.push_back(createRecord(1, "OCC", "Public", "ZoneC"));
    m_zones.push_back(createRecord(1, "OCC", "NonPublic", "ZoneA"));
    m_zones.push_back(createRecord(1, "OCC", "NonPublic", "ZoneB"));
    m_zones.push_back(createRecord(1, "OCC", "NonPublic", "ZoneC"));
    m_zones.push_back(createRecord(3, "DBG", "Public", "ZoneA"));
    m_zones.push_back(createRecord(3, "DBG", "Public", "ZoneB"));
    m_zones.push_back(createRecord(3, "DBG", "Public", "ZoneC"));
    m_zones.push_back(createRecord(3, "DBG", "NonPublic", "ZoneA"));
    m_zones.push_back(createRecord(3, "DBG", "NonPublic", "ZoneB"));
    m_zones.push_back(createRecord(3, "DBG", "NonPublic", "ZoneC"));
    m_zones.push_back(createRecord(4, "MSM", "Public", "ZoneA"));
    m_zones.push_back(createRecord(4, "MSM", "Public", "ZoneB"));
    m_zones.push_back(createRecord(4, "MSM", "Public", "ZoneC"));
    m_zones.push_back(createRecord(4, "MSM", "NonPublic", "ZoneA"));
    m_zones.push_back(createRecord(4, "MSM", "NonPublic", "ZoneB"));
    m_zones.push_back(createRecord(4, "MSM", "NonPublic", "ZoneC"));    

    for (unsigned int i = 0; i < m_zones.size(); i ++)
    {
        zoneMap.updateRecordMapWithItem(m_zones[i].m_pKey, m_zones[i]);
    }

    // Add in some 'non pa-manager specific' zones - but don't add them to local m_zones array
    PaZoneRecord zone = createRecord(5, "CVC", "Public", "ZoneA");
    zone.m_entityKey = TA_Base_Core::DatabaseKey::getInvalidKey();
    zoneMap.updateRecordMapWithItem(zone.m_pKey, zone);

    zone = createRecord(5, "CVC", "NonPublic", "ZoneB");
    zone.m_entityKey = TA_Base_Core::DatabaseKey::getInvalidKey();
    zoneMap.updateRecordMapWithItem(zone.m_pKey, zone);

    zone = createRecord(5, "OCC", "NonPublic", "ZoneD");
    zone.m_entityKey = TA_Base_Core::DatabaseKey::getInvalidKey();
    zoneMap.updateRecordMapWithItem(zone.m_pKey, zone);

    zoneMap.flagAsInitialised();    
}


PaZoneRecord HmiSelectionListenerTestCases::createRecord(unsigned long locationKey, 
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

void HmiSelectionListenerTestCases::tearDownSuite() 
{
    TA_App::PaTest::restoreLocation();

    delete m_hmiSelectionListener;
    m_hmiSelectionListener = 0;
}


void HmiSelectionListenerTestCases::setUp( const std::string & INFile , const std::string & OUTFile )
{
    // Clear entire selection
    m_hmiSelectionListener->processRunParamChange("Deselect", "???");

    // Keep the window stub free of messages at beginning of each test
    // NOTE: Must clear window stub last (otherwise receives a selection change notification)
    m_windowStub.reset();
}


TestSuite * HmiSelectionListenerTestCases::suite()
{
    TestSuite * testSuite = new TestSuite( "HmiSelectionListenerTestCases" );  

// Use temporary hashdef to add tests
#define ADD_UNIT_TEST(function) \
    testSuite->addTest( new TestCaller< HmiSelectionListenerTestCases >(\
                        #function,\
                        &HmiSelectionListenerTestCases::##function,this ) );

    //
    // constructor tests
    //
    ADD_UNIT_TEST(testConstructor);
    
    //
    // getZoneList tests
    //
    ADD_UNIT_TEST(testGetZoneList);    
    
    //
    // processRunParamChange tests
    //
    ADD_UNIT_TEST(testProcessRunParamChange_select_deselect);
    ADD_UNIT_TEST(testProcessRunParamChange_select_deselect_single);
    ADD_UNIT_TEST(testProcessRunParamChange_select_deselect_byStation);
    ADD_UNIT_TEST(testProcessRunParamChange_select_deselect_byArea);
    ADD_UNIT_TEST(testProcessRunParamChange_select_deselect_byZoneName);

    
    //
    // processZoneRefresh tests
    //
    ADD_UNIT_TEST(testProcessZoneRefresh_singleCall);
    ADD_UNIT_TEST(testProcessZoneRefresh_multipleCalls);

    
    //
    // onZoneSelectionChange tests
    //
    ADD_UNIT_TEST(testOnZoneSelectionChange);

    
    //
    // refreshSelectionUsingZones tests
    //
    ADD_UNIT_TEST(testRefreshSelectionUsingZones);
    
    //
    // clearAllZonesFromSelection tests
    //
    ADD_UNIT_TEST(testClearAllZonesFromSelection);
    
    //
    // addAllZonesToSelection tests
    //
    ADD_UNIT_TEST(testAddAllZonesToSelection);
    
    //
    // getTokenValue tests
    //
    ADD_UNIT_TEST(testGetTokenValue);
    
    //
    // modifyZoneSelection tests
    //
    ADD_UNIT_TEST(testModifyZoneSelection);
    
    //
    // getSpecifiedZones tests
    //
    ADD_UNIT_TEST(testGetSpecifiedZones);

#undef ADD_UNIT_TEST

	return testSuite;
}