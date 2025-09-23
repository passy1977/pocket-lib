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

#include <string>
#include <initializer_list>
#include <map>
#include <mutex>
#include <vector>
#include <sqlite3.h>

namespace pocket::services::inline v5
{

class result_set;
class database final
{
    constexpr inline static uint8_t VERSION = 2;
    static char const CREATION_SQL[];
    constexpr inline static uint32_t BUSY_TIMEOUT_MS = 3'000; // Time to wait before retrying when SQLITE_BUSY is encountered

    std::string file_db_path;
    sqlite3* db = nullptr;

    mutable std::mutex m;
public:
    using ptr = std::unique_ptr<database>;

    using parameters = std::vector<pods::variant>;
    using row = std::map<std::string, pods::variant>;

    database();
    ~database();
    POCKET_NO_COPY_NO_MOVE(database)

    bool open(const std::string& file_db_path);
    void close();

    std::optional<std::unique_ptr<result_set>> execute(const std::string&& query, const parameters& parameters = {});

    int64_t update(const std::string&& query, const parameters& parameters = {});

private:
    friend result_set;

    bool is_created(uint8_t& db_version) noexcept;
    bool create(const char creation_sql[]);
    bool rm();

    void lock();
    void unlock();
    void set_wal_mode();

    // Helper function to handle SQLITE_BUSY with retry
    template<typename Func>
    auto execute_with_retry(Func&& func, int max_retries = 3) -> decltype(func());


};


}
