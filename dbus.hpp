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
using Value = sdbusplus::message::variant<bool, uint32_t, uint64_t, std::string,
                                          std::vector<std::string>>;

using DbusPropertyMap = std::map<DbusProperty, Value>;
using DbusInterfaceMap = std::map<DbusInterface, DbusPropertyMap>;
using DbusInterfaceList = std::vector<DbusInterface>;

using ObjectValueTree =
    std::map<sdbusplus::message::object_path, DbusInterfaceMap>;

/**
 * Returns the managed objects for an object path and service
 *
 * Returns an empty map if there are any failures.
 *
 * @param[in] bus - the D-Bus object
 * @param[in] service - the D-Bus service name
 * @param[in] objPath - the D-Bus object path
 *
 * @return ObjectValueTree - A map of object paths to their
 *                           interfaces and properties.
 */
ObjectValueTree getManagedObjects(sdbusplus::bus::bus& bus,
                                  const std::string& service,
                                  const std::string& objPath);

/**
 * Get the D-Bus service name for the object path and interface.
 *
 * Returns an empty string if the service can't be found.
 *
 * @param[in] bus - the D-Bus object
 * @param[in] objPath - the D-Bus object path
 * @param[in] interface - the D-Bus interface name
 *
 * @return string - the service name
 */
std::string getService(sdbusplus::bus::bus& bus, const std::string& path,
                       const std::string& interface);

/**
 * Returns all properties on a particular interface on a
 * particular D-Bus object.
 *
 * Returns an empty map if there are any failures.
 *
 * @param[in] bus - the D-Bus object
 * @param[in] service - the D-Bus service name
 * @param[in] objPath - the D-Bus object path
 * @param[in] interface - the D-Bus interface name
 *
 * @return DbusPropertyMap - The map of property names to values
 */
DbusPropertyMap getAllProperties(sdbusplus::bus::bus& bus,
                                 const std::string& service,
                                 const std::string& objPath,
                                 const std::string& interface);
}
}
