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

#include "pocket-daos/dao.hpp"
#include "pocket/tree.hpp"


namespace pocket::daos::inline v5
{

using pods::group;
using namespace std;

template<>
vector<group::ptr> dao::get_all<group>(int64_t group_id, bool to_synch) const
{
    //vector<group::ptr> ret;
    tree ret;


    if(auto&& opt_rs = database->execute("SELECT * FROM " + group::get_name() + (to_synch ? " WHERE synchronized = 0" : (group_id < 0 ? " WHERE deleted = 0" : " WHERE deleted = 0 AND group_id = " + std::to_string(group_id))) + " ORDER BY group_id, id"); opt_rs) //throw exception
    {
        for(auto&& row : **opt_rs)
        {
            dao_read_write<group> dao;
            if(auto&& it = dao.read(row); it.get())
            {
                //ret.push_back(move(it));
                ret + it;
            }
        }
    }

    return ret.get();
}

int64_t dao::persist_private(const group::ptr& t, bool return_rows_modified) const
{

    dao_read_write<group> dao_rw;

    auto&& params = dao_rw.write(t);
    int64_t last_insert_id = daos::dao::NO_ID;
    if(t->id > 0)
    {
        auto count = database->update(R"(
UPDATE groups
SET
    server_id = ?,
    user_id = ?,
    group_id = ?,
    server_group_id = ?,
    title = ?,
    icon = ?,
    _note = ?,
    synchronized = ?,
    deleted = ?,
    timestamp_creation = ?
WHERE
    id = ?
        )", params);
        
        if(count > 0)
        {
            last_insert_id = t->id;
        }
        else
        {
            last_insert_id = daos::dao::NO_ID;
        }
    }
    else
    {
        auto count = database->update(R"(
INSERT INTO groups
(
    server_id,
    user_id,
    group_id,
    server_group_id,
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
    ?,
    ?,
    ?,
    ?
)
        )", params);
        
        if(count > 0)
        {
            last_insert_id = get_last_inserted_id();
        }
        else
        {
            last_insert_id = daos::dao::NO_ID;
        }
       
    }

    return last_insert_id;
}


}

