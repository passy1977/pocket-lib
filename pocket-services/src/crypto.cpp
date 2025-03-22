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

string get_open_ssl_error() 
{
    BIO *bio = BIO_new(BIO_s_mem());
    ERR_print_errors(bio);
    char *buf;
    size_t len = BIO_get_mem_data(bio, &buf);
    string ret(buf, len);
    BIO_free(bio);
    return ret;
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


    auto bio = BIO_new_mem_buf(pub_key.data(), static_cast<int>(pub_key.length()));
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
    int num_encoded = EVP_EncodeBlock(out, data, static_cast<int>(data_len));

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

std::vector<uint8_t> crypto_base64_decode(std::string data, bool url_compliant)
{
    if(url_compliant)
    {
        str_replace_all(data, "_", "/");

        str_replace_all(data, "-", "+");
    }

    vector<uint8_t> ret;
    
    auto buffer = new(nothrow) uint8_t[data.size()];
    if(buffer == nullptr)
    {
        throw_rsa_error("buffer impossible alloc");
    }
    memset(buffer, 0x00, data.size());

    auto b64 = BIO_new(BIO_f_base64());
    auto bmem = BIO_new_mem_buf(data.data(), static_cast<int>(data.size()));
    bmem = BIO_push(b64, bmem);

    BIO_set_flags(bmem, BIO_FLAGS_BASE64_NO_NL);
    BIO_set_close(bmem, BIO_CLOSE);
    int buffer_length = BIO_read(bmem, buffer, static_cast<int>(data.size()));
    if(buffer_length > 0)
    {
        for (uint32_t i = 0; i < buffer_length; i++)
        {
            ret.push_back(buffer[i]);
        }
    }
    else
    {
        throw runtime_error("BIO_read less then 0 bytes read err:" + to_string(ERR_get_error()));
    }
    
    BIO_free_all(bmem);

    delete [] buffer;

    return ret;
}

string crypto_generate_random_string(size_t length)
{
    string const CHARACTERS = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";

    random_device random_device;
    mt19937 generator(random_device());
    uniform_int_distribution<> distribution(0, static_cast<int>(CHARACTERS.size() - 1));

    string random_string;

    for (size_t i = 0; i < length; ++i)
    {
        random_string += CHARACTERS[distribution(generator)];
    }

    return random_string;
}


aes::aes(const string&& iv, const string& key)
{
    if (iv.length() != AES_BLOCK_SIZE)
    {
        throw runtime_error("iv size != " + to_string(AES_BLOCK_SIZE));
    }

//    uint8_t i = 0;
//    for (; i < key.size() && i < KEY_SIZE; i++)
//    {
//        this->key[i] = key[i];
//    }
//    for (; i < KEY_SIZE; i++)
//    {
//        this->key[i] = PADDING;
//    }

    memcpy(this->key, set_key_padding(key).data(), KEY_SIZE);

    memset(this->iv, 0x00, AES_BLOCK_SIZE);
    memcpy(this->iv, iv.data(), AES_BLOCK_SIZE);

    /* Create and initialise the context */
    if(!(ctx = EVP_CIPHER_CTX_new()))
    {
        throw runtime_error(get_open_ssl_error());
    }

}

aes::~aes()
{
    for(auto&& b : key)
    {
        b = 0;
    }
    for(auto&& b : iv)
    {
        b = 0;
    }

    /* Clean up */
    EVP_CIPHER_CTX_free(ctx);
}

std::string aes::encrypt(const string_view& plain, bool url_compliant) const
{
    if(plain.empty())
    {
        return  "";
    }

    auto cipher_text = new(nothrow) uint8_t[((plain.size() + AES_BLOCK_SIZE) / AES_BLOCK_SIZE) * AES_BLOCK_SIZE];
    if(cipher_text == nullptr)
    {
        throw runtime_error("No memory for cipher_text");
    }

    int len = 0;
    int cipher_text_len = 0;

    /*
     * Initialise the encryption operation. IMPORTANT - ensure you use a key
     * and IV size appropriate for your cipher
     * In this example we are using 256 bit AES (i.e. a 256 bit key). The
     * IV size for *most* modes is the same as the block size. For AES this
     * is 128 bits
     */
    if(EVP_EncryptInit_ex(ctx, EVP_aes_256_cbc(), nullptr, this->key, this->iv) != 1)
    {
        throw runtime_error(get_open_ssl_error());
    }

    /*
     * Provide the message to be encrypted, and obtain the encrypted output.
     * EVP_EncryptUpdate can be called multiple times if necessary
     */
    if(EVP_EncryptUpdate(ctx, cipher_text, &len, reinterpret_cast<const uint8_t *>(plain.data()), static_cast<int>(plain.size()) ) != 1)
    {
        throw runtime_error(get_open_ssl_error());
    }
    cipher_text_len = len;

    /*
     * Finalise the encryption. Further ciphertext bytes may be written at
     * this stage.
     */
    if(EVP_EncryptFinal_ex(ctx, cipher_text + len, &len) != 1)
    {
        throw runtime_error(get_open_ssl_error());
    }
    cipher_text_len += len;
    
    auto&& ret = crypto_base64_encode(cipher_text, cipher_text_len, url_compliant);

    delete[] cipher_text;

    return ret;
}

std::string aes::decrypt(const string_view& encrypted, bool url_compliant) const
{
    if(encrypted.empty())
    {
        return  "";
    }

    auto&& cipher = crypto_base64_decode(encrypted.data(), url_compliant);

    auto plain_text = new(nothrow) uint8_t[encrypted.size()];
    if(plain_text == nullptr)
    {
        throw runtime_error("No memory for plain_text");
    }


    int len = 0;
    int plain_text_len = 0;

    /*
    * Initialise the decryption operation. IMPORTANT - ensure you use a key
    * and IV size appropriate for your cipher
    * In this example we are using 256 bit AES (i.e. a 256 bit key). The
    * IV size for *most* modes is the same as the block size. For AES this
    * is 128 bits
    */
    if(EVP_DecryptInit_ex(ctx, EVP_aes_256_cbc(), nullptr, key, iv) != 1)
    {
        throw runtime_error(get_open_ssl_error());
    }

    /*
    * Provide the message to be decrypted, and obtain the plaintext output.
    * EVP_DecryptUpdate can be called multiple times if necessary.
    */
    if(EVP_DecryptUpdate(ctx, plain_text, &len, cipher.data(), static_cast<int>(cipher.size())) != 1)
    {
        throw runtime_error(get_open_ssl_error());
    }
    plain_text_len = len;

    /*
    * Finalise the decryption. Further plaintext bytes may be written at
    * this stage.
    */
    if(EVP_DecryptFinal_ex(ctx, plain_text + len, &len) != 1)
    {
        throw runtime_error(get_open_ssl_error());
    }
    plain_text_len += len;

    string ret;

    for(uint32_t i = 0; i < plain_text_len; i++)
    {
        ret.push_back(static_cast<char>(plain_text[i]));
    }

    delete[] plain_text;

    return ret;
}

vector<uint8_t> aes::set_key_padding(const string_view& key) noexcept
{
    uint8_t local_key[KEY_SIZE]{0};
    uint8_t i = 0;
    for (; i < key.size() && i < KEY_SIZE; i++)
    {
        local_key[i] = key[i];
    }
    for (; i < KEY_SIZE; i++)
    {
        local_key[i] = PADDING;
    }

    return {local_key, local_key + sizeof(local_key)};
}

}

