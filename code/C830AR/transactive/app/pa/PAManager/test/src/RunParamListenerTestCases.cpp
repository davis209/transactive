/**
  * The source code in this file is the property of
  * Ripple Systems and is not for redistribution
  * in any form.
  *
  * Source:   $File: //depot/4669_T01271350/4669/transactive/app/pa/PAManager/test/src/RunParamListenerTestCases.cpp $
  * @author:  Ripple
  * @version: $Revision: #1 $
  *
  * Last modification: $DateTime: 2008/11/28 16:26:01 $
  * Last modified by:  $Author: builder $
  *
  */
#pragma warning (disable : 4786)

#include "StdAfx.h"
#include "RunParamListenerTestCases.h"
#include "core/utilities/src/DebugUtil.h"
#include "core/utilities/src/TaAssert.h"
#include "app/pa/PAManager/src/RunParamListener.h"
#include "app/pa/PAManager/src/CachedConfig.h"
#include "app/pa/PAManager/src/IRunParamChangedListener.h"
#include "core/utilities/src/RunParams.h"
#include "Common.h"

// We must use one of the already registed run param listener tags
const std::string TEST_PARAM_1 = "Select";
const std::string TEST_PARAM_2 = "Display";
const std::string TEST_PARAM_3 = "Deselect";
const std::string TEST_PARAM_4 = "SynchroniseSelected";
const std::string BAD_TEST_PARAM = "Blah";
const std::string TEST_VALUE_1 = "TestValue";
const std::string TEST_VALUE_2 = "TestValue2";
const std::string TEST_VALUE_3 = "TestValue3";

using namespace TA_Base_Core;

// Place out here to improve compile efficiency
static CachedConfig::ELocationType g_originalLocation;

//////////////////////////////
class DummyListener : public IRunParamChangedListener
{
public:
    typedef std::pair <std::string, std::string> StringPair;

    void processRunParamChange(const std::string& name, const std::string& value)
    {
        received.push_back(StringPair(name, value));
    }
    std::vector<StringPair> received;
};
//////////////////////////////


void RunParamListenerTestCases::setUpSuite( const std::string & inFile ) 
{
    // Must attach to an identity before working with it
    RunParamListener::getInstance().retrieveInitialRunParamIdentity();

    // Must set the location for some of these tests
    TA_App::PaTest::setToValidLocation();
}


void RunParamListenerTestCases::tearDownSuite() 
{
    TA_App::PaTest::restoreLocation();
}
   

void RunParamListenerTestCases::tearDown( const std::string & INFile )
{
    // Confirm all listeners de-registered after each method
    for (unsigned int i = 0; i < RunParamListener::getInstance().m_registeredTags.size(); i ++)
    {
        // This can occur when a test fails - the remaining tests will fail as a result..
        Assert(RunParamListener::getInstance().m_registeredTags[i].m_listener == 0);
        // Clear any queued updates
        RunParamListener::getInstance().m_registeredTags[i].m_valueChanges.clear();
    }
}
    
/////////////////////////////////////////////////
//
// processInternalRunParamChange Tests
//
/////////////////////////////////////////////////
void RunParamListenerTestCases::testProcessInternalRunParamChange()
{
    DummyListener listener;

    try
    {
        // Confirm that when the run param listener receives a run param change
        // it is propogated to registered
        RunParamListener::getInstance().registerListener(TEST_PARAM_1, listener);
    
        RunParamListener::getInstance().processInternalRunParamChange(TEST_PARAM_2, "Blah");
            
        // The listener shouldn't have received this one
        Assert(0 == listener.received.size());
    
        for (int i = 0; i < 10; i ++)
        {
            RunParamListener::getInstance().processInternalRunParamChange(TEST_PARAM_1, TEST_VALUE_1);
    
            Assert(i + 1 == (int)listener.received.size());
        
            // Confirm the data it received corresponds
            Assert(listener.received[i].first == TEST_PARAM_1);
            Assert(listener.received[i].second == TEST_VALUE_1);
        }
    }
    catch (...)
    {
        RunParamListener::getInstance().deRegisterListener(listener);
        throw;
    }

    RunParamListener::getInstance().deRegisterListener(listener);
}


/////////////////////////////////////////////////
//
// registerListener Tests
//
/////////////////////////////////////////////////
void RunParamListenerTestCases::testRegisterListener()
{   
    int i;

    // How many additional messages to pump through
    enum { OVERFLOW = 10 };

    // Registering a new listener should cause us to receive up to RunParamListener::MAX_VALUE_CHANGES 
    // buffered changes (the most recent) - if we perform a delayed registration
    for (i = 0; i < RunParamListener::MAX_VALUE_CHANGES + OVERFLOW; i ++)
    {
        std::ostringstream ostr;
        ostr << TEST_VALUE_1 << i;
        RunParamListener::getInstance().processInternalRunParamChange(TEST_PARAM_1, ostr.str());
        RunParamListener::getInstance().processInternalRunParamChange(TEST_PARAM_2, "Blah");
    }

    // Confirm that the value change buffer is limited to RunParamListener::MAX_VALUE_CHANGES
    RunParamListener::RunParamChangeStruct* rs = RunParamListener::getInstance().findChangeStruct(TEST_PARAM_2);
    Assert(rs->m_valueChanges.size() == RunParamListener::MAX_VALUE_CHANGES);
    
    DummyListener listener;
    RunParamListener::getInstance().registerListener(TEST_PARAM_1, listener);
    RunParamListener::getInstance().registerListener(TEST_PARAM_3, listener);
    
    // Now listener should contain values up to and < than 'RunParamListener::MAX_VALUE_CHANGES + OVERFLOW'
    for (i = OVERFLOW; i < RunParamListener::MAX_VALUE_CHANGES + OVERFLOW; i ++)
    {
        std::ostringstream ostr;
        ostr << TEST_VALUE_1 << i;
        // Confirm the buffer contains only messages from 'OVERFLOW'
        int bufIndex = i - OVERFLOW;
        Assert(bufIndex < (int)listener.received.size());
        Assert(listener.received[bufIndex].first == TEST_PARAM_1);
        Assert(listener.received[bufIndex].second == ostr.str());
    }

    listener.received.clear();
    
    // Now dispatch some more messages
    RunParamListener::getInstance().processInternalRunParamChange(TEST_PARAM_1, TEST_VALUE_2);
    RunParamListener::getInstance().processInternalRunParamChange(TEST_PARAM_2, TEST_VALUE_1);
    RunParamListener::getInstance().processInternalRunParamChange(TEST_PARAM_3, TEST_VALUE_3);
    
    Assert(2 == listener.received.size());
    Assert(listener.received[0].first == TEST_PARAM_1);
    Assert(listener.received[0].second == TEST_VALUE_2);
    Assert(listener.received[1].first == TEST_PARAM_3);
    Assert(listener.received[1].second == TEST_VALUE_3);


    // test that cannot add a second listener on same tag - should trip precondition check
    try
    {
        DummyListener listener2;
        RunParamListener::getInstance().registerListener(TEST_PARAM_1, listener2);
        Assert(false);
    }
    catch (...)
    {
        Assert(true);
    }

    // Confirm cannot register to an unrecognized tag name
    try
    {
        RunParamListener::getInstance().registerListener(BAD_TEST_PARAM, listener);
        Assert(false);
    }
    catch (...)
    {
        Assert(true);
    }

    RunParamListener::getInstance().deRegisterListener(listener);
}


/////////////////////////////////////////////////
//
// deRegisterListener Tests
//
/////////////////////////////////////////////////
void RunParamListenerTestCases::testDeRegisterListener()
{
    DummyListener listener, listener2, listener3;
    RunParamListener::getInstance().registerListener(TEST_PARAM_1, listener);
    RunParamListener::getInstance().registerListener(TEST_PARAM_2, listener);
    
    RunParamListener::getInstance().registerListener(TEST_PARAM_3, listener2);
    RunParamListener::getInstance().registerListener(TEST_PARAM_4, listener3);
   
    // Should receive these updates
    RunParamListener::getInstance().processInternalRunParamChange(TEST_PARAM_1, TEST_VALUE_1);
    RunParamListener::getInstance().processInternalRunParamChange(TEST_PARAM_2, TEST_VALUE_1);
    Assert(2 == listener.received.size());
    listener.received.clear();


    // Remove centre listener
    {
        Assert(0 == listener2.received.size());
        RunParamListener::getInstance().processInternalRunParamChange(TEST_PARAM_3, TEST_VALUE_1);
        Assert(1 == listener2.received.size());
        RunParamListener::getInstance().deRegisterListener(listener2);
    }

    // If we deregister the listener, should no longer receive updates (for anything)
    RunParamListener::getInstance().deRegisterListener(listener);

    RunParamListener::getInstance().processInternalRunParamChange(TEST_PARAM_1, TEST_VALUE_1);
    RunParamListener::getInstance().processInternalRunParamChange(TEST_PARAM_2, TEST_VALUE_1);

    // Shouldn't receive these updates
    RunParamListener::getInstance().processInternalRunParamChange(TEST_PARAM_1, TEST_VALUE_1);
    RunParamListener::getInstance().processInternalRunParamChange(TEST_PARAM_2, TEST_VALUE_1);
    Assert(0 == listener.received.size());

    Assert(0 == listener3.received.size());
    RunParamListener::getInstance().processInternalRunParamChange(TEST_PARAM_4, TEST_VALUE_1);
    Assert(1 == listener3.received.size());
    RunParamListener::getInstance().deRegisterListener(listener3);
}


/////////////////////////////////////////////////
//
// retrieveInitialRunParamIdentity Tests
//
/////////////////////////////////////////////////
void RunParamListenerTestCases::testRetrieveInitialRunParamIdentity()
{   
    // String in format of <STN>,<INT>,<anything>
    const std::string identityStr = "AAA,0,";
    const std::string rParamIdentityStr = identityStr + "somedata,gfsnhg,at";
    
    for (int i = 0; i < 4; i ++)
    {
        RunParams::getInstance().set(TEST_PARAM_1.c_str(), "");
        RunParams::getInstance().set(TEST_PARAM_2.c_str(), "");
        RunParams::getInstance().set(TEST_PARAM_3.c_str(), "");
        RunParams::getInstance().set(TEST_PARAM_4.c_str(), "");

        // Shouldnt' have an identity by default
        Assert( !RunParamListener::getInstance().retrieveInitialRunParamIdentity() );   
        
        //## test for findStringIdentifier
        Assert(0 == RunParamListener::getInstance().findIdentityString().length());
        //##

        switch (i)
        {
        case 0:
            RunParams::getInstance().set(TEST_PARAM_1.c_str(), rParamIdentityStr.c_str());
            break;
        case 1:
            RunParams::getInstance().set(TEST_PARAM_2.c_str(), rParamIdentityStr.c_str());
            break;
        case 2:
            RunParams::getInstance().set(TEST_PARAM_3.c_str(), rParamIdentityStr.c_str());
            break;
        case 3:
            RunParams::getInstance().set(TEST_PARAM_4.c_str(), rParamIdentityStr.c_str());
            break;
        };
        
        //## test for findStringIdentifier
        Assert(identityStr == RunParamListener::getInstance().findIdentityString());
        //##

        // Should be able to determine initial runparam identity
        Assert(RunParamListener::getInstance().retrieveInitialRunParamIdentity());
        // Internally set identifier string should match
        Assert(RunParamListener::getInstance().m_identityString.getValue() == identityStr);
    }

    RunParams::getInstance().set(TEST_PARAM_1.c_str(), "");
    RunParams::getInstance().set(TEST_PARAM_2.c_str(), "");
    RunParams::getInstance().set(TEST_PARAM_3.c_str(), "");
    RunParams::getInstance().set(TEST_PARAM_4.c_str(), "");

    // Reset the classes identity for other tests
    RunParamListener::getInstance().setNewIdentityFromIdentifier("");
}


/////////////////////////////////////////////////
//
// findVirtualLocation Tests
//
/////////////////////////////////////////////////
void RunParamListenerTestCases::testFindVirtualLocation()
{
    const std::string location = "OCC";
    const std::string identityStr = location + ",26378,sadg";
  
    RunParams::getInstance().set(TEST_PARAM_1.c_str(), identityStr.c_str());
    
    // Confirm the runparams class correctly identifies the location from the parameter
    std::string foundLocation = RunParamListener::getInstance().findVirtualLocation();
    
    Assert(location == foundLocation);

    RunParams::getInstance().set(TEST_PARAM_1.c_str(), "");  
}

/////////////////////////////////////////////////
//
// extractActualCommand Tests
//
/////////////////////////////////////////////////
void RunParamListenerTestCases::testExtractActualComand()
{
    Assert("def" == RunParamListener::getInstance().extractActualComand("abc", "abcdef"));
    Assert("def" == RunParamListener::getInstance().extractActualComand("abc", "abc   def"));
    Assert("" == RunParamListener::getInstance().extractActualComand("a.b.c", "a.b.c"));
    Assert("" == RunParamListener::getInstance().extractActualComand("a.b.c", "a.b.c "));
    Assert("This is a test" == RunParamListener::getInstance().extractActualComand("a.b.c", "a.b.c This is a test"));
    Assert("DBG.PA.Public.PAZONE" == RunParamListener::getInstance().extractActualComand("a.b.c", "a.b.c DBG.PA.Public.PAZONE"));
}


/////////////////////////////////////////////////
//
// testRegisterTag Tests
//
/////////////////////////////////////////////////
void RunParamListenerTestCases::testRegisterTag()
{
    const std::string tagName = "RandomTag";

    RunParamListener::getInstance().registerTag(tagName);

    RunParamListener::RunParamChangeStruct& cs = RunParamListener::getInstance().m_registeredTags.back();
    
    Assert(cs.m_paramName == tagName);
    
    // Now remove
    RunParamListener::getInstance().m_registeredTags.pop_back();       
}


/////////////////////////////////////////////////
//
// testFindChangeStruct Tests
//
/////////////////////////////////////////////////
void RunParamListenerTestCases::testFindChangeStruct()
{
    Assert(0 != RunParamListener::getInstance().findChangeStruct(TEST_PARAM_1));
    Assert(TEST_PARAM_1 == RunParamListener::getInstance().findChangeStruct(TEST_PARAM_1)->m_paramName);

    Assert(0 != RunParamListener::getInstance().findChangeStruct(TEST_PARAM_2));
    Assert(TEST_PARAM_2 == RunParamListener::getInstance().findChangeStruct(TEST_PARAM_2)->m_paramName);

    Assert(0 != RunParamListener::getInstance().findChangeStruct(TEST_PARAM_3));
    Assert(TEST_PARAM_3 == RunParamListener::getInstance().findChangeStruct(TEST_PARAM_3)->m_paramName);

    Assert(0 != RunParamListener::getInstance().findChangeStruct(TEST_PARAM_4));
    Assert(TEST_PARAM_4 == RunParamListener::getInstance().findChangeStruct(TEST_PARAM_4)->m_paramName);

    Assert(RunParamListener::getInstance().m_registeredTags.end()
             == RunParamListener::getInstance().findChangeStruct(BAD_TEST_PARAM));
}


/////////////////////////////////////////////////
//
// testDispatchChangeToListener Tests
//
/////////////////////////////////////////////////
void RunParamListenerTestCases::testDispatchChangeToListener()
{
    DummyListener listener;

    Assert(RunParamListener::getInstance().dispatchChangeToListener(listener, TEST_PARAM_1, TEST_VALUE_1));

    Assert(1 == listener.received.size() );
    Assert(listener.received[0].first == TEST_PARAM_1);
    Assert(listener.received[0].second == TEST_VALUE_1);

    const std::string newIdentity("OCC,1280,");
    const std::string mismatch1("OCC,640,");
    const std::string mismatch2("DBG,1280,");
    RunParamListener::getInstance().setNewIdentityFromIdentifier(newIdentity);    
    listener.received.clear();
    
    // First three shouldn't be received, invalid identity
    Assert(!RunParamListener::getInstance().dispatchChangeToListener(listener, TEST_PARAM_2, TEST_VALUE_2));
    Assert(0 == listener.received.size());
    Assert(!RunParamListener::getInstance().dispatchChangeToListener(listener, TEST_PARAM_2, mismatch1 + TEST_VALUE_2));
    Assert(0 == listener.received.size());
    Assert(!RunParamListener::getInstance().dispatchChangeToListener(listener, TEST_PARAM_2, mismatch2 + TEST_VALUE_2));
    Assert(0 == listener.received.size());
    
    // Third one should strip out 'newIdentity' and only pass through TEST_VALUE_2
    Assert(RunParamListener::getInstance().dispatchChangeToListener(listener, TEST_PARAM_2, newIdentity + TEST_VALUE_2));
    Assert(1 == listener.received.size());
    Assert(listener.received[0].first == TEST_PARAM_2 );
    Assert(listener.received[0].second == TEST_VALUE_2 );

    RunParamListener::getInstance().setNewIdentityFromIdentifier("");
}


/////////////////////////////////////////////////
//
// testFindEndOfIdentityString Tests
//
/////////////////////////////////////////////////
void RunParamListenerTestCases::testFindEndOfIdentityString()
{
    Assert(2 == RunParamListener::getInstance().findEndOfIdentityString(",,"));
    Assert(3 == RunParamListener::getInstance().findEndOfIdentityString(", ,"));
    Assert(4 == RunParamListener::getInstance().findEndOfIdentityString("A,B,"));
    Assert(4 == RunParamListener::getInstance().findEndOfIdentityString("A,B,,,,,"));
    Assert(4 == RunParamListener::getInstance().findEndOfIdentityString("A,B,asf,sfd,sgd"));
    Assert(std::string::npos == RunParamListener::getInstance().findEndOfIdentityString("A,B"));
}


/////////////////////////////////////////////////
//
// testExtractStringIdentifierString Tests
//
/////////////////////////////////////////////////
void RunParamListenerTestCases::testExtractStringIdentifierString()
{
    Assert(",," == RunParamListener::getInstance().extractStringIdentifierString(",,"));
    Assert("" == RunParamListener::getInstance().extractStringIdentifierString(","));
    Assert("" == RunParamListener::getInstance().extractStringIdentifierString("A,B"));
    Assert("A,B," == RunParamListener::getInstance().extractStringIdentifierString("A,B,"));
    Assert("A,B," == RunParamListener::getInstance().extractStringIdentifierString("A,B,C"));
    Assert("A,B," == RunParamListener::getInstance().extractStringIdentifierString("A,B,C,Dfgajgkj,asdf,f"));
}


TestSuite * RunParamListenerTestCases::suite()
{
    TestSuite * testSuite = new TestSuite( "RunParamListenerTestCases" );  

#define ADD_UNIT_TEST(function) \
    testSuite->addTest( new TestCaller< RunParamListenerTestCases >(\
                        #function,\
                        &RunParamListenerTestCases::##function,this ) );

    //
    // processInternalRunParamChange tests
    //
    ADD_UNIT_TEST(testProcessInternalRunParamChange);

    //
    // registerListener tests
    //
    ADD_UNIT_TEST(testRegisterListener);

    //
    // deRegisterListener tests
    //
    ADD_UNIT_TEST(testDeRegisterListener);

    //
    // retrieveInitialRunParamIdentity tests
    //
    ADD_UNIT_TEST(testRetrieveInitialRunParamIdentity);

    //
    // findVirtualLocation tests
    //
    ADD_UNIT_TEST(testFindVirtualLocation);

    //
    // extractActualCommand tests
    //
    ADD_UNIT_TEST(testExtractActualComand);

    //
    // registerTag tests
    //
    ADD_UNIT_TEST(testRegisterTag);

    //
    // findChangeStruct tests
    //
    ADD_UNIT_TEST(testFindChangeStruct);

    //
    // dispatchChangeToListener tests
    //
    ADD_UNIT_TEST(testDispatchChangeToListener);

    //
    // findEndOfIdentityString tests
    //
    ADD_UNIT_TEST(testFindEndOfIdentityString);

    //
    // extractStringIdentifierString tests
    //
    ADD_UNIT_TEST(testExtractStringIdentifierString);

#undef ADD_UNIT_TEST


	return testSuite;
}