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
#include <openssl/rsa.h>
#include <openssl/engine.h>

#include <iomanip>
#include <iostream>
#include <cstring>
#include <stdexcept>


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



optional<string> crypto_encrypt_rsa(const std::string_view& pub_key, const std::string_view& plain_text)
{

    string k = R"(
-----BEGIN PUBLIC KEY-----
MIIBIjANBgkqhkiG9w0BAQEFAAOCAQ8AMIIBCgKCAQEAh/GDeTWnA15/M1Yiw+Cm0Umm+kCInzLWqFpKsWQ5g36GNnr24v0/cNyxRziSYKvF2bg9MHoyrqyfNP0bZ85NmAe3uc75uONYinjKwwRy1arp6LWO387ATIvpyGGGWKs8cYgEm5qndHW4NTYp7IRHE4KKyT24FVCD0ASOJx4HTZkSev3XnvLaV3oIlM8Ay3Ogf8NHhJKny91SQ8YSGrJ7nFZoAWlk/mv1Z43DS9P1KoTSCzX2LrF2H7ARF59mnx/8T1NWK+VltssxF0kJWzDVSCVJfILzoF3Kr8EUX42boKTszeOw/+X6614SEs6vvNwOSQMl83Q5MzIyAVRkw+58VQIDAQAB
-----END PUBLIC KEY-----
)";

    EVP_PKEY_CTX *ctx = nullptr;
    ENGINE *eng = nullptr;
    unsigned char *out = (uint8_t*)plain_text.data(), *in = nullptr;
    size_t outlen = 0, inlen  = 0;
    EVP_PKEY *pkey = NULL;
// Creiamo un BIO della memoria
    BIO *bio = BIO_new_mem_buf(k.data(), k.length());
    if (bio == NULL) {
        // Gestire l'errore
        throw runtime_error("BIO_new_mem_buf()");
    }

// Leggiamo la chiave pubblica dal BIO
pkey = PEM_read_bio_PUBKEY(bio, &pkey, NULL, NULL);
    if (pkey == NULL) {
        // Gestire l'errore
        throw runtime_error("PEM_read_bio_PUBKEY()");
    }
// Dobbiamo chiudere il BIO
    BIO_free(bio);

// Verificare se la chiave è stata caricata correttamente


/*
 * NB: assumes key, eng, in, inlen are already set up
 * and that key is an RSA private key
 */
    ctx = EVP_PKEY_CTX_new(pkey, eng);
    if (!ctx)
    {
        /* Error occurred */
        throw runtime_error("EVP_PKEY_CTX_new()");
    }

    if (EVP_PKEY_encrypt_init(ctx) <= 0)
    {
        /* Error */
        throw runtime_error("EVP_PKEY_encrypt_init()");
    }

    if (EVP_PKEY_CTX_set_rsa_padding(ctx, RSA_PKCS1_OAEP_PADDING) <= 0)
    {
        /* Error */
        throw runtime_error("EVP_PKEY_CTX_set_rsa_padding()");
    }

    /* Determine buffer length */
    if (EVP_PKEY_encrypt(ctx, NULL, &outlen, in, inlen) <= 0)
    {
        /* Error */
        throw runtime_error("EVP_PKEY_encrypt()");
    }

    out = reinterpret_cast<unsigned char *>(OPENSSL_malloc(outlen));
    if (!out)
    {
        /* malloc failure */
        throw runtime_error("OPENSSL_malloc()");
    }


    if (EVP_PKEY_encrypt(ctx, out, &outlen, in, inlen) <= 0)
    {
        /* Error */
        throw runtime_error("EVP_PKEY_encrypt()");
    }


    EVP_PKEY_free(pkey);
    EVP_PKEY_CTX_free(ctx);
    OPENSSL_free(out);


    //return string(reinterpret_cast<const char*>(buffer), strnlen(reinterpret_cast<const char*>(buffer), sizeof(buffer)));
    return string("");
}

}