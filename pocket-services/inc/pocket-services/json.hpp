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
#include "pocket-pods/response.hpp"
#include "BS_thread_pool.hpp"

#include <nlohmann/json.hpp>
#include <optional>


namespace pocket::services::inline v5
{

void json_parse_response(BS::thread_pool<4>& pool, std::string_view response, pods::response& json_response);

pods::device json_to_device(const nlohmann::basic_json<>& json, uint64_t& user_timestamp_last_update);

pods::device json_to_device(const std::string_view& str_json, uint64_t& user_timestamp_last_update);

pods::user json_to_user(const nlohmann::basic_json<>& json);

pods::user json_to_user(const std::string_view& str_json);

pods::group json_to_group(const std::string_view& str_json);

pods::group json_to_group(const nlohmann::basic_json<>& json);

pods::group_field json_to_group_field(const std::string_view& str_json);

pods::group_field json_to_group_field(const nlohmann::basic_json<>& json);

pods::field json_to_field(const std::string_view& str_json);

pods::field json_to_field(const nlohmann::basic_json<>& json);

}