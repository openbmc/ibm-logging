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
#include "dbus.hpp"

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
        using namespace phosphor::logging;
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

std::string getService(sdbusplus::bus::bus& bus, const std::string& objPath,
                       const std::string& interface)
{
    auto method = bus.new_method_call(MAPPER_BUSNAME, MAPPER_PATH, MAPPER_IFACE,
                                      "GetObject");

    method.append(objPath);
    method.append(std::vector<std::string>({interface}));

    auto reply = bus.call(method);
    if (reply.is_method_error())
    {
        return std::string{};
    }

    std::map<std::string, std::vector<std::string>> response;
    reply.read(response);

    if (response.empty())
    {
        return std::string{};
    }

    return response.begin()->first;
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
        using namespace phosphor::logging;
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
}
}
