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

#include <gtest/gtest.h>
#include <iostream>
#include <thread>
using namespace std;

#include "pocket-controllers/session.hpp"
#include "BS_thread_pool.hpp"

struct ThreadPoolTest : public ::testing::Test
{
    BS::thread_pool<6> pool;
    BS::synced_stream sync_out;

};


TEST_F(ThreadPoolTest, Test) try
{

    std::future<void> my_future = pool.submit_task(
            []
            {
                for(auto i : {1,2,3,4,5,6,7,8})
                {

                    this_thread::sleep_until(std::chrono::system_clock::now() + std::chrono::seconds(1));
                }
                std::cout << "my_future" << std::endl;
            });

    std::future<void> my_future1 = pool.submit_task(
            [this]
            {
                for(auto i : {1,2})
                {
                    this_thread::sleep_until(std::chrono::system_clock::now() + std::chrono::seconds(1));
                }
                std::cout << "my_future2" << std::endl;
                sync_out.println("ciao");
            });

    try
    {
        my_future.get();
        my_future1.get();
    }
    catch (const std::exception& e)
    {
        sync_out.println("Caught exception: ", e.what());
    }
    std::cout << "end" << std::endl;

}
catch (const std::exception& e)
{
    std::cerr << e.what() << std::endl;
    ASSERT_TRUE(false);
}

