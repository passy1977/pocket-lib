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
#include <cinttypes>
#include <memory>

namespace pocket::iface::inline v5
{

template <typename T>
struct synchronizable
{
    using ptr = std::unique_ptr<T>;

    int64_t id = 0;
    int64_t server_id = 0;
    int64_t user_id = 0;
    bool synchronized{true};
    bool deleted{false};
    uint64_t timestamp_creation = 0;

    virtual ~synchronizable() = default;
};

}
