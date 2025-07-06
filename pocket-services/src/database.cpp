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

#include "pocket-services/database.hpp"
#include "pocket-services/result-set.hpp"
#include "pocket/globals.hpp"

#include <stdexcept>
#include <iostream>
#include <vector>
#include <sstream>
#include <filesystem>


#include <unistd.h>
namespace pocket::services::inline v5
{

using namespace std;
using namespace std::filesystem;
using pods::variant;
using
enum pods::variant::type;

char const database::CREATION_SQL[] = R"sql(
CREATE TABLE `user` ( `id` INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT, `name` text NOT NULL, `email` text NOT NULL, `passwd` text NOT NULL, status integer NOT NULL DEFAULT '0', `timestamp_last_update` INTEGER NOT NULL DEFAULT 0);
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


database::database() = default;

database::~database() try
{
    close();
}
catch (const exception& e)
{
    cerr << e.what() << std::endl;
    debug(typeid(*this).name(), e.what());
}

bool database::open(const string& file_db_path) 
{
    lock_guard<mutex> lg(m);

    if(sqlite3_threadsafe() == 0)
    {
        throw runtime_error("sqlite3 is not thread safe");
    }

    int rc = sqlite3_open_v2(file_db_path.c_str(), &db, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE | SQLITE_OPEN_FULLMUTEX, nullptr);
    if(rc != SQLITE_OK)
    {
        string msg = "Error opening database: ";
        msg += sqlite3_errmsg(db);
        sqlite3_close(db);
        throw runtime_error(msg);
    }

    database::file_db_path = file_db_path;

    uint8_t version = 0;
    if(is_created(version)) //throw exception
    {
        switch(version)
        {
            case 1:
                error(typeid(*this).name(), "Db version not supported, delete and resynch");
                [[fallthrough]];
            default:
                [[likely]] case 2:
                break;
        }
    }
    else
    {
        try
        {
            return create(CREATION_SQL); //throw exception
        }
        catch (const runtime_error& e) 
        {
            error(typeid(this).name(), e.what());
            return false;
        }
    }

    return true;
}


inline void database::close()
{
    if(db == nullptr)
    {
        return;
    }

    //unlock();

    int rc = sqlite3_close_v2(db);
    if(rc != SQLITE_OK)
    {
        string msg = "Error closing database:";
        msg += sqlite3_errmsg(db);
        db = nullptr;
        unlock();
        throw runtime_error(msg);
    }
    db = nullptr;

}

bool database::is_created(uint8_t& db_version) noexcept try
{
    lock();
    result_set rs(*this, "SELECT * FROM metadata"); //throw exception
    if(rs.get_statement_stat() != SQLITE_OK)
    {
        unlock();
        return false;
    }

    if(auto it = optional(*rs.begin()); it)
    {
        db_version = it->begin()->second.to_integer();
    }

    unlock();
    return true;
}
catch (...)
{
    cerr << "Unhandled exception is_created()" << endl;

    unlock();

    auto e_ptr = current_exception();

    if (e_ptr)
    {
        try
        {
            rethrow_exception(e_ptr);
        }
        catch (const runtime_error& e)
        {
            cerr << e.what() << endl;
            return false;
        }
    }
    return false;
}


bool database::create(const char creation_sql[])
{
    if(strlen(creation_sql) == 0)
    {
        return false;
    }

    vector<string> result;
    stringstream ss(creation_sql);
    string part;

    lock();

    uint8_t i = 0;
    bool error = false;
    while(getline(ss, part, ';'))
    {
        i++;
        if(trim(part).empty())
        {
            continue;
        }
        try
        {

            result_set rs(*this, part, {variant{VERSION}}); //throw exception
            if(rs.get_statement_stat() != SQLITE_OK)
            {
                error = true;
                break;
            }
        }
        catch (const exception& e)
        {
            close(); //throw exception
            rm(); //throw exception
            unlock();
            throw runtime_error("Impossible execute query:" + part + " at row:" + to_string(i) + " error:" + e.what());
        }

    }

    if(error)
    {
        close(); //throw exception
        rm(); //throw exception
        unlock();
        throw runtime_error("Impossible execute query:" + part + " at row:" + to_string(i));
    }

    sqlite3_commit_hook(db, nullptr, nullptr);

    info(typeid(*this).name(), "Create database:" + file_db_path);

    unlock();
    return true;
}

bool database::rm()
{
    if(exists(file_db_path))
    {
        remove(file_db_path);
        return true;
    }
    else
    {
        return false;
    }
}


void database::lock()
{
#ifndef POCKET_DISABLE_DB_LOCK
    char* err = nullptr;
    if(int rc = sqlite3_exec(db, "PRAGMA locking_mode = EXCLUSIVE;", nullptr, nullptr, &err); rc != SQLITE_OK)
    {
        string msg = "Database lock error";
        if(err)
        {
            msg += ":";
            msg += err;
            sqlite3_free(err);
        }
        throw runtime_error(msg);
    }
#endif
}

void database::unlock()
{
#ifndef POCKET_DISABLE_DB_LOCK
    char* err = nullptr;
    if(int rc = sqlite3_exec(db, "PRAGMA locking_mode = NORMAL;", nullptr, nullptr, &err); rc != SQLITE_OK)
    {
        string msg = "Database unlock error";
        if(err)
        {
            msg += ":";
            msg += err;
            sqlite3_free(err);
        }
        throw runtime_error(msg);
    }
#endif
}

optional<result_set::ptr> database::execute(const string&& query, const parameters& parameters) try
{
    lock();
    auto rs = make_unique<result_set>(*this, query, parameters);

    if(rs->get_statement_stat() != SQLITE_OK)
    {
        return nullopt;
    }

    unlock();
    return rs;
}
catch (...)
{
    unlock();
    throw;
}


int64_t database::update(const string&& query, const parameters& parameters) try
{
    lock();
    auto rs = make_unique<result_set>(*this, query, parameters);

    if(rs->get_statement_stat() != SQLITE_OK)
    {
        return -1;
    }

    unlock();
    return rs->get_total_changes();
}
catch (...)
{
    unlock();
    throw;
}

}
