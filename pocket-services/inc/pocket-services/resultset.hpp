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

#include "pocket/globals.hpp"
#include "pocket-pods/variant.hpp"
#include "pocket-services/database.hpp"

namespace pocket::services::inline v5
{

class result_set final : private std::map<std::string, pods::variant>
{
    sqlite3_stmt *stmt = nullptr;
    uint64_t count = 0;

    std::map<std::string, std::pair<int, int>> columns; //idx, sql_type

    class database& database;
    int statement_status = SQLITE_OK;
public:
    explicit result_set(class database& database, const std::string& query, const database::parameters& parameters = {});

    inline explicit result_set(class database& database, const std::string&& query, const database::parameters& parameters = {})
            : result_set(database, query, parameters)
    {}
    ~result_set();
    POCKET_NO_COPY_NO_MOVE(result_set)

    using map::iterator;
    using map::const_iterator;
    using map::reverse_iterator;
    using map::const_reverse_iterator;

    using map::begin;
    using map::end;

    using map::operator[];

    inline int get_statement_status() const noexcept
    {
        return statement_status;
    }
};


}