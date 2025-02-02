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

    template<iface::require_pod T>
    std::future<bool> update_database_table(const std::vector<T*> vect)
    {
        return pool.submit_task([this, vect]
         {
             try
             {

                 daos::dao dao(database);
                 for(auto&& it : vect)
                 {
                     if(dao.persist<T>(std::make_unique<T>(*it)) == 0)
                     {
                         std::string msg = "Persist error for " + T::get_name() + " id:" + std::to_string(it->id) + "it->server_id:" + std::to_string(it->server_id);
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

};

}