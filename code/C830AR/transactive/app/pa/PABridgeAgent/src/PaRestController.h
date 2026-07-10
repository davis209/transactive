#pragma once

#include <functional>
#include <map>
#include <string>

#include "bus/pa_4669/pa_agent/IDL/src/IPAAgentCorbaDef.h"
#include "core/naming/src/NamedObject.h"

namespace TA_IRS_App
{
    typedef TA_Base_Core::NamedObject<TA_Base_Bus::IPAAgentCorbaDef,
        TA_Base_Bus::IPAAgentCorbaDef_ptr,
        TA_Base_Bus::IPAAgentCorbaDef_var> PAAgentNamedObject;

    class PaRestController
    {
    public:
        typedef std::function<std::string()> SessionIdProvider;

        PaRestController(const std::string& paAgentEntityName, SessionIdProvider sessionIdProvider);

        std::string handle(const std::string& method, const std::string& path, const std::string& body, int& statusCode);

    private:
        std::string ok(const std::string& json);
        std::string error(int& statusCode, int code, const std::string& message);
        std::string getPathId(const std::string& path, const std::string& prefix, const std::string& suffix) const;
        TA_Base_Bus::IPAAgentCorbaDef::ZoneCoverage toZoneCoverage(const std::map<std::string, std::string>& body, const std::string& key) const;
        TA_Base_Bus::IPAAgentCorbaDef::TrainCoverage toTrainCoverage(const std::map<std::string, std::string>& body, const std::string& key) const;

        PAAgentNamedObject m_paAgent;
        SessionIdProvider m_sessionIdProvider;
    };
}
