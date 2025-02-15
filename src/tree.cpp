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

#include "pocket/tree.hpp"

#include <memory>

namespace pocket::inline v5
{

using namespace std;
using pods::group;

bool tree::operator+(group::ptr& group) noexcept
{
    if(group->id == 0)
    {
        return false;
    }

    if(container.contains(group->id))
    {
        return false;
    }

    if(container.contains(group->group_id))
    {
        auto&& [father_level, father ]= container[group->group_id];
        if(father_level + 1 > level)
        {
            idx.emplace_back();
            level++;
        }

        idx[level].push_back(group->id);

        container[group->id] = {level, make_unique<class group>(*group)};
        return true;
    }
    else
    {
        if(level == -1)
        {
            idx.emplace_back();
            level = 0;
        }
        idx[0].push_back(group->id);

        container[group->id] = {0, make_unique<class group>(*group)};
        return true;
    }
}

vector<group::ptr> tree::get() const noexcept
{

    vector<group::ptr> ret;

    for(const auto& level : idx)
    {
        for(const auto& group_id : level)
        {
            auto&& g = container[group_id].second;
            ret.push_back(std::move(g));
        }
    }
    return ret;
}


}

