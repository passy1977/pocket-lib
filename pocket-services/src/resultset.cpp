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

#include "pocket-services/resultset.hpp"

namespace pocket::services::inline v5
{

using namespace std;
using pods::variant;
using enum pods::variant::type;

result_set::result_set(class database& database, const std::string& query, const database::parameters& parameters)
        : database(database)
{
    statement_status = sqlite3_prepare_v3(database.db, query.c_str(), query.length(), 0, &stmt, nullptr);
    if( statement_status == SQLITE_OK )
    {
        for(int i = 1; auto &&param : parameters) {
            switch (param.get_type()) {
                default:
                case TEXT: sqlite3_bind_text(stmt, i, param.to_text().c_str(), -1, SQLITE_TRANSIENT); break;
                case INT: sqlite3_bind_int(stmt, i, static_cast<int32_t>(param.to_integer())); break;
                case INT64: sqlite3_bind_int64(stmt, i, param.to_integer()); break;
                case FLOAT: sqlite3_bind_double(stmt, i, param.to_float()); break;
            }
            i++;
        }

        int rc = SQLITE_DONE;
        if (rc = sqlite3_step(stmt); rc != SQLITE_DONE && rc != SQLITE_ERROR)
        {
            for (int i = 0; i < sqlite3_column_count(stmt); i++)
            {
                int type = 0;
                switch (sqlite3_column_type(stmt, i))
                {
                    case (SQLITE3_TEXT):  type = SQLITE3_TEXT;  break;
                    case (SQLITE_INTEGER): type = SQLITE_INTEGER; break;
                    case (SQLITE_FLOAT): type = SQLITE_FLOAT; break;
                    case (SQLITE_BLOB): type = SQLITE_BLOB; break;
                    default: break;
                }
                columns[sqlite3_column_name(stmt, i)] = pair<int, int>{i, type};
            }
            sqlite3_finalize(stmt);
        }
        else if (rc == SQLITE_ERROR)
        {
            sqlite3_finalize(stmt);
            throw runtime_error("Impossible execute query err:" + string(sqlite3_errmsg(database.db)));
        }
        count++;
    }
    else if(statement_status == SQLITE_ERROR)
    {
        sqlite3_finalize(stmt);
        throw runtime_error("Impossible execute query err:" + string(sqlite3_errmsg(database.db)));
    }
}

result_set::~result_set() = default;



}