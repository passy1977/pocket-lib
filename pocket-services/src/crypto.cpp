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
#include "pocket/globals.hpp"

#include <openssl/sha.h>
#include <openssl/evp.h>
#include <openssl/rsa.h>
#include <openssl/engine.h>

#include <iomanip>
#include <iostream>
#include <cstring>
#include <stdexcept>
#include <string>
#include <random>
#include <chrono>
#include <memory>

namespace pocket::services::inline v5
{

using namespace std;

namespace
{

void throw_rsa_error(const string& msg)
{
    char err[256] = { 0 };
    ERR_error_string_n(ERR_get_error(), err, sizeof(err));

    throw runtime_error(msg + " err: " + err);
}

}

string crypto_encode_sha512(const string_view& str) noexcept
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

string crypto_encrypt_rsa(const string_view& pub_key, const string_view& plain_text, bool url_compliant)
{
    
    ENGINE *eng = nullptr;
    EVP_PKEY *pkey = nullptr;
    uint8_t *out = nullptr;
    size_t out_len = 0;


    BIO *bio = BIO_new_mem_buf(pub_key.data(), pub_key.length());
    if (bio == nullptr)
    {
        throw_rsa_error("Error on alloc BIO");
    }

    pkey = PEM_read_bio_PUBKEY(bio, &pkey, nullptr, nullptr);
    if (pkey == nullptr)
    {
        throw_rsa_error("Error on read public key");
    }

    auto ctx = EVP_PKEY_CTX_new(pkey, eng);
    if (!ctx)
    {
        throw_rsa_error("Error on allocates public key algorithm context");
    }

    if (EVP_PKEY_encrypt_init(ctx) <= 0)
    {
        throw_rsa_error("Error on initializes a public key algorithm context");
    }

    if (EVP_PKEY_CTX_set_rsa_padding(ctx, RSA_PKCS1_PADDING) <= 0)
    {
        throw_rsa_error("Error on set rsa padding");
    }

    if (EVP_PKEY_encrypt(ctx, nullptr, &out_len, reinterpret_cast<const uint8_t *>(plain_text.data()), plain_text.length()) <= 0)
    {
        throw_rsa_error("Error determine buffer length");
    }

    if (out = reinterpret_cast<uint8_t *>(OPENSSL_malloc(out_len)); out == nullptr)
    {
        throw_rsa_error("Error on OPENSSL_malloc()");
    }

    if (EVP_PKEY_encrypt(ctx, out, &out_len, reinterpret_cast<const uint8_t *>(plain_text.data()), plain_text.length()) <= 0)
    {
        throw_rsa_error("Error on encrypt");
    }

    auto&& ret = crypto_base64_encode(out, out_len, url_compliant);

    BIO_free_all(bio);
    EVP_PKEY_free(pkey);
    EVP_PKEY_CTX_free(ctx);
    OPENSSL_free(out);


    return ret;
}

string crypto_base64_encode(const uint8_t* data, size_t data_len, bool url_compliant)
{
    if(data == nullptr)
    {
        throw_rsa_error("data null");
    }

    size_t out_len = (static_cast<size_t>(EVP_ENCODE_LENGTH(data_len)) + 2) / 3 * 4;

    auto out = new(nothrow) uint8_t[out_len];
    if(out == nullptr)
    {
        throw_rsa_error("out impossible alloc");
    }
    int num_encoded = EVP_EncodeBlock(out, data, data_len);

    if (num_encoded < out_len && ((data[0] & 0x80) == 0 || (data[1] & 0x80) == 0))
    {
        memset(&out[num_encoded], '=', out_len - num_encoded);
    }

    string ret(reinterpret_cast<char*>(out), num_encoded);

    if(url_compliant)
    {
        str_replace_all(ret, "/", "_");

        str_replace_all(ret, "+", "-");
    }

    delete[] out;

    return ret;
}


string crypto_generate_random_string(size_t length)
{
    string const CHARACTERS = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";

    random_device random_device;
    mt19937 generator(random_device());
    uniform_int_distribution<> distribution(0, CHARACTERS.size() - 1);

    string random_string;

    for (size_t i = 0; i < length; ++i)
    {
        random_string += CHARACTERS[distribution(generator)];
    }

    return random_string;
}


crypto::crypto(char const iv[], const string& passwd)
: iv(iv)
, passwd(passwd)
{

}

crypto::~crypto() = default;

}

