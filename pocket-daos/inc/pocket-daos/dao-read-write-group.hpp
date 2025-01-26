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

#include "pocket-daos/dao-read-write.hpp"
#include "pocket-pods/group.hpp"

#include <stdexcept>

namespace pocket::daos::inline v5
{

template<>
class dao_read_write<pods::group> final : public iface::read_write<services::database::row, pods::group>
{
public:
    dao_read_write() = default;
    ~dao_read_write() override = default;
    POCKET_NO_COPY_NO_MOVE(dao_read_write)

    std::optional<pods::group> read(services::database::row& row) override;

    bool write() override;
};



}