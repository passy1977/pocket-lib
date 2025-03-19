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
using namespace tinyxml2;


session::session(const optional<string>& config_json, const optional<string>& config_path)
{
    if(!config_json)
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
    logout(nullopt);
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

    if(!device)
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
    if(!user_opt)
    {
        error(typeid(this).name(), "user empty");
        return nullopt;
    }

    auto&& user = *user_opt;

    if(user == nullptr)
    {
        error(typeid(this).name(), "User nullptr");
        return nullopt;
    }
    
    dao_user dao(database);
    
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

    if(user_from_net)
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

bool session::send_data(const std::optional<pods::user::ptr>& user_opt)
{
    if(secret.empty())
    {
        error(typeid(this).name(), "Session not valid");
        return false;
    }
    
    auto&& user = user_opt.value();

    if(user == nullptr)
    {
        error(typeid(this).name(), "User nullptr");
        return false;
    }
    
    return synchronizer->send_data(user);
}


bool session::logout(const optional<user::ptr>& user_opt)
{
    if(user_opt)
    {
        synchronizer->invalidate_data(user_opt.value());
    }
    
    if(database)
    {
        database->close();
    }
    fill(secret.begin(), secret.end(), 0x00);
    unlock();
    
    config = nullptr;
    database = nullptr;
    synchronizer = nullptr;

    view_group = nullptr;
    view_group_field = nullptr;
    view_field = nullptr;

    device = nullopt;

    status = nullptr;
    
    secret = "";
    
    return true;
}

bool session::export_data(const optional<user::ptr>& user_opt, string full_path_file, bool enable_aes)
{
    if(full_path_file.empty())
    {
        return false;
    }
    
    if(full_path_file.starts_with("file://"))
    {
        full_path_file = &full_path_file[7];
    }

    if(secret.empty())
    {
        error(typeid(this).name(), "Session not valid");
        return false;
    }
    
    if(!user_opt)
    {
        error(typeid(this).name(), "User empty");
        return false;
    }

    auto&& user = user_opt.value();

    if(user == nullptr)
    {
        error(typeid(this).name(), "User nullptr");
        return false;
    }
    
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

    ofstream file(full_path_file);

    if (file.is_open())
    {
        file << json.dump();
        file.close();
        return true;
    }
    else
    {
        error(typeid(this).name(), "File not open:" + full_path_file);
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

    
    if(secret.empty())
    {
        error(typeid(this).name(), "Session not valid");
        return false;
    }
    
    if(!user_opt)
    {
        error(typeid(this).name(), "User empty");
        return false;
    }
    
    auto&& user = user_opt.value();

    if(user == nullptr)
    {
        error(typeid(this).name(), "User nullptr");
        return false;
    }
    
    if(device->user_id != user->id)
    {
        throw runtime_error("User id not match");
    }

    daos::dao dao{database};
    auto aes = services::aes(POCKET_AES_CBC_IV, user->passwd);

    json data = json::parse(ifstream(full_path_file));

    if(data["groups"].is_null() || !data["groups"].is_array())
    {
        return false;
    }

    dao.del_all<field>();
    dao.del_all<group_field>();
    dao.del_all<group>();

    for(auto&& json_group : data["groups"])
    {
        import_data(user, json_group, dao, aes, nullopt, enable_aes);
    }

    return true;
}

bool session::import_data_legacy(const std::optional<pods::user::ptr>& user_opt, std::string full_path_file, bool enable_aes)
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

    if(!user_opt)
    {
        error(typeid(this).name(), "User empty");
        return false;
    }

    if(secret.empty())
    {
        error(typeid(this).name(), "Session not valid");
        return false;
    }
    
    auto&& user = user_opt.value();

    if(!user)
    {
        error(typeid(this).name(), "User nullptr");
        return false;
    }
    
    if(device->user_id != user->id)
    {
        throw runtime_error("User id not match");
    }

    daos::dao dao{database};
    auto aes = services::aes(POCKET_AES_CBC_IV, user->passwd);


    XMLDocument document;
    document.LoadFile(full_path_file.c_str());

    if(document.ErrorID())
    {
        throw runtime_error(document.ErrorStr());
    }

    dao.del_all<field>();
    dao.del_all<group_field>();
    dao.del_all<group>();

    XMLElement *root = document.FirstChildElement("groups");

    XMLElement *element = root->FirstChildElement();

    auto group = make_unique<class group>();
    group->id = 0;

    while (element)
    {
        import_data_legacy_group(user, dao, element, aes, group, enable_aes);

        element = element->NextSiblingElement();
    }

    return true;
}

void session::export_data(json& json, const daos::dao& dao, const services::aes& aes, const pods::group::ptr& group, bool enable_aes) const
{
    if(group->deleted)
    {
        return;
    }
    if(enable_aes)
    {
        group->title = aes.decrypt(group->title);
        group->note = aes.decrypt(group->note);
        group->icon = aes.decrypt(group->icon);
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
            gf->title = aes.decrypt(gf->title);
        }
        json_group["groupsFields"].push_back(serialize_json(gf, true));
    }

    for(const auto& f : dao.get_all<field>(group->id))
    {
        if(f->deleted)
        {
            continue;
        }
        if(enable_aes)
        {
            f->title = aes.decrypt(f->title);
            f->value = aes.decrypt(f->value);
        }
        json_group["fields"].push_back(serialize_json(f, true));
    }

    json["groups"].push_back(json_group);
}

void session::import_data(const pods::user::ptr& user, nlohmann::json& json_group, const daos::dao& dao, const services::aes& aes, std::optional<pods::group*> father, bool enable_aes) const
{
    auto&& g = json_to_group(json_group, true);
    if(g.deleted)
    {
        return;
    }
    if(father)
    {
        g.group_id = father.value()->id;
    }
    if(enable_aes)
    {
        g.title = aes.encrypt(g.title);
        g.note = aes.encrypt(g.note);
        g.icon = aes.encrypt(g.icon);
    }
    g.user_id = user->id;
    g.synchronized = false;
    g.id = dao.persist<pods::group>(make_unique<pods::group>(g), false);

    if(!json_group["groupsFields"].is_null() &&  json_group["groupsFields"].is_array())
    {
        for(auto&& json_group_field : json_group["groupsFields"])
        {
            auto&& gf = json_to_group_field(json_group_field, true);
            if(gf.deleted)
            {
                continue;
            }
            if(enable_aes)
            {
                gf.title = aes.encrypt(g.title);
            }
            gf.group_id = g.id;
            gf.user_id = user->id;
            gf.synchronized = false;
            gf.id = dao.persist<group_field>(make_unique<group_field>(gf), false);
        }
    }

    if(!json_group["fields"].is_null() && json_group["fields"].is_array())
    {
        for(auto&& json_field: json_group["fields"])
        {
            auto&& f = json_to_field(json_field, true);
            if(f.deleted)
            {
                continue;
            }
            if(enable_aes)
            {
                f.title = aes.encrypt(g.title);
            }
            f.group_id = g.id;
            f.user_id = user->id;
            f.synchronized = false;
            f.id = dao.persist<field>(make_unique<field>(f), false);

        }
    }

    if(!json_group["groups"].is_null() && json_group["groups"].is_array())
    {
        for(auto&& jg: json_group["groups"])
        {
            import_data(user, jg, dao, aes, &g, enable_aes);
        }
    }

}

void session::import_data_legacy_group(const pods::user::ptr& user, const daos::dao &dao, const tinyxml2::XMLElement *element, const services::aes& aes, const pods::group::ptr &father, bool enable_aes) const
{
    group::ptr group = nullptr;
    string &&name = element->Name();
    if (name == "group" || name == "entry")
    {

        group = make_unique<struct group>();
        if(enable_aes)
        {
            group->title = aes.encrypt(element->Attribute("title"));
            if (element->FindAttribute("icon"))
            {
                group->icon = aes.encrypt(element->Attribute("icon"));
            }
            if (element->FindAttribute("note"))
            {
                group->note = aes.encrypt(element->Attribute("note"));
            }
        }
        else
        {
            group->title = element->Attribute("title");
            if (element->FindAttribute("icon"))
            {
                group->icon = element->Attribute("icon");
            }
            if (element->FindAttribute("note"))
            {
                group->note = element->Attribute("note");
            }
        }
        
        group->group_id = father->id;
        group->synchronized = false;
        group->user_id = user->id;
        
        group->id = dao.persist<struct group>(group, true);

        auto child = element->FirstChildElement();
        while(child)
        {

            import_data_legacy_group(user, dao, child, aes, group, enable_aes);

            child = child->NextSiblingElement();
        }

    }
    else if (name == "groupField")
    {
        if(group == nullptr)
        {
            group = make_unique<class group>(*father);
        }
        import_data_legacy_group_field(user, dao, element, aes, group, enable_aes);
    }
    else if (name == "field")
    {
        if(group == nullptr)
        {
            group = make_unique<class group>(*father);
        }
        import_data_legacy_field(user, dao, element, aes, group, enable_aes);
    }
}

void session::import_data_legacy_group_field(const pods::user::ptr& user, const daos::dao &dao, const tinyxml2::XMLElement *element, const services::aes& aes, const pods::group::ptr &father, bool enable_aes) const
{
    auto group_field = make_unique<struct group_field>();
    if(enable_aes)
    {
        group_field->title = aes.encrypt(element->Attribute("title"));
    
    }
    else
    {
	group_field->title = element->Attribute("title");
    }    
    if (element->FindAttribute("hidden"))
    {
        group_field->is_hidden = element->BoolAttribute("hidden");
    }
    group_field->group_id = father->id;
    group_field->synchronized = false;
    group_field->user_id = user->id;

    dao.persist<struct group_field>(group_field);
}

void session::import_data_legacy_field(const pods::user::ptr& user, const daos::dao &dao, const tinyxml2::XMLElement *element, const services::aes& aes, const pods::group::ptr &father, bool enable_aes) const
{
    auto field = make_unique<struct field>();
    if(enable_aes)
    {
        field->title = aes.encrypt(element->Attribute("title"));
        if (element->FindAttribute("value"))
	{
    	    field->value = aes.encrypt(element->Attribute("value"));
        }
    }
    else
    {
	field->title = element->Attribute("title");
	if (element->FindAttribute("value"))
	{
    	    field->value = element->Attribute("value");
        }
    }    

    if (element->FindAttribute("hidden"))
    {
        field->is_hidden = element->BoolAttribute("hidden");
    }
    field->group_id = father->id;
    field->synchronized = false;
    field->user_id = user->id;

    dao.persist<struct field>(field);
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

