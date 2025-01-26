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
    constexpr std::vector<iface::pod<T>> get_all()
    {
        std::vector<iface::pod<T>> ret;


        if(auto&& opt_rs = database->execute("SELECT * FROM " + T::get_name() + " WHERE deleted = 0"); opt_rs.has_value())
        {
            for(auto&& row : **opt_rs)
            {
                if constexpr (std::is_same_v<T, pods::group>)
                {
                    dao_read_write<pods::group> dao;
                    if(auto&& g = dao.read(row); g.has_value())
                    {
                        ret.push_back(*g);
                    }
                }
            }
        }

        return ret;
    }

};



} // pocket
