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

#include "pocket-pods/user.hpp"
#include "pocket-pods/device.hpp"
#include "pocket-pods/group.hpp"
#include "pocket-pods/group_field.hpp"
#include "pocket-pods/field.hpp"

#include <vector>

namespace pocket::pods::inline v5
{

struct response
{
    std::string token;
    pods::user::ptr user;
    pods::device::ptr device;
    std::vector<group::ptr> groups;
    std::vector<group_field::ptr> groups_fields;
    std::vector<field::ptr> fields;
};

}