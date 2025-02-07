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
#include <iostream>
#include <vector>

#define POCKET_ANSI_COLOR_RED     "\x1b[31m"
#define POCKET_ANSI_COLOR_GREEN   "\x1b[32m"
#define POCKET_ANSI_COLOR_YELLOW  "\x1b[33m"
#define POCKET_ANSI_COLOR_BLUE    "\x1b[34m"
#define POCKET_ANSI_COLOR_MAGENTA "\x1b[35m"
#define POCKET_ANSI_COLOR_CYAN    "\x1b[36m"
#define POCKET_ANSI_COLOR_RESET   "\x1b[0m"
#define POCKET_NEW_LINE   "\n"

#define POCKET_NO_COPY_NO_MOVE(clazz) \
clazz(const clazz&) = delete; \
clazz& operator=(const clazz&) = delete; \
clazz(clazz&&) = delete; \
clazz& operator=(clazz&&) = delete;


namespace pocket::inline v5
{
    constexpr char DATA_FOLDER[] = ".pocket";
    constexpr char LOCK_EXTENSION[] = ".lock";
    constexpr char API_VERSION[] = "/api/v5";
    constexpr char DIVISOR[] = "|";

    constexpr auto debug(const std::string& app_tag, const std::string& log) noexcept
    {
       std::printf(POCKET_ANSI_COLOR_BLUE "[%s] %s" POCKET_ANSI_COLOR_RESET POCKET_NEW_LINE, app_tag.c_str(), log.c_str());
    }

    constexpr inline auto debug(const std::string& app_tag, const std::string&& log) noexcept
    {
        debug(app_tag, log);
    }

    constexpr auto info(const std::string& app_tag, const std::string& log) noexcept
    {
        std::printf(POCKET_ANSI_COLOR_GREEN "[%s] %s" POCKET_ANSI_COLOR_RESET POCKET_NEW_LINE, app_tag.c_str(), log.c_str());
    }

    constexpr inline auto info(const std::string& app_tag, const std::string&& log) noexcept
    {
        info(app_tag, log);
    }

    constexpr auto error(const std::string& app_tag, const std::string& log) noexcept
    {
        std::printf(POCKET_ANSI_COLOR_RED "[%s] %s" POCKET_ANSI_COLOR_RESET POCKET_NEW_LINE, app_tag.c_str(), log.c_str());
    }

    constexpr auto error(const std::string& app_tag, const std::string&& log) noexcept
    {
        error(app_tag, log);
    }

    void str_replace_all(std::string &s, const std::string_view &to_replace, const std::string_view &replacement) noexcept;

    template<typename T>
    void vector_copy_unique_ptr(const std::vector<typename T::ptr>& src, std::vector<std::unique_ptr<T>>& dst) noexcept
    {
        for (const auto&& ptr : src)
        {
            if (ptr)
            {
                dst.push_back(ptr->clone());
            }
        }
    }

    template<typename T>
    void vector_copy_ref(const std::vector<typename T::ptr>& src, std::vector<T*>& dst) noexcept
    {
        for (auto& ptr : src)
        {
            if (ptr)
            {
                dst.push_back(ptr.get());
            }
        }
    }
}