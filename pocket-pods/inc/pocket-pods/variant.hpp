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

#include <memory>
#include <optional>

namespace pocket::pods::inline v5
{


class variant final
{
public:
    using ptr = std::unique_ptr<variant>;

    enum class type
    {
        INT = 1,
        FLOAT,
        TEXT,
        INT64,
    };
    using enum variant::type;

private:
    variant::type t;
    int64_t integer_value = 0;
    float float_value = 0;
    std::string text_value;
public:
    explicit variant(int32_t value) noexcept;
    explicit variant(int64_t value) noexcept;
    explicit variant(float value) noexcept;
    explicit variant(const std::string& value) noexcept;
    explicit variant(const std::string&& value) noexcept;

    inline int64_t to_integer() const noexcept
    {
        return integer_value;
    }

    inline float to_float() const noexcept
    {
        return float_value;
    }

    inline std::string to_text() const noexcept
    {
        return text_value;
    }

    inline enum type get_type() const noexcept
    {
        return t;
    }

};

}