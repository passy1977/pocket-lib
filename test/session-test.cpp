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
  "id": 58,
  "uuid": "8ed5b7e7-2a14-4fe6-989a-4950af217770",
  "status": "ACTIVE",
  "timestampCreation": 1738271290,
  "userId": 3,
  "host": "http://127.0.0.1:8081",
  "hostPublicKey": "-----BEGIN PUBLIC KEY-----\nMIIBIjANBgkqhkiG9w0BAQEFAAOCAQ8AMIIBCgKCAQEAnIQU+mqVljqUpnxV22OyzKo8gEesdw/YTcTzVQHY+PbaZR38yHaDBTMRZ+zMK8BpzneyqsQyqCSk+s1Yy8q1DPHA48RpzItGAcINMbiDDOKqHkqc9nL0JB68rxd09v8TsZ605dW7AsuU5SePE+mFGserOPjE7HHof01oZ7a/GtSvrYmqEBoxIPSKZN6ZdvqU91pDvzry5bZswYbqRsemkjQaQIdEPG16J92FovcYhZa+N/vc+ebkGsWL+5qgZ1NFwDan8xRR/wSDV0/JlCzwl9GkT35gqKsEZXFTZ8Wy7uQMP+binkG0MCxtQ8lwJ8vCQ0gWb4JN5nilAfSMR9w7pwIDAQAB\n-----END PUBLIC KEY-----\n"
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