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

#include "pocket-controllers/config.hpp"
#include "pocket-services/database.hpp"
#include "pocket-pods/user.hpp"
#include "pocket-pods/device.hpp"
#include "pocket/globals.hpp"

#include "BS_thread_pool.hpp"

#include <optional>

namespace pocket::controllers::inline v5
{

class session final
{


    controllers::config::ptr config = nullptr;
    services::database::ptr database = nullptr;


    pods::device::opt device;
    pods::user::opt user;

    BS::thread_pool<4> pool;
    BS::synced_stream sync_out;
public:
    explicit session(const std::optional<std::string>& config_json, const std::optional<std::string>& config_path = {});
    ~session();
    POCKET_NO_COPY_NO_MOVE(session)

    const pods::device::opt& init();

    const pods::user::opt& login(const std::string& user, const std::string& passwd);

};

}