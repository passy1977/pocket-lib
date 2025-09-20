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
#include <memory>
#include <cmath>
#include <limits>

#include "pocket-pods/variant.hpp"

using namespace pocket::pods;

class VariantTest : public ::testing::Test 
{
protected:
    void SetUp() override {}
    void TearDown() override {}
};

TEST_F(VariantTest, DefaultConstructor) 
{
    variant v;
    EXPECT_EQ(v.get_type(), variant::type::NULL_T);
    EXPECT_EQ(v.to_integer(), 0);
    EXPECT_EQ(v.to_float(), 0.0);
    EXPECT_EQ(v.to_text(), "");
}

TEST_F(VariantTest, NullptrConstructor) 
{
    variant v(nullptr);
    EXPECT_EQ(v.get_type(), variant::type::NULL_T);
    EXPECT_EQ(v.to_integer(), 0);
    EXPECT_EQ(v.to_float(), 0.0);
    EXPECT_EQ(v.to_text(), "");
}

TEST_F(VariantTest, Int32Constructor) 
{
    variant v(42);
    EXPECT_EQ(v.get_type(), variant::type::INT);
    EXPECT_EQ(v.to_integer(), 42);
    EXPECT_EQ(v.to_text(), "42");
}

TEST_F(VariantTest, UInt32Constructor) 
{
    variant v(static_cast<uint32_t>(42));
    EXPECT_EQ(v.get_type(), variant::type::INT);
    EXPECT_EQ(v.to_integer(), 42);
    EXPECT_EQ(v.to_text(), "42");
}

TEST_F(VariantTest, Int64Constructor) 
{
    variant v(static_cast<int64_t>(9223372036854775807LL)); // Max int64_t
    EXPECT_EQ(v.get_type(), variant::type::INT64);
    EXPECT_EQ(v.to_integer(), 9223372036854775807LL);
    EXPECT_EQ(v.to_text(), "9223372036854775807");
}

TEST_F(VariantTest, UInt64Constructor) 
{
    variant v(static_cast<uint64_t>(1234567890123456789ULL));
    EXPECT_EQ(v.get_type(), variant::type::INT64);
    EXPECT_EQ(v.to_integer(), static_cast<int64_t>(1234567890123456789ULL));
}

TEST_F(VariantTest, DoubleConstructor) 
{
    variant v(3.14159);
    EXPECT_EQ(v.get_type(), variant::type::DOUBLE);
    EXPECT_DOUBLE_EQ(v.to_float(), 3.14159);
    EXPECT_EQ(v.to_text(), std::to_string(3.14159));
}

TEST_F(VariantTest, StringConstructorCopy) 
{
    std::string test_string = "Hello, World!";
    variant v(test_string);
    EXPECT_EQ(v.get_type(), variant::type::TEXT);
    EXPECT_EQ(v.to_text(), "Hello, World!");
}

TEST_F(VariantTest, StringConstructorMove) 
{
    variant v(std::string("Hello, World!"));
    EXPECT_EQ(v.get_type(), variant::type::TEXT);
    EXPECT_EQ(v.to_text(), "Hello, World!");
}

TEST_F(VariantTest, EmptyStringConstructor) 
{
    variant v(std::string(""));
    EXPECT_EQ(v.get_type(), variant::type::TEXT);
    EXPECT_EQ(v.to_text(), "");
}

TEST_F(VariantTest, NegativeIntegerValues) 
{
    variant v_int(-42);
    EXPECT_EQ(v_int.get_type(), variant::type::INT);
    EXPECT_EQ(v_int.to_integer(), -42);
    EXPECT_EQ(v_int.to_text(), "-42");
    
    variant v_int64(static_cast<int64_t>(-9223372036854775807LL));
    EXPECT_EQ(v_int64.get_type(), variant::type::INT64);
    EXPECT_EQ(v_int64.to_integer(), -9223372036854775807LL);
}

TEST_F(VariantTest, NegativeDoubleValues) 
{
    variant v(-3.14159);
    EXPECT_EQ(v.get_type(), variant::type::DOUBLE);
    EXPECT_DOUBLE_EQ(v.to_float(), -3.14159);
    EXPECT_EQ(v.to_text(), std::to_string(-3.14159));
}

TEST_F(VariantTest, ZeroValues) 
{
    variant v_int(0);
    EXPECT_EQ(v_int.get_type(), variant::type::INT);
    EXPECT_EQ(v_int.to_integer(), 0);
    EXPECT_EQ(v_int.to_text(), "0");
    
    variant v_double(0.0);
    EXPECT_EQ(v_double.get_type(), variant::type::DOUBLE);
    EXPECT_DOUBLE_EQ(v_double.to_float(), 0.0);
    
    variant v_int64(static_cast<int64_t>(0));
    EXPECT_EQ(v_int64.get_type(), variant::type::INT64);
    EXPECT_EQ(v_int64.to_integer(), 0);
}

TEST_F(VariantTest, StringWithNumbers) 
{
    variant v("12345");
    EXPECT_EQ(v.get_type(), variant::type::TEXT);
    EXPECT_EQ(v.to_text(), "12345");
    // When stored as text, to_integer() should return the stored integer_value (likely 0)
    EXPECT_EQ(v.to_integer(), 0);
}

TEST_F(VariantTest, SpecialDoubleValues) 
{
    // Test with infinity
    variant v_inf(std::numeric_limits<double>::infinity());
    EXPECT_EQ(v_inf.get_type(), variant::type::DOUBLE);
    EXPECT_TRUE(std::isinf(v_inf.to_float()));
    
    // Test with NaN
    variant v_nan(std::numeric_limits<double>::quiet_NaN());
    EXPECT_EQ(v_nan.get_type(), variant::type::DOUBLE);
    EXPECT_TRUE(std::isnan(v_nan.to_float()));
}

TEST_F(VariantTest, LargeStrings) 
{
    std::string large_string(10000, 'A');
    variant v(large_string);
    EXPECT_EQ(v.get_type(), variant::type::TEXT);
    EXPECT_EQ(v.to_text(), large_string);
    EXPECT_EQ(v.to_text().length(), 10000);
}

TEST_F(VariantTest, SpecialCharacters) 
{
    std::string special = "Special chars: àáâãäåæçèéêë 中文 🚀";
    variant v(special);
    EXPECT_EQ(v.get_type(), variant::type::TEXT);
    EXPECT_EQ(v.to_text(), special);
}

TEST_F(VariantTest, TypeConversions) 
{
    // Test that integer types convert properly to text
    variant v_int(123);
    EXPECT_EQ(v_int.to_text(), "123");
    
    variant v_int64(static_cast<int64_t>(123456789012345LL));
    EXPECT_EQ(v_int64.to_text(), "123456789012345");
    
    variant v_double(123.456);
    EXPECT_EQ(v_double.to_text(), std::to_string(123.456));
    
    // Test that text types return 0 for numeric conversions
    variant v_text("Hello");
    EXPECT_EQ(v_text.to_integer(), 0);
    EXPECT_EQ(v_text.to_float(), 0.0);
}

TEST_F(VariantTest, BoundaryValues) 
{
    // Test maximum values
    variant v_max_int(std::numeric_limits<int32_t>::max());
    EXPECT_EQ(v_max_int.get_type(), variant::type::INT);
    EXPECT_EQ(v_max_int.to_integer(), std::numeric_limits<int32_t>::max());
    
    variant v_min_int(std::numeric_limits<int32_t>::min());
    EXPECT_EQ(v_min_int.get_type(), variant::type::INT);
    EXPECT_EQ(v_min_int.to_integer(), std::numeric_limits<int32_t>::min());
    
    variant v_max_int64(std::numeric_limits<int64_t>::max());
    EXPECT_EQ(v_max_int64.get_type(), variant::type::INT64);
    EXPECT_EQ(v_max_int64.to_integer(), std::numeric_limits<int64_t>::max());
}