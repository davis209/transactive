#pragma once

#include <map>
#include <string>

#include "bus/signs_4669/TisManagerCorbaDef/src/ISTISManagerCorbaDef.h"
#include "bus/signs_4669/TisManagerCorbaDef/src/ITTISManagerCorbaDef.h"

namespace TA_IRS_App
{
    class TisJsonConverter
    {
    public:
        static TA_Base_Core::TrainList toTrainList(const std::map<std::string, std::string>& request);
        static TA_Base_Core::TrainPIDList toTrainPidList(const std::map<std::string, std::string>& request);
        static TA_Base_Core::STISDestinationList toStisDestinations(const std::map<std::string, std::string>& request);
        static TA_Base_Core::STISDestination toStisDestination(const std::map<std::string, std::string>& request);

        static TA_Base_Core::ELibrarySection librarySection(const std::string& value);
        static TA_Base_Core::EDisplayMode displayMode(const std::string& value);
        static TA_Base_Core::EJustification justification(const std::string& value);
        static TA_Base_Core::ETimeScheduleChangeType timeScheduleChangeType(const std::string& value);
        static TA_Base_Core::ERATISMessageType ratisMessageType(const std::string& value);
        static TA_Base_Core::EPIDControl pidControl(const std::string& value);

        static std::string toJson(const TA_Base_Core::TTISDisplayResult& data);
        static std::string toJson(const TA_Base_Core::TrainDownloadStatus& data);
        static std::string toJson(const TA_Base_Core::TrainDataVersion& data);
        static std::string toJson(const TA_Base_Core::TimeScheduleChange& data);
        static std::string toJson(const TA_Base_Bus::ISTISManagerCorbaDef::IncomingRATISEvent& data);
        static std::string toJson(const TA_Base_Bus::ISTISManagerCorbaDef::RATISMessageApprovalDetails& data);
        static std::string toJson(const TA_Base_Bus::ISTISManagerCorbaDef::RATISMessageDetails& data);
        static std::string toJson(const TA_Base_Bus::ISTISManagerCorbaDef::RATISMessageList& data);
        static std::string toJson(const TA_Base_Core::TrainDownloadList& data);
        static std::string toJson(const TA_Base_Core::TrainDataVersionList& data);
        static std::string toJson(const TA_Base_Core::TrainDataVersionAlarmList& data);
        static std::string toJson(const TA_Base_Core::CurrentDisplayingMessage& data);

        static std::string commandType(TA_Base_Core::TisCommandType value);
        static std::string downloadType(TA_Base_Core::EDownloadChangeType value);
        static std::string scheduleChangeType(TA_Base_Core::ETimeScheduleChangeType value);
        static std::string ratisType(TA_Base_Core::ERATISMessageType value);
        static std::string ratisStatus(TA_Base_Core::ERATISMessageStatus value);
    };
}
