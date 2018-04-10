#pragma once

#include <iostream>
#include <sdbusplus/bus.hpp>
#include <sdbusplus/server/object.hpp>
#include <sdbusplus/server.hpp>
#include "config.h"
#include "org/open_power/Logging/EselToPel/server.hpp"


namespace openpower
{

namespace logging
{

using PelIfaces = sdbusplus::server::object::object<
    sdbusplus::org::open_power::Logging::server::EselToPel>;


class Pel : public PelIfaces
{
    public:
        Pel() = delete;
        Pel(const Pel&) = delete;
        Pel operator=(const Pel&) = delete;
        Pel(Pel&&) = delete;
        Pel& operator=(Pel&&) = delete;
        virtual ~Pel() = default;

        Pel(sdbusplus::bus::bus& bus,
            const std::string& path,
            const std::string text):
            PelIfaces(bus,path.c_str())
            {
                pel(text);
                this->emit_object_added();
            }

};//end class Pel

}//namespace logging
} //namespace openpower
