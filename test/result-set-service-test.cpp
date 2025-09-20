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
#include "pocket-services/database.hpp"
#include "pocket-services/result-set.hpp"
#include "pocket-pods/variant.hpp"
#include <filesystem>

using namespace pocket::services;
using namespace pocket::pods;

class ResultSetServiceTest : public ::testing::Test 
{
protected:
    std::string test_db_path;
    std::unique_ptr<database> db;

    void SetUp() override 
    {
        test_db_path = "/tmp/test_resultset_" + std::to_string(time(nullptr)) + ".db";
        db = std::make_unique<database>();
        ASSERT_TRUE(db->open(test_db_path));
        
        // Insert test data
        database::parameters params = {
            variant("Test User 1"),
            variant("user1@example.com"),
            variant("password_hash1"),
            variant(1),
            variant(static_cast<int64_t>(1234567890))
        };
        
        auto result1 = db->execute(
            "INSERT INTO user (name, email, passwd, status, timestamp_last_update) VALUES (?, ?, ?, ?, ?)",
            params
        );
        ASSERT_TRUE(result1.has_value());
        
        params = {
            variant("Test User 2"),
            variant("user2@example.com"),
            variant("password_hash2"),
            variant(0),
            variant(static_cast<int64_t>(1234567891))
        };
        
        auto result2 = db->execute(
            "INSERT INTO user (name, email, passwd, status, timestamp_last_update) VALUES (?, ?, ?, ?, ?)",
            params
        );
        ASSERT_TRUE(result2.has_value());
    }

    void TearDown() override 
    {
        if (db) {
            db->close();
        }
        if (std::filesystem::exists(test_db_path)) {
            std::filesystem::remove(test_db_path);
        }
    }
};

// Test basic result set functionality
TEST_F(ResultSetServiceTest, BasicResultIteration)
{
    auto result_opt = db->execute("SELECT * FROM user ORDER BY id");
    ASSERT_TRUE(result_opt.has_value());
    
    auto& result = *result_opt.value();
    EXPECT_EQ(result.size(), 2);
    
    int count = 0;
    for (const auto& row : result) {
        EXPECT_FALSE(row.empty());
        count++;
    }
    EXPECT_EQ(count, 2);
}

TEST_F(ResultSetServiceTest, ArrayAccessOperator)
{
    auto result_opt = db->execute("SELECT name, email FROM user ORDER BY id");
    ASSERT_TRUE(result_opt.has_value());
    
    auto& result = *result_opt.value();
    ASSERT_EQ(result.size(), 2);
    
    auto& first_row = result[0];
    auto& second_row = result[1];
    
    EXPECT_EQ(first_row.find("name")->second.to_text(), "Test User 1");
    EXPECT_EQ(first_row.find("email")->second.to_text(), "user1@example.com");
    
    EXPECT_EQ(second_row.find("name")->second.to_text(), "Test User 2");
    EXPECT_EQ(second_row.find("email")->second.to_text(), "user2@example.com");
}

TEST_F(ResultSetServiceTest, EmptyResultSet)
{
    auto result_opt = db->execute("SELECT * FROM user WHERE email = 'nonexistent@example.com'");
    ASSERT_TRUE(result_opt.has_value());
    
    auto& result = *result_opt.value();
    EXPECT_EQ(result.size(), 0);
    EXPECT_TRUE(result.empty());
    EXPECT_EQ(result.begin(), result.end());
}

TEST_F(ResultSetServiceTest, DataTypeAccess)
{
    auto result_opt = db->execute("SELECT id, name, status, timestamp_last_update FROM user WHERE email = 'user1@example.com'");
    ASSERT_TRUE(result_opt.has_value());
    
    auto& result = *result_opt.value();
    ASSERT_EQ(result.size(), 1);
    
    auto& row = result[0];
    
    auto id_it = row.find("id");
    ASSERT_NE(id_it, row.end());
    EXPECT_GT(id_it->second.to_integer(), 0);
    
    auto name_it = row.find("name");
    ASSERT_NE(name_it, row.end());
    EXPECT_EQ(name_it->second.to_text(), "Test User 1");
    
    auto status_it = row.find("status");
    ASSERT_NE(status_it, row.end());
    EXPECT_EQ(status_it->second.to_integer(), 1);
    
    auto timestamp_it = row.find("timestamp_last_update");
    ASSERT_NE(timestamp_it, row.end());
    EXPECT_EQ(timestamp_it->second.to_integer(), 1234567890);
}

TEST_F(ResultSetServiceTest, ParameterizedQuery)
{
    database::parameters params = {variant("user1@example.com")};
    
    auto result_opt = db->execute("SELECT name FROM user WHERE email = ?", params);
    ASSERT_TRUE(result_opt.has_value());
    
    auto& result = *result_opt.value();
    EXPECT_EQ(result.size(), 1);
    
    auto& row = result[0];
    auto name_it = row.find("name");
    ASSERT_NE(name_it, row.end());
    EXPECT_EQ(name_it->second.to_text(), "Test User 1");
}

TEST_F(ResultSetServiceTest, SuccessfulStatement)
{
    auto result_opt = db->execute("SELECT * FROM user");
    ASSERT_TRUE(result_opt.has_value());
    
    auto& result = *result_opt.value();
    EXPECT_EQ(result.get_statement_stat(), SQLITE_OK);
}

TEST_F(ResultSetServiceTest, InvalidStatement)
{
    // Invalid SQL should throw an exception
    EXPECT_THROW({
        auto result_opt = db->execute("INVALID SQL QUERY");
    }, std::runtime_error);
}