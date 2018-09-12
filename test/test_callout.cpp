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
#include "callout.hpp"
#include "dbus.hpp"

#include <experimental/filesystem>
#include <fstream>

#include <gtest/gtest.h>

using namespace ibm::logging;

class CalloutTest : public ::testing::Test
{
  protected:
    virtual void SetUp()
    {
        char dir[] = {"./calloutsXXXXXX"};

        persistDir = mkdtemp(dir);
    }

    virtual void TearDown()
    {
        fs::remove_all(persistDir);
    }

    fs::path persistDir;
};

TEST_F(CalloutTest, TestPersist)
{
    using namespace std::literals::string_literals;

    auto bus = sdbusplus::bus::new_default();
    std::string objectPath{"/callout/path/0"};
    std::string calloutPath{"/some/inventory/object"};
    size_t id = 0;
    uint64_t ts = 5;

    DbusPropertyMap assetProps{{"BuildDate"s, Value{"Date42"s}},
                               {"Manufacturer"s, Value{"Mfg42"s}},
                               {"Model"s, Value{"Model42"s}},
                               {"PartNumber"s, Value{"PN42"s}},
                               {"SerialNumber"s, Value{"SN42"s}}};
    {
        auto callout = std::make_unique<Callout>(bus, objectPath, calloutPath,
                                                 id, ts, assetProps);
        callout->serialize(persistDir);

        ASSERT_EQ(fs::exists(persistDir / std::to_string(id)), true);
    }

    // Test object restoration
    {
        auto callout = std::make_unique<Callout>(bus, objectPath, id, ts);

        ASSERT_EQ(callout->deserialize(persistDir), true);

        ASSERT_EQ(callout->id(), id);
        ASSERT_EQ(callout->ts(), ts);
        ASSERT_EQ(callout->path(), calloutPath);
        ASSERT_EQ(callout->buildDate(),
                  assetProps["BuildDate"].get<std::string>());
        ASSERT_EQ(callout->manufacturer(),
                  assetProps["Manufacturer"].get<std::string>());
        ASSERT_EQ(callout->model(), assetProps["Model"].get<std::string>());
        ASSERT_EQ(callout->partNumber(),
                  assetProps["PartNumber"].get<std::string>());
        ASSERT_EQ(callout->serialNumber(),
                  assetProps["SerialNumber"].get<std::string>());
    }

    // Test a serialization failure due to a bad timestamp
    {
        auto callout = std::make_unique<Callout>(bus, objectPath, id, ts + 1);

        ASSERT_EQ(callout->deserialize(persistDir), false);
        ASSERT_EQ(fs::exists(persistDir / std::to_string(id)), false);
    }
}
