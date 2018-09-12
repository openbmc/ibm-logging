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
#include "config.h"

#include "manager.hpp"

#include "callout.hpp"
#include "policy_find.hpp"

#include <phosphor-logging/log.hpp>

namespace ibm
{
namespace logging
{

namespace fs = std::experimental::filesystem;
using namespace phosphor::logging;
using sdbusplus::exception::SdBusError;

Manager::Manager(sdbusplus::bus::bus& bus) :
    bus(bus),
    addMatch(bus,
             sdbusplus::bus::match::rules::interfacesAdded() +
                 sdbusplus::bus::match::rules::path_namespace(LOGGING_PATH),
             std::bind(std::mem_fn(&Manager::interfaceAdded), this,
                       std::placeholders::_1)),
    removeMatch(bus,
                sdbusplus::bus::match::rules::interfacesRemoved() +
                    sdbusplus::bus::match::rules::path_namespace(LOGGING_PATH),
                std::bind(std::mem_fn(&Manager::interfaceRemoved), this,
                          std::placeholders::_1))
#ifdef USE_POLICY_INTERFACE
    ,
    policies(POLICY_JSON_PATH)
#endif
{
    createAll();
}

void Manager::createAll()
{
    try
    {
        auto objects = getManagedObjects(bus, LOGGING_BUSNAME, LOGGING_PATH);

        for (const auto& object : objects)
        {
            const auto& interfaces = object.second;

            auto propertyMap = interfaces.find(LOGGING_IFACE);

            if (propertyMap != interfaces.end())
            {
                createWithRestore(object.first, interfaces);
            }
        }
    }
    catch (const SdBusError& e)
    {
        log<level::ERR>("sdbusplus error getting logging managed objects",
                        entry("ERROR=%s", e.what()));
    }
}

void Manager::createWithRestore(const std::string& objectPath,
                                const DbusInterfaceMap& interfaces)
{
    createObject(objectPath, interfaces);

    restoreCalloutObjects(objectPath, interfaces);
}

void Manager::create(const std::string& objectPath,
                     const DbusInterfaceMap& interfaces)
{
    createObject(objectPath, interfaces);

    createCalloutObjects(objectPath, interfaces);
}

void Manager::createObject(const std::string& objectPath,
                           const DbusInterfaceMap& interfaces)
{
#ifdef USE_POLICY_INTERFACE
    auto logInterface = interfaces.find(LOGGING_IFACE);
    createPolicyInterface(objectPath, logInterface->second);
#endif
}

void Manager::erase(EntryID id)
{
    fs::remove_all(getSaveDir(id));
    childEntries.erase(id);
    entries.erase(id);
}

void Manager::addInterface(const std::string& objectPath, InterfaceType type,
                           std::experimental::any& object)
{
    auto id = getEntryID(objectPath);
    auto entry = entries.find(id);

    if (entry == entries.end())
    {
        InterfaceMap interfaces;
        interfaces.emplace(type, object);
        entries.emplace(id, std::move(interfaces));
    }
    else
    {
        entry->second.emplace(type, object);
    }
}

void Manager::addChildInterface(const std::string& objectPath,
                                InterfaceType type,
                                std::experimental::any& object)
{
    auto id = getEntryID(objectPath);
    auto entry = childEntries.find(id);

    // childEntries is:
    // A map of error log entry IDs to:
    //  a map of interface types to:
    //    a vector of interface objects

    if (entry == childEntries.end())
    {
        ObjectList objects{object};
        InterfaceMapMulti interfaces;
        interfaces.emplace(type, std::move(objects));
        childEntries.emplace(id, std::move(interfaces));
    }
    else
    {
        auto i = entry->second.find(type);
        if (i == entry->second.end())
        {
            ObjectList objects{objects};
            entry->second.emplace(type, objects);
        }
        else
        {
            i->second.emplace_back(object);
        }
    }
}

#ifdef USE_POLICY_INTERFACE
void Manager::createPolicyInterface(const std::string& objectPath,
                                    const DbusPropertyMap& properties)
{
    auto values = policy::find(policies, properties);

    auto object = std::make_shared<PolicyObject>(bus, objectPath.c_str(), true);

    object->eventID(std::get<policy::EIDField>(values));
    object->description(std::get<policy::MsgField>(values));

    object->emit_object_added();

    std::experimental::any anyObject = object;

    addInterface(objectPath, InterfaceType::POLICY, anyObject);
}
#endif

void Manager::createCalloutObjects(const std::string& objectPath,
                                   const DbusInterfaceMap& interfaces)
{
    // Use the associations property in the org.openbmc.Associations
    // interface to find any callouts.  Then grab all properties on
    // the Asset interface for that object in the inventory to use
    // in our callout objects.

    auto associations = interfaces.find(ASSOC_IFACE);
    if (associations == interfaces.end())
    {
        return;
    }

    const auto& properties = associations->second;
    auto assocProperty = properties.find("associations");
    auto assocValue = assocProperty->second.get<AssociationsPropertyType>();

    auto id = getEntryID(objectPath);
    auto calloutNum = 0;
    DbusSubtree subtree;

    for (const auto& association : assocValue)
    {
        try
        {
            if (std::get<forwardPos>(association) != "callout")
            {
                continue;
            }

            auto callout = std::get<endpointPos>(association);

            if (subtree.empty())
            {
                subtree = getSubtree(bus, "/", 0, ASSET_IFACE);
                if (subtree.empty())
                {
                    break;
                }
            }

            auto service = getService(callout, ASSET_IFACE, subtree);
            if (service.empty())
            {
                continue;
            }

            auto properties =
                getAllProperties(bus, service, callout, ASSET_IFACE);
            if (properties.empty())
            {
                continue;
            }

            auto calloutPath = getCalloutObjectPath(objectPath, calloutNum);

            auto object = std::make_shared<Callout>(
                bus, calloutPath, callout, calloutNum,
                getLogTimestamp(interfaces), properties);

            auto dir = getCalloutSaveDir(id);
            if (!fs::exists(dir))
            {
                fs::create_directories(dir);
            }
            object->serialize(dir);

            std::experimental::any anyObject = object;
            addChildInterface(objectPath, InterfaceType::CALLOUT, anyObject);
            calloutNum++;
        }
        catch (const SdBusError& e)
        {
            log<level::ERR>("sdbusplus exception", entry("ERROR=%s", e.what()));
        }
    }
}

void Manager::restoreCalloutObjects(const std::string& objectPath,
                                    const DbusInterfaceMap& interfaces)
{
    auto saveDir = getCalloutSaveDir(getEntryID(objectPath));

    if (!fs::exists(saveDir))
    {
        return;
    }

    size_t id;
    for (auto& f : fs::directory_iterator(saveDir))
    {
        try
        {
            id = std::stoul(f.path().filename());
        }
        catch (std::exception& e)
        {
            log<level::ERR>("Invalid IBM logging callout save file. Deleting",
                            entry("FILE=%s", f.path().c_str()));
            fs::remove(f.path());
            continue;
        }

        auto path = getCalloutObjectPath(objectPath, id);
        auto callout = std::make_shared<Callout>(bus, path, id,
                                                 getLogTimestamp(interfaces));
        if (callout->deserialize(saveDir))
        {
            callout->emit_object_added();
            std::experimental::any anyObject = callout;
            addChildInterface(objectPath, InterfaceType::CALLOUT, anyObject);
        }
    }
}

void Manager::interfaceAdded(sdbusplus::message::message& msg)
{
    sdbusplus::message::object_path path;
    DbusInterfaceMap interfaces;

    msg.read(path, interfaces);

    // Find the Logging.Entry interface with all of its properties
    // to pass to create().
    if (interfaces.find(LOGGING_IFACE) != interfaces.end())
    {
        create(path, interfaces);
    }
}

uint64_t Manager::getLogTimestamp(const DbusInterfaceMap& interfaces)
{
    auto interface = interfaces.find(LOGGING_IFACE);
    if (interface != interfaces.end())
    {
        auto property = interface->second.find("Timestamp");
        if (property != interface->second.end())
        {
            return property->second.get<uint64_t>();
        }
    }

    return 0;
}

fs::path Manager::getSaveDir(EntryID id)
{
    return fs::path{ERRLOG_PERSIST_PATH} / std::to_string(id);
}

fs::path Manager::getCalloutSaveDir(EntryID id)
{
    return getSaveDir(id) / "callouts";
}

std::string Manager::getCalloutObjectPath(const std::string& objectPath,
                                          uint32_t calloutNum)
{
    return fs::path{objectPath} / "callouts" / std::to_string(calloutNum);
}

void Manager::interfaceRemoved(sdbusplus::message::message& msg)
{
    sdbusplus::message::object_path path;
    DbusInterfaceList interfaces;

    msg.read(path, interfaces);

    // If the Logging.Entry interface was removed, then remove
    // our object

    auto i = std::find(interfaces.begin(), interfaces.end(), LOGGING_IFACE);

    if (i != interfaces.end())
    {
        erase(getEntryID(path));
    }
}
} // namespace logging
} // namespace ibm
