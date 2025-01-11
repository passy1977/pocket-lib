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
#include <iostream>

#include "pocket-controllers/session.hpp"

struct thread_pool_test : public ::testing::Test {};


TEST_F(thread_pool_test, coroutine_test) try
{


}
catch (const std::exception& e)
{
    std::cerr << e.what() << std::endl;
    ASSERT_TRUE(false);
}

TEST_F(thread_pool_test, session_init) try
{

}
catch (const std::exception& e)
{
    std::cerr << e.what() << std::endl;
    ASSERT_TRUE(false);
}