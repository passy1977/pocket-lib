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

#include "pocket-daos/dao-field.hpp"


namespace pocket::daos::inline v5
{

using pods::field;

template<>
uint64_t dao::persist<field>(const field::ptr& t)
{

    dao_read_write<field> dao_rw;

    auto&& params = dao_rw.write(t);
    int64_t count = 0;
    if(t->id > 0)
    {
        count = database->update(R"(
UPDATE fields
SET
    group_id = ?,
    server_group_id = ?,
    group_field_id = ?,
    server_group_field_id = ?,
    title = ?,
    value = ?,
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
INSERT INTO fields
(
    group_id,
    server_group_id,
    group_field_id,
    server_group_field_id,
    title,
    value,
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
    ?,
    ?
)
        )", params);
    }

    return count;
}

}
