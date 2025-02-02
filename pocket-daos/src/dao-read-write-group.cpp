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
using parameters = services::database::parameters;


group::ptr dao_read_write<group>::read(row& row)
{
    auto group = make_unique<pods::group>();
    group->id = row["id"].to_integer();
    group->server_id = row["server_id"].to_integer();
    group->user_id = row["user_id"].to_integer();
    group->group_id = row["group_id"].to_integer();
    group->server_group_id = row["server_group_id"].to_integer();
    group->title  = row["title"].to_text();
    group->icon = row["icon"].to_text();
    group->note = row["_note"].to_text();
    group->synchronized = row["synchronized"].to_integer();
    group->deleted = row["deleted"].to_integer();
    group->timestamp_creation = row["timestamp_creation"].to_integer();
    return group;
}

parameters dao_read_write<group>::write(const group::ptr& t)
{
    if(t.get() == nullptr)
    {
        return {};
    }

    vector<pods::variant> ret;
    ret.push_back(t->server_id);
    ret.push_back(t->user_id);
    ret.push_back(t->group_id);
    ret.push_back(t->server_group_id);
    ret.push_back(t->title);
    ret.push_back(t->icon);
    ret.push_back(t->note);
    ret.push_back(t->synchronized);
    ret.push_back(t->deleted);
    ret.push_back(t->timestamp_creation);
    if(t->id)
    {
        ret.emplace_back(t->id);
    }
    return ret;
}

}

