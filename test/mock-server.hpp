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

#include <thread>
#include <atomic>
#include <string>
#include <map>
#include <functional>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

namespace pocket::test
{

/**
 * Simple HTTP mock server for testing network connections
 */
class MockServer
{
public:
    struct Response 
    {
        int status_code = 200;
        std::string content_type = "text/plain";
        std::string body = "OK";
        std::map<std::string, std::string> headers;
    };

    using RouteHandler = std::function<Response(const std::string& method, const std::string& path, const std::string& body)>;

private:
    std::thread server_thread;
    std::atomic<bool> running{false};
    int server_socket = -1;
    int port;
    std::map<std::string, RouteHandler> routes;

    void run_server();
    std::string parse_request(const std::string& request, std::string& method, std::string& path, std::string& body);
    std::string build_response(const Response& response);

public:
    explicit MockServer(int port = 0); // 0 = auto-assign port
    ~MockServer();

    // No copy/move
    MockServer(const MockServer&) = delete;
    MockServer& operator=(const MockServer&) = delete;
    MockServer(MockServer&&) = delete;
    MockServer& operator=(MockServer&&) = delete;

    void start();
    void stop();
    int get_port() const { return port; }
    std::string get_base_url() const { return "http://127.0.0.1:" + std::to_string(port); }

    // Route registration
    void add_route(const std::string& method_path, RouteHandler handler);
    void add_get(const std::string& path, RouteHandler handler);
    void add_post(const std::string& path, RouteHandler handler);
    void add_put(const std::string& path, RouteHandler handler);
    void add_delete(const std::string& path, RouteHandler handler);

    // Convenience methods for common responses
    void add_simple_get(const std::string& path, const std::string& response_body);
    void add_json_get(const std::string& path, const std::string& json_response);
};

}