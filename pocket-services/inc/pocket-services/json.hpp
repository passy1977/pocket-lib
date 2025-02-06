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
#include "pocket-pods/net-transport.hpp"
#include "BS_thread_pool.hpp"

#include <nlohmann/json.hpp>
#include <optional>


namespace pocket::services::inline v5
{

void json_parse_net_transport(BS::thread_pool<6>& pool, std::string_view json_response, pods::net_transport& net_transport);

std::optional<std::string> net_transport_serialize_json(BS::thread_pool<6>& pool, const pods::net_transport& net_transport);

pods::device json_to_device(const nlohmann::json& json);

pods::user json_to_user(const nlohmann::json& json);

}