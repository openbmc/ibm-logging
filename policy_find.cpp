/**
 * Copyright © 2018 IBM Corporation
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include "policy_find.hpp"

#include <phosphor-logging/log.hpp>
#include <sstream>

namespace ibm
{
namespace logging
{
namespace policy
{

static constexpr auto HOST_EVENT = "org.open_power.Host.Error.Event";

namespace optional_ns = std::experimental;

/**
 * Returns a property value from a map of properties.
 *
 * @tparam - T the property data type
 * @param[in] properties - the property map
 * @param[in] name - the property name
 *
 * @return optional<T> - the property value
 */
template <typename T>
optional_ns::optional<T> getProperty(const DbusPropertyMap& properties,
                                     const std::string& name)
{
    auto prop = properties.find(name);

    if (prop != properties.end())
    {
        return prop->second.template get<T>();
    }

    return {};
}

/**
 * Finds a value in the AdditionalData property, which is
 * an array of strings in the form of:
 *
 *    NAME=VALUE
 *
 * @param[in] additionalData - the AdditionalData property contents
 * @param[in] name - the name of the value to find
 *
 * @return optional<std::string> - the data value. Will not be empty if found.
 */
optional_ns::optional<std::string>
    getAdditionalDataItem(const std::vector<std::string>& additionalData,
                          const std::string& name)
{
    std::string value;

    for (const auto& item : additionalData)
    {
        if (item.find(name + "=") != std::string::npos)
        {
            value = item.substr(item.find('=') + 1);
            if (!item.empty())
            {
                return value;
            }
        }
    }

    return {};
}

/**
 * Returns a string version of the severity from the PEL
 * log in the extended SEL data from the host, where a PEL stands
 * for 'Platform Event Log' and is an IBM standard for error logging
 * that OpenPower host firmware uses.
 *
 * The severity is the 11th byte in the 'User Header' section in a PEL
 * that starts at byte 48.  We only need the first nibble, which signifies
 * the type - 'Recovered', 'Predictive', 'Critical', etc.
 *
 *  type value   |   type     |  returned severity string
 *  ------------------------------------
 *  1                Recovered   Informational
 *  2                Predictive  Warning
 *  everything else  na          Critical
 *
 * @param[in] data - the PEL string in the form of "00 11 22 33 4e ff"
 *
 * @return optional<std::string> - the severity string as listed above
 */
optional_ns::optional<std::string> getESELSeverity(const std::string& data)
{
    // The User Header section starts at byte 48, and take into account
    // the input data is a space separated string representation of HEX data.
    static constexpr auto UH_OFFSET = 48 * 4;

    // The eye catcher is "UH"
    static constexpr auto UH_EYECATCHER = "55 48";

    // The severity is the 11th byte in the section, and take into
    // account a byte is "BB "
    static constexpr auto UH_SEV_OFFSET = 10 * 3;

    std::string severity = "Critical";

    // The only values that don't map to "Critical"
    const std::map<std::string, std::string> sevTypes{{"1", "Informational"},
                                                      {"2", "Warning"}};
    if (data.size() <= (UH_OFFSET + UH_SEV_OFFSET))
    {
        return {};
    }

    // Sanity check that the User Header section is there.
    auto userHeader = data.substr(UH_OFFSET, 5);
    if (userHeader.compare(UH_EYECATCHER))
    {
        return {};
    }

    // The severity type nibble is a full byte in the string.
    auto sevType = data.substr(UH_OFFSET + UH_SEV_OFFSET, 1);

    auto sev = sevTypes.find(sevType);
    if (sev != sevTypes.end())
    {
        severity = sev->second;
    };

    return severity;
}

/**
 * Returns the search modifier to use, but if it isn't found
 * in the table then code should then call getSearchModifier()
 * and try again.
 *
 * This is to be tolerant of the policy table not having
 * entries for every device path or FRU callout, and trying
 * again gives code a chance to find the more generic entries
 * for those classes of errors rather than not being found
 * at all.
 *
 * e.g. If the device path is missing in the table, then it
 * can still find the generic "Failed to read from an I2C
 * device" entry.
 *
 * @param[in] message- the error message, like xyz.A.Error.B
 * @param[in] properties - the property map for the error
 *
 * @return string - the search modifier
 *                  may be empty if none found
 */
std::string getSearchModifierFirstTry(const std::string& message,
                                      const DbusPropertyMap& properties)
{
    auto data =
        getProperty<std::vector<std::string>>(properties, "AdditionalData");

    if (!data)
    {
        return std::string{};
    }

    // Try the called out device path as the search modifier
    auto devPath = getAdditionalDataItem(*data, "CALLOUT_DEVICE_PATH");
    if (devPath)
    {
        return *devPath;
    }

    // For Host.Error.Event errors, try <callout>||<severity string>
    // as the search modifier.
    if (message == HOST_EVENT)
    {
        auto callout = getAdditionalDataItem(*data, "CALLOUT_INVENTORY_PATH");
        if (callout)
        {
            auto selData = getAdditionalDataItem(*data, "ESEL");
            if (selData)
            {
                auto severity = getESELSeverity(*selData);
                if (severity)
                {
                    return *callout + "||" + *severity;
                }
            }
        }
    }

    return std::string{};
}

/**
 * Returns the search modifier to use.
 *
 * The modifier is used when the error name itself isn't granular
 * enough to find a policy table entry.  The modifier is determined
 * using rules provided by the IBM service team.
 *
 * Not all errors need a modifier, so this function isn't
 * guaranteed to find one.
 *
 * @param[in] properties - the property map for the error
 *
 * @return string - the search modifier
 *                  may be empty if none found
 */
auto getSearchModifier(const DbusPropertyMap& properties)
{
    // The modifier may be one of several things within the
    // AdditionalData property.  Try them all until one
    // is found.

    auto data =
        getProperty<std::vector<std::string>>(properties, "AdditionalData");

    if (!data)
    {
        return std::string{};
    }

    // AdditionalData fields where the value is the modifier
    static const std::vector<std::string> ADFields{"CALLOUT_INVENTORY_PATH",
                                                   "RAIL_NAME", "INPUT_NAME"};

    optional_ns::optional<std::string> mod;
    for (const auto& field : ADFields)
    {
        mod = getAdditionalDataItem(*data, field);
        if (mod)
        {
            return *mod;
        }
    }

    // Next are the AdditionalData fields where the value needs
    // to be massaged to get the modifier.

    // A device path, but we only care about the type
    mod = getAdditionalDataItem(*data, "CALLOUT_DEVICE_PATH");
    if (mod)
    {
        // The table only handles I2C and FSI
        if ((*mod).find("i2c") != std::string::npos)
        {
            return std::string{"I2C"};
        }
        else if ((*mod).find("fsi") != std::string::npos)
        {
            return std::string{"FSI"};
        }
    }

    // A hostboot procedure ID
    mod = getAdditionalDataItem(*data, "PROCEDURE");
    if (mod)
    {
        // Convert decimal (e.g. 109) to hex (e.g. 6D)
        std::ostringstream stream;
        try
        {
            stream << std::hex << std::stoul((*mod).c_str());
            auto value = stream.str();

            if (!value.empty())
            {
                std::transform(value.begin(), value.end(), value.begin(),
                               toupper);
                return value;
            }
        }
        catch (std::exception& e)
        {
            using namespace phosphor::logging;
            log<level::ERR>("Invalid PROCEDURE value found",
                            entry("PROCEDURE=%s", mod->c_str()));
        }
    }

    return std::string{};
}

PolicyProps find(const policy::Table& policy,
                 const DbusPropertyMap& errorLogProperties)
{
    auto errorMsg = getProperty<std::string>(errorLogProperties,
                                             "Message"); // e.g. xyz.X.Error.Y
    if (errorMsg)
    {
        FindResult result;

        // Try with the FirstTry modifier first, and then the regular one.

        auto modifier =
            getSearchModifierFirstTry(*errorMsg, errorLogProperties);

        if (!modifier.empty())
        {
            result = policy.find(*errorMsg, modifier);
        }

        if (!result)
        {
            modifier = getSearchModifier(errorLogProperties);

            result = policy.find(*errorMsg, modifier);
        }

        if (result)
        {
            return {(*result).get().ceid, (*result).get().msg};
        }
    }
    else
    {
        using namespace phosphor::logging;
        log<level::ERR>("No Message metadata found in an error");
    }

    return {policy.defaultEID(), policy.defaultMsg()};
}
} // namespace policy
} // namespace logging
} // namespace ibm
