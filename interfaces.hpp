#pragma once

#include <com/ibm/Logging/Policy/server.hpp>

namespace ibm
{
namespace logging
{

template <typename... T>
using ServerObject = typename sdbusplus::server::object::object<T...>;

using PolicyInterface = sdbusplus::com::ibm::Logging::server::Policy;
using PolicyObject = ServerObject<PolicyInterface>;

enum class InterfaceType
{
    POLICY
};
}
}
