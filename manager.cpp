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
#include "policy_find.hpp"

namespace ibm
{
namespace logging
{

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
    auto objects = getManagedObjects(bus, LOGGING_BUSNAME, LOGGING_PATH);

    for (const auto& object : objects)
    {
        const auto& interfaces = object.second;

        auto propertyMap = std::find_if(
            interfaces.begin(), interfaces.end(),
            [](const auto& i) { return i.first == LOGGING_IFACE; });

        if (propertyMap != interfaces.end())
        {
            create(object.first, propertyMap->second);
        }
    }
}

void Manager::create(const std::string& objectPath,
                     const DbusPropertyMap& properties)
{

#ifdef USE_POLICY_INTERFACE
    createPolicyInterface(objectPath, properties);
#endif
}

void Manager::erase(EntryID id)
{
    auto entry = entries.find(id);

    if (entry != entries.end())
    {
        entries.erase(entry);
    }
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

void Manager::interfaceAdded(sdbusplus::message::message& msg)
{
    sdbusplus::message::object_path path;
    DbusInterfaceMap interfaces;

    msg.read(path, interfaces);

    // Find the Logging.Entry interface with all of its properties
    // to pass to create().
    auto propertyMap =
        std::find_if(interfaces.begin(), interfaces.end(),
                     [](const auto& i) { return i.first == LOGGING_IFACE; });

    if (propertyMap != interfaces.end())
    {
        create(path, propertyMap->second);
    }
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
}
}
