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

#include "pocket-controllers/session.hpp"
#include "pocket-services/network.hpp"
#include "pocket-services/crypto.hpp"
#include "pocket-daos/dao-user.hpp"

#include <filesystem>
#include <thread>
#include <fstream>
#include <unistd.h>


namespace pocket::controllers::inline v5
{
using pods::device;
using pods::user;
using services::database;
using services::network;
using services::synchronizer;
using services::crypto_encode_sha512;
using daos::dao_user;
using namespace std;
using namespace std::filesystem;



session::session(const optional<string>& config_json, const optional<string>& config_path)
{
    if(!config_json.has_value())
    {
        throw runtime_error("config_json not defined it's mandatory");
    }

    if(config_json->empty())
    {
        throw runtime_error("config_json empty mandatory");
    }

    this->config = make_unique<class config>(config_path);

    device = std::move(config->parse(*config_json));

    if(check_lock())
    {
        runtime_error("Another session handle:" + device->uuid);
    }

    lock();

    info(typeid(*this).name(), "Create new session:" + device->uuid);
}

session::~session() try
{
    database->close();
    for(auto& it : secret)
    {
        it = '\0';
    }
    unlock();
}
catch (const runtime_error& e)
{
    error(typeid(*this).name(), e.what());
}

const device::opt& session::init()
{
    if(config.get() == nullptr)
    {
        throw runtime_error("config not defined");
    }

    if(!device.has_value())
    {
        throw runtime_error("config not defined");
    }

    auto&& file_db_path = config->get_config_path();

    if(!file_db_path.ends_with(path::preferred_separator))
    {
        file_db_path += path::preferred_separator;
    }

    file_db_path += device->uuid;
    file_db_path += ".db";

    database = make_unique<class database>();

    uint8_t attempts = 10;
    while(!database->open(file_db_path) && attempts > 0)
    {
        this_thread::sleep_for(chrono::milliseconds(10));
        attempts--;
    }

    if(attempts == 0)
    {
        throw runtime_error("Database busy");
    }

    synchronizer = make_unique<class synchronizer>(database, secret, *device);

    return device;
}

std::optional<pods::user::ptr> session::login(const string& email, const string& passwd) try
{

    dao_user dao(database);

    uint64_t timestamp_last_update = synchronizer::FULL_SYNC;
    auto&& user_from_db = dao.login(email, crypto_encode_sha512(passwd));
    if(user_from_db)
    {
        auto&& user = user_from_db.value();
        timestamp_last_update = user.timestamp_last_update;
    }

    bool remote_connection_error = false;
    optional<user::ptr> user_from_net = nullopt;
    try
    {
        user_from_net = std::move(synchronizer->retrieve_data(timestamp_last_update, email, passwd));
    }
    catch (const runtime_error& e)
    {
        remote_connection_error = true;
        error(typeid(this).name(), e.what());
    }



    if(user_from_net.has_value())
    {
        auto&& user = user_from_net.value();
        if(
            user_from_db.has_value() && user->id != user_from_db->id
            || user_from_db->status != user::stat::ACTIVE
        )
        {
            return nullopt;
        }

        device->user_id = user->id;
        user->passwd = std::move(crypto_encode_sha512(passwd));
        dao.persist(user);
        return std::move(user);
    }
    else if(user_from_db.has_value() && remote_connection_error)
    {
        auto&& user = user_from_db.value();

        if(
            user.id != device->user_id
            || user_from_db->status != user::stat::ACTIVE
        )
        {
            return nullopt;
        }

        user.passwd = std::move(crypto_encode_sha512(passwd));
        dao.persist(user);
        return make_unique<pods::user>(user);
    }
    else
    {
        return nullopt;
    }

}
catch(const exception& e)
{
    error(typeid(this).name(), e.what());
    return nullopt;
}

void session::lock()
{
    if(config == nullptr)
    {
        return;
    }

    pid_t pid = getpid();

    ofstream out(config->get_config_path() + LOCK_EXTENSION);

    if (!out)
    {
        throw runtime_error("Error: Could not open file for writing.");
    }

    out << pid << endl;

    out.close();
}

void session::unlock()
{
    if (exists(config->get_config_path() + LOCK_EXTENSION))
    {
        filesystem::remove(config->get_config_path() + LOCK_EXTENSION);  //throw exception
    }
    else
    {
        throw runtime_error("File does not exist.");
    }
}

bool session::check_lock()
{
#ifdef DISABLE_LOCK
    return false;
#else
    if (exists(config->get_config_path() + LOCK_EXTENSION))
    {
        ifstream file(config->get_config_path() + LOCK_EXTENSION);
        if (!file.is_open())
        {
            throw runtime_error("Error opening file.");
        }

        string pid((istreambuf_iterator<char>(file)), (istreambuf_iterator<char>()));

        file.close();

        info(typeid(*this).name(), "DB locked: " + config->get_config_path() + LOCK_EXTENSION + " by pid:" + pid);
        return true;
    }
    else
    {
        return false;
    }
#endif
}

}

