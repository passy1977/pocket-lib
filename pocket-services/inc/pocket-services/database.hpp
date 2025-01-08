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
#include <variant>
#include <sqlite3.h>

namespace pocket::services::inline v5
{

class result_set;
class database final
{
    constexpr inline static uint8_t VERSION = 2;
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
INSERT INTO meta VALUES (?);
    )sql";

    std::string file_db_path;
    sqlite3* db = nullptr;

    mutable std::mutex m;
public:
    using ptr = std::unique_ptr<database>;

    using parameters = std::initializer_list<pods::variant>;

    database();
    ~database();
    POCKET_NO_COPY_NO_MOVE(database)

    bool open(const std::string& file_db_path);
    void close();

    bool check_lock(const std::string& file_db_path);

private:
    friend result_set;

    bool is_created() noexcept;
    bool create();

    void write_lock();
    void delete_lock();
};


}