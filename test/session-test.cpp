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
  "id": 54,
  "uuid": "3421c14a-4846-42b6-b8e1-d551f52b8329",
  "status": "ACTIVE",
  "timestampCreation": 1738099361,
  "userId": 3,
  "host": "http://127.0.0.1:8081",
  "hostPublicKey": "MIIBIjANBgkqhkiG9w0BAQEFAAOCAQ8AMIIBCgKCAQEAs4kRD+4/gaut/fm6ijRLW3mIDFwKDsEIQyhJcCJTx8ARIzJ/JVkIpXg7pVoZmfAA1HqjwBQN7p7/8h41lF5caIR3dqcBkWL6adUSG+GVr+DmaA+QvkgTXm2f4dOaqzLqw3hnT+A8kNVa3npI8LFmA+RopWx9G79J2cL8KAX81cVJOp5pCMTmN8zHfdHObXRQITQ4zv93Fyi1ZZ6/gP/fzJJDq4FWgHemDFA1rAKASmtUVZ4f9YX4Zsonrzsh07v6xKbIY+sZ9F1W1hcZceOlMbTlB9tF8hXbtzX7pUfjsd40tJHI8MeiuvBXXZlMF+sRdsS2FXqbBBspbK0Fk18juQIDAQAB"
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