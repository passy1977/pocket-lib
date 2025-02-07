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
    constexpr inline static char CREATION_SQL[] = R"sql(
CREATE TABLE `user` ( `id` INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT, `name` text NOT NULL, `email` text NOT NULL, `passwd` text NOT NULL, status integer NOT NULL DEFAULT '0', `timestamp_last_update` INTEGER NOT NULL DEFAULT 0);
CREATE TABLE `properties` ( `id` INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT, user_id integer NOT NULL DEFAULT 0, server_id integer NOT NULL DEFAULT 0, `_key` TEXT NOT NULL DEFAULT '', `_value` TEXT NOT NULL DEFAULT '');
CREATE TABLE fields ( `id` integer PRIMARY KEY AUTOINCREMENT, user_id integer NOT NULL DEFAULT 0, server_id integer NOT NULL DEFAULT 0, `group_id` integer NOT NULL DEFAULT 0, `server_group_id` integer NOT NULL DEFAULT 0, `group_field_id` integer NOT NULL DEFAULT 0, `server_group_field_id` integer NOT NULL DEFAULT 0, `title` text NOT NULL, `value` text NOT NULL, `is_hidden` integer NOT NULL, synchronized integer NOT NULL DEFAULT 0, deleted integer NOT NULL DEFAULT '0', `timestamp_creation` INTEGER NOT NULL DEFAULT 0, FOREIGN KEY (user_id) REFERENCES addresses (id));
CREATE TABLE groups_fields (id integer primary key autoincrement, user_id integer NOT NULL DEFAULT 0, server_id integer NOT NULL DEFAULT 0, `group_id` integer NOT NULL DEFAULT 0, `server_group_id` integer NOT NULL DEFAULT 0, title text not null, is_hidden integer not null, synchronized integer NOT NULL DEFAULT 0, deleted integer NOT NULL DEFAULT '0', `timestamp_creation` INTEGER NOT NULL DEFAULT 0, FOREIGN KEY (user_id) REFERENCES addresses (id));
CREATE TABLE groups ( `id` integer PRIMARY KEY AUTOINCREMENT, user_id integer NOT NULL DEFAULT 0, server_id integer NOT NULL DEFAULT 0, group_id integer, server_group_id integer, `title` text NOT NULL, `icon` text NOT NULL DEFAULT 'UNUSED', `_note` text, synchronized integer NOT NULL DEFAULT 0, deleted integer NOT NULL DEFAULT '0', `timestamp_creation` INTEGER NOT NULL DEFAULT 0, FOREIGN KEY (user_id) REFERENCES addresses (id));
CREATE TABLE metadata (version INTEGER);
CREATE INDEX `groups_title` ON `groups` (title);
CREATE INDEX `groups_fields_group_id` ON `groups_fields` (`group_id`);
CREATE INDEX fields_group_field_id ON fields (group_field_id);
CREATE INDEX groups_server_id ON groups (server_id);
CREATE INDEX groups_fields_server_id ON groups_fields (server_id);
CREATE INDEX fields_server_id ON fields (server_id);

CREATE INDEX groups_user_id ON groups (user_id);
CREATE INDEX groups_fields_user_id ON groups_fields (user_id);
CREATE INDEX fields_user_id ON fields (user_id);
CREATE INDEX groups_deleted ON groups (deleted);
CREATE INDEX groups_fields_deleted ON groups_fields (deleted);
CREATE INDEX fields_deleted ON fields (deleted);
INSERT INTO metadata VALUES (?);
    )sql";

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


};


}
