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

ObjectValueTree getManagedObjects(
        sdbusplus::bus::bus& bus,
        const std::string& service,
        const std::string& objPath)
{
    ObjectValueTree interfaces;

    auto method = bus.new_method_call(
                      service.c_str(),
                      objPath.c_str(),
                      "org.freedesktop.DBus.ObjectManager",
                      "GetManagedObjects");

    auto reply = bus.call(method);

    if (reply.is_method_error())
    {
        using namespace phosphor::logging;
        log<level::ERR>("Failed to get managed objects",
                entry("PATH=%s", objPath.c_str()));
    }
    else
    {
        reply.read(interfaces);
    }

    return interfaces;
}

}
}
