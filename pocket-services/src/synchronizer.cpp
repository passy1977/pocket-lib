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

            database->disable_foreign_keys();
            auto&& fut_group = update_database_table<group, std::vector<group::ptr>>(json_response.groups);
//            auto&& fut_group_field = update_database_table<group_field, std::vector<group_field::ptr>>(json_response.groups_fields);
//            auto&& fut_field = update_database_table<field, std::vector<field::ptr>>(json_response.fields);
            database->enable_foreign_keys();

            if(!fut_group.get())
            {
                error(typeid(this).name(), "Some error on populate groups table");
                return nullopt;
            }

//            if(!fut_group_field.get())
//            {
//                error(typeid(this).name(), "Some error on populate groups_fields table");
//                return nullopt;
//            }
//
//            if(!fut_field.get())
//            {
//                error(typeid(this).name(), "Some error on populate fields table");
//                return nullopt;
//            }

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
        } catch (const invalid_argument& ia) {
            throw runtime_error(ia.what());
        } catch (const out_of_range& oor) {
            throw runtime_error(oor.what());
        }
    }
}
//
//future<void> synchronizer::update_group_table(const struct response& response)
//{
//    promise<void> prom;
//    auto&& fut = prom.get_future();
//    pool.detach_task([this, &prom, vect = &response.groups]
//     {
//         try
//         {
//             dao dao(database);
//
//             for(auto&& it : *vect)
//             {
//                if(dao.persist<group>(it) == 0)
//                {
//                    string msg = "Persist group error id:" + to_string(it->id) + "it->server_id:" + to_string(it->server_id);
//                    error(typeid(this).name(),  msg);
//                }
//             }
//
//             prom.set_value();
//         }
//         catch (const runtime_error& e)
//         {
//             error(typeid(this).name(), e.what());
//         }
//     });
//
//    return fut;
//}
//
//future<void> synchronizer::update_group_field_table(const struct response& response)
//{
//    promise<void> prom;
//    auto&& fut = prom.get_future();
//    pool.detach_task([this, &prom, vect = &response.groups]
//    {
//        try
//        {
//            dao dao(database);
//
//            for(auto&& it : *vect)
//            {
//                if(dao.persist<group>(it) == 0)
//                {
//                    string msg = "Persist group error id:" + to_string(it->id) + "it->server_id:" + to_string(it->server_id);
//                    error(typeid(this).name(),  msg);
//                }
//            }
//
//            prom.set_value();
//        }
//        catch (const runtime_error& e)
//        {
//            error(typeid(this).name(), e.what());
//        }
//    });
//
//    return fut;
//}
//
//future<void> synchronizer::update_field_table(const struct response& response)
//{
//    promise<void> prom;
//    auto&& fut = prom.get_future();
//    pool.detach_task([this, &prom, vect = &response.groups_fields]
//     {
//         try
//         {
//             dao dao(database);
//
//             for(auto&& it : *vect)
//             {
//                 if(dao.persist<group_field>(it) == 0)
//                 {
//                     string msg = "Persist group_field error id:" + to_string(it->id) + "it->server_id:" + to_string(it->server_id);
//                     error(typeid(this).name(),  msg);
//                 }
//             }
//
//             prom.set_value();
//         }
//         catch (const runtime_error& e)
//         {
//             error(typeid(this).name(), e.what());
//         }
//     });
//
//    return fut;
//}


}

