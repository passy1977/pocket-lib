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
#include "pocket-services/json.hpp"

#include <nlohmann/json.hpp>
#include <filesystem>
#include <cstdlib>
#include <stdexcept>

namespace pocket::controllers::inline v5
{

using pods::device;
using services::json_to_device;
using services::json_to_aes_cbc_iv;
using services::json_to_cors_header_token;
using nlohmann::json;
using namespace std;
using namespace std::filesystem;


config::config(const optional<string>& config_path) try
{
    string&& absolute_path = config_path.value_or(getenv("HOME"));
    if(absolute_path.empty())
    {
        throw runtime_error("Impossible get HOME env");
    }

    if(absolute_path.starts_with("file://"))
    {
        absolute_path = &absolute_path[7];
    }
    
    
    if(!absolute_path.ends_with(path::preferred_separator))
    {
        absolute_path += path::preferred_separator;
    }

    absolute_path += DATA_FOLDER;

//    if(absolute_path.empty())
//    {
    if(!exists(absolute_path))
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
//    }

    this->config_path = absolute_path;
}
catch (const nlohmann::detail::parse_error& e)
{
    throw runtime_error(e.what());
}

tuple<device, string, string> config::parse(const string_view& config_json) try
{
    auto&& device = json_to_device(config_json);
    auto&&aes_cbc_iv = json_to_aes_cbc_iv(config_json);
    auto&&cors_header_token = json_to_cors_header_token(config_json);

    if(device.user_id == 0)
    {
        throw runtime_error("Invalid type or non defined field userId");
    }

    if(device.host.empty())
    {
        throw runtime_error("Invalid type or non defined field host");
    }

    if(device.host_pub_key.empty())
    {
        throw runtime_error("Invalid type or non defined field host_pub_key");
    }

    debug(typeid(*this).name(), "Create new config");

    return {device, aes_cbc_iv, cors_header_token};
}
catch (const runtime_error& e)
{
    throw;
}
catch (const nlohmann::detail::parse_error& e)
{
    throw runtime_error(e.what());
}


}

