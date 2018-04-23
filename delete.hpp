#pragma once

#include "interfaces.hpp"
#include "manager.hpp"

namespace ibm
{
namespace logging
{

/**
 *  @class Delete
 *
 *  Implements the xyz.openbmc_project.Object.Delete interface
 *  to delete an IBM logging object.
 */
class Delete : public DeleteObject
{
  public:
    Delete() = delete;
    Delete(const Delete&) = delete;
    Delete& operator=(const Delete&) = delete;
    Delete(Delete&&) = default;
    Delete& operator=(Delete&&) = default;
    virtual ~Delete() = default;

    /**
     * Constructor
     *
     * @param[in] bus - the D-Bus bus object
     * @param[in] path - the object path
     * @param[in] manager - the Manager object
     * @param[in] deferSignals - if the object creation signals
     *                           should be deferred
     */
    Delete(sdbusplus::bus::bus& bus, const std::string& path, Manager& manager,
           bool deferSignals) :
        DeleteObject(bus, path.c_str(), deferSignals),
        path(path), manager(manager)
    {
    }

    /**
     * The Delete D-Bus method
     */
    inline void delete_() override
    {
        manager.erase(path);
    }

  private:
    /**
     * The logging entry object path
     */
    const std::string path;

    /**
     * The Manager object
     */
    Manager& manager;
};
}
}
