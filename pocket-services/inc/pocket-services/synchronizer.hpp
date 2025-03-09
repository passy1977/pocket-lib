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
        USER_ID_NOT_MATCH = 605,
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

    std::optional<pods::user::ptr> retrieve_data(int64_t timestamp_last_update, const std::string_view& email, const std::string_view& passwd);

    bool send_data(const pods::user::ptr& user);

    bool invalidate_data(const pods::user::ptr& user);

    inline void set_status(stat status) noexcept
    {
        synchronizer::status = status;
    }

    inline const stat* set_status() const noexcept
    {
        return &status;
    }

private:
    stat status = stat::READY;

    std::optional<pods::user::ptr> parse_data_from_net(const std::string_view& response, pods::server_id_helper& data);

    template<iface::require_pod T>
    std::future<bool> update_database_table(const std::vector<T*> vect, pods::server_id_helper& data)
    {
        return pool.submit_task([this, vect, data]() mutable
         {
             try
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
                         if(data.groups_fields_server_id.contains(it->server_id))
                         {
                             it->id = data.groups_fields_server_id[it->server_id];
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
                        if(dao.rm<T>(it->id) == 0)
                        {
                            std::string msg = "Remove error for " + T::get_name() + " id:" + std::to_string(it->id) + " it->server_id:" + std::to_string(it->server_id);
                            error(typeid(this).name(),  msg);
                            return false;
                        }
                     }
                     else if(dao.persist<T>(std::make_unique<T>(*it)) == 0)
                     {
                         std::string msg = "Persist error for " + T::get_name() + " id:" + std::to_string(it->id) + " it->server_id:" + std::to_string(it->server_id);
                         error(typeid(this).name(),  msg);
                         return false;
                     }
                 }
                 return true;
             }
             catch (const std::runtime_error& e)
             {
                 error(typeid(this).name(), e.what());
                 return false;
             }
         });
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
