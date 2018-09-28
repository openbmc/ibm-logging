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
#include "policy_find.hpp"
#include "policy_table.hpp"

#include <experimental/filesystem>
#include <fstream>

#include <gtest/gtest.h>

using namespace ibm::logging;
namespace fs = std::experimental::filesystem;

static constexpr auto HOST_EVENT = "org.open_power.Host.Error.Event";

// ESEL contents all of the way up to right before the severity
// byte in the UH section
static const std::string eSELBase =
    "ESEL="
    "00 00 df 00 00 00 00 20 00 04 07 5a 04 aa 00 00 50 48 00 30 01 00 e5 00 "
    "00 00 f6 ca c9 da 5b b7 00 00 f6 ca d1 8a 2d e6 42 00 00 08 00 00 00 00 "
    "00 00 00 00 00 00 00 00 89 00 03 44 89 00 03 44 55 48 00 18 01 00 e5 00 "
    "13 03 ";

static const std::string noUHeSEL =
    "ESEL="
    "00 00 df 00 00 00 00 20 00 04 07 5a 04 aa 00 00 50 48 00 30 01 00 e5 00 "
    "00 00 f6 ca c9 da 5b b7 00 00 f6 ca d1 8a 2d e6 42 00 00 08 00 00 00 00 "
    "00 00 00 00 00 00 00 00 89 00 03 44 89 00 03 44 00 00 00 18 01 00 e5 00 "
    "13 03 10";

// ESEL Severity bytes
static const std::string SEV_RECOVERED = "10";
static const std::string SEV_PREDICTIVE = "20";
static const std::string SEV_UNRECOV = "40";
static const std::string SEV_CRITICAL = "50";
static const std::string SEV_DIAG = "60";

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
    },

    {
    "dtls":[
      {
        "CEID":"IIIIIII",
        "mod":"/match/this/path",
        "msg":"Error IIIIIII"
      }
    ],
    "err":"xyz.openbmc_project.Error.Test8"
    },

    {
    "dtls":[
      {
        "CEID":"JJJJJJJJ",
        "mod":"/inventory/core0||Warning",
        "msg":"Error JJJJJJJJ"
      },
      {
        "CEID":"KKKKKKKK",
        "mod":"/inventory/core1||Informational",
        "msg":"Error KKKKKKKK"
      },
      {
        "CEID":"LLLLLLLL",
        "mod":"/inventory/core2||Critical",
        "msg":"Error LLLLLLLL"
      },
      {
        "CEID":"MMMMMMMM",
        "mod":"/inventory/core3||Critical",
        "msg":"Error MMMMMMMM"
      },
      {
        "CEID":"NNNNNNNN",
        "mod":"/inventory/core4||Critical",
        "msg":"Error NNNNNNNN"
      },
      {
        "CEID":"OOOOOOOO",
        "mod":"/inventory/core5",
        "msg":"Error OOOOOOOO"
      },
      {
        "CEID":"PPPPPPPP",
        "mod":"/inventory/core5||Critical",
        "msg":"Error PPPPPPPP"
      }
    ],
    "err":"org.open_power.Host.Error.Event"
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
        char dir[] = {"./jsonTestXXXXXX"};

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
    policy::Table policy{jsonFile};
    ASSERT_EQ(policy.isLoaded(), true);

    ////////////////////////////////////
    // Basic search, no modifier
    std::string err{"xyz.openbmc_project.Error.Test2"};
    std::string mod;

    auto details = policy.find(err, mod);
    ASSERT_EQ(static_cast<bool>(details), true);
    if (details)
    {
        ASSERT_EQ((*details).get().ceid, "XYZ222");
        ASSERT_EQ((*details).get().msg, "Error XYZ222");
    }

    /////////////////////////////////////
    // Not found
    err = "foo";
    details = policy.find(err, mod);
    ASSERT_EQ(static_cast<bool>(details), false);

    /////////////////////////////////////
    // Test with a modifier
    err = "xyz.openbmc_project.Error.Test3";
    mod = "mod3";

    details = policy.find(err, mod);
    ASSERT_EQ(static_cast<bool>(details), true);
    if (details)
    {
        ASSERT_EQ((*details).get().ceid, "CCCCCC");
        ASSERT_EQ((*details).get().msg, "Error CCCCCC");
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

    // A basic search with no modifier
    {
        DbusPropertyMap testProperties{
            {"Message"s, Value{"xyz.openbmc_project.Error.Test1"s}}};

        auto values = policy::find(policy, testProperties);
        ASSERT_EQ(std::get<policy::EIDField>(values), "ABCD1234");
        ASSERT_EQ(std::get<policy::MsgField>(values), "Error ABCD1234");
    }

    // Use CALLOUT_INVENTORY_PATH from the AdditionalData property
    {
        std::vector<std::string> ad{"FOO=BAR"s, "CALLOUT_INVENTORY_PATH=mod2"s};
        DbusPropertyMap testProperties{
            {"Message"s, Value{"xyz.openbmc_project.Error.Test3"s}},
            {"AdditionalData"s, ad}};

        auto values = policy::find(policy, testProperties);
        ASSERT_EQ(std::get<policy::EIDField>(values), "BBBBBB");
        ASSERT_EQ(std::get<policy::MsgField>(values), "Error BBBBBB");
    }

    // Use an I2C DEVICE_PATH from the AdditionalData property
    {
        std::vector<std::string> ad{"FOO=BAR"s,
                                    "CALLOUT_DEVICE_PATH=/some/i2c/path"s};
        DbusPropertyMap testProperties{
            {"Message"s, Value{"xyz.openbmc_project.Error.Test4"s}},
            {"AdditionalData"s, ad}};

        auto values = policy::find(policy, testProperties);
        ASSERT_EQ(std::get<policy::EIDField>(values), "DDDDDDDD");
        ASSERT_EQ(std::get<policy::MsgField>(values), "Error DDDDDDDD");
    }

    // Use an FSI DEVICE_PATH from the AdditionalData property
    {
        std::vector<std::string> ad{"FOO=BAR"s,
                                    "CALLOUT_DEVICE_PATH=/some/fsi/path"s};
        DbusPropertyMap testProperties{
            {"Message"s, Value{"xyz.openbmc_project.Error.Test4"s}},
            {"AdditionalData"s, ad}};

        auto values = policy::find(policy, testProperties);
        ASSERT_EQ(std::get<policy::EIDField>(values), "EEEEEEEE");
        ASSERT_EQ(std::get<policy::MsgField>(values), "Error EEEEEEEE");
    }

    // Use PROCEDURE from the AdditionalData property
    {
        std::vector<std::string> ad{"FOO=BAR"s, "PROCEDURE=109"s};
        DbusPropertyMap testProperties{
            {"Message"s, Value{"xyz.openbmc_project.Error.Test5"s}},
            {"AdditionalData"s, ad}};

        auto values = policy::find(policy, testProperties);
        ASSERT_EQ(std::get<policy::EIDField>(values), "FFFFFFFF");
        ASSERT_EQ(std::get<policy::MsgField>(values), "Error FFFFFFFF");
    }

    // Use RAIL_NAME from the AdditionalData property
    {
        std::vector<std::string> ad{"FOO=BAR"s, "RAIL_NAME=RAIL_5"s};
        DbusPropertyMap testProperties{
            {"Message"s, Value{"xyz.openbmc_project.Error.Test6"s}},
            {"AdditionalData"s, ad}};

        auto values = policy::find(policy, testProperties);
        ASSERT_EQ(std::get<policy::EIDField>(values), "GGGGGGGG");
        ASSERT_EQ(std::get<policy::MsgField>(values), "Error GGGGGGGG");
    }

    // Use INPUT_NAME from the AdditionalData property
    {
        std::vector<std::string> ad{"FOO=BAR"s, "INPUT_NAME=INPUT_42"s};
        DbusPropertyMap testProperties{
            {"Message"s, Value{"xyz.openbmc_project.Error.Test7"s}},
            {"AdditionalData"s, ad}};

        auto values = policy::find(policy, testProperties);
        ASSERT_EQ(std::get<policy::EIDField>(values), "HHHHHHHH");
        ASSERT_EQ(std::get<policy::MsgField>(values), "Error HHHHHHHH");
    }

    // Test not finding an entry.
    {
        DbusPropertyMap testProperties{{"Message"s, Value{"hello world"s}}};

        auto values = policy::find(policy, testProperties);
        ASSERT_EQ(std::get<policy::EIDField>(values), policy.defaultEID());
        ASSERT_EQ(std::get<policy::MsgField>(values), policy.defaultMsg());
    }

    // Test that strange AdditionalData values don't break anything
    {
        std::vector<std::string> ad{"FOO"s, "INPUT_NAME="s};
        DbusPropertyMap testProperties{
            {"Message"s, Value{"xyz.openbmc_project.Error.Test7"s}},
            {"AdditionalData"s, ad}};

        auto values = policy::find(policy, testProperties);
        ASSERT_EQ(std::get<policy::EIDField>(values), policy.defaultEID());
        ASSERT_EQ(std::get<policy::MsgField>(values), policy.defaultMsg());
    }

    // Test a device path modifier match
    {
        std::vector<std::string> ad{"CALLOUT_DEVICE_PATH=/match/this/path"s};
        DbusPropertyMap testProperties{
            {"Message"s, Value{"xyz.openbmc_project.Error.Test8"s}},
            {"AdditionalData"s, ad}};

        auto values = policy::find(policy, testProperties);
        ASSERT_EQ(std::get<policy::EIDField>(values), "IIIIIII");
        ASSERT_EQ(std::get<policy::MsgField>(values), "Error IIIIIII");
    }

    // Test a predictive SEL matches on 'callout||Warning'
    {
        std::vector<std::string> ad{eSELBase + SEV_PREDICTIVE,
                                    "CALLOUT_INVENTORY_PATH=/inventory/core0"s};
        DbusPropertyMap testProperties{
            {"Message"s, Value{"org.open_power.Host.Error.Event"s}},
            {"AdditionalData"s, ad}};

        auto values = policy::find(policy, testProperties);
        ASSERT_EQ(std::get<policy::EIDField>(values), "JJJJJJJJ");
        ASSERT_EQ(std::get<policy::MsgField>(values), "Error JJJJJJJJ");
    }

    // Test a recovered SEL matches on 'callout||Informational'
    {
        std::vector<std::string> ad{eSELBase + SEV_RECOVERED,
                                    "CALLOUT_INVENTORY_PATH=/inventory/core1"s};
        DbusPropertyMap testProperties{
            {"Message"s, Value{"org.open_power.Host.Error.Event"s}},
            {"AdditionalData"s, ad}};

        auto values = policy::find(policy, testProperties);
        ASSERT_EQ(std::get<policy::EIDField>(values), "KKKKKKKK");
        ASSERT_EQ(std::get<policy::MsgField>(values), "Error KKKKKKKK");
    }

    // Test a critical severity matches on 'callout||Critical'
    {
        std::vector<std::string> ad{eSELBase + SEV_CRITICAL,
                                    "CALLOUT_INVENTORY_PATH=/inventory/core2"s};
        DbusPropertyMap testProperties{
            {"Message"s, Value{"org.open_power.Host.Error.Event"s}},
            {"AdditionalData"s, ad}};

        auto values = policy::find(policy, testProperties);
        ASSERT_EQ(std::get<policy::EIDField>(values), "LLLLLLLL");
        ASSERT_EQ(std::get<policy::MsgField>(values), "Error LLLLLLLL");
    }

    // Test an unrecoverable SEL matches on 'callout||Critical'
    {
        std::vector<std::string> ad{eSELBase + SEV_UNRECOV,
                                    "CALLOUT_INVENTORY_PATH=/inventory/core3"s};
        DbusPropertyMap testProperties{
            {"Message"s, Value{"org.open_power.Host.Error.Event"s}},
            {"AdditionalData"s, ad}};

        auto values = policy::find(policy, testProperties);
        ASSERT_EQ(std::get<policy::EIDField>(values), "MMMMMMMM");
        ASSERT_EQ(std::get<policy::MsgField>(values), "Error MMMMMMMM");
    }

    // Test a Diagnostic SEL matches on 'callout||Critical'
    {
        std::vector<std::string> ad{eSELBase + SEV_DIAG,
                                    "CALLOUT_INVENTORY_PATH=/inventory/core4"s};
        DbusPropertyMap testProperties{
            {"Message"s, Value{"org.open_power.Host.Error.Event"s}},
            {"AdditionalData"s, ad}};

        auto values = policy::find(policy, testProperties);
        ASSERT_EQ(std::get<policy::EIDField>(values), "NNNNNNNN");
        ASSERT_EQ(std::get<policy::MsgField>(values), "Error NNNNNNNN");
    }

    // Test a short eSEL still matches the normal callout
    {
        std::vector<std::string> ad{eSELBase,
                                    "CALLOUT_INVENTORY_PATH=/inventory/core5"s};
        DbusPropertyMap testProperties{
            {"Message"s, Value{"org.open_power.Host.Error.Event"s}},
            {"AdditionalData"s, ad}};

        auto values = policy::find(policy, testProperties);
        ASSERT_EQ(std::get<policy::EIDField>(values), "OOOOOOOO");
        ASSERT_EQ(std::get<policy::MsgField>(values), "Error OOOOOOOO");
    }

    // Test an eSEL with no UH section still matches a normal callout
    {
        std::vector<std::string> ad{noUHeSEL,
                                    "CALLOUT_INVENTORY_PATH=/inventory/core5"s};
        DbusPropertyMap testProperties{
            {"Message"s, Value{"org.open_power.Host.Error.Event"s}},
            {"AdditionalData"s, ad}};

        auto values = policy::find(policy, testProperties);
        ASSERT_EQ(std::get<policy::EIDField>(values), "OOOOOOOO");
        ASSERT_EQ(std::get<policy::MsgField>(values), "Error OOOOOOOO");
    }

    // Test a bad severity is still considered critical (by design)
    {
        std::vector<std::string> ad{eSELBase + "ZZ",
                                    "CALLOUT_INVENTORY_PATH=/inventory/core5"s};
        DbusPropertyMap testProperties{
            {"Message"s, Value{"org.open_power.Host.Error.Event"s}},
            {"AdditionalData"s, ad}};

        auto values = policy::find(policy, testProperties);
        ASSERT_EQ(std::get<policy::EIDField>(values), "PPPPPPPP");
        ASSERT_EQ(std::get<policy::MsgField>(values), "Error PPPPPPPP");
    }
}
