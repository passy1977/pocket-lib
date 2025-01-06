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
#include "pocket/globals.hpp"

#include <nlohmann/json.hpp>
#include <filesystem>
#include <cstdlib>
#include <stdexcept>

namespace pocket::controllers::inline v5
{

using pods::device;
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
            create_directories(absolute_path);
        }
        catch (const std::filesystem::filesystem_error& e)
        {
            throw runtime_error(e.what());
        }
    }

    this->config_path = absolute_path;
}

device::ptr config::parse(const string& config_json)
{

    nlohmann::json json = config_json;

    auto device = make_unique<struct device>();



    return device;
}



}

