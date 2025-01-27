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

void json_parse_response(BS::thread_pool<6>& pool, string_view response, struct response& json_response) try
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

    promise<vector<group_field::ptr>> prom_groups_fields;
    auto&& fut_groups_fields = prom_groups_fields.get_future();
    pool.detach_task([&prom_groups_fields, &json]
     {
         try
         {
             vector<group_field::ptr> ret;
             for (auto& it : json["groupsFields"].items())
             {
                 ret.push_back(make_unique<group_field>(json_to_group_field(it.value())));
             }

             prom_groups_fields.set_value(std::move(ret));
         }
         catch (const runtime_error& e)
         {
             error(APP_TAG, e.what());
         }
     });


    promise<vector<field::ptr>> prom_fields;
    auto&& fut_fields = prom_fields.get_future();
    pool.detach_task([&prom_fields, &json]
    {
     try
     {
         vector<field::ptr> ret;
         for (auto& it : json["fields"].items())
         {
             ret.push_back(make_unique<field>(json_to_field(it.value())));
         }

         prom_fields.set_value(std::move(ret));
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
    json_response.group_fields = std::move(fut_groups_fields.get());
    json_response.fields = std::move(fut_fields.get());
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
        group.server_id = json["serverId"];
    }
    else
    {
        throw runtime_error("Invalid type or non defined field serverId");
    }

    if(json.contains("groupId") && json["groupId"].is_number())
    {
        group.group_id = json["groupId"];
    }
    else
    {
        throw runtime_error("Invalid type or non defined field groupId");
    }

    if(json.contains("serverGroupId") && json["serverGroupId"].is_number())
    {
        group.server_group_id = json["serverGroupId"];
    }
    else
    {
        throw runtime_error("Invalid type or non defined field serverGroupId");
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

    if(json.contains("timestampCreation") && json["timestampCreation"].is_number())
    {
        group.timestamp_creation = json["timestampCreation"];
    }
    else
    {
        throw runtime_error("Invalid type or non defined field timestampCreation");
    }


    return group;
}



group_field json_to_group_field(const std::string_view& str_json)
{
    if(str_json.empty())
    {
        throw runtime_error("String json empty");
    }

    return json_to_group_field(json::parse(str_json));
}

group_field json_to_group_field(const nlohmann::basic_json<>& json)
{
    if (!json.is_object())
    {
        throw runtime_error("json is not a object");
    }

    group_field group_field;

    if(json.contains("id") && json["id"].is_number())
    {
        group_field.id = json["id"];
    }
    else
    {
        throw runtime_error("Invalid type or non defined field id");
    }

    if(json.contains("serverId") && json["serverId"].is_number())
    {
        group_field.server_id = json["serverId"];
    }
    else
    {
        throw runtime_error("Invalid type or non defined field serverId");
    }

    if(json.contains("groupId") && json["groupId"].is_number())
    {
        group_field.group_id = json["groupId"];
    }
    else
    {
        throw runtime_error("Invalid type or non defined field groupId");
    }

    if(json.contains("serverGroupId") && json["serverGroupId"].is_number())
    {
        group_field.server_group_id = json["serverGroupId"];
    }
    else
    {
        throw runtime_error("Invalid type or non defined field serverGroupId");
    }

    if(json.contains("title") && json["title"].is_string())
    {
        group_field.title = json["title"];
    }
    else
    {
        throw runtime_error("Invalid type or non defined field title");
    }

    if(json.contains("isHidden") && json["isHidden"].is_boolean())
    {
        group_field.is_hidden = json["isHidden"];
    }
    else
    {
        throw runtime_error("Invalid type or non defined field isHidden");
    }

    if(json.contains("deleted") && json["deleted"].is_boolean())
    {
        group_field.deleted = json["deleted"];
    }
    else
    {
        throw runtime_error("Invalid type or non defined field deleted");
    }

    if(json.contains("timestampCreation") && json["timestampCreation"].is_number())
    {
        group_field.timestamp_creation = json["timestampCreation"];
    }
    else
    {
        throw runtime_error("Invalid type or non defined field timestampCreation");
    }

    return group_field;
}















field json_to_field(const std::string_view& str_json)
{
    if(str_json.empty())
    {
        throw runtime_error("String json empty");
    }

    return json_to_field(json::parse(str_json));
}

field json_to_field(const nlohmann::basic_json<>& json)
{
    if (!json.is_object())
    {
        throw runtime_error("json is not a object");
    }

    field field;

    if(json.contains("id") && json["id"].is_number())
    {
        field.id = json["id"];
    }
    else
    {
        throw runtime_error("Invalid type or non defined field id");
    }

    if(json.contains("serverId") && json["serverId"].is_number())
    {
        field.server_id = json["serverId"];
    }
    else
    {
        throw runtime_error("Invalid type or non defined field serverId");
    }

    if(json.contains("groupId") && json["groupId"].is_number())
    {
        field.group_id = json["groupId"];
    }
    else
    {
        throw runtime_error("Invalid type or non defined field groupId");
    }

    if(json.contains("serverGroupId") && json["serverGroupId"].is_number())
    {
        field.server_group_id = json["serverGroupId"];
    }
    else
    {
        throw runtime_error("Invalid type or non defined field serverGroupId");
    }


    if(json.contains("groupFieldId") && json["groupFieldId"].is_number())
    {
        field.group_field_id = json["groupFieldId"];
    }
    else
    {
        throw runtime_error("Invalid type or non defined field groupFieldId");
    }

    if(json.contains("serverGroupFieldId") && json["serverGroupFieldId"].is_number())
    {
        field.server_group_field_id = json["serverGroupFieldId"];
    }
    else
    {
        throw runtime_error("Invalid type or non defined field serverGroupFieldId");
    }

    if(json.contains("title") && json["title"].is_string())
    {
        field.title = json["title"];
    }
    else
    {
        throw runtime_error("Invalid type or non defined field title");
    }

    if(json.contains("value") && json["value"].is_string())
    {
        field.value = json["value"];
    }
    else
    {
        throw runtime_error("Invalid type or non defined field value");
    }

    if(json.contains("isHidden") && json["isHidden"].is_boolean())
    {
        field.is_hidden = json["isHidden"];
    }
    else
    {
        throw runtime_error("Invalid type or non defined field isHidden");
    }

    if(json.contains("deleted") && json["deleted"].is_boolean())
    {
        field.deleted = json["deleted"];
    }
    else
    {
        throw runtime_error("Invalid type or non defined field deleted");
    }

    if(json.contains("timestampCreation") && json["timestampCreation"].is_number())
    {
        field.timestamp_creation = json["timestampCreation"];
    }
    else
    {
        throw runtime_error("Invalid type or non defined field timestampCreation");
    }


    return field;
}

}