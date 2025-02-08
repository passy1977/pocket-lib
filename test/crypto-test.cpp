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

#include "pocket-services/crypto.hpp"

#include <gtest/gtest.h>


using namespace pocket::services;
using namespace std;

struct crypto_test : public ::testing::Test {};

TEST_F(crypto_test, base_encrypt) try
{
    using pocket::services::crypto;

    crypto crypto("123456789qwertyu", "123456789qwertyu123456789qwertyu");

    auto enc = crypto.encrypt("ciao");

    ASSERT_TRUE(enc == "SPGUc3Sz2ftgNjm1Bz10rg==");


}
catch (const std::exception& e)
{
    std::cerr << e.what() << std::endl;
    ASSERT_TRUE(false);
}
