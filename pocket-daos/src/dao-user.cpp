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

#include "pocket-daos/dao-user.hpp"
#include "pocket-services/database.hpp"
#include "pocket-services/result-set.hpp"


#include <optional>

namespace pocket::daos::inline v5
{

using namespace std;
using services::database;
using services::result_set;
using pods::user;

dao_user::dao_user(services::database::ptr& database) noexcept
:database(database)
{

}

user::opt dao_user::get()
{

    if(auto&& opt_rs = database->execute("SELECT * FROM user LIMIT 1"); opt_rs.has_value()) //throw exception
    {
        for(auto&& row : **opt_rs)
        {
            user ret {
                    .id = static_cast<uint64_t>(row["id"].to_integer()),
                    .name = row["name"].to_text(),
                    .email = row["email"].to_text(),
                    .passwd = row["passwd"].to_text(),
                    .status = user::stat(row["passwd"].to_integer()),
                    .timestamp_last_update = static_cast<uint64_t>(row["timestamp_last_update"].to_integer())
            };
            return ret;
        }
    }
    return nullopt;
}

user::opt dao_user::login(const string& email, const string& passwd)
{
    if(auto&& opt_rs = database->execute("SELECT * FROM user WHERE email = ? AND passwd = ?", { email, passwd }); opt_rs.has_value()) //throw exception
    {
        for(auto&& row : **opt_rs)
        {
            user ret {
                    .id = static_cast<uint64_t>(row["id"].to_integer()),
                    .name = row["name"].to_text(),
                    .email = row["email"].to_text(),
                    .passwd = row["passwd"].to_text(),
                    .status = user::stat(row["passwd"].to_integer()),
                    .timestamp_last_update = static_cast<uint64_t>(row["timestamp_last_update"].to_integer())
            };
            return ret;
        }
    }
    return nullopt;
}

bool dao_user::persist(const pods::user& user)
{
    database::parameters params = {
            user.name,
            user.email,
            user.passwd,
            static_cast<uint8_t>(user.status),
            user.timestamp_last_update,
            user.id
    };

    int64_t count = 0;
    if(get().has_value())
    {
        count = database->update(R"(
UPDATE user
SET
    name = ?,
    email = ?,
    passwd = ?,
    status = ?,
    timestamp_last_update = ?
WHERE
    id = ?
        )", params);
    }
    else
    {
        count = database->update(R"(
INSERT INTO user
(
    name,
    email,
    passwd,
    status,
    timestamp_last_update,
    id
) VALUES (
    ?,
    ?,
    ?,
    ?,
    ?,
    ?
)
        )", params);
    }

    return count > 0;
}

}

