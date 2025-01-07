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

#include "pocket-controllers/config.hpp"

#include <nlohmann/json.hpp>
#include <filesystem>
#include <cstdlib>
#include <stdexcept>

namespace pocket::controllers::inline v5
{

using pods::device;
using nlohmann::json;
using namespace std;
using namespace std::filesystem;


config::config(const optional<string>& config_path)
{
    string&& absolute_path = config_path.value_or(getenv("HOME"));
    if(absolute_path.empty())
    {
        throw runtime_error("Impossible get HOME env");
    }

    if(!absolute_path.ends_with(path::preferred_separator))
    {
        absolute_path += path::preferred_separator;
    }

    absolute_path += DATA_FOLDER;

    if(!is_directory(absolute_path))
    {
        try
        {
            if(create_directories(absolute_path))
            {
                info(typeid(*this).name(), "Create new folder:" + absolute_path);
            }
        }
        catch (const exception& e)
        {
            throw runtime_error(e.what());
        }
    }

    this->config_path = absolute_path;
}

device config::parse(const string& config_json)
{

    auto&& json = json::parse(config_json);
    if (!json.is_object())
    {
        throw runtime_error("Config json is not a object");
    }

    device device;

    if(json.contains("uuid") && json["uuid"].is_string())
    {
        device.uuid = json["uuid"];
    }
    else
    {
        throw runtime_error("Invalid type or non defined field uuid");
    }

    if(json.contains("user_uuid") && json["user_uuid"].is_string())
    {
        device.user_uuid = json["user_uuid"];
    }
    else
    {
        throw runtime_error("Invalid type or non defined field user_uuid");
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

    debug(typeid(*this).name(), "Create new config");

    return device;
}



}

