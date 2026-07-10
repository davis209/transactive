#include "JsonUtil.h"

#include "json.hpp"

#include <cstdlib>
#include <sstream>

namespace
{
    typedef nlohmann::json Json;

    Json parseObject(const std::string& body)
    {
        if (body.empty())
        {
            return Json::object();
        }

        Json parsed = Json::parse(body);
        if (!parsed.is_object())
        {
            return Json::object();
        }
        return parsed;
    }

    Json parseValue(const std::string& value)
    {
        if (value.empty())
        {
            return Json();
        }

        try
        {
            return Json::parse(value);
        }
        catch (...)
        {
            return Json(value);
        }
    }

    std::string scalarToString(const Json& value)
    {
        if (value.is_string())
        {
            return value.get<std::string>();
        }
        if (value.is_boolean())
        {
            return value.get<bool>() ? "true" : "false";
        }
        if (value.is_number_unsigned())
        {
            std::ostringstream out;
            out << value.get<unsigned long>();
            return out.str();
        }
        if (value.is_number_integer())
        {
            std::ostringstream out;
            out << value.get<long>();
            return out.str();
        }
        if (value.is_number_float())
        {
            std::ostringstream out;
            out << value.get<double>();
            return out.str();
        }
        if (value.is_null())
        {
            return "";
        }
        return value.dump();
    }
}

namespace TA_IRS_App
{
    std::string JsonUtil::quote(const std::string& value)
    {
        return Json(value).dump();
    }

    std::string JsonUtil::property(const std::string& name, const std::string& value, bool rawValue)
    {
        Json object = Json::object();
        object[name] = rawValue ? parseValue(value) : Json(value);
        std::string dumped = object.dump();
        return dumped.substr(1, dumped.size() - 2);
    }

    std::string JsonUtil::object(const std::vector<std::string>& properties)
    {
        std::string text = "{";
        for (std::size_t i = 0; i < properties.size(); ++i)
        {
            if (i != 0)
            {
                text += ",";
            }
            text += properties[i];
        }
        text += "}";

        return Json::parse(text).dump();
    }

    std::string JsonUtil::array(const std::vector<std::string>& values)
    {
        std::string text = "[";
        for (std::size_t i = 0; i < values.size(); ++i)
        {
            if (i != 0)
            {
                text += ",";
            }
            text += values[i];
        }
        text += "]";

        return Json::parse(text).dump();
    }

    std::map<std::string, std::string> JsonUtil::parseFlatObject(const std::string& body)
    {
        std::map<std::string, std::string> result;
        Json parsed = parseObject(body);
        for (Json::const_iterator it = parsed.begin(); it != parsed.end(); ++it)
        {
            result[it.key()] = scalarToString(it.value());
        }
        return result;
    }

    std::vector<unsigned long> JsonUtil::parseUnsignedArray(const std::map<std::string, std::string>& values, const std::string& key)
    {
        std::vector<unsigned long> result;
        std::map<std::string, std::string>::const_iterator it = values.find(key);
        if (it == values.end())
        {
            return result;
        }

        Json parsed = parseValue(it->second);
        if (!parsed.is_array())
        {
            return result;
        }

        for (Json::const_iterator item = parsed.begin(); item != parsed.end(); ++item)
        {
            if (item->is_number_unsigned())
            {
                result.push_back(item->get<unsigned long>());
            }
            else if (item->is_number_integer())
            {
                result.push_back(static_cast<unsigned long>(item->get<long>()));
            }
            else if (item->is_string())
            {
                result.push_back(static_cast<unsigned long>(std::strtoul(item->get<std::string>().c_str(), 0, 10)));
            }
        }
        return result;
    }

    bool JsonUtil::parseBool(const std::map<std::string, std::string>& values, const std::string& key, bool defaultValue)
    {
        std::map<std::string, std::string>::const_iterator it = values.find(key);
        if (it == values.end())
        {
            return defaultValue;
        }

        Json parsed = parseValue(it->second);
        if (parsed.is_boolean())
        {
            return parsed.get<bool>();
        }
        if (parsed.is_number())
        {
            return parsed.get<int>() != 0;
        }
        if (parsed.is_string())
        {
            const std::string value = parsed.get<std::string>();
            return value == "true" || value == "1" || value == "yes" || value == "Y";
        }
        return defaultValue;
    }

    unsigned long JsonUtil::parseUnsigned(const std::map<std::string, std::string>& values, const std::string& key, unsigned long defaultValue)
    {
        std::map<std::string, std::string>::const_iterator it = values.find(key);
        if (it == values.end() || it->second.empty())
        {
            return defaultValue;
        }

        Json parsed = parseValue(it->second);
        if (parsed.is_number_unsigned())
        {
            return parsed.get<unsigned long>();
        }
        if (parsed.is_number_integer())
        {
            return static_cast<unsigned long>(parsed.get<long>());
        }
        if (parsed.is_string())
        {
            return static_cast<unsigned long>(std::strtoul(parsed.get<std::string>().c_str(), 0, 10));
        }
        return defaultValue;
    }

    std::string JsonUtil::getString(const std::map<std::string, std::string>& values, const std::string& key, const std::string& defaultValue)
    {
        std::map<std::string, std::string>::const_iterator it = values.find(key);
        return it == values.end() ? defaultValue : it->second;
    }
}
