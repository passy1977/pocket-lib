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

    daos::dao::list<T> get_list(int64_t group_id) const
    {
        auto&& ret = dao.get_all<T>(group_id, false);
        for(auto&& it : ret)
        {
            if(enable_aes)
            {
                decode(it);
            }
        }
        return ret;
    }

    inline daos::dao::list<T> get_list(const T::ptr it) const
    {
        return get_list(it->group_id);
    }

    inline int64_t del(int64_t group_id) const
    {
        return dao.del<T>(group_id);
    }

    inline int64_t del(const T::ptr& it) const
    {
        if(it == nullptr)
        {
            return daos::dao::NO_ID;
        }
        return dao.del<T>(it->it);
    }

    int64_t store(const T::ptr& it) const
    {
        return dao.persist<T>(it);
    }


private:
    void test() const noexcept = delete;

    constexpr void decode(T::ptr& it) const
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
