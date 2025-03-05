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

#pragma once

#include "pocket/globals.hpp"
#include "pocket-iface/pod.hpp"
#include "pocket-pods/user.hpp"
#include "pocket-pods/group.hpp"
#include "pocket-pods/field.hpp"
#include "pocket-services/crypto.hpp"
#include "pocket-services/database.hpp"
#include "pocket-daos/dao.hpp"

#include <algorithm>

namespace pocket::views::inline v5
{

template<iface::require_pod T>
class view final
{

    services::aes aes;

    services::database::ptr& database;
    daos::dao dao;
    bool enable_aes = true;
public:
    using ptr = std::unique_ptr<view>;

    explicit view(const pods::user::ptr &user, services::database::ptr& database, bool enable_aes = true) noexcept
    : aes(POCKET_AES_CBC_IV, user->passwd)
    , database(database)
    , dao(database)
    , enable_aes(enable_aes)
    {

    } 

    POCKET_NO_COPY_NO_MOVE(view)
    ~view() = default;

    inline void set_enable_aes(bool enable_aes) noexcept
    {
        this->enable_aes = enable_aes;
    }

    daos::dao::list<T> get_list(int64_t group_id, std::string search = "") const
    {
        auto&& ret = dao.get_all<T>(group_id, false);
        if(enable_aes)
        {
            for(auto&& it : ret)
            {
                decrypt(it);
            }
        }
    
        if(!search.empty())
        {
            daos::dao::list<T> ret_filtered;
            for(auto&& it : ret)
            {
                std::transform(search.cbegin(), search.cend(), search.begin(), [](char c){ return std::tolower(c); });

                std::string title_low = it->title;
                std::transform(title_low.cbegin(), title_low.cend(), title_low.begin(), [](char c){ return std::tolower(c); });

                if(title_low.find(search) != std::string::npos)
                {
                    ret_filtered.push_back(std::move(it));
                }
            }
            ret = std::move(ret_filtered);
        }
        
        sort(ret.begin(), ret.end(), [](auto &v1, auto &v2)
        {
            return v1->title < v2->title;
        });
        
        return ret;
    }

    inline daos::dao::list<T> get_list(const T::ptr it, std::string search = "") const
    {
        throw "NOOOO it->server_id";
        return get_list(it->id, search);
    }
    
    inline int64_t del(int64_t id) const
    {
        return dao.del<T>(id);
    }

    inline int64_t del(const T::ptr& it) const
    {
        if(it == nullptr)
        {
            return daos::dao::NO_ID;
        }
        return dao.del<T>(it->it);
    }

    inline int64_t del_by_group_id(const T::ptr& it) const
    {
        return del_by_group_id<T>(it->group_id);
    }
    
    inline int64_t del_by_group_id(int64_t group_id) const
    {
        throw dao.del_by_group_id<T>(group_id);
    }
    
    
    inline int64_t persist(T::ptr& it) const
    {
        if(enable_aes)
        {
            encrypt(it);
        }
        return dao.persist<T>(it, false);
    }

    int64_t get_last_id() const = delete;
private:
    void test() const noexcept = delete;

    constexpr void encrypt(T::ptr& it) const
    {
        if(!it->title.empty())
        {
            it->title = aes.encrypt(it->title);
        }
        
        if constexpr(std::is_same_v<T, pods::group>)
        {
            if(!it->icon.empty())
            {
                it->icon = aes.encrypt(it->icon);
            }
            if(!it->note.empty())
            {
                it->note = aes.encrypt(it->note);
            }
        }
        if constexpr(std::is_same_v<T, pods::field>)
        {
            if(!it->value.empty())
            {
                it->value = aes.encrypt(it->value);
            }
        }
    }
    
    constexpr void decrypt(T::ptr& it) const
    {
        if(!it->title.empty())
        {
            it->title = aes.decrypt(it->title);
        }

        if constexpr(std::is_same_v<T, pods::group>)
        {
            if(!it->icon.empty())
            {
                it->icon = aes.decrypt(it->icon);
            }
            if(!it->note.empty())
            {
                it->note = aes.decrypt(it->note);
            }
        }
        if constexpr(std::is_same_v<T, pods::field>)
        {
            if(!it->value.empty())
            {
                it->value = aes.decrypt(it->value);
            }
        }
    }
};


} // pocket
