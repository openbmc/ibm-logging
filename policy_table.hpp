#pragma once

#include <experimental/optional>
#include <map>
#include <vector>
#include "config.h"

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

using DetailsList = std::vector<Details>;
using DetailsIterator = DetailsList::const_iterator;

using PolicyMap = std::map<std::string, DetailsList>;

namespace optional_ns = std::experimental;

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
         * @return optional<DetailsIterator> - an interator to the details
         *                                     entry
         */
        optional_ns::optional<DetailsIterator> find(
                const std::string& error,
                const std::string& modifier) const;

    private:

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


}
}
}
