#include "TisJsonUtil.h"

#include "json.hpp"

#include <cstdlib>
#include <sstream>

namespace
{
    typedef nlohmann::json Json;

    Json parseObject(const std::string& body)
    {
        if (body.empty()) return Json::object();
        Json parsed = Json::parse(body);
        return parsed.is_object() ? parsed : Json::object();
    }

    Json parseValue(const std::string& value)
    {
        if (value.empty()) return Json();
        try { return Json::parse(value); }
        catch (...) { return Json(value); }
    }

    std::string scalarToString(const Json& value)
    {
        if (value.is_string()) return value.get<std::string>();
        if (value.is_boolean()) return value.get<bool>() ? "true" : "false";
        if (value.is_null()) return "";
        return value.dump();
    }
}

namespace TA_IRS_App
{
    std::string TisJsonUtil::property(const std::string& name, const std::string& value, bool rawValue)
    {
        Json object = Json::object();
        object[name] = rawValue ? parseValue(value) : Json(value);
        const std::string dumped = object.dump();
        return dumped.substr(1, dumped.size() - 2);
    }

    std::string TisJsonUtil::object(const std::vector<std::string>& properties)
    {
        std::string text = "{";
        for (std::size_t i = 0; i < properties.size(); ++i)
        {
            if (i != 0) text += ",";
            text += properties[i];
        }
        text += "}";
        return Json::parse(text).dump();
    }

    std::string TisJsonUtil::array(const std::vector<std::string>& values)
    {
        std::string text = "[";
        for (std::size_t i = 0; i < values.size(); ++i)
        {
            if (i != 0) text += ",";
            text += values[i];
        }
        text += "]";
        return Json::parse(text).dump();
    }

    std::map<std::string, std::string> TisJsonUtil::parseFlatObject(const std::string& body)
    {
        std::map<std::string, std::string> result;
        Json parsed = parseObject(body);
        for (Json::const_iterator it = parsed.begin(); it != parsed.end(); ++it)
        {
            result[it.key()] = scalarToString(it.value());
        }
        return result;
    }

    std::vector<unsigned long> TisJsonUtil::parseUnsignedArray(const std::map<std::string, std::string>& values, const std::string& key)
    {
        std::vector<unsigned long> result;
        std::map<std::string, std::string>::const_iterator it = values.find(key);
        if (it == values.end()) return result;
        Json parsed = parseValue(it->second);
        if (!parsed.is_array()) return result;
        for (Json::const_iterator item = parsed.begin(); item != parsed.end(); ++item)
        {
            if (item->is_number()) result.push_back(item->get<unsigned long>());
            else if (item->is_string()) result.push_back(std::strtoul(item->get<std::string>().c_str(), 0, 10));
        }
        return result;
    }

    bool TisJsonUtil::parseBool(const std::map<std::string, std::string>& values, const std::string& key, bool defaultValue)
    {
        const std::string value = getString(values, key);
        if (value.empty()) return defaultValue;
        Json parsed = parseValue(value);
        if (parsed.is_boolean()) return parsed.get<bool>();
        if (parsed.is_number()) return parsed.get<int>() != 0;
        if (parsed.is_string()) return parsed.get<std::string>() == "true" || parsed.get<std::string>() == "1";
        return defaultValue;
    }

    unsigned long TisJsonUtil::parseUnsigned(const std::map<std::string, std::string>& values, const std::string& key, unsigned long defaultValue)
    {
        const std::string value = getString(values, key);
        if (value.empty()) return defaultValue;
        Json parsed = parseValue(value);
        if (parsed.is_number()) return parsed.get<unsigned long>();
        if (parsed.is_string()) return std::strtoul(parsed.get<std::string>().c_str(), 0, 10);
        return defaultValue;
    }

    long TisJsonUtil::parseLong(const std::map<std::string, std::string>& values, const std::string& key, long defaultValue)
    {
        const std::string value = getString(values, key);
        if (value.empty()) return defaultValue;
        Json parsed = parseValue(value);
        if (parsed.is_number()) return parsed.get<long>();
        if (parsed.is_string()) return std::strtol(parsed.get<std::string>().c_str(), 0, 10);
        return defaultValue;
    }

    std::string TisJsonUtil::getString(const std::map<std::string, std::string>& values, const std::string& key, const std::string& defaultValue)
    {
        std::map<std::string, std::string>::const_iterator it = values.find(key);
        return it == values.end() ? defaultValue : it->second;
    }
}
