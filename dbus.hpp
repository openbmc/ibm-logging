#pragma once

#include <map>
#include <sdbusplus/server.hpp>
#include <string>
#include <vector>

namespace ibm
{
namespace logging
{

using DbusInterface = std::string;
using DbusProperty = std::string;
using Value = sdbusplus::message::variant<bool, uint32_t, uint64_t,
                                          std::string,
                                          std::vector<std::string>>;

using DbusPropertyMap = std::map<DbusProperty, Value>;
using DbusInterfaceMap = std::map<DbusInterface, DbusPropertyMap>;
using DbusInterfaceList = std::vector<DbusInterface>;

using ObjectValueTree =
    std::map<sdbusplus::message::object_path, DbusInterfaceMap>;

ObjectValueTree getManagedObjects(
        sdbusplus::bus::bus& bus,
        const std::string& service,
        const std::string& objPath);
}
}

