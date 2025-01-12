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
#include "pocket-iface/synchronizable.hpp"
#include "pocket-pods/device.hpp"
#include "pocket-pods/user.hpp"


#include <optional>

namespace pocket::services::inline v5
{

//template <typename T>
//concept C = requires(T x) {
//    // Requirements on type `T`:
//    {*x} -> std::convertible_to<typename T::inner>;
//    typename T::id;
//    typename T::server_id;
//    typename T::user_id;
//};
//
//
//template <iface::synchronizable F>
//iface::synchronizable::ptr factory_from_json_to_device(std::optional<std::string> str_json);
//
//}

pods::device factory_from_json_to_device(const std::string& str_json);

pods::user factory_from_json_to_user(const std::string& str_json);

}