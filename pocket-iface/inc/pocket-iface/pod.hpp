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
#include "pocket-iface/has-name.hpp"
#include "pocket-iface/synchronizable.hpp"

#include <memory>

namespace pocket::iface::inline v5
{

template<typename T>
struct pod : public has_name, public synchronizable<T>
{
    using ptr = std::unique_ptr<T>;

    ~pod() override = default;

};

template<typename T>
concept require_pod = requires {
    typename iface::pod<T>;
};


}