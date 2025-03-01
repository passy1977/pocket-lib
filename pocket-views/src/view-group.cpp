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

#include "pocket-views/view-group.hpp"


namespace pocket::views::inline v5
{

template<>
inline daos::dao::list<group> view<group>::get_list(const group::ptr it, const std::string_view& search) const
{
    if(it == nullptr)
    {
        return {};
    }
    return get_list(it->id, search);
}

template<>
void view<group>::test() const noexcept
{

}


}

