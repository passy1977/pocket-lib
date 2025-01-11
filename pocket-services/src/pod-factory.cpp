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

#include "pocket-services/pod-factory.hpp"

#include <nlohmann/json.hpp>

namespace pocket::services::inline v5
{

using namespace std;
using namespace nlohmann;
using iface::pod;

pod::ptr factory_from_json(optional<string> str_json)
{
//    auto&& json = json::parse(str_json);
//    if (!json.is_object())
//    {
//        throw runtime_error("Config json is not a object");
//    }

    return {};
}

}