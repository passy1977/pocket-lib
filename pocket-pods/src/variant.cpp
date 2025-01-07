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


variant::variant(int64_t value) noexcept
: type(INTEGER)
, integer_value(value)
{

}

variant::variant(float value) noexcept
: type(FLOAT)

{

}

variant::variant(const std::string& value) noexcept
: type(TEXT)
, text_value(value)
{

}

variant::variant(const std::string&& value) noexcept
: variant(value)
{

}

}