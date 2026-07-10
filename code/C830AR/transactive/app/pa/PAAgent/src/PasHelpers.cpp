/**
 * The source code in this file is the property of
 * Ripple Systems and is not for redistribution
 * in any form.
 *
 * Source:   $File: //depot/C830AR_TIP/C830AR/transactive/app/pa/PAAgent/src/PasHelpers.cpp $
 * @author:  Ripple
 * @version: $Revision: #12 $
 *
 * Last modification: $DateTime: 2025/02/10 22:34:14 $
 * Last modified by:  $Author: limin.zhu $
 *
 */

#include "pch.h"
#include "AbstractBroadcast.h"
#include "CachedConfig.h"
#include "PasHelpers.h"
#include "bus/pa_4669/pa_agent/IDL/src/IPAAgentCorbaDef.h"
#include "bus/pa_4669/pa_agent/IDL/src/IPAAgentCorbaDefDescribe.h"
#include "core/alarm/src/AlarmHelper.h"
#include "core/alarm/src/AlarmHelperManager.h"
#include "core/alarm/src/NonUniqueAlarmHelper.h"
#include "core/data_access_interface/entity_access/src/ConsoleAccessFactory.h" // TD19669
#include "core/exceptions/src/PasExceptions.h"
#include "core/exceptions/src/ThrowException.h"
#include "core/message/src/AuditMessageSender.h"
#include "core/message/src/CommsMessageSender.h"
#include "core/message/src/MessagePublicationManager.h"
#include "core/message/src/StateUpdateMessageSender.h"
#include "core/message/types/PAAgentAlarms_MessageTypes.h"
#include "core/message/types/PAAgentAudit_MessageTypes.h"
#include "core/message/types/PAAgentComms_MessageTypes.h"
#include "core/message/types/PAAgentStateUpdate_MessageTypes.h"
#include "core/types/src/ta_types.h"
#include "core/utilities/src/DebugUtilEx.h"
#include "core/utilities/src/TAAssert.h"
#include "core/utilities/src/RunParams.h"
#include "core/utilities/src/stdutil/strings.h"
#include "core/utilities/src/MakeNameValuePairsString.h"
#include "core/utilities/src/stdutil/CorbaDefUtil.h"
#include "core/data_access_interface/src/LocationAccessFactory.h"
#include "core/data_access_interface/src/LocationAccessFactoryEx.h"
#include <boost/endian.hpp>
#include <boost/range/algorithm.hpp>
#include <ace/OS.h>
#include <algorithm>
#include <iomanip>
#include <iostream>
#include <list>
#include <map>
#include <bitset>

using namespace TA_Base_Core;
using namespace TA_IRS_Bus;
using namespace TA_IRS_App;

namespace
{
    template <class T>
    void try_catch(std::string desc, T f)
    {
        try
        {
            f();
        }
        catch (const std::exception& e)
        {
            LOG_ERROR("%s: %s", desc, e.what());
        }
        catch (...)
        {
            LOG_ERROR("%s: unknown", desc);
        }
    }
}

CachedConfig* PasHelpers::m_cachedConfigInstance = NULL;

PasHelpers::PasHelpers()
/*/TD17994, haipeng, TD18093
    :m_nonUniqueAlarmHelper(TA_Base_Core::AlarmHelperManager::getInstance().getNonUniqueAlarmHelper())
// ++liuyu TD4657
,m_alarmHelper(AlarmHelperManager::getInstance().getAlarmHelper())*/
{
    FUNCTION_ENTRY("PasHelpers");

    m_auditSender = MessagePublicationManager::getInstance().getAuditMessageSenderPtr(PAAgentAudit::Context);
    m_stateUpdateMessageSender = MessagePublicationManager::getInstance().getStateUpdateMessageSenderPtr(PAAgentStateUpdate::Context);
    m_paAgentCommsSender = MessagePublicationManager::getInstance().getCommsMessageSenderPtr(PAAgentComms::Context);

    if (!m_cachedConfigInstance)
    {
        m_cachedConfigInstance = CachedConfig::getInstance();
    }

    //TD18607
    m_pasAlarm = "";
    //TD18607

    FUNCTION_EXIT;
}

PasHelpers::~PasHelpers()
{
    FUNCTION_ENTRY("~PasHelpers");

    // liuyu++ TD4657
    TA_Base_Core::AlarmHelperManager::getInstance().cleanUp();
    // ++liuyu TD4657
    //TD18607
    m_pasAlarm = "";
    //TD18607

    FUNCTION_EXIT;
}

PasHelpers& PasHelpers::instance()
{
    static auto s_instance = new PasHelpers();
    return *s_instance;
}

TA_Base_Bus::IPAAgentCorbaDef::ZoneCoverage PasHelpers::remove_zones_where_id_larger_than(const TA_Base_Bus::IPAAgentCorbaDef::ZoneCoverage& coverage, size_t the_zone_id)
{
    LOG_SCOPE("PasHelpers::remove_zones_where_id_larger_than");
    FUNCTION_ENTRY("remove_zones_where_id_larger_than");

    std::vector<size_t> zones;
    zones.reserve(coverage.length());
    bool removed = false;

    for (size_t i = 0; i < coverage.length(); ++i)
    {
        try
        {
            auto id = CachedMaps::getInstance()->getPaZoneIdFromKey(coverage[i]);

            if (the_zone_id < id)
            {
                LOG_DEBUG("remove_zones_where_id_larger_than(): remove zone, key=%d, id=%d", coverage[i], id);
                removed = true;
                continue;
            }

            zones.push_back(id);
        }
        catch (...)
        {
        }
    }

    if (!removed)
    {
        FUNCTION_EXIT;
        return coverage;
    }

    TA_Base_Bus::IPAAgentCorbaDef::ZoneCoverage res;
    res.length(zones.size());

    for (auto i = 0; i < zones.size(); ++i)
    {
        res[i]  = zones[i];
    }

    FUNCTION_EXIT;
    return res;
}

TA_Base_Bus::IPAAgentCorbaDef::ZoneCoverage& PasHelpers::remove_zones_where_id_larger_than(TA_Base_Bus::IPAAgentCorbaDef::ZoneCoverage& coverage, size_t the_zone_id)
{
    auto res = remove_zones_where_id_larger_than(coverage, the_zone_id);

    if (res.length() != coverage.length())
    {
        coverage = res;
    }

    return coverage;
}

void PasHelpers::PAZoneKeysToCoverage(const Keys& keys, TA_Base_Bus::IPAAgentCorbaDef::ZoneCoverage& coverage)
{
    coverage = st::to_corba(keys);
}

void PasHelpers::coverageToPAZoneKeys(const TA_Base_Bus::IPAAgentCorbaDef::ZoneCoverage& coverage, Keys& keys)
{
    keys = st::to_cpp(coverage);
}

void PasHelpers::keysToLocalCoverageBitmap(const Keys& keys, std::uint32_t& localCoverageBitmap)
{
    FUNCTION_ENTRY("keysToLocalCoverageBitmap");

    localCoverageBitmap = 0;

    for (Keys::const_iterator it = keys.begin(); it != keys.end(); ++it)
    {
        try
        {
            int zoneId = CachedMaps::getInstance()->getPaZoneIdFromKey(*it);

            if (zoneId >= 1 && zoneId <= 32)
            {
                // zoneId is between 1 and 32 inclusive - so we set the appropriate bit
                std::uint32_t bit = 1;
                bit <<= (zoneId - 1);
                localCoverageBitmap |= bit;
            }
            else
            {
                // Handle in the catch block below
                TA_THROW(CachedMappingNotFoundException("PA Zone zoneId is out of range"));
            }
        }
        catch (CachedMappingNotFoundException& e)
        {
            handleCachedMappingNotFoundException(e);
            continue;
        }
    }

    FUNCTION_EXIT;
}

void PasHelpers::keysToLocalCoverageBitmap(const Keys& keys, std::uint64_t& localCoverageBitmap)
{
    FUNCTION_ENTRY("keysToLocalCoverageBitmap");

    localCoverageBitmap = 0;

    for (Keys::const_iterator it = keys.begin(); it != keys.end(); ++it)
    {
        try
        {
            int zoneId = CachedMaps::getInstance()->getPaZoneIdFromKey(*it);

            if (zoneId >= 1 && zoneId <= 64)
            {
                // zoneId is between 1 and 32 inclusive - so we set the appropriate bit
                std::uint64_t bit = 1;
                bit <<= (zoneId - 1);
                localCoverageBitmap |= bit;
            }
            else
            {
                // Handle in the catch block below
                TA_THROW(CachedMappingNotFoundException("PA Zone zoneId is out of range"));
            }
        }
        catch (CachedMappingNotFoundException& e)
        {
            handleCachedMappingNotFoundException(e);
        }
    }

    FUNCTION_EXIT;
}

void PasHelpers::keysToGlobalCoverageBitmaps(const Keys& keys, std::uint32_t* globalCoverageBitmaps)
{
    FUNCTION_ENTRY("keysToGlobalCoverageBitmaps");

    std::uint32_t* localCoverageBitmap;

    for (Keys::const_iterator it = keys.begin(); it != keys.end(); ++it)
    {
        try
        {
            int zoneId = CachedMaps::getInstance()->getPaZoneIdFromKey(*it);
            unsigned long locationKey = CachedMaps::getInstance()->getPaZoneLocationKeyFromKey(*it);
            unsigned int stationId = CachedMaps::getInstance()->getPasStationIdFromLocationKey(locationKey);

            localCoverageBitmap = &globalCoverageBitmaps[stationId - 1];

            if (zoneId >= 1 && zoneId <= 32)
            {
                // zoneId is between 1 and 32 inclusive - so we set the appropriate bit
                *localCoverageBitmap |= 1UL << (zoneId - 1);
            }
            else
            {
                // Handle in the catch block below
                TA_THROW(CachedMappingNotFoundException("PA Zone zoneId is out of range"));
            }
        }
        catch (CachedMappingNotFoundException& e)
        {
            // Unrecognised zoneId
            handleCachedMappingNotFoundException(e);
        }
    }

    FUNCTION_EXIT;
}

void PasHelpers::localCoverageBitmapToKeys(std::uint32_t localCoverageBitmap, Keys& keys, unsigned long locationKey)
{
    FUNCTION_ENTRY("localCoverageBitmapToKeys");

    keys.clear();

    for (int bit = 0; bit < 32; ++bit)
    {
        if ((localCoverageBitmap & (1UL << bit)) == 0)
        {
            // Bit is not set so we continue
            continue;
        }

        // Bit is set
        unsigned long paZoneId = bit + 1;

        try
        {
            unsigned long zoneKey = CachedMaps::getInstance()->getKeyFromPaZoneIdAndLocation(paZoneId, locationKey);
            keys.push_back(zoneKey);
        }
        catch (CachedMappingNotFoundException& e)
        {
            // Unrecognised zoneId
            handleCachedMappingNotFoundException(e);
        }
    }

    FUNCTION_EXIT;
}

void PasHelpers::localCoverageBitmapToKeys(std::uint64_t localCoverageBitmap, Keys& keys, unsigned long locationKey)
{
    FUNCTION_ENTRY("localCoverageBitmapToKeys");

    keys.clear();

    for (int bit = 0; bit < 64; ++bit)
    {
        if ((localCoverageBitmap & (1ULL << bit)) == 0)
        {
            // Bit is not set so we continue
            continue;
        }

        // Bit is set
        unsigned long paZoneId = bit + 1;

        try
        {
            unsigned long zoneKey = CachedMaps::getInstance()->getKeyFromPaZoneIdAndLocation(paZoneId, locationKey);
            keys.push_back(zoneKey);
        }
        catch (CachedMappingNotFoundException& e)
        {
            // Unrecognised zoneId
            handleCachedMappingNotFoundException(e);
        }
    }

    FUNCTION_EXIT;
}

void PasHelpers::globalCoverageBitmapsToKeys(const std::uint32_t* globalCoverageBitmaps, Keys& keys)
{
    FUNCTION_ENTRY("globalCoverageBitmapsToKeys");

    keys.clear();

    for (int stationId = 1; stationId <= TA_Base_Bus::IPAAgentCorbaDef::PA_MAXSTNID; ++stationId)
    {
        std::uint32_t localCoverageBitmap = globalCoverageBitmaps[stationId - 1];
        unsigned long locationKey(-1);

        try
        {
            locationKey = CachedMaps::getInstance()->getLocationKeyFromPasStationId(stationId);
        }
        catch (CachedMappingNotFoundException&)
        {
            // This is expected as not all of the PAS stations (1-50) are defined
            // Ignore all zones in this location
            continue;
        }

        for (int bit = 0; bit < 32; ++bit)
        {
            if ((localCoverageBitmap & (1UL << bit)) == 0)
            {
                // Bit is not set so we continue
                continue;
            }

            // Bit is set
            unsigned long paZoneId = bit + 1;

            try
            {
                unsigned long zoneKey = CachedMaps::getInstance()->getKeyFromPaZoneIdAndLocation(paZoneId, locationKey);
                keys.push_back(zoneKey);
            }
            catch (CachedMappingNotFoundException& e)
            {
                // Invalid configuration in the databases
                handleCachedMappingNotFoundException(e);
            }
        } // for bit
    } // for stationId

    FUNCTION_EXIT;
}

void PasHelpers::coverageToLocalCoverageBitmap(const TA_Base_Bus::IPAAgentCorbaDef::ZoneCoverage& coverage, std::uint32_t& localCoverageBitmap)
{
    FUNCTION_ENTRY("coverageToLocalCoverageBitmap");

    Keys keys;
    coverageToPAZoneKeys(coverage, keys);
    keysToLocalCoverageBitmap(keys, localCoverageBitmap);

    FUNCTION_EXIT;
}

void PasHelpers::coverageToLocalCoverageBitmap(const TA_Base_Bus::IPAAgentCorbaDef::ZoneCoverage& coverage, std::uint64_t& localCoverageBitmap)
{
    FUNCTION_ENTRY("coverageToLocalCoverageBitmap");

    Keys keys;
    coverageToPAZoneKeys(coverage, keys);
    keysToLocalCoverageBitmap(keys, localCoverageBitmap);

    FUNCTION_EXIT;
}

void PasHelpers::localCoverageBitmapToCoverage(std::uint32_t localCoverageBitmap, TA_Base_Bus::IPAAgentCorbaDef::ZoneCoverage& coverage, unsigned long locationKey)
{
    FUNCTION_ENTRY("localCoverageBitmapToCoverage");

    Keys keys;
    localCoverageBitmapToKeys(localCoverageBitmap, keys, locationKey);
    PAZoneKeysToCoverage(keys, coverage);

    FUNCTION_EXIT;
}

void PasHelpers::localCoverageBitmapToCoverage(std::uint64_t localCoverageBitmap, TA_Base_Bus::IPAAgentCorbaDef::ZoneCoverage& coverage, unsigned long locationKey)
{
    FUNCTION_ENTRY("localCoverageBitmapToCoverage");

    Keys keys;
    localCoverageBitmapToKeys(localCoverageBitmap, keys, locationKey);
    PAZoneKeysToCoverage(keys, coverage);

    FUNCTION_EXIT;
}

void PasHelpers::coverageToGlobalCoverageBitmaps(const TA_Base_Bus::IPAAgentCorbaDef::ZoneCoverage& coverage, std::uint32_t* globalCoverageBitmaps)
{
    FUNCTION_ENTRY("coverageToGlobalCoverageBitmaps");

    Keys keys;
    coverageToPAZoneKeys(coverage, keys);
    keysToGlobalCoverageBitmaps(keys, globalCoverageBitmaps);

    FUNCTION_EXIT;
}

void PasHelpers::globalCoverageBitmapsToCoverage(const std::uint32_t* globalCoverageBitmaps, TA_Base_Bus::IPAAgentCorbaDef::ZoneCoverage& coverage)
{
    FUNCTION_ENTRY("globalCoverageBitmapsToCoverage");

    Keys keys;
    globalCoverageBitmapsToKeys(globalCoverageBitmaps, keys);
    PAZoneKeysToCoverage(keys, coverage);

    FUNCTION_EXIT;
}

std::vector<int> PasHelpers::local_coverage_bitmap_to_zone_id(std::uint32_t localCoverageBitmap)
{
    std::vector<int> zones;
    zones.reserve(32);
    std::bitset<32> bits{localCoverageBitmap};

    for (auto i = 0; i < 32; ++i)
    {
        if (bits[i])
        {
            zones.emplace_back(i + 1);
        }
    }

    return zones;
}

std::vector<int> PasHelpers::local_coverage_bitmap_to_zone_id(std::uint64_t localCoverageBitmap)
{
    std::vector<int> zones;
    zones.reserve(64);
    std::bitset<64> bits{localCoverageBitmap};

    for (auto i = 0; i < 64; ++i)
    {
        if (bits[i])
        {
            zones.emplace_back(i + 1);
        }
    }

    return zones;
}

std::vector<int> PasHelpers::local_zone_coverage_to_zone_id(const TA_Base_Bus::IPAAgentCorbaDef::ZoneCoverage& coverage)
{
    std::vector<int> zones;
    zones.reserve(coverage.length());

    for (auto i = 0; i < coverage.length(); ++i)
    {
        try
        {
            int zoneId = CachedMaps::getInstance()->getPaZoneIdFromKey(coverage[i]);
            LOG_ERROR_IF(zoneId < 1 || 64 < zoneId, boost::format("PA Zone zoneId is out of range: key=%d, id=%d") % coverage[i] % zoneId);
            zones.emplace_back(zoneId);
        }
        catch (CachedMappingNotFoundException& e)
        {
            handleCachedMappingNotFoundException(e);
        }
    }

    boost::sort(zones);
    return zones;
}

bool PasHelpers::zoneIsAPagingConsole(const TA_Base_Bus::IPAAgentCorbaDef::ZoneCoverage& zones)
{
    FUNCTION_ENTRY("zoneIsAPagingConsole");

    // if there is more than one zone, then this cant be a local console
    // if there is none, then again it cant be a local console
    if (zones.length() != 1)
    {
        FUNCTION_EXIT;
        return false;
    }

    FUNCTION_EXIT;
    return zoneIsAPagingConsole(zones[0]);
}

bool PasHelpers::zoneIsAPagingConsole(unsigned long zone)
{
    FUNCTION_ENTRY("zoneIsAPagingConsole");

    // now see if the console Id is in the list of paging consoles
    std::list<unsigned long> pagingConsoleKeys = CachedConfig::getInstance()->getPagingConsoleKeys();

    try
    {
        auto zoneId = CachedMaps::getInstance()->getPaZoneIdFromKey(zone);
        auto it = std::find(pagingConsoleKeys.begin(), pagingConsoleKeys.end(), zoneId);
        auto isPGC = it != pagingConsoleKeys.end();
        LOG_DEBUG("zoneIsAPagingConsole(): %s", nvps(zone, zoneId, isPGC));

        FUNCTION_EXIT;
        return isPGC;
    }
    catch (...)
    {
        // invalid zone - it will be validated as part of the broadcast
    }

    FUNCTION_EXIT;
    return false;
}

// TD19669
bool PasHelpers::zoneIsAPagingConsole(unsigned long sourceId, const std::string& session)
{
    FUNCTION_ENTRY("zoneIsAPagingConsole");

    try
    {
        IConsole* localConsole = ConsoleAccessFactory::getInstance().getConsoleFromSession(session, false);
        unsigned long pagingConsoleZoneId   = localConsole->getPagingConsoleZoneId();

        LOG_INFO("zoneIsAPagingConsole(): Console=%s PagingConsoleZoneId=%d", localConsole->getName(), pagingConsoleZoneId);

        if (sourceId == localConsole->getPagingConsoleSourceId())
        {
            std::list<unsigned long> pagingConsoleKeys = CachedConfig::getInstance()->getPagingConsoleKeys();

            auto it = std::find(pagingConsoleKeys.begin(), pagingConsoleKeys.end(), pagingConsoleZoneId);
            auto isPGC = it != pagingConsoleKeys.end();
            LOG_DEBUG("zoneIsAPagingConsole(): %s", nvps(sourceId, session, isPGC));

            FUNCTION_EXIT;
            return isPGC;
        }
        else
        {
            LOG_ERROR("The specified source [%d] does not match with this console's sourceId.", sourceId);
        }

        delete localConsole;
    }
    catch (...)
    {
        // invalid zone - it will be validated as part of the broadcast
    }

    FUNCTION_EXIT;
    return false;
}

// TD19669
// Note can return -1 if messageKey is not found
int PasHelpers::getAdhocOffset(unsigned long messageKey)
{
    FUNCTION_ENTRY("getAdhocOffset");

    unsigned long messageId = 0;

    try
    {
        messageId = CachedMaps::getInstance()->getStationDvaMessageIdFromKey(messageKey);
    }
    catch (CachedMappingNotFoundException& e)
    {
        handleCachedMappingNotFoundException(e);

        FUNCTION_EXIT;
        return -1;
    }

    int adhocMessageOffset = 0;

    if (CachedConfig::getInstance()->getIsMaster())
    {
        adhocMessageOffset = messageId - 1;
    }
    else
    {
        if (CachedConfig::getInstance()->getIsDepot())
        {
            adhocMessageOffset = messageId - 1;
        }
        else
        {
            // Normal station
            adhocMessageOffset = messageId - TA_Base_Bus::IPAAgentCorbaDef::PA_NBPREREC
                - TA_Base_Bus::IPAAgentCorbaDef::PA_NBADHOC
                - 1;
        }
    }

    FUNCTION_EXIT;
    return adhocMessageOffset;
}

#if 0
void PasHelpers::hexDumpDebug(const char* prefix, const std::uint8_t* buffer, size_t length)
{
    FUNCTION_ENTRY("hexDumpDebug");

    static const int MAX_BYTES_LOGGED = 128;

    if (DebugUtil::getInstance().getLevel() == DebugUtil::DebugDebug)
    {
        std::stringstream ss;

        ss << prefix << std::hex;

        for (int i = 0; i < length; i++)
        {
            if (i >= MAX_BYTES_LOGGED)
            {
                ss << " ...";
                break;
            }

            unsigned int value = 0xff & (unsigned int)buffer[i];
            ss << " 0x" << value;
        }

        LOG_DEBUG(ss);
    }

    FUNCTION_EXIT;
}
#else
void PasHelpers::hexDumpDebug(const char* prefix, const std::uint8_t* buffer, size_t length, std::string delimiter)
{
    LOG_DEBUG("%s(%zu bytes): %s", prefix, length, st::to_hex_string(buffer, length, delimiter));
}
#endif

void PasHelpers::setBit(std::uint8_t* buffer, unsigned int bitOffset, bool value)
{
    FUNCTION_ENTRY("setBit");

    unsigned int byteOffset = bitOffset / 8;
    unsigned int byteBitOffset = bitOffset % 8;

    std::uint8_t mask = 1 << byteBitOffset;

    if (value)
    {
#if 0
        // Set bit
        buffer[byteOffset] = buffer[byteOffset] & mask;
#else
        buffer[byteOffset] = buffer[byteOffset] | mask;
#endif
    }
    else
    {
        // Remove bit
        std::uint8_t invertedMask = ~mask;

        buffer[byteOffset] = buffer[byteOffset] & invertedMask;
    }

    FUNCTION_EXIT;
}

bool PasHelpers::getBit(std::uint8_t* buffer, unsigned int bitOffset)
{
    FUNCTION_ENTRY("getBit");
    unsigned int byteOffset = bitOffset / 8;
    unsigned int byteBitOffset = bitOffset % 8;

    std::uint8_t mask = 1 << byteBitOffset;

    FUNCTION_EXIT;
    return (buffer[byteOffset] & mask) != 0;
}

void PasHelpers::set8bit(std::uint8_t* buffer, unsigned long offset, std::uint8_t value)
{
    FUNCTION_ENTRY("set8bit");

    buffer[offset] = value;

    FUNCTION_EXIT;
}

// Host to Big endian / Network Byte order conversion
//
// 0x1234 ->   0x12      0x34
//           [offset] [offset+1]
//
void PasHelpers::set16bit(std::uint8_t* buffer, unsigned long offset, std::uint16_t value)
{
    FUNCTION_ENTRY("set16bit");

    set8bit(buffer, offset, (value >> 8));      // (value/0x0100)
    set8bit(buffer, offset + 1, (value & 0x00ff));  // (value%0x0100)

    FUNCTION_EXIT;
}

// Host to Big endian / Network Byte order conversion
//
// 0x12345678 ->   0x12      0x34       0x56       0x78
//               [offset] [offset+1] [offset+2] [offset+3]
//
void PasHelpers::set32bit(std::uint8_t* buffer, unsigned long offset, std::uint32_t value)
{
    FUNCTION_ENTRY("set32bit");

    set16bit(buffer, offset, static_cast<std::uint16_t>(value >> 16));        // (value/0x00010000)
    set16bit(buffer, offset + 2, static_cast<std::uint16_t>(value & 0x0000ffff)); // (value%0x00010000)

    FUNCTION_EXIT;
}

void PasHelpers::set64bit(std::uint8_t* buffer, unsigned long offset, std::uint64_t value)
{
    boost::endian::store_big_u64(buffer + offset, value);
}

std::uint8_t PasHelpers::get8bit(std::uint8_t* buffer, unsigned long offset)
{
    FUNCTION_ENTRY("get8bit");
    FUNCTION_EXIT;
    return buffer[offset];
}

// Big endian / Network Byte order to Host conversion
//
//    0x12      0x34     -> 0x1234
//  [offset] [offset+1]
//
std::uint16_t PasHelpers::get16bit(std::uint8_t* buffer, unsigned long offset)
{
    FUNCTION_ENTRY("get16bit");

    std::uint16_t mostSignificantByte  = get8bit(buffer, offset);
    std::uint16_t leastSignificantByte = get8bit(buffer, offset + 1);

    FUNCTION_EXIT;
    return (mostSignificantByte << 8) + leastSignificantByte;
}

// Big endian / Network Byte order to Host conversion
//
//    0x12      0x34       0x56       0x78    -> 0x12345678
//  [offset] [offset+1] [offset+2] [offset+3]
//
std::uint32_t PasHelpers::get32bit(std::uint8_t* buffer, unsigned long offset)
{
    FUNCTION_ENTRY("get32bit");

    std::uint32_t mostSignificantShort  = get16bit(buffer, offset);
    std::uint32_t leastSignificantShort = get16bit(buffer, offset + 2);

    FUNCTION_EXIT;
    return (mostSignificantShort << 16) + leastSignificantShort;
}

std::uint64_t PasHelpers::get64bit(std::uint8_t* buffer, unsigned long offset)
{
    return boost::endian::load_big_u64(buffer + offset);
}

void PasHelpers::throwBroadcastIdNotFoundException(const std::string& what, const std::string& broadcastId) //throw(TA_Base_Bus::IPAAgentCorbaDef::BroadcastIdNotFound)
{
    FUNCTION_ENTRY("throwBroadcastIdNotFoundException");

    std::ostringstream msg;
    msg << what << " " << broadcastId;

    TA_Base_Bus::IPAAgentCorbaDef::BroadcastIdNotFound exception;
    exception.what = msg.str().c_str();
    LOG_ERROR(msg);
    LOGOUT(msg.str());

    FUNCTION_EXIT;
    TA_THROW(exception);

    FUNCTION_EXIT;
}

void PasHelpers::throwFepcCommsFailureException(const std::string& what, const std::string& broadcastId) //throw(TA_Base_Bus::IPAAgentCorbaDef::FepcCommsFailure)
{
    FUNCTION_ENTRY("throwFepcCommsFailureException");

    // Audit failure message for tracking failed requests
    TA_Base_Bus::IPAAgentCorbaDef::FepcCommsFailure exception;
    exception.what = what.c_str();
    LOG_ERROR(what);
    LOGOUT(what);

    FUNCTION_EXIT;
    TA_THROW(exception);

    FUNCTION_EXIT;
}

void PasHelpers::throwBroadcastLimitReachedException(const std::string& broadcastId) //throw(TA_Base_Bus::IPAAgentCorbaDef::FepcCommsFailure)
{
    FUNCTION_ENTRY("throwBroadcastLimitReachedException");

    std::ostringstream msg;
    msg << "Request failed due to broadcast limit ("
        << CachedConfig::getInstance()->getMaxBroadcasts()
        << ")";

    TA_Base_Bus::IPAAgentCorbaDef::BroadcastLimitReached exception;
    exception.what = msg.str().c_str();
    LOG_ERROR(msg);
    LOGOUT(msg.str());

    FUNCTION_EXIT;
    TA_THROW(exception);

    FUNCTION_EXIT;
}

void PasHelpers::throwTrainAgentCommsFailureException(const std::string& what, const std::string& broadcastId)
//throw(TA_Base_Bus::IPAAgentCorbaDef::TrainAgentCommsFailure)
{
    FUNCTION_ENTRY("throwTrainAgentCommsFailureException");

    TA_Base_Bus::IPAAgentCorbaDef::TrainAgentCommsFailure exception;
    exception.what = what.c_str();
    LOG_ERROR(what);
    LOGOUT(what);

    FUNCTION_EXIT;
    TA_THROW(exception);

    FUNCTION_EXIT;
}

void PasHelpers::throwAccessDeniedException(const std::string& reason, const std::string& broadcastId)
//throw(TA_Base_Core::AccessDeniedException)
{
    FUNCTION_ENTRY("throwAccessDeniedException");

    TA_Base_Core::AccessDeniedException exception;
    exception.reason = reason.c_str();
    LOG_ERROR(reason);
    LOGOUT(reason);

    FUNCTION_EXIT;
    TA_THROW(exception);

    FUNCTION_EXIT;
}

/*
void PasHelpers::throwBadParameterException(const std::string& reason, const std::string& broadcastId)
    //throw(TA_Base_Core::BadParameterException)
{
    // Audit failure message for tracking failed requests
    std::string auditMessage("Request failed due to invalid parameter - ");
    auditMessage += reason;
    submitAuditMessage(auditMessage, true, "", broadcastId);

    TA_Base_Core::BadParameterException exception;
    exception.reason = reason.c_str();
    LOG_ERROR(reason);
    LOGOUT(reason);
    TA_THROW(exception);
}

void PasHelpers::throwDatabaseErrorException(const std::string& reason, const std::string& broadcastId)
    //throw(TA_Base_Core::DatabaseErrorException)
{
    TA_Base_Core::DatabaseErrorException exception;
    exception.reason = reason.c_str();
    LOG_ERROR(reason);
    LOGOUT(reason);
    TA_THROW(exception);
}

void PasHelpers::throwTisAgentCommsFailureException(const std::string& what, const std::string& broadcastId)
    //throw(TA_Base_Bus::IPAAgentCorbaDef::TisAgentCommsFailure)
{
    TA_Base_Bus::IPAAgentCorbaDef::TisAgentCommsFailure exception;
    exception.what = what.c_str();
    LOG_ERROR(what);
    LOGOUT(what);
    TA_THROW(exception);
}

*/

void PasHelpers::handleCachedMappingNotFoundException(const TA_Base_Core::CachedMappingNotFoundException& e, const std::string& description)
{
    FUNCTION_ENTRY("handleCachedMappingNotFoundException");

    if (description == "")
    {
        LOG_ERROR(e.what());
    }
    else
    {
        LOG_ERROR(description);
    }

    FUNCTION_EXIT;
}

////////////////////////////////////////////////////////
//
//            Audit message senders
//
////////////////////////////////////////////////////////

void PasHelpers::submitPaBroadcastStationDvaRequestAuditMessage(const std::string& broadcastId,
                                                                const TA_Base_Bus::IPAAgentCorbaDef::MessageKeySeq& messages,
                                                                unsigned short dwellInSecs,
                                                                bool hasChime,
                                                                const TA_Base_Core::DateTime& startTime,
                                                                const TA_Base_Core::DateTime& stopTime,
                                                                unsigned short periodInSecs,
                                                                bool isSynchronisedWithTis,
                                                                const TA_Base_Bus::IPAAgentCorbaDef::ZoneCoverage& zones,
                                                                bool isLocalPlayback,
                                                                const std::string& sessionId)
{
    LOG_SCOPE("submitPaBroadcastStationDvaRequestAuditMessage");
    FUNCTION_ENTRY("submitPaBroadcastStationDvaRequestAuditMessage");

    // build the parameters
    DescriptionParameters parameters;

    // TD17449
    auto locationName = ThisLocation::name();
    NameValuePair locationNameParam("LocationName", locationName);
    parameters.push_back(&locationNameParam);
    // TD17449

    NameValuePair broadcastIdParam("BroadcastId", broadcastId);
    parameters.push_back(&broadcastIdParam);

    std::ostringstream messageList;

    try
    {
        for (int i = 0; i < messages.length(); ++i)
        {
            messageList << CachedMaps::getInstance()->getStationDvaMessageIdFromKey(messages[i]);

            if (i != (messages.length() - 1))
            {
                // avoid adding comma for the last item
                messageList << ",";
            }
        }
    }
    catch (TA_Base_Core::CachedMappingNotFoundException& e)
    {
        LOG_EXCEPTION_CATCH(SourceInfo, "TA_Base_Core::CachedMappingNotFoundException", e.what());

        FUNCTION_EXIT;
        TA_THROW(TA_Base_Core::BadParameterException("The specified message does not exist in the database"));
    }

    NameValuePair messageIdsParam("ListOfMessageIds", messageList.str());
    parameters.push_back(&messageIdsParam);

    std::ostringstream temp;
    temp << dwellInSecs;
    NameValuePair dwellParam("Dwell", temp.str());
    parameters.push_back(&dwellParam);

    temp.str("");
    temp << hasChime;
    NameValuePair chimeParam("HasChime", temp.str());
    parameters.push_back(&chimeParam);

    NameValuePair startParam("Start", formatTime(startTime));
    parameters.push_back(&startParam);

    NameValuePair stopParam("Stop", formatTime(stopTime));
    parameters.push_back(&stopParam);

    temp.str("");
    temp << periodInSecs;
    NameValuePair periodParam("Period", temp.str());
    parameters.push_back(&periodParam);

    temp.str("");
    temp << isSynchronisedWithTis;
    NameValuePair syncParam("IsTisSynchronised", temp.str());
    parameters.push_back(&syncParam);

    NameValuePair zoneParam("ListOfLocationsAndZones", formatZones(zones));
    parameters.push_back(&zoneParam);

    // jeffrey++ TD10365, TD10393
    std::ostringstream messageLabelList;

    try
    {
        for (int i = 0; i < messages.length(); ++i)
        {
            StationDvaMessageRecord record = CachedMaps::getInstance()->getStationDvaMessageRecordFromKey(messages[i]);
            messageLabelList << checkLabel(record.m_label);

            if (i != (messages.length() - 1))
            {
                // avoid adding comma for the last item
                messageLabelList << ",";
            }
        }
    }
    catch (TA_Base_Core::CachedMappingNotFoundException& e)
    {
        LOG_EXCEPTION_CATCH(SourceInfo, "TA_Base_Core::CachedMappingNotFoundException", e.what());

        FUNCTION_EXIT;
        TA_THROW(TA_Base_Core::BadParameterException("The specified message does not exist in the database"));
    }

    NameValuePair messageLabelsParam("MsgLabels", messageLabelList.str());
    parameters.push_back(&messageLabelsParam);

    // ++jeffrey TD10365

    // send the message
    if (!isLocalPlayback)
    {
        submitAuditMessage(TA_Base_Core::PAAgentAudit::PaBroadcastStationDvaRequest,
                           parameters,
                           sessionId);
    }

    FUNCTION_EXIT;
}

void PasHelpers::submitPaBroadcastStationLiveRequestAuditMessage(const std::string& broadcastId,
                                                                 unsigned short sourceId,
                                                                 const TA_Base_Bus::IPAAgentCorbaDef::ZoneCoverage& zones,
                                                                 const std::string& sessionId)
{
    LOG_SCOPE("submitPaBroadcastStationLiveRequestAuditMessage");
    FUNCTION_ENTRY("submitPaBroadcastStationLiveRequestAuditMessage");

    // build the parameters
    DescriptionParameters parameters;

    // jeffrey++ TD10320
    auto locationName = ThisLocation::name();
    NameValuePair locationNameParam("LocationName", locationName);
    parameters.push_back(&locationNameParam);
    // ++jeffrey TD10320

    NameValuePair broadcastIdParam("BroadcastId", broadcastId);
    parameters.push_back(&broadcastIdParam);

    std::ostringstream temp;
    temp << static_cast<int>(sourceId);
    NameValuePair sourceIdParam("Source", temp.str());
    parameters.push_back(&sourceIdParam);

    NameValuePair zoneParam("ListOfLocationsAndZones", formatZones(zones));
    parameters.push_back(&zoneParam);

    // send the message
    submitAuditMessage(TA_Base_Core::PAAgentAudit::PaBroadcastStationLiveRequest, parameters, sessionId);

    FUNCTION_EXIT;
}

void PasHelpers::submitPaBroadcastStationMusicRequestAuditMessage(const std::string& broadcastId,
                                                                  const TA_Base_Bus::IPAAgentCorbaDef::EMusicType& musicType,
                                                                  const std::string& sessionId)
{
    LOG_SCOPE("submitPaBroadcastStationMusicRequestAuditMessage");
    FUNCTION_ENTRY("submitPaBroadcastStationMusicRequestAuditMessage");

    // build the parameters
    DescriptionParameters parameters;

    NameValuePair broadcastIdParam("BroadcastId", broadcastId);
    parameters.push_back(&broadcastIdParam);

    std::string musicTypeStr;

    switch (musicType)
    {
        // liuyu++ TD8857
        // case TA_Base_Bus::IPAAgentCorbaDef::NoMusic:
        case TA_Base_Bus::IPAAgentCorbaDef::NoOccMusic:
            musicTypeStr = "None";
            break;

        case TA_Base_Bus::IPAAgentCorbaDef::NoLocalMusic:
            musicTypeStr = "None";
            break;

            // ++liuyu TD8857
        case TA_Base_Bus::IPAAgentCorbaDef::LocalMusic:
            musicTypeStr = "Local";
            break;

        case TA_Base_Bus::IPAAgentCorbaDef::OccMusic:
            musicTypeStr = "Occ";
            break;
    }

    NameValuePair musicSourceParam("NoneLocalOrOcc", musicTypeStr);
    parameters.push_back(&musicSourceParam);

    // send the message
    submitAuditMessage(TA_Base_Core::PAAgentAudit::PaBroadcastStationMusicRequest, parameters, sessionId);

    FUNCTION_EXIT;
}

void PasHelpers::submitPaRecordAdhocRequestAuditMessage(const std::string& broadcastId,
                                                        unsigned char sourceId,
                                                        unsigned long messageKey,
                                                        const std::string& sessionId)
{
    LOG_SCOPE("submitPaRecordAdhocRequestAuditMessage");
    FUNCTION_ENTRY("submitPaRecordAdhocRequestAuditMessage");

    // build the parameters
    DescriptionParameters parameters;

    NameValuePair broadcastIdParam("BroadcastId", broadcastId);
    parameters.push_back(&broadcastIdParam);

    std::ostringstream temp;
    temp << static_cast<int>(sourceId);
    NameValuePair sourceIdParam("Source", temp.str());
    parameters.push_back(&sourceIdParam);

    // jeffrey++ TD10365, TD10393, TD 13609
    std::ostringstream messageId;

    try
    {
        messageId << CachedMaps::getInstance()->getStationDvaMessageIdFromKey(messageKey);
    }
    catch (TA_Base_Core::CachedMappingNotFoundException& e)
    {
        LOG_EXCEPTION_CATCH(SourceInfo, "TA_Base_Core::CachedMappingNotFoundException", e.what());

        FUNCTION_EXIT;
        TA_THROW(TA_Base_Core::BadParameterException("The specified message does not exist in the database"));
    }

    NameValuePair messageIdParam("MessageId", messageId.str());
    parameters.push_back(&messageIdParam);

    std::ostringstream messageLabel;

    try
    {
        StationDvaMessageRecord record = CachedMaps::getInstance()->getStationDvaMessageRecordFromKey(messageKey);
        messageLabel << checkLabel(record.m_label);
    }
    catch (TA_Base_Core::CachedMappingNotFoundException& e)
    {
        LOG_EXCEPTION_CATCH(SourceInfo, "TA_Base_Core::CachedMappingNotFoundException", e.what());

        FUNCTION_EXIT;
        TA_THROW(TA_Base_Core::BadParameterException("The specified message does not exist in the database"));
    }

    NameValuePair messageLabelsParam("MsgLabel", messageLabel.str());
    parameters.push_back(&messageLabelsParam);
    // ++jeffrey TD10365

    // send the message
    submitAuditMessage(TA_Base_Core::PAAgentAudit::PaRecordAdhocRequest, parameters, sessionId);

    FUNCTION_EXIT;
}

void PasHelpers::submitPaRetryStationBroadcastRequestAuditMessage(const std::string& broadcastId,
                                                                  const TA_Base_Bus::IPAAgentCorbaDef::EBroadcastType& broadcastType,
                                                                  const TA_Base_Bus::IPAAgentCorbaDef::ZoneCoverage& zones,
                                                                  const std::string& sessionId)
{
    LOG_SCOPE("submitPaRetryStationBroadcastRequestAuditMessage");
    FUNCTION_ENTRY("submitPaRetryStationBroadcastRequestAuditMessage");

    // build the parameters
    DescriptionParameters parameters;

    std::string typeStr;

    switch (broadcastType)
    {
        case TA_Base_Bus::IPAAgentCorbaDef::StationMusic:
            typeStr = "Music";
            break;

        case TA_Base_Bus::IPAAgentCorbaDef::StationLive:
            typeStr = "Live";
            break;

        case TA_Base_Bus::IPAAgentCorbaDef::StationDva:
            typeStr = "DVA";
            break;

        case TA_Base_Bus::IPAAgentCorbaDef::StationRecording:
            typeStr = "Recording";
            break;
    }

    NameValuePair broadcastTypeParam("BroadcastType", typeStr);
    parameters.push_back(&broadcastTypeParam);

    NameValuePair broadcastIdParam("BroadcastId", broadcastId);
    parameters.push_back(&broadcastIdParam);

    NameValuePair zoneParam("ListOfLocationsAndZones", formatZones(zones));
    parameters.push_back(&zoneParam);

    // send the message
    submitAuditMessage(TA_Base_Core::PAAgentAudit::PaRetryStationBroadcastRequest, parameters, sessionId);

    FUNCTION_EXIT;
}

void PasHelpers::submitPaBroadcastEventTriggeredStationDvaRequestAuditMessage(const std::string& broadcastId,
                                                                              unsigned long messageKey,
                                                                              const std::string& messageVersionNumber,
                                                                              bool hasChime,
                                                                              bool isSynchronisedWithTis,
                                                                              unsigned long durationInSecs,
                                                                              unsigned short periodInSecs,
                                                                              const TA_Base_Bus::IPAAgentCorbaDef::ZoneCoverage& zones,
                                                                              const std::string& sessionId)
{
    LOG_SCOPE("submitPaBroadcastEventTriggeredStationDvaRequestAuditMessage");
    FUNCTION_ENTRY("submitPaBroadcastEventTriggeredStationDvaRequestAuditMessage");

    // build the parameters
    DescriptionParameters parameters;

    NameValuePair broadcastIdParam("BroadcastId", broadcastId);
    parameters.push_back(&broadcastIdParam);

    std::ostringstream messageId;

    try
    {
        messageId << CachedMaps::getInstance()->getStationDvaMessageIdFromKey(messageKey);
    }
    catch (TA_Base_Core::CachedMappingNotFoundException& e)
    {
        LOG_EXCEPTION_CATCH(SourceInfo, "TA_Base_Core::CachedMappingNotFoundException", e.what());

        FUNCTION_EXIT;
        TA_THROW(TA_Base_Core::BadParameterException("The specified message does not exist in the database"));
    }

    NameValuePair messageIdParam("MessageId", messageId.str());
    parameters.push_back(&messageIdParam);

    NameValuePair versionParam("Version", messageVersionNumber);
    parameters.push_back(&versionParam);

    std::ostringstream chime;
    chime << hasChime;
    NameValuePair chimeParam("HasChime", chime.str());
    parameters.push_back(&chimeParam);

    std::ostringstream temp;
    temp << isSynchronisedWithTis;
    NameValuePair syncParam("IsTisSynchronised", temp.str());
    parameters.push_back(&syncParam);

    temp.str("");
    temp << durationInSecs;
    NameValuePair durationParam("Duration", temp.str());
    parameters.push_back(&durationParam);

    temp.str("");
    temp << periodInSecs;
    NameValuePair periodParam("Period", temp.str());
    parameters.push_back(&periodParam);

    NameValuePair zoneParam("ListOfLocationsAndZones", formatZones(zones));
    parameters.push_back(&zoneParam);

    // jeffrey++ TD10365, TD10393
    std::ostringstream messageLabel;

    try
    {
        StationDvaMessageRecord record = CachedMaps::getInstance()->getStationDvaMessageRecordFromKey(messageKey);
        messageLabel << checkLabel(record.m_label);
    }
    catch (TA_Base_Core::CachedMappingNotFoundException& e)
    {
        LOG_EXCEPTION_CATCH(SourceInfo, "TA_Base_Core::CachedMappingNotFoundException", e.what());

        FUNCTION_EXIT;
        TA_THROW(TA_Base_Core::BadParameterException("The specified message does not exist in the database"));
    }

    NameValuePair messageLabelsParam("MsgLabel", messageLabel.str());
    parameters.push_back(&messageLabelsParam);
    // ++jeffrey TD10365

    // send the message
    submitAuditMessage(TA_Base_Core::PAAgentAudit::PaBroadcastEventTriggeredStationDvaRequest, parameters, sessionId);

    FUNCTION_EXIT;
}

void PasHelpers::submitPaBroadcastEventTriggeredStationDvaForStationRequestAuditMessage(const std::string& broadcastId,
                                                                                        unsigned long messageKey,
                                                                                        const std::string& messageVersionNumber,
                                                                                        bool hasChime,
                                                                                        bool isSynchronisedWithTis,
                                                                                        unsigned long durationInSecs,
                                                                                        unsigned short periodInSecs,
                                                                                        unsigned long stationKey,
                                                                                        const std::string& sessionId)
{
    LOG_SCOPE("submitPaBroadcastEventTriggeredStationDvaForStationRequestAuditMessage");
    FUNCTION_ENTRY("submitPaBroadcastEventTriggeredStationDvaForStationRequestAuditMessage");

    // build the parameters
    DescriptionParameters parameters;

    NameValuePair broadcastIdParam("BroadcastId", broadcastId);
    parameters.push_back(&broadcastIdParam);

    std::ostringstream messageId;

    try
    {
        messageId << CachedMaps::getInstance()->getStationDvaMessageIdFromKey(messageKey);
    }
    catch (TA_Base_Core::CachedMappingNotFoundException& e)
    {
        LOG_EXCEPTION_CATCH(SourceInfo, "TA_Base_Core::CachedMappingNotFoundException", e.what());

        FUNCTION_EXIT;
        TA_THROW(TA_Base_Core::BadParameterException("The specified message does not exist in the database"));
    }

    NameValuePair messageIdParam("MessageId", messageId.str());
    parameters.push_back(&messageIdParam);

    NameValuePair versionParam("Version", messageVersionNumber);
    parameters.push_back(&versionParam);

    std::ostringstream temp;
    temp << hasChime;
    NameValuePair chimeParam("HasChime", temp.str());
    parameters.push_back(&chimeParam);

    temp.str("");
    temp << isSynchronisedWithTis;
    NameValuePair syncParam("IsTisSynchronised", temp.str());
    parameters.push_back(&syncParam);

    temp.str("");
    temp << durationInSecs;
    NameValuePair durationParam("Duration", temp.str());
    parameters.push_back(&durationParam);

    temp.str("");
    temp << periodInSecs;
    NameValuePair periodParam("Period", temp.str());
    parameters.push_back(&periodParam);

    std::string stationName;

    try
    {
        stationName = CachedMaps::getInstance()->getLocationNameFromKey(stationKey);
    }
    catch (TA_Base_Core::CachedMappingNotFoundException& e)
    {
        LOG_EXCEPTION_CATCH(SourceInfo, "TA_Base_Core::CachedMappingNotFoundException", e.what());

        FUNCTION_EXIT;
        TA_THROW(TA_Base_Core::BadParameterException("The specified station does not exist in the database"));
    }

    NameValuePair stationParam("Station", stationName);
    parameters.push_back(&stationParam);

    // jeffrey++ TD10365, TD10393
    std::ostringstream messageLabel;

    try
    {
        StationDvaMessageRecord record = CachedMaps::getInstance()->getStationDvaMessageRecordFromKey(messageKey);
        messageLabel << checkLabel(record.m_label);
    }
    catch (TA_Base_Core::CachedMappingNotFoundException& e)
    {
        LOG_EXCEPTION_CATCH(SourceInfo, "TA_Base_Core::CachedMappingNotFoundException", e.what());

        FUNCTION_EXIT;
        TA_THROW(TA_Base_Core::BadParameterException("The specified message does not exist in the database"));
    }

    NameValuePair messageLabelsParam("MsgLabel", messageLabel.str());
    parameters.push_back(&messageLabelsParam);
    // ++jeffrey TD10365

    // send the message
    submitAuditMessage(TA_Base_Core::PAAgentAudit::PaBroadcastEventTriggeredStationDvaForStationRequest, parameters, sessionId);

    FUNCTION_EXIT;
}

void PasHelpers::submitPaBroadcastEventTriggeredStationDvaForAreaRequestAuditMessage(const std::string& broadcastId,
                                                                                     unsigned long messageKey,
                                                                                     const std::string& messageVersionNumber,
                                                                                     bool hasChime,
                                                                                     bool isSynchronisedWithTis,
                                                                                     unsigned long durationInSecs,
                                                                                     unsigned short periodInSecs,
                                                                                     const std::string& zoneArea,
                                                                                     const std::string& sessionId)
{
    LOG_SCOPE("submitPaBroadcastEventTriggeredStationDvaForAreaRequestAuditMessage");
    FUNCTION_ENTRY("submitPaBroadcastEventTriggeredStationDvaForAreaRequestAuditMessage");

    // build the parameters
    DescriptionParameters parameters;

    NameValuePair broadcastIdParam("BroadcastId", broadcastId);
    parameters.push_back(&broadcastIdParam);

    std::ostringstream messageId;

    try
    {
        messageId << CachedMaps::getInstance()->getStationDvaMessageIdFromKey(messageKey);
    }
    catch (TA_Base_Core::CachedMappingNotFoundException& e)
    {
        LOG_EXCEPTION_CATCH(SourceInfo, "TA_Base_Core::CachedMappingNotFoundException", e.what());

        FUNCTION_EXIT;
        TA_THROW(TA_Base_Core::BadParameterException("The specified message does not exist in the database"));
    }

    NameValuePair messageIdParam("MessageId", messageId.str());
    parameters.push_back(&messageIdParam);

    NameValuePair versionParam("Version", messageVersionNumber);
    parameters.push_back(&versionParam);

    std::ostringstream temp;
    temp << hasChime;
    NameValuePair chimeParam("HasChime", temp.str());
    parameters.push_back(&chimeParam);

    temp.str("");
    temp << isSynchronisedWithTis;
    NameValuePair syncParam("IsTisSynchronised", temp.str());
    parameters.push_back(&syncParam);

    temp.str("");
    temp << durationInSecs;
    NameValuePair durationParam("Duration", temp.str());
    parameters.push_back(&durationParam);

    temp << periodInSecs;
    NameValuePair periodParam("Period", temp.str());
    parameters.push_back(&periodParam);

    NameValuePair areaParam("Area", zoneArea);
    parameters.push_back(&areaParam);

    // jeffrey++ TD10365, TD10393
    std::ostringstream messageLabel;

    try
    {
        StationDvaMessageRecord record = CachedMaps::getInstance()->getStationDvaMessageRecordFromKey(messageKey);
        messageLabel << checkLabel(record.m_label);
    }
    catch (TA_Base_Core::CachedMappingNotFoundException& e)
    {
        LOG_EXCEPTION_CATCH(SourceInfo, "TA_Base_Core::CachedMappingNotFoundException", e.what());

        FUNCTION_EXIT;
        TA_THROW(TA_Base_Core::BadParameterException("The specified message does not exist in the database"));
    }

    NameValuePair messageLabelsParam("MsgLabel", messageLabel.str());
    parameters.push_back(&messageLabelsParam);
    // ++jeffrey TD10365

    // send the message
    submitAuditMessage(TA_Base_Core::PAAgentAudit::PaBroadcastEventTriggeredStationDvaForAreaRequest, parameters, sessionId);

    FUNCTION_EXIT;
}

void PasHelpers::submitPaBroadcastEventTriggeredStationDvaForGroupRequestAuditMessage(const std::string& broadcastId,
                                                                                      unsigned long messageKey,
                                                                                      const std::string& messageVersionNumber,
                                                                                      bool hasChime,
                                                                                      bool isSynchronisedWithTis,
                                                                                      unsigned long durationInSecs,
                                                                                      unsigned short periodInSecs,
                                                                                      const std::string& zoneGroupLabel,
                                                                                      const std::string& sessionId)
{
    LOG_SCOPE("submitPaBroadcastEventTriggeredStationDvaForGroupRequestAuditMessage");
    FUNCTION_ENTRY("submitPaBroadcastEventTriggeredStationDvaForGroupRequestAuditMessage");

    // build the parameters
    DescriptionParameters parameters;

    NameValuePair broadcastIdParam("BroadcastId", broadcastId);
    parameters.push_back(&broadcastIdParam);

    std::ostringstream messageId;

    try
    {
        messageId << CachedMaps::getInstance()->getStationDvaMessageIdFromKey(messageKey);
    }
    catch (TA_Base_Core::CachedMappingNotFoundException& e)
    {
        LOG_EXCEPTION_CATCH(SourceInfo, "TA_Base_Core::CachedMappingNotFoundException", e.what());

        FUNCTION_EXIT;
        TA_THROW(TA_Base_Core::BadParameterException("The specified message does not exist in the database"));
    }

    NameValuePair messageIdParam("MessageId", messageId.str());
    parameters.push_back(&messageIdParam);

    NameValuePair versionParam("Version", messageVersionNumber);
    parameters.push_back(&versionParam);

    std::ostringstream temp;
    temp << hasChime;
    NameValuePair chimeParam("HasChime", temp.str());
    parameters.push_back(&chimeParam);

    temp.str("");
    temp << isSynchronisedWithTis;
    NameValuePair syncParam("IsTisSynchronised", temp.str());
    parameters.push_back(&syncParam);

    temp.str("");
    temp << durationInSecs;
    NameValuePair durationParam("Duration", temp.str());
    parameters.push_back(&durationParam);

    temp << periodInSecs;
    NameValuePair periodParam("Period", temp.str());
    parameters.push_back(&periodParam);

    NameValuePair groupParam("Group", zoneGroupLabel);
    parameters.push_back(&groupParam);

    // jeffrey++ TD10365, TD10393
    std::ostringstream messageLabel;

    try
    {
        StationDvaMessageRecord record = CachedMaps::getInstance()->getStationDvaMessageRecordFromKey(messageKey);
        messageLabel << checkLabel(record.m_label);
    }
    catch (TA_Base_Core::CachedMappingNotFoundException& e)
    {
        LOG_EXCEPTION_CATCH(SourceInfo, "TA_Base_Core::CachedMappingNotFoundException", e.what());

        FUNCTION_EXIT;
        TA_THROW(TA_Base_Core::BadParameterException("The specified message does not exist in the database"));
    }

    NameValuePair messageLabelsParam("MsgLabel", messageLabel.str());
    parameters.push_back(&messageLabelsParam);
    // ++jeffrey TD10365

    // send the message
    submitAuditMessage(TA_Base_Core::PAAgentAudit::PaBroadcastEventTriggeredStationDvaForGroupRequest, parameters, sessionId);

    FUNCTION_EXIT;
}

void PasHelpers::submitPaStationBroadcastResultAuditMessage(const std::string& broadcastId,
                                                            const TA_Base_Bus::IPAAgentCorbaDef::ZoneCoverage& failedZones,
                                                            bool broadcastFailed,
                                                            std::map<int, std::string> failedReasons,//TD10320
                                                            const std::string& sessionId)
{
    LOG_SCOPE("submitPaStationBroadcastResultAuditMessage");
    FUNCTION_ENTRY("submitPaStationBroadcastResultAuditMessage");

    // build the parameters
    DescriptionParameters parameters;
    // change event value to station name, add station name parameter

    std::string locationName = ThisLocation::name();

    NameValuePair stationNameParam("StationName", locationName);
    parameters.push_back(&stationNameParam);
    // broadcastid
    NameValuePair broadcastIdParam("BroadcastId", broadcastId);
    parameters.push_back(&broadcastIdParam);

    std::string failedZoneStr;
    std::string failedReasonStr;//TD10320

    if (failedZones.length() > 0)
    {
        failedZoneStr = formatZones(failedZones);
        failedReasonStr = formatReasons(failedReasons, failedZones);//TD10320
    }
    else
    {
        if (broadcastFailed)
        {
            failedZoneStr = "All";
        }
        else
        {
            failedZoneStr = "None";
        }
    }

    NameValuePair zoneParam("ListOfZoneFailuresIfAny", failedZoneStr);
    parameters.push_back(&zoneParam);
    NameValuePair failedReason("ListOfReasonFailuresIfAny", failedReasonStr);//TD10320
    parameters.push_back(&failedReason);//TD10320

    // send the message
    submitAuditMessage(TA_Base_Core::PAAgentAudit::PaStationBroadcastResult, parameters, sessionId);

    FUNCTION_EXIT;
}

void PasHelpers::submitPaTrainBroadcastResultAuditMessage(const std::string& broadcastId,
                                                          const TA_Base_Bus::IPAAgentCorbaDef::TrainCoverage& failedTrains,
                                                          bool broadcastFailed,
                                                          const std::string& sessionId)
{
    LOG_SCOPE("submitPaTrainBroadcastResultAuditMessage");
    FUNCTION_ENTRY("submitPaTrainBroadcastResultAuditMessage");

    // build the parameters
    DescriptionParameters parameters;

    //     NameValuePair broadcastIdParam ( "BroadcastId", broadcastId );
    //     parameters.push_back ( &broadcastIdParam );

    //     std::ostringstream failedTrainStr;
    //     if ( failedTrains.length() > 0 )
    //     {
    //         for (int i = 0; i < failedTrains.length(); ++i)
    //      {
    //             failedTrainStr << failedTrains[i];
    //
    //             if (i < (failedTrains.length()-1))
    //             {
    //                 failedTrainStr << ",";
    //             }
    //      }
    //     }
    //     else
    //     {
    //         if (broadcastFailed)
    //         {
    //             failedTrainStr << "All";
    //         }
    //         else
    //         {
    //             failedTrainStr << "None";
    //         }
    //     }
    //
    //     NameValuePair zoneParam ( "ListOfZoneFailuresIfAny", failedTrainStr.str() );
    //     parameters.push_back ( &zoneParam );

    // send the message
    submitAuditMessage(TA_Base_Core::PAAgentAudit::PaStationBroadcastResult,
                       //    submitAuditMessage( TA_Base_Core::PAAgentAudit::PATrainBroadcastTerminatedAudit,
                       parameters,
                       sessionId);

    FUNCTION_EXIT;
}

void PasHelpers::submitPaTerminateBroadcastAuditMessage(const std::string& broadcastId,
                                                        const std::string& sessionId)
{
    LOG_SCOPE("submitPaTerminateBroadcastAuditMessage");
    FUNCTION_ENTRY("submitPaTerminateBroadcastAuditMessage");

    // build the parameters
    DescriptionParameters parameters;

    NameValuePair broadcastIdParam("BroadcastId", broadcastId);
    parameters.push_back(&broadcastIdParam);

    // send the message
    submitAuditMessage(TA_Base_Core::PAAgentAudit::PaTerminateBroadcast, parameters, sessionId);

    FUNCTION_EXIT;
}

void PasHelpers::submitPaRemoveBroadcastAuditMessage(const std::string& broadcastId,
                                                     const std::string& sessionId)
{
    LOG_SCOPE("submitPaRemoveBroadcastAuditMessage");
    FUNCTION_ENTRY("submitPaRemoveBroadcastAuditMessage");

    // build the parameters
    DescriptionParameters parameters;

    NameValuePair broadcastIdParam("BroadcastId", broadcastId);
    parameters.push_back(&broadcastIdParam);

    // send the message
    submitAuditMessage(TA_Base_Core::PAAgentAudit::PaRemoveBroadcast, parameters, sessionId);

    FUNCTION_EXIT;
}

void PasHelpers::submitPaSetAdhocTypeAuditMessage(char oldType, char newType, unsigned long messageKey, const std::string& sessionId)
{
    LOG_SCOPE("submitPaSetAdhocTypeAuditMessage");
    FUNCTION_ENTRY("submitPaSetAdhocTypeAuditMessage");

    // build the parameters
    DescriptionParameters parameters;

    std::ostringstream temp;
    // jeffrey++ TD10393
    //temp << oldType;
    temp << convertTypeString(oldType);
    NameValuePair oldTypeParam("OldType", temp.str());
    parameters.push_back(&oldTypeParam);

    temp.str("");
    //temp << newType;
    temp << convertTypeString(newType);
    // ++jeffrey TD10393
    NameValuePair newTypeParam("NewType", temp.str());
    parameters.push_back(&newTypeParam);

    std::ostringstream messageId;

    try
    {
        messageId << CachedMaps::getInstance()->getStationDvaMessageIdFromKey(messageKey);
    }
    catch (TA_Base_Core::CachedMappingNotFoundException& e)
    {
        LOG_EXCEPTION_CATCH(SourceInfo, "TA_Base_Core::CachedMappingNotFoundException", e.what());

        FUNCTION_EXIT;
        TA_THROW(TA_Base_Core::BadParameterException("The specified message does not exist in the database"));
    }

    NameValuePair messageIdParam("MessageId", messageId.str());
    parameters.push_back(&messageIdParam);

    // send the message
    submitAuditMessage(TA_Base_Core::PAAgentAudit::PaSetAdhocType,
                       parameters,
                       sessionId);

    FUNCTION_EXIT;
}

void PasHelpers::submitPaSetAdhocLabelAuditMessage(const std::string& oldLabel, const std::string& newLabel, unsigned long messageKey, const std::string& sessionId)
{
    LOG_SCOPE("submitPaSetAdhocLabelAuditMessage");
    FUNCTION_ENTRY("submitPaSetAdhocLabelAuditMessage");

    // build the parameters
    DescriptionParameters parameters;

    // jeffrey++ TD10393
    //NameValuePair oldLabelParam ( "OldLabel", oldLabel );
    NameValuePair oldLabelParam("OldLabel", checkLabel(oldLabel));
    parameters.push_back(&oldLabelParam);

    //NameValuePair newLabelParam ( "NewLabel", newLabel );
    NameValuePair newLabelParam("NewLabel", checkLabel(newLabel));
    parameters.push_back(&newLabelParam);
    // ++jeffrey TD10393

    std::ostringstream messageId;

    try
    {
        messageId << CachedMaps::getInstance()->getStationDvaMessageIdFromKey(messageKey);
    }
    catch (TA_Base_Core::CachedMappingNotFoundException& e)
    {
        LOG_EXCEPTION_CATCH(SourceInfo, "TA_Base_Core::CachedMappingNotFoundException", e.what());

        FUNCTION_EXIT;
        TA_THROW(TA_Base_Core::BadParameterException("The specified message does not exist in the database"));
    }

    NameValuePair messageIdParam("MessageId", messageId.str());
    parameters.push_back(&messageIdParam);

    // send the message
    submitAuditMessage(TA_Base_Core::PAAgentAudit::PaSetAdhocLabel, parameters, sessionId);

    FUNCTION_EXIT;
}

void PasHelpers::submitPaChangeBroadcastIdAuditMessage(const std::string& oldBroadcastId, const std::string& newBroadcastId, const std::string& sessionId)
{
    LOG_SCOPE("submitPaChangeBroadcastIdAuditMessage");
    FUNCTION_ENTRY("submitPaChangeBroadcastIdAuditMessage");

    // build the parameters
    DescriptionParameters parameters;

    NameValuePair oldIdParam("OldId", oldBroadcastId);
    parameters.push_back(&oldIdParam);

    NameValuePair newIdParam("NewId", newBroadcastId);
    parameters.push_back(&newIdParam);

    // send the message
    submitAuditMessage(TA_Base_Core::PAAgentAudit::PaChangeBroadcastId, parameters, sessionId);

    FUNCTION_EXIT;
}

void PasHelpers::submitPaAgentBroadcastStateAuditMessage(const std::string& broadcastId, const TA_Base_Bus::IPAAgentCorbaDef::EBroadcastState& broadcastState)
{
    LOG_SCOPE(boost::format("PasHelpers::submitPaAgentBroadcastStateAuditMessage[%s]") % nvps(broadcastId, broadcastState));
    FUNCTION_ENTRY("submitPaAgentBroadcastStateAuditMessage");

    // build the parameters
    DescriptionParameters parameters;
    auto locationName = ThisLocation::name();
    NameValuePair stationNameParam("StationName", locationName);
    parameters.push_back(&stationNameParam);
    NameValuePair broadcastIdParam("BroadcastId", broadcastId);
    parameters.push_back(&broadcastIdParam);

    std::string stateStr = "Unknown State";

    switch (broadcastState)
    {
        case TA_Base_Bus::IPAAgentCorbaDef::Creation:
            stateStr = "Creation";
            break;

        case TA_Base_Bus::IPAAgentCorbaDef::Execution:
            stateStr = "Execution";
            break;

        case TA_Base_Bus::IPAAgentCorbaDef::Runtime:
            stateStr = "Runtime";
            break;

        case TA_Base_Bus::IPAAgentCorbaDef::Termination:
            stateStr = "Termination";
            break;

        case TA_Base_Bus::IPAAgentCorbaDef::PendingRemoval:
            stateStr = "PendingRemoval";
            break;
    }

    NameValuePair broadcastStateParam("BroadcastState", stateStr);
    parameters.push_back(&broadcastStateParam);

    // send the message
    submitAuditMessage(TA_Base_Core::PAAgentAudit::PaAgentBroadcastState, parameters);

    FUNCTION_EXIT;
}

void PasHelpers::submitPaAgentBroadcastStateFailureAuditMessage(const std::string& broadcastId, const TA_Base_Bus::IPAAgentCorbaDef::EGlobalState& globalState)
{
    LOG_SCOPE(boost::format("PasHelpers::submitPaAgentBroadcastStateFailureAuditMessage[%s]") % nvps(broadcastId, globalState));
    FUNCTION_ENTRY("submitPaAgentBroadcastStateFailureAuditMessage");

    // build the parameters
    DescriptionParameters parameters;

    NameValuePair broadcastIdParam("BroadcastId", broadcastId);
    parameters.push_back(&broadcastIdParam);

    std::string stateStr = "Unknown reason";

    switch (globalState)
    {
        case TA_Base_Bus::IPAAgentCorbaDef::GLOBAL_STATE_NO_FAILURE:
            return;   // not a failure

        case TA_Base_Bus::IPAAgentCorbaDef::GLOBAL_STATE_NO_FREE_ANNOUNCE_ID:
            stateStr = "No free announce ID";
            break;

        case TA_Base_Bus::IPAAgentCorbaDef::GLOBAL_STATE_NO_FREE_MESSAGE_SEQ_ID:
            stateStr = "No free message sequence ID";
            break;

        case TA_Base_Bus::IPAAgentCorbaDef::GLOBAL_STATE_NO_FREE_DVA_CHANNEL:
            stateStr = "No free DVA channel";
            break;

        case TA_Base_Bus::IPAAgentCorbaDef::GLOBAL_STATE_RECORDING_SOURCE_BUSY:
            stateStr = "Recording source busy";
            break;

        case TA_Base_Bus::IPAAgentCorbaDef::GLOBAL_STATE_MESSAGE_BUSY_RECORDING:
            stateStr = "Message busy recording";
            break;

        case TA_Base_Bus::IPAAgentCorbaDef::GLOBAL_STATE_MESSAGE_BUSY_BROADCASTING:
            stateStr = "Message busy broadcasting";
            break;

        case TA_Base_Bus::IPAAgentCorbaDef::GLOBAL_STATE_COMMS_FAILURE_TRAIN_AGENT:
            stateStr = "Train Agent comms failure";
            break;

        case TA_Base_Bus::IPAAgentCorbaDef::GLOBAL_STATE_COMMS_FAILURE_TIS_AGENT:
            stateStr = "TIS Agent comms failure";
            break;

        case TA_Base_Bus::IPAAgentCorbaDef::GLOBAL_STATE_COMMS_FAILURE_AUTHENTICATION_AGENT:
            stateStr = "Authentication Agent comms failure";
            break;

        case TA_Base_Bus::IPAAgentCorbaDef::GLOBAL_STATE_HARDWARE_FAILURE_TRAIN_AGENT:
            stateStr = "Train Agent hardware failure";
            break;

        case TA_Base_Bus::IPAAgentCorbaDef::GLOBAL_STATE_HARDWARE_FAILURE_TIS_AGENT:
            stateStr = "TIS Agent hardware failure";
            break;

        case TA_Base_Bus::IPAAgentCorbaDef::GLOBAL_STATE_HARDWARE_FAILURE_PA_AGENT:
            stateStr = "PA Agent hardware failure";
            break;

        case TA_Base_Bus::IPAAgentCorbaDef::GLOBAL_STATE_NO_PERMISSION_FAILURE_TRAIN_AGENT:
            stateStr = "No permission from Train Agent";
            break;

        case TA_Base_Bus::IPAAgentCorbaDef::GLOBAL_STATE_NO_PERMISSION_FAILURE_TIS_AGENT:
            stateStr = "No permission from TIS Agent";
            break;

        case TA_Base_Bus::IPAAgentCorbaDef::GLOBAL_STATE_INVALID_PROTOCOL_FAILURE_TRAIN_AGENT:
            stateStr = "Invalid protocol from Train Agent";
            break;

        case TA_Base_Bus::IPAAgentCorbaDef::GLOBAL_STATE_INVALID_PROTOCOL_FAILURE_TIS_AGENT:
            stateStr = "Invalid protocol from TIS Agent";
            break;

        case TA_Base_Bus::IPAAgentCorbaDef::GLOBAL_STATE_INVALID_PROTOCOL_FAILURE_PA_AGENT:
            stateStr = "Invalid protocol from PA Agent";
            break;

        case TA_Base_Bus::IPAAgentCorbaDef::GLOBAL_STATE_PUBLIC_DVA_SET_VERSION_CONFLICT:
            stateStr = "DVA set A version conflict";
            break;

        case TA_Base_Bus::IPAAgentCorbaDef::GLOBAL_STATE_RADIO_CALL_RESET:
            stateStr = "Audio Call Dropped";
            break;
    }

    NameValuePair broadcastStateFailureParam("BroadcastStateFailure", stateStr);
    parameters.push_back(&broadcastStateFailureParam);

    // send the message
    submitAuditMessage(TA_Base_Core::PAAgentAudit::PaAgentBroadcastStateFailure, parameters);

    FUNCTION_EXIT;
}

void PasHelpers::submitPaAgentDVAMessageUpdateAuditMessage(char oldType,
                                                           char newType,
                                                           const std::string& oldLabel,
                                                           const std::string& newLabel,
                                                           unsigned long messageKey)
{
    LOG_SCOPE("submitPaAgentDVAMessageUpdateAuditMessage");
    FUNCTION_ENTRY("submitPaAgentDVAMessageUpdateAuditMessage");
    LOG_DEBUG("submitPaAgentDVAMessageUpdateAuditMessage(): %s", nvps(messageKey, oldType, newType, oldLabel, newLabel));

    // build the parameters
    DescriptionParameters parameters;

    std::ostringstream messageId;

    try
    {
        messageId << CachedMaps::getInstance()->getStationDvaMessageIdFromKey(messageKey);
    }
    catch (TA_Base_Core::CachedMappingNotFoundException& e)
    {
        LOG_EXCEPTION_CATCH(SourceInfo, "TA_Base_Core::CachedMappingNotFoundException", e.what());

        FUNCTION_EXIT;
        TA_THROW(TA_Base_Core::BadParameterException("The specified message does not exist in the database"));
    }

    NameValuePair messageIdParam("MessageId", messageId.str());
    parameters.push_back(&messageIdParam);

    std::ostringstream temp;
    temp << oldType;
    NameValuePair oldTypeParam("OldType", temp.str());
    parameters.push_back(&oldTypeParam);

    temp.str("");
    temp << newType;
    NameValuePair newTypeParam("NewType", temp.str());
    parameters.push_back(&newTypeParam);

    // jeffrey++ TD10393
    //NameValuePair oldLabelParam ( "OldLabel", oldLabel );
    NameValuePair oldLabelParam("OldLabel", checkLabel(oldLabel));
    parameters.push_back(&oldLabelParam);

    //NameValuePair newLabelParam ( "NewLabel", newLabel );
    NameValuePair newLabelParam("NewLabel", checkLabel(newLabel));
    parameters.push_back(&newLabelParam);

    // send the message
    submitAuditMessage(TA_Base_Core::PAAgentAudit::PaAgentDVAMessageUpdate,
                       parameters);

    FUNCTION_EXIT;
}

void PasHelpers::submitTrainPAPrerecordedAuditMessage(const std::string& broadcastId,
                                                      const TA_Base_Bus::IPAAgentCorbaDef::TrainCoverage& trains,
                                                      unsigned long messageKey,
                                                      unsigned short libraryVersion,
                                                      const TA_Base_Bus::ITrainPaCorba::MessageSchedule& announcementPeriod,
                                                      const std::string& sessionId)
{
    LOG_SCOPE("submitTrainPAPrerecordedAuditMessage");
    FUNCTION_ENTRY("submitTrainPAPrerecordedAuditMessage");

    // build the parameters
    DescriptionParameters parameters;

    NameValuePair broadcastIdParam("ID", broadcastId);
    parameters.push_back(&broadcastIdParam);

    NameValuePair trainParam("Train", "");
    parameters.push_back(&trainParam);

    std::ostringstream temp;
    temp << messageKey;
    NameValuePair messageKeyParam("MessageKey", temp.str());
    parameters.push_back(&messageKeyParam);

    temp.str("");
    temp << libraryVersion;
    NameValuePair libraryParam("Library", temp.str());
    parameters.push_back(&libraryParam);

    NameValuePair intervalParam("Interval", formatInterval(announcementPeriod));
    parameters.push_back(&intervalParam);

    // for each train
    for (int i = 0; i < trains.length(); i++)
    {
        // set the train Id Param
        std::ostringstream tempStr;
        tempStr << trains[i];
        trainParam.value = tempStr.str();

        // send the message
        submitAuditMessage(TA_Base_Core::PAAgentAudit::TrainPAPrerecorded, parameters, sessionId);
    }

    FUNCTION_EXIT;
}

void PasHelpers::submitTrainPACyclicAuditMessage(const std::string& broadcastId,
                                                 const TA_Base_Bus::IPAAgentCorbaDef::TrainCoverage& trains,
                                                 unsigned long messageKey,
                                                 unsigned short libraryVersion,
                                                 const TA_Base_Bus::ITrainPaCorba::MessageSchedule& announcementPeriod,
                                                 const std::string& sessionId)
{
    LOG_SCOPE("submitTrainPACyclicAuditMessage");
    FUNCTION_ENTRY("submitTrainPACyclicAuditMessage");

    // build the parameters
    DescriptionParameters parameters;

    NameValuePair broadcastIdParam("ID", broadcastId);
    parameters.push_back(&broadcastIdParam);

    NameValuePair trainParam("Train", "");
    parameters.push_back(&trainParam);

    std::ostringstream temp;
    temp << messageKey;
    NameValuePair messageKeyParam("MessageKey", temp.str());
    parameters.push_back(&messageKeyParam);

    temp.str("");
    temp << libraryVersion;
    NameValuePair libraryParam("Library", temp.str());
    parameters.push_back(&libraryParam);

    NameValuePair intervalParam("Interval", formatInterval(announcementPeriod));
    parameters.push_back(&intervalParam);

    // for each train
    for (int i = 0; i < trains.length(); i++)
    {
        // set the train Id Param
        std::ostringstream tempStr;
        tempStr << trains[i];
        trainParam.value = tempStr.str();

        // send the message
        submitAuditMessage(TA_Base_Core::PAAgentAudit::TrainPACyclic, parameters, sessionId);
    }

    FUNCTION_EXIT;
}

void PasHelpers::submitTrainPASetupLiveAnnouncementAuditMessage(const std::string& broadcastId,
                                                                const TA_Base_Bus::IPAAgentCorbaDef::TrainCoverage& trains,
                                                                const std::string& sessionId)
{
    LOG_SCOPE("submitTrainPASetupLiveAnnouncementAuditMessage");
    FUNCTION_ENTRY("submitTrainPASetupLiveAnnouncementAuditMessage");

    // build the parameters
    DescriptionParameters parameters;

    NameValuePair broadcastIdParam("ID", broadcastId);
    parameters.push_back(&broadcastIdParam);

    NameValuePair trainParam("Train", "");
    parameters.push_back(&trainParam);

    // for each train
    for (int i = 0; i < trains.length(); i++)
    {
        // set the train Id Param
        std::ostringstream tempStr;
        tempStr << trains[i];
        trainParam.value = tempStr.str();

        // send the message
        submitAuditMessage(TA_Base_Core::PAAgentAudit::TrainPASetupLiveAnnouncement, parameters, sessionId);
    }

    FUNCTION_EXIT;
}

void PasHelpers::submitTrainPAContinueLiveAnnouncementAuditMessage(const std::string& broadcastId, unsigned long train)
{
    LOG_SCOPE("submitTrainPAContinueLiveAnnouncementAuditMessage");
    FUNCTION_ENTRY("submitTrainPAContinueLiveAnnouncementAuditMessage");

    // build the parameters
    DescriptionParameters parameters;

    NameValuePair broadcastIdParam("ID", broadcastId);
    parameters.push_back(&broadcastIdParam);

    std::ostringstream tempStr;
    tempStr << train;
    NameValuePair trainParam("Train", tempStr.str());
    parameters.push_back(&trainParam);

    // send the message
    submitAuditMessage(TA_Base_Core::PAAgentAudit::TrainPAContinueLiveAnnouncement, parameters);

    FUNCTION_EXIT;
}

void PasHelpers::submitTrainPAEndLiveAnnouncementAuditMessage(const std::string& broadcastId,
                                                              const TA_Base_Bus::IPAAgentCorbaDef::TrainCoverage& trains)
{
    LOG_SCOPE("submitTrainPAEndLiveAnnouncementAuditMessage");
    FUNCTION_ENTRY("submitTrainPAEndLiveAnnouncementAuditMessage");

    // build the parameters
    DescriptionParameters parameters;

    NameValuePair broadcastIdParam("ID", broadcastId);
    parameters.push_back(&broadcastIdParam);

    NameValuePair trainParam("Train", "");
    parameters.push_back(&trainParam);

    // for each train
    for (int i = 0; i < trains.length(); i++)
    {
        // set the train Id Param
        std::ostringstream tempStr;
        tempStr << trains[i];
        trainParam.value = tempStr.str();

        // send the message
        submitAuditMessage(TA_Base_Core::PAAgentAudit::TrainPAEndLiveAnnouncement, parameters);
    }

    FUNCTION_EXIT;
}

void PasHelpers::submitPaZoneBroadcastFailureAuditMessage(unsigned long failedZone, const std::string& reason, const std::string& sessionId)
{
    LOG_SCOPE("submitPaZoneBroadcastFailureAuditMessage");
    FUNCTION_ENTRY("submitPaZoneBroadcastFailureAuditMessage");

    // build the parameters
    DescriptionParameters parameters;

    // get the zone Id from the key
    std::ostringstream tempZone;
    std::ostringstream tempLocation;

    try
    {
        tempZone << CachedMaps::getInstance()->getPaZoneIdFromKey(failedZone);
        tempLocation << CachedMaps::getInstance()->getLocationNameFromKey(CachedMaps::getInstance()->getPaZoneLocationKeyFromKey(failedZone));
    }
    catch (TA_Base_Core::CachedMappingNotFoundException& e)
    {
        LOG_EXCEPTION_CATCH(SourceInfo, "TA_Base_Core::CachedMappingNotFoundException", e.what());

        // what if the broadcast failed due to an invalid zone?
        // just add the key as the zone Id so the audit message is generated as this
        // might be the reason the broadcast failed.
        if (tempZone.str().size() < 1)
        {
            tempZone << failedZone;
        }

        // this location
        try
        {
            // try to look up this location name
            tempLocation << CachedMaps::getInstance()->getLocationNameFromKey(CachedConfig::getInstance()->getAgentLocationKey());
        }
        catch (...)
        {
            // if it fails just use the key
            tempLocation << CachedConfig::getInstance()->getAgentLocationKey();
        }
    }

    NameValuePair zoneIdParam("ZoneId", tempZone.str());
    parameters.push_back(&zoneIdParam);

    NameValuePair locationParam("Location", tempLocation.str());
    parameters.push_back(&locationParam);

    NameValuePair reasonParam("Reason", reason);
    parameters.push_back(&reasonParam);

    // send the message
    submitAuditMessage(TA_Base_Core::PAAgentAudit::PaZoneBroadcastFailure, parameters, sessionId);

    FUNCTION_EXIT;
}

void PasHelpers::submitPaTrainBroadcastFailureAuditMessage(unsigned long trainId,
                                                           unsigned long location,
                                                           const std::string& reason,
                                                           const std::string& sessionId)
{
    LOG_SCOPE("submitPaTrainBroadcastFailureAuditMessage");
    FUNCTION_ENTRY("submitPaTrainBroadcastFailureAuditMessage");

    // build the parameters
    DescriptionParameters parameters;

    std::ostringstream temp;
    temp << trainId;
    NameValuePair trainParam("TrainId", temp.str());
    parameters.push_back(&trainParam);

    temp.str("");
    temp << location;
    NameValuePair locationParam("Location", temp.str());
    parameters.push_back(&locationParam);

    NameValuePair reasonParam("Reason", reason);
    parameters.push_back(&reasonParam);

    // send the message
    submitAuditMessage(TA_Base_Core::PAAgentAudit::PaTrainBroadcastFailure, parameters, sessionId);

    FUNCTION_EXIT;
}

void PasHelpers::submitPaTrainBroadcastFailureAuditMessages(const TA_Base_Bus::IPAAgentCorbaDef::TrainCoverage& trains,
                                                            unsigned long location,
                                                            const std::string& reason,
                                                            const std::string& sessionId)
{
    LOG_SCOPE("submitPaTrainBroadcastFailureAuditMessages");
    FUNCTION_ENTRY("submitPaTrainBroadcastFailureAuditMessages");

    for (int i = 0; i < trains.length(); i++)
    {
        submitPaTrainBroadcastFailureAuditMessage(trains[i],
                                                  location,
                                                  reason,
                                                  sessionId);
    }

    FUNCTION_EXIT;
}

void PasHelpers::submitPaAgentDependencyFailureAuditMessage(const std::string& component,
                                                            const std::string& impact)
{
    LOG_SCOPE("submitPaAgentDependencyFailureAuditMessage");
    FUNCTION_ENTRY("submitPaAgentDependencyFailureAuditMessage");

    // build the parameters
    DescriptionParameters parameters;

    NameValuePair componentParam("Component", component);
    parameters.push_back(&componentParam);

    NameValuePair impactParam("Impact", impact);
    parameters.push_back(&impactParam);

    // send the message
    submitAuditMessage(TA_Base_Core::PAAgentAudit::PaAgentDependencyFailure, parameters);

    FUNCTION_EXIT;
}

void PasHelpers::submitPAAgentOnlineUpdateAuditMessage(const std::string& parameter)
{
    LOG_SCOPE("submitPAAgentOnlineUpdateAuditMessage");
    FUNCTION_ENTRY("submitPAAgentOnlineUpdateAuditMessage");

    // build the parameters
    DescriptionParameters parameters;

    NameValuePair parameterParam("Parameter", parameter);
    parameters.push_back(&parameterParam);

    // send the message
    submitAuditMessage(TA_Base_Core::PAAgentAudit::PAAgentOnlineUpdate, parameters);

    FUNCTION_EXIT;
}

// TD 15128
void PasHelpers::submitPaPrioritySchemeUpdateAuditMessage()
{
    LOG_SCOPE("submitPaPrioritySchemeUpdateAuditMessage");
    FUNCTION_ENTRY("submitPaPrioritySchemeUpdateAuditMessage");

    if (CachedConfig::getInstance()->getAgentKey() == 0)
    {
        // The agent has not been initialised completely, we can only log this error
        // below

        FUNCTION_EXIT;
        throw;
    }

    // construct the DescriptionParameters list
    DescriptionParameters parameters;

    std::string locationName = CachedMaps::getInstance()->getLocationNameFromKey(CachedConfig::getInstance()->getAgentLocationKey());

    NameValuePair pair1("LocationName", locationName);
    parameters.push_back(&pair1);

    // send the message
    submitAuditMessage(TA_Base_Core::PAAgentAudit::PaPrioritySchemeUpdate, parameters);

    FUNCTION_EXIT;
}

// TD 15128

// ExceptionChecked
// Threadsafe between:
// - external corba request threads
// - class child threads
void PasHelpers::submitAuditMessage(const MessageType& messageType,
                                    const DescriptionParameters& parameters,
                                    const std::string& sessionId /* = "" */)
{
    LOG_SCOPE("submitAuditMessage");
    FUNCTION_ENTRY("submitAuditMessage");
    TA_ASSERT(m_auditSender, "m_auditSender is NULL");

    bool submitted = false;
    std::string reason = "";

    // If the agent has not been initialised, we can only log this message
    if (CachedConfig::getInstance()->getAgentKey() != 0)
    {
        try
        {
            m_auditSender->sendAuditMessage(messageType,                                 // Message Type
                                            //CachedConfig::getInstance()->getAgentKey(),  // Entity key
                                            //TD17452
                                            CachedConfig::getInstance()->getMonitorProcessKey(),
                                            parameters,                       // Description
                                            "",                                          // Additional details
                                            sessionId,                                   // SessionID if applicable
                                            "",                                          // AlarmID if an alarm associated with event
                                            "",                                          // IncidentKey if incident associated with event
                                            "");                                         // EventID of a parent event, used to link events

            submitted = true;
        } // try block
        catch (TA_Base_Core::TransactiveException& ex)
        {
            reason = ex.what();
        }
        catch (...)
        {
            reason = "Unknown reason.";
        }
    }
    else
    {
        // log the fact that the message cant be sent
        // also log the details of the message
        reason = "Agent not initialised.";
    }

    // failed to submit - make sure all the details are logged
    if (!submitted)
    {
        std::ostringstream msg;
        msg << "Failed to submit audit message " << messageType.getTypeKey() << ". " << reason << std::endl
            << "Parameters are: ";

        for (DescriptionParameters::const_iterator iter = parameters.begin(); iter != parameters.end(); iter++)
        {
            msg << (*iter)->name << "=" << (*iter)->value << ", ";
        }

        msg << "SessionId=" << sessionId;

        LOG_ERROR(msg);
    }

    FUNCTION_EXIT;
}

std::string PasHelpers::formatTime(time_t time)
{
    FUNCTION_ENTRY("formatTime");

    // song toan++
    // TD 10320

    // Many places set time to 0 where the time is unused or unavailable.
    // However, this function will blindly return "01-Jan-1970 08:00:00" in
    // these cases. In order to avoid misunderstanding we don't print the
    // timestamp if the time is 0
    std::ostringstream timeStr;

    if (time == 0)
    {
        timeStr << "N/A";

        FUNCTION_EXIT;
        return timeStr.str();
    }

    // song toan++
    // TD 10320

    struct tm timeStruct;
    ACE_OS::localtime_r(&time, &timeStruct);

    // DD-Mon-YYYY HH:MM:SS
    timeStr << std::setw(2) << std::setfill('0') << timeStruct.tm_mday << "-"; // DD-

    // Mon-
    switch (timeStruct.tm_mon)
    {
        case 0:
            timeStr << "Jan-";
            break;

        case 1:
            timeStr << "Feb-";
            break;

        case 2:
            timeStr << "Mar-";
            break;

        case 3:
            timeStr << "Apr-";
            break;

        case 4:
            timeStr << "May-";
            break;

        case 5:
            timeStr << "Jun-";
            break;

        case 6:
            timeStr << "Jul-";
            break;

        case 7:
            timeStr << "Aug-";
            break;

        case 8:
            timeStr << "Sep-";
            break;

        case 9:
            timeStr << "Oct-";
            break;

        case 10:
            timeStr << "Nov-";
            break;

        case 11:
            timeStr << "Dec-";
            break;
    }

    timeStr << (timeStruct.tm_year + 1900) << " "; // YYYY

    timeStr << std::setw(2) << std::setfill('0') << timeStruct.tm_hour << ":"; // HH:
    timeStr << std::setw(2) << std::setfill('0') << timeStruct.tm_min << ":"; // MM:
    timeStr << std::setw(2) << std::setfill('0') << timeStruct.tm_sec; // SS
    FUNCTION_EXIT;
    return timeStr.str();
}

std::string PasHelpers::formatZones(const TA_Base_Bus::IPAAgentCorbaDef::ZoneCoverage& zones)
{
    FUNCTION_ENTRY("formatZones");

    // build a map of location keys to lists of zones for those location keys
    // change zoneId from unsigned long to string to map with actual zoneId
    // which can be obtained in the stn/occ equipment after "_" e.g. BBS.PA.PUBLIC.PZON_02A => 02A
    // unsigned long zoneId = CachedMaps::getInstance()->getPaZoneIdFromKey(zones[i]);
    typedef std::list<std::string> ZoneList;
    typedef std::map<unsigned long, ZoneList> LocationKeyToZoneListMap;

    LocationKeyToZoneListMap locationKeyToZoneListMap;

    std::ostringstream destinationList;

    try
    {
        // build the map
        for (int i = 0; i < zones.length(); ++i)
        {
            TA_IRS_Bus::PaZoneRecord record = CachedMaps::getInstance()->getPaZoneRecordFromKey(zones[i]);

            std::string equipmentName = record.m_occEquipmentEntityName.empty() ? record.m_stnEquipmentEntityName : record.m_occEquipmentEntityName;
            std::vector<std::string> tokens;

            PasHelpers::instance().tokenise(equipmentName, tokens, "PZON_");
            std::string zoneId = "";
            std::stringstream zoneNum;

            if (tokens.size() != 2)
            {
                zoneNum << zones[i];
                zoneId = zoneNum.str();
                LOG_DEBUG("No match for PZON_ , the equipmentName = %s, zoneID = %ld ", equipmentName, zoneId);
                //TA_THROW(InvalidPasConfigurationException("PAZone equipment is invalid") );
            }
            else
            {
                zoneId = tokens[1];
            }

            unsigned long locationKey = CachedMaps::getInstance()->getPaZoneLocationKeyFromKey(zones[i]);
            locationKeyToZoneListMap[locationKey].push_back(zoneId);
            tokens.clear();
        }

        // then print the list out
        // DBG(1,2,3);MSM(4,7,1) ...

        for (LocationKeyToZoneListMap::iterator locIter = locationKeyToZoneListMap.begin();
             locIter != locationKeyToZoneListMap.end(); /*locIter incremented below */)
        {
            // location name
            destinationList << CachedMaps::getInstance()->getLocationNameFromKey(locIter->first)
                << "(";

            for (ZoneList::iterator zoneIter = locIter->second.begin();
                 zoneIter != locIter->second.end(); /* zoneIter incremented below */)
            {
                destinationList << (*zoneIter);

                // avoid adding comma for the last item
                if (++zoneIter != locIter->second.end())
                {
                    destinationList << ",";
                }
            }

            // avoid adding semicolon for the last item
            if (++locIter != locationKeyToZoneListMap.end())
            {
                destinationList << ");";
            }
            else
            {
                destinationList << ")";
            }
        }
    }
    catch (TA_Base_Core::CachedMappingNotFoundException& e)
    {
        LOG_EXCEPTION_CATCH(SourceInfo, "TA_Base_Core::CachedMappingNotFoundException", e.what());

        FUNCTION_EXIT;
        TA_THROW(TA_Base_Core::BadParameterException("The specified zone does not exist in the database"));
    }

    FUNCTION_EXIT;
    return destinationList.str();
}

//TD19881
std::string PasHelpers::formatZone(int zoneID)
{
    FUNCTION_ENTRY("formatZone");

    std::string zoneStr("");

    try
    {
        // build the map
        TA_IRS_Bus::PaZoneRecord record = CachedMaps::getInstance()->getPaZoneRecordFromKey(zoneID);

        std::string equipmentName = record.m_occEquipmentEntityName.empty() ? record.m_stnEquipmentEntityName : record.m_occEquipmentEntityName;
        std::vector<std::string> tokens;

        PasHelpers::instance().tokenise(equipmentName, tokens, "PZON_");
        std::stringstream zoneNum;

        if (tokens.size() != 2)
        {
            zoneNum << zoneID;
            zoneStr = zoneNum.str();
            LOG_DEBUG("No match for PZON_ , the equipmentName = %s, zoneID = %ld ", equipmentName, zoneID);
        }
        else
        {
            zoneStr = tokens[1];
        }

        tokens.clear();
    }
    catch (TA_Base_Core::CachedMappingNotFoundException& e)
    {
        LOG_EXCEPTION_CATCH(SourceInfo, "TA_Base_Core::CachedMappingNotFoundException", e.what());

        FUNCTION_EXIT;
        TA_THROW(TA_Base_Core::BadParameterException("The specified zone does not exist in the database"));
    }

    FUNCTION_EXIT;
    return zoneStr;
}

//TD19881

std::string PasHelpers::formatReasons(const std::map<int, std::string>& reasons, const TA_Base_Bus::IPAAgentCorbaDef::ZoneCoverage& zones)//TD10320
{
    FUNCTION_ENTRY("formatReasons");

    std::ostringstream reasonList;
    int i;
    std::map<int, std::string>::const_iterator resIter;

    for (resIter = reasons.begin(), i = 0;
         resIter != reasons.end(); ++i/*resIter incremented below */)
    {
        // location name
        //TD19881
        std::string zoneStr = formatZone(zones[i]);
        //TD19881
        reasonList << zoneStr << "(";
        reasonList << resIter->second;

        // avoid adding semicolon for the last item
        if (++resIter != reasons.end())
        {
            reasonList << ");";
        }
        else
        {
            reasonList << ")";
        }
    }

    FUNCTION_EXIT;
    return reasonList.str();
}

//TD10320

std::string PasHelpers::formatInterval(const TA_Base_Bus::ITrainPaCorba::MessageSchedule& interval)
{
    FUNCTION_ENTRY("formatInterval");

    std::ostringstream formatStr;
    formatStr << std::setw(2) << std::setfill('0') << interval.period << ":"
        << std::setw(2) << std::setfill('0') << interval.duration;
    FUNCTION_EXIT;
    return formatStr.str();
}

////////////////////////////////////////////////////////
//
//            Comms message senders
//
////////////////////////////////////////////////////////

void PasHelpers::sendCurrentBroadcastsUpdate(const TA_Base_Bus::IPAAgentCorbaDef::CurrentBroadcastsUpdateData& updateData)
{
    LOG_SCOPE("PasHelpers::sendCurrentBroadcastsUpdate");
    FUNCTION_ENTRY("sendCurrentBroadcastsUpdate");

    LOG_INFO("Sending CurrentBroadcastsUpdate message to subscribers");

    try_catch("sendCurrentBroadcastsUpdate()", [&]
    {
        CORBA::Any data;

        data <<= updateData;

        m_paAgentCommsSender->sendCommsMessage(PAAgentComms::CurrentBroadcastsUpdate,                  // Context::Type
                                               CachedConfig::getInstance()->getAgentKey(),             // EntityKey
                                               data,                                                   // Data
                                               CachedConfig::getInstance()->getAgentSubsystemKey(),    // Subsystem
                                               CachedConfig::getInstance()->getAgentLocationKey(),     // LocationKey
                                               NULL);
    });

    FUNCTION_EXIT;
}

void PasHelpers::sendDvaMessagesUpdate(const TA_Base_Bus::IPAAgentCorbaDef::DvaMessagesUpdateData& updateData)
{
    LOG_SCOPE("PasHelpers::sendDvaMessagesUpdate");
    FUNCTION_ENTRY("sendDvaMessagesUpdate");

    LOG_INFO("Sending DvaMessagesUpdate message to subscribers");

    try_catch("sendDvaMessagesUpdate()", [&]
    {
        CORBA::Any data;

        data <<= updateData;

        m_paAgentCommsSender->sendCommsMessage(PAAgentComms::DvaMessagesUpdate,                        // Context::Type
                                               CachedConfig::getInstance()->getAgentKey(),             // EntityKey
                                               data,                                                   // Data
                                               CachedConfig::getInstance()->getAgentSubsystemKey(),    // Subsystem
                                               CachedConfig::getInstance()->getAgentLocationKey(),     // LocationKey
                                               NULL);
    });

    FUNCTION_EXIT;
}

////////////////////////////////////////////////////////
//
//            State update message senders
//
////////////////////////////////////////////////////////

void PasHelpers::sendPAAgentBroadcastParameterUpdate(TA_Base_Bus::IPAAgentCorbaDef::EBroadcastParameter parameter, const CORBA::Any& parameterData, const char* broadcastId)
{
    LOG_SCOPE("PasHelpers::sendPAAgentBroadcastParameterUpdate");
    FUNCTION_ENTRY("sendPAAgentBroadcastParameterUpdate");

    LOG_INFO("Sending PAAgentBroadcastParameterUpdate message to subscribers");

    try_catch("sendPAAgentBroadcastParameterUpdate()", [&]
    {
        TA_Base_Bus::IPAAgentCorbaDef::BroadcastParameter* disposableUpdateData = new TA_Base_Bus::IPAAgentCorbaDef::BroadcastParameter;

        disposableUpdateData->broadcastId   = broadcastId;
        disposableUpdateData->parameter     = parameter;
        disposableUpdateData->data          = parameterData;

        CORBA::Any packetData;

        packetData <<= disposableUpdateData;

        m_stateUpdateMessageSender->sendStateUpdateMessage(PAAgentStateUpdate::PAAgentBroadcastParameterUpdate,
                                                           0,
                                                           CachedConfig::getInstance()->getAgentName(),
                                                           packetData);

        disposableUpdateData = NULL;
    });

    FUNCTION_EXIT;
}

void PasHelpers::sendPAAgentRemovedBroadcastUpdate(const char* updateData)
{
    LOG_SCOPE("PasHelpers::sendPAAgentRemovedBroadcastUpdate");
    FUNCTION_ENTRY("sendPAAgentRemovedBroadcastUpdate");

    LOG_INFO("Sending PAAgentRemovedBroadcastUpdate message to subscribers");

    try_catch("sendPAAgentRemovedBroadcastUpdate()", [&]
    {
        CORBA::Any data;

        data <<= CORBA::Any::from_string(updateData, 0);

        m_stateUpdateMessageSender->sendStateUpdateMessage(PAAgentStateUpdate::PAAgentRemovedBroadcastUpdate,
                                                           0,
                                                           CachedConfig::getInstance()->getAgentName(),
                                                           data);
    });

    FUNCTION_EXIT;
}

void PasHelpers::sendPAAgentNewBroadcastUpdate(const TA_Base_Bus::IPAAgentCorbaDef::BroadcastConfig& updateData)
{
    LOG_SCOPE("PasHelpers::sendPAAgentNewBroadcastUpdate");
    FUNCTION_ENTRY("sendPAAgentNewBroadcastUpdate");

    LOG_INFO("Sending PAAgentNewBroadcastUpdate message to subscribers");

    try_catch("sendPAAgentNewBroadcastUpdate()", [&]
    {
        CORBA::Any data;

        data <<= updateData;

        m_stateUpdateMessageSender->sendStateUpdateMessage(PAAgentStateUpdate::PAAgentNewBroadcastUpdate,
                                                           0,
                                                           CachedConfig::getInstance()->getAgentName(),
                                                           data);
    });

    FUNCTION_EXIT;
}

void PasHelpers::sendPAAgentRenamedBroadcastUpdate(const TA_Base_Bus::IPAAgentCorbaDef::CurrentBroadcastsUpdateData& updateData)
{
    LOG_SCOPE("PasHelpers::sendPAAgentRenamedBroadcastUpdate");
    FUNCTION_ENTRY("sendPAAgentRenamedBroadcastUpdate");

    LOG_INFO("Sending PAAgentRenamedBroadcastUpdate message to subscribers");

    try_catch("sendPAAgentRenamedBroadcastUpdate()", [&]
    {
        CORBA::Any data;

        data <<= updateData;

        m_stateUpdateMessageSender->sendStateUpdateMessage(PAAgentStateUpdate::PAAgentRenamedBroadcastUpdate,
                                                           0,
                                                           CachedConfig::getInstance()->getAgentName(),
                                                           data);
    });

    FUNCTION_EXIT;
}

void PasHelpers::tokenise(const std::string& str, std::vector<std::string>& tokens, const std::string& delimiters)
{
    FUNCTION_ENTRY("tokenise");

    /*
    // Skip delimiters at beginning.
    std::string::size_type lastPos = str.find_first_not_of(delimiters, 0);
    // Find first "non-delimiter".
    std::string::size_type pos     = str.find_first_of(delimiters, lastPos);

    while (std::string::npos != pos || std::string::npos != lastPos)
    {
        // Found a token, add it to the vector.
        tokens.push_back(str.substr(lastPos, pos - lastPos));
        // Skip delimiters.  Note the "not_of"
        lastPos = str.find_first_not_of(delimiters, pos);
        // Find next "non-delimiter"
        pos = str.find_first_of(delimiters, lastPos);
    }
    */
    std::string::size_type firstPos = str.find(delimiters, 0);

    if (std::string::npos != firstPos && firstPos > 0)
    {
        firstPos = 0;
    }

    // Find first "non-delimiter".
    std::string::size_type secondpos = str.find(delimiters, firstPos);

    while (std::string::npos != secondpos)
    {
        tokens.push_back(str.substr(firstPos, secondpos - firstPos));

        // Found a token, add it to the vector.

        // Skip delimiters.  Note the "not_of"
        firstPos = secondpos + delimiters.size();
        secondpos = str.find_first_of(delimiters, firstPos);

        if (std::string::npos != secondpos)
        {
            tokens.push_back(str.substr(firstPos, secondpos - firstPos));
            firstPos = secondpos + delimiters.size();
            secondpos = str.find_first_of(delimiters, firstPos);

            if (secondpos == std::string::npos)
            {
                tokens.push_back(str.substr(firstPos, str.size() - firstPos));
            }
        }
        else
        {
            tokens.push_back(str.substr(firstPos, str.size() - firstPos));
        }
    }

    FUNCTION_EXIT;
}

////////////////////////////////////////////////////////
//
//            Alarm message senders
//
////////////////////////////////////////////////////////

// ExceptionChecked
void PasHelpers::raiseInvalidPAConfigurationAlarm(const std::string& detail)
{
    FUNCTION_ENTRY("raiseInvalidPAConfigurationAlarm");

    try
    {
        if (CachedConfig::getInstance()->getAgentKey() == 0)
        {
            // The agent has not been initialised completely, we can only log this error
            // below

            FUNCTION_EXIT;
            throw;
        }

        // construct the DescriptionParameters list
        DescriptionParameters parameters;

        std::string locationName = CachedMaps::getInstance()->getLocationNameFromKey(
            CachedConfig::getInstance()->getAgentLocationKey());

        NameValuePair pair1("LocationName", locationName);
        parameters.push_back(&pair1);
        NameValuePair pair2("Detail", detail);
        parameters.push_back(&pair2);

        TA_Base_Core::AlarmHelperManager::getInstance().getAlarmHelper().submitAlarmWithAsset(PAAgentAlarms::InvalidPAConfiguration,
                                                                                              CachedConfig::getInstance()->getAgentKey(),             // entity key
                                                                                              CachedConfig::getInstance()->getAgentTypeKey(),         // entity type key
                                                                                              parameters,                                  // parameters
                                                                                              CachedConfig::getInstance()->getAgentName(),            // entity name
                                                                                              CachedConfig::getInstance()->getAgentLocationKey(),     // region key
                                                                                              CachedConfig::getInstance()->getAgentSubsystemKey(),    // subystem key
                                                                                              CachedConfig::getInstance()->getAgentAssetName(),
                                                                                              TA_Base_Core::AlarmConstants::defaultTime);
    }
    catch (...)
    {
        std::ostringstream msg;
        msg << "Failed to raise the InvalidPAConfiguration alarm.  Detail='"
            << detail << "'";
        LOG_ERROR(msg);
    }

    FUNCTION_EXIT;
}

// ExceptionChecked
void PasHelpers::closeInvalidPAConfigurationAlarm()
{
    FUNCTION_ENTRY("closeInvalidPAConfigurationAlarm");

    try
    {
        TA_Base_Core::AlarmHelperManager::getInstance().getAlarmHelper().closeAlarmAtLocation(PAAgentAlarms::InvalidPAConfiguration,
                                                                                              CachedConfig::getInstance()->getAgentKey(),
                                                                                              CachedConfig::getInstance()->getAgentLocationKey()
        );
    }
    catch (...)
    {
        LOG_ERROR("Failed to close the InvalidPAConfiguration alarm");
    }

    FUNCTION_EXIT;
}

// TD 15128
/*
// ExceptionChecked
void PasHelpers::raisePASPrioritySchemeUpdateNonUniqueAlarm()
{
    FUNCTION_ENTRY("raisePASPrioritySchemeUpdateNonUniqueAlarm");

    // Threadsafe

    // Obtain handle to the unique Alarm Helper
    // liuyu++ TD4657
    // NonUniqueAlarmHelper* alarmHelper = AlarmHelperManager::getInstance().getNonUniqueAlarmHelper();
    // ++liuyu TD4657

    try
    {
        if (CachedConfig::getInstance()->getAgentKey() == 0)
        {
            // The agent has not been initialised completely, we can only log this error
            // below
            throw;
        }

        // construct the DescriptionParameters list
        DescriptionParameters parameters;

        std::string locationName = CachedMaps::getInstance()->getLocationNameFromKey(
            CachedConfig::getInstance()->getAgentLocationKey() );

        NameValuePair pair1 ( "LocationName", locationName );
        parameters.push_back ( &pair1 );

        // Don't care about the returned alarmId
        // liuyu++ TD4657
        // m_alarmHelpersubmitAlarm (
        TA_Base_Core::AlarmHelperManager::getInstance().getNonUniqueAlarmHelper().submitAlarmWithAsset (
        // ++liuyu TD4657
            PAAgentAlarms::PASPrioritySchemeUpdate,
            CachedConfig::getInstance()->getAgentKey(),             // entity key
            CachedConfig::getInstance()->getAgentTypeKey(),         // entity type key
            parameters,                                  // parameters
            CachedConfig::getInstance()->getAgentName(),            // entity name
            CachedConfig::getInstance()->getAgentLocationKey(),     // region key
            CachedConfig::getInstance()->getAgentSubsystemKey(),    // subystem key
            CachedConfig::getInstance()->getAgentAssetName(),
            TA_Base_Core::AlarmConstants::defaultTime );
    }
    catch(...)
    {
        std::ostringstream msg;
        msg << "Failed to raise the PASPrioritySchemeUpdate alarm";
        LOG_ERROR(msg);
        // liuyu++ TD4657
        // delete alarmHelper;
        // alarmHelper = NULL;
        // ++liuyu TD4657
    }

    // liuyu++ TD4657
    // delete alarmHelper;
    // alarmHelper = NULL;
    // ++liuyu TD4657

    FUNCTION_EXIT;
}
*/
// TD 15128

// ExceptionChecked
void PasHelpers::raisePASCommsFailureAlarm()
{
    FUNCTION_ENTRY("raisePASCommsFailureAlarm");

    // Threadsafe

    if (!m_pasAlarm.empty())
    {
        //alarm is already raised
        FUNCTION_EXIT;
        return;
    }

    try
    {
        if (CachedConfig::getInstance()->getAgentKey() == 0)
        {
            // The agent has not been initialised completely, we can only log this error
            // below

            FUNCTION_EXIT;
            throw;
        }

        // construct the DescriptionParameters list
        DescriptionParameters parameters;

        std::string locationName = ThisLocation::name();

        NameValuePair pair1("LocationName", locationName);
        parameters.push_back(&pair1);

        std::string alarmId = TA_Base_Core::AlarmHelperManager::getInstance().getNonUniqueAlarmHelper().submitAlarmWithAsset(
            PAAgentAlarms::PASCommsFailure,
            CachedConfig::getInstance()->getAgentKey(),             // entity key
            CachedConfig::getInstance()->getAgentTypeKey(),         // entity type key
            parameters,                                             // parameters
            CachedConfig::getInstance()->getAgentName(),            // entity name
            CachedConfig::getInstance()->getAgentLocationKey(),     // region key
            CachedConfig::getInstance()->getAgentSubsystemKey(),    // subystem key
            CachedConfig::getInstance()->getAgentAssetName(),
            TA_Base_Core::AlarmConstants::defaultTime);

        m_pasAlarm = alarmId;
    }
    catch (...)
    {
        std::ostringstream msg;
        msg << "Failed to raise the PASCommsFailure alarm";
        LOG_ERROR(msg);
        // delete alarmHelper;
        // alarmHelper = NULL;
    }

    // delete alarmHelper;
    // alarmHelper = NULL;
    // ++liuyu TD4657

    FUNCTION_EXIT;
}

// ExceptionChecked
void PasHelpers::closePASCommsFailureAlarm()
{
    FUNCTION_ENTRY("closePASCommsFailureAlarm");

    if (m_pasAlarm.empty())
    {
        //no alarm has been raised, so there is no alarm to close
        FUNCTION_EXIT;
        return;
    }

    try
    {
        TA_Base_Core::AlarmHelperManager::getInstance().getNonUniqueAlarmHelper().closeAlarmAtLocation(m_pasAlarm, CachedConfig::getInstance()->getAgentLocationKey());
        m_pasAlarm = "";
    }
    catch (...)
    {
        LOG_ERROR("Failed to close the PASCommsFailure alarm");
    }

    FUNCTION_EXIT;
}

// ExceptionChecked
void PasHelpers::raisePASVersionConflictAlarm(const std::string& stationInConflict)
{
    FUNCTION_ENTRY("raisePASVersionConflictAlarm");

    try
    {
        if (CachedConfig::getInstance()->getAgentKey() == 0)
        {
            // The agent has not been initialised completely, we can only log this error
            // below

            FUNCTION_EXIT;
            throw;
        }

        // construct the DescriptionParameters list
        DescriptionParameters parameters;

        auto locationName = ThisLocation::name();

        NameValuePair pair1("LocationName", locationName);
        parameters.push_back(&pair1);

        NameValuePair pair2("TargetLocationName", stationInConflict);
        parameters.push_back(&pair2);

        TA_Base_Core::AlarmHelperManager::getInstance().getAlarmHelper().submitAlarmWithAsset(PAAgentAlarms::PASVersionConflict,
                                                                                              CachedConfig::getInstance()->getAgentKey(),             // entity key
                                                                                              CachedConfig::getInstance()->getAgentTypeKey(),         // entity type key
                                                                                              parameters,                                  // parameters
                                                                                              CachedConfig::getInstance()->getAgentName(),            // entity name
                                                                                              CachedConfig::getInstance()->getAgentLocationKey(),     // region key
                                                                                              CachedConfig::getInstance()->getAgentSubsystemKey(),    // subystem key
                                                                                              CachedConfig::getInstance()->getAgentAssetName(),
                                                                                              TA_Base_Core::AlarmConstants::defaultTime);
    }
    catch (...)
    {
        std::ostringstream msg;
        msg << "Failed to raise the PASVersionConflict alarm";
        LOG_ERROR(msg);
    }

    FUNCTION_EXIT;
}

// ExceptionChecked
void PasHelpers::closePASVersionConflictAlarm()
{
    FUNCTION_ENTRY("closePASVersionConflictAlarm");

    try
    {
        TA_Base_Core::AlarmHelperManager::getInstance().getAlarmHelper().closeAlarmAtLocation(PAAgentAlarms::PASVersionConflict,
                                                                                              CachedConfig::getInstance()->getAgentKey(),
                                                                                              CachedConfig::getInstance()->getAgentLocationKey()
        );
    }
    catch (...)
    {
        LOG_ERROR("Failed to close the PASVersionConflict alarm");
    }

    FUNCTION_EXIT;
}

void PasHelpers::raiseTrainLibraryVersionAlarm(int trainID, int libraryVersion, int iscsLibraryVersion)
{
    FUNCTION_ENTRY("raiseTrainLibraryVersionAlarm");

    // no need to lock the map - only called by the corba message thread

    TrainIdToAlarmDetailsMap::iterator findIter = m_trainToAlarmIdMap.find(trainID);

    if (findIter != m_trainToAlarmIdMap.end())
    {
        if (findIter->second.trainLibraryVersion == libraryVersion)
        {
            // this alarm already raised
            FUNCTION_EXIT;
            return;
        }
        else
        {
            // close and re-raise the alarm
            closeTrainLibraryVersionAlarm(trainID);
        }
    }

    // Threadsafe

    try
    {
        if (CachedConfig::getInstance()->getAgentKey() == 0)
        {
            // The agent has not been initialised completely, we can only log this error
            // below

            FUNCTION_EXIT;
            throw;
        }

        // construct the DescriptionParameters list
        DescriptionParameters parameters;

        std::stringstream trainIdStr;
        trainIdStr << trainID;
        std::stringstream libraryStr;
        libraryStr << libraryVersion;
        std::stringstream iscsLibraryStr;
        iscsLibraryStr << iscsLibraryVersion;

        NameValuePair pair1("TrainId", trainIdStr.str());
        parameters.push_back(&pair1);
        NameValuePair pair2("TrainVersion", libraryStr.str());
        parameters.push_back(&pair2);
        NameValuePair pair3("IscsVersion", iscsLibraryStr.str());
        parameters.push_back(&pair3);

        TrainAlarm newAlarm;
        newAlarm.trainLibraryVersion = libraryVersion;
        newAlarm.alarmId = "";
        /* TODO uncomment this as part of TD 15292 when the alarm type is added
        newAlarm.alarmId =
            TA_Base_Core::AlarmHelperManager::getInstance().getNonUniqueAlarmHelper().submitAlarm (
                PAAgentAlarms::TrainPALibraryInconsistentWithISCS,
                CachedConfig::getInstance()->getAgentKey(),             // entity key
                CachedConfig::getInstance()->getAgentTypeKey(),         // entity type key
                parameters,                                  // parameters
                CachedConfig::getInstance()->getAgentName(),            // entity name
                CachedConfig::getInstance()->getAgentLocationKey(),     // region key
                CachedConfig::getInstance()->getAgentSubsystemKey() );  // subystem key
        */

        m_trainToAlarmIdMap[trainID] = newAlarm;
    }
    catch (...)
    {
        std::ostringstream msg;
        msg << "Failed to raise the TrainPALibraryInconsistentWithISCS alarm";
        LOG_ERROR(msg);
    }

    FUNCTION_EXIT;
}

void PasHelpers::closeTrainLibraryVersionAlarms()
{
    FUNCTION_ENTRY("closeTrainLibraryVersionAlarms");

    try
    {
        /* TODO uncomment this as part of TD 15292 when the alarm type is added
        m_alarmHelpercloseAlarmAtLocation(
            PAAgentAlarms::TrainPALibraryInconsistentWithISCS,
            CachedConfig::getInstance()->getAgentKey(),
            CachedConfig::getInstance()->getAgentLocationKey()
            );
        */

        m_trainToAlarmIdMap.clear();
    }
    catch (...)
    {
        LOG_ERROR("Failed to close the TrainPALibraryInconsistentWithISCS alarm");
    }

    FUNCTION_EXIT;
}

void PasHelpers::closeTrainLibraryVersionAlarm(int trainID)
{
    FUNCTION_ENTRY("closeTrainLibraryVersionAlarm");

    TrainIdToAlarmDetailsMap::iterator findIter = m_trainToAlarmIdMap.find(trainID);

    if (findIter == m_trainToAlarmIdMap.end())
    {
        FUNCTION_EXIT;
        return;        // not active
    }

    try
    {
        /* TODO uncomment this as part of TD 15292 when the alarm type is added
        TA_Base_Core::AlarmHelperManager::getInstance().getNonUniqueAlarmHelper().closeAlarmAtLocation(findIter->second.alarmId,
        CachedConfig::getInstance()->getAgentLocationKey());
        */
        m_trainToAlarmIdMap.erase(findIter);
    }
    catch (...)
    {
        LOG_ERROR("Failed to close the TrainPALibraryInconsistentWithISCS alarm");
    }

    FUNCTION_EXIT;
}

void PasHelpers::closeTrainLibraryVersionAlarmOnVersionChange(int newIscsVersion)
{
    FUNCTION_ENTRY("closeTrainLibraryVersionAlarmOnVersionChange");

    TrainIdToAlarmDetailsMap::iterator iter = m_trainToAlarmIdMap.begin();

    while (iter != m_trainToAlarmIdMap.end())
    {
        // close the alarm if the train now matches
        if (iter->second.trainLibraryVersion == newIscsVersion)
        {
            try
            {
                //              TA_Base_Core::AlarmHelperManager::getInstance().getNonUniqueAlarmHelper().closeAlarmAtLocation(iter->second.alarmId,
                //                                              CachedConfig::getInstance()->getAgentLocationKey());
            }
            catch (...)
            {
                LOG_ERROR("Failed to close the TrainPALibraryInconsistentWithISCS alarm");
            }

            m_trainToAlarmIdMap.erase(iter++);
        }
        else
        {
            ++iter;
        }
    }

    FUNCTION_EXIT;
}

void PasHelpers::sendBroadcastProgressUpdate(AbstractBroadcast* broadcast)
{
    FUNCTION_ENTRY("sendBroadcastProgressUpdate");

    LOG_INFO("Sending BroadcastProgressUpdate message for broadcastId=%s", broadcast->getBroadcastId());

    CORBA::Any data;
    NameValuePair nvp(TA_Base_Bus::IPAAgentCorbaDef::BROADCAST_ID_FILTER_STRING, broadcast->getBroadcastId());

    std::vector<NameValuePair*> extraFilterableData;
    extraFilterableData.push_back(&nvp);

    data <<= broadcast->getProgressReportCopy();

    try_catch("sendBroadcastProgressUpdate(): sendCommsMessage", [&]
    {
        // Send Comms message for PA Managers
        m_paAgentCommsSender->sendCommsMessage(
            PAAgentComms::BroadcastProgressUpdate,                  // Context::Type
            m_cachedConfigInstance->getAgentKey(),                  // EntityKey
            data,                                                   // Data
            m_cachedConfigInstance->getAgentSubsystemKey(),         // Subsystem
            m_cachedConfigInstance->getAgentLocationKey(),          // LocationKey
            &extraFilterableData);                                  // BroadcastId
    });

    try_catch("sendBroadcastProgressUpdate(): sendStateUpdateMessage", [&]
    {
        // Send State update message for PA Agents
        m_stateUpdateMessageSender->sendStateUpdateMessage(
            PAAgentStateUpdate::PAAgentBroadcastProgressUpdate,
            0,
            m_cachedConfigInstance->getAgentName(),
            data);
    });

    FUNCTION_EXIT;
}

// jeffrey++ TD10393
std::string PasHelpers::convertTypeString(char InType)
{
    FUNCTION_ENTRY("convertTypeString");

    std::string OutType;

    if (InType == 'E')
    {
        OutType = "Emergency";
    }
    else if (InType == 'N')
    {
        OutType = "Normal";
    }
    else
    {
        OutType = "Deleted";
    }

    FUNCTION_EXIT;
    return OutType;
}

std::string PasHelpers::checkLabel(const std::string& InLabel)
{
    FUNCTION_ENTRY("checkLabel");

    std::string OutLabel;

    if (InLabel.empty())
    {
        OutLabel = "<empty>";
    }
    else
    {
        OutLabel = InLabel;
    }

    FUNCTION_EXIT;
    return OutLabel;
}
