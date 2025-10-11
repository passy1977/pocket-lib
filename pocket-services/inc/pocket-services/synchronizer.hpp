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
#include "pocket-services/database.hpp"
#include "pocket-pods/device.hpp"
#include "pocket-pods/user.hpp"
#include "pocket-pods/helpers.hpp"
#include "pocket-daos/dao.hpp"
#include "BS_thread_pool.hpp"

#include <optional>
#include <string_view>

namespace pocket::services::inline v5
{

class synchronizer final
{
    services::database::ptr& database;
    std::string& secret;
    pods::device& device;

    long timeout = 0;
    long connect_timeout = 0;

    BS::thread_pool<> pool{6};
public:
    enum class stat : uint64_t
    {
        READY = 0,
        BUSY,
        USER_NOT_FOUND = 600,
        WRONG_SIZE_TOKEN = 601,
        DEVICE_ID_NOT_MATCH = 602,
        DEVICE_NOT_FOUND = 603,
        SECRET_NOT_MATCH = 604,
        PASSWD_ERROR = 605,
        TIMESTAMP_LAST_UPDATE_NOT_MATCH = 606,
        CACHE_NOT_FOND = 607,
        SECRET_EMPTY = 608,
        TIMESTAMP_LAST_NOT_PARSABLE = 609,
        ERROR = USER_NOT_FOUND + 100,
        JSON_PARSING_ERROR = USER_NOT_FOUND + 100 + 1,
        DB_GROUP_ERROR = USER_NOT_FOUND + 100 + 2,
        DB_GROUP_FIELD_ERROR = USER_NOT_FOUND + 100 + 3,
        DB_FIELD_ERROR = USER_NOT_FOUND + 100 + 4,
        DB_GENERIC_ERROR = USER_NOT_FOUND + 100 + 5,
        NO_NETWORK = USER_NOT_FOUND + 100 + 6,
        MAP_ID_ERROR = USER_NOT_FOUND + 100 + 7,
        LOCAL_DEVICE_ID_NOT_MATCH = DEVICE_ID_NOT_MATCH + 200,
        OK = 200
    };
    
    using ptr = std::unique_ptr<synchronizer>;

    static inline constexpr uint8_t FULL_SYNC = 0;
    static inline constexpr uint8_t EMAIL_MAX_SIZE = 32;
    static inline constexpr uint8_t PASSWD_MAX_SIZE = 32;

    explicit synchronizer(services::database::ptr& database, std::string& secret, pods::device& device) noexcept
    : database(database)
    , secret(secret)
    , device(device)
    {}
    POCKET_NO_COPY_NO_MOVE(synchronizer)

    pods::user::opt_ptr retrieve_data(int64_t timestamp_last_update, const std::string_view& email, const std::string_view& passwd);

    pods::user::opt_ptr send_data(const pods::user::ptr& user);

    bool change_passwd(const pods::user::ptr& user, const std::string_view& new_passwd, bool change_passwd_data_on_server);

    bool invalidate_data(const pods::user::ptr& user);

    bool heartbeat(const pods::user::ptr& user, uint64_t& timestamp_last_update);

    inline void set_status(stat status) noexcept
    {
        if(status == stat::NO_NETWORK)
        {
            no_network = true;
        }
        synchronizer::status = status;
    }

    inline const stat* get_status() const noexcept
    {
        return &status;
    }

    inline bool is_no_network() const noexcept
    {
        return no_network;
    }

    inline void set_timeout(long timeout) noexcept
    {
        synchronizer::timeout = timeout;
    }

    inline void set_connect_timeout(long connect_timeout) noexcept
    {
        synchronizer::connect_timeout = connect_timeout;
    }

private:
    stat status = stat::READY;
    bool no_network = false;

    pods::user::opt_ptr parse_data_from_net(const std::string_view& response, pods::server_id_helper& data);

    bool parse_data_from_change_passwd(const std::string_view& response);

    template<iface::require_pod T>
    bool update_database_table(const std::vector<T*> vect, pods::server_id_helper& data) try
    {
     daos::dao dao(database);
     for(auto&& it : vect)
     {
         it->user_id = device.user_id;
         it->synchronized = 1;
         if constexpr (std::is_same_v<T, pods::group>)
         {
            if(data.groups_server_id.contains(it->server_id))
            {
                it->id = data.groups_server_id[it->server_id];
            }
         }
         else if constexpr (std::is_same_v<T, pods::group_field>)
         {
             if(data.group_fields_server_id.contains(it->server_id))
             {
                 it->id = data.group_fields_server_id[it->server_id];
             }
         }
         else if constexpr (std::is_same_v<T, pods::field>)
         {
             if(data.fields_server_id.contains(it->server_id))
             {
                 it->id = data.fields_server_id[it->server_id];
             }
         }

         if(it->deleted)
         {
             
            if(it->id && dao.rm<T>(it->id) == 0)
            {
                std::string msg = "Remove error for " + T::get_name() + " id:" + std::to_string(it->id) + " it->server_id:" + std::to_string(it->server_id);
                error(typeid(this).name(),  msg);
                return false;
            }
            else
            {
                continue;
            }
         }
         else if(auto last_id = dao.persist<T>(std::make_unique<T>(*it), false); last_id == 0)
         {
             std::string msg = "Persist error for " + T::get_name() + " id:" + std::to_string(it->id) + " it->server_id:" + std::to_string(it->server_id);
             error(typeid(this).name(),  msg);
             return false;
         }
         else
         {
             it->id = last_id;
             if constexpr (std::is_same_v<T, pods::group>)
             {
                if(!data.groups_server_id.contains(it->server_id))
                {
                    data.groups_server_id[it->server_id] = last_id;
                }
             }
             else if constexpr (std::is_same_v<T, pods::group_field>)
             {
                 if(!data.group_fields_server_id.contains(it->server_id))
                 {
                     data.group_fields_server_id[it->server_id] = last_id;
                 }
             }
             else if constexpr (std::is_same_v<T, pods::field>)
             {
                 if(!data.fields_server_id.contains(it->server_id))
                 {
                     data.fields_server_id[it->server_id] = last_id;
                 }
             }
         }
     }
     
     for(auto&& it: vect)
     {
         if(it->deleted)
         {
             continue;
         }
         
         bool perform_persist = false;
         

         if(it->group_id == 0 && it->server_group_id > 0)
         {
             if(data.groups_server_id.contains(it->server_group_id))
             {
                 it->group_id = data.groups_server_id[it->server_group_id];
                 perform_persist = true;
             }
         }

         if(it->group_id > 0 && it->server_group_id == 0)
         {
             if(auto&& g = dao.get<T>(it->group_id); g)
             {
                 it->server_group_id = g.value()->server_id;
                 perform_persist = true;
             }
         }
         
         if constexpr (std::is_same_v<T, pods::field>)
         {
             if(it->group_id == 0 && it->server_group_id > 0)
             {
                 if(data.group_fields_server_id.contains(it->server_id))
                 {
                     it->server_group_field_id = data.group_fields_server_id[it->server_group_field_id];
                     perform_persist = true;
                 }
             }
             
             if(it->group_field_id > 0 && it->server_group_field_id == 0)
             {
                 if(auto&& g = dao.get<pods::group_field>(it->group_field_id); g)
                 {
                     it->server_group_field_id = g.value()->server_id;
                     perform_persist = true;
                 }
             }
         }
         
         if(perform_persist)
         {
             dao.persist<T>(std::make_unique<T>(*it));
         }

     }

     return true;
}
catch (const std::runtime_error& e)
{
     error(typeid(this).name(), e.what());
     return false;
}


    template<iface::require_pod T>
    std::future<std::vector<typename T::ptr>> collect_data_table()
    {
        return pool.submit_task([this]
        {
                return daos::dao(database).get_all<T>(daos::dao::NO_ID, true);
        });
    }

};

}
