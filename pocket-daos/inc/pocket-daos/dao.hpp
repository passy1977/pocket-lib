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

#include <vector>

namespace pocket::daos::inline v5
{


class dao final
{
    services::database::ptr&database;
public:
    explicit dao(services::database::ptr& database) noexcept
    : database(database)
    {}
    POCKET_NO_COPY_NO_MOVE(dao)
    ~dao() = default;


    template<iface::require_pod T>
    constexpr std::vector<typename iface::pod<T>::ptr> get_all() const
    {
        std::vector<typename iface::pod<T>::ptr> ret;


        if(auto&& opt_rs = database->execute("SELECT * FROM " + T::get_name() + " WHERE deleted = 0"); opt_rs.has_value()) //throw exception
        {
            for(auto&& row : **opt_rs)
            {
                if constexpr (std::is_same_v<T, pods::group>)
                {
                    dao_read_write<pods::group> dao;
                    if(auto&& it = dao.read(row); it.get())
                    {
//                        ret.push_back(*g);
                    }
                }
                else if constexpr (std::is_same_v<T, pods::group_field>)
                {
                    dao_read_write<pods::group_field> dao;
                    if(auto&& it = dao.read(row); it.get())
                    {
//                        ret.push_back(*g);
                    }
                }
                else if constexpr (std::is_same_v<T, pods::field>)
                {
                    dao_read_write<pods::field> dao;
                    if(auto&& it = dao.read(row); it.get())
                    {
                        //ret.push_back(*g);
                    }
                }
            }
        }

        return ret;
    }

    void update_all_index();

    template<iface::require_pod T>
    inline int64_t del(uint64_t id) const
    {
        return database->update("UPDATE " + T::get_name() + " SET deleted = 1 WHERE id = ?", { {id} });
    }

    template<iface::require_pod T>
    inline int64_t del(const T::ptr& t) const
    {
        return del(t->server_id);
    }

    template<iface::require_pod T>
    inline int64_t rm(uint64_t server_id) const
    {
        return database->update("DELETE FROM " + T::get_name() + " WHERE deleted = 1 AND server_id = ?", { {server_id} });
    }

    template<iface::require_pod T>
    inline int64_t rm(const T::ptr& t) const
    {
        return rm(t->id);
    }

    template<iface::require_pod T>
    uint64_t persist(const T::ptr& t)
    {
        throw std::runtime_error("Not implemented");
    }

};



} // pocket
