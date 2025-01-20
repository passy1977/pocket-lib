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

#include "pocket-controllers/session.hpp"
using namespace pocket::controllers;

constexpr char data[] = R"json(
{
  "id": 53,
  "uuid": "40549568-c2ea-43ed-b8c3-b77880b4ac57",
  "status": "ACTIVE",
  "timestampLastUpdate": 1737393644,
  "timestampLastLogin": 1737393644,
  "timestampCreation": 1737393648,
  "userId": 3,
  "host": "http://127.0.0.1:8181",
  "hostPublicKey": "MIIBIjANBgkqhkiG9w0BAQEFAAOCAQ8AMIIBCgKCAQEAxB4AYB/q/39uNCUDYIHfdOjp8DbIy2i+3I58Wzv2FGtXsWQRy0iqQYXq5QL5HfhIu0KwMkwFtR6pjfc4V+7+9ANgk83cz7dHKBp0mTuz3ST2fRhx6M8fH2vyIjACBrVITV3hHI4WGb1NSnlqERQDQYviEaj+jLr0tFPZ9l+hxf7JcXTZImGvMxh4g5rdPpqxNPJUhq9+v7d3q45uXZC5w6BOb65CqeT+GsVmgO21nMBTapxcDD3ZyORHQlcZJWCBrRMZdTLiNzZgMP+Cj4ah2Y6o8TqpUTLFowpvewk+9SXOefcH69CR482DJM5d+ad7zlPE7xxFFwxNJ1zDParvqwIDAQAB"
}
)json";

struct session_test : public ::testing::Test {};

TEST_F(session_test, config_parse) try
{
    config config;

    config.parse(data);

}
catch (const std::exception& e)
{
    std::cerr << e.what() << std::endl;
    ASSERT_TRUE(false);
}

TEST_F(session_test, session_init) try
{
    session session(data);

    session.init();

    session.login("passy.linux@zresa.it", "pwd");

}
catch (const std::exception& e)
{
    std::cerr << e.what() << std::endl;
    ASSERT_TRUE(false);
}