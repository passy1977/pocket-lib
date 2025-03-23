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
#include "pocket-pods/device.hpp"


namespace pocket::controllers::inline v5
{

class config final
{
    std::string config_path;

public:
    using ptr = std::unique_ptr<config>;

    explicit config(const std::optional<std::string>& config_path = {});
    POCKET_NO_COPY_NO_MOVE(config)

    pods::device parse(const std::string_view& config_json);

    inline std::string get_config_path() const noexcept
    {
        return config_path;
    }
};

}