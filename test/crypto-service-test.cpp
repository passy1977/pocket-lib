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
#include "pocket-services/crypto.hpp"
#include <iostream>
#include <vector>

using namespace pocket::services;

class CryptoServiceTest : public ::testing::Test 
{
protected:
    void SetUp() override {}
    void TearDown() override {}
};

// Test SHA512 encoding
TEST_F(CryptoServiceTest, EncodeShA512Basic)
{
    std::string input = "test";
    std::string result = crypto_encode_sha512(input);
    
    // SHA512 should always produce 128 hex characters (64 bytes * 2)
    EXPECT_EQ(result.length(), 128);
    
    // Test known hash
    std::string expected = "ee26b0dd4af7e749aa1a8ee3c10ae9923f618980772e473f8819a5d4940e0db27ac185f8a0e1d5f84f88bc887fd67b143732c304cc5fa9ad8e6f57f50028a8ff";
    EXPECT_EQ(result, expected);
}

TEST_F(CryptoServiceTest, EncodeShA512Empty)
{
    std::string input = "";
    std::string result = crypto_encode_sha512(input);
    
    EXPECT_EQ(result.length(), 128);
    // Known hash for empty string
    std::string expected = "cf83e1357eefb8bdf1542850d66d8007d620e4050b5715dc83f4a921d36ce9ce47d0d13c5d85f2b0ff8318d2877eec2f63b931bd47417a81a538327af927da3e";
    EXPECT_EQ(result, expected);
}

TEST_F(CryptoServiceTest, EncodeShA512Consistency)
{
    std::string input = "pocket-lib test string";
    std::string result1 = crypto_encode_sha512(input);
    std::string result2 = crypto_encode_sha512(input);
    
    EXPECT_EQ(result1, result2);
    EXPECT_EQ(result1.length(), 128);
}

// Test Base64 encoding/decoding
TEST_F(CryptoServiceTest, Base64EncodeDecodeBasic)
{
    std::string input = "Hello World!";
    std::string encoded = crypto_base64_encode(reinterpret_cast<const uint8_t*>(input.data()), input.length(), false);
    std::vector<uint8_t> decoded = crypto_base64_decode(encoded, false);
    
    std::string decoded_str(decoded.begin(), decoded.end());
    EXPECT_EQ(input, decoded_str);
}

TEST_F(CryptoServiceTest, Base64EncodeDecodeEmpty)
{
    std::string input = "";
    // Empty string encoding should handle gracefully or throw expected exception
    EXPECT_NO_THROW({
        std::string encoded = crypto_base64_encode(reinterpret_cast<const uint8_t*>(input.data()), input.length(), false);
        if (!encoded.empty()) {
            std::vector<uint8_t> decoded = crypto_base64_decode(encoded, false);
            std::string decoded_str(decoded.begin(), decoded.end());
            EXPECT_EQ(input, decoded_str);
        }
    });
}

TEST_F(CryptoServiceTest, Base64UrlCompliant)
{
    std::string input = "test?+/=";
    std::string encoded_normal = crypto_base64_encode(reinterpret_cast<const uint8_t*>(input.data()), input.length(), false);
    std::string encoded_url = crypto_base64_encode(reinterpret_cast<const uint8_t*>(input.data()), input.length(), true);
    
    // URL compliant should replace + and / with - and _
    EXPECT_EQ(encoded_url.find('+'), std::string::npos);
    EXPECT_EQ(encoded_url.find('/'), std::string::npos);
    // Note: Implementation may still include padding '=' characters
    
    // Both should decode to same result
    std::vector<uint8_t> decoded_normal = crypto_base64_decode(encoded_normal, false);
    std::vector<uint8_t> decoded_url = crypto_base64_decode(encoded_url, true);
    
    EXPECT_EQ(decoded_normal, decoded_url);
}

TEST_F(CryptoServiceTest, Base64BinaryData)
{
    std::vector<uint8_t> binary_data = {0x00, 0x01, 0x02, 0xFF, 0xFE, 0xFD};
    std::string encoded = crypto_base64_encode(binary_data.data(), binary_data.size(), false);
    std::vector<uint8_t> decoded = crypto_base64_decode(encoded, false);
    
    EXPECT_EQ(binary_data, decoded);
}

// Test random string generation
TEST_F(CryptoServiceTest, GenerateRandomStringLength)
{
    for (size_t len : {0, 1, 10, 32, 64, 128}) {
        std::string random_str = crypto_generate_random_string(len);
        EXPECT_EQ(random_str.length(), len);
    }
}

TEST_F(CryptoServiceTest, GenerateRandomStringUniqueness)
{
    std::string random1 = crypto_generate_random_string(32);
    std::string random2 = crypto_generate_random_string(32);
    
    EXPECT_NE(random1, random2);
    EXPECT_EQ(random1.length(), 32);
    EXPECT_EQ(random2.length(), 32);
}

TEST_F(CryptoServiceTest, GenerateRandomStringCharacters)
{
    std::string random_str = crypto_generate_random_string(100);
    
    // Check all characters are printable ASCII
    for (char c : random_str) {
        EXPECT_TRUE(c >= 32 && c <= 126) << "Character: " << static_cast<int>(c);
    }
}

// Test AES encryption/decryption
TEST_F(CryptoServiceTest, AESEncryptDecryptBasic)
{
    std::string iv = "1234567890123456"; // 16 bytes
    std::string key = "password123";
    std::string plaintext = "This is a test message for AES encryption!";
    
    aes cipher(iv, key);
    
    std::string encrypted = cipher.encrypt(plaintext);
    std::string decrypted = cipher.decrypt(encrypted);
    
    EXPECT_EQ(plaintext, decrypted);
    EXPECT_NE(plaintext, encrypted);
}

TEST_F(CryptoServiceTest, AESEncryptDecryptEmpty)
{
    std::string iv = "1234567890123456";
    std::string key = "password123";
    std::string plaintext = "";
    
    aes cipher(iv, key);
    
    std::string encrypted = cipher.encrypt(plaintext);
    std::string decrypted = cipher.decrypt(encrypted);
    
    EXPECT_EQ(plaintext, decrypted);
}

TEST_F(CryptoServiceTest, AESUrlCompliant)
{
    std::string iv = "1234567890123456";
    std::string key = "password123";
    std::string plaintext = "Test message with special chars: +/=";
    
    aes cipher(iv, key);
    
    std::string encrypted_normal = cipher.encrypt(plaintext, false);
    std::string encrypted_url = cipher.encrypt(plaintext, true);
    
    // URL compliant should not contain +, /, =
    EXPECT_EQ(encrypted_url.find('+'), std::string::npos);
    EXPECT_EQ(encrypted_url.find('/'), std::string::npos);
    EXPECT_EQ(encrypted_url.find('='), std::string::npos);
    
    // Both should decrypt to same result
    std::string decrypted_normal = cipher.decrypt(encrypted_normal, false);
    std::string decrypted_url = cipher.decrypt(encrypted_url, true);
    
    EXPECT_EQ(plaintext, decrypted_normal);
    EXPECT_EQ(plaintext, decrypted_url);
}

TEST_F(CryptoServiceTest, AESKeyPadding)
{
    std::string short_key = "short";
    std::string long_key = "this_is_a_very_long_key_that_exceeds_32_bytes_significantly";
    
    auto padded_short = aes::set_key_padding(short_key);
    auto padded_long = aes::set_key_padding(long_key);
    
    EXPECT_EQ(padded_short.size(), 32); // AES key size
    EXPECT_EQ(padded_long.size(), 32);
    
    // Short key should be padded
    EXPECT_EQ(padded_short[short_key.length()], '$'); // PADDING char
    
    // Long key should be truncated
    for (size_t i = 0; i < 32; ++i) {
        EXPECT_EQ(padded_long[i], static_cast<uint8_t>(long_key[i]));
    }
}

TEST_F(CryptoServiceTest, AESConsistency)
{
    std::string iv = "1234567890123456";
    std::string key = "password123";
    std::string plaintext = "Consistency test message";
    
    aes cipher1(iv, key);
    aes cipher2(iv, key);
    
    std::string encrypted1 = cipher1.encrypt(plaintext);
    std::string encrypted2 = cipher2.encrypt(plaintext);
    
    // Same IV and key should produce same result
    EXPECT_EQ(encrypted1, encrypted2);
    
    std::string decrypted1 = cipher1.decrypt(encrypted1);
    std::string decrypted2 = cipher2.decrypt(encrypted2);
    
    EXPECT_EQ(plaintext, decrypted1);
    EXPECT_EQ(plaintext, decrypted2);
}

TEST_F(CryptoServiceTest, AESMoveSemantics)
{
    std::string iv = "1234567890123456";
    std::string key = "password123";
    std::string plaintext = "Move semantics test";
    
    aes cipher(iv, key);
    
    // Test rvalue encrypt/decrypt
    std::string encrypted = cipher.encrypt(std::string(plaintext));
    std::string decrypted = cipher.decrypt(std::string(encrypted));
    
    EXPECT_EQ(plaintext, decrypted);
}

TEST_F(CryptoServiceTest, AESLargeData)
{
    std::string iv = "1234567890123456";
    std::string key = "password123";
    
    // Create large plaintext (multiple AES blocks)
    std::string plaintext;
    for (int i = 0; i < 1000; ++i) {
        plaintext += "This is block " + std::to_string(i) + " of test data. ";
    }
    
    aes cipher(iv, key);
    
    std::string encrypted = cipher.encrypt(plaintext);
    std::string decrypted = cipher.decrypt(encrypted);
    
    EXPECT_EQ(plaintext, decrypted);
    EXPECT_GT(encrypted.length(), 0);
}