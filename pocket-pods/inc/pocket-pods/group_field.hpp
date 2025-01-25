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

struct group_field final : public iface::pod<group_field>
{

    uint64_t group_id{0};
    std::string title{""};
    bool is_hidden{false};
    bool synchronized{true};
    bool deleted{false};
    uint64_t timestamp_last_update = 0;
    uint64_t timestamp_creation = 0;

    ~group_field() override;

    inline const std::string& get_base_path() const noexcept override {
        static std::string const ret = "group_field";
        return ret;
    }

};

} // pocket
