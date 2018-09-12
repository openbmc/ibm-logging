#pragma once

#include "dbus.hpp"
#include "policy_table.hpp"

namespace ibm
{
namespace logging
{
namespace policy
{

constexpr auto EIDField = 0;
constexpr auto MsgField = 1;
using PolicyProps = std::tuple<std::string, std::string>;

/**
 * Finds the policy table details based on the properties
 * in the xyz.openbmc_project.Logging.Entry interface.
 *
 * @param[in] policy - the policy table object
 * @param[in] errorLogProperties - the map of the error log
 *            properties for the xyz.openbmc_project.Logging.Entry
 *            interface
 * @return PolicyProps - a tuple of policy details.
 */
PolicyProps find(const Table& policy,
                 const DbusPropertyMap& errorLogProperties);
} // namespace policy
} // namespace logging
} // namespace ibm
