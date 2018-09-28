#pragma once

#include "config.h"

#include <experimental/optional>
#include <map>
#include <vector>

namespace ibm
{
namespace logging
{
namespace policy
{

/**
 *  The details of a policy table entry:
 *  - search modifier
 *  - error message
 *  - common error event ID
 */
struct Details
{
    std::string modifier;
    std::string msg;
    std::string ceid;
};

namespace optional_ns = std::experimental;

using DetailsList = std::vector<Details>;
using DetailsReference = std::reference_wrapper<const Details>;
using FindResult = optional_ns::optional<DetailsReference>;

using PolicyMap = std::map<std::string, DetailsList>;

/**
 * @class Table
 *
 * This class wraps the error policy table data, and provides the
 * ability to find a policy table entry based on the error and a
 * search modifier.  This data contains additional information
 * about error logs and may be system specific.
 */
class Table
{
  public:
    Table() = delete;
    ~Table() = default;
    Table(const Table&) = default;
    Table& operator=(const Table&) = default;
    Table(Table&&) = default;
    Table& operator=(Table&&) = default;

    /**
     * Constructor
     *
     * @param[in] jsonFile - the path to the policy JSON.
     */
    explicit Table(const std::string& jsonFile);

    /**
     * Says if the JSON has been loaded successfully.
     *
     * @return bool
     */
    inline bool isLoaded() const
    {
        return loaded;
    }

    /**
     * Finds an entry in the policy table based on the
     * error and the search modifier.
     *
     * @param[in] error - the error, like xyz.openbmc_project.Error.X
     * @param[in] modifier - the search modifier, used to find the entry
     *                   when multiple ones share the same error
     *
     * @return optional<DetailsReference> - the details entry
     */
    FindResult find(const std::string& error,
                    const std::string& modifier) const;

    /**
     * The default event ID to use when a match in the table
     * wasn't found.
     *
     * @return std::string
     */
    inline std::string defaultEID() const
    {
        return defaultPolicyEID;
    }

    /**
     * The default error message to use when a match in the table
     * wasn't found.
     *
     * @return std::string
     */
    inline std::string defaultMsg() const
    {
        return defaultPolicyMessage;
    }

  private:
    /**
     * The default event ID
     */
    const std::string defaultPolicyEID{DEFAULT_POLICY_EID};

    /**
     * The default event message
     */
    const std::string defaultPolicyMessage{DEFAULT_POLICY_MSG};

    /**
     * Loads the JSON data into the PolicyMap map
     *
     * @param[in] jsonFile - the path to the .json file
     */
    void load(const std::string& jsonFile);

    /**
     * Reflects if the JSON was successfully loaded or not.
     */
    bool loaded = false;

    /**
     * The policy table
     */
    PolicyMap policies;
};
} // namespace policy
} // namespace logging
} // namespace ibm
