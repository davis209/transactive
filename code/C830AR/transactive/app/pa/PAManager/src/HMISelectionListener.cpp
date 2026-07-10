/**
 * The source code in this file is the property of
 * Ripple Systems and is not for redistribution
 * in any form.
 *
 * Source: $File: //depot/C830AR_TIP/C830AR/transactive/app/pa/PAManager/src/HMISelectionListener.cpp $
 * @author Jade Lee
 * @version $Revision: #4 $
 * Last modification: $DateTime: 2025/03/05 13:16:38 $
 * Last modified by: $Author: limin.zhu $
 *
 * HmiSelectionListener manages asynchronous Zone List updates received from the HMI,
 * posting out notifications as required (to the PA GUI)
 *
 */

#include "app/pa/PAManager/src/StdAfx.h"
#include "app/pa/PAManager/src/HmiSelectionListener.h"
#include "app/pa/PAManager/src/RunParamListener.h"
#include "app/pa/PAManager/src/CachedConfig.h"
#include "core/synchronisation/src/ThreadGuard.h"
#include "core/utilities/src/RunParams.h"
#include "core/exceptions/src/PasExceptions.h"
#include "core/utilities/src/TAAssert.h"
#include "core/utilities/src/RunParamsEx.h"
#include "bus/pa/cachedmaps/src/CachedMaps.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

using namespace TA_Base_Core;
using namespace TA_IRS_Core;
using namespace TA_IRS_Bus;

const std::string WILDCARD = "???";
// Special case selection constant, has same interpretation as WILDCARD
const std::string ALL_KEYWORD = "ALL";

HmiSelectionListener::HmiSelectionListener(HWND hWnd)
    :
    m_notificationHwnd(hWnd),
    m_duplicateNames(0),
    m_tagRegistrationComplete(false)
{
    LOG_SCOPE("HmiSelectionListener");

    // Register our interest for these tags with the main runparams interface
    // We will block any immediate processRunParamChange callbacks from within
    // these following calls (to ensure we begin with a blank selection)
    // by keeping m_tagRegistrationComplete == false
    RunParamListener::getInstance().registerListener(RunParamListener::DESELECT_TAG, *this);
    // Register the select tag second, to get the select statements
    // AFTER the deselect statements..
    RunParamListener::getInstance().registerListener(RunParamListener::SELECT_TAG, *this);
    // Register the overall selection tag last - this updates the selection fully
    // (in a non-cumulative manner, unlike the first two registered tags)
    RunParamListener::getInstance().registerListener(RunParamListener::UPDATE_SELECTION_TAG, *this);

    // Flag completion of registration
    m_tagRegistrationComplete = true;

    for (auto name : {RunParamListener::DESELECT_TAG, RunParamListener::SELECT_TAG, RunParamListener::UPDATE_SELECTION_TAG})
    {
        if (auto v = RunParamsEx::get_optional(name))
        {
            processRunParamChange(name, v.value());
        }
    }
}

HmiSelectionListener::~HmiSelectionListener()
{
    RunParamListener::getInstance().deRegisterListener(*this);
    TA_ASSERT(IsWindow(m_notificationHwnd), "Window destroyed before listener was");
}

void HmiSelectionListener::processRunParamChange(const std::string& name, const std::string& value)
{
    LOG_SCOPE("HmiSelectionListener::processRunParamChange");
    LOG_DEBUG("processRunParamChange(): %s", nvps(name, value));

    if (!m_tagRegistrationComplete)
    {
        // May ignore until all tags registered
        return;
    }

    bool selectionUpdated = false;

    if (boost::iequals(name, RunParamListener::SELECT_TAG))
    {
        selectionUpdated |= onZoneSelectionChange(value, SELECT);
    }
    else if (boost::iequals(name, RunParamListener::DESELECT_TAG))
    {
        selectionUpdated |= onZoneSelectionChange(value, DESELECT);
    }
    else if (boost::iequals(name, RunParamListener::UPDATE_SELECTION_TAG))
    {
        // The update selection tag provides individual entity selection data
        selectionUpdated |= processZoneRefresh(value, SELECT);
    }
    else
    {
        TA_ASSERT(FALSE, "Unrecognized run param change");
    }

    // Send out a notification that there is updated HMI group data
    if (selectionUpdated)
    {
        TA_ASSERT(IsWindow(m_notificationHwnd), "Window destroyed");
        PostMessage(m_notificationHwnd, WM_PA_STN_HMI_ZONE_SEL_CHANGE, 0, 0);
    }
}

const std::list<unsigned long> HmiSelectionListener::getZoneList()
{
    TA_Base_Core::ThreadGuard sync(m_zoneListLock);

    std::list<unsigned long> result;

    for (std::set<unsigned long>::iterator it = m_zoneList.begin();
         it != m_zoneList.end();
         it++)
    {
        result.push_back(*it);
    }

    return result;
}

void HmiSelectionListener::clearAllZonesFromSelection()
{
    TA_Base_Core::ThreadGuard sync(m_zoneListLock);
    m_zoneList.clear();
}

void HmiSelectionListener::modifyZoneSelection(const std::vector<unsigned long>& zones, EOperation operation)
{
    TA_Base_Core::ThreadGuard sync(m_zoneListLock);

    switch (operation)
    {
        case SELECT:
            // Add items to zone list set
            m_zoneList.insert(zones.begin(), zones.end());
            break;

        case DESELECT:
        {
            // Remove items from zone list set
            // (some or all may not be present, in which case no change)
            for (std::vector<unsigned long>::const_iterator itr = zones.begin();
                 itr != zones.end();
                 itr++)
            {
                m_zoneList.erase(*itr);
            }
        }
        break;

        default:
            TA_ASSERT(FALSE, "Unrecognized operation");
    };
}

std::string HmiSelectionListener::getTokenValue(const std::vector<std::string>& tokens,
                                                PaZoneRecord::ETokenIdentifier id)
{
    try
    {
        return tokens.at(id);
    }
    catch (const std::out_of_range&)
    {
        TA_THROW(TA_Base_Core::InvalidPasConfigurationException("Invalid arguments"));
    }
}

HmiSelectionListener::IChainedFilter* HmiSelectionListener::createNewFilter(const std::vector<std::string>& tokens,
                                                                            PaZoneRecord::ETokenIdentifier id)
{
    if ((unsigned int)id >= tokens.size())
    {
        // Out of range item, treat as wildcard
        return 0;
    }

    EDataPointType dataPointType = UNKNOWN;

    if (tokens.size() > 0)
    {
        const std::string occDatapointPrefix("OCC_");
        // Assume a station in the presence of a station string unless detect otherwise
        dataPointType = STATION;

        if (stricmp(tokens.at(0).c_str(), WILDCARD.c_str()) == 0)
        {
            // Wildcard for the station, meaning indeterminate (or rather doesn't matter)
            dataPointType = UNKNOWN;
        }
        else if (tokens.at(0).length() > occDatapointPrefix.length())
        {
            std::string tokenPrefix = tokens.at(0).substr(0, occDatapointPrefix.length()).c_str();

            if (stricmp(occDatapointPrefix.c_str(), tokenPrefix.c_str()) == 0)
            {
                dataPointType = OCC;
            }
        }
    }

    const std::string& value = tokens.at(id);

    if (stricmp(value.c_str(), WILDCARD.c_str()) == 0)
    {
        // Wildcard match returns null also
        return 0;
    }
    else
    {
        // If not a wildcard, use it as filtering parameter
        return new FilterPaZoneChain<std::string>(id, value, dataPointType);
    }
}

HmiSelectionListener::IChainedFilter* HmiSelectionListener::linkFilters(IChainedFilter* current,
                                                                        IChainedFilter* link)
{
    if (0 == link)
    {
        // Nothing to link
        return current;
    }
    else if (0 != current)
    {
        // Link to primary input
        current->addFilter(link);
        // Always return primary
        return current;
    }
    else
    {
        // Create new primary
        return link;
    }
}

std::vector<unsigned long> HmiSelectionListener::getSpecifiedZones(const std::vector<std::string>& tokens)
{
    LOG_SCOPE("HmiSelectionListener::getSpecifiedZones");
    LOG_DEBUG("getSpecifiedZones(): %s", nvps(tokens));

    // Filters will be null when wildcarded, or when the index doesn't exist
    std::auto_ptr<IChainedFilter> stationFilter(createNewFilter(tokens, PaZoneRecord::STATION));
    std::auto_ptr<IChainedFilter> areaFilter(createNewFilter(tokens, PaZoneRecord::AREA));
    std::auto_ptr<IChainedFilter> labelFilter(createNewFilter(tokens, PaZoneRecord::ZONESHORTNAME));

    IChainedFilter* completeFilter = 0;

    // Now chain all the filters together (if there are any filters)
    completeFilter = linkFilters(0, stationFilter.get());
    completeFilter = linkFilters(completeFilter, areaFilter.get());
    completeFilter = linkFilters(completeFilter, labelFilter.get());

    // If chainedFilter remains null, that is an acceptable parameter to getPaManagerSpecificZones
    // (means no filtering, all records will be returned)
    return getPaManagerSpecificZones(completeFilter);
}

// ExceptionChecked
bool HmiSelectionListener::processZoneRefresh(const std::string& value, EOperation operation)
{
    FUNCTION_ENTRY("processZoneRefresh");

    bool selectionUpdated = false;

    // The string 'value' has format <#count>,<entity>,<entity>...<entity>
    // Where #count is the total number of entities that will be sent through via RunParams
    // (and we wait until all are received before finally processing - stored in buffer in the interim)
    try
    {
        // jeffrey++ TD9615
        unsigned int count = 0;
        std::vector<std::string> tokens;
        unsigned int i = 0;

        if (value.find(";", 0) < value.size())
        {
            std::string::size_type foundOffset = value.find(",", 0);
            count = atoi(value.substr(0, foundOffset).c_str());
            const std::string& tmpvalue = value.substr(foundOffset + 1, value.size());
            tokens = GenericCachedMapBase::getAllTokens(tmpvalue, ";");
        }
        else
        {
            i = 1;
            tokens = GenericCachedMapBase::getAllTokens(value, ",");
            count = atoi(tokens[0].c_str());
        }

        // There should at least be the count token
        if (0 == tokens.size())
        {
            TA_THROW(TA_Base_Core::TransactiveException("processZoneRefresh: Invalid number of tokens present"));
        }

        for (; i < tokens.size(); i++)
            // ++jeffrey TD9615
        {
            std::string lowercase(tokens[i]);
            std::transform(lowercase.begin(), lowercase.end(), lowercase.begin(), tolower);

            // Insert strings as lowercase, to allow for case insensitive comparison later
            if (!m_entityNameBuffer.insert(lowercase).second)
            {
                // This entity was already present in the set - we're being sent duplicate names
                LOG_ERROR("Duplicate zone names being sent from schematics: %s", lowercase);

                // We cannot proceed, because the size of the set won't correspond with 'count'
                //TA_THROW(TA_Base_Core::TransactiveException("Duplicate zone names being sent from schematics"));
                // Can proceed if track the number of duplicate names
                m_duplicateNames++;
            }
        }

        unsigned int zonesParsed = m_duplicateNames + m_entityNameBuffer.size();

        if (zonesParsed > count)
        {
            // Shouldn't receive more entities than sent by schematics - reset internal state
            m_entityNameBuffer.clear();
            m_duplicateNames = 0;
            TA_THROW(TA_Base_Core::TransactiveException("Too many zone definitions received"));
        }
        else if (count == zonesParsed)
        {
            // We've received all the zones now - may update the selection
            refreshSelectionUsingZones(m_entityNameBuffer, operation);

            selectionUpdated = true;

            // Clear the buffer now that selection has been refreshed
            m_entityNameBuffer.clear();
            m_duplicateNames = 0;
        }
    }
    catch (...)
    {
        LOG(SourceInfo, TA_Base_Core::DebugUtil::ExceptionCatch, "Unknown", "Caught in HmiSelectionListener::processZoneRefresh");
    }

    FUNCTION_EXIT;

    return selectionUpdated;
}

void HmiSelectionListener::refreshSelectionUsingZones(const std::set<std::string>& zoneNames, EOperation operation)
{
    // Must translate zoneNames strings into actual IDs
    std::vector<unsigned long> zoneIds;

    // Extract all records - then need to build up string representation which can be searched
    // through the input set of zone names (if there's a match, add it to zoneIds array)
    PaZoneRecords allRecords = TA_IRS_Bus::CachedMaps::getInstance()->getPaZoneCachedMap().getAllRecords();

    for (PaZoneRecordsIt it = allRecords.begin(); it != allRecords.end(); it++)
    {
        std::string stringRepresentation = it->m_stnEquipmentEntityName;

        // Treat everything as lowercase, to make case insensitive
        std::transform(stringRepresentation.begin(),
                       stringRepresentation.end(),
                       stringRepresentation.begin(),
                       tolower);

        if (zoneNames.end() != zoneNames.find(stringRepresentation))
        {
            // This record matches one of the ones we want to select
            zoneIds.push_back(it->m_pKey);
            // Don't allow second addition
            continue;
        }

        // Insepect the OCC equipment entity name
        stringRepresentation = it->m_occEquipmentEntityName;

        // Treat everything as lowercase, to make case insensitive
        std::transform(stringRepresentation.begin(),
                       stringRepresentation.end(),
                       stringRepresentation.begin(),
                       tolower);

        if (zoneNames.end() != zoneNames.find(stringRepresentation))
        {
            // This record matches one of the ones we want to select
            zoneIds.push_back(it->m_pKey);
            continue;
        }
    }

    if (zoneIds.size() != zoneNames.size())
    {
        // Couldn't find the all of the input zone names in our cached map
        std::ostringstream ostr;
        ostr << "HmiSelectionListener::refreshSelectionUsingZones: Found "
            << zoneIds.size() << " zones, expected " << zoneNames.size();

        LOG_ERROR(ostr);
    }

    if (SELECT == operation)
    {
        // When refreshing with a SELECT, deselect everything first
        clearAllZonesFromSelection();
    }
    else if (DESELECT == operation)
    {
        // When refreshing with a DESELECT, select everything first
        addAllZonesToSelection();
    }

    modifyZoneSelection(zoneIds, operation);
}

void HmiSelectionListener::addAllZonesToSelection()
{
    LOG_DEBUG("addAllZonesToSelection()");

    // All keyword collects ALL zones in selection modification
    const std::vector<unsigned long> allZones = getPaManagerSpecificZones(nullptr);

    modifyZoneSelection(allZones, SELECT);
}

bool HmiSelectionListener::onZoneSelectionChange(const std::string& value, EOperation operation)
{
    LOG_SCOPE("HmiSelectionListener::onZoneSelectionChange");
    LOG_DEBUG("onZoneSelectionChange(): %s", nvps(value, operation));

    if (boost::iequals(value, ALL_KEYWORD))
    {
        // Treat the ALL keyword as a special case
        // For current implementation, it should behave identically to the ???
        // (except not allowed as wildcard in the extended AAA.PA.BBB.CCC format)
        if (DESELECT == operation)
        {
            // Optimization (for deselect operation only)
            clearAllZonesFromSelection();
        }
        else
        {
            addAllZonesToSelection();
        }
    }
    else if (boost::iequals(value, WILDCARD) && DESELECT == operation)
    {
        // Special case purely for efficiency reasons (generic update function could work)
        // Clear all zones from selection
        clearAllZonesFromSelection();
    }
    else
    {
        // Must check
        std::vector<std::string> tokens = GenericCachedMapBase::getAllTokens(value, ".");
        LOG_DEBUG("onZoneSelectionChange(): %s", nvps(tokens));
        modifyZoneSelection(getSpecifiedZones(tokens), operation);
    }

    return true;
}

template <class CompareValue>
HmiSelectionListener::FilterPaZoneChain<CompareValue>::FilterPaZoneChain(
    PaZoneRecord::ETokenIdentifier filterType,
    const CompareValue& value,
    EDataPointType datapointType)
    :
    m_filterType(filterType),
    m_filterValue(value),
    m_datapointType(datapointType)
{
}

template <class CompareValue>
void HmiSelectionListener::FilterPaZoneChain<CompareValue>::addFilter(const ICachedMapFilter<PaZoneRecord>* filter)
{
    TA_ASSERT(filter != 0, "Input filter is null");

    m_subFilters.push_back(filter);
}

template <class CompareValue>
bool HmiSelectionListener::FilterPaZoneChain<CompareValue>::isMatch(
    const PaZoneRecord& record) const
{
    bool match = false;

    switch (m_filterType)
    {
        case PaZoneRecord::STATION:

            // If we're at OCC, we will have loaded all the (for example) OCC_DBG.* records
            // But we may receive OCC_DBG.* OR DBG.* commands from the schematics (depending
            // on if they're running the overall or station level schematic).  Which means
            // we need to handle the special case OCC command specially (additionally, the
            // zoneShortName is different between matching OCC/Station zone records)
            if (UNKNOWN == m_datapointType || OCC == m_datapointType)
            {
                // Check location name against OCC location name
                match |= (0 == stricmp(m_filterValue.c_str(), record.m_occLocationName.c_str()));
            }

            if (UNKNOWN == m_datapointType || STATION == m_datapointType)
            {
                match |= (0 == stricmp(m_filterValue.c_str(), record.m_locationName.c_str()));
            }

            break;

        case PaZoneRecord::AREA:
            // Public / Non-Public
            match = (0 == stricmp(m_filterValue.c_str(), record.m_area.c_str()));
            break;

        case PaZoneRecord::ZONESHORTNAME:

            // m_label is the long name (ie Upper Concourse 1), PZON_01 is an example of the short name
            if (UNKNOWN == m_datapointType || OCC == m_datapointType)
            {
                // Check short name against the OCC zone name
                match |= (0 == stricmp(m_filterValue.c_str(), record.m_occZoneShortName.c_str()));
            }

            if (UNKNOWN == m_datapointType || STATION == m_datapointType)
            {
                match |= (0 == stricmp(m_filterValue.c_str(), record.m_stnZoneShortName.c_str()));
            }

            break;

        default:
            TA_ASSERT(FALSE, "Unknown match test");
    };

    if (!match)
    {
        // Doesn't matter if other filters match or not, primary match failed
        return false;
    }

    // Check all attached filters
    for (std::list<const ICachedMapFilter<PaZoneRecord>*>::const_iterator it = m_subFilters.begin();
         it != m_subFilters.end();
         it++)
    {
        if (!(*it)->isMatch(record))
        {
            // Any of sub filters return non-match, so do we
            return false;
        }
    }

    // If none of local or subcomponent match tests failed, all must have matched
    return true;
}

const std::vector<unsigned long> HmiSelectionListener::getPaManagerSpecificZones(IChainedFilter* filter)
{
    PaZoneGroupCachedMap::ZoneGroupFilter exclusionFilter(CachedConfig::getInstance()->isAtOcc(),
                                                          CachedConfig::getInstance()->getDepotLocationKey());

    ICachedMapFilter<PaZoneRecord>* combinedFilter = 0;

    // Append additional level of filtering
    if (nullptr == filter)
    {
        // No filter to combine with
        combinedFilter = &exclusionFilter;
    }
    else
    {
        filter->addFilter(&exclusionFilter);
        combinedFilter = filter;
    }

    return TA_IRS_Bus::CachedMaps::getInstance()->getPaZoneCachedMap().getZoneIdsUsingFilter(combinedFilter);
}
