/**
 * The source code in this file is the property of
 * Ripple Systems and is not for redistribution
 * in any form.
 *
 * Source:   $File: //depot/C830AR_TIP/C830AR/transactive/app/pa/PAAgent/src/Table1610.cpp $
 * @author:  Ripple
 * @version: $Revision: #7 $
 *
 * Last modification: $DateTime: 2025/09/11 17:51:01 $
 * Last modified by:  $Author: limin.zhu $
 *
 */

#include "pch.h"
#include "Table1610.h"
#include "PasHelpers.h"
#include "PasTableManager.h"
#include "PASConnection.h"
#include "CachedConfig.h"
#include "bus/pa/cachedmaps/src/PaDvaVersionCachedMap.h"
#include "bus/pa/cachedmaps/src/PaDvaPublicVersionCachedMap.h"
#include "core/utilities/src/TAAssert.h"
#include "core/utilities/src/DebugUtilEx.h"
#include "core/message/src/MessagePublicationManager.h"
#include "core/message/types/PAAgentComms_MessageTypes.h"
#include "core/message/src/CommsMessageSender.h"
#include "core/data_access_interface/src/LocationAccessFactoryEx.h"

using namespace TA_Base_Core;
using namespace TA_IRS_Bus;

const size_t TABLE_1610_PRIVATE_ADHOC_1_SUB_OFFSET = 0;
const size_t TABLE_1610_PRIVATE_ADHOC_2_SUB_OFFSET = 6;
const size_t TABLE_1610_PRIVATE_ADHOC_3_SUB_OFFSET = 12;
const size_t TABLE_1610_PRIVATE_ADHOC_4_SUB_OFFSET = 18;
const size_t TABLE_1610_PRIVATE_ADHOC_5_SUB_OFFSET = 24;
const size_t TABLE_1610_PRIVATE_ADHOC_6_SUB_OFFSET = 30;
const size_t TABLE_1610_PRIVATE_PRERECORDED_SUB_OFFSET = 36;

const size_t TABLE_1610_PUBLIC_ADHOC_1_SUB_OFFSET = 42;
const size_t TABLE_1610_PUBLIC_ADHOC_2_SUB_OFFSET = 48;
const size_t TABLE_1610_PUBLIC_ADHOC_3_SUB_OFFSET = 54;
const size_t TABLE_1610_PUBLIC_ADHOC_4_SUB_OFFSET = 60;
const size_t TABLE_1610_PUBLIC_ADHOC_5_SUB_OFFSET = 66;
const size_t TABLE_1610_PUBLIC_ADHOC_6_SUB_OFFSET = 72;
const size_t TABLE_1610_PUBLIC_PRERECORDED_SUB_OFFSET = 78;

const size_t TABLE_1610_RECORD_SIZE = 84;

namespace TA_IRS_App
{
    // ExceptionChecked
    Table1610::Table1610()
        : AbstractPASTable(1610)
    {
        LOG_SCOPE("Table1610::Table1610");

        FUNCTION_ENTRY("Constructor");
        TA_ASSERT(CachedConfig::getInstance()->getIsMaster(), "Master only table created for station");

        // Used to avoid comparing DVA SET A for Depot
        m_depotLocationKey = LocationDepot::key();

        // Initialise with null values for all configured locations

        m_dvaVersionRecordMap.clear();

        TA_Base_Bus::IPAAgentCorbaDef::DvaPublicVersionRecord newDvaVersionRecord;

        //Maochun Sun++
        //TD13407
        DvaPublicVersionRecords allVersionData = TA_IRS_Bus::CachedMaps::getInstance()->getDvaPublicVersionCachedMap().getAllRecords();

        //for (int stationId=1; stationId<=MAXSTNID; ++stationId)
        for (DvaPublicVersionRecordsIt it = allVersionData.begin(); it != allVersionData.end(); it++)
            //++Maochun Sun
            //TD13407
        {
            //Maochun Sun++
            //TD13407
            //unsigned long locationKey;
            //++Maochun Sun
            //TD13407
            try
            {
                //Maochun Sun++
                //TD13407

                //locationKey = CachedMaps::getInstance()->getLocationKeyFromPasStationId(stationId);

                //m_dvaVersionRecordMap[locationKey] = newDvaVersionRecord;
                //m_dvaVersionRecordMap[locationKey].m_locationKey = locationKey;
                //m_dvaVersionRecordMap[locationKey].m_pKey = DatabaseKey::getInvalidKey(); // Unknown

                //          newDvaVersionRecord.m_privateAdhoc1 = ((*it).m_privateAdhoc1).c_str();
                //          newDvaVersionRecord.m_privateAdhoc2 = ((*it).m_privateAdhoc2).c_str();
                //          newDvaVersionRecord.m_privateAdhoc3 = ((*it).m_privateAdhoc3).c_str();
                //          newDvaVersionRecord.m_privateAdhoc4 = ((*it).m_privateAdhoc4).c_str();
                //          newDvaVersionRecord.m_privateAdhoc5 = ((*it).m_privateAdhoc5).c_str();
                //          newDvaVersionRecord.m_privateAdhoc6 = ((*it).m_privateAdhoc6).c_str();
                //          newDvaVersionRecord.m_privatePreRecorded = ((*it).m_privatePreRecorded).c_str();
                newDvaVersionRecord.m_publicAdhoc1 = ((*it).m_publicAdhoc1).c_str();
                newDvaVersionRecord.m_publicAdhoc2 = ((*it).m_publicAdhoc2).c_str();
                newDvaVersionRecord.m_publicAdhoc3 = ((*it).m_publicAdhoc3).c_str();
                newDvaVersionRecord.m_publicAdhoc4 = ((*it).m_publicAdhoc4).c_str();
                newDvaVersionRecord.m_publicAdhoc5 = ((*it).m_publicAdhoc5).c_str();
                newDvaVersionRecord.m_publicAdhoc6 = ((*it).m_publicAdhoc6).c_str();
                newDvaVersionRecord.m_publicPreRecorded = ((*it).m_publicPreRecorded).c_str();

                m_dvaVersionRecordMap[(*it).m_locationKey] = newDvaVersionRecord;
                m_dvaVersionRecordMap[(*it).m_locationKey].m_locationKey = (*it).m_locationKey;
                m_dvaVersionRecordMap[(*it).m_locationKey].m_pKey = (*it).m_pKey;

                //++Maochun Sun
                //TD13407
            }
            catch (TA_Base_Core::CachedMappingNotFoundException&)
            {
                // This is expected as not all of the PAS stations (1-50) are defined
                // Just ignore
            }
        }

        FUNCTION_EXIT;
    }

    void Table1610::processRead()
    {
        LOG_SCOPE("Table1610::processRead");

        ThreadGuard guard(m_lock);

        unsigned long stationOffset = 0;
        bool dataChanged = false;

        for (int stationId = 1; stationId <= MAXSTNID; ++stationId)
        {
            LOG_SCOPE(std::to_string(stationId));
            unsigned long locationKey = 0;

            try
            {
                locationKey = CachedMaps::getInstance()->getLocationKeyFromPasStationId(stationId);
                LOG_SCOPE(LocationX{locationKey}.name());

                // Important that processLocalDvaVersionRecord is before dataChanged due to
                // short-circuit evaluation of the OR operator
                LocationKeyToDvaVersionRecordMapIt findIter = m_dvaVersionRecordMap.find(locationKey);

                if (findIter != m_dvaVersionRecordMap.end())
                {
                    dataChanged |= processLocalDvaVersionRecord(m_dvaVersionRecordMap[locationKey], stationOffset);
                }
                else
                {
                    LOG_ERROR("Mising DvaPublicVersionCachedMap in table PA_DVA_MESSAGE_PUBLIC_VERSION for location: %lu", locationKey);
                }
            }
            catch (TA_Base_Core::CachedMappingNotFoundException&)
            {
                // This is expected as not all of the PAS stations (1-50) are defined
                // Just ignore
            }

            stationOffset += TABLE_1610_RECORD_SIZE;
        }

        // Send changed data to the OCC PA Managers so they will get the latest information
        // on the version conflicts ASAP (i.e. before the database has been changed by the
        // station agents via table 610
        if (dataChanged)
        {
            LOG_INFO("Sending DvaVersionsUpdate message to subscribers");

            CORBA::Any data;

            data <<= getDvaVersionRecordsCopy();

            m_paAgentCommsSender->sendCommsMessage(
                PAAgentComms::DvaVersionsUpdate,                        // Context::Type
                CachedConfig::getInstance()->getAgentKey(),             // EntityKey
                data,                                                   // Data
                CachedConfig::getInstance()->getAgentSubsystemKey(),    // Subsystem
                CachedConfig::getInstance()->getAgentLocationKey(),     // LocationKey
                NULL);
        }
    }

    bool Table1610::processLocalDvaVersionRecord(TA_Base_Bus::IPAAgentCorbaDef::DvaPublicVersionRecord& localDvaVersionRecord, unsigned long stationOffset)
    {
        LOG_SCOPE("Table1610::processLocalDvaVersionRecord");

        // Guarded in processRead()

        bool dataChanged = false;

        //    dataChanged |= processOneVersionId(localDvaVersionRecord.m_privateAdhoc1, TABLE_1610_PRIVATE_ADHOC_1_SUB_OFFSET + stationOffset);
        //    dataChanged |= processOneVersionId(localDvaVersionRecord.m_privateAdhoc2, TABLE_1610_PRIVATE_ADHOC_2_SUB_OFFSET + stationOffset);
        //    dataChanged |= processOneVersionId(localDvaVersionRecord.m_privateAdhoc3, TABLE_1610_PRIVATE_ADHOC_3_SUB_OFFSET + stationOffset);
        //    dataChanged |= processOneVersionId(localDvaVersionRecord.m_privateAdhoc4, TABLE_1610_PRIVATE_ADHOC_4_SUB_OFFSET + stationOffset);
        //    dataChanged |= processOneVersionId(localDvaVersionRecord.m_privateAdhoc5, TABLE_1610_PRIVATE_ADHOC_5_SUB_OFFSET + stationOffset);
        //    dataChanged |= processOneVersionId(localDvaVersionRecord.m_privateAdhoc6, TABLE_1610_PRIVATE_ADHOC_6_SUB_OFFSET + stationOffset);
        //    dataChanged |= processOneVersionId(localDvaVersionRecord.m_privatePreRecorded, TABLE_1610_PRIVATE_PRERECORDED_SUB_OFFSET + stationOffset);
        dataChanged |= processOneVersionId(localDvaVersionRecord.m_publicAdhoc1, TABLE_1610_PUBLIC_ADHOC_1_SUB_OFFSET + stationOffset);
        dataChanged |= processOneVersionId(localDvaVersionRecord.m_publicAdhoc2, TABLE_1610_PUBLIC_ADHOC_2_SUB_OFFSET + stationOffset);
        dataChanged |= processOneVersionId(localDvaVersionRecord.m_publicAdhoc3, TABLE_1610_PUBLIC_ADHOC_3_SUB_OFFSET + stationOffset);
        dataChanged |= processOneVersionId(localDvaVersionRecord.m_publicAdhoc4, TABLE_1610_PUBLIC_ADHOC_4_SUB_OFFSET + stationOffset);
        dataChanged |= processOneVersionId(localDvaVersionRecord.m_publicAdhoc5, TABLE_1610_PUBLIC_ADHOC_5_SUB_OFFSET + stationOffset);
        dataChanged |= processOneVersionId(localDvaVersionRecord.m_publicAdhoc6, TABLE_1610_PUBLIC_ADHOC_6_SUB_OFFSET + stationOffset);
        dataChanged |= processOneVersionId(localDvaVersionRecord.m_publicPreRecorded, TABLE_1610_PUBLIC_PRERECORDED_SUB_OFFSET + stationOffset);

        //Maochun Sun++
        //TD13407
        if (dataChanged)
        {
            TA_IRS_Bus::DvaPublicVersionRecord record;
            record.m_locationKey = localDvaVersionRecord.m_locationKey;
            record.m_pKey = localDvaVersionRecord.m_pKey;
            //      record.m_privateAdhoc1 = localDvaVersionRecord.m_privateAdhoc1;
            //      record.m_privateAdhoc2 = localDvaVersionRecord.m_privateAdhoc2;
            //      record.m_privateAdhoc3 = localDvaVersionRecord.m_privateAdhoc3;
            //      record.m_privateAdhoc4 = localDvaVersionRecord.m_privateAdhoc4;
            //      record.m_privateAdhoc5 = localDvaVersionRecord.m_privateAdhoc5;
            //      record.m_privateAdhoc6 = localDvaVersionRecord.m_privateAdhoc6;
            //      record.m_privatePreRecorded = localDvaVersionRecord.m_privatePreRecorded;
            record.m_publicAdhoc1 = localDvaVersionRecord.m_publicAdhoc1;
            record.m_publicAdhoc2 = localDvaVersionRecord.m_publicAdhoc2;
            record.m_publicAdhoc3 = localDvaVersionRecord.m_publicAdhoc3;
            record.m_publicAdhoc4 = localDvaVersionRecord.m_publicAdhoc4;
            record.m_publicAdhoc5 = localDvaVersionRecord.m_publicAdhoc5;
            record.m_publicAdhoc6 = localDvaVersionRecord.m_publicAdhoc6;
            record.m_publicPreRecorded = localDvaVersionRecord.m_publicPreRecorded;

            TA_IRS_Bus::CachedMaps::getInstance()->setDvaMessagePublicVersionRecord(record, true);
        }

        //++Maochun Sun
        //TD13407

        return dataChanged;
    }

    bool Table1610::processOneVersionId(CORBA::String_member& dvaVersionId, unsigned long offset)
    {
        // Guarded in processRead()

        unsigned short day = PasHelpers::instance().get8bit(m_buffer, offset + 0);
        unsigned short month = PasHelpers::instance().get8bit(m_buffer, offset + 1);
        unsigned short year = PasHelpers::instance().get8bit(m_buffer, offset + 2);
        unsigned short hour = PasHelpers::instance().get8bit(m_buffer, offset + 3);
        unsigned short minute = PasHelpers::instance().get8bit(m_buffer, offset + 4);
        unsigned short version = PasHelpers::instance().get8bit(m_buffer, offset + 5);

        std::ostringstream versionString;
        std::string originalVersionString(dvaVersionId);

        // jeffrey++ TD10338
        //versionString << day  << "/" << month  << "/" << year << " "
        //              << hour << ":" << minute << " "
        //              << version;
        versionString << day << "." << month << "." << year << "."
            << hour << "." << minute << "."
            << version;
        // ++jeffrey TD10338

        if (versionString.str() != originalVersionString)
        {
            // Data has been updated
            dvaVersionId = versionString.str().c_str();
            return true;
        }

        // No change
        return false;
    }

    TA_Base_Bus::IPAAgentCorbaDef::DvaPublicVersionRecords* Table1610::getDvaVersionRecordsCopy()
        //throw ( TA_Base_Core::PasConnectionException )
    {
        TA_Base_Bus::IPAAgentCorbaDef::DvaPublicVersionRecords* newDvaVersionRecords = new TA_Base_Bus::IPAAgentCorbaDef::DvaPublicVersionRecords;

        // jeffrey++ TD10404
        //ThreadGuard guard( m_lock ); // ReEntrant
        // ++jeffrey TD10404

        newDvaVersionRecords->length(m_dvaVersionRecordMap.size());

        LocationKeyToDvaVersionRecordMapIt it = m_dvaVersionRecordMap.begin();
        int count = 0;

        for (; it != m_dvaVersionRecordMap.end(); ++it, ++count)
        {
            (*newDvaVersionRecords)[count] = it->second;
        }

        return newDvaVersionRecords;
    }

    void Table1610::processPublicSetVersionConflicts()
    {
        //    std::string stationInConflict = getFirstStationInConflict();
        //
        //    if (stationInConflict.empty())
        //    {
        //        // ISCS-PAS Synchronised
        //
        //        PasHelpers::instance().closePASVersionConflictAlarm();
        //    }
        //    else
        //    {
        //        // Conflict detected
        //
        //        PasHelpers::instance().raisePASVersionConflictAlarm(stationInConflict);
        //    }
    }

    std::string Table1610::getFirstStationInConflict()
    {
        //    ThreadGuard guard( m_lock ); // ReEntrant
        //
        //    std::string stationInConflict("");
        //
        //    for (int stationId=1; stationId<=MAXSTNID; ++stationId)
        //    {
        //        unsigned long locationKey;
        //        std::string   locationName;
        //        try
        //        {
        //            // Can return -1 if station does not exist
        //            locationKey = CachedMaps::getInstance()->getLocationKeyFromPasStationId(stationId);
        //            locationName = CachedMaps::getInstance()->getLocationNameFromKey( locationKey );
        //
        //            // Don't compare non-defined locations and depot.
        //            if (locationKey != m_depotLocationKey)
        //            {
        //                if (isStationInConflict(locationKey))
        //                {
        //                    if( stationInConflict != "" )
        //                        stationInConflict += ", ";
        //                    stationInConflict += locationName;
        //                    break;
        //                }
        //            }
        //        }
        //        catch(TA_Base_Core::CachedMappingNotFoundException&)
        //        {
        //            // This is expected as not all of the PAS stations (1-50) are defined
        //            // Just ignore and continue
        //        }
        //    }
        //
        //     return stationInConflict;
        return "";
    }

    //Maochun Sun++
    //TD13407
    std::string Table1610::getFirstStationInConflict(std::vector<unsigned int> stationIdList)
    {
        //    ThreadGuard guard( m_lock ); // ReEntrant
        //
        //    std::string stationInConflict("");
        //
        //  for(int i=0; i<stationIdList.size(); i++)
        //    {
        //      int stationId = stationIdList[i];
        //
        //        unsigned long locationKey;
        //        std::string   locationName;
        //        try
        //        {
        //            // Can return -1 if station does not exist
        //            locationKey = CachedMaps::getInstance()->getLocationKeyFromPasStationId(stationId);
        //            locationName = CachedMaps::getInstance()->getLocationNameFromKey( locationKey );
        //
        //            // Don't compare non-defined locations and depot.
        //            if (locationKey != m_depotLocationKey)
        //            {
        //                if (isStationInConflict(locationKey))
        //                {
        //                    if( stationInConflict != "" )
        //                        stationInConflict += ", ";
        //                    stationInConflict += locationName;
        //                    break;
        //                }
        //            }
        //        }
        //        catch(TA_Base_Core::CachedMappingNotFoundException&)
        //        {
        //            // This is expected as not all of the PAS stations (1-50) are defined
        //            // Just ignore and continue
        //        }
        //    }
        //
        //    return stationInConflict;
        return "";
    }

    //++Maochun Sun
    //TD13407

    bool Table1610::isStationInConflict(unsigned long locationKey)
    {
        //    const TA_Base_Bus::IPAAgentCorbaDef::DvaVersionRecord& sourceVersions = m_dvaVersionRecordMap[CachedConfig::getInstance()->getAgentLocationKey()];
        //    const TA_Base_Bus::IPAAgentCorbaDef::DvaVersionRecord& targetVersions = m_dvaVersionRecordMap[locationKey];
        //
        //    if ( strcmp(sourceVersions.m_publicPreRecorded, targetVersions.m_publicPreRecorded) != 0)
        //    {
        //        return true;
        //    }
        //
        //    if ( strcmp(sourceVersions.m_publicAdhoc1, targetVersions.m_publicAdhoc1) != 0)
        //    {
        //        return true;
        //    }
        //
        //    if ( strcmp(sourceVersions.m_publicAdhoc2, targetVersions.m_publicAdhoc2) != 0)
        //    {
        //        return true;
        //    }
        //
        //    if ( strcmp(sourceVersions.m_publicAdhoc3, targetVersions.m_publicAdhoc3) != 0)
        //    {
        //        return true;
        //    }
        //
        //    if ( strcmp(sourceVersions.m_publicAdhoc4, targetVersions.m_publicAdhoc4) != 0)
        //    {
        //        return true;
        //    }
        //
        //    if ( strcmp(sourceVersions.m_publicAdhoc5, targetVersions.m_publicAdhoc5) != 0)
        //    {
        //        return true;
        //    }
        //
        //    if ( strcmp(sourceVersions.m_publicAdhoc6, targetVersions.m_publicAdhoc6) != 0)
        //    {
        //        return true;
        //    }
        //
        //    // Else no conflict detected
        //
        return false;
    }

    ReadTable1610::ReadTable1610(Table1610& table,
                                 Table560& table560,
                                 TA_IRS_Bus::Scheduler& socketScheduler,
                                 TA_IRS_Bus::Scheduler& processScheduler,
                                 IPasEventSource& eventSource)
        : PASReadEvent(socketScheduler, processScheduler, eventSource)
        , m_table(table)
        , m_table560(table560)
    {
    }

    void ReadTable1610::readTable()
    {
        LOG_SCOPE("ReadTable1610::readTable");

        ThreadGuard guard(m_table.m_lock);

        PASConnection::getInstance()->readTable(m_table.TABLE_NUMBER,
                                                m_table.m_buffer,
                                                m_table.BUFFER_SIZE);

        m_processScheduler.post(new ProcessTable1610(m_table));

        m_table560.resetFlags(this,
                              Table560::TABLE_560_TABLE_1610_FLAG_A_BIT_OFFSET,
                              Table560::TABLE_560_TABLE_1610_FLAG_B_BIT_OFFSET);
    }

    ProcessTable1610::ProcessTable1610(Table1610& table)
        : m_table(table)
    {
    }

    void ProcessTable1610::consume()
    {
        LOG_SCOPE("ProcessTable1610::consume");

        m_table.processRead();
        PasTableReadCounter::instance().increase(m_table.TABLE_NUMBER);

        //Maochun Sun++
        //TD13407
        m_table.notifyIscs();
        //++Maochun Sun
        //TD13407

        delete this;
    }

    void ProcessTable1610::cancel()
    {
        LOG_SCOPE("ProcessTable1610::cancel");
        delete this;
    }
}
