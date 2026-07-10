/**
 * The source code in this file is the property of
 * Ripple Systems and is not for redistribution
 * in any form.
 *
 * Source:   $File: //depot/C830AR_TIP/C830AR/transactive/app/pa/PAAgent/src/CachedConfig.cpp $
 * @author:  Ripple
 * @version: $Revision: #9 $
 *
 * Last modification: $DateTime: 2025/01/06 12:26:20 $
 * Last modified by:  $Author: limin.zhu $
 *
 */
// CachedConfig.cpp: implementation of the CachedConfig class.
//
//////////////////////////////////////////////////////////////////////

#include "pch.h"
#include "CachedConfig.h"
#include "bus/scada/datapoint_library/src/DataPoint.h"
#include "core/synchronisation/src/ThreadGuard.h"
#include "core/utilities/src/TAAssert.h"
#include "core/utilities/src/DebugUtilEx.h"
#include "core/utilities/src/RunParams.h"
#include "core/utilities/src/MakeNameValuePairsString.h"
#include <string>
#include <vector>

using namespace TA_Base_Core;

CachedConfig* CachedConfig::m_me = NULL;

NonReEntrantThreadLockable CachedConfig::m_lockForConfigUpdate;
NonReEntrantThreadLockable CachedConfig::m_lockForObserverMap;

// Note regarding locks
//
// Locks have been deliberately removed for FastPollRate and
// WorkerThreadsSleepTime for performance reasons.  Only a single
// thread will be updating the member variables and therefore it
// is relatively safe.  What may occasionally happen is that the
// members are read just as an online update is applied.  The worst
// that could happen is that the value becomes partially corrupted,
// the degree to which depends on the architecture.  As these
// particular parameters are read very frequently, the next time
// the member is read, the value will be correct.  The worst
// conceivable corruption would be a value almost double that
// intended.  One byte example
//
// Original value       00001111
// Intended new value   00010000
// Corrupted value      00011111
//
// So long as the corruption is temporary and does not cause the
// agent to crash it is acceptable.

//NonReEntrantThreadLockable CachedConfig::m_lockForFastPoll;
//NonReEntrantThreadLockable CachedConfig::m_lockForWorkerThreads;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CachedConfig::CachedConfig()
    : m_agentKey(0),
    m_agentName(""),
    m_agentTypeKey(0),
    m_agentLocationKey(0),
    m_agentSubsystemKey(0),
    m_agentInRuntime(false),
    m_assetName(""),
    m_monitorProcessKey(0)
{
    //m_decisionComponentIds.clear();
    m_keyToObserversMap.clear();

    // Initialise map to flag all members (except for AgentX) as uninitialised

    m_memberToInitialisedFlagMap[&m_agentKey] = true;
    m_memberToInitialisedFlagMap[&m_agentName] = true;
    m_memberToInitialisedFlagMap[&m_agentTypeKey] = true;
    m_memberToInitialisedFlagMap[&m_agentLocationKey] = true;
    m_memberToInitialisedFlagMap[&m_agentSubsystemKey] = true;
    m_memberToInitialisedFlagMap[&m_agentInRuntime] = true;

    // We don't need to explicitly set the other members to false as the
    // default construction for the bool used when the map is accessed with an
    // unrecognised member pointer will always be false.

    // Refer to Google Groups: http://groups.google.com.au/groups?hl=en&lr=&ie=UTF-8&threadm=c1b53j%249sl%241%40newshispeed.ch&rnum=29&prev=/groups%3Fq%3D%2522default%2Bconstructor%2522%2Bbool%26hl%3Den%26lr%3D%26ie%3DUTF-8%26start%3D20%26sa%3DN
}

CachedConfig::~CachedConfig()
{
    m_keyToObserversMap.clear();
}

CachedConfig* CachedConfig::getInstance()
{
    if (0 == m_me)
    {
        // Double checking to prevent multiple threads
        // creating multiple instances.

        ThreadGuard guard(m_lockForConfigUpdate);

        if (0 == m_me)
        {
            m_me = new CachedConfig();
        }
    }

    return m_me;
}

void CachedConfig::removeInstance()
{
    //
    // Guard this to prevent multiple threads atempting
    // to delete/create simultaneously
    //
    ThreadGuard guard(m_lockForConfigUpdate);

    if (m_me != NULL)
    {
        delete m_me;
        m_me = NULL;
    }
}

/**
 * attachObserver
 *
 * Adds the ICachedConfigObserver instance to the collection of observers.  If the observer
 * already exists for the key, no change will be made.
 *
 * @param observer pointer to the ICachedConfigObserver instance
 *
 * @param key the ECachedConfigItemKey the observer is interested in
 *
 */
void CachedConfig::attachObserver(ICachedConfigObserver* observer, ECachedConfigItemKey key)
{
    // Search for all observers associated with the key
    std::pair<KeyToObserversMapIt, KeyToObserversMapIt> range = m_keyToObserversMap.equal_range(key);

    bool found(false);

    for (; range.first != range.second; ++range.first)
    {
        if ((range.first)->second == observer)
        {
            found = true;
            break;
        }
    }

    if (!found)
    {
        ThreadGuard guard(m_lockForObserverMap);
        m_keyToObserversMap.insert(key2observer(key, observer));
    }
}

/**
 * detachObserver
 *
 * Removes all instances of the ICachedConfigObserver pointer in the observer collection.  The
 * observer will be removed for all keys.
 *
 */
void CachedConfig::detachObserver(ICachedConfigObserver* observer)
{
    ThreadGuard guard(m_lockForObserverMap);

    KeyToObserversMapIt it = m_keyToObserversMap.begin();

    while (it != m_keyToObserversMap.end())
    {
        if (it->second == observer)
        {
            m_keyToObserversMap.erase(it++);
        }
        else
        {
            ++it;
        }
    }
}

/**
 * notifyInterestedObservers
 *
 * Calls the processCachedConfigUpdate() method on all registered observers interested in the
 * specified key.  Implementors of ICachedConfigObserver MUST make sure that no exceptions
 * are unhandled.
 *
 * @param key used to call specific observers
 *
 */
void CachedConfig::notifyInterestedObservers(ECachedConfigItemKey key)
{
    ThreadGuard guard(m_lockForObserverMap);

    // Search for all observers associated with a key
    std::pair<KeyToObserversMapIt, KeyToObserversMapIt> range = m_keyToObserversMap.equal_range(key);

    for (; range.first != range.second; ++range.first)
    {
        try
        {
            (range.first)->second->processCachedConfigUpdate(key);
        }
        catch (...)
        {
            // Do nothing and continue
        }
    }
}

void CachedConfig::setAgentKey(unsigned long param)
{
    ThreadGuard guard(m_lockForConfigUpdate);
    m_agentKey = param;
    m_memberToInitialisedFlagMap[&m_agentKey] = true;
    LOG_DEBUG("setAgentKey(): %s", nvps(m_agentKey));
}

void CachedConfig::setAgentTypeKey(unsigned long param)
{
    ThreadGuard guard(m_lockForConfigUpdate);
    m_agentTypeKey = param;
    m_memberToInitialisedFlagMap[&m_agentTypeKey] = true;
    LOG_DEBUG("setAgentTypeKey(): %s", nvps(m_agentTypeKey));
}

void CachedConfig::setAgentLocationKey(unsigned long param)
//throw(TA_Base_Core::InvalidECSConfigurationException)
{
    if (param == 0)
    {
        std::ostringstream msg;
        msg << "Invalid PA Agent parameter.  "
            << "AgentLocationKey=" << param;

        TA_THROW(InvalidPasConfigurationException(msg.str().c_str()));
    }

    ThreadGuard guard(m_lockForConfigUpdate);
    m_agentLocationKey = param;
    m_memberToInitialisedFlagMap[&m_agentLocationKey] = true;
    LOG_DEBUG("setAgentLocationKey(): %s", nvps(m_agentLocationKey));
}

void CachedConfig::setAgentSubsystemKey(unsigned long param)
{
    ThreadGuard guard(m_lockForConfigUpdate);
    m_agentSubsystemKey = param;
    m_memberToInitialisedFlagMap[&m_agentSubsystemKey] = true;
    LOG_DEBUG("setAgentSubsystemKey(): %s", nvps(m_agentSubsystemKey));
}

void CachedConfig::setAgentInRuntime(bool param)
{
    //Implicitly threadsafe: ThreadGuard guard( m_lockForConfigUpdate );
    m_agentInRuntime = param;
    m_memberToInitialisedFlagMap[&m_agentInRuntime] = true;
    LOG_DEBUG("setAgentInRuntime(): %s", nvps(m_agentInRuntime));
}

void CachedConfig::setAgentAssetName(std::string param)
{
    ThreadGuard guard(m_lockForConfigUpdate);
    m_assetName = param;
    m_memberToInitialisedFlagMap[&m_assetName] = true;
    LOG_DEBUG("setAgentAssetName(): %s", nvps(m_assetName));
}

void CachedConfig::setLocalATSAgentKey(unsigned long param) //throw(TA_Base_Core::InvalidPasConfigurationException)
{
    if (param == 0)
    {
        std::ostringstream msg;
        msg << "Invalid PA Agent parameter.  "
            << "LocalATSAgentKey=" << param;

        TA_THROW(InvalidPasConfigurationException(msg.str().c_str()));
    }

    ThreadGuard guard(m_lockForConfigUpdate);
    m_localATSAgentKey = param;
    m_memberToInitialisedFlagMap[&m_localATSAgentKey] = true;
    LOG_DEBUG("setLocalATSAgentKey(): %s", nvps(m_localATSAgentKey));
}

void CachedConfig::setLocalTrainAgentKey(unsigned long param) //throw(TA_Base_Core::InvalidPasConfigurationException)
{
    if (param == 0)
    {
        std::ostringstream msg;
        msg << "Invalid PA Agent parameter.  "
            << "LocalTrainAgentKey=" << param;

        TA_THROW(InvalidPasConfigurationException(msg.str().c_str()));
    }

    ThreadGuard guard(m_lockForConfigUpdate);
    m_localTrainAgentKey = param;
    m_memberToInitialisedFlagMap[&m_localTrainAgentKey] = true;
    LOG_DEBUG("setLocalTrainAgentKey(): %s", nvps(m_localTrainAgentKey));
}

void CachedConfig::setFastPollRateInMSecs(unsigned long param)
{
    //Acceptable threadsafety - see notes above
    //ThreadGuard guard( m_lockForFastPoll );
    m_fastPollRateInMSecs = param;
    m_memberToInitialisedFlagMap[&m_fastPollRateInMSecs] = true;
    LOG_DEBUG("setFastPollRateInMSecs(): %s", nvps(m_fastPollRateInMSecs));
}

void CachedConfig::setWorkerThreadsSleepTimeInMSecs(unsigned long param)
{
    //Acceptable threadsafety - see notes above
    //ThreadGuard guard( m_lockForWorkerThreads );
    m_workerThreadsSleepTimeInMSecs = param;
    m_memberToInitialisedFlagMap[&m_workerThreadsSleepTimeInMSecs] = true;
    LOG_DEBUG("setWorkerThreadsSleepTimeInMSecs(): %s", nvps(m_workerThreadsSleepTimeInMSecs));
}

void CachedConfig::setSocketTimeoutInSecs(unsigned long param)
{
    // Scope to limit locking as the notifyInterestedObservers() takes an unknown amount of time
    // Don't want to hold up any getX() operations in particular
    {
        ThreadGuard guard(m_lockForConfigUpdate);
        m_socketTimeoutInSecs = param;
        m_memberToInitialisedFlagMap[&m_socketTimeoutInSecs] = true;
        LOG_DEBUG("setSocketTimeoutInSecs(): %s", nvps(m_socketTimeoutInSecs));
    }

    notifyInterestedObservers(CONFIG_SOCKET_TIMEOUT);
}

void CachedConfig::setRetries(unsigned long param)
{
    ThreadGuard guard(m_lockForConfigUpdate);
    m_retries = param;
    m_memberToInitialisedFlagMap[&m_retries] = true;
    LOG_DEBUG("setRetries(): %s", nvps(m_retries));
}

void CachedConfig::setPrimaryFEPCAddress(const std::string& param) //throw(TA_Base_Core::InvalidPasConfigurationException)
{
    if (param == "")
    {
        TA_THROW(InvalidPasConfigurationException("Empty PrimaryFEPCAddress set for PA Agent"));
    }

    // Scope to limit locking as the notifyInterestedObservers() takes an unknown amount of time
    // Don't want to hold up any getX() operations in particular
    {
        ThreadGuard guard(m_lockForConfigUpdate);
        m_primaryFEPCAddress = param;
        m_memberToInitialisedFlagMap[&m_primaryFEPCAddress] = true;
        LOG_DEBUG("setPrimaryFEPCAddress(): %s", nvps(m_primaryFEPCAddress));
    }

    notifyInterestedObservers(CONFIG_PRIMARY_FEPC_ADDRESS);
}

//TD17452
void CachedConfig::setMonitorProcessKey(unsigned long param)
{
    ThreadGuard guard(m_lockForConfigUpdate);
    m_monitorProcessKey = param;
    m_memberToInitialisedFlagMap[&m_monitorProcessKey] = true;
    LOG_DEBUG("setMonitorProcessKey(): %s", nvps(m_monitorProcessKey));
}

//TD17452

void CachedConfig::setSecondaryFEPCAddress(const std::string& param) //throw(TA_Base_Core::InvalidPasConfigurationException)
{
    if (param == "")
    {
        TA_THROW(InvalidPasConfigurationException("Empty SecondaryFEPCAddress set for PA Agent"));
    }

    // Scope to limit locking as the notifyInterestedObservers() takes an unknown amount of time
    // Don't want to hold up any getX() operations in particular
    {
        ThreadGuard guard(m_lockForConfigUpdate);
        m_secondaryFEPCAddress = param;
        m_memberToInitialisedFlagMap[&m_secondaryFEPCAddress] = true;
        LOG_DEBUG("setSecondaryFEPCAddress(): %s", nvps(m_secondaryFEPCAddress));
    }

    notifyInterestedObservers(CONFIG_SECONDARY_FEPC_ADDRESS);
}

void CachedConfig::setPrimaryFEPCPort(unsigned long param) //throw(TA_Base_Core::InvalidPasConfigurationException)
{
    if (param > 65535)
    {
        std::ostringstream msg;
        msg << "Invalid PA Agent parameter.  "
            << "PrimaryFEPCPort=" << param;

        TA_THROW(InvalidPasConfigurationException(msg.str().c_str()));
    }

    // Convert unsigned long to string now so we save processing when retrieving
    char buffer[40] = {0};
    sprintf(buffer, "%lu", param);

    // Scope to limit locking as the notifyInterestedObservers() takes an unknown amount of time
    // Don't want to hold up any getX() operations in particular
    {
        ThreadGuard guard(m_lockForConfigUpdate);
        m_primaryFEPCPort = buffer;
        m_memberToInitialisedFlagMap[&m_primaryFEPCPort] = true;
        LOG_DEBUG("setPrimaryFEPCPort(): %s", nvps(m_primaryFEPCPort));
    }

    notifyInterestedObservers(CONFIG_PRIMARY_FEPC_PORT);
}

void CachedConfig::setSecondaryFEPCPort(unsigned long param) //throw(TA_Base_Core::InvalidPasConfigurationException)
{
    if (param > 65535)
    {
        std::ostringstream msg;
        msg << "Invalid PA Agent parameter.  "
            << "SecondaryFEPCPort=" << param;

        TA_THROW(InvalidPasConfigurationException(msg.str().c_str()));
    }

    // Convert unsigned long to string now so we save processing when retrieving
    char buffer[40] = {0};
    sprintf(buffer, "%lu", param);

    // Scope to limit locking as the notifyInterestedObservers() takes an unknown amount of time
    // Don't want to hold up any getX() operations in particular
    {
        ThreadGuard guard(m_lockForConfigUpdate);
        m_secondaryFEPCPort = buffer;
        m_memberToInitialisedFlagMap[&m_secondaryFEPCPort] = true;
        LOG_DEBUG("setSecondaryFEPCPort(): %s", nvps(m_secondaryFEPCPort));
    }

    notifyInterestedObservers(CONFIG_SECONDARY_FEPC_PORT);
}

void CachedConfig::setIsMaster(bool param)
{
    //Implicitly threadsafe: ThreadGuard guard( m_lockForConfigUpdate );
    m_isMaster = param;
    m_memberToInitialisedFlagMap[&m_isMaster] = true;
    LOG_DEBUG("setIsMaster(): %s", nvps(m_isMaster));
}

void CachedConfig::setIsDepot(bool param)
{
    //Implicitly threadsafe: ThreadGuard guard( m_lockForConfigUpdate );
    m_isDepot = param;
    m_memberToInitialisedFlagMap[&m_isDepot] = true;
    LOG_DEBUG("setIsDepot(): %s", nvps(m_isDepot));
}

void CachedConfig::setLocalTrainAgentEntityName(const std::string& param)
{
    ThreadGuard guard(m_lockForConfigUpdate);
    m_localTrainAgentEntityName = param;
    m_memberToInitialisedFlagMap[&m_localTrainAgentEntityName] = true;
    LOG_DEBUG("setLocalTrainAgentEntityName(): %s", nvps(m_localTrainAgentEntityName));
}

void CachedConfig::setTisDisplayDurationInSecs(unsigned long param)
{
    ThreadGuard guard(m_lockForConfigUpdate);
    m_tisDisplayDuration = param;
    m_memberToInitialisedFlagMap[&m_tisDisplayDuration] = true;
    LOG_DEBUG("setTisDisplayDurationInSecs(): %s", nvps(m_tisDisplayDuration));
}

void CachedConfig::setKeepTerminatedBroadcastsInSecs(unsigned long param)
{
    ThreadGuard guard(m_lockForConfigUpdate);
    m_keepTerminatedBroadcastsInSecs = param;
    m_memberToInitialisedFlagMap[&m_keepTerminatedBroadcastsInSecs] = true;
    LOG_DEBUG("setKeepTerminatedBroadcastsInSecs(): %s", nvps(m_keepTerminatedBroadcastsInSecs));
}

void CachedConfig::setMaxBroadcasts(unsigned long param)
{
    ThreadGuard guard(m_lockForConfigUpdate);
    m_maxBroadcasts = param;
    m_memberToInitialisedFlagMap[&m_maxBroadcasts] = true;
    LOG_DEBUG("setMaxBroadcasts(): %s", nvps(m_maxBroadcasts));
}

void CachedConfig::setMajorSummaryAlarmDP(TA_Base_Bus::DataPoint* dp)
{
    TA_ASSERT(dp, "dp is NULL");
    ThreadGuard guard(m_lockForConfigUpdate);
    m_majorSummaryAlarmDP = dp;
    m_memberToInitialisedFlagMap[&m_majorSummaryAlarmDP] = true;
    LOG_DEBUG("setMajorSummaryAlarmDP(): %s", nvps(m_majorSummaryAlarmDP->getDataPointName()));
}

void CachedConfig::setMinorSummaryAlarmDP(TA_Base_Bus::DataPoint* dp)
{
    TA_ASSERT(dp, "dp is NULL");
    ThreadGuard guard(m_lockForConfigUpdate);
    m_minorSummaryAlarmDP = dp;
    m_memberToInitialisedFlagMap[&m_minorSummaryAlarmDP] = true;
    LOG_DEBUG("setMinorSummaryAlarmDP(): %s", nvps(m_minorSummaryAlarmDP->getDataPointName()));
}

void CachedConfig::setFireCountdownActiveDP(TA_Base_Bus::DataPoint* dp)
{
    TA_ASSERT(dp, "dp is NULL");
    ThreadGuard guard(m_lockForConfigUpdate);
    m_fireCountdownActiveDP = dp;
    m_memberToInitialisedFlagMap[&m_fireCountdownActiveDP] = true;
    LOG_DEBUG("setFireCountdownActiveDP(): %s", nvps(m_fireCountdownActiveDP->getDataPointName()));
}

void CachedConfig::setFireCountdownActiveOutputDP(TA_Base_Bus::DataPoint* dp)
{
    TA_ASSERT(dp, "dp is NULL");
    ThreadGuard guard(m_lockForConfigUpdate);
    m_fireCountdownActiveOutputDP = dp;
    m_memberToInitialisedFlagMap[&m_fireCountdownActiveOutputDP] = true;
    LOG_DEBUG("setFireCountdownActiveOutputDP(): %s", nvps(m_fireCountdownActiveOutputDP->getDataPointName()));
}

void CachedConfig::setTrainDvaMessageLibraryVersion(unsigned long param)
{
    ThreadGuard guard(m_lockForConfigUpdate);
    m_trainDvaMessageLibraryVersion = param;
    m_memberToInitialisedFlagMap[&m_trainDvaMessageLibraryVersion] = true;
    LOG_DEBUG("setTrainDvaMessageLibraryVersion(): %s", nvps(m_trainDvaMessageLibraryVersion));
}

void CachedConfig::setPagingConsoleKeys(const std::list<unsigned long>& pagingConsoleKeyList)
{
    ThreadGuard guard(m_lockForConfigUpdate);
    m_pagingConsoleKeys = pagingConsoleKeyList;
    m_memberToInitialisedFlagMap[&m_pagingConsoleKeys] = true;
    LOG_DEBUG("setPagingConsoleKeys(): %s", nvps(m_pagingConsoleKeys));
}

// ++ Bug456
void CachedConfig::setDvaBroadcastTimeoutInMsecs(unsigned long timeout)
{
    ThreadGuard guard(m_lockForConfigUpdate);
    m_dvaBroadcastTimeoutInMsecs = timeout;
    m_memberToInitialisedFlagMap[&m_dvaBroadcastTimeoutInMsecs] = true;
    LOG_DEBUG("setDvaBroadcastTimeoutInMsecs(): %s", nvps(m_dvaBroadcastTimeoutInMsecs));
}

// ++ Bug456

unsigned long CachedConfig::getAgentKey()
{
    ThreadGuard guard(m_lockForConfigUpdate);
    TA_ASSERT(m_memberToInitialisedFlagMap[&m_agentKey], "m_agentKey not set");

    return m_agentKey;
}

//TD17452
unsigned long CachedConfig::getMonitorProcessKey()
{
    ThreadGuard guard(m_lockForConfigUpdate);
    TA_ASSERT(m_memberToInitialisedFlagMap[&m_monitorProcessKey], "m_monitorProcessKey not set");

    return m_monitorProcessKey;
}

//TD17452

std::string CachedConfig::getAgentName()
{
    ThreadGuard guard(m_lockForConfigUpdate);
    TA_ASSERT(m_memberToInitialisedFlagMap[&m_agentName], "m_agentName not set");

    if (m_agentName.size() == 0)
    {
        // Not set
        m_agentName = RunParams::getInstance().get(RPARAM_ENTITYNAME);
    }

    return m_agentName;
}

std::string CachedConfig::getAgentAssetName()
{
    ThreadGuard guard(m_lockForConfigUpdate);
    TA_ASSERT(m_memberToInitialisedFlagMap[&m_assetName], "m_assetName not set");
    return m_assetName;
}

unsigned long CachedConfig::getAgentTypeKey()
{
    ThreadGuard guard(m_lockForConfigUpdate);
    TA_ASSERT(m_memberToInitialisedFlagMap[&m_agentTypeKey], "m_agentTypeKey not set");

    return m_agentTypeKey;
}

unsigned long CachedConfig::getAgentLocationKey()
{
    ThreadGuard guard(m_lockForConfigUpdate);
    TA_ASSERT(m_memberToInitialisedFlagMap[&m_agentLocationKey], "m_agentLocationKey not set");

    return m_agentLocationKey;
}

unsigned long CachedConfig::getAgentSubsystemKey()
{
    ThreadGuard guard(m_lockForConfigUpdate);
    TA_ASSERT(m_memberToInitialisedFlagMap[&m_agentSubsystemKey], "m_agentSubsystemKey not set");

    return m_agentSubsystemKey;
}

unsigned long CachedConfig::getLocalATSAgentKey()
{
    ThreadGuard guard(m_lockForConfigUpdate);
    TA_ASSERT(m_memberToInitialisedFlagMap[&m_localATSAgentKey], "m_localATSAgentKey not set");

    return m_localATSAgentKey;
}

unsigned long CachedConfig::getLocalTrainAgentKey()
{
    ThreadGuard guard(m_lockForConfigUpdate);
    TA_ASSERT(m_memberToInitialisedFlagMap[&m_localTrainAgentKey], "m_localTrainAgentKey not set");

    return m_localTrainAgentKey;
}

unsigned long CachedConfig::getFastPollRateInMSecs()
{
    //Acceptable threadsafety - see notes above
    //ThreadGuard guard( m_lockForFastPoll );
    TA_ASSERT(m_memberToInitialisedFlagMap[&m_fastPollRateInMSecs], "m_fastPollRateInMSecs not set");

    return m_fastPollRateInMSecs;
}

unsigned long CachedConfig::getWorkerThreadsSleepTimeInMSecs()
{
    //Acceptable threadsafety - see notes above
    //ThreadGuard guard( m_lockForWorkerThreads );
    TA_ASSERT(m_memberToInitialisedFlagMap[&m_workerThreadsSleepTimeInMSecs], "m_workerThreadsSleepTimeInMSecs not set");

    return m_workerThreadsSleepTimeInMSecs;
}

unsigned long CachedConfig::getSocketTimeoutInSecs()
{
    ThreadGuard guard(m_lockForConfigUpdate);
    TA_ASSERT(m_memberToInitialisedFlagMap[&m_socketTimeoutInSecs], "m_socketTimeoutInSecs not set");

    return m_socketTimeoutInSecs;
}

unsigned long CachedConfig::getRetries()
{
    ThreadGuard guard(m_lockForConfigUpdate);
    TA_ASSERT(m_memberToInitialisedFlagMap[&m_retries], "m_retries not set");

    return m_retries;
}

std::string CachedConfig::getPrimaryFEPCAddress()
{
    ThreadGuard guard(m_lockForConfigUpdate);
    TA_ASSERT(m_memberToInitialisedFlagMap[&m_primaryFEPCAddress], "m_primaryFEPCAddress not set");

    return m_primaryFEPCAddress;
}

std::string CachedConfig::getSecondaryFEPCAddress()
{
    ThreadGuard guard(m_lockForConfigUpdate);
    TA_ASSERT(m_memberToInitialisedFlagMap[&m_secondaryFEPCAddress], "m_secondaryFEPCAddress not set");

    return m_secondaryFEPCAddress;
}

std::string CachedConfig::getPrimaryFEPCPort()
{
    ThreadGuard guard(m_lockForConfigUpdate);
    TA_ASSERT(m_memberToInitialisedFlagMap[&m_primaryFEPCPort], "m_primaryFEPCPort not set");

    return m_primaryFEPCPort;
}

std::string CachedConfig::getSecondaryFEPCPort()
{
    ThreadGuard guard(m_lockForConfigUpdate);
    TA_ASSERT(m_memberToInitialisedFlagMap[&m_secondaryFEPCPort], "m_secondaryFEPCPort not set");

    return m_secondaryFEPCPort;
}

bool CachedConfig::getIsMaster()
{
    //Implicitly threadsafe: ThreadGuard guard( m_lockForConfigUpdate );
    TA_ASSERT(m_memberToInitialisedFlagMap[&m_isMaster], "m_isMaster not set");

    return m_isMaster;
}

bool CachedConfig::getIsDepot()
{
    //Implicitly threadsafe: ThreadGuard guard( m_lockForConfigUpdate );
    TA_ASSERT(m_memberToInitialisedFlagMap[&m_isDepot], "m_isDepot not set");

    return m_isDepot;
}

std::string CachedConfig::getLocalTrainAgentEntityName()
{
    ThreadGuard guard(m_lockForConfigUpdate);
    TA_ASSERT(m_memberToInitialisedFlagMap[&m_localTrainAgentEntityName], "m_localTrainAgentEntityName not set");

    return m_localTrainAgentEntityName;
}

bool CachedConfig::getIsLocalATSAgentConfigured()
{
    ThreadGuard guard(m_lockForConfigUpdate);

    // We test against the location key because it is faster compared to string comparisons.
    return m_memberToInitialisedFlagMap[&m_localATSAgentKey];
}

unsigned long CachedConfig::getTisDisplayDurationInSecs()
{
    ThreadGuard guard(m_lockForConfigUpdate);
    TA_ASSERT(m_memberToInitialisedFlagMap[&m_tisDisplayDuration], "m_tisDisplayDuration not set");

    return m_tisDisplayDuration;
}

unsigned long CachedConfig::getKeepTerminatedBroadcastsInSecs()
{
    ThreadGuard guard(m_lockForConfigUpdate);
    TA_ASSERT(m_memberToInitialisedFlagMap[&m_keepTerminatedBroadcastsInSecs], "m_keepTerminatedBroadcastsInSecs not set");

    return m_keepTerminatedBroadcastsInSecs;
}

unsigned long CachedConfig::getMaxBroadcasts()
{
    ThreadGuard guard(m_lockForConfigUpdate);
    TA_ASSERT(m_memberToInitialisedFlagMap[&m_maxBroadcasts], "m_maxBroadcasts not set");

    return m_maxBroadcasts;
}

TA_Base_Bus::DataPoint* CachedConfig::getMajorSummaryAlarmDP()
{
    ThreadGuard guard(m_lockForConfigUpdate);
    TA_ASSERT(m_memberToInitialisedFlagMap[&m_majorSummaryAlarmDP], "m_majorSummaryAlarmDP not set");

    return m_majorSummaryAlarmDP;
}

TA_Base_Bus::DataPoint* CachedConfig::getMinorSummaryAlarmDP()
{
    ThreadGuard guard(m_lockForConfigUpdate);
    TA_ASSERT(m_memberToInitialisedFlagMap[&m_minorSummaryAlarmDP], "m_minorSummaryAlarmDP not set");

    return m_minorSummaryAlarmDP;
}

TA_Base_Bus::DataPoint* CachedConfig::getFireCountdownActiveDP()
{
    ThreadGuard guard(m_lockForConfigUpdate);
    TA_ASSERT(m_memberToInitialisedFlagMap[&m_fireCountdownActiveDP], "m_fireCountdownActiveDP not set");

    return m_fireCountdownActiveDP;
}

TA_Base_Bus::DataPoint* CachedConfig::getFireCountdownActiveOutputDP()
{
    ThreadGuard guard(m_lockForConfigUpdate);
    TA_ASSERT(m_memberToInitialisedFlagMap[&m_fireCountdownActiveOutputDP], "m_fireCountdownActiveOutputDP not set");

    return m_fireCountdownActiveOutputDP;
}

unsigned long CachedConfig::getTrainDvaMessageLibraryVersion()
{
    ThreadGuard guard(m_lockForConfigUpdate);
    TA_ASSERT(m_memberToInitialisedFlagMap[&m_trainDvaMessageLibraryVersion], "m_trainDvaMessageLibraryVersion not set");

    return m_trainDvaMessageLibraryVersion;
}

std::list<unsigned long> CachedConfig::getPagingConsoleKeys()
{
    ThreadGuard guard(m_lockForConfigUpdate);
    TA_ASSERT(m_memberToInitialisedFlagMap[&m_pagingConsoleKeys], "m_pagingConsoleKeys not set");

    return m_pagingConsoleKeys;
}

// ++ Bug456
unsigned long CachedConfig::getDvaBroadcastTimeoutInMsecs()
{
    ThreadGuard guard(m_lockForConfigUpdate);
    TA_ASSERT(m_memberToInitialisedFlagMap[&m_dvaBroadcastTimeoutInMsecs], "m_dvaBroadcastTimeoutInMsecs not set");

    return m_dvaBroadcastTimeoutInMsecs;
}
