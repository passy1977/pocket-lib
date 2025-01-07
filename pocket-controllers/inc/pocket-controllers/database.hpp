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

#include <string>
#include <vector>
#include <map>
#include <mutex>
#include <variant>
#include <sqlite3.h>

namespace pocket::controllers::inline v5
{

class result_set;
class database final
{

    constexpr inline static char CREATION_SQL[] = R"sql(
    CREATE TABLE `user` ( `id` INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT, user_uuid TEXT NOT NULL DEFAULT '', status integer NOT NULL DEFAULT '0');
    CREATE TABLE `properties` ( `id` INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT, user_id integer NOT NULL DEFAULT 0, server_id integer NOT NULL DEFAULT 0, `_key` TEXT NOT NULL DEFAULT '', `_value` TEXT NOT NULL DEFAULT '');
    CREATE TABLE fields ( `id` integer PRIMARY KEY AUTOINCREMENT, user_id integer NOT NULL DEFAULT 0, server_id integer NOT NULL DEFAULT 0, `group_id` integer NOT NULL DEFAULT 0, `group_field_id` integer NOT NULL DEFAULT 0, `title` text NOT NULL, `value` text NOT NULL, `is_hidden` integer NOT NULL, synchronized integer NOT NULL DEFAULT 0, deleted integer NOT NULL DEFAULT '0', FOREIGN KEY (user_id) REFERENCES addresses (id));
    CREATE TABLE group_fields (id integer primary key autoincrement, user_id integer NOT NULL DEFAULT 0, server_id integer NOT NULL DEFAULT 0, `group_id` integer NOT NULL DEFAULT 0, title text not null, is_hidden integer not null, synchronized integer NOT NULL DEFAULT 0, deleted integer NOT NULL DEFAULT '0', is_temporary integer, FOREIGN KEY (user_id) REFERENCES addresses (id));
    CREATE TABLE groups ( `id` integer PRIMARY KEY AUTOINCREMENT, user_id integer NOT NULL DEFAULT 0, server_id integer NOT NULL DEFAULT 0, group_id integer, server_group_id integer, `title` text NOT NULL, `icon` text NOT NULL DEFAULT 'UNUSED', `_note` text, synchronized integer NOT NULL DEFAULT 0, deleted integer NOT NULL DEFAULT '0', shared integer, FOREIGN KEY (user_id) REFERENCES addresses (id));
    CREATE INDEX `groups_title` ON `groups` (title);
    CREATE INDEX `group_fields_group_id` ON `group_fields` (`group_id`);
    CREATE INDEX fields_group_field_id ON fields (group_field_id);
    CREATE INDEX groups_server_id ON groups (server_id);
    CREATE INDEX group_fields_server_id ON group_fields (server_id);
    CREATE INDEX fields_server_id ON fields (server_id);
    CREATE INDEX groups_user_id ON groups (user_id);
    CREATE INDEX group_fields_user_id ON group_fields (user_id);
    CREATE INDEX fields_user_id ON fields (user_id);
    CREATE INDEX groups_deleted ON groups (deleted);
    CREATE INDEX group_fields_deleted ON group_fields (deleted);
    CREATE INDEX fields_deleted ON fields (deleted);
    )sql";

    std::string file_db_path;
    sqlite3* db = nullptr;

    mutable std::mutex m;
public:
    using ptr = std::unique_ptr<database>;

    database();
    ~database();
    POCKET_NO_COPY_NO_MOVE(database)

    bool open(const std::string& file_db_path);
    void close();



private:
    friend result_set;

    bool is_created();
    bool create();

    void write_lock();
    bool check_lock(const std::string& file_db_path) noexcept;
    void delete_lock();
};

class result_set final
{
    template<typename T>
    using parameter = std::vector<std::variant<T>>;

    enum class Type {
        INTEGER = SQLITE_INTEGER,
        FLOAT = SQLITE_FLOAT,
        TEXT = SQLITE3_TEXT,
        BLOB = SQLITE_BLOB,
    };


    sqlite3_stmt *stmt = nullptr;
    uint64_t count = 0;

    std::map<std::string, std::pair<int, int>> columns; //idx, sql_type

    class database& database;
public:
    explicit result_set(class database& database, const std::string& query);

    inline explicit result_set(class database& database, const std::string&& query)
     : result_set(database, query)
    {}

    POCKET_NO_COPY_NO_MOVE(result_set)


};


}
