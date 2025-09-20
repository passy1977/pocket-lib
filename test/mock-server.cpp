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

#include "mock-server.hpp"
#include <iostream>
#include <sstream>
#include <cstring>
#include <cerrno>

namespace pocket::test
{

MockServer::MockServer(int port) : port(port)
{
}

MockServer::~MockServer()
{
    stop();
}

void MockServer::start()
{
    if (running.load()) {
        return; // Already running
    }

    // Create socket
    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket < 0) {
        throw std::runtime_error("Failed to create socket");
    }

    // Set socket options to reuse address
    int opt = 1;
    if (setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        close(server_socket);
        throw std::runtime_error("Failed to set socket options");
    }

    // Set socket to non-blocking for accept timeout
    struct timeval timeout;
    timeout.tv_sec = 1;  // 1 second timeout
    timeout.tv_usec = 0;
    if (setsockopt(server_socket, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout)) < 0) {
        close(server_socket);
        throw std::runtime_error("Failed to set socket timeout");
    }

    // Bind socket
    struct sockaddr_in address{};
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(port);

    if (bind(server_socket, (struct sockaddr*)&address, sizeof(address)) < 0) {
        close(server_socket);
        throw std::runtime_error("Failed to bind socket");
    }

    // Get the actual port if auto-assigned
    if (port == 0) {
        socklen_t len = sizeof(address);
        if (getsockname(server_socket, (struct sockaddr*)&address, &len) == 0) {
            port = ntohs(address.sin_port);
        }
    }

    // Listen for connections
    if (listen(server_socket, 10) < 0) {
        close(server_socket);
        throw std::runtime_error("Failed to listen on socket");
    }

    running.store(true);
    server_thread = std::thread(&MockServer::run_server, this);
}

void MockServer::stop()
{
    if (!running.load()) {
        return; // Already stopped
    }

    running.store(false);
    
    if (server_socket >= 0) {
        close(server_socket);
        server_socket = -1;
    }

    if (server_thread.joinable()) {
        server_thread.join();
    }
}

void MockServer::run_server()
{
    while (running.load()) {
        struct sockaddr_in client_address{};
        socklen_t client_len = sizeof(client_address);
        
        int client_socket = accept(server_socket, (struct sockaddr*)&client_address, &client_len);
        if (client_socket < 0) {
            if (errno == EWOULDBLOCK || errno == EAGAIN) {
                // Timeout occurred, continue loop to check if we should stop
                continue;
            } else if (running.load()) {
                std::cerr << "Accept failed: " << strerror(errno) << std::endl;
            }
            continue;
        }

        // Read request with timeout
        char buffer[4096];
        ssize_t bytes_read = recv(client_socket, buffer, sizeof(buffer) - 1, 0);
        if (bytes_read > 0) {
            buffer[bytes_read] = '\0';
            std::string request(buffer);
            
            std::string method, path, body;
            parse_request(request, method, path, body);
            
            std::cout << "Mock server received: " << method << " " << path << std::endl;
            
            // Find matching route
            std::string route_key = method + " " + path;
            Response response;
            
            auto it = routes.find(route_key);
            if (it != routes.end()) {
                response = it->second(method, path, body);
            } else {
                // Try to find prefix matches for API routes like /api/v5/*
                bool found_match = false;
                for (const auto& route : routes) {
                    std::string route_method_path = route.first;
                    
                    // Extract method and path from route key
                    size_t space_pos = route_method_path.find(' ');
                    if (space_pos != std::string::npos) {
                        std::string route_method = route_method_path.substr(0, space_pos);
                        std::string route_path = route_method_path.substr(space_pos + 1);
                        
                        // Check if method matches and path starts with route path
                        if (method == route_method && path.find(route_path) == 0) {
                            response = route.second(method, path, body);
                            found_match = true;
                            break;
                        }
                    }
                }
                
                if (!found_match) {
                    // Default 404 response
                    response.status_code = 404;
                    response.body = "Not Found: " + method + " " + path;
                }
            }
            
            std::string http_response = build_response(response);
            send(client_socket, http_response.c_str(), http_response.length(), 0);
        }
        
        close(client_socket);
    }
}

std::string MockServer::parse_request(const std::string& request, std::string& method, std::string& path, std::string& body)
{
    std::istringstream iss(request);
    std::string line;
    
    // Parse first line: METHOD PATH HTTP/1.1
    if (std::getline(iss, line)) {
        std::istringstream first_line(line);
        std::string http_version;
        first_line >> method >> path >> http_version;
    }
    
    // Skip headers for now (could be enhanced to parse them)
    std::string headers;
    while (std::getline(iss, line) && line != "\r" && !line.empty()) {
        headers += line + "\n";
    }
    
    // Read body (remaining content)
    std::string remaining;
    while (std::getline(iss, line)) {
        body += line + "\n";
    }
    
    return headers;
}

std::string MockServer::build_response(const Response& response)
{
    std::ostringstream oss;
    oss << "HTTP/1.1 " << response.status_code << " ";
    
    // Add standard status messages
    switch (response.status_code) {
        case 200: oss << "OK"; break;
        case 404: oss << "Not Found"; break;
        case 500: oss << "Internal Server Error"; break;
        default: oss << "Unknown"; break;
    }
    oss << "\r\n";
    
    // Add headers
    oss << "Content-Type: " << response.content_type << "\r\n";
    oss << "Content-Length: " << response.body.length() << "\r\n";
    oss << "Connection: close\r\n";
    
    for (const auto& header : response.headers) {
        oss << header.first << ": " << header.second << "\r\n";
    }
    
    oss << "\r\n"; // Empty line separating headers from body
    oss << response.body;
    
    return oss.str();
}

void MockServer::add_route(const std::string& method_path, RouteHandler handler)
{
    routes[method_path] = std::move(handler);
}

void MockServer::add_get(const std::string& path, RouteHandler handler)
{
    add_route("GET " + path, std::move(handler));
}

void MockServer::add_post(const std::string& path, RouteHandler handler)
{
    add_route("POST " + path, std::move(handler));
}

void MockServer::add_put(const std::string& path, RouteHandler handler)
{
    add_route("PUT " + path, std::move(handler));
}

void MockServer::add_delete(const std::string& path, RouteHandler handler)
{
    add_route("DELETE " + path, std::move(handler));
}

void MockServer::add_simple_get(const std::string& path, const std::string& response_body)
{
    add_get(path, [response_body](const std::string&, const std::string&, const std::string&) {
        Response resp;
        resp.body = response_body;
        return resp;
    });
}

void MockServer::add_json_get(const std::string& path, const std::string& json_response)
{
    add_get(path, [json_response](const std::string&, const std::string&, const std::string&) {
        Response resp;
        resp.content_type = "application/json";
        resp.body = json_response;
        return resp;
    });
}

}