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

#include "pocket-iface/synchronizable.hpp"

#include <memory>
#include <optional>
#include <string>


namespace pocket::pods::inline v5
{

struct device final : public iface::synchronizable<device>
{
    using opt = std::optional<device>;
    using ptr = std::unique_ptr<device>;

    enum class status {
        NOT_ACTIVE = 1,
        ACTIVE = 0,
        DELETED = 2,
        INVALIDATED = 3
    };

    std::string uuid;
    std::string version;
    std::string token;
    std::string host;
    std::string host_pub_key;
    uint64_t timestamp_creation = 0;
    status status = status::NOT_ACTIVE;

    std::string secret;

    ~device() override;
};

} // pocket
