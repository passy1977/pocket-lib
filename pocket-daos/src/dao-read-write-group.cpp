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

#include "pocket-daos/dao-read-write-group.hpp"

namespace pocket::daos::inline v5
{
using namespace std;
using pods::group;
using services::result_set;
using row = services::database::row;


optional<group> dao_read_write<group>::read(services::database::row& row)
{
    group group;
    group.group_id = row["group_id"].to_integer();
    group.server_group_id = row["server_group_id"].to_integer();
    group.title  = row["title"].to_text();
    group.icon = row["icon"].to_text();
    group.note = row["_note"].to_text();
    group.synchronized = row["synchronized"].to_integer();
    group.deleted = row["deleted"].to_integer();
    group.shared = row["shared"].to_integer();
    group.timestamp_creation = row["timestamp_creation"].to_integer();
    return group;
}

bool dao_read_write<group>::write()
{
    throw std::runtime_error("Not implemented");
}

}

