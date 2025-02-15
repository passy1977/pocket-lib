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
#include "pocket-pods/group.hpp"

#include <vector>
#include <map>



namespace pocket::inline v5
{

class tree final
{

    int16_t level = -1;
    mutable std::map<uint64_t, std::pair<uint8_t, pods::group::ptr>> container; //id <level, group>
    mutable std::vector<std::vector<uint64_t>> idx; //[level] -> idx of level
public:
    tree() = default;
    POCKET_NO_COPY_NO_MOVE(tree)


    bool operator+(pods::group::ptr& group) noexcept;

    std::vector<pods::group::ptr> get() const noexcept;


};

}