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
#include "pocket-services/json.hpp"
#include "pocket-services/network.hpp"
#include "pocket-services/crypto.hpp"
#include "pocket-daos/dao-user.hpp"

#include <filesystem>
#include <thread>


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

    info(typeid(*this).name(), "Create new session:" + device->uuid);
}

session::~session()
{
    database->close();
    for(auto& it : secret)
    {
        it = '\0';
    }
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
    auto&& user_from_db = dao.login(email, std::move(crypto_encode_sha512(passwd)));
    if(user_from_db)
    {
        auto&& user = user_from_db.value();
        timestamp_last_update = user.timestamp_last_update;
    }

    auto&& it = synchronizer->get_data(timestamp_last_update, email, passwd);
    if(it.has_value())
    {
        auto&& user = it.value();
        user->passwd = std::move(crypto_encode_sha512(passwd));
        dao.write(user);
        return std::move(user);
    }
    else if(user_from_db.has_value())
    {
        auto&& user = user_from_db.value();
        user.passwd = std::move(crypto_encode_sha512(passwd));
        user.timestamp_last_update = timestamp_last_update;
        dao.write(user);
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

}

