#pragma once

#include <experimental/filesystem>
#include "dbus.hpp"
#include "interfaces.hpp"

namespace ibm
{
namespace logging
{

namespace fs = std::experimental::filesystem;

/**
 *  @class Callout
 *
 *  This class provides information about a callout by utilizing the
 *  xyz.openbmc_project.Inventory.Decorator.Asset and
 *  xyz.openbmc_project.Common.ObjectPath interfaces.
 *
 *  It also has the ability to persist and restore its data.
 */
class Callout : public CalloutObject
{
  public:
    Callout() = delete;
    Callout(const Callout&) = delete;
    Callout& operator=(const Callout&) = delete;
    Callout(Callout&&) = default;
    Callout& operator=(Callout&&) = default;
    ~Callout() = default;

    /**
     * Constructor
     *
     * Populates the Asset D-Bus properties with data from the property map.
     *
     * @param[in] bus - D-Bus object
     * @param[in] objectPath - object path
     * @param[in] inventoryPath - inventory path of the callout
     * @param[in] id - which callout this is
     * @param[in] timestamp - timestamp when the log was created
     * @param[in] properties - the properties for the Asset interface.
     */
    Callout(sdbusplus::bus::bus& bus, const std::string& objectPath,
            const std::string& inventoryPath, size_t id, uint64_t timestamp,
            const DbusPropertyMap& properties);
    /**
     * Constructor
     *
     * This version is for when the object is being restored and does
     * not take the properties map.
     *
     * @param[in] bus - D-Bus object
     * @param[in] objectPath - object path
     * @param[in] id - which callout this is
     * @param[in] timestamp - timestamp when the log was created
     * @param[in] properties - the properties for the Asset interface.
     */
    Callout(sdbusplus::bus::bus& bus, const std::string& objectPath, size_t id,
            uint64_t timestamp);

    /**
     * Returns the callout ID
     *
     * @return id - the ID
     */
    inline auto id() const
    {
        return entryID;
    }

    /**
     * Sets the callout ID
     *
     * @param[in] id - the ID
     */
    inline void id(uint32_t id)
    {
        entryID = id;
    }

    /**
     * Returns the timestamp
     *
     * @return timestamp
     */
    inline auto ts() const
    {
        return timestamp;
    }

    /**
     * Sets the timestamp
     *
     * @param[in] ts - the timestamp
     */
    inline void ts(uint64_t ts)
    {
        timestamp = ts;
    }

  private:
    /**
     * The unique identifier for the callout, as error logs can have
     * multiple callouts.  They start at 0.
     */
    size_t entryID;

    /**
     * The timestamp of when the error log was created.
     * Used for ensuring the callout data is being restored for
     * the correct error log.
     */
    uint64_t timestamp;
};
}
}
