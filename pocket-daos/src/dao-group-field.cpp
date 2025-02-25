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

#include "pocket-daos/dao-group-field.hpp"

namespace pocket::daos::inline v5
{

using pods::group_field;

template<>
int64_t dao::persist<pods::group_field>(const pods::group_field::ptr& t)
{

    dao_read_write<group_field> dao_rw;

    auto&& params = dao_rw.write(t);
    int64_t count = 0;
    if(t->id > 0)
    {
        count = database->update(R"(
UPDATE groups_fields
SET
    server_id = ?,
    user_id = ?,
    group_id = ?,
    server_group_id = ?,
    title = ?,
    is_hidden = ?,
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
INSERT INTO groups_fields
(
    server_id,
    user_id,
    group_id,
    server_group_id,
    title,
    is_hidden,
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
    ?,
    ?,
    ?
)
        )", params);
    }

    return count;
}

}
