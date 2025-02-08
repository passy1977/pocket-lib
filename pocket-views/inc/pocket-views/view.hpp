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
#include "pocket-iface/pod.hpp"
#include "pocket-views/view-group.hpp"
#include "pocket-views/view-group-field.hpp"
#include "pocket-views/view-field.hpp"
#include "pocket-pods/user.hpp"
#include "pocket-services/crypto.hpp"
#include "pocket-services/database.hpp"


namespace pocket::views::inline v5
{

template<iface::require_pod T>
class view final
{

    services::aes aes;
    services::database::ptr& database;
public:
    using ptr = std::unique_ptr<view>;

    explicit view(const pods::user::ptr &user, services::database::ptr& database) noexcept
    : aes(POCKET_AES_CBC_IV, user->passwd)
    , database(database)
    {

    }

    POCKET_NO_COPY_NO_MOVE(view)
    ~view() = default;

};


} // pocket
