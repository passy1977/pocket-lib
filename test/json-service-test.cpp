/***************************************************************************
 *
 * Pocket
 * Copyright (C) 2018/2025 Antonio Salsi <passy.linux@zresa.it>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 ***************************************************************************/

#include <gtest/gtest.h>
#include "pocket-services/json.hpp"
#include "pocket-pods/device.hpp"
#include "pocket-pods/user.hpp"
#include "pocket-pods/group.hpp"
#include "pocket-pods/group-field.hpp"
#include "pocket-pods/field.hpp"
#include "pocket-pods/helpers.hpp"
#include "BS_thread_pool.hpp"
#include <nlohmann/json.hpp>

using namespace pocket::services;
using namespace pocket::pods;
using json = nlohmann::json;

class JsonServiceTest : public ::testing::Test 
{
protected:
    void SetUp() override {}
    void TearDown() override {}
    
    json create_device_json() {
        return json{
            {"id", 1},
            {"uuid", "test-uuid-123"},
            {"status", "ACTIVE"},
            {"timestampCreation", 1234567890},
            {"userId", 42}
        };
    }
    
    json create_user_json() {
        return json{
            {"id", 1},
            {"name", "Test User"},
            {"email", "test@example.com"},
            {"status", "ACTIVE"}  // Should be string, not number
        };
    }
    
    json create_net_helper_json() {
        return json{
            {"timestampLastUpdate", 1234567890},
            {"user", create_user_json()},
            {"device", create_device_json()},
            {"groups", json::array()},
            {"groupFields", json::array()},
            {"fields", json::array()}
        };
    }
};

// Test device JSON conversion
TEST_F(JsonServiceTest, DeviceFromJsonBasic)
{
    json device_json = create_device_json();
    device dev = json_to_device(device_json);
    
    EXPECT_EQ(dev.id, 1);
    EXPECT_EQ(dev.uuid, "test-uuid-123");
    EXPECT_EQ(dev.user_id, 42);
    EXPECT_EQ(dev.timestamp_creation, 1234567890);
}

TEST_F(JsonServiceTest, DeviceFromJsonStringView)
{
    json device_json = create_device_json();
    std::string json_str = device_json.dump();
    std::string_view json_view(json_str);
    
    device dev = json_to_device(json_view);
    
    EXPECT_EQ(dev.id, 1);
    EXPECT_EQ(dev.uuid, "test-uuid-123");
    EXPECT_EQ(dev.user_id, 42);
}

// Test user JSON conversion  
TEST_F(JsonServiceTest, UserFromJsonBasic)
{
    json user_json = create_user_json();
    user usr = json_to_user(user_json);
    
    EXPECT_EQ(usr.id, 1);
    EXPECT_EQ(usr.name, "Test User");
    EXPECT_EQ(usr.email, "test@example.com");
    EXPECT_EQ(static_cast<int>(usr.status), static_cast<int>(user::stat::ACTIVE));
    // Note: passwd and timestamp_last_update are not parsed by json_to_user
}

// Test AES IV extraction
TEST_F(JsonServiceTest, AesIvExtraction)
{
    json config_json = json{
        {"aesCbcIv", "test_iv_value_123"}  // Correct field name
    };
    
    std::string json_str = config_json.dump();
    std::string iv = json_to_aes_cbc_iv(json_str);
    
    EXPECT_EQ(iv, "test_iv_value_123");
}

// Test net_helper parsing
TEST_F(JsonServiceTest, NetHelperParsingValid)
{
    BS::thread_pool<> pool(2);
    net_helper helper;
    json valid_json = create_net_helper_json();
    std::string json_str = valid_json.dump();
    
    EXPECT_NO_THROW(json_parse_net_helper(pool, json_str, helper));
}

TEST_F(JsonServiceTest, NetHelperParsingEmpty)
{
    BS::thread_pool<> pool(2);
    net_helper helper;
    
    EXPECT_THROW(json_parse_net_helper(pool, "", helper), std::runtime_error);
}

TEST_F(JsonServiceTest, NetHelperParsingMissingTimestamp)
{
    BS::thread_pool<> pool(2);
    net_helper helper;
    json incomplete_json = create_net_helper_json();
    incomplete_json.erase("timestampLastUpdate");
    std::string json_str = incomplete_json.dump();
    
    EXPECT_THROW(json_parse_net_helper(pool, json_str, helper), std::runtime_error);
}

// Test group serialization - simplified version
TEST_F(JsonServiceTest, GroupSerialization)
{
    auto group_ptr = std::make_unique<group>();
    group_ptr->id = 1;
    group_ptr->title = "Test Group";
    
    // Test that serialization doesn't crash
    EXPECT_NO_THROW(serialize_json(group_ptr));
}

// Test error handling
TEST_F(JsonServiceTest, EmptyJsonHandling)
{
    json empty_json = json::object();
    
    // These functions should throw exceptions for missing required fields
    EXPECT_THROW({
        device dev = json_to_device(empty_json);
    }, std::runtime_error);
    
    EXPECT_THROW({
        user usr = json_to_user(empty_json);
    }, std::runtime_error);
}