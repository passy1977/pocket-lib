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

#include "pocket-services/synchronizer.hpp"
#include "pocket-services/network.hpp"
#include "pocket-services/json.hpp"

#include <openssl/sha.h>
#include <stdexcept>
#include <iomanip>

namespace pocket::services::inline v5
{

using namespace std;
using namespace pods;

namespace
{
constexpr char ERROR_HTTP_CODE[] = "http_code";
}

optional<device> synchronizer::get_full_data(uint64_t timestamp_last_update, string_view email, string_view passwd)
{
    if(email.empty() || passwd.empty())
    {
        throw runtime_error("Some parameter are empty");
    }

    uint8_t hash[SHA512_DIGEST_LENGTH];
    SHA512(reinterpret_cast<const uint8_t *>(passwd.data()), passwd.length(), hash);

    stringstream ss;
    for (auto&& i : hash)
    {
        ss << hex << setw(2) << setfill('0')  << static_cast<uint32_t>(i);
    }

    promise<string> prom;

    auto&& fut = prom.get_future();

    pool.submit_task([this, timestamp_last_update, email, passwd = ss.str(), &prom]
     {
         network network;

         try
         {
             prom.set_value(network.perform(network::method::GET, device.host + API_VERSION + "/session/" + device.uuid + "/" + to_string(timestamp_last_update) + "/" + string(email) + "/" + passwd));
         }
         catch (const runtime_error& e)
         {
             prom.set_value(string(ERROR_HTTP_CODE) + e.what());
         }

     })
    .wait();

    auto&& response = fut.get();
    if(!response.starts_with(ERROR_HTTP_CODE))
    {
        try
        {
            struct response json_response;
            json_parse_response(pool, response, json_response);


        }
        catch (const runtime_error& e)
        {
            throw runtime_error(e.what());
        }
    }
    else
    {
        try {
            int http_code = stoi(response.substr(strnlen(ERROR_HTTP_CODE, sizeof ERROR_HTTP_CODE)));
            if(http_code == 401)
            {
                return nullopt;
            }
            else
            {
                throw runtime_error(response);
            }
        } catch (const std::invalid_argument& ia) {
            throw runtime_error(ia.what());
        } catch (const std::out_of_range& oor) {
            throw runtime_error(oor.what());
        }
    }

    return nullopt;
}



}

