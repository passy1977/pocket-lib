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
#include "pocket-iface/read-write.hpp"


#include <stdexcept>

namespace pocket::daos::inline v5
{


template<iface::require_pod T>
class dao_read_write final : public iface::read_write<services::database::row, services::database::parameters, T>
{
public:
    dao_read_write() = default;
    ~dao_read_write() override = default;
    POCKET_NO_COPY_NO_MOVE(dao_read_write)

    T::ptr read(services::database::row& row) override
    {
        throw std::runtime_error("Not implemented");
    }

    services::database::parameters write(const T::ptr& t) override
    {
        throw std::runtime_error("Not implemented");
    }
};

}