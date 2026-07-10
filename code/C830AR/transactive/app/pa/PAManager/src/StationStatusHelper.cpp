/**
 * The source code in this file is the property of
 * Ripple Systems and is not for redistribution
 * in any form.
 *
 * Source: $File: //depot/C830AR_TIP/C830AR/transactive/app/pa/PAManager/src/StationStatusHelper.cpp $
 * @author Jade Lee
 * @version $Revision: #3 $
 * Last modification: $DateTime: 2025/01/20 18:40:08 $
 * Last modified by: $Author: limin.zhu $
 *
 * StationStatusHelper contains helper functions for all functionality
 * common between station status dialogs (because the hierarchy isn't
 * split up to allow sharing of station specific functionality)
 *
 */

#include "app/pa/PAManager/src/StdAfx.h"
#include "app/pa/PAManager/src/StationStatusHelper.h"
#include "app/pa/PAManager/src/CachedConfig.h"
#include "bus/pa/cachedmaps/src/CachedMaps.h"
#include "app/pa/PAManager/src/PaLayoutHelper.h"
#include "app/pa/PAManager/src/PaRightsManager.h"
#include "bus/pa_4669/pa_agent/IDL/src/IPAAgentCorbaDefDescribe.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

using namespace TA_IRS_Bus;

static const int STATUS_WIDTH_PIXELS = 110;
// jeffrey++ TD8850
//static const PRIORITY_WIDTH_PIXELS = 65;
static const int PRIORITY_WIDTH_PIXELS = 75;
// ++jeffrey TD8850

// song toan++
// TD 10316
static const int STATION_LOC_WIDTH_PIXELS = 50;
static const CString STATION_LOC_HEADER    = "Loc";
// ++song toan
// TD 10316

static const CString STATION_ZONE_HEADER    = "Zone";
static const CString STATUS_HEADER          = "Status";
static const CString PRIORITY_HEADER        = "Priority";

void StationStatusHelper::retryStationBroadcast(PAAgentNamedObject& agent,
                                                const std::string& broadcastId,
                                                const LockedPacketProgressReport& lockedProgressReport,
                                                const GenericStatusModel::ERetryMode& mode)
{
    LOG_SCOPE("StationStatusHelper::retryStationBroadcast");

    if (!PaRightsManager::getInstance().hasRightToRetryBroadcast(broadcastId, agent))
    {
        PaErrorHandler::displayError(PaErrorHandler::INSUFFICIENT_RIGHTS);
        return;
    }

    CWaitCursor hourGlassCursor;

    bool collectFailedOnly = (GenericStatusModel::FAILED == mode);

    // Collect set of trains we want to retry
    TA_Base_Bus::IPAAgentCorbaDef::ZoneCoverage zones =  collectStationZones(
        lockedProgressReport,
        collectFailedOnly);

    if (0 == zones.length())
    {
        // Silently ignore - this means someone at another
        // terminal has retried the request (and there are no items
        // with a failed state anymore - since they're retrying)
        // *Effectively someone has already retried for us*
        return;
    }

    // Allow all exceptions to propogate
    CORBA_CALL(agent, retryStationBroadcast, (broadcastId.c_str(), zones, CachedConfig::getInstance()->getSessionId().c_str()));
}

TA_Base_Bus::IPAAgentCorbaDef::ZoneCoverage
StationStatusHelper::collectStationZones(const LockedPacketProgressReport& lockedProgressReport, bool includeFailedOnly)
{
    // Go through all zone objects, and return those zones that
    // have a status not matching any of the states listed in excludedStates parameter

    // Obtain direct reference to data
    const TA_Base_Bus::IPAAgentCorbaDef::BroadcastProgressUpdateData& lockedData = lockedProgressReport.getLockedData();

    // Obtain reference to specialized data (for stations)
    const TA_Base_Bus::IPAAgentCorbaDef::LocalProgressSeq lockedZoneData =
        lockedData.localProgresses;

    unsigned int index = 0;
    unsigned int i;
    std::vector<unsigned long> zoneVector;

    for (i = 0; i < lockedZoneData.length(); i++)
    {
        const TA_Base_Bus::IPAAgentCorbaDef::LocalProgress& zp = lockedZoneData[i];

        if (includeFailedOnly)
        {
            if (!zp.errorOccurred)
            {
                // No failures for this train, don't include for result
                continue;
            }
        }

        // This zone state didn't match any excluded state, can include in result
        zoneVector.push_back(zp.zoneKeyOrTrainId);
    }

    TA_Base_Bus::IPAAgentCorbaDef::ZoneCoverage zones;
    zones.length(zoneVector.size());

    for (i = 0; i < zoneVector.size(); i++)
    {
        zones[i] = zoneVector[i];
    }

    return zones;
}

bool StationStatusHelper::addToStationReportLists(// song toan++
                                                  // TD 10316
                                                  bool isOCC,
                                                  CListCtrlSort& failuresList,
                                                  CListCtrlSort& progressList,
                                                  // ++song toan
                                                  // TD 10316
                                                  const TA_Base_Bus::IPAAgentCorbaDef::LocalProgressSeq& zoneProgresses)
{
    LOG_SCOPE("StationStatusHelper::addToStationReportLists");

    bool result = true;

    // Go through each zone and update the list
    for (unsigned int nZone = 0; nZone < zoneProgresses.length(); ++nZone)
    {
        // TD #10316
        // Get the zones starting from the end of the list.
        // Since each one is inserted at index 0, they will display in the lists
        // in reverse order unless we do this.
        const TA_Base_Bus::IPAAgentCorbaDef::LocalProgress& zp = zoneProgresses[zoneProgresses.length() - nZone - 1];

        result &= addStationReportEntry(
            // song toan++
            // TD 10316
            isOCC,
            // ++song toan
            // TD 10316
            zp,
            failuresList,
            progressList);
    }

    return result;
}

/*
Station DVA and Station Live
[Station + Zone] [Status]
One list shows where broadcast in initialisation phase
    - Connecting
    - In Progress
    - Complete

[Station + Zone] [Status] [Priority]
The second list shows unsucessful ones
    - Iff an override, simply shows priority (priority value required)
    - Any other reason, it is left BLANK
*/
bool StationStatusHelper::addStationReportEntry(// song toan++
                                                // TD 10316
                                                bool isOCC,
                                                const TA_Base_Bus::IPAAgentCorbaDef::LocalProgress& zoneProgress,
                                                CListCtrlSort& failuresList,
                                                CListCtrlSort& progressList)
{
    LOG_SCOPE("StationStatusHelper::addStationReportEntry");

    // song toan++
    // TD 10316
    // determine column indexes
    int colLoc(0), colZone(0), colStatus(0), colPriority(0);
    getColumnIndexes(isOCC, colLoc, colZone, colStatus, colPriority);
    // ++song toan
    // TD 10316

    unsigned long zoneKeyOrTrainId = zoneProgress.zoneKeyOrTrainId;

    bool result = true;

    std::string stationZoneLabel;
    std::string stationLocLabel;
    int zoneId = 0;

    TA_IRS_Bus::PaZoneRecord* pZoneRecord = NULL;

    try
    {
        TA_IRS_Bus::PaZoneRecord record = TA_IRS_Bus::CachedMaps::getInstance()->getPaZoneRecordFromKey(zoneKeyOrTrainId);
        // song toan++
        // TD 10316
        stationZoneLabel = record.m_label;
        stationLocLabel = record.m_locationName;
        zoneId = record.m_id;

        pZoneRecord = new TA_IRS_Bus::PaZoneRecord(record);
        // +song toan
        // TD 10316
    }
    catch (const TA_Base_Core::CachedMappingNotFoundException&)
    {
        // Couldn't find this zone in the database
        result = false;
        // song toan++
        // TD 10316
        stationZoneLabel = "UNKNOWN ZONE";
        stationLocLabel = "???";

        pZoneRecord = new TA_IRS_Bus::PaZoneRecord();
        // ++song toan
        // TD 10316
    }

    std::string statusString;

    try
    {
        statusString = getStationStatusString(zoneProgress.localState);
    }
    catch (const TA_Base_Core::TransactiveException&)
    {
        statusString = "UNKNOWN";
        result = false;
    }

    LOG_SCOPE(boost::format("ZONE[%02d:%s]") % zoneId % stationZoneLabel);

    try
    {
        // song toan++
        // TD 10316
        // which list to fill (Progress or Failure)?
        CListCtrlSort& theList = zoneProgress.errorOccurred ? failuresList : progressList;
        int iItem = theList.InsertItem(0, "");
        theList.SetItemData(0, (DWORD)pZoneRecord);

        if (isOCC)
        {
            // Location column is at OCC only
            theList.SetItemText(iItem, colLoc, stationLocLabel.c_str());
        }

        //
        theList.SetItemText(iItem, colZone, stationZoneLabel.c_str());
        theList.SetItemText(iItem, colStatus, statusString.c_str());

        // Only set the text showing a priority value if failed overriden
        // (otherwise it's empty)
        if (zoneProgress.errorOccurred
            && (TA_Base_Bus::IPAAgentCorbaDef::LOCAL_STATE_FAILED_OVERRIDDEN ==
                zoneProgress.localState))
        {
            CString priorityStr;
            priorityStr.Format("%d", zoneProgress.overridingSourcePriorityForZonesOnly);
            failuresList.SetItemText(iItem, colPriority, priorityStr);
        }

        LOG_DEBUG("addStationReportEntry(): %s", nvps(statusString, stationLocLabel, stationZoneLabel, zoneProgress));

        // song toan++
        // TD 10316
    }
    catch (const TA_Base_Core::TransactiveException&)
    {
        // General failure (couldn't classify zone)
        // - nothing will have been added to the list
        result = false;
    }

    return result;
}

const std::string StationStatusHelper::getStationStatusString(const TA_Base_Bus::IPAAgentCorbaDef::ELocalState& status)
{
    return GenericStatusModel::getStatusString(status);
}

// song toan++
// TD 10316
void StationStatusHelper::getColumnIndexes(bool isOCC,
                                           int& colLoc,
                                           int& colZone,
                                           int& colStatus,
                                           int& colPriority)
{
    colLoc = 0;
    colZone = isOCC ? 1 : 0;
    colStatus = isOCC ? 2 : 1;
    colPriority = isOCC ? 3 : 2;
}

// ++song toan++
// TD 10316

void StationStatusHelper::setupReportWindowList(
    // song toan++
    // TD 10316
    bool isOCC,
    CListCtrlSort& targetList,
    // ++song toan
    // TD 10316
    bool isFailureList)
{
    // Determine total width, accounting for scroll bar
    // jeffrey++ TD8850
    //int headerWidth = PaLayoutHelper::getListHeaderWidth(targetList, true);
    int headerWidth = PaLayoutHelper::getListHeaderWidth(targetList, false);
    // ++jeffrey TD8850

    // song toan++
    // TD 10316
    //
    // First column does not display combined station / zone string anymore.
    // Below is a summary of possible columns of both Progress/Failure list controls
    // at different locations
    //
    //        \  List|                      |
    // Location\ con-|       Progress       |            Failure
    //          \trol|                      |
    // ----------\---+----------------------+-------------------------------
    // OCC           | Location,Zone,Status | Location,Zone,Status,Priority
    // --------------+----------------------+-------------------------------
    // Others        |          Zone,Status |          Zone,Status,Priority

    int remainWidth = headerWidth - STATUS_WIDTH_PIXELS;

    if (isFailureList)
    {
        // priority column is only at failures list
        remainWidth -= PRIORITY_WIDTH_PIXELS;
    }

    if (isOCC)
    {
        // Location column is available at OCC
        remainWidth -= STATION_LOC_WIDTH_PIXELS;
    }

    int stationZoneWidthPixels = remainWidth;

    // determine column indexes
    int colLoc(0), colZone(0), colStatus(0), colPriority(0);
    getColumnIndexes(isOCC, colLoc, colZone, colStatus, colPriority);

    if (isOCC)
    {
        // Location is shown at first column of both lists
        targetList.InsertColumn(colLoc,
                                STATION_LOC_HEADER,
                                LVCFMT_LEFT,
                                STATION_LOC_WIDTH_PIXELS);
    }

    targetList.InsertColumn(colZone,
                            STATION_ZONE_HEADER,
                            LVCFMT_LEFT,
                            stationZoneWidthPixels);

    // Status header
    targetList.InsertColumn(colStatus,
                            STATUS_HEADER,
                            LVCFMT_LEFT,
                            STATUS_WIDTH_PIXELS);

    if (isFailureList)
    {
        // Priority shown on end of failures list,
        // it has one additional column..
        targetList.InsertColumn(colPriority,
                                PRIORITY_HEADER,
                                LVCFMT_LEFT,
                                PRIORITY_WIDTH_PIXELS);
    }

    // ++song toan
    // TD 10316

    // ++AZ TES 9613, tooltips set in ListCtrlSelNoFocus
    //targetList.SetExtendedStyle(targetList.GetExtendedStyle() | LVS_EX_INFOTIP);
    // AZ++
}
