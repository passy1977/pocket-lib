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

pods::user::opt_ptr synchronizer::retrieve_data(uint64_t timestamp_last_update, const std::string_view& email, const std::string_view& passwd)
{
    if(email.empty() || passwd.empty())
    {
        throw runtime_error("Some parameter are empty");
    }

    set_status(stat::BUSY);
    
    auto&& fut_data = pool.submit_task([this]
    {
       try
       {
           server_id_helper data{
                   .valid = true
           };
           dao dao(database);

           auto&& g = dao.get_all<group>();
           for_each(g.begin(), g.end(), [&data](auto&& it) mutable { data.groups_server_id[it->server_id] = it->id; });

           auto&& gf = dao.get_all<group_field>();
           for_each(gf.begin(), gf.end(), [&data](auto&& it) mutable { data.group_fields_server_id[it->server_id] = it->id; });

           auto&& f = dao.get_all<field>();
           for_each(f.begin(), f.end(), [&data](auto&& it) mutable { data.fields_server_id[it->server_id] = it->id; });

           return data;
       }

       catch (const runtime_error& e)
       {
           set_status(stat::MAP_ID_ERROR);
           error(typeid(this).name(), e.what());
           return server_id_helper{
                   .groups_server_id = {},
                   .group_fields_server_id = {},
                   .fields_server_id = {},
                   .valid = false
           };
       }

    });

    server_id_helper data = fut_data.get();

    auto&& fut_response = pool.submit_task([this, timestamp_last_update, email = email.data(), passwd = passwd.data()] () mutable
     {
         network network;
         if(timeout)
         {
             network.set_timeout(timeout);
         }

         if(connect_timeout)
         {
             network.set_connect_timeout(connect_timeout);
         }
         try
         {
             if(secret.empty())
             {
                 secret = crypto_generate_random_string(10);
             }
#ifdef POCKET_FORCE_TIMESTAMP_LAST_UPDATE
             timestamp_last_update = POCKET_FORCE_TIMESTAMP_LAST_UPDATE;
#endif
             auto crypt = crypto_encrypt_rsa(device.host_pub_key, to_string(device.id) + DIVISOR + secret  + DIVISOR + to_string(timestamp_last_update) + DIVISOR + email + DIVISOR + passwd);

             auto&& content = network.perform(network::method::GET, device.host + API_VERSION + "/" + device.uuid + "/" + crypt);
             set_status(stat{network.get_http_code()});
             return content;

         }
         catch (const runtime_error& e)
         {
             set_status(stat{network.get_http_code()});
             secret = "";
             return string(ERROR_HTTP_CODE) + e.what();
         }
     });

    try
    {

        auto&& ret = parse_data_from_net(fut_response.get(), data);

        set_status(stat::READY);

        return ret;
    }
    catch (const runtime_error& e)
    {
        set_status(stat::NO_NETWORK);
        throw;
    }

}


pods::user::opt_ptr synchronizer::send_data(const pods::user::ptr& user)
{
    if(status != stat::READY)
    {
        error(typeid(this).name(), "No network impossible send data");
        return nullopt;
    }

    auto&& fut_data = pool.submit_task([this]
    {
        try
        {
           server_id_helper data{
                   .valid = true
           };
           dao dao(database);

           auto&& g = dao.get_all<group>(daos::dao::NO_ID, true);
           for_each(g.begin(), g.end(), [&data](auto&& it) mutable { data.groups_server_id[it->server_id] = it->id; });

           auto&& gf = dao.get_all<group_field>(daos::dao::NO_ID, true);
           for_each(gf.begin(), gf.end(), [&data](auto&& it) mutable { data.group_fields_server_id[it->server_id] = it->id; });

           auto&& f = dao.get_all<field>(daos::dao::NO_ID, true);
           for_each(f.begin(), f.end(), [&data](auto&& it) mutable { data.fields_server_id[it->server_id] = it->id; });

           return data;
        }

        catch (const runtime_error& e)
        {
            set_status(stat::MAP_ID_ERROR);
           error(typeid(this).name(), e.what());
           return server_id_helper{
                   .groups_server_id = {},
                   .group_fields_server_id = {},
                   .fields_server_id = {},
                   .valid = false
           };
        }

    });

    auto&& data = fut_data.get();

    auto&& fut_response = pool.submit_task([this, email = user->email, passwd = user->passwd, timestamp_last_update = user->timestamp_last_update]() mutable
    {

        network network;
        if(timeout)
        {
            network.set_timeout(timeout);
        }

        if(connect_timeout)
        {
            network.set_connect_timeout(connect_timeout);
        }
        try
        {
            auto&& ret = pool.submit_task([this]
            {
                try
                {
                    net_helper net_helper;

                    auto&& fut_group = collect_data_table<group>();
                    net_helper.groups = fut_group.get();

                    auto&& fur_group_field = collect_data_table<group_field>();
                    net_helper.group_fields = fur_group_field.get();

                    auto&& fut_field = collect_data_table<field>();
                    net_helper.fields = fut_field.get();

                    return net_helper;
                }
                catch (const runtime_error& e)
                {
                    set_status(stat::DB_GENERIC_ERROR);
                    return net_helper{};
                }
            });

#ifdef POCKET_FORCE_TIMESTAMP_LAST_UPDATE
            timestamp_last_update = POCKET_FORCE_TIMESTAMP_LAST_UPDATE;
#endif

            auto crypt = crypto_encrypt_rsa(device.host_pub_key, to_string(device.id) + DIVISOR + secret  + DIVISOR + to_string(timestamp_last_update) + DIVISOR + email + DIVISOR + passwd) ;

            auto&& data = net_helper_serialize_json(ret.get());

            auto&& content = network.perform(network::method::POST, device.host + API_VERSION + "/" + device.uuid + "/" + crypt, {}, data);
            set_status(stat{network.get_http_code()});
            return content;
        }
        catch (const runtime_error& e)
        {
            set_status(stat{network.get_http_code()});
            secret = "";
            return string(ERROR_HTTP_CODE) + e.what();
        }
    });


    try
    {
        
        auto&& ret = parse_data_from_net(fut_response.get(), data);

        set_status(stat::READY);

        return ret;
    }
    catch (const runtime_error& e)
    {
        set_status(stat::NO_NETWORK);
        throw;
    }

}

bool synchronizer::change_passwd(const pods::user::ptr& user, const std::string_view& new_passwd, bool change_passwd_data_on_server)
{
    if(status != stat::READY)
    {
        error(typeid(this).name(), "No network impossible send data");
        return false;
    }

    if(user == nullptr)
    {
        return false;
    }

    auto&& fut_response = pool.submit_task([this, email = user->email, passwd = user->passwd, new_passwd, timestamp_last_update = user->timestamp_last_update, change_passwd_data_on_server]() mutable
   {

       network network;
       if(timeout)
       {
           network.set_timeout(timeout);
       }

       if(connect_timeout)
       {
           network.set_connect_timeout(connect_timeout);
       }

       try
       {

#ifdef POCKET_FORCE_TIMESTAMP_LAST_UPDATE
           timestamp_last_update = POCKET_FORCE_TIMESTAMP_LAST_UPDATE;
#endif
           auto crypt = crypto_encrypt_rsa(device.host_pub_key, to_string(device.id) + DIVISOR + secret  + DIVISOR + to_string(timestamp_last_update) + DIVISOR + email + DIVISOR + passwd + DIVISOR + new_passwd.data());

           auto&& content = network.perform(network::method::PUT, device.host + API_VERSION + "/" + device.uuid + "/" + crypt + "/" + to_string(change_passwd_data_on_server));
           set_status(stat{network.get_http_code()});
           return content;
       }
       catch (const runtime_error& e)
       {
           set_status(stat{network.get_http_code()});
           secret = "";
           return string(ERROR_HTTP_CODE) + e.what();
       }
   });


    try
    {
        if(parse_data_from_change_passwd(fut_response.get()))
        {
            set_status(stat::READY);

            return true;
        }
        return false;
    }
    catch (const runtime_error& e)
    {
        set_status(stat::NO_NETWORK);
        throw;
    }
}

bool synchronizer::invalidate_data(const user::ptr& user)
{
    if(status != stat::READY)
    {
        error(typeid(this).name(), "No network impossible send data");
        return false;
    }

    if(user == nullptr)
    {
        return false;
    }


    auto&& fut_response = pool.submit_task([this, email = user->email, passwd = user->passwd, timestamp_last_update = user->timestamp_last_update]() mutable
       {
           set_status(stat::BUSY);
           network network;
           if(timeout)
           {
               network.set_timeout(timeout);
           }

           if(connect_timeout)
           {
               network.set_connect_timeout(connect_timeout);
           }
           try
           {

    #ifdef POCKET_FORCE_TIMESTAMP_LAST_UPDATE
               timestamp_last_update = POCKET_FORCE_TIMESTAMP_LAST_UPDATE;
    #endif
               auto crypt = crypto_encrypt_rsa(device.host_pub_key, to_string(device.id) + DIVISOR + secret  + DIVISOR + to_string(timestamp_last_update) + DIVISOR + email + DIVISOR + passwd) ;

               network.perform(network::method::DEL, device.host + API_VERSION + "/" + device.uuid + "/" + crypt);
               set_status(stat{network.get_http_code()});
               return status;
           }
           catch (const runtime_error& e)
           {
               set_status(stat{network.get_http_code()});
               return status;
           }
       });


    try
    {
        if(fut_response.get() == stat::OK)
        {
            set_status(stat::READY);
            return true;
        }
        else
        {
            return false;
        }
    }
    catch (const runtime_error& e)
    {
        throw;
    }
}

bool synchronizer::heartbeat(const pods::user::ptr& user, uint64_t& timestamp_last_update)
{
    if(status != stat::READY)
    {
        error(typeid(this).name(), "No network impossible send data");
        return false;
    }

    if(user == nullptr)
    {
        return false;
    }

    set_status(stat::BUSY);


    auto&& fut_response = pool.submit_task([this, timestamp_last_update = timestamp_last_update] () mutable
    {
        network network;
        if(timeout)
        {
            network.set_timeout(timeout);
        }

        if(connect_timeout)
        {
            network.set_connect_timeout(connect_timeout);
        }
        try
        {
            if(secret.empty())
            {
                secret = crypto_generate_random_string(10);
            }
#ifdef POCKET_FORCE_TIMESTAMP_LAST_UPDATE
            timestamp_last_update = POCKET_FORCE_TIMESTAMP_LAST_UPDATE;
#endif
            auto crypt = crypto_encrypt_rsa(device.host_pub_key, to_string(device.id) + DIVISOR + secret  + DIVISOR + to_string(timestamp_last_update));

            auto&& content = network.perform(network::method::GET, device.host + API_VERSION + "/heartbeat" + "/" + device.uuid + "/" + crypt);
            set_status(stat{network.get_http_code()});
            return content;

        }
        catch (const runtime_error& e)
        {
            set_status(stat{network.get_http_code()});
            secret = "";
            return string(ERROR_HTTP_CODE) + e.what();
        }
    });

    try
    {
        auto response = fut_response.get();

        if(starts_with(response, ERROR_HTTP_CODE)) 
        {
            set_status(stat::TIMESTAMP_LAST_UPDATE_NOT_MATCH);
            error(typeid(this).name(), "Response:" + response);
            timestamp_last_update = 0;
            return false;
        }

        try
        {

            timestamp_last_update = json_to_timestamp(response);
        }
        catch (const runtime_error& e)
        {
            set_status(stat::JSON_PARSING_ERROR);
            error(typeid(this).name(), e.what());
            return false;
        }

        set_status(stat::READY);

        return timestamp_last_update > 0;
    }
    catch (const runtime_error& e)
    {
        timestamp_last_update = 0;
        set_status(stat::NO_NETWORK);
        throw;
    }

}


pods::user::opt_ptr synchronizer::parse_data_from_net(const std::string_view& response, server_id_helper& data)
{
    if(!response.starts_with(ERROR_HTTP_CODE))
    {
        set_status(stat::BUSY);
        try
        {

            struct net_helper net_helper;
            try
            {
                json_parse_net_helper(pool, response, net_helper);
            }
            catch (const runtime_error& e)
            {
                set_status(stat::JSON_PARSING_ERROR);
                error(typeid(this).name(), e.what());
                return nullopt;
            }

            if(net_helper.device->id != device.id)
            {
                set_status(stat::LOCAL_DEVICE_ID_NOT_MATCH);
                return nullopt;
            }

            auto&& fut_group = update_database_table<group>(net_helper.get_vector_ref<group>(), data);
            if(!fut_group)
            {
                set_status(stat::DB_GROUP_ERROR);
                error(typeid(this).name(), "Some error on populate groups table");
                return nullopt;
            }

            auto&& fut_group_field = update_database_table<group_field>(net_helper.get_vector_ref<group_field>(), data);
            if(!fut_group_field)
            {
                set_status(stat::DB_GROUP_FIELD_ERROR);
                error(typeid(this).name(), "Some error on populate group_fields table");
                return nullopt;
            }

            auto&& fut_field = update_database_table<field>(net_helper.get_vector_ref<field>(), data);
            if(!fut_field)
            {
                set_status(stat::DB_FIELD_ERROR);
                error(typeid(this).name(), "Some error on populate fields table");
                return nullopt;
            }

            timestamp_last_update = net_helper.device->timestamp_last_update;

            set_status(stat::READY);
            return {std::move(net_helper.user) };
        }
        catch (const runtime_error& e)
        {
            set_status(stat::ERROR);
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
            cerr << "Response:" << response << " Unhandled exception" << endl;

            auto e_ptr = current_exception();

            if (e_ptr)
            {
                try
                {
                    rethrow_exception(e_ptr);
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

bool synchronizer::parse_data_from_change_passwd(const string_view& response)
{
    return response == "true";
}


}

