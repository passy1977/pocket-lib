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

#pragma once

#include "pocket/globals.hpp"

#include <string>
#include <openssl/aes.h>

#ifndef POCKET_AES_CBC_IV
#error POCKET_AES_CBC_IV not defined
#endif

typedef struct evp_cipher_ctx_st EVP_CIPHER_CTX;
namespace pocket::services::inline v5
{

std::string crypto_encode_sha512(const std::string_view& str) noexcept;

std::string crypto_encrypt_rsa(const std::string_view& pub_key, const std::string_view& plain_text, bool url_compliant = true);

std::string crypto_base64_encode(const uint8_t* data, size_t data_len, bool url_compliant = true);

std::vector<uint8_t> crypto_base64_decode(std::string data, bool url_compliant = true);

std::string crypto_generate_random_string(size_t length);

class aes final
{
    static inline constexpr uint8_t KEY_SIZE = 32;
    static inline constexpr char PADDING = '$';

    uint8_t key[KEY_SIZE]{0};
    uint8_t iv[AES_BLOCK_SIZE]{0};

    EVP_CIPHER_CTX *ctx = nullptr;
public:
    using ptr = std::unique_ptr<aes>;

    aes(const std::string&& iv, const std::string& key);
    POCKET_NO_COPY_NO_MOVE(aes)
    ~aes();

    std::string encrypt(const std::string_view &plain) const;
    inline std::string encrypt(const std::string_view &&plain) const
    {
        return encrypt(plain);
    }

    std::string  decrypt(const std::string_view &encrypted) const;
    inline std::string decrypt(const std::string_view &&encrypted) const
    {
        return decrypt(encrypted);
    }


};

}