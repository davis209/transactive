/**
  * The source code in this file is the property of
  * Ripple Systems and is not for redistribution
  * in any form.
  *
  * Source:   $File: //depot/4669_T01271350/4669/transactive/app/pa/PAManager/test/src/UnitTestAccessBridge.h $
  * @author:  Ripple
  * @version: $Revision: #1 $
  *
  * Last modification: $DateTime: 2008/11/28 16:26:01 $
  * Last modified by:  $Author: builder $
  *
  */
#pragma once

class GenericStatusModel;
class PaProgressReportSubscriber;

/// Provides a bridge between unit tests and access to private / protected members and
/// methods of classes (so no need for multiple declarations of friend classes in source)
class UnitTestAccessBridge
{
public:

    /**
     * simple accessor 
     */
    static PaProgressReportSubscriber& getProgressReportSubscriber(GenericStatusModel& model);

    /**
     * sents a progress report to the specified subscriber
     */
    static void pushProgressReport(PaProgressReportSubscriber& subscriber,
            const TA_IRS_Bus::IPAAgentCorbaDef::BroadcastProgressUpdateData& progressReport);

    /**
     * Sets up some locations in the CachedMaps class 
     *  so that CachedMaps::getLocationNameFromKey works
     *
     * @param count the number of locations to set up
     */
    static void setupSomeLocations(UINT count);
};