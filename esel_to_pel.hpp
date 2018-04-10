#pragma once

#include <iostream>
#include <sdbusplus/bus.hpp>
#include <sdbusplus/server/object.hpp>
#include <sdbusplus/server.hpp>
#include "org/open_power/Logging/EselToPel/server.hpp"
#include "config.h"
#include "pel.hpp"


namespace openpower
{
namespace logging
{
namespace pel
{

class Manager
{
    public:
        Manager() = delete;
        Manager(const Manager&) = delete;
        Manager& operator=(const Manager&) = delete;
        Manager(Manager&&) = default;
        Manager& operator=(Manager&&) = default;
        ~Manager() = default;

        Manager(sdbusplus::bus::bus& bus):
                matchCreated(bus,
                    sdbusplus::bus::match::rules::interfacesAdded() +
                    sdbusplus::bus::match::rules::path_namespace(
                    "/xyz/openbmc_project/logging"),
                    std::bind(std::mem_fn(&Manager::created),
                    this, std::placeholders::_1)),bus(bus)
                {
                }

    private:
        sdbusplus::bus::match_t matchCreated;

        std::string objPath;
        std::string eselData;
        std::string eselStr;

        sdbusplus::bus::bus& bus;

        std::vector<std::unique_ptr<openpower::logging::Pel>> pelObjects;

        void created(sdbusplus::message::message& msg);

        //void processPels(sdbusplus::bus::bus& bus);
};//end class Manager
}//end namespace pel

}//end namespace logging
}//end namespace openpower
