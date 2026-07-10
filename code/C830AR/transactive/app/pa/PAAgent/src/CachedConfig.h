/**
 * The source code in this file is the property of
 * Ripple Systems and is not for redistribution
 * in any form.
 *
 * Source:   $File: //depot/C830AR_TIP/C830AR/transactive/app/pa/PAAgent/src/CachedConfig.h $
 * @author:  Ripple
 * @version: $Revision: #6 $
 *
 * Last modification: $DateTime: 2024/12/10 11:18:50 $
 * Last modified by:  $Author: limin.zhu $
 *
 */
// CachedConfig.h: interface for the CachedConfig class.
//
//////////////////////////////////////////////////////////////////////

#pragma once
#include <string>
#include <map>
#include "core/exceptions/src/PasExceptions.h"
#include "core/synchronisation/src/NonReEntrantThreadLockable.h"
#include "core/naming/src/NamedObject.h"

class ICachedConfigObserver;
namespace TA_Base_Bus
{
    class DataPoint;
}

class CachedConfig
{
public:

    // Observable descriptors

    typedef enum
    {
        CONFIG_PRIMARY_FEPC_ADDRESS,
        CONFIG_SECONDARY_FEPC_ADDRESS,
        CONFIG_PRIMARY_FEPC_PORT,
        CONFIG_SECONDARY_FEPC_PORT,
        CONFIG_SOCKET_TIMEOUT
    } ECachedConfigItemKey;

    /**
     * getInstance
     *
     * Returns an instance of the class
     *
     * @return the pointer to this singleton instance
     *
     */
    static CachedConfig* getInstance();

    /**
     * removeInstance
     *
     * Removes the instance of the class (if already created) and cleanup the members.  Primarily
     * used upon program termination (e.g. from main()) so that Purify does not consider this class
     * and its members to be leaks.
     *
     */
    static void removeInstance();

    void setAgentKey(unsigned long param);
    // Removed setAgentName() as this is now taken from RunParams
    void setAgentTypeKey(unsigned long param);
    void setAgentLocationKey(unsigned long param); //throw(TA_Base_Core::InvalidPasConfigurationException);
    void setAgentSubsystemKey(unsigned long param);
    void setAgentInRuntime(bool param);
    void setAgentAssetName(std::string param);
    //TD17452
    void setMonitorProcessKey(unsigned long param);

    //TD17452

    void setLocalATSAgentKey(unsigned long param); //throw(TA_Base_Core::InvalidPasConfigurationException);
    void setLocalTrainAgentKey(unsigned long param); //throw(TA_Base_Core::InvalidPasConfigurationException);
    void setFastPollRateInMSecs(unsigned long param);
    void setWorkerThreadsSleepTimeInMSecs(unsigned long param);
    void setSocketTimeoutInSecs(unsigned long param);
    void setRetries(unsigned long param);
    void setPrimaryFEPCAddress(const std::string& param); //throw(TA_Base_Core::InvalidPasConfigurationException);
    void setSecondaryFEPCAddress(const std::string& param); //throw(TA_Base_Core::InvalidPasConfigurationException);

    // Converts from unsigned long to string
    void setPrimaryFEPCPort(unsigned long param); //throw(TA_Base_Core::InvalidPasConfigurationException);
    void setSecondaryFEPCPort(unsigned long param); //throw(TA_Base_Core::InvalidPasConfigurationException);

    void setIsMaster(bool param);
    void setIsDepot(bool param);

    // Set to null if the agent is unavailable
    void setLocalTrainAgentEntityName(const std::string& param);

    void setTisDisplayDurationInSecs(unsigned long param);
    void setKeepTerminatedBroadcastsInSecs(unsigned long param);
    void setMaxBroadcasts(unsigned long param);

    void setMajorSummaryAlarmDP(TA_Base_Bus::DataPoint* dp);
    void setMinorSummaryAlarmDP(TA_Base_Bus::DataPoint* dp);
    void setFireCountdownActiveDP(TA_Base_Bus::DataPoint* dp);
    void setFireCountdownActiveOutputDP(TA_Base_Bus::DataPoint* dp);

    void setTrainDvaMessageLibraryVersion(unsigned long param);

    void setPagingConsoleKeys(const std::list<unsigned long>& pagingConsoleKeyList);
    // ++ Bug456
    void setDvaBroadcastTimeoutInMsecs(unsigned long timeout);
    // ++ Bug456

    // The standard entity details below should not assert / //throw since we may need to raise
    // alarms with half-complete configuration.
    unsigned long       getAgentKey();
    std::string         getAgentName();
    unsigned long       getAgentTypeKey();
    unsigned long       getAgentLocationKey();
    unsigned long       getAgentSubsystemKey();
    bool                getAgentInRuntime();
    std::string         getAgentAssetName();

    // These may assert / //throw.
    unsigned long       getLocalATSAgentKey();
    unsigned long       getLocalTrainAgentKey();
    unsigned long       getFastPollRateInMSecs();
    unsigned long       getWorkerThreadsSleepTimeInMSecs();
    unsigned long       getSocketTimeoutInSecs();
    unsigned long       getRetries();
    // ++ Bug456
    unsigned long         getDvaBroadcastTimeoutInMsecs();
    // ++ Bug456
    //TD17452
    unsigned long       getMonitorProcessKey();
    //TD17452

    std::string         getPrimaryFEPCAddress();
    std::string         getSecondaryFEPCAddress();
    std::string         getPrimaryFEPCPort();
    std::string         getSecondaryFEPCPort();

    bool                getIsMaster();
    bool                getIsDepot();

    std::string         getLocalTrainAgentEntityName();

    bool                getIsLocalATSAgentConfigured();
    unsigned long       getTisDisplayDurationInSecs();
    unsigned long       getKeepTerminatedBroadcastsInSecs();
    unsigned long       getMaxBroadcasts();

    TA_Base_Bus::DataPoint* getMajorSummaryAlarmDP();
    TA_Base_Bus::DataPoint* getMinorSummaryAlarmDP();
    TA_Base_Bus::DataPoint* getFireCountdownActiveDP();
    TA_Base_Bus::DataPoint* getFireCountdownActiveOutputDP();

    unsigned long       getTrainDvaMessageLibraryVersion();

    std::list<unsigned long>    getPagingConsoleKeys();

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
    void attachObserver(ICachedConfigObserver* observer, ECachedConfigItemKey key);

    /**
     * detachObserver
     *
     * Removes all instances of the ICachedConfigObserver pointer in the observer collection.  The
     * observer will be removed for all keys.
     *
     */
    void detachObserver(ICachedConfigObserver* observer);

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
    void notifyInterestedObservers(ECachedConfigItemKey key);

private:

    CachedConfig();
    virtual ~CachedConfig();

    static CachedConfig* m_me;

    static TA_Base_Core::NonReEntrantThreadLockable m_lockForConfigUpdate;
    //static TA_Base_Core::NonReEntrantThreadLockable m_lockForFastPoll;
    //static TA_Base_Core::NonReEntrantThreadLockable m_lockForWorkerThreads;
    static TA_Base_Core::NonReEntrantThreadLockable m_lockForObserverMap;

    // global parameters
    unsigned long               m_agentKey;
    std::string                 m_agentName;
    unsigned long               m_agentTypeKey;
    unsigned long               m_agentLocationKey;
    unsigned long               m_agentSubsystemKey;
    bool                        m_agentInRuntime;

    std::string                 m_assetName;
    //TD17452
    unsigned long               m_monitorProcessKey;

    // StationPAAgent and MasterPAAgent parameters
    unsigned long               m_localATSAgentKey;
    unsigned long               m_localTrainAgentKey;
    unsigned long               m_fastPollRateInMSecs;
    unsigned long               m_workerThreadsSleepTimeInMSecs;
    unsigned long               m_socketTimeoutInSecs;
    unsigned long               m_retries;
    std::string                 m_primaryFEPCAddress;
    std::string                 m_secondaryFEPCAddress;
    std::string                 m_primaryFEPCPort;
    std::string                 m_secondaryFEPCPort;

    bool                        m_isMaster;
    bool                        m_isDepot;

    std::string                 m_localTrainAgentEntityName;
    unsigned long               m_tisDisplayDuration;
    unsigned long               m_keepTerminatedBroadcastsInSecs;
    unsigned long               m_maxBroadcasts;

    TA_Base_Bus::DataPoint* m_majorSummaryAlarmDP;
    TA_Base_Bus::DataPoint* m_minorSummaryAlarmDP;
    TA_Base_Bus::DataPoint* m_fireCountdownActiveDP;
    TA_Base_Bus::DataPoint* m_fireCountdownActiveOutputDP;

    unsigned long               m_trainDvaMessageLibraryVersion;

    // Paging console keys
    std::list<unsigned long>    m_pagingConsoleKeys;

    // Observer maps

    typedef std::multimap<ECachedConfigItemKey, ICachedConfigObserver*>  KeyToObserversMap;
    typedef KeyToObserversMap::iterator                                  KeyToObserversMapIt;
    typedef KeyToObserversMap::value_type                                key2observer;

    KeyToObserversMap   m_keyToObserversMap;

    // Initialised Member map

    typedef std::map<void*, bool>                                       MemberPointerToBoolMap;

    MemberPointerToBoolMap m_memberToInitialisedFlagMap;

    // ++ Bug456
    unsigned long                 m_dvaBroadcastTimeoutInMsecs;
    // ++ Bug456
};

class ICachedConfigObserver
{
public:

    virtual void processCachedConfigUpdate(CachedConfig::ECachedConfigItemKey key) = 0;
};
