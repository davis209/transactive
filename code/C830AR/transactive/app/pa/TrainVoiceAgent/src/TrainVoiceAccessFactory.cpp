#include "TrainVoiceAccessFactory.h"
#include "core/data_access_interface/src/DatabaseFactory.h"
#include "core/data_access_interface/src/IDatabase.h"
#include "core/data_access_interface/src/SQLCode.h"
#include <memory>
#include <stdexcept>

namespace TA_IRS_App
{
    namespace
    {
        void validateText(const std::string& value)
        {
            for (std::string::const_iterator it = value.begin(); it != value.end(); ++it)
                if (*it == '\\' || static_cast<unsigned char>(*it) < 0x20 || *it == 0x7f)
                    throw std::invalid_argument("Unsupported database value");
        }
    }
    TrainVoiceAccessFactory& TrainVoiceAccessFactory::getInstance()
    {
        static TrainVoiceAccessFactory factory;
        return factory;
    }
    std::vector<TrainVoiceDevice> TrainVoiceAccessFactory::getDevices()
    {
        TA_Base_Core::IDatabase* db = TA_Base_Core::DatabaseFactory::getInstance().getDatabase(TA_Base_Core::Radio_Cd, TA_Base_Core::Read);
        TA_Base_Core::SQLStatement sql;
        db->prepareSQLStatement(sql, "TRAINVOICE_SELECT_97001");
        const char* names[] = {"TRAIN_ID", "TRAIN_NAME", "CAR_ID", "CAR_NAME", "CAR_POSITION", "DEVICE_KEY", "DEVICE_TYPE", "DEVICE_NAME", "PROTOCOL_DEVICE_ID", "IP_ADDRESS", "CONTROL_PORT", "SIP_EXTENSION"};
        std::vector<std::string> columns(names, names + sizeof(names) / sizeof(names[0]));
        TA_Base_Core::IData* raw = db->executeQuery(sql, columns);
        std::vector<TrainVoiceDevice> devices;
        do
        {
            std::unique_ptr<TA_Base_Core::IData> data(raw);
            for (unsigned long row = 0; row < data->getNumRows(); ++row)
            {
                TrainVoiceDevice d;
                d.trainId = data->getUnsignedLongData(row, "TRAIN_ID"); d.trainName = data->getStringData(row, "TRAIN_NAME");
                d.carId = data->getUnsignedLongData(row, "CAR_ID"); d.carName = data->getStringData(row, "CAR_NAME");
                d.carPosition = data->getUnsignedLongData(row, "CAR_POSITION"); d.deviceKey = data->getUnsignedLongData(row, "DEVICE_KEY");
                d.deviceType = data->getStringData(row, "DEVICE_TYPE"); d.deviceName = data->getStringData(row, "DEVICE_NAME");
                d.protocolDeviceId = data->getUnsignedLongData(row, "PROTOCOL_DEVICE_ID"); d.ipAddress = data->getStringData(row, "IP_ADDRESS");
                d.controlPort = data->getUnsignedLongData(row, "CONTROL_PORT"); d.sipExtension = data->getStringData(row, "SIP_EXTENSION");
                devices.push_back(d);
            }
            data.reset(); raw = 0;
        } while (db->moreData(raw));
        return devices;
    }
    std::vector<TrainVoiceCall> TrainVoiceAccessFactory::getActiveCalls()
    {
        TA_Base_Core::IDatabase* db = TA_Base_Core::DatabaseFactory::getInstance().getDatabase(TA_Base_Core::Radio_Cd, TA_Base_Core::Read);
        TA_Base_Core::SQLStatement sql;
        db->prepareSQLStatement(sql, "TRAINVOICE_SELECT_97002");
        const char* names[] = {"CALL_ID", "TRAIN_ID", "CAR_ID", "DEVICE_KEY", "AUDIO_FUNCTION", "SIP_EXTENSION", "SIP_CALL_ID", "CLIENT_ID", "STARTED_AT"};
        std::vector<std::string> columns(names, names + sizeof(names) / sizeof(names[0]));
        TA_Base_Core::IData* raw = db->executeQuery(sql, columns);
        std::vector<TrainVoiceCall> calls;
        do
        {
            std::unique_ptr<TA_Base_Core::IData> data(raw);
            for (unsigned long row = 0; row < data->getNumRows(); ++row)
            {
                TrainVoiceCall c;
                c.callId = data->getStringData(row, "CALL_ID"); c.trainId = data->getUnsignedLongData(row, "TRAIN_ID");
                c.carId = data->getUnsignedLongData(row, "CAR_ID"); c.deviceKey = data->getUnsignedLongData(row, "DEVICE_KEY");
                c.audioFunction = data->getUnsignedLongData(row, "AUDIO_FUNCTION"); c.sipExtension = data->getStringData(row, "SIP_EXTENSION");
                c.sipCallId = data->getStringData(row, "SIP_CALL_ID"); c.clientId = data->getStringData(row, "CLIENT_ID");
                c.startedAt = data->getStringData(row, "STARTED_AT"); calls.push_back(c);
            }
            data.reset(); raw = 0;
        } while (db->moreData(raw));
        return calls;
    }
    bool TrainVoiceAccessFactory::addActiveCall(const TrainVoiceCall& call)
    {
        validateText(call.callId); validateText(call.sipExtension); validateText(call.sipCallId); validateText(call.clientId);
        TA_Base_Core::IDatabase* db = TA_Base_Core::DatabaseFactory::getInstance().getDatabase(TA_Base_Core::Radio_Cd, TA_Base_Core::Write);
        TA_Base_Core::SQLStatement sql;
        db->prepareSQLStatement(sql, "TRAINVOICE_INSERT_97003", db->escapeInsertString(call.callId), call.trainId,
            call.carId, call.deviceKey, call.audioFunction, db->escapeInsertString(call.sipExtension),
            db->escapeInsertString(call.sipCallId), db->escapeInsertString(call.clientId));
        db->executeModification(sql);
        TA_Base_Core::SQLStatement query;
        db->prepareSQLStatement(query, "TRAINVOICE_SELECT_97006", db->escapeQueryString(call.callId));
        const char* names[] = {"TRAIN_ID", "CAR_ID", "DEVICE_KEY", "AUDIO_FUNCTION", "SIP_EXTENSION", "SIP_CALL_ID", "CLIENT_ID"};
        std::vector<std::string> columns(names, names + sizeof(names) / sizeof(names[0]));
        std::unique_ptr<TA_Base_Core::IData> data(db->executeQuery(query, columns));
        if (!data || data->getNumRows() != 1) throw std::runtime_error("Unable to verify active call");
        return data->getUnsignedLongData(0, "TRAIN_ID") == call.trainId && data->getUnsignedLongData(0, "CAR_ID") == call.carId &&
            data->getUnsignedLongData(0, "DEVICE_KEY") == call.deviceKey && data->getUnsignedLongData(0, "AUDIO_FUNCTION") == call.audioFunction &&
            data->getStringData(0, "SIP_EXTENSION") == call.sipExtension && data->getStringData(0, "SIP_CALL_ID") == call.sipCallId &&
            data->getStringData(0, "CLIENT_ID") == call.clientId;
    }
    void TrainVoiceAccessFactory::removeActiveCall(const std::string& callId)
    {
        validateText(callId);
        TA_Base_Core::IDatabase* db = TA_Base_Core::DatabaseFactory::getInstance().getDatabase(TA_Base_Core::Radio_Cd, TA_Base_Core::Write);
        TA_Base_Core::SQLStatement sql;
        db->prepareSQLStatement(sql, "TRAINVOICE_DELETE_97004", db->escapeQueryString(callId));
        db->executeModification(sql);
    }
    void TrainVoiceAccessFactory::addPendingRequest(unsigned long trainId, const std::string& type, unsigned long sourceDeviceId)
    {
        if (trainId == 0 || trainId > 254 || (type != "DRIVER" && type != "PASSENGER") || sourceDeviceId > 65535)
            throw std::invalid_argument("Invalid call request");
        TA_Base_Core::IDatabase* db = TA_Base_Core::DatabaseFactory::getInstance().getDatabase(TA_Base_Core::Radio_Cd, TA_Base_Core::Write);
        TA_Base_Core::SQLStatement sql;
        db->prepareSQLStatement(sql, "TRAINVOICE_INSERT_97007", trainId, type, sourceDeviceId);
        db->executeModification(sql);
    }
    std::vector<TrainVoiceCallRequest> TrainVoiceAccessFactory::getPendingRequests()
    {
        TA_Base_Core::IDatabase* db = TA_Base_Core::DatabaseFactory::getInstance().getDatabase(TA_Base_Core::Radio_Cd, TA_Base_Core::Read);
        TA_Base_Core::SQLStatement sql;
        db->prepareSQLStatement(sql, "TRAINVOICE_SELECT_97008");
        const char* names[] = {"REQUEST_ID", "TRAIN_ID", "REQUEST_TYPE", "SOURCE_DEVICE_ID", "RECEIVED_AT"};
        std::vector<std::string> columns(names, names + sizeof(names) / sizeof(names[0]));
        TA_Base_Core::IData* raw = db->executeQuery(sql, columns);
        std::vector<TrainVoiceCallRequest> requests;
        do
        {
            std::unique_ptr<TA_Base_Core::IData> data(raw);
            for (unsigned long row = 0; row < data->getNumRows(); ++row)
            {
                TrainVoiceCallRequest request;
                const std::string requestId = data->getStringData(row, "REQUEST_ID");
                if (requestId.empty() || requestId.find_first_not_of("0123456789") != std::string::npos)
                    throw std::runtime_error("Invalid request ID in database");
                request.requestId = std::stoull(requestId);
                request.trainId = data->getUnsignedLongData(row, "TRAIN_ID");
                request.requestType = data->getStringData(row, "REQUEST_TYPE");
                request.sourceDeviceId = data->getUnsignedLongData(row, "SOURCE_DEVICE_ID");
                request.receivedAt = data->getStringData(row, "RECEIVED_AT");
                requests.push_back(request);
            }
            data.reset(); raw = 0;
        } while (db->moreData(raw));
        return requests;
    }
    void TrainVoiceAccessFactory::removePendingRequest(unsigned long long requestId)
    {
        if (requestId == 0) throw std::invalid_argument("Invalid request ID");
        TA_Base_Core::IDatabase* db = TA_Base_Core::DatabaseFactory::getInstance().getDatabase(TA_Base_Core::Radio_Cd, TA_Base_Core::Write);
        TA_Base_Core::SQLStatement sql;
        db->prepareSQLStatement(sql, "TRAINVOICE_DELETE_97009", std::to_string(requestId));
        db->executeModification(sql);
    }
}
