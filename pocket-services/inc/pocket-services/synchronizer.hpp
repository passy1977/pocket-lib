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
#include "pocket-pods/net-transport.hpp"
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
    bool network_login = false;
    uint64_t http_code = 0;
    
    BS::thread_pool<6> pool;
public:
    using ptr = std::unique_ptr<synchronizer>;

    static inline constexpr uint8_t FULL_SYNC = 0;
    static inline constexpr uint8_t EMAIL_MAX_SIZE = 32;
    static inline constexpr uint8_t PASSWD_MAX_SIZE = 32;

    explicit synchronizer(services::database::ptr& database, std::string& secret, pods::device& device) noexcept
    : database(database)
    , secret(secret)
    , device(device) {}
    POCKET_NO_COPY_NO_MOVE(synchronizer)

    std::optional<pods::user::ptr> retrieve_data(int64_t timestamp_last_update, const std::string_view& email, const std::string_view& passwd);

    bool send_data(const pods::user::ptr& user);
    
    inline uint64_t get_http_code() const noexcept
    {
        return http_code;
    }
private:
struct data_server_id
    {
        std::map<int64_t, int64_t> groups_server_id;
        std::map<int64_t, int64_t> groups_fields_server_id;
        std::map<int64_t, int64_t> fields_server_id;
        bool valid;
    };

    std::optional<pods::user::ptr> parse_data_from_net(const std::string_view& response, data_server_id& data);

    template<iface::require_pod T>
    std::future<bool> update_database_table(const std::vector<T*> vect, data_server_id& data)
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
                        if(dao.rm<T>(it->server_id) == 0)
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
                return daos::dao(database).get_all<T>(true);
        });
    }

};

}
