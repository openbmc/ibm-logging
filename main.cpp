/**
 * Copyright © 2018 IBM Corporation
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include <sdbusplus/bus.hpp>
#include <sdbusplus/server/manager.hpp>
#include "delete_all.hpp"
#include "manager.hpp"
#include "config.h"

int main()
{
    auto bus = sdbusplus::bus::new_default();

    sdbusplus::server::manager::manager objManager(bus, LOGGING_PATH);

    ibm::logging::Manager manager{bus};
    ibm::logging::DeleteAll da{bus, LOGGING_PATH, manager};

    bus.request_name(IBM_LOGGING_BUSNAME);

    while (true)
    {
        bus.process_discard();
        bus.wait();
    }

    return 0;
}
