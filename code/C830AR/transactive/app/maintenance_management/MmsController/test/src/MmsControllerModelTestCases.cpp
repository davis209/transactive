/**
  * The source code in this file is the property of
  * Ripple Systems and is not for redistribution
  * in any form.
  *
  * Source:   $File: //depot/4669_T01271350/4669/transactive/app/maintenance_management/MmsController/test/src/MmsControllerModelTestCases.cpp $
  * @author:  Ripple
  * @version: $Revision: #1 $
  *
  * Last modification: $DateTime: 2008/11/28 16:26:01 $
  * Last modified by:  $Author: builder $
  *
  */
#pragma warning (disable : 4786 4290)

#include "StdAfx.h"

#include "AuditMessageSenderStub.h"
#include "MmsControllerModelTestCases.h"
#include "Common.h"

#include "app\maintenance_management\MmsController\src\MmsControllerModel.h"
#include "app\maintenance_management\MmsController\src\CachedConfig.h"

#include "core/utilities/src/DebugUtil.h"
#include "core/utilities/src/TaAssert.h"

using namespace TA_Base_Core;
using namespace TA_Base_Bus;
using namespace TA_IRS_App;

static const std::string EQUIPMENT_NAME_0("DBG.PA.PUBLIC");
static const unsigned long EQUIPMENT_KEY_0(1);

static const std::string EQUIPMENT_NAME_1("DBG.PA.NONPUBLIC");
static const unsigned long EQUIPMENT_KEY_1(2);

/////////////////////////////////////////////////
//
// getGlobalMmsInhibit Tests
//
/////////////////////////////////////////////////
void MmsControllerModelTestCases::testgetGlobalMmsInhibit()
{
    MmsControllerModel model;

    CachedConfig::getInstance().getMmsInhibitAgent()->setGlobalMmsInhibit(true, "");
    Assert(true == model.getGlobalMmsInhibit());

    CachedConfig::getInstance().getMmsInhibitAgent()->setGlobalMmsInhibit(false, "");
    Assert(false == model.getGlobalMmsInhibit());

    // Confirm that exceptions are propogated on error
    CachedConfig::getInstance().getMmsInhibitAgent()->setThrowExceptionOnNextGlobalGet();   

    try
    {
        model.getGlobalMmsInhibit();

        // MmsControllerModel shouldn't have caught exception
        Assert(false);
    }
    catch (...)
    {
        // Exception was propogated properly
        Assert(true);
    }
}


/////////////////////////////////////////////////
//
// setGlobalMmsInhibit Tests
//
/////////////////////////////////////////////////
void MmsControllerModelTestCases::testsetGlobalMmsInhibit_success()
{
    MmsControllerModel model;

    model.setGlobalMmsInhibit(true);

    // Verify correct audit message went through, and state updated
    Assert(AuditMessageSender::verifyLastAuditMessage(
                TA_Base_Core::MmsAudit::MMSGlobalInhibitApplied) );
    Assert(true == CachedConfig::getInstance().getMmsInhibitAgent()->getGlobalMmsInhibit());

    model.setGlobalMmsInhibit(false);

    // Verify correct audit message went through, and state updated
    Assert(AuditMessageSender::verifyLastAuditMessage(
                TA_Base_Core::MmsAudit::MMSGlobalInhibitRemoved) );
    Assert(false == CachedConfig::getInstance().getMmsInhibitAgent()->getGlobalMmsInhibit());
}


void MmsControllerModelTestCases::testsetGlobalMmsInhibit_failure()
{
    MmsControllerModel model;

    // Inhibit set to false as baseline
    bool baseLineInhibit = false;
    model.setGlobalMmsInhibit(baseLineInhibit);

    // Upon a failure should log appropriate failed audit message, but not propogate exceptions or
    // display message boxes
    CachedConfig::getInstance().getMmsInhibitAgent()->setThrowExceptionOnNextGlobalSet();   

    try
    {
        model.setGlobalMmsInhibit(!baseLineInhibit);
        
        Assert(AuditMessageSender::verifyLastAuditMessage(
                TA_Base_Core::MmsAudit::MMSGlobalInhibitFailed ) );
    }
    catch (...)
    {
        // Exceptions shouldn't be propogated, but logged via audit messages
        Assert(false);
    }
    
    ///////////////////////////////////////////////////////////////
    // Test for rights failure (which should display a message box)
    TA_IRS_App::MmsTest::g_theTestApp.m_messageBoxDisplayed = FALSE;
    
    RightsLibrary::disableRight(TA_Base_Bus::aca_MMS_GLOBAL_INHIBIT);

    model.setGlobalMmsInhibit(!baseLineInhibit);

    // Confirm there was an attempt to display message box, and that global MMS didn't change
    Assert(baseLineInhibit == CachedConfig::getInstance().getMmsInhibitAgent()->getGlobalMmsInhibit());
    Assert(TRUE == TA_IRS_App::MmsTest::g_theTestApp.m_messageBoxDisplayed);

    RightsLibrary::restoreRights();
}
   
/////////////////////////////////////////////////
//
// getSubsystemMmsInhibit Tests
//
/////////////////////////////////////////////////
void MmsControllerModelTestCases::testgetSubsystemMmsInhibit()
{
    MmsControllerModel model;

    IDataNodeProxy::setInhibitState(EQUIPMENT_NAME_0, true);
    IDataNodeProxy::setInhibitState(EQUIPMENT_NAME_1, true);
    Assert(true == model.getSubsystemMmsInhibit(EQUIPMENT_NAME_0));
    Assert(true == model.getSubsystemMmsInhibit(EQUIPMENT_NAME_1));

    IDataNodeProxy::setInhibitState(EQUIPMENT_NAME_0, false);
    Assert(false == model.getSubsystemMmsInhibit(EQUIPMENT_NAME_0));
    Assert(true == model.getSubsystemMmsInhibit(EQUIPMENT_NAME_1));

    // Confirm that exceptions are propogated on error
    IDataNodeProxy::setThrowExceptionOnNextGet();   

    try
    {
        model.getSubsystemMmsInhibit(EQUIPMENT_NAME_0);

        // MmsControllerModel shouldn't have caught exception
        Assert(false);
    }
    catch (...)
    {
        // Exception was propogated properly
        Assert(true);
    }
}
    
/////////////////////////////////////////////////
//
// setSubsystemMmsInhibit Tests
//
/////////////////////////////////////////////////  
void MmsControllerModelTestCases::testsetSubsystemMmsInhibit_success()
{
    MmsControllerModel model;

    model.setSubsystemMmsInhibit(EQUIPMENT_NAME_0, EQUIPMENT_KEY_0, true);

    // Verify correct audit message went through, and state updated
    Assert(AuditMessageSender::verifyLastAuditMessage(
                TA_Base_Core::MmsAudit::MMSSubsystemInhibitApplied) );
    Assert(true == IDataNodeProxy::getInhibitState(EQUIPMENT_NAME_0));

    model.setSubsystemMmsInhibit(EQUIPMENT_NAME_0, EQUIPMENT_KEY_0, false);

    // Verify correct audit message went through, and state updated
    Assert(AuditMessageSender::verifyLastAuditMessage(
                TA_Base_Core::MmsAudit::MMSSubsystemInhibitRemoved) );
    Assert(false == IDataNodeProxy::getInhibitState(EQUIPMENT_NAME_0));

}

     
void MmsControllerModelTestCases::testsetSubsystemMmsInhibit_failure()
{
    MmsControllerModel model;

    // Inhibit set to false as baseline
    bool baseLineInhibit = false;
    model.setSubsystemMmsInhibit(EQUIPMENT_NAME_0, EQUIPMENT_KEY_0, baseLineInhibit);

    // Upon a failure should log appropriate failed audit message, but not propogate exceptions or
    // display message boxes
    IDataNodeProxy::setThrowExceptionOnNextSet();   

    try
    {
        model.setSubsystemMmsInhibit(EQUIPMENT_NAME_0, EQUIPMENT_KEY_0, !baseLineInhibit);
        
        Assert(AuditMessageSender::verifyLastAuditMessage(
                TA_Base_Core::MmsAudit::MMSSubsystemInhibitFailed ) );
    }
    catch (...)
    {
        // Exceptions shouldn't be propogated, but logged via audit messages
        Assert(false);
    }
    
    ///////////////////////////////////////////////////////////////
    // Test for rights failure (which should display a message box)
    TA_IRS_App::MmsTest::g_theTestApp.m_messageBoxDisplayed = FALSE;
    
    RightsLibrary::disableRight(TA_Base_Bus::aca_MMS_SUBSYSTEM_INHIBIT);

    bool returnVal = model.setSubsystemMmsInhibit(EQUIPMENT_NAME_0, EQUIPMENT_KEY_0, !baseLineInhibit);

    // Confirm function returns false, indicating insufficient privilege to modify this subsystem
    Assert(false == returnVal);
    // Inhibit state shouldn't have changed (because of insufficient rights)
    Assert(baseLineInhibit == IDataNodeProxy::getInhibitState(EQUIPMENT_NAME_0));
    
    RightsLibrary::restoreRights();
}
      
/////////////////////////////////////////////////
//
// getDatanodeProxy Tests
//
/////////////////////////////////////////////////  
void MmsControllerModelTestCases::testgetDatanodeProxy()
{
    MmsControllerModel model;

    // Unable to test much useful here..
    TA_Base_Bus::DataNodeProxySmartPtr proxy = model.getDatanodeProxy(EQUIPMENT_NAME_0);

    Assert(proxy->m_corbaObjectName == EQUIPMENT_NAME_0);
}
   
/////////////////////////////////////////////////
//
// sendGlobalInhibtAuditMessage Tests
//
/////////////////////////////////////////////////  
void MmsControllerModelTestCases::testsendGlobalInhibitAuditMessage()
{
    MmsControllerModel model;

    model.sendGlobalInhibitAuditMessage(MmsControllerModel::INHIBIT_APPLIED);
    Assert(AuditMessageSender::verifyLastAuditMessage(
                TA_Base_Core::MmsAudit::MMSGlobalInhibitApplied) );

    model.sendGlobalInhibitAuditMessage(MmsControllerModel::INHIBIT_REMOVED);
    Assert(AuditMessageSender::verifyLastAuditMessage(
                TA_Base_Core::MmsAudit::MMSGlobalInhibitApplied) );

    model.sendGlobalInhibitAuditMessage(MmsControllerModel::INHIBIT_FAILED);
    Assert(AuditMessageSender::verifyLastAuditMessage(
                TA_Base_Core::MmsAudit::MMSGlobalInhibitApplied) );

}
   

/////////////////////////////////////////////////
//
// sendSubsystemInhibitAuditMessage Tests
//
/////////////////////////////////////////////////            
void MmsControllerModelTestCases::testsendSubsystemInhibitAuditMessage()
{
    MmsControllerModel model;

    model.sendSubsystemInhibitAuditMessage(EQUIPMENT_NAME_0, MmsControllerModel::INHIBIT_APPLIED);
    Assert(AuditMessageSender::verifyLastAuditMessage(
                TA_Base_Core::MmsAudit::MMSSubsystemInhibitApplied) );

    model.sendSubsystemInhibitAuditMessage(EQUIPMENT_NAME_0, MmsControllerModel::INHIBIT_REMOVED);
    Assert(AuditMessageSender::verifyLastAuditMessage(
                TA_Base_Core::MmsAudit::MMSSubsystemInhibitApplied) );

    model.sendSubsystemInhibitAuditMessage(EQUIPMENT_NAME_0, MmsControllerModel::INHIBIT_FAILED);
    Assert(AuditMessageSender::verifyLastAuditMessage(
                TA_Base_Core::MmsAudit::MMSSubsystemInhibitApplied) );

}
      
/////////////////////////////////////////////////
//
// submitAuditMessage Tests
//
/////////////////////////////////////////////////  
void MmsControllerModelTestCases::testsubmitAuditMessage()
{
    MmsControllerModel model;

    TA_Base_Core::DescriptionParameters dummyParams;
    
    // Subsystem audit messages
    model.submitAuditMessage(TA_Base_Core::MmsAudit::MMSSubsystemInhibitApplied, dummyParams);
    Assert(AuditMessageSender::verifyLastAuditMessage(
                TA_Base_Core::MmsAudit::MMSSubsystemInhibitApplied) );
    model.submitAuditMessage(TA_Base_Core::MmsAudit::MMSSubsystemInhibitRemoved, dummyParams);
    Assert(AuditMessageSender::verifyLastAuditMessage(
                TA_Base_Core::MmsAudit::MMSSubsystemInhibitRemoved) );
    model.submitAuditMessage(TA_Base_Core::MmsAudit::MMSSubsystemInhibitFailed, dummyParams);
    Assert(AuditMessageSender::verifyLastAuditMessage(
                TA_Base_Core::MmsAudit::MMSSubsystemInhibitFailed) );
    
    // Global audit messages
    model.submitAuditMessage(TA_Base_Core::MmsAudit::MMSGlobalInhibitApplied, dummyParams);
    Assert(AuditMessageSender::verifyLastAuditMessage(
                TA_Base_Core::MmsAudit::MMSGlobalInhibitApplied) );
    model.submitAuditMessage(TA_Base_Core::MmsAudit::MMSGlobalInhibitRemoved, dummyParams);
    Assert(AuditMessageSender::verifyLastAuditMessage(
                TA_Base_Core::MmsAudit::MMSGlobalInhibitRemoved) );
    model.submitAuditMessage(TA_Base_Core::MmsAudit::MMSGlobalInhibitFailed, dummyParams);
    Assert(AuditMessageSender::verifyLastAuditMessage(
                TA_Base_Core::MmsAudit::MMSGlobalInhibitFailed) );
}     


void MmsControllerModelTestCases::setUpSuite( const std::string & inFile ) 
{
}


void MmsControllerModelTestCases::tearDownSuite() 
{
}


void MmsControllerModelTestCases::setUp( const std::string & INFile , const std::string & OUTFile )
{
    AuditMessageSender::clearAuditMessages();
}


TestSuite * MmsControllerModelTestCases::suite()
{
    TestSuite * testSuite = new TestSuite( "MmsControllerModelTestCases" );  

// Use temporary hashdef to add tests
#define ADD_UNIT_TEST(function) \
    testSuite->addTest( new TestCaller< MmsControllerModelTestCases >(\
                        #function,\
                        &MmsControllerModelTestCases::##function,this ) );

    //
    // getGlobalMmsInhibit tests
    //
    ADD_UNIT_TEST(testgetGlobalMmsInhibit);

    //
    // setGlobalMmsInhibit tests
    //
    ADD_UNIT_TEST(testsetGlobalMmsInhibit_success);
    ADD_UNIT_TEST(testsetGlobalMmsInhibit_failure);

    //
    // getSubsystemMmsInhibit tests
    //  
    ADD_UNIT_TEST(testgetSubsystemMmsInhibit);    

    //
    // setSubsystemMmsInhibit tests
    //  
    ADD_UNIT_TEST(testsetSubsystemMmsInhibit_success);    
    ADD_UNIT_TEST(testsetSubsystemMmsInhibit_failure);    

    //
    // getDatanodeProxy tests
    //  
    ADD_UNIT_TEST(testgetDatanodeProxy);    

    //
    // sendGlobalInhibitAuditMessage tests
    //  
    ADD_UNIT_TEST(testsendGlobalInhibitAuditMessage);       

    //
    // sendSubsystemInhibit tests
    //  
    ADD_UNIT_TEST(testsendSubsystemInhibitAuditMessage);    

    //
    // submitAuditMessage tests
    //  
    ADD_UNIT_TEST(testsubmitAuditMessage);    

#undef ADD_UNIT_TEST

	return testSuite;
}