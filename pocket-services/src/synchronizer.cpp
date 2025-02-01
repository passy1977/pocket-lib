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
}

optional<user::ptr> synchronizer::get_data(uint64_t timestamp_last_update, string_view email, string_view passwd)
{
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
             if(secret.empty())
             {
                 secret = crypto_generate_random_string(10);
             }

             auto crypt = crypto_encrypt_rsa(device.host_pub_key, to_string(device.id) + DIVISOR + secret);

             prom.set_value(network.perform(network::method::GET, device.host + API_VERSION + "/session/" + device.uuid + "/" + crypt + "/" + to_string(timestamp_last_update) + "/" + string(email) + "/" + passwd));

         }
         catch (const runtime_error& e)
         {
             secret = "";
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
                     for_each(g.begin(), g.end(), [&data](auto &&it) mutable { data.groups_server_id[it->server_id] = it->id; });

                     auto&& gf = dao.get_all<group_field>();
                     for_each(gf.begin(), gf.end(), [&data](auto &&it) mutable { data.groups_fields_server_id[it->server_id] = it->id; });

                     auto&& f = dao.get_all<field>();
                     for_each(f.begin(), f.end(), [&data](auto &&it) mutable { data.fields_server_id[it->server_id] = it->id; });

                     prom_data.set_value(data);
                 }
                 catch (const runtime_error& e)
                 {
                     error(typeid(this).name(), e.what());
                 }
             });

            struct response json_response;
            try
            {
                json_parse_response(pool, response, json_response);
            }
            catch (const runtime_error& e)
            {
                error(typeid(this).name(), e.what());
                return nullopt;
            }

            data_server_id data;
            try
            {
                data = std::move(fut_data.get());
            }
            catch (const runtime_error& e)
            {
                error(typeid(this).name(), e.what());
                return nullopt;
            }

            update_database_table<group>(json_response, data.groups_server_id);
            update_database_table<group_field>(json_response, data.groups_fields_server_id);
            update_database_table<field>(json_response, data.fields_server_id);

            if(json_response.device->id == device.id)
            {
                return {std::move(json_response.user) };
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
            if(http_code >= 600)
            {
                error(typeid(this).name(), "Server error http_code:" + to_string(http_code));
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
}

void synchronizer::update_group_table(const data_server_id& data)
{
//    promise<void> prom_groups;
//    auto&& fut_groups = prom_groups.get_future();
//    pool.detach_task([&prom_groups, groups_server_id = &data.groups_server_id]
//     {
//         try
//         {
//             prom_groups.set_value();
//         }
//         catch (const runtime_error& e)
//         {
//             error(typeid(this).name(), e.what());
//         }
//     });
//
//    fut_groups.get();

}

void synchronizer::update_group_field_table(const data_server_id& data)
{
//    promise<void> prom_groups_fields;
//    auto&& fut_groups_fields = prom_groups_fields.get_future();
//    pool.detach_task([&prom_groups_fields, group_fields_server_id = &data.groups_fields_server_id]
//     {
//         try
//         {
//             prom_groups_fields.set_value();
//         }
//         catch (const runtime_error& e)
//         {
//             error(typeid(this).name(), e.what());
//         }
//     });
//
//
//    fut_groups_fields.get();

}

void synchronizer::update_field_table(const data_server_id& data)
{
//    promise<void> prom_fields;
//    auto&& fut_fields = prom_fields.get_future();
//    pool.detach_task([&prom_fields, fields_server_id = &data.fields_server_id]
//     {
//         try
//         {
//             prom_fields.set_value();
//         }
//         catch (const runtime_error& e)
//         {
//             error(typeid(this).name(), e.what());
//         }
//     });
//
//    fut_fields.get();
}


}

