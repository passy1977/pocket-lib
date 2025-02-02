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

#include "pocket-iface/pod.hpp"

namespace pocket::pods::inline v5
{

struct field final : public iface::pod<field>
{

    uint64_t group_id{0};
    uint64_t server_group_id{0};
    uint64_t group_field_id{0};
    uint64_t server_group_field_id{0};
    std::string title;
    std::string value;
    bool is_hidden{false};
    bool synchronized{true};
    bool deleted{false};
    uint64_t timestamp_creation = 0;

    ~field() override = default;

    static inline const std::string& get_name() noexcept {
        static std::string const ret = "fields";
        return ret;
    }

};

} // pocket
