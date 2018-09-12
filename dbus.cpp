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
#include "dbus.hpp"

#include <phosphor-logging/log.hpp>

namespace ibm
{
namespace logging
{

constexpr auto MAPPER_BUSNAME = "xyz.openbmc_project.ObjectMapper";
constexpr auto MAPPER_PATH = "/xyz/openbmc_project/object_mapper";
constexpr auto MAPPER_IFACE = "xyz.openbmc_project.ObjectMapper";
constexpr auto PROPERTY_IFACE = "org.freedesktop.DBus.Properties";

using namespace phosphor::logging;

ObjectValueTree getManagedObjects(sdbusplus::bus::bus& bus,
                                  const std::string& service,
                                  const std::string& objPath)
{
    ObjectValueTree interfaces;

    auto method = bus.new_method_call(service.c_str(), objPath.c_str(),
                                      "org.freedesktop.DBus.ObjectManager",
                                      "GetManagedObjects");

    auto reply = bus.call(method);

    if (reply.is_method_error())
    {
        log<level::ERR>("Failed to get managed objects",
                        entry("SERVICE=%s", service.c_str()),
                        entry("PATH=%s", objPath.c_str()));
    }
    else
    {
        reply.read(interfaces);
    }

    return interfaces;
}

DbusPropertyMap getAllProperties(sdbusplus::bus::bus& bus,
                                 const std::string& service,
                                 const std::string& objPath,
                                 const std::string& interface)
{
    DbusPropertyMap properties;

    auto method = bus.new_method_call(service.c_str(), objPath.c_str(),
                                      PROPERTY_IFACE, "GetAll");
    method.append(interface);
    auto reply = bus.call(method);

    if (reply.is_method_error())
    {
        log<level::ERR>("Failed to get all properties",
                        entry("SERVICE=%s", service.c_str()),
                        entry("PATH=%s", objPath.c_str()),
                        entry("INTERFACE=%s", interface.c_str()));
    }
    else
    {
        reply.read(properties);
    }

    return properties;
}

DbusSubtree getSubtree(sdbusplus::bus::bus& bus, const std::string& root,
                       int depth, const std::string& interface)
{
    DbusSubtree tree;

    auto method = bus.new_method_call(MAPPER_BUSNAME, MAPPER_PATH, MAPPER_IFACE,
                                      "GetSubTree");
    method.append(root);
    method.append(depth);
    method.append(std::vector<std::string>({interface}));
    auto reply = bus.call(method);

    if (reply.is_method_error())
    {
        log<level::ERR>("Failed to get subtree", entry("ROOT=%s", root.c_str()),
                        entry("INTERFACE=%s", interface.c_str()));
    }
    else
    {
        reply.read(tree);
    }

    return tree;
}

DbusService getService(const std::string& objPath, const std::string& interface,
                       const DbusSubtree& tree)
{
    DbusService service;

    auto services = tree.find(objPath);
    if (services != tree.end())
    {
        auto s = std::find_if(services->second.begin(), services->second.end(),
                              [&interface](const auto& entry) {
                                  auto i =
                                      std::find(entry.second.begin(),
                                                entry.second.end(), interface);
                                  return i != entry.second.end();
                              });
        if (s != services->second.end())
        {
            service = s->first;
        }
    }

    return service;
}
} // namespace logging
} // namespace ibm
