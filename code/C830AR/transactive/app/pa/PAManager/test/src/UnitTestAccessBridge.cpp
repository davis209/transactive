/**
  * The source code in this file is the property of
  * Ripple Systems and is not for redistribution
  * in any form.
  *
  * Source:   $File: //depot/4669_T01271350/4669/transactive/app/pa/PAManager/test/src/UnitTestAccessBridge.cpp $
  * @author:  Ripple
  * @version: $Revision: #1 $
  *
  * Last modification: $DateTime: 2008/11/28 16:26:01 $
  * Last modified by:  $Author: builder $
  *
  */
#include "StdAfx.h"
#include "UnitTestAccessBridge.h"

#include "app/pa/pamanager/src/GenericStatusModel.h"
#include "app/pa/pamanager/src/PaProgressReportSubscriber.h"

#include "bus/pa/cachedmaps/src/CachedMaps.h"

PaProgressReportSubscriber& UnitTestAccessBridge::getProgressReportSubscriber(GenericStatusModel& model)
{
    return model.m_progressReportSubscriber;
}


void UnitTestAccessBridge::pushProgressReport(
    PaProgressReportSubscriber& subscriber,
    const TA_IRS_Bus::IPAAgentCorbaDef::BroadcastProgressUpdateData& progressReport)
{
    subscriber.processProgressReport(progressReport);
}


void UnitTestAccessBridge::setupSomeLocations(UINT count)
{
    TA_IRS_Bus::CachedMaps& cm = *TA_IRS_Bus::CachedMaps::getInstance();

    TA_Base_Core::ThreadGuard guard(cm.m_lockForLocationKeyToNameMap );

    cm.m_locationKeyToNameMap.clear();

    for (UINT i = 0; i < count; i ++)
    {
        UINT lKey = i + 1;
        std::ostringstream ostr;
        ostr << "LOCN " << lKey;
        cm.m_locationKeyToNameMap[lKey] = ostr.str();
    }

    cm.m_isLocationKeyToNameMapInitialised = true;
}