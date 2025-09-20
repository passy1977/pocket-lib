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

#include "pocket-services/result-set.hpp"

namespace pocket::services::inline v5
{

using namespace std;
using pods::variant;
using enum pods::variant::type;

result_set::result_set(class database& database, const std::string& query, const database::parameters& parameters)
        : database(database)
{
    sqlite3_stmt *stmt = nullptr;

    debug(typeid(*this).name(), query);
    statement_stat = sqlite3_prepare_v3(database.db, query.c_str(), static_cast<int>(query.length()), 0, &stmt, nullptr);
    
    // Ensure statement is always finalized using RAII-like pattern
    auto stmt_guard = [&stmt]() {
        if(stmt != nullptr) {
            sqlite3_finalize(stmt);
            stmt = nullptr;
        }
    };
    
    if(statement_stat == SQLITE_OK )
    {
        for(int i = 1; auto &&param : parameters) {
            switch (param.get_type()) {
                default:
                case TEXT:
                    sqlite3_bind_text(stmt, i, param.to_text().c_str(), -1, SQLITE_TRANSIENT);
                    debug(typeid(*this).name(), to_string(i) + ": " + param.to_text());
                    break;
                case INT:
                    sqlite3_bind_int(stmt, i, static_cast<int32_t>(param.to_integer()));
                    debug(typeid(*this).name(), to_string(i) + ": " + param.to_text());
                    break;
                case INT64:
                    sqlite3_bind_int64(stmt, i, param.to_integer());
                    debug(typeid(*this).name(), to_string(i) + ": " + param.to_text());
                    break;
                case DOUBLE:
                    sqlite3_bind_double(stmt, i, param.to_float());
                    debug(typeid(*this).name(), to_string(i) + ": " + param.to_text());
                    break;
            }
            i++;
        }

        map<std::string, uint8_t> columns; //idx, sql_type

        int rc = SQLITE_DONE;
        if (rc = sqlite3_step(stmt); rc != SQLITE_DONE && rc != SQLITE_ERROR)
        {
            for (int i = 0; i < sqlite3_column_count(stmt); i++)
            {
                columns[sqlite3_column_name(stmt, i)] = i;
            }
            sqlite3_reset(stmt);

            while (sqlite3_step(stmt) != SQLITE_DONE && !columns.empty())
            {
                database::row row;
                for(auto&& [column, i] : columns)
                {
                    switch (sqlite3_column_type(stmt, i))
                    {
                        case SQLITE3_TEXT:
                            row.try_emplace(column, reinterpret_cast<const char *>(sqlite3_column_text(stmt, i)));
                            break;
                        case SQLITE_INTEGER:
                            row.try_emplace(column, sqlite3_column_int(stmt, i));
                            break;
                        case SQLITE_FLOAT:
                            row.try_emplace(column, sqlite3_column_double(stmt, i));
                            break;
                        case SQLITE_NULL:
                            row.try_emplace(column, nullptr);
                            break;
                        default: break;
                    }
                }

                push_back(row);
            }
        }
        else if (rc == SQLITE_DONE)
        {
            total_changes = sqlite3_total_changes64(database.db);
        }
        else if (rc == SQLITE_ERROR)
        {
            stmt_guard(); // Finalize statement before throwing
            throw runtime_error("Impossible execute query err:" + string(sqlite3_errmsg(database.db)));
        }

    }
    else if(statement_stat == SQLITE_ERROR)
    {
        stmt_guard(); // Finalize statement before throwing
        throw runtime_error("Impossible execute query err:" + string(sqlite3_errmsg(database.db)));
    }
    
    // Always finalize the statement at the end
    stmt_guard();

}

result_set::~result_set() = default;



}
