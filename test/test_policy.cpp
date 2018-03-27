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
#include <fstream>
#include <gtest/gtest.h>
#include <experimental/filesystem>
#include "policy_table.hpp"
#include "policy_find.hpp"

using namespace ibm::logging;
namespace fs = std::experimental::filesystem;

static constexpr auto json = R"(
[
    {
    "dtls":[
      {
        "CEID":"ABCD1234",
        "mod":"",
        "msg":"Error ABCD1234"
      }
    ],
    "err":"xyz.openbmc_project.Error.Test1"
    },

    {
    "dtls":[
      {
        "CEID":"XYZ222",
        "mod":"",
        "msg":"Error XYZ222"
      }
    ],
    "err":"xyz.openbmc_project.Error.Test2"
    },

    {
    "dtls":[
      {
        "CEID":"AAAAAA",
        "mod":"mod1",
        "msg":"Error AAAAAA"
      },
      {
        "CEID":"BBBBBB",
        "mod":"mod2",
        "msg":"Error BBBBBB"
      },
      {
        "CEID":"CCCCCC",
        "mod":"mod3",
        "msg":"Error CCCCCC"
      }
    ],
    "err":"xyz.openbmc_project.Error.Test3"
    },

    {
    "dtls":[
      {
        "CEID":"DDDDDDDD",
        "mod":"I2C",
        "msg":"Error DDDDDDDD"
      },
      {
        "CEID":"EEEEEEEE",
        "mod":"FSI",
        "msg":"Error EEEEEEEE"
      }
    ],
    "err":"xyz.openbmc_project.Error.Test4"
    },

    {
    "dtls":[
      {
        "CEID":"FFFFFFFF",
        "mod":"6D",
        "msg":"Error FFFFFFFF"
      }
    ],

    "err":"xyz.openbmc_project.Error.Test5"
    },
    {
    "dtls":[
      {
        "CEID":"GGGGGGGG",
        "mod":"RAIL_5",
        "msg":"Error GGGGGGGG"
      }
    ],

    "err":"xyz.openbmc_project.Error.Test6"
    },
    {
    "dtls":[
      {
        "CEID":"HHHHHHHH",
        "mod":"INPUT_42",
        "msg":"Error HHHHHHHH"
      }
    ],
    "err":"xyz.openbmc_project.Error.Test7"
    }
])";


/**
 * Helper class to write the above json to a file and then
 * remove it when the tests are over.
 */
class PolicyTableTest : public ::testing::Test
{
    protected:

        virtual void SetUp()
        {
            char dir[] = {"/tmp/jsonTestXXXXXX"};

            jsonDir = mkdtemp(dir);
            jsonFile = jsonDir / "policy.json";

            std::ofstream f{jsonFile};
            f << json;
        }

        virtual void TearDown()
        {
            fs::remove_all(jsonDir);
        }

        fs::path jsonDir;
        fs::path jsonFile;
};


/**
 * Test finding entries in the policy table
 */
TEST_F(PolicyTableTest, TestTable)
{
    {
        policy::Table policy{jsonFile};
        ASSERT_EQ(policy.isLoaded(), true);

        ////////////////////////////////////
        //Basic search, no modifier
        std::string err{"xyz.openbmc_project.Error.Test2"};
        std::string mod;

        auto details = policy.find(err, mod);
        if (details)
        {
            ASSERT_EQ((*details)->ceid, "XYZ222");
            ASSERT_EQ((*details)->msg, "Error XYZ222");
        }
        else
        {
            ASSERT_EQ(true, false);
        }

        /////////////////////////////////////
        //Not found
        err = "foo";
        details = policy.find(err, mod);
        if (details)
        {
            ASSERT_EQ(true, false);
        }

        /////////////////////////////////////
        //Test with a modifier
        err = "xyz.openbmc_project.Error.Test3";
        mod = "mod3";

        details = policy.find(err, mod);
        if (details)
        {
            ASSERT_EQ((*details)->ceid, "CCCCCC");
            ASSERT_EQ((*details)->msg, "Error CCCCCC");
        }
        else
        {
            ASSERT_EQ(true, false);
        }
    }
}

/**
 * Test policy::find() that uses the data from a property
 * map to find entries in the policy table.
 */
TEST_F(PolicyTableTest, TestFinder)
{
    using namespace std::literals::string_literals;

    policy::Table policy{jsonFile};
    ASSERT_EQ(policy.isLoaded(), true);

    //A basic search with no modifier
    {
        DbusPropertyMap testProperties
        {
            {"Message"s, Value{"xyz.openbmc_project.Error.Test1"s}}
        };

        auto values = policy::find(policy, testProperties);
        ASSERT_EQ(std::get<policy::EIDField>(values), "ABCD1234");
        ASSERT_EQ(std::get<policy::MsgField>(values), "Error ABCD1234");
    }

    //Use CALLOUT_INVENTORY_PATH from the AdditionalData property
    {
        std::vector<std::string> ad{
                "FOO=BAR"s,
                "CALLOUT_INVENTORY_PATH=mod2"s};
        DbusPropertyMap testProperties
        {
            {"Message"s, Value{"xyz.openbmc_project.Error.Test3"s}},
            {"AdditionalData"s, ad}
        };

        auto values = policy::find(policy, testProperties);
        ASSERT_EQ(std::get<policy::EIDField>(values), "BBBBBB");
        ASSERT_EQ(std::get<policy::MsgField>(values), "Error BBBBBB");
    }

    //Use an I2C DEVICE_PATH from the AdditionalData property
    {
        std::vector<std::string> ad{
                "FOO=BAR"s,
                "CALLOUT_DEVICE_PATH=/some/i2c/path"s};
        DbusPropertyMap testProperties
        {
            {"Message"s, Value{"xyz.openbmc_project.Error.Test4"s}},
            {"AdditionalData"s, ad}
        };

        auto values = policy::find(policy, testProperties);
        ASSERT_EQ(std::get<policy::EIDField>(values), "DDDDDDDD");
        ASSERT_EQ(std::get<policy::MsgField>(values), "Error DDDDDDDD");
    }

    //Use an FSI DEVICE_PATH from the AdditionalData property
    {
        std::vector<std::string> ad{
                "FOO=BAR"s,
                "CALLOUT_DEVICE_PATH=/some/fsi/path"s};
        DbusPropertyMap testProperties
        {
            {"Message"s, Value{"xyz.openbmc_project.Error.Test4"s}},
            {"AdditionalData"s, ad}
        };

        auto values = policy::find(policy, testProperties);
        ASSERT_EQ(std::get<policy::EIDField>(values), "EEEEEEEE");
        ASSERT_EQ(std::get<policy::MsgField>(values), "Error EEEEEEEE");
    }

    //Use PROCEDURE from the AdditionalData property
    {
        std::vector<std::string> ad{
                "FOO=BAR"s,
                "PROCEDURE=109"s};
        DbusPropertyMap testProperties
        {
            {"Message"s, Value{"xyz.openbmc_project.Error.Test5"s}},
            {"AdditionalData"s, ad}
        };

        auto values = policy::find(policy, testProperties);
        ASSERT_EQ(std::get<policy::EIDField>(values), "FFFFFFFF");
        ASSERT_EQ(std::get<policy::MsgField>(values), "Error FFFFFFFF");
    }

    //Use RAIL_NAME from the AdditionalData property
    {
        std::vector<std::string> ad{
                "FOO=BAR"s,
                "RAIL_NAME=RAIL_5"s};
        DbusPropertyMap testProperties
        {
            {"Message"s, Value{"xyz.openbmc_project.Error.Test6"s}},
            {"AdditionalData"s, ad}
        };

        auto values = policy::find(policy, testProperties);
        ASSERT_EQ(std::get<policy::EIDField>(values), "GGGGGGGG");
        ASSERT_EQ(std::get<policy::MsgField>(values), "Error GGGGGGGG");
    }

    //Use INPUT_NAME from the AdditionalData property
    {
        std::vector<std::string> ad{
                "FOO=BAR"s,
                "INPUT_NAME=INPUT_42"s};
        DbusPropertyMap testProperties
        {
            {"Message"s, Value{"xyz.openbmc_project.Error.Test7"s}},
            {"AdditionalData"s, ad}
        };

        auto values = policy::find(policy, testProperties);
        ASSERT_EQ(std::get<policy::EIDField>(values), "HHHHHHHH");
        ASSERT_EQ(std::get<policy::MsgField>(values), "Error HHHHHHHH");
    }

    //Test not finding an entry.
    {
        DbusPropertyMap testProperties
        {
            {"Message"s, Value{"hello world"s}}
        };

        auto values = policy::find(policy, testProperties);
        ASSERT_EQ(std::get<policy::EIDField>(values), policy.defaultEID());
        ASSERT_EQ(std::get<policy::MsgField>(values), policy.defaultMsg());
    }
}

