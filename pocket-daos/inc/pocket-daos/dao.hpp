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
#include "pocket-services/database.hpp"
#include "pocket-services/result-set.hpp"
#include "pocket-iface/pod.hpp"
#include "pocket-daos/dao-read-write-group.hpp"
#include "pocket-daos/dao-read-write-group-field.hpp"
#include "pocket-daos/dao-read-write-field.hpp"
#include "pocket-pods/net-transport.hpp"

#include <vector>

namespace pocket::daos::inline v5
{

class dao final
{
    services::database::ptr&database;
public:
    template<iface::require_pod T>
    using list = std::vector<typename iface::pod<T>::ptr>;

    static constexpr int64_t NO_ID = -1;

    explicit dao(services::database::ptr& database) noexcept
    : database(database)
    {}
    POCKET_NO_COPY_NO_MOVE(dao)
    ~dao() = default;


    template<iface::require_pod T>
    list<T> get_all(int64_t group_id = -1, bool to_synch = false) const
    {
        std::vector<typename iface::pod<T>::ptr> ret;


        if(auto&& opt_rs = database->execute("SELECT * FROM " + T::get_name() + (to_synch ? " WHERE synchronized = 0" : (group_id < 0 ? " WHERE deleted = 0" : " WHERE deleted = 0 AND group_id = " + std::to_string(group_id))) + " ORDER BY group_id, id"); opt_rs.has_value()) //throw exception
        {
            for(auto&& row : **opt_rs)
            {
                if constexpr (std::is_same_v<T, pods::group>)
                {
                    dao_read_write<pods::group> dao;
                    if(auto&& it = dao.read(row); it.get())
                    {
                        ret.push_back(std::move(it));
                    }
                }
                else if constexpr (std::is_same_v<T, pods::group_field>)
                {
                    dao_read_write<pods::group_field> dao;
                    if(auto&& it = dao.read(row); it.get())
                    {
                        ret.push_back(std::move(it));
                    }
                }
                else if constexpr (std::is_same_v<T, pods::field>)
                {
                    dao_read_write<pods::field> dao;
                    if(auto&& it = dao.read(row); it.get())
                    {
                        ret.push_back(std::move(it));
                    }
                }
            }
        }

        return ret;
    }

    void update_all_index(const pods::net_transport& net_transport);

    template<iface::require_pod T>
    inline int64_t del(int64_t id) const
    {
        return database->update("UPDATE " + T::get_name() + " SET deleted = 1, synchronized = 0 WHERE id = ?", { {id} });
    }

    template<iface::require_pod T>
    inline int64_t del(const T::ptr& t) const
    {
        return del(t->server_id);
    }
    
    template<iface::require_pod T>
    inline int64_t del_by_group_id(int64_t id) const
    {
        return database->update("UPDATE " + T::get_name() + " SET deleted = 1, synchronized = 0 WHERE group_id = ?", { {id} });
    }

    template<iface::require_pod T>
    inline int64_t del_by_group_id(const T::ptr& t) const
    {
        return del_by_group_id(t->server_id);
    }

    template<iface::require_pod T>
    inline int64_t rm(int64_t id) const
    {
        return database->update("DELETE FROM " + T::get_name() + " WHERE deleted = 1 AND id = ?", { {id} });
    }

    template<iface::require_pod T>
    inline int64_t rm(const T::ptr& t) const
    {
        return rm(t->id);
    }

    template<iface::require_pod T>
    inline int64_t rm_by_group_id(const T::ptr& t) const
    {
        return rm_by_group_id(t->id);
    }

    template<iface::require_pod T>
    inline int64_t rm_by_group_id(int64_t group_id) const
    {
        return database->update("DELETE FROM " + T::get_name() + " WHERE deleted = 1 AND group_id = ?", { {group_id} });
    }
    
    template<iface::require_pod T>
    constexpr int64_t persist(const T::ptr& t, bool return_rows_modified = true) const
    {
        if constexpr (std::is_same_v<T, pods::group>)
        {
            return persist<pods::group>(t, return_rows_modified);
        }
        else if constexpr (std::is_same_v<T, pods::group_field>)
        {
            return persist<pods::group_field>(t, return_rows_modified);
        }
        else if constexpr (std::is_same_v<T, pods::field>)
        {
            return persist<pods::field>(t, return_rows_modified);
        }
    }

    template<iface::require_pod T>
    int64_t get_last_id() const { return NO_ID; };
private:
    int64_t get_last_inserted_id() const
    {
        if(auto&& opt_rs = database->execute("SELECT last_insert_rowid() AS id"); opt_rs.has_value()) //throw exception
        {
            if(auto&& it = *opt_rs; !it->empty())
            {
                return (*it->begin())["id"].to_integer();
            }
        }
        return daos::dao::NO_ID;
    }
    
};


} // pocket
