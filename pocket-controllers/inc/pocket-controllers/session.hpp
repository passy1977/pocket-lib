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

#include "pocket-controllers/config.hpp"
#include "pocket-services/database.hpp"
#include "pocket-services/synchronizer.hpp"
#include "pocket-services/crypto.hpp"
#include "pocket-services/json.hpp"
#include "pocket-pods/user.hpp"
#include "pocket-pods/device.hpp"
#include "pocket/globals.hpp"
#include "pocket-views/view.hpp"

#include "tinyxml2.h"

#include <optional>


#ifndef POCKET_ENABLE_AES
#warning AES disabled, data stored non safe
#define POCKET_ENABLE_AES (0)
#endif


namespace pocket::controllers::inline v5
{

class session final
{


    controllers::config::ptr config = nullptr;
    services::database::ptr database = nullptr;
    services::synchronizer::ptr synchronizer = nullptr;

    views::view<pods::group>::ptr view_group = nullptr;
    views::view<pods::group_field>::ptr view_group_field = nullptr;
    views::view<pods::field>::ptr view_field = nullptr;

    std::string secret;
    std::string aes_cbc_iv;
    pods::device::opt device;

    const services::synchronizer::stat* status = nullptr;
    bool offline = false;
public:
    explicit session(const std::optional<std::string>& config_json, const std::optional<std::string>& config_path = {});
    ~session();
    POCKET_NO_COPY_NO_MOVE(session)

    const pods::device::opt& init();

    std::optional<pods::user::ptr> login(const std::string& email, const std::string& passwd, bool enable_aes = true);

    std::optional<pods::user::ptr> retrieve_data(const std::optional<pods::user::ptr>& user_opt, bool enable_aes = true);

    std::optional<pods::user::ptr> send_data(const std::optional<pods::user::ptr>& user_opt);

    std::optional<pods::user::ptr> change_passwd(const std::optional <pods::user::ptr>& user_opt, const std::string_view& full_path_file, const std::string_view& new_passwd, bool enable_aes = true, bool change_passwd_data_on_server = true);

    bool logout(const std::optional<pods::user::ptr>& user_opt);
    
    bool soft_logout(const std::optional<pods::user::ptr>& user_opt);
    
    bool export_data(const std::optional<pods::user::ptr>& user_opt, std::string full_path_file, bool enable_aes = true);

    bool import_data(const std::optional<pods::user::ptr>& user_opt, std::string full_path_file, bool enable_aes = true);

    bool import_data_legacy(const std::optional<pods::user::ptr>& user_opt, std::string full_path_file, bool enable_aes = true);

    inline const std::string& get_aes_cbc_iv() const noexcept
    {
        return aes_cbc_iv;
    }

    inline services::synchronizer::stat get_status() const noexcept
    {
        return *status;
    }
    
    inline const views::view<pods::group>::ptr& get_view_group() const noexcept
    {
        return view_group;
    }

    inline const views::view<pods::group_field>::ptr& get_view_group_field() const noexcept
    {
        return view_group_field;
    }

    inline const views::view<pods::field>::ptr& get_view_field() const noexcept
    {
        return view_field;
    }

    inline void set_synchronizer_timeout(long timeout) const noexcept
    {
        if(synchronizer)
        {
            synchronizer->set_timeout(timeout);
        }
    }

    inline void set_synchronizer_connect_timeout(long connect_timeout) const noexcept
    {
        if(synchronizer)
        {
            synchronizer->set_connect_timeout(connect_timeout);
        }
    }
    
    inline void set_offline(bool offline) noexcept
    {
        this->offline = offline;
    }
    
private:
    void export_data(nlohmann::json& json, const daos::dao& dao, const services::aes& aes, const pods::group::ptr& group, bool enable_aes) const;

    void import_data(const pods::user::ptr& user, nlohmann::json& json_group, const daos::dao& dao, const services::aes& aes, std::optional<pods::group*> father, bool enable_aes) const;

    void import_data_legacy_group(const pods::user::ptr& user, const daos::dao &dao, const tinyxml2::XMLElement *element, const services::aes& aes, const pods::group::ptr &father, bool enable_aes) const;

    void import_data_legacy_group_field(const pods::user::ptr& user, const daos::dao &dao, const tinyxml2::XMLElement *element, const services::aes& aes, const pods::group::ptr &father, bool enable_aes) const;

    void import_data_legacy_field(const pods::user::ptr& user, const daos::dao &dao, const tinyxml2::XMLElement *element, const services::aes& aes, const pods::group::ptr &father, bool enable_aes) const;

    void lock();

    void unlock();

    bool check_lock();
};

}
