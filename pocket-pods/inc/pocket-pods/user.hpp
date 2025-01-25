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

#include <memory>
#include <string>
#include <optional>

namespace pocket::pods::inline v5
{

struct user final
{
    using opt = std::optional<user>;
    using ptr = std::unique_ptr<user>;

    enum class status {
        NOT_ACTIVE = 1,
        ACTIVE = 0,
        DELETED = 2,
        INVALIDATED = 3
    };

    uint64_t id = 0;
    uint64_t server_id;
    std::string name;
    std::string email;
    std::string passwd;
    status status = status::NOT_ACTIVE;
    uint64_t timestamp_last_update = 0;

    ~user() = default;
};

}