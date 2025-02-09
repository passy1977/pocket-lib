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
#include "pocket-services/synchronizer.hpp"
#include "pocket-services/crypto.hpp"
#include "pocket-pods/user.hpp"
#include "pocket-pods/device.hpp"
#include "pocket/globals.hpp"
#include "pocket-views/view.hpp"

#include <optional>

namespace pocket::controllers::inline v5
{

class session final
{


    controllers::config::ptr config = nullptr;
    services::database::ptr database = nullptr;
    services::synchronizer::ptr synchronizer = nullptr;

    views::view<pods::group>::ptr view_group = nullptr;
    views::view<pods::group_field>::ptr view_group_field = nullptr;
    views::view<pods::field>::ptr view_field = nullptr;

    std::string secret;
    pods::device::opt device;


public:
    explicit session(const std::optional<std::string>& config_json, const std::optional<std::string>& config_path = {});
    ~session();
    POCKET_NO_COPY_NO_MOVE(session)

    const pods::device::opt& init();

    std::optional<pods::user::ptr> login(const std::string& email, const std::string& passwd);

    std::optional<pods::user::ptr> retrieve_data(const std::optional<pods::user::ptr>& user_opt);

    bool send_data(const std::optional<pods::user::ptr>& user);

    inline const views::view<pods::group>::ptr& get_view_group() const noexcept
    {
        return view_group;
    }

    inline const views::view<pods::group_field>::ptr& get_view_group_field() const noexcept
    {
        return view_group_field;
    }

    inline const views::view<pods::field>::ptr& get_view_field() const noexcept
    {
        return view_field;
    }
private:
    void lock();

    void unlock();

    bool check_lock();
};

}