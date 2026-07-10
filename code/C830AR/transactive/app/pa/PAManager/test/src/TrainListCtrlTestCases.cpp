/**
  * The source code in this file is the property of
  * Ripple Systems and is not for redistribution
  * in any form.
  *
  * Source:   $File: //depot/4669_T01271350/4669/transactive/app/pa/PAManager/test/src/TrainListCtrlTestCases.cpp $
  * @author:  Ripple
  * @version: $Revision: #1 $
  *
  * Last modification: $DateTime: 2008/11/28 16:26:01 $
  * Last modified by:  $Author: builder $
  *
  */
#pragma warning (disable : 4786 4290)

#include "StdAfx.h"

#include <deque>

#include "UnitTestAccessBridge.h"
#include "TrainListCtrlTestCases.h"
#include "common.h"
#include "TestUtils.h"

#include "app/pa/PAManager/src/TrainListCtrl.h"
#include "bus/pa/cachedmaps/src/CachedMaps.h"

#include <sstream>

using namespace TA_App;
using namespace TA_Base_Bus;

enum { LOCATIONS_SET_UP = 4 };

TrainListCtrlTestCases::TrainListCtrlTestCases ( const std::string & name ) 
: 
TestCase ( name ),
m_trainListCtrl(0),
m_windowStub(WC_LISTVIEW)       // Required to process messages correctly
{
}

/////////////////////////////////////////////////
//
// updateTrainList Tests
//
/////////////////////////////////////////////////
void TrainListCtrlTestCases::testUpdateTrainList()
{   
    for (int iteration = 0; iteration < 2; iteration ++)
    {
        TrainListCtrl& list = getNewListCtrl();
        std::vector<TA_Base_Bus::ITrainInformationCorbaDef::TrainDetailsNotification> data;
        // Add in one more train than location, to have invalid locations
        // (which should be handled by TrainListCtrl OK)
        enum { TRAIN_ID_MAX = LOCATIONS_SET_UP + 1};
    
        // Add in 5 trains
        for (UINT i = 0; i < TRAIN_ID_MAX; i ++)
        {
            TA_Base_Bus::ITrainInformationCorbaDef::TrainDetailsNotification td = getTd();
            // Reverse the order of train IDs as we add items to list
            // (to check the sorting order)
            td.trainID = i + 1;
            td.currentLocation = i + 1;
            td.serviceNumber = CORBA::string_dup("1");
            list.updateTrainList(TA_IRS_Bus::ITrainSelectorGUI::Add, td);
            data.push_back(td);
        }
                
        verifyListContains(list, data);
    
        data.clear();
    
        // Now update elements of the train list
        for (i = 0; i < TRAIN_ID_MAX; i ++)
        {
            TA_Base_Bus::ITrainInformationCorbaDef::TrainDetailsNotification td = getTd();
            td.trainID = i + 1;
            // Change the current location and service number
            td.currentLocation = 1 + ((i + 2) % LOCATIONS_SET_UP);
            td.serviceNumber = CORBA::string_dup("2");
            
            list.updateTrainList(TA_IRS_Bus::ITrainSelectorGUI::Update, td);
            data.push_back(td);      
        }
    
        TA_Base_Bus::ITrainInformationCorbaDef::TrainDetailsNotification invalidUpdate;
        invalidUpdate.trainID = TRAIN_ID_MAX + 1;
        invalidUpdate.currentLocation = 1;
        invalidUpdate.serviceNumber = CORBA::string_dup("1");
    
        // Try to update a train that isn't already in list
        list.updateTrainList(TA_IRS_Bus::ITrainSelectorGUI::Update, invalidUpdate);
        // An error message will have been displayed, confirm it came through
        Assert(TestUtils::confirmErrorReceivedAndClearWindowStub(
                            PaErrorHandler::INVALID_TRAIN_DATA, 
                            m_errorStub));    
    
        verifyListContains(list, data);
    
        // Now remove the locations
        if (0 == iteration)
        {
            for (i = 0; i < TRAIN_ID_MAX; i ++)
            {
                TA_Base_Bus::ITrainInformationCorbaDef::TrainDetailsNotification td = getTd();
                td.trainID = i + 1;
                
                list.updateTrainList(TA_IRS_Bus::ITrainSelectorGUI::Delete, td);

                // Pop the first element (that's the one we're removing)
                data.erase(data.begin());
                    
                // Verify the list is up to date
                verifyListContains(list, data);
                
                // Try and remove item a second time, should display error
                list.updateTrainList(TA_IRS_Bus::ITrainSelectorGUI::Delete, td);
                Assert(TestUtils::confirmErrorReceivedAndClearWindowStub(
                            PaErrorHandler::INVALID_TRAIN_DATA, 
                            m_errorStub));    
                
                // List should be unaffected
                verifyListContains(list, data);
            }
        }
        else
        {
            TA_Base_Bus::ITrainInformationCorbaDef::TrainDetailsNotification td = getTd();
            // Use the clearAll function
            list.updateTrainList(TA_IRS_Bus::ITrainSelectorGUI::ClearAll, td);
        }   
        
        // Confirm all entries cleared
        Assert(0 == list.GetItemCount());        
    }
}


/////////////////////////////////////////////////
//
// findSortedInsertionPoint Tests
//
/////////////////////////////////////////////////
void TrainListCtrlTestCases::testFindSortedInsertionPoint()
{
    TrainListCtrl& list = getNewListCtrl();
 
    enum { TRAIN_ID_MAX = LOCATIONS_SET_UP};
    
    // Add in the trains in reverse order to verify sorting
    for (int i = 0; i < TRAIN_ID_MAX; i ++)
    {
        TA_Base_Bus::ITrainInformationCorbaDef::TrainDetailsNotification td = getTd();
        // Reverse the order of train IDs as we add items to list
        // (to check the sorting order)
        td.trainID = ((TRAIN_ID_MAX - 1) - i) + 1;
        td.currentLocation = i + 1;
        td.serviceNumber = CORBA::string_dup("1");
        list.updateTrainList(TA_IRS_Bus::ITrainSelectorGUI::Add, td);
    }

    // all items should be in ascending order of trainId
    for (i = 0; i < list.GetItemCount(); i ++)
    {
        if (i + 1 < list.GetItemCount())
        {
            Assert(list.GetItemData(i) < list.GetItemData(i + 1));
        }
    }

    // Verify the function directly
    for (i = 0; i < list.GetItemCount(); i ++)
    {   
        TA_Base_Bus::ITrainInformationCorbaDef::TrainDetailsNotification td = getTd();
        td.trainID = (UINT)(list.GetItemData(i) - 1);     
        Assert(list.findSortedInsertionPoint(td) == i);

        td.trainID = (UINT)(list.GetItemData(i));     
        Assert(list.findSortedInsertionPoint(td) == i + 1);
    }
}


/////////////////////////////////////////////////
//
// findTrainIndex Tests
//
/////////////////////////////////////////////////
void TrainListCtrlTestCases::testFindTrainIndex()
{
    TrainListCtrl& list = getNewListCtrl();
    enum { TRAIN_ID_MAX = 5 };
    int trainIds[] = { 5, 15, 10, 25, 20 };

    for (UINT i = 0; i < TRAIN_ID_MAX; i ++)
    {
        TA_Base_Bus::ITrainInformationCorbaDef::TrainDetailsNotification td = getTd();
        td.trainID = trainIds[i];
        td.currentLocation = i + 1;
        td.serviceNumber = CORBA::string_dup("1");
        list.updateTrainList(TA_IRS_Bus::ITrainSelectorGUI::Add, td);
    }
    
    Assert(list.GetItemCount() == TRAIN_ID_MAX);

    for (i = 0; i < TRAIN_ID_MAX; i ++)
    {
        TA_Base_Bus::ITrainInformationCorbaDef::TrainDetailsNotification td = getTd();
        td.trainID = trainIds[i];

        int index = list.findTrainIndex(td);
        DWORD foundId = list.GetItemData(index);
        Assert((int)foundId == trainIds[i]);
    }

    // Now confirm finding a dud train ID returns -1
    TA_Base_Bus::ITrainInformationCorbaDef::TrainDetailsNotification td = getTd();
    td.trainID = 1;
    int index = list.findTrainIndex(td);
    Assert(-1 == index);
}


/////////////////////////////////////////////////
//
// getLocationString Tests
//
/////////////////////////////////////////////////
void TrainListCtrlTestCases::testGetLocationString()
{
    srand(50);

    TrainListCtrl& list = getNewListCtrl();
    
    TA_Base_Bus::ITrainInformationCorbaDef::TrainDetailsNotification td = getTd();
    
    for (UINT i = 0; i < 10; i ++)
    {
        UINT locationKey = 1 + (rand() % LOCATIONS_SET_UP);
        td.currentLocation = locationKey;    
        Assert(list.getLocationString(td) == 
			TA_IRS_Bus::CachedMaps::getInstance()->getLocationNameFromKey(locationKey).c_str());
    }
    
    td.currentLocation = 15;
    // This next location shouldn't exist
    Assert(list.getLocationString(td) == "?");    
}


/////////////////////////////////////////////////
//
// getServiceString Tests
//
/////////////////////////////////////////////////
void TrainListCtrlTestCases::testGetServiceString()
{
    srand(50);

    TrainListCtrl& list = getNewListCtrl();
    TA_Base_Bus::ITrainInformationCorbaDef::TrainDetailsNotification td = getTd();

    for (UINT i = 0; i < 10; i ++)
    {
        std::ostringstream ostr;
        ostr << (rand() % 50);
        
        td.serviceNumber = CORBA::string_dup(ostr.str().c_str());

        Assert(list.getServiceString(td) == ostr.str().c_str());
    }
}


/////////////////////////////////////////////////
//
// getTrainIdString Tests
//
/////////////////////////////////////////////////
void TrainListCtrlTestCases::testGetTrainIdString()
{
    srand(50);

    TrainListCtrl& list = getNewListCtrl();
    
    for (UINT i = 0; i < 10; i ++)
    {   
        TA_Base_Bus::ITrainInformationCorbaDef::TrainDetailsNotification td = getTd();
        std::ostringstream ostr;
        td.trainID = rand() % 50;
        ostr << ((int)td.trainID);   
        std::string str = ostr.str();  
        
        Assert(list.getTrainIdString(td) == str.c_str());
    }
}


/////////////////////////////////////////////////
//
// mixed selection Tests
//
/////////////////////////////////////////////////
void TrainListCtrlTestCases::testSelectionUpdating()
{
    TrainListCtrl& list = getNewListCtrl();
    enum { TRAIN_ID_MAX = 5 };
    int trainIds[] = { 5, 15, 10, 25, 20 };

    for (int i = 0; i < TRAIN_ID_MAX; i ++)
    {
        TA_Base_Bus::ITrainInformationCorbaDef::TrainDetailsNotification td = getTd();
        td.trainID = trainIds[i];
        td.currentLocation = i + 1;
        td.serviceNumber = CORBA::string_dup("1");
        list.updateTrainList(TA_IRS_Bus::ITrainSelectorGUI::Add, td);
    }
    
    Assert(list.GetItemCount() == TRAIN_ID_MAX);

    //////////////////////
    // testSelectAll
    list.selectAll();
    for (i = 0; i < list.GetItemCount(); i ++)
    {
        Assert(LVIS_SELECTED == list.GetItemState(i, LVIS_SELECTED));
    }
    
    //////////////////////
    // testDeselectAll
    list.deselectAll();
    for (i = 0; i < list.GetItemCount(); i ++)
    {
        Assert(0 == list.GetItemState(i, LVIS_SELECTED));
    }

    //////////////////////
    // testGetSelectedTrainIds
    // This tests for TD Defect #3315
    list.selectAll();
    std::vector<unsigned int> selection = list.getSelectedTrainIds();
    
    for (UINT trainIndex = 0; trainIndex < TRAIN_ID_MAX; trainIndex ++)
    {
        // Progressively clear the known train IDs from returned array
        // (in doing so confirming each train ID was returned)
        UINT trainId = trainIds[trainIndex];

        std::vector<unsigned int>::iterator itr = 
                std::find(selection.begin(), selection.end(), trainId);

        Assert(itr != selection.end());
        selection.erase(itr);
    }

    Assert(0 == selection.size());

    //////////////////////
    // testSetSelectedTrains
    {
        // Select all trains
        selection.clear();
        // Add in all the trains
        for (i = 0; i < TRAIN_ID_MAX; i ++)
        {
            selection.push_back(trainIds[i]);
        }
        list.setSelectedTrains(selection);
        for (i = 0; i < list.GetItemCount(); i ++)
        {
            Assert(LVIS_SELECTED == list.GetItemState(i, LVIS_SELECTED));
        }
    }

    {
        // Now select a single item
        selection.clear();
        enum { SEL_INDEX = 1 };
        selection.push_back(trainIds[SEL_INDEX]);
        
        // Don't deselect first - should occur automatically with setSelectedTrains
        list.setSelectedTrains(selection);
        for (i = 0; i < list.GetItemCount(); i ++)
        {
            bool isSelected = ((DWORD)trainIds[SEL_INDEX] == list.GetItemData(i));
            
            if (isSelected)
            {
                Assert(LVIS_SELECTED == list.GetItemState(i, LVIS_SELECTED));
            }
            else
            {
                Assert(0 == list.GetItemState(i, LVIS_SELECTED));
            }
        }
    }
}


void TrainListCtrlTestCases::tearDownSuite() 
{
    TA_App::PaTest::restoreLocation();

    deleteListCtrl();

    PaErrorHandler::setPrimaryWindowHandle(0);
}

void TrainListCtrlTestCases::setUpSuite( const std::string & inFile ) 
{
    TA_App::PaTest::setToValidLocation();
    UnitTestAccessBridge::setupSomeLocations(LOCATIONS_SET_UP);

    PaErrorHandler::setPrimaryWindowHandle(m_errorStub.getHWnd());
}


void TrainListCtrlTestCases::setUp( const std::string & INFile , const std::string & OUTFile )
{    
}


void TrainListCtrlTestCases::tearDown( const std::string & INFile ) 
{
}


void TrainListCtrlTestCases::deleteListCtrl()
{
    if (0 != m_trainListCtrl)
    {        
        // Must delete all the underlying items to refresh properly
        m_trainListCtrl->DeleteAllItems();
        m_trainListCtrl->Detach();
        
        delete m_trainListCtrl;
        m_trainListCtrl = 0;
        
        WindowStub::processMsgQueue();        
    }
}


TrainListCtrl& TrainListCtrlTestCases::getNewListCtrl()
{
    deleteListCtrl();
    
    m_trainListCtrl = new TrainListCtrl();
    m_trainListCtrl->Attach(m_windowStub.getHWnd());

    // This is required to get the item text coming through correctly
    // (there are three columns in the list control)
    m_trainListCtrl->InsertColumn(0, "");
    m_trainListCtrl->InsertColumn(1, "");
    m_trainListCtrl->InsertColumn(2, "");

    WindowStub::processMsgQueue();
    m_windowStub.reset();
    
    return *m_trainListCtrl;
}


void TrainListCtrlTestCases::verifyListContains
(
 TrainListCtrl& list, 
 std::vector<TA_Base_Bus::ITrainInformationCorbaDef::TrainDetailsNotification>& data
)
{    
    int itemCount = list.GetItemCount();

    Assert(itemCount == (int)data.size());

    // Verify all contained in list control
    // (first element of each item should be literal location)
    for (UINT i = 0; i < (UINT)list.GetItemCount(); i ++)
    {
        DWORD trainId = list.GetItemData(i);
        TA_Base_Bus::ITrainInformationCorbaDef::TrainDetailsNotification* td = 0;

        for (UINT t = 0; t < data.size(); t ++)
        {
            if (data[t].trainID == trainId)
            {
                td = &data[t];
                break;
            }
        }

        // Verify the item data component is the unique train ID
        Assert(0 != td);
        
        CString rString;

        // --This next chunk tests for TD Defect #3315--
        {
            rString = list.GetItemText( i, 0);
            
            std::string locationStr;
    
            try
            {
                locationStr = CachedMaps::getInstance()->getLocationNameFromKey(td->currentLocation);
            }
            catch(TA_Base_Core::CachedMappingNotFoundException& )
            {
                // unknown location - handled OK
                locationStr = "?";
            }
    
            Assert(locationStr == rString.GetBuffer(0));
        }

        // Next element is service string
        rString = list.GetItemText(i, 1);
        Assert(rString == list.getServiceString(*td));
        
        // Next is train ID string
        rString = list.GetItemText(i, 2);
        Assert(rString == list.getTrainIdString(*td));
    
    }
}


TA_Base_Bus::ITrainInformationCorbaDef::TrainDetailsNotification TrainListCtrlTestCases::getTd()
{
    // If we don't initialise these classes to a proper starting state, they die during
    // destruction because CORBA attempts to delete some uninitialised string members
    TA_Base_Bus::ITrainInformationCorbaDef::TrainDetailsNotification result;
    result.currentLocation = 0;
    result.isInbound = false;
    result.isMute = false;
    result.isValid = false;
    result.ITSINumber._ptr = 0;
    result.serviceNumber._ptr = 0;
    result.trainID = 0;

    return result;
}


TestSuite * TrainListCtrlTestCases::suite()
{
    TestSuite * testSuite = new TestSuite( "TrainListCtrlTestCases" );  
    
// Use temporary hashdef to add tests
#define ADD_UNIT_TEST(function) \
    testSuite->addTest( new TestCaller< TrainListCtrlTestCases >(\
                        #function,\
                        &TrainListCtrlTestCases::##function,this ) );

    //
    // updateTrainList tests
    //
    ADD_UNIT_TEST(testUpdateTrainList);

    //
    // findSortedInsertionPoint tests
    //
    ADD_UNIT_TEST(testFindSortedInsertionPoint);

    //
    // findTrainIndex tests
    //
    ADD_UNIT_TEST(testFindTrainIndex);

    //
    // getLocationString tests
    //
    ADD_UNIT_TEST(testGetLocationString);

    //
    // getServiceString tests
    //
    ADD_UNIT_TEST(testGetServiceString);

    //
    // getTrainIdString tests
    //
    ADD_UNIT_TEST(testGetTrainIdString);

    //
    // mixed selection tests
    //
    ADD_UNIT_TEST(testSelectionUpdating);


#undef ADD_UNIT_TEST

	return testSuite;
}
