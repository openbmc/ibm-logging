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
 *  Implements the xyz.openbmc_project.Collect.DeleteAll interface
 *  that will delete all ibm-logging entries.
 */
class DeleteAll : public DeleteAllObject
{
  public:
    DeleteAll() = delete;
    DeleteAll(const DeleteAll&) = delete;
    DeleteAll& operator=(const DeleteAll&) = delete;
    DeleteAll(DeleteAll&&) = default;
    DeleteAll& operator=(DeleteAll&&) = default;
    virtual ~DeleteAll() = default;

    /**
     * Constructor
     *
     * @param[in] bus - the D-Bus bus object
     * @param[in] path - the object path
     * @param[in] manager - the Manager object
     */
    DeleteAll(sdbusplus::bus::bus& bus, const std::string& path,
              Manager& manager) :
        DeleteAllObject(bus, path.c_str()),
        manager(manager)
    {
    }

    /**
     * The Delete D-Bus method
     */
    inline void deleteAll() override
    {
        manager.eraseAll();
    }

  private:
    /**
     * The Manager object
     */
    Manager& manager;
};
}
}
