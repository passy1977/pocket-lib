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

#include "pocket-daos/dao.hpp"
#include "pocket-pods/group.hpp"

namespace pocket::daos::inline v5
{

template<>
dao::list<pods::group> dao::get_all<pods::group>(int64_t group_id, bool to_synch) const;

template<>
int64_t dao::persist<pods::group>(const pods::group::ptr& t, bool return_rows_modified) const;



}
