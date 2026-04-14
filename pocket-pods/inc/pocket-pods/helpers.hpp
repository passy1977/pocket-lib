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

#include "pocket-pods/user.hpp"
#include "pocket-pods/device.hpp"
#include "pocket-pods/group.hpp"
#include "pocket-pods/group-field.hpp"
#include "pocket-pods/field.hpp"
#include "pocket/globals.hpp"

#include <vector>
#include <type_traits>
#include <map>

namespace pocket::pods::inline v5
{

template<typename T>
void vector_copy_ref(const std::vector<typename T::ptr>& src, std::vector<T*>& dst) noexcept
{
    for (auto& ptr : src)
    {
        if (ptr)
        {
            dst.push_back(ptr.get());
        }
    }
}

struct net_helper
{
    uint64_t timestamp_last_update = 0;
    pods::user::ptr user;
    pods::device::ptr device;
    std::vector<group::ptr> groups;
    std::vector<group_field::ptr> group_fields;
    std::vector<field::ptr> fields;

    template<iface::require_pod T>
    constexpr inline std::vector<T*> get_vector_ref() noexcept
    {
        std::vector<T*> ret;
        if constexpr(std::is_same_v<T, group>)
        {
            vector_copy_ref<group>(groups, ret);
        }
        else if constexpr(std::is_same_v<T, group_field>)
        {
            vector_copy_ref<group_field>(group_fields, ret);
        }
        if constexpr(std::is_same_v<T, field>)
        {
            vector_copy_ref<field>(fields, ret);
        }
        return ret;
    }

};

struct server_id_helper
{
    std::map<int64_t, int64_t> groups_server_id; //server_id/id
    std::map<int64_t, int64_t> group_fields_server_id; //server_id/id
    std::map<int64_t, int64_t> fields_server_id; //server_id/id
    bool valid;
};

}
