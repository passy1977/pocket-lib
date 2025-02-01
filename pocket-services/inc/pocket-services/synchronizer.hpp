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
#include "pocket-pods/response.hpp"
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

    BS::thread_pool<6> pool;
public:
    using ptr = std::unique_ptr<synchronizer>;

    static inline constexpr uint8_t FULL_SYNC = 0;

    explicit synchronizer(services::database::ptr& database, std::string& secret, pods::device& device) noexcept
    : database(database)
    , secret(secret)
    , device(device) {}
    POCKET_NO_COPY_NO_MOVE(synchronizer)

    std::optional<pods::user::ptr> get_data(uint64_t timestamp_last_update, std::string_view email, std::string_view passwd);

private:
    struct data_server_id
    {
        std::map<uint64_t, uint64_t> groups_server_id;
        std::map<uint64_t, uint64_t> groups_fields_server_id;
        std::map<uint64_t, uint64_t> fields_server_id;
    };

    template<iface::require_pod T>
    std::future<void> update_database_table(struct pods::response& response, const std::map<uint64_t, uint64_t>& data_server_id)
    {
        std::promise<void> prom;
        auto&& fut = prom.get_future();
        pool.detach_task([this, &prom, type = std::forward<pods::response>(response), data_server_id]
         {
             try
             {

//                 if constexpr (std::is_same_v<T, pods::group>)
//                 {
//                    //for(auto&& it : response)
//                 }
//                 else if constexpr (std::is_same_v<T, pods::group_field>)
//                 {
//
//                 }
//                 else if constexpr (std::is_same_v<T, pods::field>)
//                 {
//
//                 }

                 prom.set_value();
             }
             catch (const std::runtime_error& e)
             {
                 error(typeid(this).name(), e.what());
             }
         });

        return fut;
    }

    void update_group_table(const data_server_id& data);
    void update_group_field_table(const data_server_id& data);
    void update_field_table(const data_server_id& data);

};

}