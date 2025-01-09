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
#include "pocket-services/resultset.hpp"
#include "pocket/globals.hpp"

#include <stdexcept>
#include <iostream>
#include <vector>
#include <sstream>
#include <algorithm>
#include <filesystem>
#include <fstream>

#include <unistd.h>
namespace pocket::services::inline v5
{

using namespace std;
using namespace std::filesystem;
using pods::variant;
using enum pods::variant::type;

database::database() = default;
database::~database() try
{
    close();
}
catch(const exception& e)
{
    cerr << e.what() << std::endl;
    debug(typeid(*this).name(), e.what());
}

bool database::open(const string& file_db_path)
{
    lock_guard<mutex> lg(m);

    if(check_lock(file_db_path + LOCK_EXTENSION)) //throw exception
    {
        return false;
    }

    int rc = sqlite3_open_v2(file_db_path.c_str(), &db, SQLITE_OPEN_READWRITE|SQLITE_OPEN_CREATE|SQLITE_OPEN_FULLMUTEX, nullptr);
    if (rc != SQLITE_OK)
    {
        string msg = "Error opening database: ";
        msg += sqlite3_errmsg(db);
        sqlite3_close(db);
        throw runtime_error(msg);
    }

    database::file_db_path = file_db_path;

    lock();

    if(is_created()) //throw exception
    {
       //todo: check version
    }
    else
    {
        create(); //throw exception
    }

    return true;
}



inline void database::close()
{
    if(db == nullptr)
    {
        return;
    }
    int rc = sqlite3_close_v2(db);
    if (rc != SQLITE_OK)
    {
        string msg = "Error closing database:";
        msg += sqlite3_errmsg(db);
        db = nullptr;
        unlock();
        throw runtime_error(msg);
    }
    db = nullptr;
    unlock();
}

bool database::is_created() noexcept try
{
    result_set rs(*this, "SELECT * FROM meta"); //throw exception
    if(rs.get_statement_status() != SQLITE_OK)
    {
        return false;
    }

    return true;
}
catch (...)
{
    return false;
}

bool database::create()
{
    vector<string> result;
    stringstream ss(CREATION_SQL);
    string part;

    uint8_t i = 0;
    bool error = false;
    while (getline(ss, part, ';'))
    {
        i++;
        try
        {
            result_set rs(*this, part, { variant{VERSION} }); //throw exception
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

    info(typeid(*this).name(), "Create database:" + file_db_path);

    return true;
}

bool database::rm()
{
    if (exists(file_db_path))
    {
        remove(file_db_path);
        return true;
    }
    else
    {
        return false;
    }
}


bool database::check_lock(const string& file_db_path)
{
#ifdef DISABLE_LOCK
    return false;
#else
    if (exists(file_db_path))
    {
        ifstream file(file_db_path);
        if (!file.is_open())
        {
            throw runtime_error("Error opening file.");
        }

        string pid((istreambuf_iterator<char>(file)), (istreambuf_iterator<char>()));

        file.close();

        info(typeid(*this).name(), "DB locked: " + file_db_path + LOCK_EXTENSION + " by pid:" + pid);
        return true;
    }
    else
    {
        return false;
    }
#endif
}


void database::lock()
{
    char* err = nullptr;
    int rc = sqlite3_exec(db, "PRAGMA locking_mode = EXCLUSIVE;", nullptr, nullptr, &err);
    if (rc != SQLITE_OK)
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
}

void database::unlock()
{
    char* err = nullptr;
    int rc = sqlite3_exec(db, "PRAGMA locking_mode = NORMAL;", nullptr, nullptr, &err);
    if (rc != SQLITE_OK)
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

}





