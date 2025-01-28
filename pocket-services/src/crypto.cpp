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

#include <openssl/sha.h>
#include <openssl/evp.h>
#include <openssl/evperr.h>
#include <openssl/rsa.h>
#include <openssl/types.h>


#include <iomanip>
#include <iostream>
#include <cstring>


namespace pocket::services::inline v5
{

using namespace std;

string crypto_encode_sha512(const std::string_view& str) noexcept
{
    uint8_t hash[SHA512_DIGEST_LENGTH];
    SHA512(reinterpret_cast<const uint8_t *>(str.data()), str.length(), hash);

    stringstream ss;
    for (auto&& i : hash)
    {
        ss << hex << setw(2) << setfill('0') << static_cast<uint32_t>(i);
    }

    return ss.str();
}



optional<string> crypto_encrypt_rsa(const std::string_view& pub_key, const std::string_view& plain_text) noexcept
{
    uint8_t buffer[256];
    int bytes_written = 0;
    
    EVP_ENCODE_CTX* ctx = EVP_ENCODE_CTX_new();
    if(ctx == nullptr)
    {
        return nullopt;
    }

    EVP_EncodeInit(ctx);

    if (EVP_RSAEncryption(ctx, pub_key, EVP_PKEY_PUBLIC()) != 1) {
        printf("Errore in EVP_RSAEncryption\n");
        return nullopt;
    }

    if (EVP_EncodeUpdate(ctx, buffer, &bytes_written, reinterpret_cast<const uint8_t*>(plain_text.data()), plain_text.length()) != 1) {
        printf("Errore in EVP_EncodeUpdate\n");
        return nullopt;
    }

    EVP_EncodeFinal(ctx, buffer + bytes_written, &bytes_written);


    return string(reinterpret_cast<const char*>(buffer), strnlen(reinterpret_cast<const char*>(buffer), sizeof(buffer)));
}

}