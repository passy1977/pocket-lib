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
#include "pocket-services/crypto.hpp"
#include "pocket-daos/dao.hpp"

#include <stdexcept>
#include <ranges>

namespace pocket::services::inline v5
{

using namespace std;
using namespace pods;
using namespace daos;

namespace
{
constexpr char ERROR_HTTP_CODE[] = "http_code";
constexpr char APP_TAG[] = "synchronizer";
}

optional<device::ptr> synchronizer::get_full_data(uint64_t timestamp_last_update, string_view email, string_view passwd)
{
    struct data_server_id
    {
        std::vector<uint64_t> groups_server_id;
        std::vector<uint64_t> group_fields_server_id;
        std::vector<uint64_t> fields_server_id;
    };

    if(email.empty() || passwd.empty())
    {
        throw runtime_error("Some parameter are empty");
    }

    promise<string> prom;
    auto&& fut = prom.get_future();
    pool.submit_task([this, timestamp_last_update, email, passwd = std::move(crypto_encode_sha512(passwd)), &prom]
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

            promise<data_server_id> prom_data;
            auto&& fut_data = prom_data.get_future();
            pool.detach_task([this, &prom_data]
             {
                 try
                 {
                     data_server_id data ;
                     dao dao(database);

                     auto&& g = dao.get_all<group>();
                     transform(g.begin(), g.end(), data.groups_server_id.begin(), [](auto it) { return it.server_id; });

                     auto&& gf = dao.get_all<group_field>();
                     transform(gf.begin(), gf.end(), data.group_fields_server_id.begin(), [](auto it) { return it.server_id; });

                     auto&& f = dao.get_all<field>();
                     transform(f.begin(), f.end(), data.fields_server_id.begin(), [](auto it) { return it.server_id; });

                     prom_data.set_value(data);
                 }
                 catch (const runtime_error& e)
                 {
                     error(APP_TAG, e.what());
                 }
             });

            struct response json_response;
            try
            {
                json_parse_response(pool, response, json_response);

                if(!handle_token(json_response))
                {
                    return nullopt;
                }
            }
            catch (const runtime_error& e)
            {
                error(APP_TAG, e.what());
                return nullopt;
            }

            data_server_id data;
            try
            {
                data = std::move(fut_data.get());
            }
            catch (const runtime_error& e)
            {
                error(APP_TAG, e.what());
                return nullopt;
            }

            promise<void> prom_groups;
            auto&& fut_groups = prom_groups.get_future();
            pool.detach_task([&prom_groups, groups_server_id = &data.groups_server_id]
             {
                 try
                 {

                     prom_groups.set_value();
                 }
                 catch (const runtime_error& e)
                 {
                     error(APP_TAG, e.what());
                 }
             });

            promise<void> prom_groups_fields;
            auto&& fut_groups_fields = prom_groups_fields.get_future();
            pool.detach_task([&prom_groups_fields, group_fields_server_id = &data.group_fields_server_id]
             {
                 try
                 {

                     prom_groups_fields.set_value();
                 }
                 catch (const runtime_error& e)
                 {
                     error(APP_TAG, e.what());
                 }
             });

            promise<void> prom_fields;
            auto&& fut_fields = prom_fields.get_future();
            pool.detach_task([&prom_fields, fields_server_id = &data.fields_server_id]
             {
                 try
                 {

                     prom_fields.set_value();
                 }
                 catch (const runtime_error& e)
                 {
                     error(APP_TAG, e.what());
                 }
             });

            fut_groups.get();
            fut_groups_fields.get();
            fut_fields.get();


            if(json_response.device.get())
            {
                return {std::move(json_response.device) };
            }
            else
            {
                return nullopt;
            }
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

bool synchronizer::handle_token(const response& response) const noexcept
{
    auto&& token = crypto_decode_rsa(device.host_pub_key, response.token);

    return false;
}


}

