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

#include "pocket/globals.hpp"

#include <algorithm>

namespace pocket::inline v5
{

using namespace std;
    
void str_replace_all(string &s, const string_view &to_replace, const string_view &replacement) noexcept
{
    size_t pos = 0;
    while ((pos = s.find(to_replace, pos)) != string_view::npos)
    {
        s.replace(pos, to_replace.length(), replacement);
        pos += replacement.length();
    }
}

string& ltrim(string &s) noexcept
{
    s.erase(s.begin(), find_if(s.begin(), s.end(), [](unsigned char ch){ return !isspace(ch); }));
    return s;
}

string& rtrim(string &s) noexcept
{
    s.erase(find_if(s.rbegin(), s.rend(), [](unsigned char ch){ return !isspace(ch); }).base(), s.end());
    return s;
}

string& trim(string &s) noexcept
{
    return ltrim(rtrim(s));
}
    
time_t get_current_time_GMT() noexcept
{
    auto now = std::chrono::system_clock::now();
    auto now_ms = std::chrono::time_point_cast<std::chrono::milliseconds>(now);

    auto epoch = now_ms.time_since_epoch();
    auto value = std::chrono::duration_cast<std::chrono::seconds>(epoch);

    return value.count();
}
    
}
