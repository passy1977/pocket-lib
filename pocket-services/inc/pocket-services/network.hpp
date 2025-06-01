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
#include "pocket-pods/variant.hpp"

#include <curl/curl.h>
#include <vector>
#include <map>

#ifndef POCKET_MAX_BUFFER_RESPONSE_SIZE
#define POCKET_MAX_BUFFER_RESPONSE_SIZE 10485760
#endif

namespace pocket::services::inline v5
{

class network final
{

    using parameters = std::vector<pods::variant>;
    using map_parameters = std::map<std::string, pods::variant>;

    CURL* curl = nullptr;
    curl_slist* headers = nullptr;
    
    uint64_t http_code = 0;
    long timeout = 0;
    long connect_timeout = 0;
public:    
    enum class method
    {
        GET, POST, PUT, DEL
    };

    static inline constexpr uint32_t MAX_BUFFER = POCKET_MAX_BUFFER_RESPONSE_SIZE;

    network();
    ~network();
    POCKET_NO_COPY_NO_MOVE(network)

    std::string perform(method method, const std::string_view& url, const map_parameters& params = {}, const std::string_view& json_data = {});

    inline void set_timeout(long timeout) noexcept
    {
        network::timeout = timeout;
    }

    inline void set_connect_timeout(long connect_timeout) noexcept
    {
        network::connect_timeout = connect_timeout;
    }

    inline uint64_t get_http_code() const noexcept
    {
        return http_code;
    }
private:
    static size_t callback(char* buf, size_t size, size_t nmemb, std::string* ret_data);

};

}
