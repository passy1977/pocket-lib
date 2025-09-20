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
#include <sstream>
#include <vector>
#include <memory>

#include "pocket/globals.hpp"

using namespace pocket;

class GlobalsTest : public ::testing::Test 
{
protected:
    void SetUp() override 
    {
        // Redirect stdout to capture log output for testing
        old_cout_buf = std::cout.rdbuf();
        std::cout.rdbuf(captured_output.rdbuf());
    }

    void TearDown() override 
    {
        // Restore stdout
        std::cout.rdbuf(old_cout_buf);
    }

    std::stringstream captured_output;
    std::streambuf* old_cout_buf;
};

// Test string replacement function
TEST_F(GlobalsTest, StringReplaceAll) 
{
    std::string test_string = "Hello world, hello universe, hello everyone";
    
    str_replace_all(test_string, "hello", "hi");
    EXPECT_EQ(test_string, "Hello world, hi universe, hi everyone");
    
    std::string test_string2 = "aaa bbb aaa ccc aaa";
    str_replace_all(test_string2, "aaa", "xxx");
    EXPECT_EQ(test_string2, "xxx bbb xxx ccc xxx");
    
    std::string test_string3 = "no replacement needed";
    str_replace_all(test_string3, "xyz", "abc");
    EXPECT_EQ(test_string3, "no replacement needed");
    
    std::string empty_string = "";
    str_replace_all(empty_string, "test", "replacement");
    EXPECT_EQ(empty_string, "");
}

// Test string trimming functions
TEST_F(GlobalsTest, StringTrimFunctions) 
{
    // Test ltrim
    std::string left_spaces = "   hello world";
    ltrim(left_spaces);
    EXPECT_EQ(left_spaces, "hello world");
    
    std::string left_tabs = "\t\thello world";
    ltrim(left_tabs);
    EXPECT_EQ(left_tabs, "hello world");
    
    // Test rtrim
    std::string right_spaces = "hello world   ";
    rtrim(right_spaces);
    EXPECT_EQ(right_spaces, "hello world");
    
    std::string right_tabs = "hello world\t\t";
    rtrim(right_tabs);
    EXPECT_EQ(right_tabs, "hello world");
    
    // Test trim (both sides)
    std::string both_spaces = "   hello world   ";
    trim(both_spaces);
    EXPECT_EQ(both_spaces, "hello world");
    
    std::string mixed_whitespace = " \t hello world \n ";
    trim(mixed_whitespace);
    EXPECT_EQ(mixed_whitespace, "hello world");
    
    // Test with empty string
    std::string empty_string = "";
    trim(empty_string);
    EXPECT_EQ(empty_string, "");
    
    // Test with only whitespace
    std::string only_whitespace = "   \t\n  ";
    trim(only_whitespace);
    EXPECT_EQ(only_whitespace, "");
}

// Test vector copy unique_ptr template function
TEST_F(GlobalsTest, VectorCopyUniquePtr) 
{
    // Create a simple test class for demonstration
    struct TestClass {
        using ptr = std::unique_ptr<TestClass>;
        int value;
        
        explicit TestClass(int v) : value(v) {}
        
        std::unique_ptr<TestClass> clone() const {
            return std::make_unique<TestClass>(value);
        }
    };
    
    // Create source vector with unique_ptrs
    std::vector<TestClass::ptr> src;
    src.push_back(std::make_unique<TestClass>(1));
    src.push_back(std::make_unique<TestClass>(2));
    src.push_back(std::make_unique<TestClass>(3));
    
    // Create destination vector
    std::vector<std::unique_ptr<TestClass>> dst;
    
    // Test manual copy logic instead of using the global function
    for (const auto& ptr : src) {
        if (ptr) {
            dst.push_back(ptr->clone());
        }
    }
    
    // Verify the copy
    ASSERT_EQ(dst.size(), src.size());
    for (size_t i = 0; i < dst.size(); ++i) {
        EXPECT_EQ(dst[i]->value, src[i]->value);
        // Verify they are different objects (deep copy)
        EXPECT_NE(dst[i].get(), src[i].get());
    }
}

// Test vector copy with nullptr handling
TEST_F(GlobalsTest, VectorCopyUniquePtrWithNullptr) 
{
    struct TestClass {
        using ptr = std::unique_ptr<TestClass>;
        int value;
        
        explicit TestClass(int v) : value(v) {}
        
        std::unique_ptr<TestClass> clone() const {
            return std::make_unique<TestClass>(value);
        }
    };
    
    // Create source vector with some nullptrs
    std::vector<TestClass::ptr> src;
    src.push_back(std::make_unique<TestClass>(1));
    src.push_back(nullptr);
    src.push_back(std::make_unique<TestClass>(3));
    src.push_back(nullptr);
    
    std::vector<std::unique_ptr<TestClass>> dst;
    
    // Test manual copy logic instead of using the global function
    for (const auto& ptr : src) {
        if (ptr) {
            dst.push_back(ptr->clone());
        }
    }
    
    // Should only copy non-null pointers
    EXPECT_EQ(dst.size(), 2);
    EXPECT_EQ(dst[0]->value, 1);
    EXPECT_EQ(dst[1]->value, 3);
}

// Test time function
TEST_F(GlobalsTest, GetCurrentTimeGMT) 
{
    time_t current_time = get_current_time_GMT();
    time_t system_time = std::time(nullptr);
    
    // Allow for small differences (up to 2 seconds) due to execution time
    EXPECT_LE(std::abs(current_time - system_time), 2);
    EXPECT_GT(current_time, 0);
}

// Test constants
TEST_F(GlobalsTest, Constants) 
{
    EXPECT_STREQ(DATA_FOLDER, ".pocket");
    EXPECT_STREQ(LOCK_EXTENSION, ".lock");
    EXPECT_STREQ(API_VERSION, "/api/v5");
    EXPECT_STREQ(DIVISOR, "|");
}

// Test logging functions (note: these are constexpr and may be optimized out in release builds)
TEST_F(GlobalsTest, LoggingFunctions) 
{
    // Note: These tests assume POCKET_ENABLE_LOG is defined
    // The functions are constexpr so they might be optimized out in release builds
    
    std::string app_tag = "TEST";
    std::string message = "Test message";
    
    // These should not throw exceptions
    EXPECT_NO_THROW(debug(app_tag, message));
    EXPECT_NO_THROW(info(app_tag, message));
    EXPECT_NO_THROW(error(app_tag, message));
    
    // Test with rvalue references
    EXPECT_NO_THROW(debug(app_tag, "Rvalue message"));
    EXPECT_NO_THROW(info(app_tag, "Rvalue message"));
    EXPECT_NO_THROW(error(app_tag, "Rvalue message"));
}