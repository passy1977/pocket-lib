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

TEST_F(crypto_test, base64) try
{
    using namespace pocket::services;

    uint8_t buff[] = "ciao";

    auto enc = crypto_base64_encode(buff, sizeof(buff));

    auto dec = crypto_base64_decode(enc);

    ASSERT_TRUE(memcmp(buff, dec.data(), sizeof(buff)) == 0);

}
catch (const std::exception& e)
{
    std::cerr << e.what() << std::endl;
    ASSERT_TRUE(false);
}

TEST_F(crypto_test, base64_url) try
{
    using namespace pocket::services;

    uint8_t buff[] = "ciao";

    auto enc = crypto_base64_encode(buff, sizeof(buff), true);

    auto dec = crypto_base64_decode(enc, true);

    ASSERT_TRUE(memcmp(buff, dec.data(), sizeof(buff)) == 0);

}
catch (const std::exception& e)
{
    std::cerr << e.what() << std::endl;
    ASSERT_TRUE(false);
}

TEST_F(crypto_test, aes) try
{
    using pocket::services::aes;

    aes crypto("zxcvbnmasdfghjkl", "123456789qwertyu123456789qwertyu");

    auto&& enc = crypto.encrypt("ciao");

    ASSERT_TRUE(enc == "eKIbCVIPzA+wkVJEkbsv6g==");

    enc = crypto.encrypt("sono");

    ASSERT_TRUE(enc == "CRaDt0WrN+7ybjum5680mA==");

    auto&& dec = crypto.decrypt("eKIbCVIPzA+wkVJEkbsv6g==");

    ASSERT_TRUE(dec == "ciao");

    dec = crypto.decrypt("CRaDt0WrN+7ybjum5680mA==");

    ASSERT_TRUE(dec == "sono");

}
catch (const std::exception& e)
{
    std::cerr << e.what() << std::endl;
    ASSERT_TRUE(false);
}

TEST_F(crypto_test, aes2) try
{
    using pocket::services::aes;

    aes crypto("1234567812345678", "");

    auto&& enc = crypto.encrypt("ciao");

    ASSERT_TRUE(enc == "eKIbCVIPzA+wkVJEkbsv6g==");

    enc = crypto.encrypt("sono");

    ASSERT_TRUE(enc == "CRaDt0WrN+7ybjum5680mA==");

    auto&& dec = crypto.decrypt("eKIbCVIPzA+wkVJEkbsv6g==");

    ASSERT_TRUE(dec == "ciao");

    dec = crypto.decrypt("CRaDt0WrN+7ybjum5680mA==");

    ASSERT_TRUE(dec == "sono");

}
catch (const std::exception& e)
{
    std::cerr << e.what() << std::endl;
    ASSERT_TRUE(false);
}

