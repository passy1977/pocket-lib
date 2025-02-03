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

#include "pocket-daos/dao-group.hpp"
#include "pocket-daos/dao.hpp"


namespace pocket::daos::inline v5
{

using pods::group;

template<>
uint64_t dao::persist<group>(const group::ptr& t)
{

    dao_read_write<group> dao_rw;

    auto&& params = dao_rw.write(t);
    int64_t count = 0;
    if(t->id > 0)
    {
        count = database->update(R"(
UPDATE groups
SET
    group_id = ?,
    title = ?,
    icon = ?,
    _note = ?,
    synchronized = ?,
    deleted = ?,
    timestamp_creation = ?
WHERE
    id = ?
        )", params);
    }
    else
    {
        count = database->update(R"(
INSERT INTO groups
(
    group_id,
    title,
    icon,
    _note,
    synchronized,
    deleted,
    timestamp_creation
) VALUES (
    ?,
    ?,
    ?,
    ?,
    ?,
    ?,
    ?
)
        )", params);
    }

    return count;
}

void dao::update_all_index()
{
    auto count = database->update(R"(
UPDATE fields
SET group_id = (SELECT id FROM groups WHERE server_id = fields.server_group_id)

)");

    count = database->update(R"(
UPDATE groups_fields
SET server_id = (SELECT server_id FROM groups WHERE id = groups_fields.server_group_id)
WHERE EXISTS (
    SELECT 1 FROM groups WHERE id = groups_fields.server_group_id AND groups.user_id = groups_fields.user_id AND groups.server_id = groups_fields.server_id
);
)");


}

}

