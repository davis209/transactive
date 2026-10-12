#pragma once

#include <functional>
#include <map>
#include <string>

#include "bus/signs_4669/TisManagerCorbaDef/src/ISTISManagerCorbaDef.h"
#include "bus/signs_4669/TisManagerCorbaDef/src/ITTISManagerCorbaDef.h"
#include "bus/signs_4669/tis_agent_access/src/TISAgentAccessFactory.h"
#include "core/naming/src/NamedObject.h"

namespace TA_IRS_App
{
    typedef TA_Base_Core::NamedObject<TA_Base_Bus::ISTISManagerCorbaDef,
        TA_Base_Bus::ISTISManagerCorbaDef_ptr,
        TA_Base_Bus::ISTISManagerCorbaDef_var> STISAgentNamedObject;

    typedef TA_Base_Core::NamedObject<TA_Base_Bus::ITTISManagerCorbaDef,
        TA_Base_Bus::ITTISManagerCorbaDef_ptr,
        TA_Base_Bus::ITTISManagerCorbaDef_var> TTISAgentNamedObject;

    class TisRestController
    {
    public:
        typedef std::function<std::string()> SessionIdProvider;

        explicit TisRestController(SessionIdProvider sessionIdProvider);

        std::string handle(const std::string& method, const std::string& path, const std::string& body, int& statusCode);

    private:
        std::string error(int& statusCode, int code, const std::string& message);
        std::string getPathId(const std::string& path, const std::string& prefix, const std::string& suffix = "") const;
        TA_Base_Core::TTISPredefinedMessageParameters toTtisPredefined(const std::map<std::string, std::string>& request) const;
        TA_Base_Core::TTISFreeTextMessageParameters toTtisFreeText(const std::map<std::string, std::string>& request) const;
        TA_Base_Core::TTISMessageResetParameters toTtisReset(const std::map<std::string, std::string>& request) const;

        STISAgentNamedObject& m_stisAgent;
        TTISAgentNamedObject& m_ttisAgent;
        SessionIdProvider m_sessionIdProvider;
    };
}
