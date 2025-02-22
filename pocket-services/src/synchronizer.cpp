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
#include "pocket/globals.hpp"

#include <stdexcept>
#include <ranges>

namespace pocket::services::inline v5
{

using namespace std;
using namespace pods;
using namespace daos;

namespace
{
constexpr char ERROR_HTTP_CODE[] = "http_code: ";
}

std::optional<pods::user::ptr> synchronizer::retrieve_data(int64_t timestamp_last_update, const std::string_view& email, const std::string_view& passwd)
{
    if(email.empty() || passwd.empty())
    {
        throw runtime_error("Some parameter are empty");
    }

    auto&& fut_data = pool.submit_task([this]
    {
       try
       {
           data_server_id data{
                   .valid = true
           };
           dao dao(database);

           auto&& g = dao.get_all<group>();
           for_each(g.begin(), g.end(), [&data](auto&& it) mutable { data.groups_server_id[it->server_id] = it->id; });

           auto&& gf = dao.get_all<group_field>();
           for_each(gf.begin(), gf.end(), [&data](auto&& it) mutable { data.groups_fields_server_id[it->server_id] = it->id; });

           auto&& f = dao.get_all<field>();
           for_each(f.begin(), f.end(), [&data](auto&& it) mutable { data.fields_server_id[it->server_id] = it->id; });

           return data;
       }

       catch (const runtime_error& e)
       {
           network_login = false;
           error(typeid(this).name(), e.what());
           return data_server_id{
                   .groups_server_id = {},
                   .groups_fields_server_id = {},
                   .fields_server_id = {},
                   .valid = false
           };
       }

    });


    auto&& fut_response = pool.submit_task([this, timestamp_last_update, email = email.data(), passwd = passwd.data()] () mutable
     {
         network network;
         try
         {
             if(secret.empty())
             {
                 secret = crypto_generate_random_string(10);
             }
#ifdef FORCE_TIMESTAMP_LAST_UPDATE
             timestamp_last_update = FORCE_TIMESTAMP_LAST_UPDATE;
#endif
             auto crypt = crypto_encrypt_rsa(device.host_pub_key, to_string(device.id) + DIVISOR + secret  + DIVISOR + to_string(timestamp_last_update) + DIVISOR + email + DIVISOR + passwd);

             auto&& content = network.perform(network::method::GET, device.host + API_VERSION + "/" + device.uuid + "/" + crypt);
             http_code = network.get_http_code();
             return content;

         }
         catch (const runtime_error& e)
         {
             http_code = network.get_http_code();
             network_login = false;
             secret = "";
             return string(ERROR_HTTP_CODE) + e.what();
         }
     });

    try
    {
        network_login = true;
        data_server_id data = fut_data.get();
        return parse_data_from_net(fut_response.get(), data);
    }
    catch (const runtime_error& e)
    {
        network_login = false;
        throw;
    }

}


bool synchronizer::send_data(const pods::user::ptr& user)
{
//    if(device.id == 0 || secret.empty())
//    {
//        throw runtime_error("Seems no one has been logged");
//    }
//
    if(!network_login)
    {
        error(typeid(this).name(), "No network login impossible send data");
        return false;
    }

    auto&& fut_data = pool.submit_task([this]
    {
        try
        {
           data_server_id data{
                   .valid = true
           };
           dao dao(database);

           auto&& g = dao.get_all<group>(true);
           for_each(g.begin(), g.end(), [&data](auto&& it) mutable { data.groups_server_id[it->server_id] = it->id; });

           auto&& gf = dao.get_all<group_field>(true);
           for_each(gf.begin(), gf.end(), [&data](auto&& it) mutable { data.groups_fields_server_id[it->server_id] = it->id; });

           auto&& f = dao.get_all<field>(true);
           for_each(f.begin(), f.end(), [&data](auto&& it) mutable { data.fields_server_id[it->server_id] = it->id; });

           return data;
        }

        catch (const runtime_error& e)
        {
           error(typeid(this).name(), e.what());
           return data_server_id{
                   .groups_server_id = {},
                   .groups_fields_server_id = {},
                   .fields_server_id = {},
                   .valid = false
           };
        }

    });


    auto&& fut_response = pool.submit_task([this, email = user->email, passwd = user->passwd, timestamp_last_update = user->timestamp_last_update]() mutable
    {

        network network;
        try
        {
            auto&& ret = pool.submit_task([this]
            {
                try
                {
                    net_transport net_transport;

                    auto&& fut_group = collect_data_table<group>();
                    net_transport.groups = fut_group.get();

                    auto&& fur_group_field = collect_data_table<group_field>();
                    net_transport.groups_fields = fur_group_field.get();

                    auto&& fut_field = collect_data_table<field>();
                    net_transport.fields = fut_field.get();

                    return net_transport;
                }
                catch (const runtime_error& e)
                {
                    return net_transport{};
                }
            });

#ifdef FORCE_TIMESTAMP_LAST_UPDATE
            timestamp_last_update = FORCE_TIMESTAMP_LAST_UPDATE;
#endif

            //auto crypt = crypto_encrypt_rsa(device.host_pub_key, to_string(device.id) + DIVISOR + secret + DIVISOR + to_string(timestamp_last_update) + DIVISOR + to_string(id));
            auto crypt = crypto_encrypt_rsa(device.host_pub_key, to_string(device.id) + DIVISOR + secret  + DIVISOR + to_string(timestamp_last_update) + DIVISOR + email + DIVISOR + passwd) ;

            auto&& data = net_transport_serialize_json(ret.get());

            auto&& content = network.perform(network::method::POST, device.host + API_VERSION + "/" + device.uuid + "/" + crypt, {}, data);
            http_code = network.get_http_code();
            return content;
        }
        catch (const runtime_error& e)
        {
            http_code = network.get_http_code();
            secret = "";
            return string(ERROR_HTTP_CODE) + e.what();
        }
    });


    try
    {
        data_server_id data = fut_data.get();
        return parse_data_from_net(fut_response.get(), data).has_value();
    }
    catch (const runtime_error& e)
    {
        throw;
    }

}

std::optional<pods::user::ptr> synchronizer::parse_data_from_net(const std::string_view& response, data_server_id& data)
{
    if(!response.starts_with(ERROR_HTTP_CODE))
    {
        try
        {

            struct net_transport net_transport;
            try
            {
                json_parse_net_transport(pool, response, net_transport);
            }
            catch (const runtime_error& e)
            {
                error(typeid(this).name(), e.what());
                return nullopt;
            }

            if(net_transport.device->id != device.id)
            {
                return nullopt;
            }

            auto&& fut_group = update_database_table<group>(net_transport.get_vector_ref<group>(), data);
            if(!fut_group.get())
            {
                error(typeid(this).name(), "Some error on populate groups table");
                return nullopt;
            }

            auto&& fut_group_field = update_database_table<group_field>(net_transport.get_vector_ref<group_field>(), data);
            if(!fut_group_field.get())
            {
                error(typeid(this).name(), "Some error on populate groups_fields table");
                return nullopt;
            }

            auto&& fut_field = update_database_table<field>(net_transport.get_vector_ref<field>(), data);
            if(!fut_field.get())
            {
                error(typeid(this).name(), "Some error on populate fields table");
                return nullopt;
            }

            dao{database}.update_all_index(net_transport);

            return {std::move(net_transport.user) };
        }
        catch (const runtime_error& e)
        {
            throw;
        }
    }
    else
    {
        try
        {
            int http_code = stoi(response.substr(strnlen(ERROR_HTTP_CODE, sizeof ERROR_HTTP_CODE)).data());
            if(http_code >= 600)
            {
                error(typeid(this).name(), "Server error http_code:" + to_string(http_code));
                return nullopt;
            }
            else
            {
                throw runtime_error(response.data());
            }
        }
        catch (const invalid_argument& e)
        {
            throw runtime_error(e.what());
        }
        catch (const out_of_range& e)
        {
            throw runtime_error(e.what());
        }
        catch (...)
        {
            cerr << "Unhandled exception" << endl;

            auto eptr = current_exception();

            if (eptr)
            {
                try
                {
                    rethrow_exception(eptr);
                }
                catch (const runtime_error& e)
                {
                    throw ;
                }
            }
        }
    }
    return nullopt;
}

}

