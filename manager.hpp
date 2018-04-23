#pragma once

#include <experimental/any>
#include <experimental/filesystem>
#include <map>
#include <sdbusplus/bus.hpp>
#include "config.h"
#include "dbus.hpp"
#include "interfaces.hpp"
#ifdef USE_POLICY_INTERFACE
#include "policy_table.hpp"
#endif

namespace ibm
{
namespace logging
{

/**
 * @class Manager
 *
 * This class hosts IBM specific interfaces for the error logging
 * entry objects.  It watches for interfaces added and removed
 * signals to know when to create and delete objects.  Handling the
 * xyz.openbmc_project.Logging service going away is done at the
 * systemd service level where this app will be stopped too.
 */
class Manager
{
  public:
    Manager() = delete;
    ~Manager() = default;
    Manager(const Manager&) = delete;
    Manager& operator=(const Manager&) = delete;
    Manager(Manager&&) = delete;
    Manager& operator=(Manager&&) = delete;

    /**
     * Constructor
     *
     * @param[in] bus - the D-Bus bus object
     */
    explicit Manager(sdbusplus::bus::bus& bus);

    /**
     * Deletes the entry with the specified path
     *
     * @param[in] objectPath - the entry object path
     */
    void erase(const std::string& objectPath);

    /**
     * Erases all entries
     */
    void eraseAll();

  private:
    /**
     * The callback for an interfaces added signal
     *
     * Creates the IBM interfaces for the log entry
     * that was just created.
     *
     * @param[in] msg - the sdbusplus message
     */
    void interfaceAdded(sdbusplus::message::message& msg);

    /**
     * Creates the IBM interfaces for all existing error log
     * entries.
     */
    void createAll();

    /**
     * Creates the IBM interface(s) for a single error log.
     *
     * @param[in] objectPath - object path of the error log
     * @param[in] properties - the xyz.openbmc_project.Logging.Entry
     *                         properties
     */
    void create(const std::string& objectPath,
                const DbusPropertyMap& properties);

    /**
     * Creates the IBM policy interface for a single error log
     * and saves it in the list of interfaces.
     *
     * @param[in] objectPath - object path of the error log
     * @param[in] properties - the xyz.openbmc_project.Logging.Entry
     *                         properties
     */
#ifdef USE_POLICY_INTERFACE
    void createPolicyInterface(const std::string& objectPath,
                               const DbusPropertyMap& properties);
#endif

    /**
     * Creates the Delete interface for a single error log
     * and saves it in the list of interfaces.
     *
     * @param[in] objectPath - object path of the error log
     */
    void createDeleteInterface(const std::string& objectPath);

    /**
     * Returns the entry ID for a log
     *
     * @param[in] objectPath - the object path of the log
     *
     * @return uint32_t - the ID
     */
    inline uint32_t getEntryID(const std::string& objectPath)
    {
        std::experimental::filesystem::path path(objectPath);
        return std::stoul(path.filename());
    }

    /**
     * Adds an interface object to the entries map
     *
     * @param[in] objectPath - the object path of the log
     * @param[in] type - the interface type being added
     * @param[in] object - the interface object
     */
    void addInterface(const std::string& objectPath, InterfaceType type,
                      std::experimental::any& object);

    /**
     * The sdbusplus bus object
     */
    sdbusplus::bus::bus& bus;

    /**
     * The match object for interfacesAdded
     */
    sdbusplus::bus::match_t addMatch;

    using EntryID = uint32_t;
    using InterfaceMap = std::map<InterfaceType, std::experimental::any>;
    using EntryMap = std::map<EntryID, InterfaceMap>;

    /**
     * A map of the error log IDs to their IBM interface objects.
     * There may be multiple interfaces per ID.
     */
    EntryMap entries;

#ifdef USE_POLICY_INTERFACE
    /**
     * The class the wraps the IBM error logging policy table.
     */
    policy::Table policies;
#endif
};
}
}
