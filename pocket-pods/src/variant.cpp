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

#include "pocket-pods/variant.hpp"

namespace pocket::pods::inline v5
{

variant::variant() = default;

variant::variant(int32_t value) noexcept
: t(INT)
, integer_value(value)
{

}

variant::variant(uint32_t value) noexcept
:variant(static_cast<int32_t>(value))
{

}

variant::variant(int64_t value) noexcept
: t(INT64)
, integer_value(value)
{

}

variant::variant(uint64_t value) noexcept
:variant(static_cast<int64_t>(value))
{

}

variant::variant(double value) noexcept
: t(DOUBLE)
, double_value(value)
{

}

variant::variant(const std::string& value) noexcept
: t(TEXT)
, text_value(value)
{

}

variant::variant(const std::string&& value) noexcept
: variant(value)
{

}

variant::variant(nullptr_t) noexcept
: t(NULL_T)
{

}

}
