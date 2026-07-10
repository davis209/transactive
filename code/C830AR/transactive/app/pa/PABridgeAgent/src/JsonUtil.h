#pragma once

#include <map>
#include <sstream>
#include <string>
#include <vector>

namespace TA_IRS_App
{
    class JsonUtil
    {
    public:
        static std::string quote(const std::string& value);
        static std::string property(const std::string& name, const std::string& value, bool rawValue = false);
        static std::string object(const std::vector<std::string>& properties);
        static std::string array(const std::vector<std::string>& values);
        static std::map<std::string, std::string> parseFlatObject(const std::string& body);
        static std::vector<unsigned long> parseUnsignedArray(const std::map<std::string, std::string>& values, const std::string& key);
        static bool parseBool(const std::map<std::string, std::string>& values, const std::string& key, bool defaultValue);
        static unsigned long parseUnsigned(const std::map<std::string, std::string>& values, const std::string& key, unsigned long defaultValue);
        static std::string getString(const std::map<std::string, std::string>& values, const std::string& key, const std::string& defaultValue = "");
    };
}
