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
#include "pocket-pods/user.hpp"

#include <string_view>



namespace pocket::daos::inline v5
{

class dao_user final
{
    services::database::ptr& database;
public:

    explicit dao_user(services::database::ptr& database) noexcept;
    ~dao_user() = default;
    POCKET_NO_COPY_NO_MOVE(dao_user)

    pods::user::opt get();

    pods::user::opt login(const std::string& email, const std::string& passwd);

    bool write(const pods::user& user);

    inline bool write(const pods::user::ptr& user)
    {
        return write(*user);
    }

};

} // pocket
