/**
 * The source code in this file is the property of
 * Ripple Systems and is not for redistribution
 * in any form.
 *
 * Source: $File: //depot/C830AR_TIP/C830AR/transactive/app/pa/PAManager/src/TrainStatusHelper.cpp $
 * @author Jade Lee
 * @version $Revision: #2 $
 * Last modification: $DateTime: 2024/12/17 16:14:02 $
 * Last modified by: $Author: limin.zhu $
 *
 * TrainStatusHelper contains helper functions for all functionality
 * common between train status dialogs (because the hierarchy isn't
 * split up to allow sharing of train specific functionality)
 *
 */

#include "app/pa/PAManager/src/StdAfx.h"
#include "app/pa/PAManager/src/TrainStatusHelper.h"
#include "app/pa/PAManager/src/CachedConfig.h"
#include "bus/pa/cachedmaps/src/CachedMaps.h"
#include "app/pa/PAManager/src/PaLayoutHelper.h"
#include "app/pa/PAManager/src/PaRightsManager.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// TD17655 no need any more
//static const int TRAIN_COLUMN_WIDTH_PIXELS = 65;

// TD16865
// static const CString TRAIN_ID_HEADER    = "Train";
static const CString TRAIN_ID_HEADER	= "PV/Service";
// TD16865
static const CString STATUS_HEADER      = "Status";

void TrainStatusHelper::retryTrainBroadcast(
    PAAgentNamedObject& agent,
    const std::string& broadcastId,
    const LockedPacketProgressReport& lockedProgressReport,
    const GenericStatusModel::ERetryMode& mode,
    bool hasOverrideOption)
{
    LOG_SCOPE("TrainStatusHelper::retryTrainBroadcast");

    if (!PaRightsManager::getInstance().hasRightToRetryBroadcast(broadcastId, agent))
    {
        PaErrorHandler::displayError(PaErrorHandler::INSUFFICIENT_RIGHTS);
        return;
    }

    CWaitCursor hourGlassCursor;

    bool collectFailedOnly = (GenericStatusModel::FAILED == mode);

    // Collect set of trains we want to retry
    TA_Base_Bus::IPAAgentCorbaDef::TrainCoverage trains =  collectTrains(lockedProgressReport,
                                                                         collectFailedOnly);

    if (0 == trains.length())
    {
        // Silently ignore - this means someone at another
        // terminal has retried the request (and there are no items
        // with a failed state anymore - since they're retrying)
        // *Effectively someone has already retried for us*
        return;
    }

    // Allow all exceptions to propogate
    CORBA_CALL(agent, retryTrainBroadcast, (broadcastId.c_str(),
                                            trains,
                                            hasOverrideOption,
                                            CachedConfig::getInstance()->getSessionId().c_str()));
}

TA_Base_Bus::IPAAgentCorbaDef::TrainCoverage
TrainStatusHelper::collectTrains(const LockedPacketProgressReport& lockedProgressReport,
                                 bool includeFailedOnly)
{
    // Go through all zone objects, and return those zones that
    // have a status not matching any of the states listed in excludedStates parameter

    // Obtain direct reference to data
    const TA_Base_Bus::IPAAgentCorbaDef::BroadcastProgressUpdateData& lockedData = lockedProgressReport.getLockedData();

    // Obtain reference to specialized data (for trains)
    const TA_Base_Bus::IPAAgentCorbaDef::LocalProgressSeq lockedTrainData =
        lockedData.localProgresses;

    unsigned int index = 0;
    std::vector<unsigned long> trainVector;
    unsigned int i;

    for (i = 0; i < lockedTrainData.length(); i++)
    {
        const TA_Base_Bus::IPAAgentCorbaDef::LocalProgress& tp = lockedTrainData[i];

        if (includeFailedOnly)
        {
            if (!tp.errorOccurred)
            {
                // No failures for this train, don't include for result
                continue;
            }
        }

        trainVector.push_back(tp.zoneKeyOrTrainId);
    }

    TA_Base_Bus::IPAAgentCorbaDef::TrainCoverage trains;
    trains.length(trainVector.size());

    for (i = 0; i < trainVector.size(); i++)
    {
        trains[i] = trainVector[i];
    }

    return trains;
}

bool TrainStatusHelper::addToTrainReportLists(CListCtrl& failuresList,
                                              CListCtrl& progressList,
                                              const TA_Base_Bus::IPAAgentCorbaDef::LocalProgressSeq& trainProgresses,
                                              // TD16865
                                              std::map<unsigned int, std::string>& trainIdAndServiceNum)
    // TD16865
{
    bool result = true;

    // Go through each zone and update the list
    for (unsigned int nTrain = 0; nTrain < trainProgresses.length(); ++nTrain)
    {
        // TD #10316
        // Get the zones starting from the end of the list.
        // Since each one is inserted at index 0, they will display in the lists
        // in reverse order unless we do this.
        const TA_Base_Bus::IPAAgentCorbaDef::LocalProgress& trainProgress = trainProgresses[trainProgresses.length() - nTrain - 1];

        result &= addTrainReportEntry(trainProgress,
                                      failuresList,
                                      progressList,
                                      // TD16865
                                      trainIdAndServiceNum);
        // TD16865
    }

    return result;
}

/*
Train DVA
[trainID] [Status]
One list contains train ID and shows whether broadcast is in preparation, or successful
    - Connecting
    - In Progress
    - Disconnecting
    - Complete

[TrainId] [Status]
Second list contains train ID and shows trains that were unsuccessful
    - Unsuccessful
    - Override option (if unsuccessful and may be overridden)
*/
bool TrainStatusHelper::addTrainReportEntry(const TA_Base_Bus::IPAAgentCorbaDef::LocalProgress& trainProgress,
                                            CListCtrl& failuresList,
                                            CListCtrl& progressList,
                                            // TD16865
                                            std::map<unsigned int, std::string>& trainIdAndServiceNum)
    // TD16865
{
    unsigned long zoneKeyOrTrainId = trainProgress.zoneKeyOrTrainId;

    bool result = true;

    try
    {
        CListCtrl& targetList =
            trainProgress.errorOccurred ? failuresList : progressList;

        // TD16865
        std::string serviceNum;
        //		CString trainIdStr;

        std::map<unsigned int, std::string>::iterator itr = trainIdAndServiceNum.find(zoneKeyOrTrainId);

        if (itr == trainIdAndServiceNum.end())
        {
            serviceNum = "";
        }
        else
        {
            serviceNum = itr->second;
        }

        //        trainIdStr.Format("%d", zoneKeyOrTrainId);

        int nItem = targetList.InsertItem(0, serviceNum.c_str());
        // TD16865

        std::string trainStatusStr;

        try
        {
            trainStatusStr = getTrainStatusString(trainProgress.localState);
        }
        catch (const TA_Base_Core::TransactiveException&)
        {
            result = false;
            trainStatusStr = "UNKNOWN";
        }

        targetList.SetItemText(nItem, 1, trainStatusStr.c_str());
    }
    catch (const TA_Base_Core::TransactiveException&)
    {
        // Couldn't classify the item, nothing added to list
        result = false;
    }

    return result;
}

const std::string TrainStatusHelper::getTrainStatusString(const TA_Base_Bus::IPAAgentCorbaDef::ELocalState& status)
{
    return GenericStatusModel::getStatusString(status);
}

void TrainStatusHelper::setupReportWindowList(CListCtrl& targetList)
{
    // Determine total width, accounting for scroll bar
    // jeffrey++ TD8850
    //int headerTotalWidth = PaLayoutHelper::getListHeaderWidth(targetList, true);
    //int headerTotalWidth = PaLayoutHelper::getListHeaderWidth(targetList, false); // TD17655 no need this variable any more
    // ++jeffrey TD8850

    targetList.InsertColumn(0,
                            TRAIN_ID_HEADER,
                            LVCFMT_LEFT);
    //TRAIN_COLUMN_WIDTH_PIXELS);

    targetList.InsertColumn(1,
                            STATUS_HEADER,
                            LVCFMT_LEFT);
    //headerTotalWidth - TRAIN_COLUMN_WIDTH_PIXELS);

    targetList.SetColumnWidth(0, LVSCW_AUTOSIZE_USEHEADER);	// TD17655
    targetList.SetColumnWidth(1, LVSCW_AUTOSIZE_USEHEADER);	// TD17655

    // ++AZ TES 9613, tooltips set in ListCtrlSelNoFocus
    //targetList.SetExtendedStyle(targetList.GetExtendedStyle() | LVS_EX_INFOTIP);
    // AZ++
}
