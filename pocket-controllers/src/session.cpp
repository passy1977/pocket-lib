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
#include "pocket-services/crypto.hpp"
#include "pocket-daos/dao-user.hpp"

#include <filesystem>
#include <thread>
#include <stdexcept>
#include <fstream>

#ifdef POCKET_FORCE_TIMESTAMP_LAST_UPDATE
#warning You force user.timestamp_last_update
#endif

#ifndef POCKET_ENABLE_AES
#warning AES disabled, data stored non safe
#define POCKET_ENABLE_AES (0)
#endif

namespace pocket::controllers::inline v5
{

using namespace pods;
using namespace services;
using services::database;
using services::synchronizer;
using services::crypto_encode_sha512;
using daos::dao_user;
using views::view;
using namespace std;
using namespace std::filesystem;
using namespace nlohmann;



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

    device = config->parse(*config_json);

    if(check_lock())
    {
        throw runtime_error("Another session handle:" + device->uuid);
    }

    lock();

    info(typeid(*this).name(), "Create new session:" + device->uuid + " at:" + config->get_config_path() );
}

session::~session() try
{
    if(database)
    {
        database->close();
    }
    fill(secret.begin(), secret.end(), 0x00);
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
    status = synchronizer->set_status();
    return device;
}

std::optional<user::ptr> session::login(const string& email, const string& passwd)
{
    if(email.empty() || passwd.empty())
    {
        return nullopt;
    }

    dao_user dao(database);

    //uint64_t timestamp_last_update = synchronizer::FULL_SYNC;
    auto&& user_from_db = dao.login(email, crypto_encode_sha512(passwd));
    if(user_from_db)
    {
        auto&& user = user_from_db.value();
        user.passwd = passwd;
        return retrieve_data(make_unique<struct user>(user));
    }
    else
    {
        return retrieve_data(make_unique<struct user>(user{
                .email = email,
                .passwd = passwd
        }));
    }

}


std::optional<user::ptr> session::retrieve_data(const std::optional<pods::user::ptr>& user_opt) try
{
    if(!user_opt.has_value())
    {
        error(typeid(this).name(), "user empty");
        return nullopt;
    }

    dao_user dao(database);
    auto&& user = *user_opt;

    bool remote_connection_error = false;
    optional<user::ptr> user_from_net = nullopt;
    try
    {
        user_from_net = synchronizer->retrieve_data(user->timestamp_last_update, user->email, user->passwd);
    }
    catch (const runtime_error& e)
    {
        remote_connection_error = true;
        error(typeid(this).name(), string("Probably no connection err: ") + e.what());
    }

    if(user_from_net.has_value())
    {
        auto&& u = user_from_net.value();
        if(u->id != device->user_id &&  u->status != user::stat::ACTIVE)
        {
            return nullopt;
        }


#ifdef POCKET_FORCE_TIMESTAMP_LAST_UPDATE
        u->timestamp_last_update = POCKET_FORCE_TIMESTAMP_LAST_UPDATE;
#endif
        u->passwd = crypto_encode_sha512(user->passwd);

        dao.persist(u);
        u->passwd = user->passwd;

        view_group = make_unique<view<group>>(u, database, POCKET_ENABLE_AES);
        view_group_field = make_unique<view<group_field>>(u, database, POCKET_ENABLE_AES);
        view_field = make_unique<view<field>>(u, database, POCKET_ENABLE_AES);

        return std::move(u);
    }
    else if(remote_connection_error && !user->name.empty() && user->status == user::stat::ACTIVE)
    {
        view_group = make_unique<view<group>>(user, database, POCKET_ENABLE_AES);
        view_group_field = make_unique<view<group_field>>(user, database, POCKET_ENABLE_AES);
        view_field = make_unique<view<field>>(user, database, POCKET_ENABLE_AES);

        return make_unique<struct user>(*user);
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

bool session::send_data(const std::optional<pods::user::ptr>& user)
{
    if(!user.has_value())
    {
        return false;
    }

    return synchronizer->send_data(user.value());
}


bool session::logout(const optional<user::ptr>& user_opt)
{
    if(!user_opt.has_value())
    {
        return false;
    }
    return synchronizer->invalidate_data(user_opt.value());
}

bool session::export_data(const optional<user::ptr>& user_opt, const std::string_view& file_name, bool enable_aes)
{
    if(file_name.empty())
    {
        return false;
    }

    if(!user_opt.has_value())
    {
        error(typeid(this).name(), "User empty");
        return false;
    }

    auto&& user = user_opt.value();

    if(device->user_id != user->id)
    {
        throw runtime_error("User id not match");
    }

    nlohmann::json json;
    daos::dao dao{database};
    auto aes = services::aes(POCKET_AES_CBC_IV, user->passwd);

    for(auto& group : dao.get_all<group>(0))
    {
        export_data(json, dao, aes, group, enable_aes);
    }

    string full_path_file = config.get()->get_config_path();

    if(!full_path_file.ends_with(path::preferred_separator))
    {
        full_path_file += path::preferred_separator;
    }

    full_path_file += file_name;
    ofstream file(full_path_file);

    if (file.is_open())
    {
        file << json.dump();
        file.close();
        return true;
    }
    else
    {
        return false;
    }



}

bool session::import_data(const std::optional<pods::user::ptr>& user_opt, string full_path_file, bool enable_aes)
{
    if(full_path_file.starts_with("file://"))
    {
        full_path_file = &full_path_file[7];
    }

    if(!exists(full_path_file))
    {
        error(typeid(this).name(), "File not found:" + full_path_file);
        return false;
    }

    if(!user_opt.has_value())
    {
        error(typeid(this).name(), "User empty");
        return false;
    }

    auto&& user = user_opt.value();

    if(device->user_id != user->id)
    {
        throw runtime_error("User id not match");
    }

    daos::dao dao{database};
    auto aes = services::aes(POCKET_AES_CBC_IV, user->passwd);



    return false;
}

void session::export_data(json& json, const daos::dao& dao, const services::aes& aes, const pods::group::ptr& group, bool enable_aes)
{
    if(group->deleted)
    {
        return;
    }
    if(enable_aes)
    {
        group->title = aes.encrypt(group->title);
        group->note = aes.encrypt(group->note);
        group->icon = aes.encrypt(group->icon);
    }
    auto json_group = serialize_json(group, true);

    for(const auto& g : dao.get_all<struct group>(group->id))
    {
        export_data(json_group, dao, aes, g, enable_aes);
    }

    for(const auto& gf : dao.get_all<group_field>(group->id))
    {
        if(gf->deleted)
        {
            continue;
        }
        if(enable_aes)
        {
            gf->title = aes.encrypt(gf->title);
        }
        json_group["group_field"].push_back(serialize_json(gf, true));
    }

    for(const auto& f : dao.get_all<field>(group->id))
    {
        if(f->deleted)
        {
            continue;
        }
        if(enable_aes)
        {
            f->title = aes.encrypt(f->title);
            f->value = aes.encrypt(f->value);
        }
        json_group["field"].push_back(serialize_json(f, true));
    }

    json["groups"].push_back(json_group);
}

void session::lock()
{
#ifndef POCKET_DISABLE_LOCK
    if(config == nullptr)
    {
        return;
    }

    pid_t pid = getpid();
    string&& full_path = config->get_config_path() + device->uuid + LOCK_EXTENSION;
    
    ofstream out(full_path);

    if (!out)
    {
        throw runtime_error("Error: Could not open file for writing.");
    }

    out << pid << endl;

    out.close();
#endif
}

void session::unlock()
{
#ifndef POCKET_DISABLE_LOCK
    string&& full_path = config->get_config_path() + device->uuid + LOCK_EXTENSION;
    if (exists(full_path))
    {
        filesystem::remove(full_path);  //throw exception
    }
    else
    {
        throw runtime_error("File does not exist.");
    }
#endif
}

bool session::check_lock()
{
#ifdef POCKET_DISABLE_LOCK
    return false;
#else
    string&& full_path = config->get_config_path() + device->uuid + LOCK_EXTENSION;
    if (exists(full_path))
    {
        ifstream file(full_path);
        if (!file.is_open())
        {
            throw runtime_error("Error opening file.");
        }

        string pid((istreambuf_iterator<char>(file)), (istreambuf_iterator<char>()));

        file.close();

        info(typeid(*this).name(), "Device locked: " + full_path + " by pid:" + pid);
        return true;
    }
    else
    {
        return false;
    }
#endif
}


}

