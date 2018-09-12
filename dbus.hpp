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
using DbusService = std::string;
using DbusPath = std::string;

static constexpr auto forwardPos = 0;
static constexpr auto reversePos = 1;
static constexpr auto endpointPos = 2;
using AssociationsPropertyType =
    std::vector<std::tuple<std::string, std::string, std::string>>;

using Value = sdbusplus::message::variant<bool, uint32_t, uint64_t, std::string,
                                          std::vector<std::string>,
                                          AssociationsPropertyType>;

using DbusPropertyMap = std::map<DbusProperty, Value>;
using DbusInterfaceMap = std::map<DbusInterface, DbusPropertyMap>;
using DbusInterfaceList = std::vector<DbusInterface>;

using ObjectValueTree =
    std::map<sdbusplus::message::object_path, DbusInterfaceMap>;

using DbusSubtree =
    std::map<DbusPath, std::map<DbusService, DbusInterfaceList>>;

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
 * Returns the subtree for a root, depth, and interface.
 *
 * Returns an empty map if there are any failures.
 *
 * @param[in] bus - the D-Bus object
 * @param[in] root - the point from which to provide results
 * @param[in] depth - the number of path elements to descend
 *
 * @return DbusSubtree - A map of object paths to their
 *                       services and interfaces.
 */
DbusSubtree getSubtree(sdbusplus::bus::bus& bus, const std::string& root,
                       int depth, const std::string& interface);

/**
 * Get the D-Bus service name for the object path and interface from
 * the data returned from a GetSubTree call.
 *
 * Returns an empty string if the service can't be found.
 *
 * @param[in] objPath - the D-Bus object path
 * @param[in] interface - the D-Bus interface name
 * @param[in] tree - the D-Bus GetSubTree response
 *
 * @return string - the service name
 */
DbusService getService(const std::string& objPath, const std::string& interface,
                       const DbusSubtree& tree);

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
} // namespace logging
} // namespace ibm
