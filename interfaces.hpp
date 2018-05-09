#pragma once

#include <com/ibm/Logging/Policy/server.hpp>
#include <xyz/openbmc_project/Object/Delete/server.hpp>

namespace ibm
{
namespace logging
{

template <typename... T>
using ServerObject = typename sdbusplus::server::object::object<T...>;

using DeleteInterface = sdbusplus::xyz::openbmc_project::Object::server::Delete;
using DeleteObject = ServerObject<DeleteInterface>;

using PolicyInterface = sdbusplus::com::ibm::Logging::server::Policy;
using PolicyObject = ServerObject<PolicyInterface>;

enum class InterfaceType
{
    POLICY,
    DELETE
};
}
}
