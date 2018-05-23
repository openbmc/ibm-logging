/** Copyright © 2018 IBM Corporation
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
#include "callout.hpp"

namespace ibm
{
namespace logging
{

Callout::Callout(sdbusplus::bus::bus& bus, const std::string& objectPath,
                 size_t id, uint64_t timestamp) :
    CalloutObject(bus, objectPath.c_str(), true),
    entryID(id), timestamp(timestamp)
{
}

Callout::Callout(sdbusplus::bus::bus& bus, const std::string& objectPath,
                 const std::string& inventoryPath, size_t id,
                 uint64_t timestamp, const DbusPropertyMap& properties) :
    CalloutObject(bus, objectPath.c_str(), true),
    entryID(id), timestamp(timestamp)
{
    path(inventoryPath);

    auto it = properties.find("BuildDate");
    if (it != properties.end())
    {
        buildDate(it->second.get<std::string>());
    }

    it = properties.find("Manufacturer");
    if (it != properties.end())
    {
        manufacturer(it->second.get<std::string>());
    }

    it = properties.find("Model");
    if (it != properties.end())
    {
        model(it->second.get<std::string>());
    }

    it = properties.find("PartNumber");
    if (it != properties.end())
    {
        partNumber(it->second.get<std::string>());
    }

    it = properties.find("SerialNumber");
    if (it != properties.end())
    {
        serialNumber(it->second.get<std::string>());
    }

    emit_object_added();
}
}
}
