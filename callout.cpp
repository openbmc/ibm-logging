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
#include "config.h"

#include "callout.hpp"

#include "dbus.hpp"

#include <cereal/archives/binary.hpp>
#include <cereal/types/string.hpp>
#include <cereal/types/tuple.hpp>
#include <cereal/types/vector.hpp>
#include <experimental/filesystem>
#include <fstream>
#include <phosphor-logging/log.hpp>

CEREAL_CLASS_VERSION(ibm::logging::Callout, CALLOUT_CLASS_VERSION);

namespace ibm
{
namespace logging
{

using namespace phosphor::logging;

/**
 * Function required by Cereal for saving data
 *
 * @param[in] archive - the Cereal archive object
 * @param[in] callout - the object to save
 * @param[in] version - the version of the persisted data
 */
template <class Archive>
void save(Archive& archive, const Callout& callout, const std::uint32_t version)
{
    archive(callout.id(), callout.ts(), callout.path(), callout.buildDate(),
            callout.manufacturer(), callout.model(), callout.partNumber(),
            callout.serialNumber());
}

/**
 * Function required by Cereal for restoring data into an object
 *
 * @param[in] archive - the Cereal archive object
 * @param[in] callout - the callout object to restore
 * @param[in] version - the version of the persisted data
 */
template <class Archive>
void load(Archive& archive, Callout& callout, const std::uint32_t version)
{
    size_t id;
    uint64_t timestamp;
    std::string inventoryPath;
    std::string build;
    std::string mfgr;
    std::string model;
    std::string pn;
    std::string sn;

    archive(id, timestamp, inventoryPath, build, mfgr, model, pn, sn);

    callout.id(id);
    callout.ts(timestamp);
    callout.path(inventoryPath);
    callout.buildDate(build);
    callout.manufacturer(mfgr);
    callout.model(model);
    callout.partNumber(pn);
    callout.serialNumber(sn);
}

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

void Callout::serialize(const fs::path& dir)
{
    auto path = getFilePath(dir);
    std::ofstream stream(path.c_str(), std::ios::binary);
    cereal::BinaryOutputArchive oarchive(stream);

    oarchive(*this);
}

bool Callout::deserialize(const fs::path& dir)
{
    auto path = getFilePath(dir);

    if (!fs::exists(path))
    {
        return false;
    }

    // Save the current ID and timestamp and then use them after
    // deserialization to check that the data we are restoring
    // is for the correct error log.

    auto originalID = entryID;
    auto originalTS = timestamp;

    try
    {
        std::ifstream stream(path.c_str(), std::ios::binary);
        cereal::BinaryInputArchive iarchive(stream);

        iarchive(*this);
    }
    catch (std::exception& e)
    {
        log<level::ERR>(e.what());
        log<level::ERR>("Failed trying to restore a Callout object",
                        entry("PATH=%s", path.c_str()));
        fs::remove(path);
        return false;
    }

    if ((entryID != originalID) || (timestamp != originalTS))
    {
        log<level::INFO>(
            "Timestamp or ID mismatch in persisted Callout. Discarding",
            entry("PATH=%s", path.c_str()), entry("PERSISTED_ID=%lu", entryID),
            entry("EXPECTED_ID=%lu", originalID),
            entry("PERSISTED_TS=%llu", timestamp),
            entry("EXPECTED_TS=%llu", originalTS));
        fs::remove(path);
        return false;
    }

    return true;
}
} // namespace logging
} // namespace ibm
