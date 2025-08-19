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

using namespace std;
using namespace nlohmann;
using namespace pods;
using iface::synchronizable;
using BS::thread_pool;


namespace
{
constexpr char APP_TAG[] = "json";
}

void json_parse_net_helper(BS::thread_pool<>& pool, string_view json_response, pods::net_helper& net_helper) try
{
    if(json_response.empty())
    {
        throw runtime_error("String response empty");
    }

    auto&& json = json::parse(json_response);
    if (!json.is_object())
    {
        throw runtime_error("json is not a object");
    }

    if(json["timestampLastUpdate"].is_null() || !json["timestampLastUpdate"].is_number())
    {
        throw runtime_error("timestampLastUpdate is not a number");
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

    if(json["groupFields"].is_null() || !json["groupFields"].is_array())
    {
        throw runtime_error("groupFields is not a object");
    }

    if(json["fields"].is_null() || !json["fields"].is_array())
    {
        throw runtime_error("fields is not a object");
    }



    auto&& fut_user_device = pool.submit_task([&json]
    {
        try
        {
            auto&& u =  make_unique<struct user>( json_to_user(json["user"]) );
            auto&& d = make_unique<struct device>(json_to_device(json["device"]));

            u->timestamp_last_update = json["timestampLastUpdate"];
            d->user_id = u->id;

            return pair{std::move(u), std::move(d)};
        }
        catch (const runtime_error& e)
        {
            error(APP_TAG, e.what());
            return pair<user::ptr, device::ptr>{nullptr, nullptr};
        }
    });

    auto&& fut_groups = pool.submit_task([&json]
    {
        try
        {
            vector<group::ptr> ret;
            for (auto& it : json["groups"].items())
            {
                ret.push_back(make_unique<group>(json_to_group(it.value())));
            }

            return ret;
        }
        catch (const runtime_error& e)
        {
            error(APP_TAG, e.what());
            return  vector<group::ptr>{};
        }
    });


    auto&& fut_group_fields = pool.submit_task([&json]
     {
         try
         {
             vector<group_field::ptr> ret;
             for (auto& it : json["groupFields"].items())
             {
                 ret.push_back(make_unique<group_field>(json_to_group_field(it.value())));
             }

             return ret;
         }
         catch (const runtime_error& e)
         {
             error(APP_TAG, e.what());
             return vector<group_field::ptr>{};
         }
     });


    auto&& fut_fields = pool.submit_task([&json]
    {
        try
     {
         vector<field::ptr> ret;
         for (auto& it : json["fields"].items())
         {
             ret.push_back(make_unique<field>(json_to_field(it.value())));
         }

         return ret;
     }
     catch (const runtime_error& e)
     {
         error(APP_TAG, e.what());
         return vector<field::ptr>{};
     }
    });


    auto&& [user, device] = fut_user_device.get();
    net_helper.user = std::move(user);
    net_helper.device = std::move(device);
    net_helper.groups = fut_groups.get();
    net_helper.group_fields = fut_group_fields.get();
    net_helper.fields = fut_fields.get();
}
catch (const runtime_error& e)
{
    throw;
}
catch (...)
{
    cerr << "Unhandled exception" << endl;

    auto exception = current_exception();

    if (exception)
    {
        try
        {
            rethrow_exception(exception);
        }
        catch (const runtime_error& e)
        {
            cout << e.what() << endl;
        }
    }
}

string net_helper_serialize_json(const pods::net_helper& net_helper) try
{
    json j;

    auto groups = json::array();
    for(auto&& it : net_helper.groups)
    {
        groups.push_back(serialize_json(it));
    }
    j["groups"] = groups;

    auto group_fields = json::array();
    for(auto&& it : net_helper.group_fields)
    {
        group_fields.push_back(serialize_json(it));
    }
    j["groupFields"] = group_fields;

    auto fields = json::array();
    for(auto&& it : net_helper.fields)
    {
        fields.push_back(serialize_json(it));
    }
    j["fields"] = fields;

    return j.dump();
}
catch (const runtime_error& e)
{
    throw;
}
catch (const exception& e)
{
    cerr << "Unhandled exception" << endl;

    auto eptr = current_exception();

    if (eptr)
    {
        try
        {
            rethrow_exception(eptr);
        }
        catch (const runtime_error& e)
        {
            cout << e.what() << endl;
        }
    }

    return "700";
}

device json_to_device(const json& json)
{
    if (!json.is_object())
    {
        throw runtime_error("json is not a object");
    }

    device device;

    if(json.contains("id") && json["id"].is_number())
    {
        device.id = json["id"];
    }
    else
    {
        throw runtime_error("Invalid type or non defined field id");
    }

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

    if(json.contains("timestampCreation") && json["timestampCreation"].is_number())
    {
        device.timestamp_creation = json["timestampCreation"];
    }

    if(json.contains("status") && json["status"].is_string())
    {
        string&& ref = json["status"];
        if("NOT_ACTIVE" == ref)
        {
            device.status = device::stat::NOT_ACTIVE;
        }
        else if("ACTIVE" == ref)
        {
            device.status = device::stat::ACTIVE;
        }
        else if("DELETED" == ref)
        {
            device.status = device::stat::DELETED;
        }
        else if("INVALIDATED" == ref)
        {
            device.status = device::stat::INVALIDATED;
        }

    }
    else
    {
        device.status = device::stat::NOT_ACTIVE;
    }

    return device;
}

device json_to_device(const string_view& str_json)
{
    if(str_json.empty())
    {
        throw runtime_error("str_json json empty");
    }

    return json_to_device(json::parse(str_json));
}

std::string json_to_aes_cbc_iv(const std::string_view& str_json)
{
    if(str_json.empty())
    {
        throw runtime_error("str_json json empty");
    }

    const auto& json = json::parse(str_json);

    if (!json.is_object())
    {
        throw runtime_error("json is not a object");
    }

    string ret;
    if(json.contains("aesCbcIv") && json["aesCbcIv"].is_string())
    {
        ret = json["aesCbcIv"];
    }
    else
    {
        throw runtime_error("Invalid type or non defined field aesCbcIv");
    }

    return ret;
}

user json_to_user(const json& json)
{
    if (!json.is_object())
    {
        throw runtime_error("json is not a object");
    }

    user user;

    if(json.contains("id") && json["id"].is_number())
    {
        user.id = json["id"];
    }
    else
    {
        throw runtime_error("Invalid type or non defined field id");
    }

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
            user.status = user::stat::NOT_ACTIVE;
        }
        else if("ACTIVE" == ref)
        {
            user.status = user::stat::ACTIVE;
        }
        else if("DELETED" == ref)
        {
            user.status = user::stat::DELETED;
        }
        else if("INVALIDATED" == ref)
        {
            user.status = user::stat::INVALIDATED;
        }
    }
    else
    {
        user.status = user::stat::NOT_ACTIVE;
    }

    return user;
}


//group json_to_group(const std::string_view& str_json)
//{
//    if(str_json.empty())
//    {
//        throw runtime_error("String json empty");
//    }
//
//    return json_to_group(json::parse(str_json));
//}

group json_to_group(const json& json, bool no_id)
{
    if (!json.is_object())
    {
        throw runtime_error("json is not a object");
    }

    group group;

    if(!no_id)
    {
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

json serialize_json(const group::ptr& group, bool no_id)
{
    if(group == nullptr)
    {
        throw runtime_error("group null");
    }

    json j;

    if(!no_id)
    {
        j["id"] = group->id;
        j["serverId"] = group->server_id;
        j["groupId"] = group->group_id;
        j["serverGroupId"] = group->server_group_id;
    }
    j["title"] = group->title;
    j["icon"] = group->icon;
    j["note"] = group->note;
    j["synchronized"] = group->synchronized;
    j["deleted"] = group->deleted;
    j["timestampCreation"] = group->timestamp_creation;

    return j;
}

//group_field json_to_group_field(const std::string_view& str_json)
//{
//    if(str_json.empty())
//    {
//        throw runtime_error("String json empty");
//    }
//
//    return json_to_group_field(json::parse(str_json));
//}

group_field json_to_group_field(const json& json, bool no_id)
{
    if (!json.is_object())
    {
        throw runtime_error("json is not a object");
    }

    group_field group_field;

    if(!no_id)
    {
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

json serialize_json(const group_field::ptr& group_field, bool no_id)
{
    if(group_field == nullptr)
    {
        throw runtime_error("group_field null");
    }

    json j;

    if(!no_id)
    {
        j["id"] = group_field->id;
        j["serverId"] = group_field->server_id;
        j["groupId"] = group_field->group_id;
        j["serverGroupId"] = group_field->server_group_id;
    }
    j["title"] = group_field->title;
    j["isHidden"] = group_field->is_hidden;
    j["synchronized"] = group_field->synchronized;
    j["deleted"] = group_field->deleted;
    j["timestampCreation"] = group_field->timestamp_creation;

    return j;
}

//field json_to_field(const std::string_view& str_json)
//{
//    if(str_json.empty())
//    {
//        throw runtime_error("String json empty");
//    }
//
//    return json_to_field(json::parse(str_json));
//}

field json_to_field(const json& json, bool no_id)
{
    if (!json.is_object())
    {
        throw runtime_error("json is not a object");
    }

    field field;

    if(!no_id)
    {
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

json serialize_json(const field::ptr& field, bool no_id)
{
    if(field == nullptr)
    {
        throw runtime_error("group null");
    }

    json j;

    if(!no_id)
    {
        j["id"] = field->id;
        j["serverId"] = field->server_id;
        j["groupId"] = field->group_id;
        j["serverGroupId"] = field->server_group_id;
        j["groupFieldId"] = field->group_field_id;
        j["serverGroupFieldId"] = field->server_group_field_id;
    }
    j["title"] = field->title;
    j["value"] = field->value;
    j["isHidden"] = field->is_hidden;
    j["synchronized"] = field->synchronized;
    j["deleted"] = field->deleted;
    j["timestampCreation"] = field->timestamp_creation;

    return j;
}

}
