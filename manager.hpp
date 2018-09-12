#pragma once

#include "config.h"

#include "dbus.hpp"
#include "interfaces.hpp"

#include <experimental/any>
#include <experimental/filesystem>
#include <map>
#include <sdbusplus/bus.hpp>
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

  private:
    using EntryID = uint32_t;
    using InterfaceMap = std::map<InterfaceType, std::experimental::any>;
    using EntryMap = std::map<EntryID, InterfaceMap>;

    using ObjectList = std::vector<std::experimental::any>;
    using InterfaceMapMulti = std::map<InterfaceType, ObjectList>;
    using EntryMapMulti = std::map<EntryID, InterfaceMapMulti>;

    /**
     * Deletes the entry and any child entries with
     * the specified ID.
     *
     * @param[in] id - the entry ID
     */
    void erase(EntryID id);

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
     * The callback for an interfaces removed signal
     *
     * Removes the IBM interfaces for the log entry
     * that was just removed.
     *
     * @param[in] msg - the sdbusplus message
     */
    void interfaceRemoved(sdbusplus::message::message& msg);

    /**
     * Creates the IBM interfaces for all existing error log
     * entries.
     */
    void createAll();

    /**
     * Creates the IBM interface(s) for a single new error log.
     *
     * Any interfaces that require serialization will be created
     * and serialized here.
     *
     * @param[in] objectPath - object path of the error log
     * @param[in] interfaces - map of all interfaces and properties
     *                         on a phosphor-logging error log
     */
    void create(const std::string& objectPath,
                const DbusInterfaceMap& interfaces);

    /**
     * Creates the IBM interface(s) for a single error log after
     * the application is restarted.
     *
     * Interfaces that were persisted will be restored from their
     * previously saved filesystem data.
     *
     * @param[in] objectPath - object path of the error log
     * @param[in] interfaces - map of all interfaces and properties
     *                         on a phosphor-logging error log
     */
    void createWithRestore(const std::string& objectPath,
                           const DbusInterfaceMap& interfaces);

    /**
     * Creates the IBM interfaces for a single error log that
     * do not persist across app restarts.
     *
     * @param[in] objectPath - object path of the error log
     * @param[in] interfaces - map of all interfaces and properties
     *                         on a phosphor-logging error log
     */
    void createObject(const std::string& objectPath,
                      const DbusInterfaceMap& interfaces);

    /**
     * Returns the error log timestamp property value from
     * the passed in map of all interfaces and property names/values
     * on an error log D-Bus object.
     *
     * @param[in] interfaces - map of all interfaces and properties
     *                         on a phosphor-logging error log.
     *
     * @return uint64_t - the timestamp
     */
    uint64_t getLogTimestamp(const DbusInterfaceMap& interfaces);

    /**
     * Returns the filesystem directory to use for persisting
     * information about a particular error log.
     *
     * @param[in] id - the error log ID
     * @return path - the directory path
     */
    std::experimental::filesystem::path getSaveDir(EntryID id);

    /**
     * Returns the directory to use to save the callout information in
     *
     * @param[in] id - the error log ID
     *
     * @return path - the directory path
     */
    std::experimental::filesystem::path getCalloutSaveDir(EntryID id);

    /**
     * Returns the D-Bus object path to use for a callout D-Bus object.
     *
     * @param[in] objectPath - the object path for the error log
     * @param[in] calloutNum - the callout instance number
     *
     * @return path - the object path to use for a callout object
     */
    std::string getCalloutObjectPath(const std::string& objectPath,
                                     uint32_t calloutNum);

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
     * Creates D-Bus objects for any callouts in an error log
     * that map to an inventory object with an Asset interface.
     *
     * The created object will also host the Asset interface.
     *
     * A callout object path would look like:
     * /xyz/openbmc_project/logging/entry/5/callouts/0.
     *
     * Any objects created are serialized so the asset information
     * can always be restored.
     *
     * @param[in] objectPath - object path of the error log
     * @param[in] interfaces - map of all interfaces and properties
     *                         on a phosphor-logging error log.
     */
    void createCalloutObjects(const std::string& objectPath,
                              const DbusInterfaceMap& interfaces);

    /**
     * Restores callout objects for a particular error log that
     * have previously been saved by reading their data out of
     * the filesystem using Cereal.
     *
     * @param[in] objectPath - object path of the error log
     * @param[in] interfaces - map of all interfaces and properties
     *                         on a phosphor-logging error log.
     */
    void restoreCalloutObjects(const std::string& objectPath,
                               const DbusInterfaceMap& interfaces);

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
     * Adds an interface to a child object, which is an object that
     * relates to the main ...logging/entry/X object but has a different path.
     * The object is stored in the childEntries map.
     *
     * There can be multiple instances of a child object per type per
     * logging object.
     *
     * @param[in] objectPath - the object path of the log
     * @param[in] type - the interface type being added.
     * @param[in] object - the interface object
     */
    void addChildInterface(const std::string& objectPath, InterfaceType type,
                           std::experimental::any& object);

    /**
     * The sdbusplus bus object
     */
    sdbusplus::bus::bus& bus;

    /**
     * The match object for interfacesAdded
     */
    sdbusplus::bus::match_t addMatch;

    /**
     * The match object for interfacesRemoved
     */
    sdbusplus::bus::match_t removeMatch;

    /**
     * A map of the error log IDs to their IBM interface objects.
     * There may be multiple interfaces per ID.
     */
    EntryMap entries;

    /**
     * A map of the error log IDs to their interface objects which
     * are children of the logging objects.
     *
     * These objects have the same lifespan as their parent objects.
     *
     * There may be multiple interfaces per ID, and also multiple
     * interface instances per interface type.
     */
    EntryMapMulti childEntries;

#ifdef USE_POLICY_INTERFACE
    /**
     * The class the wraps the IBM error logging policy table.
     */
    policy::Table policies;
#endif
};
} // namespace logging
} // namespace ibm
