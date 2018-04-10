#include <iostream>
#include <sdbusplus/bus.hpp>
#include <sdbusplus/server/manager.hpp>
#include <experimental/filesystem>
#include "config.h"
#include "pel.hpp"
#include "esel_to_pel.hpp"


int main(int argc, char *argv[])
{
    auto bus = sdbusplus::bus::new_default();

    sdbusplus::server::manager::manager
                       objManager(bus,"/xyz/openbmc_project/logging");

    bus.request_name("org.open_power.Logging");

    openpower::logging::pel::Manager monitor(bus);

    while (true)
    {
        bus.process_discard();
        bus.wait();
    }

    return 0;
}
