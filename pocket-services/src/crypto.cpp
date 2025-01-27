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
#include <openssl/rsa.h>
#include <openssl/pem.h>
#include <openssl/bio.h>
#include <openssl/bn.h>
#include <openssl/err.h>

#include <iomanip>
#include <iostream>


namespace pocket::services::inline v5
{

using namespace std;

std::string crypto_encode_sha512(const std::string_view& str) noexcept
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

// Funzione per convertire una stringa in una struttura EVP_PKEY da RSA
evp_pkey_st* createPublicKey(const unsigned char* publicKeyPEM, size_t len) {
    BIO *bio = BIO_new_mem_buf(publicKeyPEM, len);
    if (!bio) {
        std::cerr << "Errore nella creazione del bio" << std::endl;
        return NULL;
    }

    evp_pkey_st* publicKey = PEM_read_bio_RSA_PUBKEY(bio, NULL, NULL, NULL);
    BIO_free_all(bio);

    if (!publicKey) {
        ERR_print_errors_fp(stderr);
    }
    return publicKey;
}

// Funzione per decifrare il messaggio
int decryptWithPublicKey(EVP_PKEY *pkey, const unsigned char *ciphertext, size_t ciphertext_len, unsigned char **plaintext) {
    RSA *rsa = EVP_PKEY_get0_RSA(pkey);

    int plaintextlen = RSA_public_decrypt(ciphertext_len,
                                          (const unsigned char*)ciphertext,
                                          (*plaintext),
                                          rsa,
                                          RSA_PKCS1_PADDING);

    if (plaintextlen < 0) {
        // Gestisci l'errore
        return -1;
    }
    *plaintextlen = plaintextlen;

    return 0;
}


std::string crypto_decode_rsa(const std::string_view& pub_key, const std::string_view& cipher_text) noexcept
{
    return "";
}

}