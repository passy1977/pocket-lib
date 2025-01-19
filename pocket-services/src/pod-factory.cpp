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

namespace
{
constexpr char APP_TAG[] = "pod-factory";
}

using namespace std;
using namespace nlohmann;
using iface::synchronizable;
using pods::device;
using pods::user;

device factory_from_json_to_device(const std::string& str_json)
{


    if(str_json.empty())
    {
        throw runtime_error("String json empty");
    }

    auto&& json = json::parse(str_json);
    if (!json.is_object())
    {
        throw runtime_error("Config json is not a object");
    }

    device device;

    if(json.contains("user_id") && json["user_id"].is_number())
    {
        device.user_id = json["user_id"];
    }
    else
    {
        throw runtime_error("Invalid type or non defined field user_id");
    }

    if(json.contains("uuid") && json["uuid"].is_string())
    {
        device.uuid = json["uuid"];
    }
    else
    {
        throw runtime_error("Invalid type or non defined field uuid");
    }

    if(json.contains("host") && json["host"].is_string())
    {
        device.host = json["host"];
    }
    else
    {
        throw runtime_error("Invalid type or non defined field host");
    }

    if(json.contains("host_pub_key") && json["host_pub_key"].is_string())
    {
        device.host_pub_key = json["host_pub_key"];
    }
    else
    {
        throw runtime_error("Invalid type or non defined field host_pub_key");
    }

    if(json.contains("timestamp_last_update") && json["timestamp_last_update"].is_number())
    {
        device.timestamp_last_update = json["timestamp_last_update"];
    }

    if(json.contains("timestamp_creation") && json["timestamp_creation"].is_string())
    {
        device.timestamp_creation = json["timestamp_creation"];
    }

    if(json.contains("status") && json["status"].is_string())
    {
        device.status = json["status"];
    }
    else
    {
        device.status = pods::device::status::INACTIVE;
    }

    return device;
}

user factory_from_json_to_user(const std::string& str_json)
{

    if(str_json.empty())
    {
        throw runtime_error("String json empty");
    }

    auto&& json = json::parse(str_json);
    if (!json.is_object())
    {
        throw runtime_error("Config json is not a object");
    }

    user user;

    if(json.contains("name") && json["name"].is_string())
    {
        user.name = json["name"];
    }
    else
    {
        throw runtime_error("Invalid type or non defined field name");
    }

    if(json.contains("email") && json["email"].is_string())
    {
        user.email = json["email"];
    }
    else
    {
        throw runtime_error("Invalid type or non defined field email");
    }

    if(json.contains("status") && json["status"].is_string())
    {
        user.status = json["status"];
    }
    else
    {
        user.status = pods::user::status::INACTIVE;
    }

    return user;
}

}