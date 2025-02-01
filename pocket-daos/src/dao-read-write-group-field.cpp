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

#include "pocket-daos/dao-read-write-group-field.hpp"

namespace pocket::daos::inline v5
{
using namespace std;
using pods::group_field;
using services::result_set;
using row = services::database::row;
using parameters = services::database::parameters;


group_field::ptr dao_read_write<group_field>::read(row& row)
{
    auto group_field = make_unique<pods::group_field>();
    group_field->id = row["id"].to_integer();
    group_field->server_id = row["server_id"].to_integer();
    group_field->user_id = row["user_id"].to_integer();
    group_field->group_id = row["group_id"].to_integer();
    group_field->server_group_id = row["server_group_id"].to_integer();
    group_field->title  = row["title"].to_text();
    group_field->is_hidden = row["is_hidden"].to_integer();
    group_field->synchronized = row["synchronized"].to_integer();
    group_field->deleted = row["deleted"].to_integer();
    group_field->timestamp_creation = row["timestamp_creation"].to_integer();
    return group_field;
}

parameters dao_read_write<group_field>::write(const group_field::ptr& t)
{
    vector<pods::variant> ret;
    ret.push_back(t->server_id);
    ret.push_back(t->user_id);
    ret.push_back(t->group_id);
    ret.push_back(t->server_group_id);
    ret.push_back(t->title);
    ret.push_back(t->is_hidden);
    ret.push_back(t->synchronized);
    ret.push_back(t->deleted);
    ret.push_back(t->timestamp_creation);
    ret.push_back(t->id);
    return ret;
}

}

