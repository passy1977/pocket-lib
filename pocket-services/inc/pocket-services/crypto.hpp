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

#include <string>

namespace pocket::services::inline v5
{

std::string crypto_encode_sha512(const std::string_view& str) noexcept;

std::string crypto_encrypt_rsa(const std::string_view& pub_key, const std::string_view& plain_text, bool url_compliant = true);

std::string crypto_base64_encode(const uint8_t* data, size_t data_len, bool url_compliant = true);

std::string crypto_base64_encode(const std::string_view& data, bool url_compliant = true);

std::string crypto_generate_random_string(size_t length);

}