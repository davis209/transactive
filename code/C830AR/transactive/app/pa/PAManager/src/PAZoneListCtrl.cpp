/**
  * The source code in this file is the property of
  * Ripple Systems and is not for redistribution
  * in any form.
  *
  * Source: $File: //depot/C830AR_TIP/C830AR/transactive/app/pa/PAManager/src/PAZoneListCtrl.cpp $
  * @author Sean Liew
  * @version $Revision: #5 $
  * Last modification: $DateTime: 2025/01/20 18:40:08 $
  * Last modified by: $Author: limin.zhu $
  *
  * Maintains a list control for PA Zones
  *  Because there is only one PA Zone list in the PA Manager application
  *  this control is mostly self contained (listens for update data updates
  *  independently, updating view as required)
  *
  */

#include "app/pa/PAManager/src/stdafx.h"
#include "bus/pa/cachedmaps/src/CachedMaps.h"
#include "app/pa/PAManager/src/pamanager.h"
#include "app/pa/PAManager/src/PaZoneListCtrl.h"
#include "app/pa/PAManager/src/CachedConfig.h"
#include "app/pa/PAManager/src/PaManagerModel.h"
#include "app/pa/PAManager/src/PaErrorHandler.h"
#include "app/pa/PAManager/src/PaLayoutHelper.h"
#include "app/pa/PAManager/src/LockControlRedraw.h"
#include "bus/pa/cachedmaps/src/CachedMaps.h"
#include "app/pa/PAManager/src/PrioritySchemeUpdateSubscriber.h"
#include "bus/mfc_extensions/src/selection_helpers/SelectionHelper.h"
#include "core/data_access_interface/src/DatabaseKey.h"
#include "core/data_access_interface/src/LocationAccessFactoryEx.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

using namespace TA_IRS_Bus;
// liuyu++ TD8849
using namespace TA_Base_Bus;
// ++liuyu TD8849
using namespace TA_Base_Core;

static const std::string ZONE_NO_LONGER_EXISTS = "*Zone no longer exists*";
static const std::string PRIORITY_UNAVAILABLE = "NA";

PaZoneListCtrl::PaZoneListCtrl()
    : m_currentMode(-1),
    m_prioritySchemeUpdateSubscriber(new PrioritySchemeUpdateSubscriber())
    // song toan++
    // Bug #276
    // Initialization of m_colSortingInfo was moved into the body
    // because PA_COL_MAX has not been determined yet
    // ,m_colSortingInfo(PA_COL_MAX)
    //
    // ++song toan
    // Bug #276
{
    // We want LVN_ITEMCHANGING notifications to be passed back to parent window
    // (and give it the opportunity to handle the notifications)
    setItemChangeRequestForwarding(TRUE);

    // song toan++
    // Bug #276

    // Determine index of the columns according to station type
    m_bIsAtOCC = CachedConfig::getInstance()->isAtOcc();

    if (m_bIsAtOCC)
    {
        // at OCC there are 4 columns
        PA_COL_LABEL = 1;
        PA_COL_AREA = 2;
        PA_COL_PRIORITY = 3;
        PA_COL_MAX = 4;
    }
    else
    {
        // At other stations there are 3 columns
        PA_COL_LABEL = 4;   // point to non-exist column just for safety.
        // it will not be used anyway.
        PA_COL_AREA = 1;
        PA_COL_PRIORITY = 2;
        PA_COL_MAX = 3;
    }

    m_colSortingInfo = new TA_Base_Bus::AutoSortListCtrl::SortInfo(PA_COL_MAX);
    // song toan++
    // Bug #276

    // liuyu++ TD8849
    // sort callback for item data
    m_colSortingInfo->setCallback(this);
    // ++liuyu TD8849

    // liuyu++ TD8849
    // TD #3869
    // Sort columns based on label
    // m_colSortingInfo.setCurrentSort(PA_COL_LABEL, TA_Base_Bus::AutoSortListCtrl::ASCENDING);
    m_colSortingInfo->setCurrentSort(PA_COL_LOCATION_NAME, TA_Base_Bus::AutoSortListCtrl::ASCENDING);
    // Make label column the active sorting column
    // m_colSortingInfo.activateSort(PA_COL_LABEL);
    m_colSortingInfo->activateSort(PA_COL_LOCATION_NAME);
    // ++liuyu TD8849

    // Priority column should have integer based sorting
    m_colSortingInfo->defineSortingSemantic(PA_COL_PRIORITY, TA_Base_Bus::AutoSortListCtrl::BY_INTEGER);
}

PaZoneListCtrl::~PaZoneListCtrl()
{
    try
    {
        delete m_prioritySchemeUpdateSubscriber;
        m_prioritySchemeUpdateSubscriber = 0;
        delete m_colSortingInfo;
        m_colSortingInfo = NULL;
    }
    catch (...)
    {
        LOG(SourceInfo, TA_Base_Core::DebugUtil::ExceptionCatch, "Unknown", "Caught in destructor");
    }
}

BEGIN_MESSAGE_MAP(PaZoneListCtrl, CListCtrl)
    //{{AFX_MSG_MAP(PaZoneListCtrl)
    ON_MESSAGE(WM_PA_PRIORITY_SCHEME_UPDATE, OnPrioritySchemeUpdate)
    ON_WM_DESTROY()
    //}}AFX_MSG_MAP
END_MESSAGE_MAP()

BOOL PaZoneListCtrl::SetItemData(const PaZoneRecord& record)
{
    LOG_SCOPE("PaZoneListCtrl::SetItemData");
    LOG_DEBUG("SetItemData(): %s", nvps(record));

    int nextIndex = InsertItem(0, "");
    int colIndex = 0;

    try
    {
        /*
                // liuyu++ TD8848
                if (CachedConfig::getInstance()->isAtOcc())
                {
                    SetItemText(nextIndex, PA_COL_LOCATION_NAME, record.m_locationName.c_str());
                    m_colSortingInfo->defineSortingSemantic(PA_COL_LOCATION_NAME, AutoSortListCtrl::BY_CALLBACK);
                    SetItemText(nextIndex, PA_COL_LABEL, record.m_label.c_str());
                    m_colSortingInfo->defineSortingSemantic(PA_COL_LABEL, AutoSortListCtrl::BY_CALLBACK);
                    // TES #635 - use abbreviated codes for PUBLIC and NONPUBLIC text
                    if (stricmp(record.m_area.c_str(), "PUBLIC") == 0)
                    {
                        SetItemText(nextIndex, PA_COL_AREA, "P");
                    }
                    else if (stricmp(record.m_area.c_str(), "NONPUBLIC") == 0)
                    {
                        SetItemText(nextIndex, PA_COL_AREA, "NP");
                    }
                    else
                    {
                        // If both tags unrecognized, fall back to raw string value
                        SetItemText(nextIndex, PA_COL_AREA, record.m_area.c_str());
                    }
                    m_colSortingInfo->defineSortingSemantic(PA_COL_AREA, AutoSortListCtrl::BY_CALLBACK);

                    SetItemText(nextIndex, PA_COL_PRIORITY, getPriorityStr(record).c_str());
                    m_colSortingInfo->defineSortingSemantic(PA_COL_PRIORITY, AutoSortListCtrl::BY_CALLBACK);
                }
                else
                {
                    SetItemText(nextIndex, PA_COL_LOCATION_NAME, record.m_label.c_str());
                    m_colSortingInfo->defineSortingSemantic(PA_COL_LOCATION_NAME, AutoSortListCtrl::BY_CALLBACK);

                    // TES #635 - use abbreviated codes for PUBLIC and NONPUBLIC text
                    if (stricmp(record.m_area.c_str(), "PUBLIC") == 0)
                    {
                        SetItemText(nextIndex, PA_COL_LABEL, "P");
                    }
                    else if (stricmp(record.m_area.c_str(), "NONPUBLIC") == 0)
                    {
                        SetItemText(nextIndex, PA_COL_LABEL, "NP");
                    }
                    else
                    {
                        // If both tags unrecognized, fall back to raw string value
                        SetItemText(nextIndex, PA_COL_LABEL, record.m_area.c_str());
                    }
                    m_colSortingInfo->defineSortingSemantic(PA_COL_LABEL, AutoSortListCtrl::BY_CALLBACK);

                    SetItemText(nextIndex, PA_COL_AREA, getPriorityStr(record).c_str());
                    m_colSortingInfo->defineSortingSemantic(PA_COL_AREA, AutoSortListCtrl::BY_CALLBACK);
                }
                // ++liuyu TD8848
        */
        // song toan++
        // Bug #276
        //
        // Previous codes are replaced by codes below which work for all stations
        // without changing meaning of columns's name
        SetItemText(nextIndex,
                    PA_COL_LOCATION_NAME,
                    m_bIsAtOCC ? record.m_locationName.c_str() : record.m_label.c_str());

        m_colSortingInfo->defineSortingSemantic(PA_COL_LOCATION_NAME, AutoSortListCtrl::BY_CALLBACK);

        if (m_bIsAtOCC)
        {
            SetItemText(nextIndex, PA_COL_LABEL, record.m_label.c_str());
            m_colSortingInfo->defineSortingSemantic(PA_COL_LABEL, AutoSortListCtrl::BY_CALLBACK);
        }

        // TES #635 - use abbreviated codes for PUBLIC and NONPUBLIC text
        if (stricmp(record.m_area.c_str(), "PUBLIC") == 0)
        {
            SetItemText(nextIndex, PA_COL_AREA, "P");
        }
        else if (stricmp(record.m_area.c_str(), "NONPUBLIC") == 0)
        {
            SetItemText(nextIndex, PA_COL_AREA, "NP");
        }
        else
        {
            // If both tags unrecognized, fall back to raw string value
            SetItemText(nextIndex, PA_COL_AREA, record.m_area.c_str());
        }

        m_colSortingInfo->defineSortingSemantic(PA_COL_AREA, AutoSortListCtrl::BY_CALLBACK);

        SetItemText(nextIndex, PA_COL_PRIORITY, getPriorityStr(record).c_str());
        m_colSortingInfo->defineSortingSemantic(PA_COL_PRIORITY, AutoSortListCtrl::BY_CALLBACK);

        // ++song toan
        // Bug #276

        // liuyu++ TD8849
        // return CListCtrl::SetItemData(nextIndex, record.m_pKey);
        PaZoneRecord* paZoneRec = new PaZoneRecord(record);
        return CListCtrl::SetItemData(nextIndex, (DWORD)paZoneRec);
        // ++liuyu TD8849
    }
    catch (...)
    {
        // Don't allow SetItemData to fail (leaving undefined columns)
        return CListCtrl::SetItemData(nextIndex, TA_Base_Core::DatabaseKey::getInvalidKey());
    }
}

BOOL PaZoneListCtrl::DeleteAllItems()
{
    //int width = PaLayoutHelper::getListHeaderWidth(*this, true);

    // liuyu++ TD8849
    int i = 0;

    while (GetItemCount() > 0)
    {
        // get the item data
        PaZoneRecord* itemData = reinterpret_cast<PaZoneRecord*>(GetItemData(0));
        delete itemData;
        // delete the item
        DeleteItem(0);
    }

    // ++liuyu TD8849

    return CListCtrl::DeleteAllItems();
}

void PaZoneListCtrl::addNullEntry(unsigned long key)
{
    int nItem = InsertItem(0, "");

    // If at OCC, label column is widest
    if (m_bIsAtOCC)
    {
        SetItemText(nItem, PA_COL_LABEL, ZONE_NO_LONGER_EXISTS.c_str());
    }

    // Ensure invalid key set to here
    CListCtrl::SetItemData(nItem, TA_Base_Core::DatabaseKey::getInvalidKey());
}

void PaZoneListCtrl::initialise(PAAgentNamedObject& agent)
{
    LOG_SCOPE("PaZoneListCtrl::initialise");
    TA_ASSERT(m_hWnd, "hWnd NULL");

    // ++AZ TES 9613, tooltips set in ListCtrlSelNoFocus
    // SetExtendedStyle(LVS_EX_FULLROWSELECT);
    // TD15628
    SetExtendedStyle(LVS_EX_FULLROWSELECT | LVS_EX_INFOTIP);
    // TD15628
    // AZ++

    int colIndex = 0;

    // 336 is derived from getListHeaderWidth call (made AFTER dialog initialised)
    // jeffrey++ TD8850
    //int headerTotalWidth = 336;
    int headerTotalWidth = 355;		// extend to take up reminder space
    // ++jeffrey TD8850

    //[in order to use function getListHeaderWidth, must be called from PostDialogCreate]
    // PaLayoutHelper::getListHeaderWidth(*this, true);

    // All smaller columns have had pixels added to the width, so when the sorting
    // arrow is displayed the text isn't truncated

    // jeffrey++ TD8850
    //const int STN_WIDTH = 60;     // +10 for sorting
    int STN_WIDTH = 0;				// change from "const int" to "int" for calculating correct
    // ZONE_WIDTH for both OCC and Stations.
    // ++jeffrey TD8850

    const int AREA_WIDTH = 65;      // +15 for sorting
    const int EST_PRIOR_WIDTH = 80; // +15 for sorting

    // jeffrey++ TD8850
    // Make the zone details column take up any remainder space
    //const int ZONE_WIDTH = headerTotalWidth - STN_WIDTH - AREA_WIDTH - EST_PRIOR_WIDTH;
    // ++jeffrey TD8850

    // liuyu++ TD8848
    if (CachedConfig::getInstance()->isAtOcc())
    {
        // jeffrey++ TD8850
        STN_WIDTH = 60;
        // ++jeffrey TD8850

        // Station column always present (wasn't in earlier versions)
        InsertColumn(colIndex++, "LOC", LVCFMT_LEFT, STN_WIDTH);
    }

    // liuyu++ TD8848

    // jeffrey++ TD8850
    // Move to here from line 233.
    // To make sure the LOC space is added in ZONE_WIDTH in non-OCC cases and leave space for VScroll bar.
    // Make the zone details column take up any remainder space
    //const int ZONE_WIDTH = headerTotalWidth - STN_WIDTH - AREA_WIDTH - EST_PRIOR_WIDTH - GetSystemMetrics(SM_CXVSCROLL);
    const int ZONE_WIDTH = headerTotalWidth - STN_WIDTH - AREA_WIDTH - EST_PRIOR_WIDTH;
    // ++jeffrey TD8850

    InsertColumn(colIndex++, "Zone", LVCFMT_LEFT, ZONE_WIDTH);
    InsertColumn(colIndex++, "Area", LVCFMT_LEFT, AREA_WIDTH);
    // TES #714: Use "Priority" rather than "Estimated Priority" as column header
    InsertColumn(colIndex++, "Priority", LVCFMT_LEFT, EST_PRIOR_WIDTH);

    // Listen out for updates
    try
    {
        m_prioritySchemeUpdateSubscriber->addListenerWindow(m_hWnd);
    }
    catch (const TA_Base_Core::TransactiveException&)
    {
        // The ad-hoc list items will not be updated (nothing major, just can't view up to date info)
        PaErrorHandler::displayError(PaErrorHandler::INIT_ERROR);
    }

    // Attempt initial update of priority data
    // (important to do after subscribed to updates, to ensure up to date)
    try
    {
        CWaitCursor hourGlassCursor;

        TA_Base_Bus::IPAAgentCorbaDef::PriorityScheme* ret = NULL;
        CORBA_CALL_RETURN(ret, agent, getPriorityScheme, ());
        std::auto_ptr< TA_Base_Bus::IPAAgentCorbaDef::PriorityScheme >  priorityScheme(ret);

        refreshPriorityData(*priorityScheme.get());
    }
    catch (const TA_Base_Bus::IPAAgentCorbaDef::FepcCommsFailure&)
    {
        PaErrorHandler::displayError(PaErrorHandler::FEPC_HARDWARE_FAILURE);
    }
    catch (const TA_Base_Core::ObjectResolutionException&)
    {
        // Comms error
        PaErrorHandler::displayError(PaErrorHandler::PA_AGENT_FAIL);
    }
    catch (const TA_Base_Core::TransactiveException&)
    {
        PaErrorHandler::displayError(PaErrorHandler::UNSPECIFIED_ERROR);
    }
    catch (const CORBA::Exception&)
    {
        // Comms error
        PaErrorHandler::displayError(PaErrorHandler::PA_AGENT_FAIL);
    }

    m_fixedHeaderCtrl.subclassHeader(GetHeaderCtrl());
}

void PaZoneListCtrl::refreshPriorityData(const TA_Base_Bus::IPAAgentCorbaDef::PriorityScheme& priorityScheme)
{
    // Copy over the data set
    m_prioritySchemeData = priorityScheme;

    refreshZonePriorityDisplay();
}

const std::string PaZoneListCtrl::getPriorityStr(const PaZoneRecord& record) const
{
    LOG_SCOPE("PaZoneListCtrl::getPriorityStr");

    if (m_currentMode <= 0)
    {
        // Current mode doesn't allow priority calculations
        return PRIORITY_UNAVAILABLE;
    }

    unsigned long stationOffset = 0;

    try
    {
        stationOffset = TA_IRS_Bus::CachedMaps::getInstance()->getPasStationIdFromLocationKey(record.m_locationKey) - 1;
    }
    catch (const TA_Base_Core::CachedMappingNotFoundException&)
    {
        // Couldn't identify the PAS station ID (not sure what this means)
        return PRIORITY_UNAVAILABLE;
    }

    // Also covers if stationOffset somehow = -1
    if (stationOffset >= m_prioritySchemeData.length())
    {
        LOG_ERROR("No priority scheme data for this station");
        return PRIORITY_UNAVAILABLE;
    }

    unsigned long zoneOffset = record.m_id - 1;
    const TA_Base_Bus::IPAAgentCorbaDef::ZonePriorities& zoneData = m_prioritySchemeData[stationOffset];

    if (zoneOffset >= zoneData.length())
    {
        LOG_ERROR("zone ID not present in zonedata set");
        return PRIORITY_UNAVAILABLE;
    }

    unsigned long modeOffset = m_currentMode - 1;
    const TA_Base_Bus::IPAAgentCorbaDef::ModePriorities& modeData = zoneData[zoneOffset];

    if (modeOffset >= modeData.length())
    {
        LOG_ERROR("No data for the current mode");
        return PRIORITY_UNAVAILABLE;
    }

    std::ostringstream priorityStr;
    unsigned char priority = (unsigned char)modeData[modeOffset];

    // Represent as integer vlaue
    priorityStr << (unsigned int)priority;

    return priorityStr.str();
}

void PaZoneListCtrl::refreshZonePriorityDisplay()
{
    // Go through each item currently in list and update priority
    for (int i = 0; i < GetItemCount(); i++)
    {
        std::string priorityStr;

        try
        {
            PaZoneRecord* record = reinterpret_cast<PaZoneRecord*> (GetItemData(i));

            if (record)
            {
                priorityStr = getPriorityStr(*record);
            }
            else
            {
                priorityStr = PRIORITY_UNAVAILABLE;
            }
        }
        catch (const TA_Base_Core::TransactiveException&)
        {
            priorityStr = PRIORITY_UNAVAILABLE;
        }

        SetItemText(i, PA_COL_PRIORITY, priorityStr.c_str());
    }
}

/*
Keys PaZoneListCtrl::getSelectedKeys() const
{
    Keys result;

    std::vector<unsigned int> selection = TA_Base_Bus::SelectionHelper::getSelectedItemIndices(*this);

    for (int i = 0; i < selection.size(); i ++)
    {
        unsigned long keyVal = GetItemData(i);

        if (TA_Base_Core::DatabaseKey::isValidKey(keyVal))
        {
            result.push_back(keyVal);
        }
    }

    return result;
}
*/

Keys PaZoneListCtrl::getAllKeys() const
{
    Keys result;

    for (int i = 0; i < GetItemCount(); i++)
    {
        PaZoneRecord* record = reinterpret_cast<PaZoneRecord*> (GetItemData(i));

        if (record)
        {
            result.push_back(record->m_pKey);
        }
    }

    return result;
}

void PaZoneListCtrl::invalidateZoneData()
{
    m_currentZoneListKeys.clear();
}

bool PaZoneListCtrl::containsPrimaryKey(unsigned long zoneKey) const
{
    bool foundItem = (std::find(m_currentZoneListKeys.begin(), m_currentZoneListKeys.end(), zoneKey)
                      != m_currentZoneListKeys.end());

    return foundItem;
}

void PaZoneListCtrl::updateZoneList(const Keys& keys)
{
    LOG_SCOPE("PaZoneListCtrl::updateZoneList");

    LOG_DEBUG("updateZoneList(): %s", nvps(keys));

    if (keys == m_currentZoneListKeys)
    {
        // No change, nothing to do
        return;
    }

    m_currentZoneListKeys = keys;

    LockControlRedraw redrawLock(*this);        // Reduce flicker

    DeleteAllItems();

    for (Keys::const_iterator it = keys.begin(); it != keys.end(); ++it)
    {
        try
        {
            PaZoneRecord& record = TA_IRS_Bus::CachedMaps::getInstance()->getPaZoneRecordFromKey(*it);

            if (!ThisLocation::is_depot())
            {
                if (32 < record.m_id)
                {
                    LOG_DEBUG("updateZoneList(): ignore zone: key=%d, id=%d, name=%s", record.m_pKey, record.m_id, record.m_occZoneShortName);
                    continue;
                }
            }

            SetItemData(record);
        }
        catch (const TA_Base_Core::CachedMappingNotFoundException&)
        {
            // This zone no longer exists in database
            addNullEntry(*it);
        }
    }
}

afx_msg LRESULT PaZoneListCtrl::OnPrioritySchemeUpdate(UINT wParam, LONG lParam)
{
    try
    {
        refreshPriorityData(m_prioritySchemeUpdateSubscriber->getMessageData(wParam, m_hWnd));
    }

    STANDARD_PA_CATCH("PaZoneListCtrl::OnPrioritySchemeUpdate", PaErrorHandler::PRIORITY_SCHEME_FAILURE);

    return 0;
}

void PaZoneListCtrl::OnDestroy()
{
    CListCtrl::OnDestroy();

    try
    {
        m_prioritySchemeUpdateSubscriber->removeListenerWindow(m_hWnd);
    }
    catch (const TA_Base_Core::TransactiveException&)
    {
        // Cannot unsubscribe priority scheme updates
        PaErrorHandler::displayError(PaErrorHandler::UNSPECIFIED_ERROR);
    }
}

void PaZoneListCtrl::setPriorityEstimationMode(int newMode)
{
    if (newMode == m_currentMode)
    {
        // No change
        return;
    }

    m_currentMode = newMode;

    // Each time the mode changes, we must refresh the priority displayed to operator
    refreshZonePriorityDisplay();
}

LRESULT PaZoneListCtrl::WindowProc(UINT message, WPARAM wParam, LPARAM lParam)
{
    TA_Base_Bus::AutoSortListCtrl::WindowProc(message, wParam, lParam, *this, *m_colSortingInfo);

    return ListCtrlSelNoFocus::WindowProc(message, wParam, lParam);
}

// liuyu++ TD8849
int PaZoneListCtrl::sortCallback(LPARAM lParam1, LPARAM lParam2, int columnIndex, TA_Base_Bus::AutoSortListCtrl::ECurrentSort currentSort)
{
    PaZoneRecord* first = NULL;
    PaZoneRecord* second = NULL;

    if (currentSort == AutoSortListCtrl::DESCENDING)
    {
        first = reinterpret_cast<PaZoneRecord*>(lParam2);
        second = reinterpret_cast<PaZoneRecord*>(lParam1);
    }
    else
    {
        // ascending or unknown - which will default to ascending
        first = reinterpret_cast<PaZoneRecord*>(lParam1);
        second = reinterpret_cast<PaZoneRecord*>(lParam2);
    }

    // song toan++
    // Bug #276
    //
    // Previous codes are replaced by codes below which work for all stations
    // without changing meaning of columns being used
    if (columnIndex == PA_COL_LOCATION_NAME)
    {
        // geographical order
        int ret = first->m_orderId - second->m_orderId;

        if (ret == 0)
        {
            ret = first->m_label.compare(second->m_label);
        }

        return ret;
    }
    else if ((columnIndex == PA_COL_LABEL) && m_bIsAtOCC)
    {
        int ret = first->m_label.compare(second->m_label);

        if (ret == 0)
        {
            ret = first->m_orderId - second->m_orderId;
        }

        return ret;
    }
    else if (columnIndex == PA_COL_AREA)
    {
        return (first->m_area.compare(second->m_area));
    }
    else if (columnIndex == PA_COL_PRIORITY)
    {
        return (getPriorityStr(*first).compare(getPriorityStr(*second)));
    }
    else
    {
        TA_ASSERT(false, "Unsupported column given to sort by callback");
    }

    // ++song toan
    // Bug #276

    return 0;
}

// ++liuyu TD8849
