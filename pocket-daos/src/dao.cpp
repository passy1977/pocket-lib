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

#include "pocket-daos/dao.hpp"
#include "pocket-pods/helpers.hpp"

#include <sstream>
#include <string>

namespace pocket::daos::inline v5
{

using namespace pods;
using namespace std;


void dao::update_all_index(const pods::net_helper& net_helper) const
{

    for(auto&& it: net_helper.groups)
    {
        if(it->group_id > 0 && it->server_group_id == 0)
        {
            if(auto&& g = get<group>(it->group_id); g)
            {
                it->server_group_id = g.value()->server_id;
                persist<group>(it);
            }
        }
    }

    for(auto&& it: net_helper.group_fields)
    {
        if(it->group_id > 0 && it->server_group_id == 0)
        {
            if(auto&& g = get<group>(it->group_id); g)
            {
                it->server_group_id = g.value()->server_id;
                persist<group_field>(it);
            }
        }
    }

    for(auto&& it: net_helper.fields)
    {
        bool perform_persist = false;
        if(it->group_id > 0 && it->server_group_id == 0)
        {
            if(auto&& g = get<group>(it->group_id); g)
            {
                it->server_group_id = g.value()->server_id;
                perform_persist = true;
            }
        }

        if(it->group_field_id > 0 && it->server_group_field_id == 0)
        {
            if(auto&& g = get<group_field>(it->group_field_id); g)
            {
                it->server_group_field_id = g.value()->server_id;
                perform_persist = true;
            }
        }

        if(perform_persist)
        {
            persist<field>(it);
        }

    }

}

}

