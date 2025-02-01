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

#include "pocket-daos/dao-read-write-field.hpp"

#include <vector>

namespace pocket::daos::inline v5
{
using namespace std;
using pods::field;
using services::result_set;
using row = services::database::row;
using parameters = services::database::parameters;


field::ptr dao_read_write<field>::read(row& row)
{
    auto field = make_unique<pods::field>();
    field->id = row["id"].to_integer();
    field->server_id = row["server_id"].to_integer();
    field->user_id = row["user_id"].to_integer();
    field->group_id = row["group_id"].to_integer();
    field->server_group_id = row["server_group_id"].to_integer();
    field->group_field_id = row["group_field_id"].to_integer();
    field->server_group_field_id = row["server_group_field_id"].to_integer();
    field->title  = row["title"].to_text();
    field->value = row["value"].to_text();
    field->is_hidden = row["is_hidden"].to_integer();
    field->synchronized = row["synchronized"].to_integer();
    field->deleted = row["deleted"].to_integer();
    field->timestamp_creation = row["timestamp_creation"].to_integer();
    return field;
}

parameters dao_read_write<field>::write(const field::ptr& t)
{
    vector<pods::variant> ret;
    ret.push_back(t->server_id);
    ret.push_back(t->user_id);
    ret.push_back(t->group_id);
    ret.push_back(t->server_group_id);
    ret.push_back(t->group_field_id);
    ret.push_back(t->server_group_field_id);
    ret.push_back(t->title);
    ret.push_back(t->value);
    ret.push_back(t->is_hidden);
    ret.push_back(t->synchronized);
    ret.push_back(t->deleted);
    ret.push_back(t->timestamp_creation);
    ret.push_back(t->id);
    return ret;
}

}

