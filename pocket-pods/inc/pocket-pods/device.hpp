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

#include "pocket-iface/synchronizable.hpp"

#include <memory>
#include <optional>
#include <string>


namespace pocket::pods::inline v5
{

struct device final
{
    using opt = std::optional<device>;
    using ptr = std::unique_ptr<device>;

    enum class stat {
        NOT_ACTIVE = 1,
        ACTIVE = 0,
        DELETED = 2,
        INVALIDATED = 3
    };

    int64_t id = 0;
    int64_t user_id = 0;
    std::string uuid;
    std::string version;
    std::string token;
    std::string host;
    std::string host_pub_key;
    int64_t timestamp_creation = 0;
    stat status = stat::NOT_ACTIVE;

};

} // pocket
