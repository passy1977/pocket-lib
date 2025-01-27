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

#include <pocket-iface/pod.hpp>

#include <cinttypes>
#include <optional>

namespace pocket::iface::inline v5
{

template<typename PR, typename RW, require_pod T>
struct read_write
{
    virtual ~read_write() = default;

    virtual std::optional<T> read(PR& row) = 0;
    virtual RW write(const T::ptr& t) = 0;

};

}