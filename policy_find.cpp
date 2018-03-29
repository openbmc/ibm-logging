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
#include <phosphor-logging/log.hpp>
#include <sstream>
#include "policy_find.hpp"

namespace ibm
{
namespace logging
{
namespace policy
{

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
 * @return optional<std::string> - the data value
 */
optional_ns::optional<std::string>
    getAdditionalDataItem(const std::vector<std::string>& additionalData,
                          const std::string& name)
{
    for (const auto& item : additionalData)
    {
        if (item.find(name + "=") != std::string::npos)
        {
            return item.substr(item.find('=') + 1);
        }
    }

    return {};
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
    std::string modifier;

    // The modifier may be one of several things within the
    // AdditionalData property.  Try them all until one
    // is found.

    auto data =
        getProperty<std::vector<std::string>>(properties, "AdditionalData");

    if (data)
    {
        // An inventory callout
        auto mod = getAdditionalDataItem(*data, "CALLOUT_INVENTORY_PATH");
        if (mod)
        {
            modifier = *mod;
        }

        if (modifier.empty())
        {
            // A device path, but we only care about the type
            mod = getAdditionalDataItem(*data, "CALLOUT_DEVICE_PATH");
            if (mod)
            {
                // The table only handles I2C and FSI
                if ((*mod).find("i2c") != std::string::npos)
                {
                    modifier = "I2C";
                }
                else
                {
                    modifier = "FSI";
                }
            }
        }

        if (modifier.empty())
        {
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
                    std::transform(value.begin(), value.end(), value.begin(),
                                   toupper);

                    modifier = value;
                }
                catch (std::exception& e)
                {
                    using namespace phosphor::logging;
                    log<level::ERR>("Invalid PROCEDURE value found",
                                    entry("PROCEDURE=%s", *mod));
                }
            }
        }

        if (modifier.empty())
        {
            // Comes from the power fault code on a voltage fault
            mod = getAdditionalDataItem(*data, "RAIL_NAME");
            if (mod)
            {
                modifier = *mod;
            }
        }

        if (modifier.empty())
        {
            // Also from the power fault code on a PGOOD fault
            mod = getAdditionalDataItem(*data, "INPUT_NAME");
            if (mod)
            {
                modifier = *mod;
            }
        }
    }

    return modifier;
}

PolicyProps find(const policy::Table& policy,
                 const DbusPropertyMap& errorLogProperties)
{
    auto errorMsg = getProperty<std::string>(errorLogProperties,
                                             "Message"); // e.g. xyz.X.Error.Y
    if (errorMsg)
    {
        auto modifier = getSearchModifier(errorLogProperties);

        auto result = policy.find(*errorMsg, modifier);

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
}
}
}
