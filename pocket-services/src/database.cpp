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
#include <thread>
#include <chrono>

namespace pocket::services::inline v5
{

using namespace std;
using namespace std::filesystem;
using pods::variant;
using enum pods::variant::type;

char const database::CREATION_SQL[] = R"sql(
CREATE TABLE `user` ( `id` INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT, `name` text NOT NULL, `email` text NOT NULL, `passwd` text NOT NULL, status integer NOT NULL DEFAULT '0', `timestamp_last_update` INTEGER NOT NULL DEFAULT 0);
CREATE TABLE fields ( `id` integer PRIMARY KEY AUTOINCREMENT, user_id integer NOT NULL DEFAULT 0, server_id integer NOT NULL DEFAULT 0, `group_id` integer NOT NULL DEFAULT 0, `server_group_id` integer NOT NULL DEFAULT 0, `group_field_id` integer NOT NULL DEFAULT 0, `server_group_field_id` integer NOT NULL DEFAULT 0, `title` text NOT NULL, `value` text NOT NULL, `is_hidden` integer NOT NULL, synchronized integer NOT NULL DEFAULT 0, deleted integer NOT NULL DEFAULT '0', `timestamp_creation` INTEGER NOT NULL DEFAULT 0, FOREIGN KEY (user_id) REFERENCES user (id));
CREATE TABLE group_fields (id integer primary key autoincrement, user_id integer NOT NULL DEFAULT 0, server_id integer NOT NULL DEFAULT 0, `group_id` integer NOT NULL DEFAULT 0, `server_group_id` integer NOT NULL DEFAULT 0, title text not null, is_hidden integer not null, synchronized integer NOT NULL DEFAULT 0, deleted integer NOT NULL DEFAULT '0', `timestamp_creation` INTEGER NOT NULL DEFAULT 0, FOREIGN KEY (user_id) REFERENCES user (id));
CREATE TABLE groups ( `id` integer PRIMARY KEY AUTOINCREMENT, user_id integer NOT NULL DEFAULT 0, server_id integer NOT NULL DEFAULT 0, group_id integer, server_group_id integer, `title` text NOT NULL, `icon` text NOT NULL DEFAULT 'UNUSED', `_note` text, synchronized integer NOT NULL DEFAULT 0, deleted integer NOT NULL DEFAULT '0', `timestamp_creation` INTEGER NOT NULL DEFAULT 0, FOREIGN KEY (user_id) REFERENCES user (id));
CREATE TABLE metadata (version INTEGER);
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
INSERT INTO metadata VALUES (?);

)sql";


database::database() : transaction_active(false) {}

database::~database() try
{
    close();
}
catch (const exception& e)
{
    cerr << e.what() << endl;
    error(typeid(*this).name(), e.what());
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

    // Set busy timeout to X seconds to handle SQLITE_BUSY
    rc = sqlite3_busy_timeout(db, BUSY_TIMEOUT_MS);
    if(rc != SQLITE_OK)
    {
        string msg = "Error setting busy timeout: ";
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
        
        // Set WAL mode after database is confirmed to exist and be accessible
        set_wal_mode();
    }
    else
    {
        try
        {
            bool result = create(CREATION_SQL); //throw exception
            if(result)
            {
                // Set WAL mode after successful database creation
                set_wal_mode();
            }
            return result;
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

    // Force rollback if there's an active transaction before closing
    if(transaction_active) 
    {
        sqlite3_exec(db, "ROLLBACK;", nullptr, nullptr, nullptr);
        transaction_active = false;
    }
    
    // Force finalize all prepared statements
    sqlite3_stmt* stmt = nullptr;
    while((stmt = sqlite3_next_stmt(db, nullptr)) != nullptr)
    {
        sqlite3_finalize(stmt);
    }

    int rc = sqlite3_close(db);
    if(rc != SQLITE_OK)
    {
        string msg = "Error closing database:";
        msg += sqlite3_errmsg(db);
        // Try force close
        rc = sqlite3_close_v2(db);
        if(rc != SQLITE_OK)
        {
            db = nullptr;
            throw runtime_error(msg);
        }
    }
    db = nullptr;

}

bool database::is_created(uint8_t& db_version) noexcept try
{
    return execute_with_retry([&] -> bool 
    {
        struct lock_guard {
            database& db;
            lock_guard(database& d) : db(d) { db.lock(); }
            ~lock_guard() { db.unlock(); }
        };
        
        lock_guard guard(*this);

        result_set rs(*this, "SELECT * FROM metadata");
        if(rs.get_statement_stat() != SQLITE_OK)
        {
            if(rs.get_statement_stat() == SQLITE_BUSY)
            {
                throw runtime_error("SQLITE_BUSY: Database is locked in is_created");
            }
            return false;
        }

        if(auto it = optional(*rs.begin()); it)
        {
            db_version = it->begin()->second.to_integer();
        }

        return true;
    });
}
catch (...)
{
    cerr << "Unhandled exception is_created()" << endl;
    
    auto e_ptr = current_exception();
    if (e_ptr)
    {
        try
        {
            rethrow_exception(e_ptr);
        }
        catch (const runtime_error& e)
        {
            error(typeid(*this).name(), e.what());
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
        debug(typeid(*this).name(), "Remove db:" + file_db_path);
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
    if(this == nullptr)
    {
        return;
    }

    if(transaction_active) 
    {
        debug(typeid(*this).name(), "Transaction already active, skipping lock");
        return;
    }
    
    debug(typeid(*this).name(), "Lock");
    // Use BEGIN IMMEDIATE instead of EXCLUSIVE locking mode for better compatibility
    char* err = nullptr;
    if(int rc = sqlite3_exec(db, "BEGIN IMMEDIATE;", nullptr, nullptr, &err); rc != SQLITE_OK)
    {
        if(rc == SQLITE_BUSY)
        {
            if(err) sqlite3_free(err);
            throw runtime_error("SQLITE_BUSY: Cannot acquire database lock");
        }
        string msg = "Database lock error";
        if(err)
        {
            msg += ":";
            msg += err;
            sqlite3_free(err);
        }
        throw runtime_error(msg);
    }
    transaction_active = true;
#endif
}

void database::unlock()
{
#ifndef POCKET_DISABLE_DB_LOCK
    if(this == nullptr)
    {
        return;
    }
    if(!transaction_active) 
    {
        debug(typeid(*this).name(), "No active transaction, skipping unlock");
        return;
    }
    
    debug(typeid(*this).name(), "Unlock");
    // Commit or rollback the transaction
    char* err = nullptr;
    if(int rc = sqlite3_exec(db, "COMMIT;", nullptr, nullptr, &err); rc != SQLITE_OK)
    {
        // If commit fails, try rollback
        sqlite3_exec(db, "ROLLBACK;", nullptr, nullptr, nullptr);
        transaction_active = false; // Reset flag even on failure
        string msg = "Database unlock error";
        if(err)
        {
            msg += ":";
            msg += err;
            sqlite3_free(err);
        }
        throw runtime_error(msg);
    }
    transaction_active = false;
#endif
}

void database::set_wal_mode() noexcept
{
    // Attempt to set WAL mode with retry mechanism
    for(int attempt = 0; attempt < 3; ++attempt)
    {
        char* err = nullptr;
        int rc = sqlite3_exec(db, "PRAGMA journal_mode=WAL;", nullptr, nullptr, &err);
        
        if(rc == SQLITE_OK)
        {
            info(typeid(*this).name(), "Successfully set WAL mode");
            return;
        }
        
        string error_msg;
        if(err)
        {
            error_msg = err;
            sqlite3_free(err);
        }
        else
        {
            error_msg = sqlite3_errmsg(db);
        }
        
        if(rc == SQLITE_BUSY && attempt < 2)
        {
            info(typeid(*this).name(), "Database busy when setting WAL mode, retrying... (attempt " + to_string(attempt + 1) + "/3)");
            this_thread::sleep_for(chrono::milliseconds(100 * (attempt + 1)));
            continue;
        }
        
        // If we get here, either it's not SQLITE_BUSY or we've exhausted retries
        info(typeid(*this).name(), "Failed to set WAL mode: " + error_msg + 
                " (continuing with default journal mode)");
        return;
    }
}

optional<result_set::ptr> database::execute(const string&& query, const parameters& parameters) try
{
    return execute_with_retry([&] -> optional<result_set::ptr> {
        lock();
        auto rs = make_unique<result_set>(*this, query, parameters);

        if(rs->get_statement_stat() != SQLITE_OK)
        {
            unlock();
            if(rs->get_statement_stat() == SQLITE_BUSY)
            {
                throw runtime_error("SQLITE_BUSY: Database is locked in execute");
            }
            return nullopt;
        }

        unlock();
        return rs;
    }, BUSY_MAX_RETRIES);
}
catch (...)
{
    unlock();
    throw;
}


int64_t database::update(const string&& query, const parameters& parameters) try
{
    return execute_with_retry([&] -> int64_t 
    {
        lock();
        auto rs = make_unique<result_set>(*this, query, parameters);

        if(rs->get_statement_stat() != SQLITE_OK)
        {
            unlock();
            if(rs->get_statement_stat() == SQLITE_BUSY)
            {
                throw runtime_error("SQLITE_BUSY: Database is locked in update");
            }
            return -1;
        }

        unlock();
        return rs->get_total_changes();
    }, BUSY_MAX_RETRIES);
}
catch (...)
{
    unlock();
    throw;
}

// Template implementation for retry logic
template<typename Func>
auto database::execute_with_retry(Func&& func, uint8_t max_retries) -> decltype(func())
{
    for(uint8_t attempt = 0; attempt < max_retries; ++attempt)
    {
        try
        {
            return func();
        }
        catch(const runtime_error& e)
        {
            // Check if it's a SQLITE_BUSY error
            string error_msg = e.what();
            if(error_msg.find("SQLITE_BUSY") != string::npos || 
               error_msg.find("database is locked") != string::npos ||
               error_msg.find("Cannot acquire database lock") != string::npos)
            {
                if(attempt < max_retries - 1)
                {
                    // Wait before retry with exponential backoff
                    auto wait_time = chrono::milliseconds(50 + (100 * attempt));
                    this_thread::sleep_for(wait_time);
                    
                    debug(typeid(*this).name(), 
                          "SQLITE_BUSY detected, retrying attempt " + 
                          to_string(attempt + 2) + "/" + 
                          to_string(max_retries) + " - Error: " + error_msg);
                    continue;
                }
                else
                {
                    error(typeid(*this).name(), 
                          "SQLITE_BUSY: Max retries reached (" + to_string(max_retries) + 
                          "), giving up. Error: " + error_msg);
                }
            }
            // Re-throw if not SQLITE_BUSY or max retries reached
            throw;
        }
    }
    // This should never be reached, but needed for template compilation
    return func();
}

}
