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
#include <thread>
#include <atomic>

using namespace pocket::services;
using namespace pocket::pods;

class DatabaseServiceTest : public ::testing::Test 
{
protected:
    std::string test_db_path;
    std::unique_ptr<database> db;

    void SetUp() override 
    {
        test_db_path = "/tmp/test_database_" + std::to_string(time(nullptr)) + ".db";
        db = std::make_unique<database>();
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

// Test database creation and opening
TEST_F(DatabaseServiceTest, OpenNewDatabase)
{
    EXPECT_TRUE(db->open(test_db_path));
    EXPECT_TRUE(std::filesystem::exists(test_db_path));
    
    auto result = db->execute("SELECT name FROM sqlite_master WHERE type='table'");
    ASSERT_TRUE(result.has_value());
    EXPECT_GT(result.value()->size(), 0);
}

TEST_F(DatabaseServiceTest, OpenExistingDatabase)
{
    EXPECT_TRUE(db->open(test_db_path));
    db->close();
    EXPECT_TRUE(db->open(test_db_path));
}

TEST_F(DatabaseServiceTest, CloseDatabase)
{
    EXPECT_TRUE(db->open(test_db_path));
    db->close();
    db->close(); // Multiple closes should not cause problems
}

// Test basic SQL execution
TEST_F(DatabaseServiceTest, ExecuteSelect)
{
    ASSERT_TRUE(db->open(test_db_path));
    
    auto result = db->execute("SELECT COUNT(*) as count FROM metadata");
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result.value()->size(), 1);
    
    auto count_it = result.value()->at(0).find("count");
    ASSERT_NE(count_it, result.value()->at(0).end());
    EXPECT_GT(count_it->second.to_integer(), 0);
}

TEST_F(DatabaseServiceTest, ExecuteInsert)
{
    ASSERT_TRUE(db->open(test_db_path));
    
    database::parameters params = {
        variant("Test User"),
        variant("test@example.com"),
        variant("password_hash"),
        variant(1),
        variant(static_cast<int64_t>(1234567890))
    };
    
    auto result = db->execute(
        "INSERT INTO user (name, email, passwd, status, timestamp_last_update) VALUES (?, ?, ?, ?, ?)",
        params
    );
    
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result.value()->size(), 0); // INSERT returns no rows
    
    // Verify insertion
    auto select_result = db->execute("SELECT * FROM user WHERE email = 'test@example.com'");
    ASSERT_TRUE(select_result.has_value());
    EXPECT_EQ(select_result.value()->size(), 1);
    
    auto& row = select_result.value()->at(0);
    auto name_it = row.find("name");
    ASSERT_NE(name_it, row.end());
    EXPECT_EQ(name_it->second.to_text(), "Test User");
}

TEST_F(DatabaseServiceTest, ParameterBindingTypes)
{
    ASSERT_TRUE(db->open(test_db_path));
    
    database::parameters params = {
        variant("String Value"),
        variant(42),
        variant(static_cast<int64_t>(1234567890)),
        variant(3.14)
    };
    
    auto create_result = db->execute(
        "CREATE TABLE test_types (str_col TEXT, int_col INTEGER, bigint_col INTEGER, real_col REAL)"
    );
    ASSERT_TRUE(create_result.has_value());
    
    auto insert_result = db->execute(
        "INSERT INTO test_types VALUES (?, ?, ?, ?)",
        params
    );
    ASSERT_TRUE(insert_result.has_value());
    
    auto select_result = db->execute("SELECT * FROM test_types");
    ASSERT_TRUE(select_result.has_value());
    EXPECT_EQ(select_result.value()->size(), 1);
    
    auto& row = select_result.value()->at(0);
    EXPECT_EQ(row.find("str_col")->second.to_text(), "String Value");
    EXPECT_EQ(row.find("int_col")->second.to_integer(), 42);
    EXPECT_EQ(row.find("bigint_col")->second.to_integer(), 1234567890);
    EXPECT_NEAR(row.find("real_col")->second.to_float(), 3.14, 0.001);
}

TEST_F(DatabaseServiceTest, InvalidSQL)
{
    ASSERT_TRUE(db->open(test_db_path));
    
    // The database should throw an exception for invalid SQL
    EXPECT_THROW({
        auto result = db->execute("INVALID SQL STATEMENT");
    }, std::runtime_error);
}

TEST_F(DatabaseServiceTest, TransactionCommit)
{
    ASSERT_TRUE(db->open(test_db_path));
    
    auto begin_result = db->execute("BEGIN TRANSACTION");
    ASSERT_TRUE(begin_result.has_value());
    
    database::parameters params = {
        variant("Test User"),
        variant("test@example.com"),
        variant("password_hash"),
        variant(1),
        variant(static_cast<int64_t>(1234567890))
    };
    
    auto insert_result = db->execute(
        "INSERT INTO user (name, email, passwd, status, timestamp_last_update) VALUES (?, ?, ?, ?, ?)",
        params
    );
    ASSERT_TRUE(insert_result.has_value());
    
    auto commit_result = db->execute("COMMIT");
    ASSERT_TRUE(commit_result.has_value());
    
    auto select_result = db->execute("SELECT * FROM user WHERE email = 'test@example.com'");
    ASSERT_TRUE(select_result.has_value());
    EXPECT_EQ(select_result.value()->size(), 1);
}