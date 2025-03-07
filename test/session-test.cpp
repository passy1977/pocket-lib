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
   "host":"http://192.168.12.110:8081",
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
    using namespace pocket::pods;
    session session(data);

    session.init();

    auto user = session.login("test@test.it", "pwd");
    ASSERT_TRUE(user.has_value());

    auto&& g1 = std::make_unique<group>();
    g1->title = "g1";
    g1->synchronized = false;
    g1->id = session.get_view_group()->persist(g1);

    auto&& gf1_1 = std::make_unique<group_field>();
    gf1_1->title = "g1 1";
    gf1_1->synchronized = false;
    gf1_1->group_id = g1->id;
    gf1_1->id = session.get_view_group_field()->persist(gf1_1);

    auto&& gf1_2 = std::make_unique<group_field>();
    gf1_2->title = "g1 2";
    gf1_2->synchronized = false;
    gf1_2->group_id = g1->id;
    gf1_2->id = session.get_view_group_field()->persist(gf1_2);

    auto&& g2 = std::make_unique<group>();
    g2->title = "g2";
    g2->group_id = g1->id;
    g2->synchronized = false;
    g2->id = session.get_view_group()->persist(g2);

    auto&& gf2_1 = std::make_unique<group_field>();
    gf2_1->title = "g2 1";
    gf2_1->synchronized = false;
    gf2_1->group_id = g2->id;
    gf2_1->id = session.get_view_group_field()->persist(gf2_1);

    auto&& gf2_2 = std::make_unique<group_field>();
    gf2_2->title = "g2 2";
    gf2_2->synchronized = false;
    gf2_2->group_id = g2->id;
    gf2_2->id = session.get_view_group_field()->persist(gf2_2);

    ASSERT_TRUE(session.send_data(user));

    auto&& gf2_3 = std::make_unique<group_field>();
    gf2_3->title = "g2 3";
    gf2_3->synchronized = false;
    gf2_3->group_id = g2->id;
    gf2_3->id = session.get_view_group_field()->persist(gf2_3);

    ASSERT_TRUE(session.send_data(user));


    gf2_2->title = "g2 2 - mod";
    gf2_2->synchronized = false;
    gf2_2->id = session.get_view_group_field()->persist(gf2_2);

    g1->title = "g1 - mod";
    g1->synchronized = false;
    g1->id = session.get_view_group()->persist(g1);

    ASSERT_TRUE(session.send_data(user));

    session.get_view_group()->del(g1->id);

    session.get_view_group_field()->del(gf2_2->id);

//    session.get_view_group()->del(2);
//
//    session.get_view_group_field()->del(1);

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
