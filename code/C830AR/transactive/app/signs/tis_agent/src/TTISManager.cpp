/**
 * The source code in this file is the property of
 * Ripple Systems and is not for redistribution in any form.
 *
 * Source:    $File: //depot/C830AR_TIP/C830AR/transactive/app/signs/tis_agent/src/TTISManager.cpp $
 * @author:   Robin Ashcroft
 * @version:  $Revision: #5 $
 *
 * Last modification: $DateTime: 2025/07/23 11:19:19 $
 * Last modified by:  $Author: jonathan $
 *
 * Manages all Train Traveller Information System-related functionality
 *
 * Implements the ITTISManagerCorbaDef CORBA interface
 *
 */

#ifdef _MSC_VER
#pragma warning( disable : 4786 )
#pragma warning( disable : 4503 )
#endif // _MSC_VER

#include "TTISManager.h"

#include "app/signs/tis_agent/src/AgentModeMonitor.h"
#include "app/signs/tis_agent/src/StatusMonitor.h"
#include "app/signs/tis_agent/src/StringHelper.h"
#include "app/signs/tis_agent/src/TISMessageDataDefinitionsHelper.h"
#include "app/signs/tis_agent/src/TTISAdHocDisplayRequest.h"
#include "app/signs/tis_agent/src/TTISApprovedRATISClearRequest.h"
#include "app/signs/tis_agent/src/TTISApprovedRATISDisplayRequest.h"
#include "app/signs/tis_agent/src/TTISPredefinedDisplayRequest.h"

#include "bus/signs_4669/tis_agent_access/src/TISAgentAccessFactory.h"
#include "bus/trains/TrainAgentCorba/idl/src/ITrainExceptionsCorba.h" // CL-21314
#include "bus/trains/TrainCommonLibrary/src/TrainAccessDeniedException.h"
#include "bus/trains/TrainCommonLibrary/src/AgentCommunicationException.h"
#include "bus/trains/TrainCommonLibrary/src/InvalidParameterException.h"
#include "bus/trains/TrainCommonLibrary/src/LocationCache.h"
#include "bus/trains/TrainCorbaProxy/src/TrainAgentProxyFactory.h"
#include "bus/generic_agent/src/GenericAgent.h"

#include "core/alarm/src/AlarmHelper.h"
#include "core/alarm/src/AlarmHelperManager.h"
#include "core/alarm/src/NonUniqueAlarmHelper.h"
#include "core/data_access_interface/entity_access/src/EntityAccessFactory.h"
#include "core/data_access_interface/entity_access/src/TTISEntityData.h"
#include "core/data_access_interface/entity_access/src/TrainAgentEntityData.h"
#include "core/data_access_interface/src/LocationAccessFactory.h"
#include "core/data_access_interface/src/SessionAccessFactory.h"
#include "core/data_access_interface/tis_agent/src/DisplayRequestAccessFactory.h"
#include "core/data_access_interface/tis_agent/src/IDisplayRequest.h"
#include "core/data_access_interface/tis_agent/src/TrainTimeScheduleAccessFactory.h"
#include "core/data_access_interface/tis_agent/src/TrainTimeScheduleData.h"
#include "core/data_access_interface/tis_agent_4669/IDL/src/TISMessageDataDefinitions.h"
#include "core/data_access_interface/tis_agent_4669/IDL/src/TISMessageDataDefinitionsDescribe.h"
#include "core/data_access_interface/tis_agent_4669/src/PredefinedMessageLibraryAccessFactory.h"
#include "core/data_access_interface/tis_agent_4669/src/PredefinedMessageLibraryTable.h"
#include "core/exceptions/src/DataException.h"
#include "core/exceptions/src/DatabaseException.h"
#include "core/exceptions/src/TransactiveException.h"
#include "core/message/src/AuditMessageSender.h"
#include "core/message/src/MessagePublicationManager.h"
#include "core/message/src/MessageSubscriptionManager.h"
#include "core/message/types/TISAlarms_MessageTypes.h"
#include "core/message/types/TISAudit_MessageTypes.h"
#include "core/message/types/TISBroadcastComms_MessageTypes.h"
#include "core/message/types/TISComms_MessageTypes.h"
#include "core/message/types/TISStateUpdate_MessageTypes.h"
#include "core/naming/src/NamedObject.h"
#include "core/utilities/src/RunParams.h"
#include "core/threads/src/ThreadMemberFunction.h"

#include <ace/OS.h>

#include <iomanip>
#include <string>
#include <vector>

using TA_Base_Core::DatabaseException;
using TA_Base_Core::DataException;
using TA_Base_Core::MessageSubscriptionManager;
using TA_Base_Core::EntityAccessFactory;
using TA_Base_Core::LocationAccessFactory;
using TA_Base_Core::ILocation;
using TA_Base_Bus::TISAgentAccessFactory;

#define PRIORITY_AUDIT_MESSAGE_PARAMETER_NAME "Priority"
#define STARTTIME_AUDIT_MESSAGE_PARAMETER_NAME "Start time"
#define ENDTIME_AUDIT_MESSAGE_PARAMETER_NAME "End time"
#define OVERRIDE_AUDIT_MESSAGE_PARAMETER_NAME "Override"
#define REPEAT_INTERVAL_AUDIT_MESSAGE_PARAMETER_NAME "Repeat interval"
#define DISPLAY_ATTRIBUTE_AUDIT_MESSAGE_PARAMETER_NAME "Display attribute"

namespace
{
    static const char CR = 0xD;
    static const char LF = 0xA;
}

namespace TA_IRS_App
{
    using TA_Base_Core::DebugUtil;

    //
    // Initialise statics.
    //

    TA_Base_Core::ReEntrantThreadLockable TTISManager::m_singletonLock;
    TTISManager* TTISManager::m_theClassInstance = NULL;

    TTISManager::TTISManager()
        : m_trainDataLock(),
        m_trainDownloadsLock(),
        m_locationKey(0),
        m_locationName(),
        m_entityName(),
        m_entityKey(0),
        m_subsystemKey(0),  // TD9178 ~ added
        m_otherAgentLocationName(""),
        m_trainDownloadList(),
        m_trainDataVersions(),
        m_trainAgentSubscriber(NULL),
        m_auditMessageSender(NULL), //TES831 Events not logged!
        m_tisMessageSender(NULL)
    {
        FUNCTION_ENTRY("TTISManager");

        m_tisMessageSender = TA_Base_Core::MessagePublicationManager::getInstance().getCommsMessageSender(TA_Base_Core::TISComms::Context);

        m_tisBroadcastMessageSender = TA_Base_Core::MessagePublicationManager::getInstance().getCommsMessageSender(TA_Base_Core::TISBroadcastComms::Context);

        m_auditMessageSender = TA_Base_Core::MessagePublicationManager::getInstance().getAuditMessageSender(TA_Base_Core::TISAudit::Context);

        m_tisStateUpdateMessageSender = TA_Base_Core::MessagePublicationManager::getInstance().getStateUpdateMessageSender(TA_Base_Core::TISStateUpdate::Context);

        FUNCTION_EXIT;
    }

    TTISManager::~TTISManager()
    {
        FUNCTION_ENTRY("~TTISManager");

        TA_Base_Core::MessageSubscriptionManager::getInstance().unsubscribeToMessages(
            static_cast<TA_Base_Core::SpecialisedMessageSubscriber<TA_Base_Core::CommsMessageCorbaDef>*>(this));

        TA_Base_Core::MessageSubscriptionManager::getInstance().unsubscribeToMessages(
            static_cast<TA_Base_Core::SpecialisedMessageSubscriber<TA_Base_Core::StateUpdateMessageCorbaDef>*>(this));

        deactivateServant();

        delete m_trainAgentSubscriber;
        m_trainAgentSubscriber = NULL;

        delete m_tisMessageSender;
        m_tisMessageSender = NULL;

        delete m_tisBroadcastMessageSender;
        m_tisBroadcastMessageSender = NULL;

        delete m_auditMessageSender;
        m_auditMessageSender = NULL;

        delete m_tisStateUpdateMessageSender;
        m_tisStateUpdateMessageSender = NULL;

        FUNCTION_EXIT;
    }

    TTISManager* TTISManager::getInstance()
    {
        FUNCTION_ENTRY("getInstance");

        TA_Base_Core::ThreadGuard guard(m_singletonLock);

        // If guard acquired then make sure the singleton is still NULL.

        if (NULL == m_theClassInstance)
        {
            // Create the one & only object.

            m_theClassInstance = new TTISManager();
        }

        FUNCTION_EXIT;
        return m_theClassInstance;
    }

    void TTISManager::initialise(TA_Base_Core::IEntityDataPtr EntityData)
    {
        LOG_SCOPE("TTISManager::initialise");
        FUNCTION_ENTRY("initialise");

        TA_IRS_Bus::TrainAgentProxyFactory::getInstance().agentInitialise(EntityData);

        m_trainTisCorbaProxy = TA_IRS_Bus::TrainAgentProxyFactory::getInstance().createTrainTisCorbaProxy();

        m_entityName = EntityData->getName();
        m_locationKey = EntityData->getLocation();
        m_locationName = EntityData->getLocationName();
        m_entityKey = EntityData->getKey();
        m_subsystemKey = EntityData->getSubsystem();
        m_entityTypeKey = EntityData->getTypeKey();

        try
        {
            m_locationType = TA_IRS_Bus::LocationCache::getInstance().getLocationTypeByKey(m_locationKey);
        }
        catch (...)
        {
            LOG_ERROR("Error retrieving location type from database");

            // do a name based check
            if (m_locationName == "OCC")
            {
                m_locationType = TA_Base_Core::ILocation::OCC;
            }
            else if (m_locationName == "BOCC")
            {
                m_locationType = TA_Base_Core::ILocation::BOCC;
            }
            else
            {
                m_locationType = TA_Base_Core::ILocation::STATION;
            }
        }

        // this is the OCC if this is the depot agent, and the depot if the occ agent
        if (TA_Base_Core::ILocation::OCC == m_locationType || TA_Base_Core::ILocation::BOCC == m_locationType)
        {
            unsigned long depotLocationKey = TA_IRS_Bus::LocationCache::getInstance().getDepotLocationKey();
            m_otherAgentLocationName = TA_IRS_Bus::LocationCache::getInstance().getLocationByKey(depotLocationKey);
        }
        else if (TA_Base_Core::ILocation::DPT == m_locationType)
        {
            unsigned long occLocationKey = TA_IRS_Bus::LocationCache::getInstance().getOccLocationKey();
            m_otherAgentLocationName = TA_IRS_Bus::LocationCache::getInstance().getLocationByKey(occLocationKey);
        }

        m_trainAgentSubscriber = new TrainAgentSubscriber(*this);

        //getInitialTrainDataVersions();
        //Init train data versions
        try
        {
            TA_IRS_Bus::TtisTypes::TisLibraryVersionList allVersions = m_trainTisCorbaProxy->getLibraryVersions();

            for (TA_IRS_Bus::TtisTypes::TisLibraryVersionList::iterator iter = allVersions.begin();
                 allVersions.end() != iter; ++iter)
            {
                m_trainDataVersions.updateTTISLibraryVersions(iter->trainId, iter->libraryVersion, iter->nextlibraryVersion);
                m_trainDataVersions.updateScheduleVersions(iter->trainId, iter->scheduleVersion, iter->nextscheduleVersion);
            }
        }
        catch (TA_IRS_Bus::AgentCommunicationException& e)
        {
            LOG_EXCEPTION_CATCH(SourceInfo, "TA_IRS_Bus::AgentCommunicationException", e.what());
        }

        getInitialTrainDownloads();

        getInitialTrainDataVersionAlarms();

        activateServantWithName(m_entityName);

        FUNCTION_EXIT;
    }

    bool TTISManager::isValid()
    {
        FUNCTION_ENTRY("isValid");
        FUNCTION_EXIT;
        return true;
    }

    void TTISManager::update(const TA_Base_Core::ConfigUpdateDetails& configUpdateDetails)
    {
        FUNCTION_ENTRY("update");
        FUNCTION_EXIT;
    }

    void TTISManager::remove()
    {
        FUNCTION_ENTRY("remove");
        FUNCTION_EXIT;
    }

    void TTISManager::start()
    {
        FUNCTION_ENTRY("start");
        FUNCTION_EXIT;
    }

    void TTISManager::stop()
    {
        FUNCTION_ENTRY("stop");
        FUNCTION_EXIT;
    }

    void TTISManager::registerForStateUpdates()
    {
        FUNCTION_ENTRY("TTISManager::registerForStateUpdates");

        subscribeToNotifications(false);

        FUNCTION_EXIT;
    }

    void TTISManager::setToControlMode()
    {
        FUNCTION_ENTRY("setToControlMode");

        if (m_entityName != "")
        {
            activateServantWithName(m_entityName);
        }

        //limin++--, for failover
        typedef TA_Base_Core::ThreadedMemberFunction< TTISManager > TTISManagerMemberFunction;
        TTISManagerMemberFunction& thrd = TTISManagerMemberFunction::alloc(*this);
        thrd.add(&TTISManager::setToControlModeInThread);
        thrd.start();
        //limin++--, for failover

        FUNCTION_EXIT;
    }

    //limin, for failover
    void TTISManager::setToControlModeInThread()
    {
        FUNCTION_ENTRY("TTISManager::setToControlModeInThread");

        subscribeToNotifications(true);

        // simply recheck train data versions
        verifyAllTrainDataVersions();

        FUNCTION_EXIT;
    }

    void TTISManager::setToMonitorMode()
    {
        FUNCTION_ENTRY("setToMonitorMode");
        deactivateServant();

        subscribeToNotifications(false);

        FUNCTION_EXIT;
    }

    unsigned short TTISManager::getFlexibleTimeOutValue()
    {
        FUNCTION_ENTRY("getFlexibleTimeOutValue");
        FUNCTION_EXIT;
        return  static_cast<unsigned short>(m_trainTisCorbaProxy->getMaximumWaitTime());
    }

    TA_Base_Core::TrainDownloadList* TTISManager::getCurrentTrainDownloads()
    {
        FUNCTION_ENTRY("getCurrentTrainDownloads");

        TA_Base_Bus::GenericAgent::ensureControlMode();
        TA_Base_Core::TrainDownloadList* trainDownloadSeq = new TA_Base_Core::TrainDownloadList();

        // Set the CORBA sequence's length before populating it
        trainDownloadSeq->length(m_trainDownloadList.size());

        unsigned int i;
        std::map< unsigned char, TA_Base_Core::TrainDownload >::iterator iter;

        for (iter = m_trainDownloadList.begin(), i = 0;
             (m_trainDownloadList.end() != iter) && (i < trainDownloadSeq->length());
             ++i, ++iter)
        {
            (*trainDownloadSeq)[i] = iter->second;
        }

        FUNCTION_EXIT;
        return trainDownloadSeq;
    }

    TA_Base_Core::TrainDataVersionList* TTISManager::getTrainDataVersions()
    {
        FUNCTION_ENTRY("getTrainDataVersions");

        TA_Base_Bus::GenericAgent::ensureControlMode();
        std::vector<TA_Base_Core::TrainDataVersion> dataVersions =
            this->m_trainDataVersions.getTrainDataVersions();
        TA_Base_Core::TrainDataVersionList* trainDataVersionSeq;

        // Set the CORBA sequence's length before populating it
        trainDataVersionSeq = new TA_Base_Core::TrainDataVersionList();

        trainDataVersionSeq->length(dataVersions.size());

        unsigned int i;

        for (i = 0; i < dataVersions.size(); i++)
        {
            (*trainDataVersionSeq)[i].trainNumber = dataVersions[i].trainNumber;
            (*trainDataVersionSeq)[i].predefinedLibraryVersion = dataVersions[i].predefinedLibraryVersion;
            (*trainDataVersionSeq)[i].nextPredefinedLibraryVersion = dataVersions[i].nextPredefinedLibraryVersion;
            (*trainDataVersionSeq)[i].trainTimeScheduleVersion = dataVersions[i].trainTimeScheduleVersion;
            (*trainDataVersionSeq)[i].nextTrainTimeScheduleVersion = dataVersions[i].nextTrainTimeScheduleVersion;
        }

        FUNCTION_EXIT;
        return trainDataVersionSeq;
    }

    TA_Base_Core::TrainDataVersionAlarmList* TTISManager::getTrainDataAlarms()
    {
        FUNCTION_ENTRY("getTrainDataAlarms");

        TA_Base_Bus::GenericAgent::ensureControlMode();

        TA_Base_Core::TrainDataVersionAlarmList* alarmList = new TA_Base_Core::TrainDataVersionAlarmList();

        alarmList->length(m_trainDataVersionAlarmList.size());

        unsigned int i = 0;

        for (std::map< unsigned char, TA_Base_Core::TrainDataVersionAlarms >::iterator iter = m_trainDataVersionAlarmList.begin();
             m_trainDataVersionAlarmList.end() != iter; ++iter, ++i)
        {
            (*alarmList)[i] = iter->second;
        }

        FUNCTION_EXIT;
        return alarmList;
    }

    void TTISManager::timeScheduleChanged(unsigned long timeSchedulePkey, TA_Base_Core::ETimeScheduleChangeType changeType, const char* sessionID)
    {
        FUNCTION_ENTRY("timeScheduleChanged");

        // send the comms message
        TA_ASSERT(m_tisMessageSender != NULL, "Tis Message Sender is null");

        TA_Base_Bus::GenericAgent::ensureControlMode();

        LOG_INFO("Sending TisTrainTimeScheduleChange message.");

        // put the data into the message
        TA_Base_Core::TimeScheduleChange change;
        change.timeSchedulePkey = timeSchedulePkey;
        change.changeType = changeType;

        CORBA::Any data;
        data <<= change;

        m_tisMessageSender->sendCommsMessage(TA_Base_Core::TISComms::TisTrainTimeScheduleChange,
                                             m_entityKey, data, m_subsystemKey, m_locationKey, NULL);

        TA_IRS_Core::ITrainTimeSchedule* pITrainTimeSchedule = TA_IRS_Core::TrainTimeScheduleAccessFactory::getInstance().getTrainTimeSchedule(timeSchedulePkey);

        std::string timeScheduleName = pITrainTimeSchedule->getName();

        delete pITrainTimeSchedule;

        TA_Base_Core::DescriptionParameters desc;
        TA_Base_Core::NameValuePair name("name", timeScheduleName);

        desc.push_back(&name);

        switch (changeType)
        {
            case TA_Base_Core::Added:
                m_auditMessageSender->sendAuditMessage(TA_Base_Core::TISAudit::TTISTimeScheduleCreated, m_entityKey,
                                                       desc,
                                                       "", // Further description text
                                                       sessionID,
                                                       "", // alarm ID - not required
                                                       "", // incident key - not required
                                                       ""); // event key - not required
                break;

            case TA_Base_Core::Deleted:
                m_auditMessageSender->sendAuditMessage(TA_Base_Core::TISAudit::TTISTimeScheduleDeleted, m_entityKey,
                                                       desc,
                                                       "", // Further description text
                                                       sessionID,
                                                       "", // alarm ID - not required
                                                       "", // incident key - not required
                                                       ""); // event key - not required
                break;

            case TA_Base_Core::Modified:
                m_auditMessageSender->sendAuditMessage(TA_Base_Core::TISAudit::TTISTimeScheduleModified, m_entityKey,
                                                       desc,
                                                       "", // Further description text
                                                       sessionID,
                                                       "", // alarm ID - not required
                                                       "", // incident key - not required
                                                       ""); // event key - not required
                break;

            default:
                break;
        }

        FUNCTION_EXIT;
    }

    void TTISManager::submitPredefinedDisplayRequest(const TA_Base_Core::TrainList& trains,
                                                     const TA_Base_Core::TTISPredefinedMessageParameters& displayParameters,
                                                     long timestamp,
                                                     const char* sessionId)
    {
        FUNCTION_ENTRY("submitPredefinedDisplayRequest");

        TA_Base_Bus::GenericAgent::ensureControlMode();
        LOG_INFO("Received client request to send TTIS Predefined display request");

        // check to see if the TTIS manager has the same library version as the agent
        unsigned short currentLibraryVersion = StatusMonitor::getInstance()->getCurrentISCSTrainLibraryVersion();

        if (currentLibraryVersion != displayParameters.libraryVersion)
        {
            LOG_ERROR("Client has different library version (%d) to agent (%d)", displayParameters.libraryVersion, currentLibraryVersion);
            //[[CL-17931
            std::string msg;

            for (unsigned int j = 0; j < trains.length(); j++)
            {
                if (currentLibraryVersion != this->m_trainDataVersions.getTrainDataVersion(trains[j]).predefinedLibraryVersion)
                {
                    TA_IRS_Bus::TrainInformationTypes::TrainDetails trainsData;

                    try
                    {
                        trainsData = m_trainAgentSubscriber->getTrainDetails(trains[j]);
                        int trainId = trainsData.trainId;
                        std::stringstream msgstr;
                        msgstr << "Train " << trainId << "/" << trainsData.serviceNumber << " has different library version to agent " << std::endl;
                        msg += msgstr.str();
                    }
                    catch (const TA_Base_Core::TransactiveException& err)
                    {
                        LOG_EXCEPTION_CATCH(SourceInfo, "TransactiveException", err.what());
                        LOG_INFO("TransactiveException while attempting to retrieve the train list or send a clear request");
                    }
                    catch (...)
                    {
                        LOG_EXCEPTION_CATCH(SourceInfo, "TransactiveException", "Unknown Exception.");
                    }
                }
            } //end for

            if (false == msg.empty())
            {
                LOG_INFO(msg);
                throw TA_Base_Bus::ITTISManagerCorbaDef::TTISInvalidParameter(msg.c_str());
            }

            //]]
            throw TA_Base_Bus::ITTISManagerCorbaDef::TTISInvalidParameter("Library versions different");
        }

        // Note there no need to validate the version of each individual train
        // TIMS does that for us, and it will return the appropriate error if it doesnt match

        try
        {
            m_trainTisCorbaProxy->displayStoredMessage(convertTrainList(trains),
                                                       convertDisplayParameters(displayParameters),
                                                       timestamp,
                                                       sessionId);
        }
        catch (const TA_IRS_Bus::AgentCommunicationException& e)
        {
            LOG_EXCEPTION_CATCH(SourceInfo, "TA_IRS_Bus::AgentCommunicationException", e.what());

            FUNCTION_EXIT;
            throw TA_Base_Bus::ITTISManagerCorbaDef::TTISGeneralFailure("Error contacting train agent");
        }
        catch (const TA_IRS_Bus::TrainAccessDeniedException& e)
        {
            LOG_EXCEPTION_CATCH(SourceInfo, "TA_IRS_Bus::TrainAccessDeniedException", e.what());

            FUNCTION_EXIT;
            throw TA_Base_Core::AccessDeniedException(e.what());
        }
        catch (const TA_IRS_Bus::InvalidParameterException& e)
        {
            LOG_EXCEPTION_CATCH(SourceInfo, "TA_IRS_Bus::InvalidParameterException", e.what());

            FUNCTION_EXIT;
            throw TA_Base_Bus::ITTISManagerCorbaDef::TTISInvalidParameter(e.what());
        }
        catch (const TA_Base_Core::TransactiveException& e)
        {
            LOG_EXCEPTION_CATCH(SourceInfo, "TA_Base_Core::TransactiveException", e.what());

            FUNCTION_EXIT;
            throw TA_Base_Bus::ITTISManagerCorbaDef::TTISGeneralFailure(e.what());
        }

        FUNCTION_EXIT;
    }

    void TTISManager::submitAdHocDisplayRequest(const TA_Base_Core::TrainList& trains,
                                                const TA_Base_Core::TTISFreeTextMessageParameters& displayParameters,
                                                long timestamp,
                                                const char* sessionId)
    {
        FUNCTION_ENTRY("submitAdHocDisplayRequest");

        TA_Base_Bus::GenericAgent::ensureControlMode();
        LOG_INFO("Received client request to send TTIS Adhoc display request");

        try
        {
            m_trainTisCorbaProxy->displayFreeTextMessage(convertTrainList(trains),
                                                         convertDisplayParameters(displayParameters),
                                                         timestamp,
                                                         sessionId);
        }
        catch (const TA_IRS_Bus::AgentCommunicationException& e)
        {
            LOG_EXCEPTION_CATCH(SourceInfo, "TA_IRS_Bus::AgentCommunicationException", e.what());

            FUNCTION_EXIT;
            throw TA_Base_Bus::ITTISManagerCorbaDef::TTISGeneralFailure("Error contacting train agent");
        }
        catch (const TA_IRS_Bus::TrainAccessDeniedException& e)
        {
            LOG_EXCEPTION_CATCH(SourceInfo, "TA_IRS_Bus::TrainAccessDeniedException", e.what());

            FUNCTION_EXIT;
            throw TA_Base_Core::AccessDeniedException(e.what());
        }
        catch (const TA_IRS_Bus::InvalidParameterException& e)
        {
            LOG_EXCEPTION_CATCH(SourceInfo, "TA_IRS_Bus::InvalidParameterException", e.what());

            FUNCTION_EXIT;
            throw TA_Base_Bus::ITTISManagerCorbaDef::TTISInvalidParameter(e.what());
        }
        catch (const TA_Base_Core::TransactiveException& e)
        {
            LOG_EXCEPTION_CATCH(SourceInfo, "TA_Base_Core::TransactiveException", e.what());

            FUNCTION_EXIT;
            throw TA_Base_Bus::ITTISManagerCorbaDef::TTISGeneralFailure(e.what());
        }

        FUNCTION_EXIT;
    }

    void TTISManager::submitClearRequest(const TA_Base_Core::TrainList& trains,
                                         const TA_Base_Core::TTISMessageResetParameters& displayParameters,
                                         CORBA::Long timestamp,
                                         const char* sessionId)
    {
        FUNCTION_ENTRY("submitClearRequest");

        TA_Base_Bus::GenericAgent::ensureControlMode();
        LOG_INFO("Received client request to send TTIS Clear request to trains");

        try
        {
            m_trainTisCorbaProxy->resetMessage(convertTrainList(trains),
                                               convertDisplayParameters(displayParameters),
                                               timestamp,
                                               sessionId);
        }
        catch (const TA_IRS_Bus::AgentCommunicationException& e)
        {
            LOG_EXCEPTION_CATCH(SourceInfo, "TA_IRS_Bus::AgentCommunicationException", e.what());

            FUNCTION_EXIT;
            throw TA_Base_Bus::ITTISManagerCorbaDef::TTISGeneralFailure("Error contacting train agent");
        }
        catch (const TA_IRS_Bus::TrainAccessDeniedException& e)
        {
            LOG_EXCEPTION_CATCH(SourceInfo, "TA_IRS_Bus::TrainAccessDeniedException", e.what());

            FUNCTION_EXIT;
            throw TA_Base_Core::AccessDeniedException(e.what());
        }
        catch (const TA_IRS_Bus::InvalidParameterException& e)
        {
            LOG_EXCEPTION_CATCH(SourceInfo, "TA_IRS_Bus::InvalidParameterException", e.what());

            FUNCTION_EXIT;
            throw TA_Base_Bus::ITTISManagerCorbaDef::TTISInvalidParameter(e.what());
        }
        catch (const TA_Base_Core::TransactiveException& e)
        {
            LOG_EXCEPTION_CATCH(SourceInfo, "TA_Base_Core::TransactiveException", e.what());

            FUNCTION_EXIT;
            throw TA_Base_Bus::ITTISManagerCorbaDef::TTISGeneralFailure(e.what());
        }

        FUNCTION_EXIT;
    }

    void TTISManager::downloadNextMessageLibrary(const TA_Base_Core::TrainList& trains,
                                                 const char* sessionId)
    {
        FUNCTION_ENTRY("downloadNextMessageLibrary");

        TA_Base_Bus::GenericAgent::ensureControlMode();

        LOG_INFO("Received client request to download next TTIS message library to a train");

        int nextMessageLibraryVersion = StatusMonitor::getInstance()->getNextISCSTrainLibraryVersion();

        TA_Base_Core::IPredefinedMessageLibrary* nextPredefinedMessageLibrary = NULL;

        // retrieve latest version from the database
        try
        {
            nextPredefinedMessageLibrary =
                TA_Base_Core::PredefinedMessageLibraryAccessFactory::getInstance().getPredefinedMessageLibrary(nextMessageLibraryVersion,
                                                                                                               TA_Base_Core::TA_TISAgentDAI::LIBRARY_TYPE_TTIS);
        }
        catch (TA_Base_Core::TransactiveException& e)
        {
            LOG_EXCEPTION_CATCH(SourceInfo, "TA_Base_Core::TransactiveException", e.what());

            throw TA_Base_Bus::ITTISManagerCorbaDef::TTISGeneralFailure("Error loading message library from ISCS database");
        }
        catch (...)
        {
            LOG_EXCEPTION_CATCH(SourceInfo, "...", "While attempting to retrieve the current Predefined Message Library from the Database");

            throw TA_Base_Bus::ITTISManagerCorbaDef::TTISGeneralFailure("Error loading message library from ISCS database");
        }

        // Get the raw library file content
        TA_IRS_Bus::TtisTypes::LibraryDataType dbFileContent;
        nextPredefinedMessageLibrary->getLibraryFileContent(dbFileContent);

		LOG_GENERIC(SourceInfo, TA_Base_Core::DebugUtil::DebugDebug,
			"LibraryData size is %ld", dbFileContent.size());

        try
        {
            m_trainTisCorbaProxy->libraryDownload(convertTrainList(trains),
                                                  dbFileContent,
                                                  nextPredefinedMessageLibrary->getVersion(),
                                                  sessionId);
        }
        catch (const TA_IRS_Bus::AgentCommunicationException& e)
        {
            LOG_EXCEPTION_CATCH(SourceInfo, "TA_IRS_Bus::AgentCommunicationException", e.what());

            FUNCTION_EXIT;
            throw TA_Base_Bus::ITTISManagerCorbaDef::TTISGeneralFailure("Error contacting train agent");
        }
        catch (const TA_IRS_Bus::TrainAccessDeniedException& e)
        {
            LOG_EXCEPTION_CATCH(SourceInfo, "TA_IRS_Bus::TrainAccessDeniedException", e.what());

            FUNCTION_EXIT;
            throw TA_Base_Core::AccessDeniedException(e.what());
        }
        catch (const TA_IRS_Bus::InvalidParameterException& e)
        {
            LOG_EXCEPTION_CATCH(SourceInfo, "TA_IRS_Bus::InvalidParameterException", e.what());

            FUNCTION_EXIT;
            throw TA_Base_Bus::ITTISManagerCorbaDef::TTISInvalidParameter(e.what());
        }

        delete nextPredefinedMessageLibrary;
        nextPredefinedMessageLibrary = NULL;

        FUNCTION_EXIT;
    }

    void TTISManager::upgradeMessageLibrary(const TA_Base_Core::TrainList& trains,
                                            const char* sessionId)
    {
        FUNCTION_ENTRY("upgradeMessageLibrary");

        TA_Base_Bus::GenericAgent::ensureControlMode();

        LOG_INFO("Received client request to upgrade the TTIS message library on a train");

        try
        {
            m_trainTisCorbaProxy->libraryUpgrade(convertTrainList(trains), sessionId);
        }
        catch (const TA_IRS_Bus::AgentCommunicationException& e)
        {
            LOG_EXCEPTION_CATCH(SourceInfo, "TA_IRS_Bus::AgentCommunicationException", e.what());

            FUNCTION_EXIT;
            throw TA_Base_Bus::ITTISManagerCorbaDef::TTISGeneralFailure("Error contacting train agent");
        }
        catch (const TA_IRS_Bus::TrainAccessDeniedException& e)
        {
            LOG_EXCEPTION_CATCH(SourceInfo, "TA_IRS_Bus::TrainAccessDeniedException", e.what());

            FUNCTION_EXIT;
            throw TA_Base_Core::AccessDeniedException(e.what());
        }
        catch (const TA_IRS_Bus::InvalidParameterException& e)
        {
            LOG_EXCEPTION_CATCH(SourceInfo, "TA_IRS_Bus::InvalidParameterException", e.what());

            FUNCTION_EXIT;
            throw TA_Base_Bus::ITTISManagerCorbaDef::TTISInvalidParameter(e.what());
        }

        FUNCTION_EXIT;
    }

    void TTISManager::downloadCurrentTimeSchedule(const TA_Base_Core::TrainList& trains,
                                                  const char* sessionId)
    {
        FUNCTION_ENTRY("downloadCurrentTimeSchedule");

        TA_Base_Bus::GenericAgent::ensureControlMode();

        LOG_INFO("Received client request to download the current TTIS Train time schedule");

        try
        {
            TA_Base_Core::TimeScheduleVersion timeSchedule;
            CORBA_CALL_RETURN(timeSchedule, TISAgentAccessFactory::getInstance().getOccSTISAgent(), getCurrentTrainTimeScheduleVersion, ());

            TA_IRS_Bus::TtisTypes::LibraryDataType scheduleData = loadCurrentTimeSchedule(timeSchedule.timeScheduleKey);

            m_trainTisCorbaProxy->scheduleDownload(convertTrainList(trains),
                                                   scheduleData,
                                                   timeSchedule.version,
                                                   sessionId);
        }
        catch (const TA_IRS_Bus::AgentCommunicationException& e)
        {
            LOG_EXCEPTION_CATCH(SourceInfo, "TA_IRS_Bus::AgentCommunicationException", e.what());

            FUNCTION_EXIT;
            throw TA_Base_Bus::ITTISManagerCorbaDef::TTISGeneralFailure("Error contacting train agent");
        }
        catch (const TA_IRS_Bus::TrainAccessDeniedException& e)
        {
            LOG_EXCEPTION_CATCH(SourceInfo, "TA_IRS_Bus::TrainAccessDeniedException", e.what());

            FUNCTION_EXIT;
            throw TA_Base_Core::AccessDeniedException(e.what());
        }
        catch (const TA_IRS_Bus::InvalidParameterException& e)
        {
            LOG_EXCEPTION_CATCH(SourceInfo, "TA_IRS_Bus::InvalidParameterException", e.what());

            FUNCTION_EXIT;
            throw TA_Base_Bus::ITTISManagerCorbaDef::TTISInvalidParameter(e.what());
        }
        catch (const TA_Base_Core::TransactiveException& e)
        {
            LOG_EXCEPTION_CATCH(SourceInfo, "TA_Base_Core::TransactiveException", e.what());

            FUNCTION_EXIT;
            throw TA_Base_Bus::ITTISManagerCorbaDef::TTISGeneralFailure(e.what());
        }

        FUNCTION_EXIT;
    }

    void TTISManager::upgradeTimeSchedule(const TA_Base_Core::TrainList& trains,
                                          const char* sessionId)
    {
        FUNCTION_ENTRY("upgradeTimeSchedule");

        TA_Base_Bus::GenericAgent::ensureControlMode();

        LOG_INFO("Received client request to upgrade the TTIS message library on a train");

        try
        {
            m_trainTisCorbaProxy->scheduleUpgrade(convertTrainList(trains), sessionId);
        }
        catch (const TA_IRS_Bus::AgentCommunicationException& e)
        {
            LOG_EXCEPTION_CATCH(SourceInfo, "TA_IRS_Bus::AgentCommunicationException", e.what());

            FUNCTION_EXIT;
            throw TA_Base_Bus::ITTISManagerCorbaDef::TTISGeneralFailure("Error contacting train agent");
        }
        catch (const TA_IRS_Bus::TrainAccessDeniedException& e)
        {
            LOG_EXCEPTION_CATCH(SourceInfo, "TA_IRS_Bus::TrainAccessDeniedException", e.what());

            FUNCTION_EXIT;
            throw TA_Base_Core::AccessDeniedException(e.what());
        }
        catch (const TA_IRS_Bus::InvalidParameterException& e)
        {
            LOG_EXCEPTION_CATCH(SourceInfo, "TA_IRS_Bus::InvalidParameterException", e.what());

            FUNCTION_EXIT;
            throw TA_Base_Bus::ITTISManagerCorbaDef::TTISInvalidParameter(e.what());
        }

        FUNCTION_EXIT;
    }

    void TTISManager::submitRatisDisplayRequest(TA_IRS_Core::IRATISMessage* message,
                                                const std::string& sessionId)
    {
        FUNCTION_ENTRY("submitRatisDisplayRequest");

        // first determine where this request is destined for
        TA_IRS_Bus::CommonTypes::TrainIdList trains;

        // for all (valid) trains
        if (true == message->isForAllTrains())
        {
            TA_IRS_Bus::TrainInformationTypes::TrainDetailsMap allTrains = m_trainAgentSubscriber->getTrainList();

            // this must get all valid mainline trains
            // because this is only applicable for the OCC TIS agent
            // a train is valid if it is communicating with this agent
            // or if ATS tells us it is somewhere on the mainline (no need to check location or localisation validity)
            for (TA_IRS_Bus::TrainInformationTypes::TrainDetailsMap::iterator iter = allTrains.begin();
                 iter != allTrains.end(); ++iter)
            {
                if ((true == iter->second.atsValid) || // in OCC ATS
                    ((true == iter->second.communicationValid) &&
                     (m_locationKey == iter->second.communicatingLocation))) // communicating with this agent
                {
                    trains.insert(iter->second.trainId);
                }
            }
        }
        // for a specific set of train(s)
        else
        {
            std::vector< unsigned char > destinationTrainIds = message->getDestinationIds();

            for (std::vector< unsigned char >::iterator iter = destinationTrainIds.begin();
                 iter != destinationTrainIds.end(); ++iter)
            {
                trains.insert(*iter);
            }
        }

        // second get the display parameters

        TA_IRS_Bus::TtisTypes::TtisFreeTextDisplayParameters displayParameters;
        displayParameters.pidList.push_back(TA_IRS_Bus::ProtocolTtisTypes::TtisPidAllTrain);
        const std::string ratisMessageText = message->getMessageText();
        displayParameters.message.assign(ratisMessageText.begin(), ratisMessageText.end());
        displayParameters.priority = convertTisPriorityToTrain(message->getPriority());

        std::string startTime = message->getStartTime().substr(8, 4); // HHMM
        std::string endTime = message->getEndTime().substr(8, 4); // HHMM
        displayParameters.displayPeriod = convertTisTimeIntervalToTrain(startTime, endTime);

        // the default TTIS attributes are used for these parameters
        TA_Base_Core::TTISLedAttributes defaultAttributes = StatusMonitor::getInstance()->getDefaultTTISLedAttributes();

        displayParameters.fontSize = convertTisFontSizeToTrain(defaultAttributes.latinFontSize);
        displayParameters.justification = convertTisJustificationToTrain(defaultAttributes.justification);
        displayParameters.intensity = convertTisIntensityToTrain(defaultAttributes.intensity);
        displayParameters.displayMode = convertTisDisplayModeToTrain(defaultAttributes.displayMode);
        displayParameters.repeatInterval = static_cast<unsigned char> (defaultAttributes.repeatInterval);

        // third get the request time (now)
        time_t timestamp = ACE_OS::time(NULL);

        // fourth if this is an automatic approval then maybe there is no session ID
        std::string operatorName;

        std::string sessionIdToUse = sessionId;

        if (true == sessionIdToUse.empty())
        {
            // use the system session ID
            sessionIdToUse = TA_Base_Core::SessionAccessFactory::getSuperSessionId();

            // An operator name of "RXH" shouild be logged instead of "Not available"
            operatorName = "RXH";
        }
        else
        {
            operatorName = STISManager::getInstance()->getOperatorNameFromSession(sessionId);
        }

        LOG_INFO("Received client request to send TTIS RATIS Adhoc display request");

        try
        {
            TTISApprovedRATISDisplayRequestPtr displayRequest(new TTISApprovedRATISDisplayRequest(trains,
                                                                                                  message->getTag(),
                                                                                                  message->getType(),
                                                                                                  message->getDestination(),
                                                                                                  operatorName,
                                                                                                  sessionIdToUse,
                                                                                                  timestamp));

            // add the request to the list of requests in progress
            {
                TA_THREADGUARD(m_ratisDisplayLock);
                m_ratisDisplayInProgress.push_back(displayRequest);
            }

            // execute the command
            m_trainTisCorbaProxy->displayFreeTextMessage(trains,
                                                         displayParameters,
                                                         timestamp,
                                                         sessionIdToUse);
            // wait for the response, and get the failed trains
            TA_IRS_Bus::CommonTypes::TrainIdList failedTrains = displayRequest->waitForCompletion(m_trainTisCorbaProxy->getMaximumWaitTime() * 1000);

            // remove the request from the list
            {
                TA_THREADGUARD(m_ratisDisplayLock);

                for (TTISApprovedRATISDisplayRequestList::iterator iter = m_ratisDisplayInProgress.begin();
                     iter != m_ratisDisplayInProgress.end(); ++iter)
                {
                    if ((*iter) == displayRequest)
                    {
                        m_ratisDisplayInProgress.erase(iter);
                        // exit the loop
                        break;
                    }
                }
            }

            // raise the alarm if necessary
            if (false == failedTrains.empty())
            {
                raiseRatisDisplayRequestAlarm(failedTrains);
            }
        }
        catch (const TA_IRS_Bus::AgentCommunicationException& e)
        {
            LOG_EXCEPTION_CATCH(SourceInfo, "TA_IRS_Bus::AgentCommunicationException", e.what());

            FUNCTION_EXIT;
            throw TA_Base_Bus::ITTISManagerCorbaDef::TTISGeneralFailure("Error contacting train agent");
        }
        catch (const TA_IRS_Bus::TrainAccessDeniedException& e)
        {
            LOG_EXCEPTION_CATCH(SourceInfo, "TA_IRS_Bus::TrainAccessDeniedException", e.what());

            FUNCTION_EXIT;
            throw TA_Base_Core::AccessDeniedException(e.what());
        }
        catch (const TA_IRS_Bus::InvalidParameterException& e)
        {
            LOG_EXCEPTION_CATCH(SourceInfo, "TA_IRS_Bus::InvalidParameterException", e.what());

            FUNCTION_EXIT;
            throw TA_Base_Bus::ITTISManagerCorbaDef::TTISInvalidParameter(e.what());
        }
        catch (const TA_Base_Core::TransactiveException& e)
        {
            LOG_EXCEPTION_CATCH(SourceInfo, "TA_Base_Core::TransactiveException", e.what());

            FUNCTION_EXIT;
            throw TA_Base_Bus::ITTISManagerCorbaDef::TTISGeneralFailure(e.what());
        }

        FUNCTION_EXIT;
    }

    void TTISManager::submitRatisClearRequest(TA_IRS_Core::IRATISMessage* message, const std::string& sessionId)
    {
        FUNCTION_ENTRY("submitRatisClearRequest");

        // first determine where this request is destined for
        TA_IRS_Bus::CommonTypes::TrainIdList trains;

        // for all (valid) trains
        if (true == message->isForAllTrains())
        {
            TA_IRS_Bus::TrainInformationTypes::TrainDetailsMap allTrains = m_trainAgentSubscriber->getTrainList();

            // this must get all valid mainline trains
            // because this is only applicable for the OCC TIS agent
            // a train is valid if it is communicating with this agent
            // or if ATS tells us it is somewhere on the mainline (no need to check location or localisation validity)
            for (TA_IRS_Bus::TrainInformationTypes::TrainDetailsMap::iterator iter = allTrains.begin();
                 iter != allTrains.end(); ++iter)
            {
                if ((true == iter->second.atsValid) || // in OCC ATS
                    ((true == iter->second.communicationValid) &&
                     (m_locationKey == iter->second.communicatingLocation))) // communicating with this agent
                {
                    trains.insert(iter->second.trainId);
                }
            }
        }
        // for a specific set of train(s)
        else
        {
            std::vector< unsigned char > destinationTrainIds = message->getDestinationIds();

            for (std::vector< unsigned char >::iterator iter = destinationTrainIds.begin();
                 iter != destinationTrainIds.end(); ++iter)
            {
                trains.insert(*iter);
            }
        }

        // second get the display parameters
        TA_IRS_Bus::TtisTypes::TtisResetParameters displayParameters;
        displayParameters.pidList.push_back(TA_IRS_Bus::ProtocolTtisTypes::TtisPidAllTrain);
        displayParameters.messageType = convertTisClearTypeToTrain(getClearTypeFromPriority(static_cast<short>(message->getPriority())));

        // third get the request time (now)
        time_t timestamp = ACE_OS::time(NULL);

        // fourth if this is an automatic approval then maybe there is no session ID
        std::string operatorName;

        std::string sessionIdToUse = sessionId;

        if (true == sessionIdToUse.empty())
        {
            // use the system session ID
            sessionIdToUse = TA_Base_Core::SessionAccessFactory::getSuperSessionId();

            // An operator name of "RXH" shouild be logged instead of "Not available"
            operatorName = "RXH";
        }
        else
        {
            operatorName = STISManager::getInstance()->getOperatorNameFromSession(sessionId);
        }

        LOG_INFO("Received client request to send TTIS RATIS Clear request to trains");

        try
        {
            TTISApprovedRATISClearRequestPtr displayRequest(new TTISApprovedRATISClearRequest(trains,
                                                                                              message->getTag(),
                                                                                              message->getType(),
                                                                                              message->getPriority(),
                                                                                              message->getStartTime(),
                                                                                              message->getEndTime(),
                                                                                              message->getDestination(),
                                                                                              operatorName,
                                                                                              sessionIdToUse,
                                                                                              timestamp));

            // add the request to the list of requests in progress
            {
                TA_THREADGUARD(m_ratisClearLock);
                m_ratisClearInProgress.push_back(displayRequest);
            }

            // execute the command
            m_trainTisCorbaProxy->resetMessage(trains,
                                               displayParameters,
                                               timestamp,
                                               sessionIdToUse.c_str());

            // wait for the response, and get the failed trains
            TA_IRS_Bus::CommonTypes::TrainIdList failedTrains = displayRequest->waitForCompletion(m_trainTisCorbaProxy->getMaximumWaitTime() * 1000);

            // remove the request from the list
            {
                TA_THREADGUARD(m_ratisClearLock);

                for (TTISApprovedRATISClearRequestList::iterator iter = m_ratisClearInProgress.begin();
                     iter != m_ratisClearInProgress.end(); ++iter)
                {
                    if ((*iter) == displayRequest)
                    {
                        m_ratisClearInProgress.erase(iter);
                        // exit the loop
                        break;
                    }
                }
            }

            // raise the alarm if necessary
            if (false == failedTrains.empty())
            {
                raiseRatisDisplayRequestAlarm(failedTrains);
            }
        }
        catch (const TA_IRS_Bus::AgentCommunicationException& e)
        {
            LOG_EXCEPTION_CATCH(SourceInfo, "TA_IRS_Bus::AgentCommunicationException", e.what());

            FUNCTION_EXIT;
            throw TA_Base_Bus::ITTISManagerCorbaDef::TTISGeneralFailure("Error contacting train agent");
        }
        catch (const TA_IRS_Bus::TrainAccessDeniedException& e)
        {
            LOG_EXCEPTION_CATCH(SourceInfo, "TA_IRS_Bus::TrainAccessDeniedException", e.what());

            FUNCTION_EXIT;
            throw TA_Base_Core::AccessDeniedException(e.what());
        }
        catch (const TA_IRS_Bus::InvalidParameterException& e)
        {
            LOG_EXCEPTION_CATCH(SourceInfo, "TA_IRS_Bus::InvalidParameterException", e.what());

            FUNCTION_EXIT;
            throw TA_Base_Bus::ITTISManagerCorbaDef::TTISInvalidParameter(e.what());
        }
        catch (const TA_Base_Core::TransactiveException& e)
        {
            LOG_EXCEPTION_CATCH(SourceInfo, "TA_Base_Core::TransactiveException", e.what());

            FUNCTION_EXIT;
            throw TA_Base_Bus::ITTISManagerCorbaDef::TTISGeneralFailure(e.what());
        }

        FUNCTION_EXIT;
    }

    void TTISManager::receiveSpecialisedMessage(const TA_Base_Core::CommsMessageCorbaDef& message)
    {
        FUNCTION_ENTRY("receiveSpecialisedMessage");

        if (TA_Base_Core::TISComms::TisTrainDataVersionUpdate.getTypeKey() == message.messageTypeKey.in())
        {
            TA_Base_Core::TrainDataVersion* versionChange = NULL;

            if (message.messageState >>= versionChange)
            {
                LOG_INFO("received version update: [train id=%d][current version=%d][next version=%d][current schedule=%d][next schedule=%d]",
                         versionChange->trainNumber, versionChange->predefinedLibraryVersion, versionChange->nextPredefinedLibraryVersion,
                         versionChange->trainTimeScheduleVersion, versionChange->nextTrainTimeScheduleVersion);

                m_trainDataVersions.updateScheduleVersions(versionChange->trainNumber,
                                                           versionChange->trainTimeScheduleVersion,
                                                           versionChange->nextTrainTimeScheduleVersion);

                m_trainDataVersions.updateTTISLibraryVersions(versionChange->trainNumber,
                                                              versionChange->predefinedLibraryVersion,
                                                              versionChange->nextPredefinedLibraryVersion);
            }
            else
            {
                LOG_ERROR("Could not extract TrainDataVersion from CommsMessageCorbaDef");
            }
        }

        if (TA_Base_Core::TISComms::TisTrainDownloadUpdate.getTypeKey() == message.messageTypeKey.in())
        {
            // this is an update from the peer agent
            // or a message that we have just sent to ourself.
            // so, if in control mode we know its a self message and it shouldnt be processed
            if (true == AgentModeMonitor::getInstance().isInControlMode())
            {
                FUNCTION_EXIT;
                return;
            }

            TA_Base_Core::TrainDownloadStatus* downloadChange = NULL;

            if (message.messageState >>= downloadChange)
            {
                TA_THREADGUARD(m_trainDownloadsLock);
                // find the item for this train
                std::map< unsigned char, TA_Base_Core::TrainDownload >::iterator findIter = m_trainDownloadList.find(downloadChange->trainNumber);

                if (findIter == m_trainDownloadList.end())
                {
                    // insert an item for this train
                    TA_Base_Core::TrainDownload downloadData;
                    downloadData.trainNumber = downloadChange->trainNumber;
                    downloadData.predefinedDownloadInProgress = false;
                    downloadData.timeScheduleDownloadInProgress = false;

                    m_trainDownloadList.insert(std::map< unsigned char, TA_Base_Core::TrainDownload >::value_type(downloadChange->trainNumber, downloadData));
                }

                // take a reference to the new or existing item
                TA_Base_Core::TrainDownload& existingDownload = m_trainDownloadList[downloadChange->trainNumber];

                switch (downloadChange->type)
                {
                    case TA_Base_Core::LibraryDownloadStart:
                        existingDownload.predefinedDownloadInProgress = downloadChange->success;
                        break;

                    case TA_Base_Core::ScheduleDownloadStart:
                        existingDownload.timeScheduleDownloadInProgress = downloadChange->success;
                        break;

                    case TA_Base_Core::LibraryDownloadFinish:
                        existingDownload.predefinedDownloadInProgress = false;
                        break;

                    case TA_Base_Core::ScheduleDownloadFinish:
                        existingDownload.timeScheduleDownloadInProgress = false;
                        break;

                    case TA_Base_Core::LibraryUpgrade:
                    case TA_Base_Core::ScheduleUpgrade:
                        // no action
                        break;

                    default:
                        TA_ASSERT(false, "Invalid update type");
                        break;
                }
            }
            else
            {
                LOG_ERROR("Could not extract TrainDownloadStatus from CommsMessageCorbaDef");
            }
        }

        FUNCTION_EXIT;
    }

    void TTISManager::receiveSpecialisedMessage(const TA_Base_Core::StateUpdateMessageCorbaDef& message)
    {
        FUNCTION_ENTRY("receiveSpecialisedMessage");

        // only process in monitor mode
        if (true == AgentModeMonitor::getInstance().isInControlMode())
        {
            FUNCTION_EXIT;
            return;
        }

        if (TA_Base_Core::TISStateUpdate::TisTrainDataVersionAlarm.getTypeKey() == message.messageTypeKey.in())
        {
            TA_Base_Core::TrainDataVersionAlarms* alarmUpdate = NULL;

            if (message.stateUpdateInfo >>= alarmUpdate)
            {
                TA_THREADGUARD(m_trainDataLock);

                m_trainDataVersionAlarmList[alarmUpdate->trainNumber] = *alarmUpdate;
            }
            else
            {
                LOG_ERROR("Could not extract TrainDataVersionAlarms from StateUpdateMessageCorbaDef");
            }
        }

        FUNCTION_EXIT;
    }

    void TTISManager::updateTrainLibraryVersions(TA_IRS_Bus::CommonTypes::TrainIdType trainId, unsigned short libraryVersion,
                                                 unsigned short nextLibraryVersion, unsigned short scheduleVersion, unsigned short nextScheduleVersion)
    {
        FUNCTION_ENTRY("updateTrainLibraryVersions");

        // update the library and schedule versions with the new version from the train agent
        // this is an update from the train agent, it means that its from a wakeup or upgrade
        // so anything in the next version buffer is now gone

        LOG_INFO("Updating library %d , next predefined library %d, and schedule version %d, next schedule version %d for train: %d",
                 libraryVersion, nextLibraryVersion, scheduleVersion, nextScheduleVersion, trainId);

        TA_Base_Core::TrainDataVersion data;

        try
        {
            data = m_trainDataVersions.getTrainDataVersion(trainId);
        }
        catch (TA_IRS_Bus::TrainDoesNotExistException&)
        {
            data.trainNumber = trainId;
            data.predefinedLibraryVersion = 0;
            data.nextPredefinedLibraryVersion = 0;
            data.trainTimeScheduleVersion = 0;
            data.nextTrainTimeScheduleVersion = 0;
        }

        //hongzhi, CL-21514, refine 40143 messages
        if ((data.predefinedLibraryVersion != libraryVersion) || (data.trainTimeScheduleVersion != scheduleVersion)
            || (data.nextPredefinedLibraryVersion != nextLibraryVersion) || (data.nextTrainTimeScheduleVersion != nextScheduleVersion))
        {
            if ((data.predefinedLibraryVersion != libraryVersion) || (data.nextPredefinedLibraryVersion != nextLibraryVersion))
            {
                m_trainDataVersions.updateTTISLibraryVersions(trainId, libraryVersion, nextLibraryVersion);
            }

            if ((data.trainTimeScheduleVersion != scheduleVersion) || (data.nextTrainTimeScheduleVersion != nextScheduleVersion))
            {
                m_trainDataVersions.updateScheduleVersions(trainId, scheduleVersion, nextScheduleVersion);
            }

            data = m_trainDataVersions.getTrainDataVersion(trainId);
            sendVersionUpdateMessage(data);

            // now check the versions dont mismatch between the master versions
            verifyTrainDataVersions(trainId);
        }

        FUNCTION_EXIT;
    }

    void TTISManager::trainDownloadStarted(TA_IRS_Bus::CommonTypes::TrainIdType trainId, ETrainLibraryType downloadType, unsigned short version)
    {
        FUNCTION_ENTRY("trainDownloadStarted");

        TA_THREADGUARD(m_trainDownloadsLock);

        std::map< unsigned char, TA_Base_Core::TrainDownload >::iterator findIter = m_trainDownloadList.find(trainId);

        if (findIter == m_trainDownloadList.end())
        {
            // insert an item for this train
            TA_Base_Core::TrainDownload downloadData;
            downloadData.trainNumber = trainId;
            downloadData.predefinedDownloadInProgress = false;
            downloadData.timeScheduleDownloadInProgress = false;

            m_trainDownloadList.insert(std::map< unsigned char, TA_Base_Core::TrainDownload >::value_type(trainId, downloadData));
        }

        // take a reference to the new or existing item
        TA_Base_Core::TrainDownload& existingDownload = m_trainDownloadList[trainId];

        TA_Base_Core::TrainDownloadStatus update;
        update.trainNumber = trainId;
        update.success = true;
        update.errorDetails = CORBA::string_dup("In Progress");

        switch (downloadType)
        {
            case ITrainEventCallback::MessageLibrary:
                existingDownload.predefinedDownloadInProgress = true;
                update.type = TA_Base_Core::LibraryDownloadStart;
                break;

            case ITrainEventCallback::TimeSchedule:
                existingDownload.timeScheduleDownloadInProgress = true;
                update.type = TA_Base_Core::ScheduleDownloadStart;
                break;

            default:
                TA_ASSERT(false, "Invalid library type");
                break;
        }

        sendDownloadUpdateMessage(update, version);

        FUNCTION_EXIT;
    }

    void TTISManager::trainDownloadComplete(TA_IRS_Bus::CommonTypes::TrainIdType trainId,
                                            ETrainLibraryType downloadType,
                                            bool success,
                                            std::string statusString,
                                            unsigned short nextVersion)
    {
        FUNCTION_ENTRY("trainDownloadComplete");

        TA_THREADGUARD(m_trainDownloadsLock);

        std::map< unsigned char, TA_Base_Core::TrainDownload >::iterator findIter = m_trainDownloadList.find(trainId);

        if (findIter == m_trainDownloadList.end())
        {
            // insert an item for this train
            TA_Base_Core::TrainDownload downloadData;
            downloadData.trainNumber = trainId;
            downloadData.predefinedDownloadInProgress = false;
            downloadData.timeScheduleDownloadInProgress = false;

            m_trainDownloadList.insert(std::map< unsigned char, TA_Base_Core::TrainDownload >::value_type(trainId, downloadData));
        }

        // take a reference to the new or existing item
        TA_Base_Core::TrainDownload& existingDownload = m_trainDownloadList[trainId];

        TA_Base_Core::TrainDownloadStatus update;
        update.trainNumber = trainId;
        update.success = success;
        update.errorDetails = CORBA::string_dup(statusString.c_str());

        // check the current version on the train
        TA_Base_Core::TrainDataVersion data;

        try
        {
            data = m_trainDataVersions.getTrainDataVersion(trainId);
        }
        catch (TA_IRS_Bus::TrainDoesNotExistException&)
        {
            // download without a train wakeup first
            // we dont know what version is on the train
            // assume 0
            data.trainNumber = trainId;
            data.predefinedLibraryVersion = 0;
            data.nextPredefinedLibraryVersion = 0;
            data.trainTimeScheduleVersion = 0;
            data.nextTrainTimeScheduleVersion = 0;
        }

        LOG_INFO("hongzhi101: [train id=%d][download type=%d][success=%d][next version=%d][status=%s]",
                 trainId, downloadType, success, nextVersion, statusString);

        switch (downloadType)
        {
            case ITrainEventCallback::MessageLibrary:
                existingDownload.predefinedDownloadInProgress = false;
                update.type = TA_Base_Core::LibraryDownloadFinish;

                // now bump up the next version
                if (true == success)
                {
                    m_trainDataVersions.updateTTISLibraryVersions(trainId, data.predefinedLibraryVersion, nextVersion);
                }

                break;

            case ITrainEventCallback::TimeSchedule:
                existingDownload.timeScheduleDownloadInProgress = false;
                update.type = TA_Base_Core::ScheduleDownloadFinish;

                // now bump up the next version
                if (true == success)
                {
                    m_trainDataVersions.updateScheduleVersions(trainId, data.trainTimeScheduleVersion, nextVersion);
                }

                break;

            default:
                TA_ASSERT(false, "Invalid library type");
                break;
        }

        // get the updated status
        data = m_trainDataVersions.getTrainDataVersion(trainId);

        LOG_INFO("hongzhi100,update.type=%d", update.type);

        // send the message
        sendDownloadUpdateMessage(update, nextVersion);

        // send the version to any observers
        sendVersionUpdateMessage(data);

        FUNCTION_EXIT;
    }

    void TTISManager::trainUpgradeComplete(TA_IRS_Bus::CommonTypes::TrainIdType trainId,
                                           ETrainLibraryType downloadType,
                                           bool success,
                                           std::string statusString,
                                           unsigned short currentVersion)
    {
        FUNCTION_ENTRY("trainUpgradeComplete");

        TA_Base_Core::TrainDownloadStatus update;
        update.trainNumber = trainId;
        update.success = success;
        update.errorDetails = CORBA::string_dup(statusString.c_str());

        TA_Base_Core::TrainDataVersion data;

        try
        {
            data = m_trainDataVersions.getTrainDataVersion(trainId);
        }
        catch (TA_IRS_Bus::TrainDoesNotExistException&)
        {
            // download without a train wakeup first
            // we dont know what version is on the train
            // assume 0
            data.trainNumber = trainId;
            data.predefinedLibraryVersion = 0;
            data.nextPredefinedLibraryVersion = 0;
            data.trainTimeScheduleVersion = 0;
            data.nextTrainTimeScheduleVersion = 0;
        }

        switch (downloadType)
        {
            case ITrainEventCallback::MessageLibrary:
                update.type = TA_Base_Core::LibraryUpgrade;

                // update the current and next version
                if (true == success)
                {
                    m_trainDataVersions.updateTTISLibraryVersions(trainId, currentVersion, data.nextPredefinedLibraryVersion);
                }

                break;

            case ITrainEventCallback::TimeSchedule:
                update.type = TA_Base_Core::ScheduleUpgrade;

                // update the current and next version
                if (true == success)
                {
                    m_trainDataVersions.updateScheduleVersions(trainId, currentVersion, data.nextTrainTimeScheduleVersion);
                }

                break;

            default:
                TA_ASSERT(false, "Invalid library type");
                break;
        }

        // get the updated status
        data = m_trainDataVersions.getTrainDataVersion(trainId);

        // send the download update message
        sendDownloadUpdateMessage(update, currentVersion);

        // send the version to any observers
        sendVersionUpdateMessage(data);

        // now check the versions dont mismatch between the master versions
        verifyTrainDataVersions(trainId);

        FUNCTION_EXIT;
    }

    void TTISManager::predefinedMessageRequestComplete(TA_IRS_Bus::CommonTypes::TrainIdType trainId,
                                                       time_t timestamp,
                                                       std::string sessionId,
                                                       const TA_Base_Core::TTISPredefinedMessageParameters& displayParameters,
                                                       bool success,
                                                       std::string statusString)
    {
        FUNCTION_ENTRY("predefinedMessageRequestComplete");

        if (false == AgentModeMonitor::getInstance().isInControlMode())
        {
            FUNCTION_EXIT;
            return;
        }

        LOG_INFO("Predefined Request for train %d, time %d, session %s complete.", trainId, timestamp, sessionId);

        //
        // first the result must be sent to the client
        //
        TA_ASSERT(m_tisMessageSender != NULL, "Tis Message Sender is null");

        TA_Base_Core::TTISDisplayResult resultMessage;
        resultMessage.trainId = trainId;
        resultMessage.sessionId = sessionId.c_str();
        resultMessage.timestamp = timestamp;
        resultMessage.originalCommand = TA_Base_Core::TisPredefinedMessageCommand;
        resultMessage.success = success;
        resultMessage.errorDetails = statusString.c_str();

        CORBA::Any data;
        data <<= resultMessage;

        m_tisMessageSender->sendCommsMessage(TA_Base_Core::TISComms::TrainDisplayResult,
                                             m_entityKey, data, m_subsystemKey, m_locationKey, NULL);

        // the following only happens if the request was a success
        if (false == success)
        {
            FUNCTION_EXIT;
            return;
        }

        //
        // Second the display request must be logged
        //

        TTISPredefinedDisplayRequest displayRequest;

        std::string operatorName = STISManager::getInstance()->getOperatorNameFromSession(sessionId);

        displayRequest.setData(trainId,
                               displayParameters.pidList,
                               displayParameters.librarySection,
                               displayParameters.libraryVersion,
                               displayParameters.messageId,
                               displayParameters.priority,
                               displayParameters.startTime.in(),
                               displayParameters.endTime.in(),
                               operatorName);

        displayRequest.log(true);

        //
        // Third, the operation must be audited
        //
        TA_Base_Core::DescriptionParameters desc;
        std::ostringstream debugStream;

        // do all train details related parameters
        std::ostringstream trainIDStream;
        std::string trainLocation;

        try
        {
            // use PV/SERVICE string
            TA_IRS_Bus::TrainInformationTypes::TrainDetails details = m_trainAgentSubscriber->getTrainDetails(trainId);
            trainIDStream << details.serviceNumber;

            if (details.currentLocation != 0)
            {
                try
                {
                    trainLocation = TA_IRS_Bus::LocationCache::getInstance().getLocationByKey(details.currentLocation);
                }
                catch (...)
                {
                }
            }
        }
        catch (...)
        {
            trainIDStream << std::setw(2) << std::setfill('0') << trainId << "/_";
        }

        if (true == trainLocation.empty())
        {
            // use agent location
            trainLocation = m_locationName;
        }

        debugStream << "[predefinedMessageRequestComplete:START] trainId = " << trainIDStream.str() << "|";
        TA_Base_Core::NameValuePair trainDP("Train", trainIDStream.str().c_str());
        desc.push_back(&trainDP); // ok

        debugStream << "train location = " << trainLocation << "|";
        TA_Base_Core::NameValuePair trainLocationDP("Train location", trainLocation.c_str());
        desc.push_back(&trainLocationDP); // ok

        // now message parameters not loaded from the predefined message library

        std::string strPidList = TISMessageDataDefinitionsHelper::convertTrainPIDListToString(displayParameters.pidList);
        debugStream << "pidList = " << strPidList << "|";
        TA_Base_Core::NameValuePair pidListDP("PID list", strPidList.c_str());
        desc.push_back(&pidListDP); // 1

        std::string strPriorityValue = TISMessageDataDefinitionsHelper::convertETTISMessagePriorityToString(displayParameters.priority);
        debugStream << "priorityValue = " << strPriorityValue << "|";
        TA_Base_Core::NameValuePair priorityValue(PRIORITY_AUDIT_MESSAGE_PARAMETER_NAME, strPriorityValue.c_str());
        desc.push_back(&priorityValue); //2

        std::string libraryStr(" ");

        switch (displayParameters.librarySection)
        {
            case TA_Base_Core::NORMAL_SECTION:
                libraryStr = "Normal Section Library";
                break;

            case TA_Base_Core::EMERGENCY_SECTION:
                libraryStr = "Emergency Section Library";
                break;

            default:
                break;
        }

        debugStream << "library = " << libraryStr << "|";
        TA_Base_Core::NameValuePair libraryDP("Library", libraryStr.c_str());
        desc.push_back(&libraryDP); // ok

        unsigned long lPriority =  static_cast<unsigned long>(displayParameters.priority);

        TA_Base_Core::NameValuePair startTimeValue("", "");
        TA_Base_Core::NameValuePair endTimeValue("", "");

        if (lPriority > TA_Base_Core::TTIS_PRIORITY_3)
        {
            startTimeValue = TA_Base_Core::NameValuePair(STARTTIME_AUDIT_MESSAGE_PARAMETER_NAME,
                                                         TISMessageDataDefinitionsHelper::convertTimeStringToFormattedTimeString(displayParameters.startTime.in()));
            desc.push_back(&startTimeValue); //3

            endTimeValue = TA_Base_Core::NameValuePair(ENDTIME_AUDIT_MESSAGE_PARAMETER_NAME,
                                                       TISMessageDataDefinitionsHelper::convertTimeStringToFormattedTimeString(displayParameters.endTime.in()));
            desc.push_back(&endTimeValue); //4
        }

        // now parameters loaded from the predefined message library

        std::string strMessageIDDP =  StatusMonitor::getInstance()->getTrainPredefinedMessageText(displayParameters.librarySection, displayParameters.messageId);
        debugStream << "messageIDDP = " << strMessageIDDP << "|";
        TA_Base_Core::NameValuePair messageIDDP("Message", strMessageIDDP.c_str());
        desc.push_back(&messageIDDP); //5

        TA_Base_Core::TTISLedAttributes defaultTTISAttributes = StatusMonitor::getInstance()->getDefaultTTISLedAttributes();

        TA_Base_Core::NameValuePair repeatIntervalValue("", "");

        if (lPriority > TA_Base_Core::TTIS_PRIORITY_3)
        {
            std::ostringstream repeatIntervalStr;
            unsigned int repeatIntrevalInt = static_cast<unsigned int> (defaultTTISAttributes.repeatInterval);
            repeatIntervalStr << repeatIntrevalInt / 60 << " Hours " << repeatIntrevalInt % 60 << "  Minutes";
            std::string rStr = repeatIntervalStr.str();
            debugStream << "repeatInterval = " << rStr << "[END]";
            repeatIntervalValue = TA_Base_Core::NameValuePair(REPEAT_INTERVAL_AUDIT_MESSAGE_PARAMETER_NAME, rStr.c_str());
            desc.push_back(&repeatIntervalValue); //7
        }

        std::string displayAttributesStr;
        displayAttributesStr = TISMessageDataDefinitionsHelper::convertTTISLedAttributesToString(defaultTTISAttributes);
        debugStream << "displayAttributes = " << displayAttributesStr << "|";
        TA_Base_Core::NameValuePair displayAttributesValue(DISPLAY_ATTRIBUTE_AUDIT_MESSAGE_PARAMETER_NAME, displayAttributesStr.c_str());
        desc.push_back(&displayAttributesValue); //6

        LOG_INFO("TTISManager::predefinedMessageRequestComplete: %s", debugStream);

        timeb curtime;
        StringHelper::isConvertToTimeb(displayRequest.getTimestamp(), curtime);

        m_auditMessageSender->sendAuditMessage(
            TA_Base_Core::TISAudit::TTISPredefinedMessage,
            m_entityKey,
            desc,
            "", // Further description text
            sessionId,
            "", // alarm ID - not required
            "", // incident key - not required
            "", // event key - not required
            NULL, // data filter - not required
            curtime); // logged time

        FUNCTION_EXIT;
    }

    void TTISManager::freeTextMessageRequestComplete(TA_IRS_Bus::CommonTypes::TrainIdType trainId,
                                                     time_t timestamp,
                                                     std::string sessionId,
                                                     const TA_Base_Core::TTISFreeTextMessageParameters& displayParameters,
                                                     bool success,
                                                     std::string statusString)
    {
        FUNCTION_ENTRY("freeTextMessageRequestComplete");

        if (false == AgentModeMonitor::getInstance().isInControlMode())
        {
            FUNCTION_EXIT;
            return;
        }

        bool isRatis = false;

        {
            TA_THREADGUARD(m_ratisDisplayLock);

            for (TTISApprovedRATISDisplayRequestList::iterator iter = m_ratisDisplayInProgress.begin();
                 iter != m_ratisDisplayInProgress.end(); ++iter)
            {
                if (true == (*iter)->freeTextMessageRequestComplete(trainId, timestamp, sessionId, displayParameters, success, statusString))
                {
                    // this is a RATIS display request
                    isRatis = true;
                    break;
                }
            }
        }

        // RATIS messages dont get sent to the client or display requests logged here
        // they are already catered for in TTISApprovedRATISDisplayRequest
        if (false == isRatis)
        {
            LOG_INFO("Free Text Request for train %d, time %d, session %s complete.", trainId, timestamp, sessionId);

            //
            // first the result must be sent to the client
            //
            TA_ASSERT(m_tisMessageSender != NULL, "Tis Message Sender is null");

            TA_Base_Core::TTISDisplayResult resultMessage;
            resultMessage.trainId = trainId;
            resultMessage.sessionId = sessionId.c_str();
            resultMessage.timestamp = timestamp;
            resultMessage.originalCommand = TA_Base_Core::TisFreeTextMessageCommand;
            resultMessage.success = success;
            resultMessage.errorDetails = statusString.c_str();

            CORBA::Any data;
            data <<= resultMessage;

            m_tisMessageSender->sendCommsMessage(TA_Base_Core::TISComms::TrainDisplayResult,
                                                 m_entityKey, data, m_subsystemKey, m_locationKey, NULL);
        }

        // the following only happens if the request was a success
        if (false == success)
        {
            FUNCTION_EXIT;
            return;
        }

        TTISAdHocDisplayRequest displayRequest;

        if (false == isRatis)
        {
            //
            // Second the display request must be logged
            //

            std::string operatorName = STISManager::getInstance()->getOperatorNameFromSession(sessionId);

            displayRequest.setData(trainId,
                                   displayParameters.pidList,
                                   displayParameters.message.in(),
                                   displayParameters.fontSize,
                                   displayParameters.justification,
                                   displayParameters.intensity,
                                   displayParameters.displayMode,
                                   displayParameters.priority,
                                   displayParameters.startTime.in(),
                                   displayParameters.endTime.in(),
                                   displayParameters.repeatInterval,
                                   operatorName);

            displayRequest.log(true);
        }

        //
        // Third, the operation must be audited
        //

        TA_Base_Core::DescriptionParameters desc;

        // do all train details related parameters
        std::ostringstream trainIDStream;
        std::string trainLocation;

        try
        {
            // use PV/SERVICE string
            TA_IRS_Bus::TrainInformationTypes::TrainDetails details = m_trainAgentSubscriber->getTrainDetails(trainId);
            trainIDStream << details.serviceNumber;

            if (details.currentLocation != 0)
            {
                try
                {
                    trainLocation = TA_IRS_Bus::LocationCache::getInstance().getLocationByKey(details.currentLocation);
                }
                catch (...)
                {
                }
            }
        }
        catch (...)
        {
            trainIDStream << std::setw(2) << std::setfill('0') << static_cast<unsigned int>(trainId) << "/_";
        }

        if (true == trainLocation.empty())
        {
            // use agent location
            trainLocation = m_locationName;
        }

        std::ostringstream debugStream;

        debugStream << "[START] trainId = " << trainIDStream.str() << "|";
        TA_Base_Core::NameValuePair trainDP("Train", trainIDStream.str().c_str());
        desc.push_back(&trainDP);

        TA_ASSERT(false == trainLocation.empty(), "Train Location not empty");
        debugStream << "trainLocation = " << trainLocation << "|";
        TA_Base_Core::NameValuePair trainLocationDP("Train location", trainLocation.c_str());
        desc.push_back(&trainLocationDP);

        std::string strMsg = std::string(displayParameters.message.in());
        debugStream << "adhocMsg = " << strMsg << "|";
        TA_Base_Core::NameValuePair adhocMessageDP("message", strMsg.c_str());
        desc.push_back(&adhocMessageDP);

        std::string strPidList = TISMessageDataDefinitionsHelper::convertTrainPIDListToString(displayParameters.pidList);
        debugStream << "pidList = " << strPidList << "|";
        TA_Base_Core::NameValuePair pidListDP("PID list", strPidList.c_str());
        desc.push_back(&pidListDP);

        //wenbo++,CL-21375
        TA_Base_Core::NameValuePair startTimeValue("", "");
        TA_Base_Core::NameValuePair endTimeValue("", "");

        if (static_cast<unsigned long>(displayParameters.priority) > TA_Base_Core::TTIS_PRIORITY_3)
        {
            std::string strStartTime = TISMessageDataDefinitionsHelper::convertTimeStringToFormattedTimeString(displayParameters.startTime.in());
            debugStream << "starttime = " << strStartTime << "|";
            //CL-21375
            startTimeValue = TA_Base_Core::NameValuePair(STARTTIME_AUDIT_MESSAGE_PARAMETER_NAME, strStartTime.c_str());

            desc.push_back(&startTimeValue);

            std::string strEndTime = TISMessageDataDefinitionsHelper::convertTimeStringToFormattedTimeString(displayParameters.endTime.in());
            debugStream << "endtime = " << strEndTime << "|";
            //CL-21375
            endTimeValue = TA_Base_Core::NameValuePair(ENDTIME_AUDIT_MESSAGE_PARAMETER_NAME, strEndTime.c_str());

            desc.push_back(&endTimeValue);
        }

        std::string strPriority = TISMessageDataDefinitionsHelper::convertETTISMessagePriorityToString(displayParameters.priority);
        debugStream << "priority = " << strPriority << "[END]";
        TA_Base_Core::NameValuePair priorityValue(PRIORITY_AUDIT_MESSAGE_PARAMETER_NAME, strPriority.c_str());
        desc.push_back(&priorityValue); //3

        //wenbo++,CL-21375
        TA_Base_Core::NameValuePair repeatIntervalValue("", "");

        if (static_cast<unsigned long>(displayParameters.priority) > TA_Base_Core::TTIS_PRIORITY_3)
        {
            std::ostringstream repeatIntervalStr;
            unsigned int repeatIntrevalInt = static_cast<unsigned int> (displayParameters.repeatInterval);
            repeatIntervalStr << repeatIntrevalInt / 60 << " Hours " << repeatIntrevalInt % 60 << " Minutes";
            //repeatIntervalStr << static_cast<unsigned int> ( displayParameters.repeatInterval ) << " Seconds";
            debugStream << "repeatInterval = " << repeatIntervalStr.str() << "|";
            //CL-21375
            repeatIntervalValue = TA_Base_Core::NameValuePair(REPEAT_INTERVAL_AUDIT_MESSAGE_PARAMETER_NAME, repeatIntervalStr.str().c_str());
            desc.push_back(&repeatIntervalValue); //1
        }

        std::ostringstream displayAttributesStr;
        displayAttributesStr << "Display Mode " << TISMessageDataDefinitionsHelper::convertEDisplayModeToString(displayParameters.displayMode) << ", "
            << "Justification " << TISMessageDataDefinitionsHelper::convertEJustificationToString(displayParameters.justification) << ", "
            << "Intensity " << TISMessageDataDefinitionsHelper::convertETTISLEDIntensityToString(displayParameters.intensity) << ", "
            << "Font Size " << TISMessageDataDefinitionsHelper::convertETTISLEDFontSizeToString(displayParameters.fontSize);
        debugStream << "displayAttributes = " << displayAttributesStr.str() << "|";
        TA_Base_Core::NameValuePair displayAttributesValue(DISPLAY_ATTRIBUTE_AUDIT_MESSAGE_PARAMETER_NAME, displayAttributesStr.str().c_str());
        desc.push_back(&displayAttributesValue); //2

        LOG_INFO("TTISManager::freeTextMessageRequestComplete: %s", debugStream);

        timeb curtime;
        StringHelper::isConvertToTimeb(displayRequest.getTimestamp(), curtime);

        m_auditMessageSender->sendAuditMessage(TA_Base_Core::TISAudit::TTISFreeTextMessage, m_entityKey,
                                               desc,
                                               "", // Further description text
                                               sessionId,
                                               "", // alarm ID - not required
                                               "", // incident key - not required
                                               "", // event key - not required
                                               NULL, // data filter - not required
                                               curtime); // logged time

        FUNCTION_EXIT;
    }

    void TTISManager::clearRequestComplete(TA_IRS_Bus::CommonTypes::TrainIdType trainId,
                                           time_t timestamp,
                                           std::string sessionId,
                                           const TA_Base_Core::TTISMessageResetParameters& displayParameters,
                                           bool success,
                                           std::string statusString)
    {
        FUNCTION_ENTRY("clearRequestComplete");

        if (false == AgentModeMonitor::getInstance().isInControlMode())
        {
            FUNCTION_EXIT;
            return;
        }

        bool isRatis = false;

        {
            TA_THREADGUARD(m_ratisClearLock);

            for (TTISApprovedRATISClearRequestList::iterator iter = m_ratisClearInProgress.begin();
                 iter != m_ratisClearInProgress.end(); ++iter)
            {
                if (true == (*iter)->clearRequestComplete(trainId, timestamp, sessionId, displayParameters, success, statusString))
                {
                    // this is a RATIS clear request
                    isRatis = true;
                    break;
                }
            }
        }

        // RATIS messages dont get sent to the client or display requests logged here
        // they are already catered for in TTISApprovedRATISClearRequest
        if (false == isRatis)
        {
            LOG_INFO("Clear Request for train %d, time %d, session %s complete.", trainId, timestamp, sessionId);

            //
            // first the result must be sent to the client
            //
            TA_ASSERT(m_tisMessageSender != NULL, "Tis Message Sender is null");

            TA_Base_Core::TTISDisplayResult resultMessage;
            resultMessage.trainId = trainId;
            resultMessage.sessionId = sessionId.c_str();
            resultMessage.timestamp = timestamp;
            resultMessage.originalCommand = TA_Base_Core::TisClearCommand;
            resultMessage.success = success;
            resultMessage.errorDetails = statusString.c_str();

            CORBA::Any data;
            data <<= resultMessage;

            m_tisMessageSender->sendCommsMessage(TA_Base_Core::TISComms::TrainDisplayResult,
                                                 m_entityKey, data, m_subsystemKey, m_locationKey, NULL);
        }

        // the following only happens if the request was a success
        if (false == success)
        {
            FUNCTION_EXIT;
            return;
        }

        //
        // Second the operation must be audited
        //

        TA_Base_Core::DescriptionParameters desc;

        // do all train details related parameters
        std::ostringstream trainIDStream;
        std::string trainLocation;

        try
        {
            // use PV/SERVICE string
            TA_IRS_Bus::TrainInformationTypes::TrainDetails details = m_trainAgentSubscriber->getTrainDetails(trainId);
            trainIDStream << details.serviceNumber;

            if (details.currentLocation != 0)
            {
                try
                {
                    trainLocation = TA_IRS_Bus::LocationCache::getInstance().getLocationByKey(details.currentLocation);
                }
                catch (...)
                {
                }
            }
        }
        catch (...)
        {
            trainIDStream << std::setw(2) << std::setfill('0') << trainId << "/_";
        }

        if (true == trainLocation.empty())
        {
            // use agent location
            trainLocation = m_locationName;
        }

        TA_Base_Core::NameValuePair trainDP("Train", trainIDStream.str().c_str());
        desc.push_back(&trainDP);

        TA_Base_Core::NameValuePair trainLocationDP("Train location", trainLocation);
        desc.push_back(&trainLocationDP);

        TA_Base_Core::NameValuePair pidListDP("PID list", TISMessageDataDefinitionsHelper::convertTrainPIDListToString(displayParameters.pidList));
        desc.push_back(&pidListDP);

        TA_Base_Core::NameValuePair clearDP("ClearType", TISMessageDataDefinitionsHelper::convertETTISClearTypeToString(displayParameters.messageType));
        desc.push_back(&clearDP);

        m_auditMessageSender->sendAuditMessage(
            TA_Base_Core::TISAudit::TTISResetMessage,
            m_entityKey,
            desc,
            "", // Further description text
            sessionId,
            "", // alarm ID - not required
            "", // incident key - not required
            ""); // event key - not required

        FUNCTION_EXIT;
    }

    void TTISManager::verifyTrainDataVersions(TA_IRS_Bus::CommonTypes::TrainIdType trainId)
    {
        FUNCTION_ENTRY("verifyTrainDataVersions");

        // check agent in control
        if (false == AgentModeMonitor::getInstance().isInControlMode())
        {
            FUNCTION_EXIT;
            return;
        }

        // if this is not the OCC or depot, then downloads are not applicable here
        if ((TA_Base_Core::ILocation::OCC != m_locationType && TA_Base_Core::ILocation::BOCC != m_locationType) &&
            (TA_Base_Core::ILocation::DPT != m_locationType))
        {
            FUNCTION_EXIT;
            return;
        }

        try
        {
            // check if this agent is responsible for the version management for this train
            bool isResponsibleForTrain = m_trainAgentSubscriber->isTrainUnderAgentAreaControl(trainId);

            if (false == isResponsibleForTrain)
            {
                FUNCTION_EXIT;
                return;
            }

            TA_THREADGUARD(m_trainDataLock);

            bool changes = false;

            // get the current alarm state for the train
            std::map< unsigned char, TA_Base_Core::TrainDataVersionAlarms >::iterator findIter = m_trainDataVersionAlarmList.find(trainId);

            if (findIter == m_trainDataVersionAlarmList.end())
            {
                // insert an item for this train
                TA_Base_Core::TrainDataVersionAlarms newTrainAlarms;
                newTrainAlarms.trainNumber = trainId;
                newTrainAlarms.messageLibraryMistmatchAlarm = CORBA::string_dup("");
                newTrainAlarms.iscsLibraryVersion = 0;
                newTrainAlarms.trainLibraryVersion = 0;
                newTrainAlarms.timeScheduleMistmatchAlarm = CORBA::string_dup("");
                newTrainAlarms.iscsScheduleVersion = 0;
                newTrainAlarms.trainScheduleVersion = 0;

                m_trainDataVersionAlarmList.insert(std::map< unsigned char, TA_Base_Core::TrainDataVersionAlarms >::value_type(trainId, newTrainAlarms));

                changes = true;
            }

            // take a reference to the entry in the map for simplicity
            // note not a copy
            TA_Base_Core::TrainDataVersionAlarms& trainAlarms = m_trainDataVersionAlarmList[trainId];

            // flags for final actions
            bool libraryAlarmShouldBeClosed = false;
            bool libraryAlarmShouldBeRaised = false;
            bool scheduleAlarmShouldBeClosed = false;
            bool scheduleAlarmShouldBeRaised = false;

            // get current ISCS library version
            unsigned short currentIscsLibraryVersion = StatusMonitor::getInstance()->getCurrentISCSTrainLibraryVersion();
            // get current ISCS schedule version
            TA_Base_Core::TimeScheduleVersion ret;
            CORBA_CALL_RETURN(ret, TISAgentAccessFactory::getInstance().getOccSTISAgent(), getCurrentTrainTimeScheduleVersion, ());
            unsigned short currentIscsTimeScheduleVersion = ret.version;

            // check if alarms are raised
            bool messageLibraryAlarmIsRaised = !std::string(trainAlarms.messageLibraryMistmatchAlarm.in()).empty();
            bool timeScheduleAlarmIsRaised = !std::string(trainAlarms.timeScheduleMistmatchAlarm.in()).empty();

            TA_Base_Core::TrainDataVersion trainVersion;

            // if the agent is not supposed to be in control of the train, and there are any alarms active, close them
            if (false == isResponsibleForTrain)
            {
                if (true == messageLibraryAlarmIsRaised)
                {
                    libraryAlarmShouldBeClosed = true;
                }

                if (true == timeScheduleAlarmIsRaised)
                {
                    scheduleAlarmShouldBeClosed = true;
                }
            }
            // responsible for train
            // determine correct alarm state
            else
            {
                // get train data versions
                trainVersion = m_trainDataVersions.getTrainDataVersion(trainId);

                //
                // evaluate the library version alarm state
                //

                // check if library alarm should be raised
                bool shouldHaveLibraryAlarm = trainVersion.predefinedLibraryVersion != currentIscsLibraryVersion;

                if (true == shouldHaveLibraryAlarm)
                {
                    // an alarm should exist
                    if (false == messageLibraryAlarmIsRaised)
                    {
                        // if not, raise it
                        libraryAlarmShouldBeRaised = true;
                    }
                    // an alarm exists
                    else
                    {
                        // check if details match
                        if ((trainAlarms.iscsLibraryVersion != currentIscsLibraryVersion) ||
                            (trainAlarms.trainLibraryVersion != trainVersion.predefinedLibraryVersion))
                        {
                            // if not, close old, raise new
                            libraryAlarmShouldBeClosed = true;
                            libraryAlarmShouldBeRaised = true;
                        }
                    }
                }
                // if not, and one is raised
                else if (true == messageLibraryAlarmIsRaised)
                {
                    // close it
                    libraryAlarmShouldBeClosed = true;
                }

                //
                // evaluate the schedule version alarm state
                //

                // check if schedule alarm should be raised
                bool shouldHaveScheduleAlarm = trainVersion.trainTimeScheduleVersion != currentIscsTimeScheduleVersion;

                if (true == shouldHaveScheduleAlarm)
                {
                    // an alarm should exist
                    if (false == timeScheduleAlarmIsRaised)
                    {
                        // if not, raise it
                        scheduleAlarmShouldBeRaised = true;
                    }
                    // an alarm exists
                    else
                    {
                        // check if details match
                        if ((trainAlarms.iscsScheduleVersion != currentIscsTimeScheduleVersion) ||
                            (trainAlarms.trainScheduleVersion != trainVersion.trainTimeScheduleVersion))
                        {
                            // if not, close old, raise new
                            scheduleAlarmShouldBeClosed = true;
                            scheduleAlarmShouldBeRaised = true;
                        }
                    }
                }
                // if not, and one is raised
                else if (true == timeScheduleAlarmIsRaised)
                {
                    // close it
                    scheduleAlarmShouldBeClosed = true;
                }
            }

            //
            // do the final actions - raising and closing alarms
            //
            if (true == libraryAlarmShouldBeClosed)
            {
                closeAlarm(trainAlarms.messageLibraryMistmatchAlarm.in());
                trainAlarms.messageLibraryMistmatchAlarm = CORBA::string_dup("");
                trainAlarms.iscsLibraryVersion = 0;
                trainAlarms.trainLibraryVersion = 0;
                changes = true;
            }

            if (true == libraryAlarmShouldBeRaised)
            {
                trainAlarms.messageLibraryMistmatchAlarm = CORBA::string_dup(raiseLibraryVersionAlarm(trainId, trainVersion.predefinedLibraryVersion, currentIscsLibraryVersion).c_str());
                trainAlarms.iscsLibraryVersion = currentIscsLibraryVersion;
                trainAlarms.trainLibraryVersion = trainVersion.predefinedLibraryVersion;
                changes = true;
            }

            if (true == scheduleAlarmShouldBeClosed)
            {
                closeAlarm(trainAlarms.timeScheduleMistmatchAlarm.in());
                trainAlarms.timeScheduleMistmatchAlarm = CORBA::string_dup("");
                trainAlarms.iscsScheduleVersion = 0;
                trainAlarms.trainScheduleVersion = 0;
                changes = true;
            }

            if (true == scheduleAlarmShouldBeRaised)
            {
                trainAlarms.timeScheduleMistmatchAlarm = CORBA::string_dup(raiseScheduleVersionAlarm(trainId, trainVersion.trainTimeScheduleVersion, currentIscsTimeScheduleVersion).c_str());
                trainAlarms.iscsScheduleVersion = currentIscsTimeScheduleVersion;
                trainAlarms.trainScheduleVersion = trainVersion.trainTimeScheduleVersion;
                changes = true;
            }

            // if changes were made
            if (true == changes)
            {
                // send sync message to peer
                sendLibraryVersionAlarmUpdate(trainAlarms);
            }
        }
        catch (TA_Base_Core::TransactiveException& e)
        {
            LOG_EXCEPTION_CATCH(SourceInfo, "TA_Base_Core::TransactiveException", e.what());
        }
        // HuangQi add for CL-21314 memory leaking start on 2010-03-11
        catch (TA_Base_Bus::ITrainExceptionsCorba::InvalidTrainException& e)
        {
            LOG_EXCEPTION_CATCH(SourceInfo, "TA_Base_Bus::ITrainExceptionsCorba::InvalidTrainException", e.details._ptr);
        }
        catch (...)
        {
            LOG_EXCEPTION_CATCH(SourceInfo, "Unknow", "Unknow exception catched");
        }

        // HuangQi add for CL-21314 memory leaking end on 2010-03-11

        FUNCTION_EXIT;
    }

    void TTISManager::verifyAllTrainDataVersions()
    {
        FUNCTION_ENTRY("verifyAllTrainDataVersions");

        // check agent in control
        if (false == AgentModeMonitor::getInstance().isInControlMode())
        {
            FUNCTION_EXIT;
            return;
        }

        // if this is not the OCC or depot, then downloads are not applicable here
        if ((TA_Base_Core::ILocation::OCC != m_locationType && TA_Base_Core::ILocation::BOCC != m_locationType) &&
            (TA_Base_Core::ILocation::DPT != m_locationType))
        {
            FUNCTION_EXIT;
            return;
        }

        TA_IRS_Bus::TrainInformationTypes::TrainDetailsMap allTrains = m_trainAgentSubscriber->getTrainList();

        for (TA_IRS_Bus::TrainInformationTypes::TrainDetailsMap::iterator iter = allTrains.begin(); iter != allTrains.end(); ++iter)
        {
            verifyTrainDataVersions(iter->second.trainId);
        }

        FUNCTION_EXIT;
    }

    void TTISManager::subscribeToNotifications(bool controlMode)
    {
        FUNCTION_ENTRY("subscribeToNotifications");

        // this agent will subscribe to two types of messages
        // the first is the download status updates,
        // this will be used by the monitor mode peer to update the current download list when the control agent sends an update to the GUI
        // This is only applicable for OCC and depot
        //
        // the second is the version update message.
        // this is a broadcast message sent out whenever a train version updates
        // it is also only used to update the internal list

        TA_Base_Core::MessageSubscriptionManager::getInstance().unsubscribeToMessages(
            static_cast<TA_Base_Core::SpecialisedMessageSubscriber<TA_Base_Core::CommsMessageCorbaDef>*>(this));

        TA_Base_Core::MessageSubscriptionManager::getInstance().unsubscribeToMessages(
            static_cast<TA_Base_Core::SpecialisedMessageSubscriber<TA_Base_Core::StateUpdateMessageCorbaDef>*>(this));

        if ((TA_Base_Core::ILocation::STATION != m_locationType) &&
            (false == controlMode))
        {
            // subscribe to 'this' agent
            MessageSubscriptionManager::getInstance().subscribeToCommsMessage(
                TA_Base_Core::TISComms::TisTrainDownloadUpdate,
                static_cast<TA_Base_Core::SpecialisedMessageSubscriber<TA_Base_Core::CommsMessageCorbaDef>*>(this),
                m_entityKey,
                m_subsystemKey,
                m_locationKey);
        }

        MessageSubscriptionManager::getInstance().subscribeToCommsMessage(
            TA_Base_Core::TISComms::TisTrainDataVersionUpdate,
            static_cast<TA_Base_Core::SpecialisedMessageSubscriber<TA_Base_Core::CommsMessageCorbaDef>*>(this),
            m_entityKey,
            m_subsystemKey,
            m_locationKey);

        if (false == controlMode)
        {
            MessageSubscriptionManager::getInstance().subscribeToStateUpdateMessage(TA_Base_Core::TISStateUpdate::TisTrainDataVersionAlarm,
                                                                                    static_cast<TA_Base_Core::SpecialisedMessageSubscriber<TA_Base_Core::StateUpdateMessageCorbaDef>*>(this),
                                                                                    TA_Base_Core::RunParams::getInstance().get(RPARAM_ENTITYNAME),
                                                                                    NULL);
        }

        FUNCTION_EXIT;
    }

    void TTISManager::getInitialTrainDataVersions()
    {
        FUNCTION_ENTRY("getInitialTrainDataVersions");

        // at startup its not possible to predict the mode we will be started in,
        // first, attempt to sync from the local peer (in case one exists)
        // if this fails, fall back to the OCC (if at depot) or depot (if at OCC)
        // a station agent can fall back to both the depot and OCC

        // build a list of location names to try to get version information from
        std::vector< std::string > locationAttemptOrder;

        // local agent
        locationAttemptOrder.push_back(m_locationName);

        // this is the OCC or depot Agent, so use the opposite agent
        if (false == m_otherAgentLocationName.empty())
        {
            locationAttemptOrder.push_back(m_otherAgentLocationName);
        }
        // this is a station agent, so try both OCC and depot
        else
        {
            try
            {
                unsigned long occLocationKey = TA_IRS_Bus::LocationCache::getInstance().getOccLocationKey();
                locationAttemptOrder.push_back(TA_IRS_Bus::LocationCache::getInstance().getLocationByKey(occLocationKey));

                unsigned long depotLocationKey = TA_IRS_Bus::LocationCache::getInstance().getDepotLocationKey();
                locationAttemptOrder.push_back(TA_IRS_Bus::LocationCache::getInstance().getLocationByKey(depotLocationKey));
            }
            catch (const TA_Base_Core::DatabaseException& ex)
            {
                LOG_EXCEPTION_CATCH(SourceInfo, "TA_Base_Core::DatabaseException",
                                    ex.what());
            }
            catch (const TA_Base_Core::DataException& ex)
            {
                LOG_EXCEPTION_CATCH(SourceInfo, "TA_Base_Core::DataException",
                                    ex.what());
            }
        }

        for (std::vector< std::string >::iterator iter = locationAttemptOrder.begin();
             iter != locationAttemptOrder.end(); ++iter)
        {
            try
            {
                TA_Base_Core::TrainDataVersionList_var versionList =
                    TISAgentAccessFactory::getInstance().getTTISAgentAtLocation(*iter)->getTrainDataVersions();

                for (unsigned int i = 0; i < versionList->length(); i++)
                {
                    this->m_trainDataVersions.updateScheduleVersions(versionList[i].trainNumber,
                                                                     versionList[i].trainTimeScheduleVersion,
                                                                     versionList[i].nextTrainTimeScheduleVersion);

                    this->m_trainDataVersions.updateTTISLibraryVersions(versionList[i].trainNumber,
                                                                        versionList[i].predefinedLibraryVersion,
                                                                        versionList[i].nextPredefinedLibraryVersion);
                }

                // exit loop on success
                break;
            }
            catch (const TA_Base_Core::DatabaseException& ex)
            {
                LOG_EXCEPTION_CATCH(SourceInfo, "TA_Base_Core::DatabaseException",
                                    ex.what());
            }
            catch (const TA_Base_Core::DataException& ex)
            {
                LOG_EXCEPTION_CATCH(SourceInfo, "TA_Base_Core::DataException",
                                    ex.what());
            }
            catch (const TA_Base_Core::ObjectResolutionException& ex)
            {
                LOG_EXCEPTION_CATCH(SourceInfo, "TA_Base_Core::ObjectResolutionException",
                                    ex.what());
            }
            catch (const CORBA::Exception& ex)
            {
                LOG_EXCEPTION_CATCH(SourceInfo, "CORBA::Exception",
                                    TA_Base_Core::CorbaUtil::getInstance().exceptionToString(ex).c_str());
            }
            catch (...)
            {
                LOG_EXCEPTION_CATCH(SourceInfo, "...", "Undefined exception "
                                    "when synchronising with control agent.");
            }
        }

        FUNCTION_EXIT;
    }

    void TTISManager::getInitialTrainDownloads()
    {
        FUNCTION_ENTRY("getInitialTrainDownloads");

        // if this is not the OCC or depot, then downloads are not applicable here
        if ((TA_Base_Core::ILocation::OCC != m_locationType && TA_Base_Core::ILocation::BOCC != m_locationType) &&
            (TA_Base_Core::ILocation::DPT != m_locationType))
        {
            FUNCTION_EXIT;
            return;
        }

        // at startup its not possible to predict the mode we will be started in,
        // attempt to sync from the local peer (in case one exists)

        try
        {
            TA_Base_Core::TrainDownloadList_var trainDownloadList = TISAgentAccessFactory::getInstance().getTTISAgentAtLocation(m_locationName)->getCurrentTrainDownloads();

            for (unsigned long i = 0; i < trainDownloadList->length(); ++i)
            {
                m_trainDownloadList[trainDownloadList[i].trainNumber] = trainDownloadList[i];
            }
        }
        catch (const TA_Base_Core::DatabaseException& ex)
        {
            LOG_EXCEPTION_CATCH(SourceInfo, "TA_Base_Core::DatabaseException",
                                ex.what());
        }
        catch (const TA_Base_Core::DataException& ex)
        {
            LOG_EXCEPTION_CATCH(SourceInfo, "TA_Base_Core::DataException",
                                ex.what());
        }
        catch (const TA_Base_Core::ObjectResolutionException& ex)
        {
            LOG_EXCEPTION_CATCH(SourceInfo, "TA_Base_Core::ObjectResolutionException",
                                ex.what());
        }
        catch (const CORBA::Exception& ex)
        {
            LOG_EXCEPTION_CATCH(SourceInfo, "CORBA::Exception",
                                TA_Base_Core::CorbaUtil::getInstance().exceptionToString(ex).c_str());
        }
        catch (...)
        {
            LOG_EXCEPTION_CATCH(SourceInfo, "...", "Undefined exception "
                                "when synchronising with control agent.");
        }

        FUNCTION_EXIT;
    }

    void TTISManager::getInitialTrainDataVersionAlarms()
    {
        FUNCTION_ENTRY("getInitialTrainDataVersionAlarms");

        // if this is not the OCC or depot, then downloads are not applicable here
        if ((TA_Base_Core::ILocation::OCC != m_locationType && TA_Base_Core::ILocation::BOCC != m_locationType) &&
            (TA_Base_Core::ILocation::DPT != m_locationType))
        {
            FUNCTION_EXIT;
            return;
        }

        TA_THREADGUARD(m_trainDataLock);

        // at startup its not possible to predict the mode we will be started in,
        // attempt to sync from the local peer (in case one exists)

        bool success = false;

        try
        {
            TA_Base_Core::TrainDataVersionAlarmList_var alarmList = TISAgentAccessFactory::getInstance().getTTISAgentAtLocation(m_locationName)->getTrainDataAlarms();

            for (unsigned long i = 0; i < alarmList->length(); ++i)
            {
                m_trainDataVersionAlarmList[alarmList[i].trainNumber] = alarmList[i];
            }

            success = true;
        }
        catch (const TA_Base_Core::DatabaseException& ex)
        {
            LOG_EXCEPTION_CATCH(SourceInfo, "TA_Base_Core::DatabaseException",
                                ex.what());
        }
        catch (const TA_Base_Core::DataException& ex)
        {
            LOG_EXCEPTION_CATCH(SourceInfo, "TA_Base_Core::DataException",
                                ex.what());
        }
        catch (const TA_Base_Core::ObjectResolutionException& ex)
        {
            LOG_EXCEPTION_CATCH(SourceInfo, "TA_Base_Core::ObjectResolutionException",
                                ex.what());
        }
        catch (const CORBA::Exception& ex)
        {
            LOG_EXCEPTION_CATCH(SourceInfo, "CORBA::Exception",
                                TA_Base_Core::CorbaUtil::getInstance().exceptionToString(ex).c_str());
        }
        catch (...)
        {
            LOG_EXCEPTION_CATCH(SourceInfo, "...", "Undefined exception "
                                "when synchronising with control agent.");
        }

        // if there was no peer agent to sync from
        if (false == success)
        {
            // then all alarms should be closed
            closeAllLibraryVersionAlarms();
        }

        FUNCTION_EXIT;
    }

    void TTISManager::sendVersionUpdateMessage(const TA_Base_Core::TrainDataVersion& trainData)
    {
        FUNCTION_ENTRY("sendVersionUpdateMessage");

        TA_ASSERT(m_tisMessageSender != NULL, "Tis Message Sender is null");

        if (false == AgentModeMonitor::getInstance().isInControlMode())
        {
            FUNCTION_EXIT;
            return;
        }

        LOG_INFO("Sending TisTrainDataVersionUpdate message, [train id=%d][current version=%d][next verson=%d][current schedule=%d][next schedule=%d]",
                 trainData.trainNumber, trainData.predefinedLibraryVersion, trainData.nextPredefinedLibraryVersion, trainData.trainTimeScheduleVersion, trainData.nextTrainTimeScheduleVersion);

        // put the data into the message
        CORBA::Any data;
        data <<= trainData;

        m_tisMessageSender->sendCommsMessage(TA_Base_Core::TISComms::TisTrainDataVersionUpdate, m_entityKey, data, m_subsystemKey, m_locationKey);

        FUNCTION_EXIT;
    }

    void TTISManager::sendDownloadUpdateMessage(const TA_Base_Core::TrainDownloadStatus& downloadStatus, unsigned short version)
    {
        FUNCTION_ENTRY("sendDownloadUpdateMessage");

        TA_ASSERT(m_tisMessageSender != NULL, "Tis Message Sender is null");

        if (false == AgentModeMonitor::getInstance().isInControlMode())
        {
            FUNCTION_EXIT;
            return;
        }

        LOG_INFO("Sending TisTrainDownloadUpdate message.");

        CORBA::Any data;
        data <<= downloadStatus;

        m_tisMessageSender->sendCommsMessage(TA_Base_Core::TISComms::TisTrainDownloadUpdate,
                                             m_entityKey, data, m_subsystemKey, m_locationKey, NULL);

        // audit the event
        TA_Base_Core::DescriptionParameters desc;

        // do all train details related parameters
        std::ostringstream trainIDStream;
        std::string trainLocation;

        try
        {
            // use PV/SERVICE string
            TA_IRS_Bus::TrainInformationTypes::TrainDetails details = m_trainAgentSubscriber->getTrainDetails(downloadStatus.trainNumber);
            trainIDStream << details.serviceNumber;

            if (details.currentLocation != 0)
            {
                try
                {
                    trainLocation = TA_IRS_Bus::LocationCache::getInstance().getLocationByKey(details.currentLocation);
                }
                catch (...)
                {
                }
            }
        }
        catch (...)
        {
            trainIDStream << std::setw(2) << std::setfill('0') << downloadStatus.trainNumber << "/_";
        }

        if (true == trainLocation.empty())
        {
            // use agent location
            trainLocation = m_locationName;
        }

        TA_Base_Core::NameValuePair trainNumber("Train", trainIDStream.str().c_str());
        desc.push_back(&trainNumber);

        TA_Base_Core::NameValuePair location("Train location", trainLocation);
        desc.push_back(&location);

        std::stringstream versionStream;
        versionStream << version;
        TA_Base_Core::NameValuePair versionPair("Version", versionStream.str());
        desc.push_back(&versionPair);

        TA_Base_Core::NameValuePair successful("Successful/Unsuccessful", downloadStatus.success ? "Successful" : "Unsuccessful");
        desc.push_back(&successful);

        switch (downloadStatus.type)
        {
            case TA_Base_Core::LibraryDownloadStart:

                LOG_INFO("Library Download Start");

                m_auditMessageSender->sendAuditMessage(TA_Base_Core::TISAudit::TTISLibraryDownloadStarted, m_entityKey,
                                                       desc,
                                                       "", // Further description text
                                                       "",
                                                       "", // alarm ID - not required
                                                       "", // incident key - not required
                                                       ""); // event key - not required

                break;

            case TA_Base_Core::LibraryDownloadFinish:

                LOG_INFO("Library Download Finish");

                m_auditMessageSender->sendAuditMessage(TA_Base_Core::TISAudit::TTISLibraryDownloadFinished, m_entityKey,
                                                       desc,
                                                       "", // Further description text
                                                       "",
                                                       "", // alarm ID - not required
                                                       "", // incident key - not required
                                                       ""); // event key - not required

                break;

            case TA_Base_Core::LibraryUpgrade:

                LOG_INFO("Library Upgrade");

                m_auditMessageSender->sendAuditMessage(TA_Base_Core::TISAudit::TTISTrainLibraryUpgrade, m_entityKey,
                                                       desc,
                                                       "", // Further description text
                                                       "",
                                                       "", // alarm ID - not required
                                                       "", // incident key - not required
                                                       ""); // event key - not required

                break;

            case TA_Base_Core::ScheduleDownloadStart:

                LOG_INFO("Schedule Download Start");

                m_auditMessageSender->sendAuditMessage(TA_Base_Core::TISAudit::TTISScheduleDownloadStarted, m_entityKey,
                                                       desc,
                                                       "", // Further description text
                                                       "",
                                                       "", // alarm ID - not required
                                                       "", // incident key - not required
                                                       ""); // event key - not required
                break;

            case TA_Base_Core::ScheduleDownloadFinish:

                LOG_INFO("Schedule Download Finish");

                m_auditMessageSender->sendAuditMessage(TA_Base_Core::TISAudit::TTISScheduleDownloadFinished, m_entityKey,
                                                       desc,
                                                       "", // Further description text
                                                       "",
                                                       "", // alarm ID - not required
                                                       "", // incident key - not required
                                                       ""); // event key - not required

                break;

            case TA_Base_Core::ScheduleUpgrade:

                LOG_INFO("Schedule Upgrade");

                m_auditMessageSender->sendAuditMessage(TA_Base_Core::TISAudit::TTISScheduleUpgrade, m_entityKey,
                                                       desc,
                                                       "", // Further description text
                                                       "",
                                                       "", // alarm ID - not required
                                                       "", // incident key - not required
                                                       ""); // event key - not required

                break;

            default:
                break;
        }

        FUNCTION_EXIT;
    }

    void TTISManager::sendLibraryVersionAlarmUpdate(const TA_Base_Core::TrainDataVersionAlarms& update)
    {
        FUNCTION_ENTRY("sendLibraryVersionAlarmUpdate");

        TA_ASSERT(m_tisStateUpdateMessageSender != NULL, "Tis Message Sender is null");

        if (false == AgentModeMonitor::getInstance().isInControlMode())
        {
            FUNCTION_EXIT;
            return;
        }

        LOG_INFO("Sending TisTrainDataVersionAlarm message.");

        // put the data into the message
        CORBA::Any data;
        data <<= update;

        m_tisStateUpdateMessageSender->sendStateUpdateMessage(TA_Base_Core::TISStateUpdate::TisTrainDataVersionAlarm,
                                                              m_entityKey,
                                                              TA_Base_Core::RunParams::getInstance().get(RPARAM_ENTITYNAME),
                                                              data, NULL);

        FUNCTION_EXIT;
    }

    std::string TTISManager::raiseLibraryVersionAlarm(TA_IRS_Bus::CommonTypes::TrainIdType trainId, unsigned short trainVersion, unsigned short iscsVersion)
    {
        FUNCTION_ENTRY("raiseLibraryVersionAlarm");
        FUNCTION_EXIT;
        return raiseVersionAlarm(TA_Base_Core::TISAlarms::TrainLibraryInconsistentWithISCS, trainId, trainVersion, iscsVersion);
    }

    std::string TTISManager::raiseScheduleVersionAlarm(TA_IRS_Bus::CommonTypes::TrainIdType trainId, unsigned short trainVersion, unsigned short iscsVersion)
    {
        FUNCTION_ENTRY("raiseScheduleVersionAlarm");
        FUNCTION_EXIT;
        return raiseVersionAlarm(TA_Base_Core::TISAlarms::TrainTimeScheduleInconsistentWithISCS, trainId, trainVersion, iscsVersion);
    }

    std::string TTISManager::raiseVersionAlarm(const TA_Base_Core::MessageType& alarmType, TA_IRS_Bus::CommonTypes::TrainIdType trainId, unsigned short trainVersion, unsigned short iscsVersion)
    {
        FUNCTION_ENTRY("raiseVersionAlarm");

        TA_Base_Core::DescriptionParameters dp;

        std::ostringstream versionStream;
        versionStream << trainVersion;
        TA_Base_Core::NameValuePair nvp1("OriginalVersion", versionStream.str());
        dp.push_back(&nvp1);

        std::ostringstream trainIDStream;

        try
        {
            // use PV/SERVICE string
            TA_IRS_Bus::TrainInformationTypes::TrainDetails details = m_trainAgentSubscriber->getTrainDetails(trainId);
            trainIDStream << details.serviceNumber;
        }
        catch (...)
        {
            trainIDStream << std::setw(2) << std::setfill('0') << trainId << "/_";
        }

        TA_Base_Core::NameValuePair nvp2("trainID", trainIDStream.str().c_str());
        dp.push_back(&nvp2);

        std::ostringstream iscsVersionStream;
        iscsVersionStream << iscsVersion;
        TA_Base_Core::NameValuePair nvp3("ISCSVersion", iscsVersionStream.str());
        dp.push_back(&nvp3);

        FUNCTION_EXIT;
        return TA_Base_Core::AlarmHelperManager::getInstance().getNonUniqueAlarmHelper().submitAlarmWithAsset(
            alarmType,
            m_entityKey,
            m_entityTypeKey,
            dp,
            m_entityName,
            m_locationKey,
            m_subsystemKey,
            StatusMonitor::getInstance()->getAgentAssetName(),
            TA_Base_Core::AlarmConstants::defaultTime);
    }

    void TTISManager::closeAllLibraryVersionAlarms()
    {
        FUNCTION_ENTRY("closeAllLibraryVersionAlarms");

        TA_Base_Core::AlarmHelperManager::getInstance().getAlarmHelper().closeAlarmAtLocation(
            TA_Base_Core::TISAlarms::TrainLibraryInconsistentWithISCS, m_entityKey, m_locationKey);

        TA_Base_Core::AlarmHelperManager::getInstance().getAlarmHelper().closeAlarmAtLocation(
            TA_Base_Core::TISAlarms::TrainTimeScheduleInconsistentWithISCS, m_entityKey, m_locationKey);

        FUNCTION_EXIT;
    }

    void TTISManager::closeAlarm(const std::string& alarmId)
    {
        FUNCTION_ENTRY("closeAlarm");

        TA_Base_Core::AlarmHelperManager::getInstance().getNonUniqueAlarmHelper().closeAlarmAtLocation(alarmId, m_locationKey);

        FUNCTION_EXIT;
    }

    void TTISManager::raiseRatisDisplayRequestAlarm(const TA_IRS_Bus::CommonTypes::TrainIdList& failedTrains)
    {
        FUNCTION_ENTRY("raiseRatisDisplayRequestAlarm");

        std::ostringstream failedStream;

        for (TA_IRS_Bus::CommonTypes::TrainIdList::const_iterator iter = failedTrains.begin();
             iter != failedTrains.end(); ++iter)
        {
            try
            {
                // use PV/SERVICE string
                TA_IRS_Bus::TrainInformationTypes::TrainDetails details = m_trainAgentSubscriber->getTrainDetails(*iter);
                failedStream << details.serviceNumber << " ";
            }
            catch (...)
            {
                failedStream << std::setw(2) << std::setfill('0') << *iter << "/_ ";
            }
        }

        TA_Base_Core::DescriptionParameters dp;
        TA_Base_Core::NameValuePair nvp("List", failedStream.str());
        dp.push_back(&nvp);

        TA_Base_Core::AlarmHelperManager::getInstance().getNonUniqueAlarmHelper().submitAlarmWithAsset(
            TA_Base_Core::TISAlarms::IncomingRatisDisplayRequestError,
            m_entityKey,
            m_entityTypeKey,
            dp,
            m_entityName,
            m_locationKey,
            m_subsystemKey,
            StatusMonitor::getInstance()->getAgentAssetName(),
            TA_Base_Core::AlarmConstants::defaultTime);

        FUNCTION_EXIT;
    }

    TA_IRS_Bus::CommonTypes::TrainIdList TTISManager::convertTrainList(const TA_Base_Core::TrainList& trainList)
    {
        FUNCTION_ENTRY("convertTrainList");

        TA_IRS_Bus::CommonTypes::TrainIdList result;

        for (unsigned int i = 0; i < trainList.length(); ++i)
        {
            result.insert(trainList[i]);
        }

        FUNCTION_EXIT;
        return result;
    }

    TA_IRS_Bus::TtisTypes::LibraryDataType TTISManager::loadCurrentTimeSchedule(unsigned long scheduleKey)
    {
        FUNCTION_ENTRY("loadCurrentTimeSchedule");

        TA_IRS_Bus::TtisTypes::LibraryDataType scheduleData;
        TA_IRS_Core::ITrainTimeSchedule* currentTrainTimeSchedule = NULL;

        try
        {
            // stream to hold all of the content
            std::ostringstream timeScheduleByteStream;

            // retrieve latest version from the database
            currentTrainTimeSchedule = TA_IRS_Core::TrainTimeScheduleAccessFactory::getInstance().getTrainTimeSchedule(scheduleKey);

            // Now build the stream of bytes to store the train time schedule

            // Version number
            TA_Base_Core::TimeScheduleVersion timeSchedule;
            CORBA_CALL_RETURN(timeSchedule, TISAgentAccessFactory::getInstance().getOccSTISAgent(), getCurrentTrainTimeScheduleVersion, ());
            timeScheduleByteStream << timeSchedule.version << CR << LF;

            // Six Adhoc messages
            timeScheduleByteStream << currentTrainTimeSchedule->getAdHocMessage(TA_IRS_Core::ITrainTimeSchedule::Adhoc1) << CR << LF;
            timeScheduleByteStream << currentTrainTimeSchedule->getAdHocMessage(TA_IRS_Core::ITrainTimeSchedule::Adhoc2) << CR << LF;
            timeScheduleByteStream << currentTrainTimeSchedule->getAdHocMessage(TA_IRS_Core::ITrainTimeSchedule::Adhoc3) << CR << LF;
            timeScheduleByteStream << currentTrainTimeSchedule->getAdHocMessage(TA_IRS_Core::ITrainTimeSchedule::Adhoc4) << CR << LF;
            timeScheduleByteStream << currentTrainTimeSchedule->getAdHocMessage(TA_IRS_Core::ITrainTimeSchedule::Adhoc5) << CR << LF;
            timeScheduleByteStream << currentTrainTimeSchedule->getAdHocMessage(TA_IRS_Core::ITrainTimeSchedule::Adhoc6) << CR << LF;

            std::vector<TA_IRS_Core::ITrainTimeScheduleEntry*> timeScheduleEntries = currentTrainTimeSchedule->getEntries();

            std::vector<TA_IRS_Core::ITrainTimeScheduleEntry*>::iterator iter;

            for (iter = timeScheduleEntries.begin(); iter != timeScheduleEntries.end(); iter++)
            {
                time_t startTime;
                time_t endTime;
                startTime = (*iter)->getStartTime();
                endTime = (*iter)->getEndTime();

                tm startTimeStruct;
                tm endTimeStruct;

                ACE_OS::localtime_r(&startTime, &startTimeStruct);
                ACE_OS::localtime_r(&endTime, &endTimeStruct);

                // Add the start and end time to the string
                timeScheduleByteStream << std::setw(2) << std::setfill('0') << startTimeStruct.tm_hour;
                timeScheduleByteStream << std::setw(2) << std::setfill('0') << startTimeStruct.tm_min;

                timeScheduleByteStream << std::setw(2) << std::setfill('0') << endTimeStruct.tm_hour;
                timeScheduleByteStream << std::setw(2) << std::setfill('0') << endTimeStruct.tm_min;

                // Repeat Interval
                int repeatInterval = (*iter)->getRepeatInterval();
                timeScheduleByteStream << std::setw(3) << std::setfill('0') << repeatInterval;

                // Message Table
                timeScheduleByteStream << (*iter)->getMessageTable();

                // Message ID
                int messageID = (*iter)->getMessageId();
                timeScheduleByteStream << std::setw(3) << std::setfill('0') << messageID;

                // Priority
                timeScheduleByteStream << (*iter)->getPriority();

                // End of entry bits n bobs
                timeScheduleByteStream << CR << LF;
            }

            // clear the time schedule entries
            // but dont delete the entries, they belong to the schedule and will
            // be deleted with it
            timeScheduleEntries.clear();

            // convert the stream to raw bytes
            std::string timeScheduleString;
            timeScheduleString = timeScheduleByteStream.str();

            for (std::string::iterator byteIter = timeScheduleString.begin();
                 byteIter != timeScheduleString.end(); ++byteIter)
            {
                scheduleData.push_back(static_cast<TA_IRS_Bus::TtisTypes::LibraryDataType::value_type> (*byteIter));
            }

            delete currentTrainTimeSchedule;
            currentTrainTimeSchedule = NULL;
        }
        catch (...)
        {
            delete currentTrainTimeSchedule;
            currentTrainTimeSchedule = NULL;

            FUNCTION_EXIT;
            throw;
        }

        FUNCTION_EXIT;
        return scheduleData;
    }

    TA_IRS_Bus::TtisTypes::TtisPredefinedDisplayParameters TTISManager::convertDisplayParameters(const TA_Base_Core::TTISPredefinedMessageParameters displayParameters)
    {
        FUNCTION_ENTRY("convertDisplayParameters");

        TA_IRS_Bus::TtisTypes::TtisPredefinedDisplayParameters result;

        result.pidList = convertTisPidListToTrain(displayParameters.pidList);
        result.libraryVersion = displayParameters.libraryVersion;
        result.identifierType = convertTisLibrarySectionToTrain(displayParameters.librarySection);
        result.identifierOffset = displayParameters.messageId;
        result.priority = convertTisMessagePriorityToTrain(displayParameters.priority);
        result.displayPeriod = convertTisTimeIntervalToTrain(displayParameters.startTime.in(), displayParameters.endTime.in());

        FUNCTION_EXIT;
        return result;
    }

    TA_IRS_Bus::TtisTypes::TtisFreeTextDisplayParameters TTISManager::convertDisplayParameters(const TA_Base_Core::TTISFreeTextMessageParameters& displayParameters)
    {
        FUNCTION_ENTRY("convertDisplayParameters");

        TA_IRS_Bus::TtisTypes::TtisFreeTextDisplayParameters result;

        result.pidList = convertTisPidListToTrain(displayParameters.pidList);
        result.fontSize = convertTisLedFontSizeToTrain(displayParameters.fontSize);
        result.justification = convertTisJustificationToTrain(displayParameters.justification);
        result.intensity = convertTisIntensityToTrain(displayParameters.intensity);
        result.displayMode = convertTisDisplayModeToTrain(displayParameters.displayMode);
        result.priority = convertTisMessagePriorityToTrain(displayParameters.priority);
        result.displayPeriod = convertTisTimeIntervalToTrain(displayParameters.startTime.in(), displayParameters.endTime.in());
        result.repeatInterval = displayParameters.repeatInterval;
        const std::string messageText = displayParameters.message.in();
        result.message.assign(messageText.begin(), messageText.end());

        FUNCTION_EXIT;
        return result;
    }

    TA_IRS_Bus::TtisTypes::TtisResetParameters TTISManager::convertDisplayParameters(const TA_Base_Core::TTISMessageResetParameters& displayParameters)
    {
        FUNCTION_ENTRY("convertDisplayParameters");

        TA_IRS_Bus::TtisTypes::TtisResetParameters result;

        result.pidList = convertTisPidListToTrain(displayParameters.pidList);
        result.messageType = convertTisClearTypeToTrain(displayParameters.messageType);

        FUNCTION_EXIT;
        return result;
    }

    TA_IRS_Bus::TtisTypes::TtisPidList TTISManager::convertTisPidListToTrain(const TA_Base_Core::TrainPIDList& pidList)
    {
        FUNCTION_ENTRY("convertTisPidListToTrain");

        TA_IRS_Bus::TtisTypes::TtisPidList result;

        for (unsigned int i = 0; i < pidList.length(); ++i)
        {
            result.push_back(convertTisPidToTrain(pidList[i]));
        }

        FUNCTION_EXIT;
        return result;
    }

    TA_IRS_Bus::ProtocolTtisTypes::ETtisPid TTISManager::convertTisPidToTrain(TA_Base_Core::EPIDSelection pid)
    {
        FUNCTION_ENTRY("convertTisPidToTrain");

        switch (pid)
        {
            case TA_Base_Core::ENTIRE_TRAIN:
                FUNCTION_EXIT;
                return TA_IRS_Bus::ProtocolTtisTypes::TtisPidAllTrain;
                break;

            case TA_Base_Core::ALL_CAR1:
                FUNCTION_EXIT;
                return TA_IRS_Bus::ProtocolTtisTypes::TtisPidCar1;
                break;

            case TA_Base_Core::ALL_CAR2:
                FUNCTION_EXIT;
                return TA_IRS_Bus::ProtocolTtisTypes::TtisPidCar2;
                break;

            case TA_Base_Core::ALL_CAR3:
                FUNCTION_EXIT;
                return TA_IRS_Bus::ProtocolTtisTypes::TtisPidCar3;
                break;

            case TA_Base_Core::CAR1_PID1:
                FUNCTION_EXIT;
                return TA_IRS_Bus::ProtocolTtisTypes::TtisPidCar1Pid1;
                break;

            case TA_Base_Core::CAR1_PID2:
                FUNCTION_EXIT;
                return TA_IRS_Bus::ProtocolTtisTypes::TtisPidCar1Pid2;
                break;

            case TA_Base_Core::CAR1_PID3:
                FUNCTION_EXIT;
                return TA_IRS_Bus::ProtocolTtisTypes::TtisPidCar1Pid3;
                break;

            case TA_Base_Core::CAR1_PID4:
                FUNCTION_EXIT;
                return TA_IRS_Bus::ProtocolTtisTypes::TtisPidCar1Pid4;
                break;

            case TA_Base_Core::CAR2_PID1:
                FUNCTION_EXIT;
                return TA_IRS_Bus::ProtocolTtisTypes::TtisPidCar2Pid1;
                break;

            case TA_Base_Core::CAR2_PID2:
                FUNCTION_EXIT;
                return TA_IRS_Bus::ProtocolTtisTypes::TtisPidCar2Pid2;
                break;

            case TA_Base_Core::CAR2_PID3:
                FUNCTION_EXIT;
                return TA_IRS_Bus::ProtocolTtisTypes::TtisPidCar2Pid3;
                break;

            case TA_Base_Core::CAR2_PID4:
                FUNCTION_EXIT;
                return TA_IRS_Bus::ProtocolTtisTypes::TtisPidCar2Pid4;
                break;

            case TA_Base_Core::CAR3_PID1:
                FUNCTION_EXIT;
                return TA_IRS_Bus::ProtocolTtisTypes::TtisPidCar3Pid1;
                break;

            case TA_Base_Core::CAR3_PID2:
                FUNCTION_EXIT;
                return TA_IRS_Bus::ProtocolTtisTypes::TtisPidCar3Pid2;
                break;

            case TA_Base_Core::CAR3_PID3:
                FUNCTION_EXIT;
                return TA_IRS_Bus::ProtocolTtisTypes::TtisPidCar3Pid3;
                break;

            case TA_Base_Core::CAR3_PID4:
                FUNCTION_EXIT;
                return TA_IRS_Bus::ProtocolTtisTypes::TtisPidCar3Pid4;
                break;

            default:

                FUNCTION_EXIT;
                throw TA_Base_Bus::ITTISManagerCorbaDef::TTISInvalidParameter("Invalid PID selection");
                break;
        }

        FUNCTION_EXIT;
        return TA_IRS_Bus::ProtocolTtisTypes::TtisPidAllTrain;
    }

    TA_IRS_Bus::ProtocolTtisTypes::ETtisResetMessageType TTISManager::convertTisClearTypeToTrain(TA_Base_Core::ETTISClearType messageType)
    {
        FUNCTION_ENTRY("convertTisClearTypeToTrain");

        //convert the enumeration from TISMessageDataDefinitions.idl to the C++ enumeration from TrainCommonLibrary
        //throw an exception if the conversion cant be performed

        switch (messageType)
        {
            case TA_Base_Core::CLEAR_ALL:
                FUNCTION_EXIT;
                return TA_IRS_Bus::ProtocolTtisTypes::TtisResetAll;
                break;

            case TA_Base_Core::CLEAR_EMERGENCY:
                FUNCTION_EXIT;
                return TA_IRS_Bus::ProtocolTtisTypes::TtisResetEmergency;
                break;

            case TA_Base_Core::CLEAR_NORMAL:
                FUNCTION_EXIT;
                return TA_IRS_Bus::ProtocolTtisTypes::TtisResetNormal;
                break;

            case TA_Base_Core::CLEAR_PRIORITY6:
                FUNCTION_EXIT;
                return TA_IRS_Bus::ProtocolTtisTypes::TtisResetPriority6;
                break;

            default:

                FUNCTION_EXIT;
                throw TA_Base_Bus::ITTISManagerCorbaDef::TTISInvalidParameter("Invalid PID messageType");
                break;
        }

        FUNCTION_EXIT;
        return TA_IRS_Bus::ProtocolTtisTypes::TtisResetAll;
    }

    unsigned char TTISManager::convertTisLibrarySectionToTrain(TA_Base_Core::ELibrarySection librarySection)
    {
        FUNCTION_ENTRY("convertTisLibrarySectionToTrain");

        //convert the enumeration from TISMessageDataDefinitions.idl to the C++ enumeration from TrainCommonLibrary
        //throw an exception if the conversion cant be performed

        switch (librarySection)
        {
            case TA_Base_Core::EMERGENCY_SECTION:
                FUNCTION_EXIT;
                return 0;
                break;

            case TA_Base_Core::NORMAL_SECTION:
                FUNCTION_EXIT;
                return 1;
                break;

            default:

                FUNCTION_EXIT;
                throw TA_Base_Bus::ITTISManagerCorbaDef::TTISInvalidParameter("Invalid librarySection");
                break;
        }

        FUNCTION_EXIT;
        return 0;
    }

    TA_IRS_Bus::ProtocolTtisTypes::ETtisPriority TTISManager::convertTisMessagePriorityToTrain(TA_Base_Core::ETTISMessagePriority priority)
    {
        FUNCTION_ENTRY("convertTisMessagePriorityToTrain");

        //convert the enumeration from TISMessageDataDefinitions.idl to the C++ enumeration from TrainCommonLibrary
        //throw an exception if the conversion cant be performed

        switch (priority)
        {
            case TA_Base_Core::TTIS_PRIORITY_1:
                FUNCTION_EXIT;
                return TA_IRS_Bus::ProtocolTtisTypes::ETtisPriorityOne;
                break;

            case TA_Base_Core::TTIS_PRIORITY_2:
                FUNCTION_EXIT;
                return TA_IRS_Bus::ProtocolTtisTypes::ETtisPriorityTwo;
                break;

            case TA_Base_Core::TTIS_PRIORITY_3:
                FUNCTION_EXIT;
                return TA_IRS_Bus::ProtocolTtisTypes::ETtisPriorityThree;
                break;

            case TA_Base_Core::TTIS_PRIORITY_4:
                FUNCTION_EXIT;
                return TA_IRS_Bus::ProtocolTtisTypes::ETtisPriorityFour;
                break;

            case TA_Base_Core::TTIS_PRIORITY_5:
                FUNCTION_EXIT;
                return TA_IRS_Bus::ProtocolTtisTypes::ETtisPriorityFive;
                break;

            case TA_Base_Core::TTIS_PRIORITY_6:
                FUNCTION_EXIT;
                return TA_IRS_Bus::ProtocolTtisTypes::ETtisPrioritySix;
                break;

            case TA_Base_Core::TTIS_PRIORITY_7:
                FUNCTION_EXIT;
                return TA_IRS_Bus::ProtocolTtisTypes::ETtisPrioritySeven;
                break;

            case TA_Base_Core::TTIS_PRIORITY_8:
                FUNCTION_EXIT;
                return TA_IRS_Bus::ProtocolTtisTypes::ETtisPriorityEight;
                break;

            default:

                FUNCTION_EXIT;
                throw TA_Base_Bus::ITTISManagerCorbaDef::TTISInvalidParameter("Invalid priority");
                break;
        }

        FUNCTION_EXIT;
        return TA_IRS_Bus::ProtocolTtisTypes::ETtisPriorityOne;
    }

    TA_IRS_Bus::ProtocolTtisTypes::ETtisPriority TTISManager::convertTisPriorityToTrain(int priority)
    {
        FUNCTION_ENTRY("convertTisPriorityToTrain");

        //convert the integer to the C++ enumeration from TrainCommonLibrary
        //throw an exception if the conversion cant be performed

        switch (priority)
        {
            case 1:
                FUNCTION_EXIT;
                return TA_IRS_Bus::ProtocolTtisTypes::ETtisPriorityOne;
                break;

            case 2:
                FUNCTION_EXIT;
                return TA_IRS_Bus::ProtocolTtisTypes::ETtisPriorityTwo;
                break;

            case 3:
                FUNCTION_EXIT;
                return TA_IRS_Bus::ProtocolTtisTypes::ETtisPriorityThree;
                break;

            case 4:
                FUNCTION_EXIT;
                return TA_IRS_Bus::ProtocolTtisTypes::ETtisPriorityFour;
                break;

            case 5:
                FUNCTION_EXIT;
                return TA_IRS_Bus::ProtocolTtisTypes::ETtisPriorityFive;
                break;

            case 6:
                FUNCTION_EXIT;
                return TA_IRS_Bus::ProtocolTtisTypes::ETtisPrioritySix;
                break;

            case 7:
                FUNCTION_EXIT;
                return TA_IRS_Bus::ProtocolTtisTypes::ETtisPrioritySeven;
                break;

            case 8:
                FUNCTION_EXIT;
                return TA_IRS_Bus::ProtocolTtisTypes::ETtisPriorityEight;
                break;

            default:

                FUNCTION_EXIT;
                throw TA_Base_Bus::ITTISManagerCorbaDef::TTISInvalidParameter("Invalid priority");
                break;
        }

        FUNCTION_EXIT;
        return TA_IRS_Bus::ProtocolTtisTypes::ETtisPriorityOne;
    }

    TA_IRS_Bus::TtisTypes::TimeInterval TTISManager::convertTisTimeIntervalToTrain(const std::string& startTime, const std::string& endTime)
    {
        FUNCTION_ENTRY("convertTisTimeIntervalToTrain");

        TA_IRS_Bus::TtisTypes::TimeInterval displayPeriod;
        std::string temp;

        // Get the start hour
        temp = startTime.substr(0, 2);
        displayPeriod.startHour = (unsigned char)atoi(temp.c_str());

        temp.empty();
        // get the start minutes
        temp = startTime.substr(2, 2);
        displayPeriod.startMin = (unsigned char)atoi(temp.c_str());

        temp.empty();
        // Get the end hour
        temp = endTime.substr(0, 2);
        displayPeriod.stopHour = (unsigned char)atoi(temp.c_str());

        temp.empty();
        // get the end minutes
        temp = endTime.substr(2, 2);
        displayPeriod.stopMin = (unsigned char)atoi(temp.c_str());

        FUNCTION_EXIT;
        return displayPeriod;
    }

    TA_IRS_Bus::ProtocolTtisTypes::ETtisFontSize TTISManager::convertTisLedFontSizeToTrain(TA_Base_Core::ETTISLEDFontSize fontSize)
    {
        FUNCTION_ENTRY("convertTisLedFontSizeToTrain");

        //convert the enumeration from TISMessageDataDefinitions.idl to the C++ enumeration from TrainCommonLibrary
        //throw an exception if the conversion cant be performed

        switch (fontSize)
        {
            case TA_Base_Core::TTIS_FONT_SIZE_SMALL:
                FUNCTION_EXIT;
                return TA_IRS_Bus::ProtocolTtisTypes::TtisFontSmall;
                break;

            case TA_Base_Core::TTIS_FONT_SIZE_LARGE:
                FUNCTION_EXIT;
                return TA_IRS_Bus::ProtocolTtisTypes::TtisFontLarge;
                break;

            default:

                FUNCTION_EXIT;
                throw TA_Base_Bus::ITTISManagerCorbaDef::TTISInvalidParameter("Invalid fontSize");
                break;
        }

        FUNCTION_EXIT;
        return TA_IRS_Bus::ProtocolTtisTypes::TtisFontSmall;
    }

    TA_IRS_Bus::ProtocolTtisTypes::ETtisFontSize TTISManager::convertTisFontSizeToTrain(TA_Base_Core::ELEDFontSize fontSize)
    {
        FUNCTION_ENTRY("convertTisFontSizeToTrain");

        //convert the enumeration from TISMessageDataDefinitions.idl to the C++ enumeration from TrainCommonLibrary
        //throw an exception if the conversion cant be performed

        switch (fontSize)
        {
            case TA_Base_Core::FONT_SIZE_SMALL:
                FUNCTION_EXIT;
                return TA_IRS_Bus::ProtocolTtisTypes::TtisFontSmall;
                break;

            case TA_Base_Core::FONT_SIZE_MEDIUM:
            case TA_Base_Core::FONT_SIZE_LARGE:
            case TA_Base_Core::UNUSED_LED_FONT_SIZE:
                FUNCTION_EXIT;
                return TA_IRS_Bus::ProtocolTtisTypes::TtisFontLarge;
                break;

            default:

                FUNCTION_EXIT;
                throw TA_Base_Bus::ITTISManagerCorbaDef::TTISInvalidParameter("Invalid fontSize");
                break;
        }

        FUNCTION_EXIT;
        return TA_IRS_Bus::ProtocolTtisTypes::TtisFontSmall;
    }

    TA_IRS_Bus::ProtocolTtisTypes::ETtisJustification TTISManager::convertTisJustificationToTrain(TA_Base_Core::EJustification justifcation)
    {
        FUNCTION_ENTRY("convertTisJustificationToTrain");

        //convert the enumeration from TISMessageDataDefinitions.idl to the C++ enumeration from TrainCommonLibrary
        //throw an exception if the conversion cant be performed

        switch (justifcation)
        {
            case TA_Base_Core::LEFT:
                FUNCTION_EXIT;
                return TA_IRS_Bus::ProtocolTtisTypes::TtisJustificationLeft;
                break;

            case TA_Base_Core::CENTRED:
                FUNCTION_EXIT;
                return TA_IRS_Bus::ProtocolTtisTypes::TtisJustificationCentre;
                break;

            case TA_Base_Core::RIGHT:
                FUNCTION_EXIT;
                return TA_IRS_Bus::ProtocolTtisTypes::TtisJustificationRight;
                break;

            default:

                FUNCTION_EXIT;
                throw TA_Base_Bus::ITTISManagerCorbaDef::TTISInvalidParameter("Invalid justifcation");
                break;
        }

        FUNCTION_EXIT;
        return TA_IRS_Bus::ProtocolTtisTypes::TtisJustificationLeft;
    }

    TA_IRS_Bus::ProtocolTtisTypes::ETtisIntensity TTISManager::convertTisIntensityToTrain(TA_Base_Core::ETTISLEDIntensity intensity)
    {
        FUNCTION_ENTRY("convertTisIntensityToTrain");

        //convert the enumeration from TISMessageDataDefinitions.idl to the C++ enumeration from TrainCommonLibrary
        //throw an exception if the conversion cant be performed

        switch (intensity)
        {
            case TA_Base_Core::NORMAL:
                FUNCTION_EXIT;
                return TA_IRS_Bus::ProtocolTtisTypes::TtisIntensityNormal;
                break;

            case TA_Base_Core::HIGH:
                FUNCTION_EXIT;
                return TA_IRS_Bus::ProtocolTtisTypes::TtisIntensityHigh;
                break;

            default:

                FUNCTION_EXIT;
                throw TA_Base_Bus::ITTISManagerCorbaDef::TTISInvalidParameter("Invalid intensity");
                break;
        }

        FUNCTION_EXIT;
        return TA_IRS_Bus::ProtocolTtisTypes::TtisIntensityNormal;
    }

    TA_IRS_Bus::ProtocolTtisTypes::ETtisDisplayMode TTISManager::convertTisDisplayModeToTrain(TA_Base_Core::EDisplayMode displayMode)
    {
        FUNCTION_ENTRY("convertTisDisplayModeToTrain");

        //convert the enumeration from TISMessageDataDefinitions.idl to the C++ enumeration from TrainCommonLibrary
        //throw an exception if the conversion cant be performed

        switch (displayMode)
        {
            case TA_Base_Core::SCROLL_LEFT:
                FUNCTION_EXIT;
                return TA_IRS_Bus::ProtocolTtisTypes::TtisDisplayModeScrollLeft;
                break;

            case TA_Base_Core::SCROLL_RIGHT:
                FUNCTION_EXIT;
                return TA_IRS_Bus::ProtocolTtisTypes::TtisDisplayModeScrollRight;
                break;

            case TA_Base_Core::SCROLL_UP:
                FUNCTION_EXIT;
                return TA_IRS_Bus::ProtocolTtisTypes::TtisDisplayModeScrollUp;
                break;

            case TA_Base_Core::SCROLL_DOWN:
                FUNCTION_EXIT;
                return TA_IRS_Bus::ProtocolTtisTypes::TtisDisplayModeScrollDown;
                break;

            case TA_Base_Core::INSTANT_ON:
                FUNCTION_EXIT;
                return TA_IRS_Bus::ProtocolTtisTypes::TtisDisplayModeInstantOn;
                break;

            case TA_Base_Core::BLINKING:
                FUNCTION_EXIT;
                return TA_IRS_Bus::ProtocolTtisTypes::TtisDisplayModeBlinking;
                break;

            case TA_Base_Core::WIPING:
                FUNCTION_EXIT;
                return TA_IRS_Bus::ProtocolTtisTypes::TtisDisplayModeWiping;
                break;

            case TA_Base_Core::SNOW:
                FUNCTION_EXIT;
                return TA_IRS_Bus::ProtocolTtisTypes::TtisDisplayModeLikeSnow;
                break;

            default:

                FUNCTION_EXIT;
                throw TA_Base_Bus::ITTISManagerCorbaDef::TTISInvalidParameter("Invalid displayMode");
                break;
        }

        FUNCTION_EXIT;
        return TA_IRS_Bus::ProtocolTtisTypes::TtisDisplayModeScrollLeft;
    }

    TA_Base_Core::ETTISClearType TTISManager::getClearTypeFromPriority(const short priority) const
    {
        FUNCTION_ENTRY("getClearTypeFromPriority");

        if (priority == 6)
        {
            FUNCTION_EXIT;
            return TA_Base_Core::CLEAR_PRIORITY6;
        }
        else if (priority < 4) // realistically is 1,2,3
        {
            FUNCTION_EXIT;
            return TA_Base_Core::CLEAR_EMERGENCY;
        }

        // all others (realistically 4,5,7,8)
        FUNCTION_EXIT;
        return TA_Base_Core::CLEAR_NORMAL;
    }
}
