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

#include "pocket-services/network.hpp"

namespace pocket::services::inline v5
{
using namespace std;
using namespace pods;

network::network()
: curl(curl_easy_init())
{
    if(!curl)
    {
        throw runtime_error("Impossible init curl");
    }
}

network::~network()
{
    if (curl)
    {
        curl_easy_cleanup(curl);
        curl = nullptr;
    }
}

size_t network::callback(char* buf, size_t size, size_t nmemb, string* ret_data)
{
    try
    {
        ret_data->append(buf, size * nmemb);
    }
    catch(bad_alloc &e)
    {
        cerr << e.what() << endl;
        return 0;
    }

    return size * nmemb;
}

std::string network::perform(network::method method, const std::string_view& url, const map_parameters& params, const std::string_view& json_data)
{
    if (headers)
    {
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    }

    string full_url;
    if (url.rfind("http://", 0) == 0)
    {
        curl_easy_setopt(curl, CURLOPT_DEFAULT_PROTOCOL, "http");
        full_url = url;
    }
    else if (url.rfind("https://", 0) == 0)
    {
        curl_easy_setopt(curl, CURLOPT_DEFAULT_PROTOCOL, "https");
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, true);
        full_url = url;
    }
    else
    {
        full_url = "http://";
        full_url += url;
    }

    parameters rows;
    string query;
    for (auto&& it = params.begin(); it!=params.end(); ++it)
    {
        if (it == params.begin())
        {
            query += "?";
        }
        else
        {
            query += "&";
        }
        auto key = curl_easy_escape(curl, it->first.c_str(), static_cast<int>(it->first.size()));
        if(key == nullptr)
        {
            throw runtime_error("key nullptr");
        }
        auto value = curl_easy_escape(curl, it->second.to_text().c_str(), static_cast<int>(it->second.to_text().size()));
        if(value == nullptr)
        {
            throw runtime_error("key nullptr");
        }

        string row = key;
        row += "=";
        row += value;
        rows.emplace_back(row);

        query += row;
        curl_free(key);
        curl_free(value);
    }

    switch (method)
    {
        case method::GET:
            curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "GET");
            info(typeid(*this).name(), full_url);
            break;

        case method::POST:
            curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "POST");
            curl_easy_setopt(curl, CURLOPT_POSTFIELDS, query.c_str());
            info(typeid(*this).name(), full_url);
            break;

        case method::PUT:
            curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "PUT");
            curl_easy_setopt(curl, CURLOPT_POSTFIELDS, query.c_str());
            info(typeid(*this).name(), full_url);
            break;

        case method::DEL:
            curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "DELETE");
            full_url += query;
            info(typeid(*this).name(), full_url);
            break;
    }

    curl_easy_setopt(curl, CURLOPT_URL, full_url.c_str());

    if (!json_data.empty())
    {
        char response[10240];
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, json_data.data());
        curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, json_data.size());
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, curl_slist_append(NULL, "Content-Type: application/json"));
    }


    string ret_data;
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, callback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &ret_data);

    auto res = curl_easy_perform(curl);
    if(res != CURLE_OK)
    {
        if (headers)
        {
            curl_slist_free_all(headers);
            headers = nullptr;
        }
        throw runtime_error(curl_easy_strerror(res));
    }

    if (headers)
    {
        curl_slist_free_all(headers);
        headers = nullptr;
    }

    long http_code;
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);
    if(http_code != 200)
    {
        throw runtime_error(to_string(http_code));
    }

    return ret_data;
}

}