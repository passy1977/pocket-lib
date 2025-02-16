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

#include "pocket-controllers/session.hpp"
#include "pocket/tree.hpp"

using namespace pocket::controllers;

constexpr char data[] = R"json(
{
   "id":59,
   "uuid":"727fd661-ed0a-4393-a8dc-c4a488a91719",
   "status":"ACTIVE",
   "timestampCreation":1739139440,
   "userId":5,
   "host":"http://127.0.0.1:8081",
   "hostPublicKey":"-----BEGIN PUBLIC KEY-----\nMIIBIjANBgkqhkiG9w0BAQEFAAOCAQ8AMIIBCgKCAQEAy1JALwTL/IkKT01sWz4ICGvookAKImKZe/5qYE6dslmm5jh5hT71T20GEKghW/BuT4d/WyMkPEkCTOzX9KX30t1Hy/ZdkTsh6Ps5gUwxRTEvLbiG9pOO8hqK2t+LNvs4BvKHrMyHVJj77jYRfaMXiq3p2YkfbPYsz8erbcj5Fu4FffdSURxmGw/cK1wagWJRHn4jUpOeLyIty421CGHhYQ1UgIPgneqZ6aEP3WUYeGlfagq7ZpJbdjFnqeYhkXOaraA4CeWGEUBfILMwKZoKOyiCXaiUF+/Abw7l+sjIxaLf+MPUgPQFnjSJ0V4l0+gDfKdXUzwYi5AbqsDsrCxinwIDAQAB\n-----END PUBLIC KEY-----\n"
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

    auto user = session.login("test@test.it", "pwd");
    ASSERT_TRUE(user.has_value());

    ASSERT_TRUE(session.send_data(user));

}
catch (const std::exception& e)
{
    std::cerr << e.what() << std::endl;
    ASSERT_TRUE(false);
}


TEST_F(session_test, tree_test) try
{

    using pocket::tree;
    using pocket::pods::group;

    tree t;

    //t.add();


}
catch (const std::exception& e)
{
    std::cerr << e.what() << std::endl;
    ASSERT_TRUE(false);
}
