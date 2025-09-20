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
#include <cstring>
#include <string>

namespace pocket::pods::inline v5
{


class variant final
{
public:
    using ptr = std::unique_ptr<variant>;

    enum class type
    {
        INT = 1,
        DOUBLE,
        TEXT,
        INT64,
        NULL_T,
    };
    using enum variant::type;

private:
    variant::type t = NULL_T;
    int64_t integer_value = 0;
    double double_value = 0;
    std::string text_value;
public: //keep the ctor not explicit
    variant();
    variant(int32_t value) noexcept;
    variant(uint32_t value) noexcept;
    variant(int64_t value) noexcept;
    variant(uint64_t value) noexcept;
    variant(double value) noexcept;
    variant(const std::string& value) noexcept;
    variant(const std::string&& value) noexcept;
    variant(std::nullptr_t) noexcept;

    [[nodiscard]] inline int64_t to_integer() const noexcept
    {
        return integer_value;
    }

    [[nodiscard]] inline double to_float() const noexcept
    {
        return double_value;
    }

    [[nodiscard]] inline std::string to_text() const noexcept
    {
        switch(t)
        {
            default: return text_value;
            case INT: return std::to_string(integer_value);
            case DOUBLE: return std::to_string(double_value);
            case INT64: return std::to_string(integer_value);
        }
    }

    [[nodiscard]] inline enum type get_type() const noexcept
    {
        return t;
    }

};

}
