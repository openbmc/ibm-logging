#pragma once

#include <com/ibm/Logging/Policy/server.hpp>
#include <xyz/openbmc_project/Common/ObjectPath/server.hpp>
#include <xyz/openbmc_project/Inventory/Decorator/Asset/server.hpp>

namespace ibm
{
namespace logging
{

template <typename... T>
using ServerObject = typename sdbusplus::server::object::object<T...>;

using ObjectPathInterface =
    sdbusplus::xyz::openbmc_project::Common::server::ObjectPath;

using CalloutInterface =
    sdbusplus::xyz::openbmc_project::Inventory::Decorator::server::Asset;
using CalloutObject = ServerObject<CalloutInterface, ObjectPathInterface>;

using PolicyInterface = sdbusplus::com::ibm::Logging::server::Policy;
using PolicyObject = ServerObject<PolicyInterface>;

enum class InterfaceType
{
    CALLOUT,
    POLICY
};
} // namespace logging
} // namespace ibm
