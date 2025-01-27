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

    int rc = sqlite3_open_v2(file_db_path.c_str(), &db, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE | SQLITE_OPEN_FULLMUTEX, nullptr);
    if(rc != SQLITE_OK)
    {
        string msg = "Error opening database: ";
        msg += sqlite3_errmsg(db);
        sqlite3_close(db);
        throw runtime_error(msg);
    }

    database::file_db_path = file_db_path;

    lock();

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
        create(CREATION_SQL); //throw exception
    }

    return true;
}


inline void database::close()
{
    if(db == nullptr)
    {
        return;
    }

    unlock();

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
    result_set rs(*this, "SELECT * FROM metadata"); //throw exception
    if(rs.get_statement_status() != SQLITE_OK)
    {
        return false;
    }

    if(auto it = optional(*rs.begin()); it)
    {
        db_version = it->begin()->second.to_integer();
    }

    return true;
}
catch (...)
{
    return false;
}

bool database::create(const char creation_sql[])
{
    vector<string> result;
    stringstream ss(creation_sql);
    string part;

    uint8_t i = 0;
    bool error = false;
    while(getline(ss, part, ';'))
    {
        i++;
        try
        {

            result_set rs(*this, part, {variant{VERSION}}); //throw exception
            if(rs.get_statement_status() != SQLITE_OK)
            {
                error = true;
                break;
            }
        }
        catch (const exception& e)
        {
            close(); //throw exception
            rm(); //throw exception
            throw runtime_error("Impossible execute query:" + part + " at row:" + to_string(i) + " error:" + e.what());
        }

    }

    if(error)
    {
        close(); //throw exception
        rm(); //throw exception
        throw runtime_error("Impossible execute query:" + part + " at row:" + to_string(i));
    }

    sqlite3_commit_hook(db, nullptr, nullptr);

    info(typeid(*this).name(), "Create database:" + file_db_path);

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

//    err = nullptr;
//    if (int rc = sqlite3_exec(db, "BEGIN EXCLUSIVE",nullptr, nullptr,&err); rc != SQLITE_OK)
//    {
//        string msg = "Database lock error";
//        if(err)
//        {
//            msg += ":";
//            msg += err;
//            sqlite3_free(err);
//        }
//        throw runtime_error(msg);
//    }
}

void database::unlock()
{
    char* err = nullptr;
//    if (int rc = sqlite3_exec(db, "COMMIT",nullptr, nullptr,&err); rc != SQLITE_OK)
//    {
//        string msg = "Database lock error";
//        if(err)
//        {
//            msg += ":";
//            msg += err;
//            sqlite3_free(err);
//        }
//        throw runtime_error(msg);
//    }

//    err = nullptr;
//    int rc = sqlite3_exec(db, "PRAGMA locking_mode = NORMAL;", nullptr, nullptr, &err);
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
}


optional<result_set::ptr> database::execute(const string&& query, const parameters& parameters)
{
    auto rs = make_unique<result_set>(*this, query, parameters);

    if(rs->get_statement_status() != SQLITE_OK)
    {
        return nullopt;
    }

    return rs;
}

int64_t database::update(const string&& query, const parameters& parameters)
{
    auto rs = make_unique<result_set>(*this, query, parameters);

    if(rs->get_statement_status() != SQLITE_OK)
    {
        return -1;
    }

    return rs->get_total_changes();
}


}