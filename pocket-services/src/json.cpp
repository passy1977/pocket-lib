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

#include "pocket-services/json.hpp"


namespace pocket::services::inline v5
{

namespace
{
constexpr char APP_TAG[] = "json";
}

using namespace std;
using namespace nlohmann;
using namespace pods;
using iface::synchronizable;

void json_parse_response(BS::thread_pool<4>& pool, string_view response, struct response& json_response) try
{
    if(response.empty())
    {
        throw runtime_error("String response empty");
    }

    auto&& json = json::parse(response);
    if (!json.is_object())
    {
        throw runtime_error("json is not a object");
    }

    if(json["token"].is_null() || !json["token"].is_string())
    {
        throw runtime_error("token is not a string");
    }

    if(json["user"].is_null() || !json["user"].is_object())
    {
        throw runtime_error("user is not a object");
    }

    if(json["device"].is_null() || !json["device"].is_object())
    {
        throw runtime_error("device is not a object");
    }

    if(json["groups"].is_null() || !json["groups"].is_array())
    {
        throw runtime_error("groups is not a object");
    }

    if(json["groupsFields"].is_null() || !json["groupsFields"].is_array())
    {
        throw runtime_error("groupsFields is not a object");
    }

    if(json["fields"].is_null() || !json["fields"].is_array())
    {
        throw runtime_error("fields is not a object");
    }


    json_response.token = json["token"];

    promise<pair<user::ptr, device::ptr>> prom_user_device;
    auto&& fut_user_device = prom_user_device.get_future();
    pool.detach_task([&prom_user_device, &json]
    {

        try
        {
            auto&& u =  make_unique<struct user>( json_to_user(json["user"]) );
            auto&& d = make_unique<struct device>(json_to_device(json["device"], u->timestamp_last_update));

            d->user_id = u->id;

            prom_user_device.set_value(std::move(
                    pair{std::move(u), std::move(d)}
            ));
        }
        catch (const runtime_error& e)
        {
            error(APP_TAG, e.what());
        }

    });

    promise<vector<group::ptr>> prom_groups;
    auto&& fut_groups = prom_groups.get_future();
    pool.detach_task([&prom_groups, &json]
    {
        try
        {
            vector<group::ptr> ret;
            for (auto& it : json["groups"].items())
            {
                ret.push_back(make_unique<group>(json_to_group(it.value())));
            }

            prom_groups.set_value(std::move(ret));
        }
        catch (const runtime_error& e)
        {
            error(APP_TAG, e.what());
        }
    });

    auto&& [user, device] = fut_user_device.get();
    json_response.user = std::move(user);
    json_response.device = std::move(device);
    json_response.groups = std::move(fut_groups.get());

//    std::vector<group::ptr> groups;
//    for(auto&& it : fut_groups.get())
//    {
//        auto a  = make_unique<pods::group::ptr>(&it);
//        groups.push_back(std::move(a));
//    }
//    json_response.groups.emplace();

//    json_response.groups_fields = std::move(json["groupsFields"]);
//    json_response.fields = std::move(json["fields"]);
    

}
catch (...)
{
    try {
        rethrow_exception(current_exception());
    } catch (const exception& e) {
        throw runtime_error(e.what());
    }
}

device json_to_device(const nlohmann::basic_json<>& json, uint64_t& user_timestamp_last_update)
{
    if (!json.is_object())
    {
        throw runtime_error("json is not a object");
    }

    device device;

    if(json.contains("userId") && json["userId"].is_number())
    {
        device.user_id = json["userId"];
    }

    if(json.contains("uuid") && json["uuid"].is_string())
    {
        device.uuid = json["uuid"];
    }
    else
    {
        throw runtime_error("Invalid type or non defined field uuid");
    }

    if(json.contains("host") && json["host"].is_string())
    {
        device.host = json["host"];
    }

    if(json.contains("hostPublicKey") && json["hostPublicKey"].is_string())
    {
        device.host_pub_key = json["hostPublicKey"];
    }

    if(json.contains("timestampLastUpdate") && json["timestampLastUpdate"].is_number())
    {
        user_timestamp_last_update = json["timestampLastUpdate"];
    }

    if(json.contains("timestampCreation") && json["timestampCreation"].is_number())
    {
        device.timestamp_creation = json["timestampCreation"];
    }

    if(json.contains("status") && json["status"].is_string())
    {
        string&& ref = json["status"];
        if("NOT_ACTIVE" == ref)
        {
            device.status = device::status::NOT_ACTIVE;
        }
        else if("ACTIVE" == ref)
        {
            device.status = device::status::ACTIVE;
        }
        else if("DELETED" == ref)
        {
            device.status = device::status::DELETED;
        }
        else if("INVALIDATED" == ref)
        {
            device.status = device::status::INVALIDATED;
        }

    }
    else
    {
        device.status = device::status::NOT_ACTIVE;
    }

    return device;
}

device json_to_device(const string_view& str_json, uint64_t& user_timestamp_last_update)
{
    if(str_json.empty())
    {
        throw runtime_error("str_json json empty");
    }

    return json_to_device(json::parse(str_json), user_timestamp_last_update);
}

user json_to_user(const nlohmann::basic_json<>& json)
{
    if (!json.is_object())
    {
        throw runtime_error("json is not a object");
    }

    user user;

    if(json.contains("name") && json["name"].is_string())
    {
        user.name = json["name"];
    }
    else
    {
        throw runtime_error("Invalid type or non defined field name");
    }

    if(json.contains("email") && json["email"].is_string())
    {
        user.email = json["email"];
    }
    else
    {
        throw runtime_error("Invalid type or non defined field email");
    }

    if(json.contains("status") && json["status"].is_string())
    {
        string&& ref = json["status"];
        if("NOT_ACTIVE" == ref)
        {
            user.status = user::status::NOT_ACTIVE;
        }
        else if("ACTIVE" == ref)
        {
            user.status = user::status::ACTIVE;
        }
        else if("DELETED" == ref)
        {
            user.status = user::status::DELETED;
        }
        else if("INVALIDATED" == ref)
        {
            user.status = user::status::INVALIDATED;
        }
    }
    else
    {
        user.status = user::status::NOT_ACTIVE;
    }

    return user;
}

user json_to_user(const string_view& str_json)
{

    if(str_json.empty())
    {
        throw runtime_error("String json empty");
    }

    return json_to_user(json::parse(str_json));
}

group json_to_group(const std::string_view& str_json)
{
    if(str_json.empty())
    {
        throw runtime_error("String json empty");
    }

    return json_to_group(json::parse(str_json));
}

group json_to_group(const nlohmann::basic_json<>& json)
{
    if (!json.is_object())
    {
        throw runtime_error("json is not a object");
    }

    group group;

    if(json.contains("id") && json["id"].is_number())
    {
        group.id = json["id"];
    }
    else
    {
        throw runtime_error("Invalid type or non defined field id");
    }

    if(json.contains("serverId") && json["serverId"].is_number())
    {
        group.id = json["serverId"];
    }
    else
    {
        throw runtime_error("Invalid type or non defined field server_id");
    }

    if(json.contains("userId") && json["userId"].is_number())
    {
        group.id = json["userId"];
    }


    if(json.contains("groupId") && json["groupId"].is_number())
    {
        group.id = json["groupId"];
    }

    if(json.contains("serverGroupId") && json["serverGroupId"].is_number())
    {
        group.id = json["serverGroupId"];
    }

    if(json.contains("title") && json["title"].is_string())
    {
        group.title = json["title"];
    }
    else
    {
        throw runtime_error("Invalid type or non defined field title");
    }

    if(json.contains("icon") && json["icon"].is_string())
    {
        group.icon = json["icon"];
    }
    else
    {
        throw runtime_error("Invalid type or non defined field icon");
    }

    if(json.contains("note") && json["note"].is_string())
    {
        group.note = json["note"];
    }
    else
    {
        throw runtime_error("Invalid type or non defined field note");
    }

    if(json.contains("deleted") && json["deleted"].is_boolean())
    {
        group.deleted = json["deleted"];
    }
    else
    {
        throw runtime_error("Invalid type or non defined field deleted");
    }

    if(json.contains("shared") && json["shared"].is_boolean())
    {
        group.shared = json["shared"];
    }

    if(json.contains("timestampCreation") && json["timestampCreation"].is_number())
    {
        group.id = json["timestampCreation"];
    }

    return group;
}

}