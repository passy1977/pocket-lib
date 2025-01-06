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

#include "pocket-controllers/config.hpp"
using namespace pocket::controllers;

constexpr char data[] = R"json({
{
    "uuid": "de10b648-9e9c-4b63-96df-2aa662fe20ba",
    "user_uuid": "1281d20e-4e6f-43d8-9aa5-099039f36206",
    "host": "127.0.0.1",
    "host_pub_key": "fookey"

})json";

struct session_test : public ::testing::Test {};

TEST_F(session_test, config)
{
    config config;
    try
    {
        config.parse(data);
    }
    catch (const std::exception& e)
    {
        std::cerr << e.what() << std::endl;
    }




}
