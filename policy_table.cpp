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
#include "policy_table.hpp"

#include <experimental/filesystem>
#include <fstream>
#include <nlohmann/json.hpp>
#include <phosphor-logging/log.hpp>

namespace ibm
{
namespace logging
{
namespace policy
{

namespace fs = std::experimental::filesystem;
using namespace phosphor::logging;

Table::Table(const std::string& jsonFile)
{
    if (fs::exists(jsonFile))
    {
        load(jsonFile);
    }
    else
    {
        log<level::INFO>("Policy table JSON file does not exist",
                         entry("FILE=%s", jsonFile.c_str()));
    }
}

void Table::load(const std::string& jsonFile)
{
    try
    {
        std::ifstream file{jsonFile};

        auto json = nlohmann::json::parse(file, nullptr, true);

        for (const auto& policy : json)
        {
            DetailsList detailsList;

            for (const auto& details : policy["dtls"])
            {
                Details d;
                d.modifier = details["mod"];
                d.msg = details["msg"];
                d.ceid = details["CEID"];
                detailsList.emplace_back(std::move(d));
            }
            policies.emplace(policy["err"], std::move(detailsList));
        }

        loaded = true;
    }
    catch (std::exception& e)
    {
        log<level::ERR>("Failed loading policy table json file",
                        entry("FILE=%s", jsonFile.c_str()),
                        entry("ERROR=%s", e.what()));
        loaded = false;
    }
}

FindResult Table::find(const std::string& error,
                       const std::string& modifier) const
{
    // First find the entry based on the error, and then find which
    // underlying details object it is with the help of the modifier.

    auto policy = policies.find(error);

    if (policy != policies.end())
    {
        // If there is no exact modifier match, then look for an entry with
        // an empty modifier - it is the catch-all for that error.
        auto details = std::find_if(
            policy->second.begin(), policy->second.end(),
            [&modifier](const auto& d) { return modifier == d.modifier; });

        if ((details == policy->second.end()) && !modifier.empty())
        {
            details =
                std::find_if(policy->second.begin(), policy->second.end(),
                             [](const auto& d) { return d.modifier.empty(); });
        }

        if (details != policy->second.end())
        {
            return DetailsReference(*details);
        }
    }

    return {};
}
} // namespace policy
} // namespace logging
} // namespace ibm
