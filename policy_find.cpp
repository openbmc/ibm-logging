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
template<typename T>
static optional_ns::optional<T> getProperty(
        const DbusPropertyMap& properties,
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
 * Returns the search modifier to use.
 *
 * The modifier is used when the error name itself isn't granular
 * enough to find a policy table entry.  What it looks for is
 * based on how rules given by the IBM service team.
 *
 * Not all errors need a modifier, so this function isn't
 * guaranteed to find one.
 *
 * @param[in] properties - the property map for the error
 *
 * @return string - the search modifier
 *                  may be empty if none found
 */
static auto getSearchModifier(
        const DbusPropertyMap& properties)
{
    std::string modifier;

    auto data = getProperty<std::vector<std::string>>(
            properties,
            "AdditionalData");

    if (data)
    {
        //TODO
    }

    return modifier;
}

PolicyProps find(
        const policy::Table& policy,
        const DbusPropertyMap& errorLogProperties)
{
    auto errorMsg = getProperty<std::string>(
            errorLogProperties, "Message"); //e.g. xyz.X.Error.Y
    if (errorMsg)
    {
        auto modifier = getSearchModifier(errorLogProperties);

        auto result = policy.find(*errorMsg, modifier);

        if (result)
        {
            return {(*result)->ceid, (*result)->msg};
        }
    }

    return {policy.defaultEID(), policy.defaultMsg()};
}

}
}
}
