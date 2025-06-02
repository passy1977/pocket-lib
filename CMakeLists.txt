cmake_minimum_required(VERSION  3.25)
project(pocket VERSION "5.0.0" LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_C_STANDARD 17)
set(CMAKE_C_STANDARD_REQUIRED ON)

add_definitions(-DPOCKET_DISABLE_LOCK)
add_definitions(-DPOCKET_DISABLE_DB_LOCK)
add_definitions(-DPOCKET_MAX_BUFFER_RESPONSE_SIZE=10485760)
#add_definitions(-DPOCKET_FORCE_TIMESTAMP_LAST_UPDATE=1739741159)
add_definitions(-DPOCKET_ENABLE_LOG)
add_definitions(-DPOCKET_ENABLE_AES)
option(POCKET_FORCE_TIMESTAMP_LAST_UPDATE "Force timestamp last update to the value who you want after login" OFF)

message(STATUS "PROJECT_SOURCE_DIR=${PROJECT_SOURCE_DIR}")
message(STATUS "CMAKE_BINARY_DIR=${CMAKE_BINARY_DIR}")

# Include the secrets file if it exists
if(EXISTS "${CMAKE_SOURCE_DIR}/secrets.cmake")
    include(secrets.cmake)
    message(STATUS "Found secrets.cmake")
else()
    message(STATUS "Not found secrets.cmake")
endif()


find_package(PkgConfig REQUIRED)
pkg_search_module(CURL REQUIRED libcurl)
if(CURL_FOUND)
    include_directories(${CURL_INCLUDE_DIR})
    message(STATUS "Using CURL ${CURL_VERSION}")
else(CURL_FOUND)
    message(FATAL_ERROR "Could not find the CURL library and development files.")
endif(CURL_FOUND)

pkg_search_module(OPENSSL REQUIRED openssl)
if(OPENSSL_FOUND)
    include_directories(${OPENSSL_INCLUDE_DIRS})
    message(STATUS "Using OpenSSL ${OPENSSL_VERSION}")
else()
    message(FATAL_ERROR "Could not find the OpenSSL library and development files.")
endif()

find_package (SQLite3)
pkg_search_module(SQLITE3 REQUIRED sqlite3)
if (SQLITE3_FOUND)
    message(STATUS "Using sqlite3 ${SQLITE3_VERSION}")
    include_directories(${SQLITE3_INCLUDE_DIRS})
else()
    message(FATAL_ERROR "Could not find the sqlite3 library and development files.")
endif (SQLITE3_FOUND)

include(FetchContent)
message(STATUS "start nlohmann_json download")
FetchContent_Declare(
        nlohmann_json
        GIT_REPOSITORY https://github.com/nlohmann/json.git
        GIT_TAG v3.11.3
)
FetchContent_MakeAvailable(nlohmann_json)

message(STATUS "start tinyxml2 download")
FetchContent_Declare(
        tinyxml2
        GIT_REPOSITORY https://github.com/leethomason/tinyxml2.git
        GIT_TAG 11.0.0
)
FetchContent_MakeAvailable(tinyxml2)


file(GLOB_RECURSE POCKET_INCLUDES CONFIGURE_DEPENDS "inc/pocket/*.hpp" "thread-pool/include/*.hpp")
file(GLOB_RECURSE POCKET_SOURCES CONFIGURE_DEPENDS "src/*.cpp")

file(GLOB_RECURSE POCKET_CONTROLLERS_INCLUDES CONFIGURE_DEPENDS "pocket-controllers/inc/pocket-controllers/*.hpp")
file(GLOB_RECURSE POCKET_CONTROLLERS_SOURCES CONFIGURE_DEPENDS "pocket-controllers/src/*.cpp")

file(GLOB_RECURSE POCKET_DAOS_INCLUDES CONFIGURE_DEPENDS "pocket-daos/inc/pocket-daos/*.hpp")
file(GLOB_RECURSE POCKET_DAOS_SOURCES CONFIGURE_DEPENDS "pocket-daos/src/*.cpp")

file(GLOB_RECURSE POCKET_IFACE_INCLUDES CONFIGURE_DEPENDS "pocket-iface/inc/pocket-iface/*.hpp")

file(GLOB_RECURSE POCKET_PODS_INCLUDES CONFIGURE_DEPENDS "pocket-pods/inc/pocket-pods/*.hpp")
file(GLOB_RECURSE POCKET_PODS_SOURCES CONFIGURE_DEPENDS "pocket-pods/src/*.cpp")

file(GLOB_RECURSE POCKET_SERVICES_INCLUDES CONFIGURE_DEPENDS "pocket-services/inc/pocket-services/*.hpp")
file(GLOB_RECURSE POCKET_SERVICES_SOURCES CONFIGURE_DEPENDS "pocket-services/src/*.cpp")

file(GLOB_RECURSE POCKET_VIEWS_INCLUDES CONFIGURE_DEPENDS "pocket-views/inc/pocket-views/*.hpp")
file(GLOB_RECURSE POCKET_VIEWS_SOURCES CONFIGURE_DEPENDS "pocket-views/src/*.cpp")

add_library(${PROJECT_NAME} STATIC
        ${POCKET_INCLUDES} ${POCKET_SOURCES}
        ${POCKET_CONTROLLERS_INCLUDES} ${POCKET_CONTROLLERS_SOURCES}
        ${POCKET_DAOS_INCLUDES} ${POCKET_DAOS_SOURCES}
        ${POCKET_IFACE_INCLUDES}
        ${POCKET_PODS_INCLUDES} ${POCKET_PODS_SOURCES}
        ${POCKET_SERVICES_INCLUDES} ${POCKET_SERVICES_SOURCES}
        ${POCKET_VIEWS_INCLUDES} ${POCKET_VIEWS_SOURCES}
)

target_link_libraries(${PROJECT_NAME}
        ${CURL_LIBRARIES}
        ${OPENSSL_LIBRARIES}
        ${SQLITE3_LIBRARIES}
        nlohmann_json::nlohmann_json
        tinyxml2
        dl
)

target_include_directories(${PROJECT_NAME} PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/inc
#        ${CMAKE_CURRENT_SOURCE_DIR}/inc/tinyxml2
        ${CMAKE_CURRENT_SOURCE_DIR}/pocket-controllers/inc
        ${CMAKE_CURRENT_SOURCE_DIR}/pocket-daos/inc
        ${CMAKE_CURRENT_SOURCE_DIR}/pocket-iface/inc
        ${CMAKE_CURRENT_SOURCE_DIR}/pocket-pods/inc
        ${CMAKE_CURRENT_SOURCE_DIR}/pocket-services/inc
        ${CMAKE_CURRENT_SOURCE_DIR}/pocket-views/inc
        ${CMAKE_CURRENT_SOURCE_DIR}/thread-pool/include
)

target_sources(${PROJECT_NAME} PUBLIC
        ${POCKET_INCLUDES} ${POCKET_SOURCES}
        ${POCKET_CONTROLLERS_INCLUDES} ${POCKET_CONTROLLERS_SOURCES}
        ${POCKET_DAOS_INCLUDES} ${POCKET_DAOS_SOURCES}
        ${POCKET_IFACE_INCLUDES}
        ${POCKET_PODS_INCLUDES} ${POCKET_PODS_SOURCES}
        ${POCKET_SERVICES_INCLUDES} ${POCKET_SERVICES_SOURCES}
        ${POCKET_VIEWS_INCLUDES} ${POCKET_VIEWS_SOURCES}
)

#Test
message(STATUS "start googletest download")
FetchContent_Declare(
        googletest
        URL https://github.com/google/googletest/archive/03597a01ee50ed33e9dfd640b249b4be3799d395.zip
)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

enable_testing()

file(GLOB_RECURSE POCKET_TEST CONFIGURE_DEPENDS "test/*.cpp")

include_directories("${CMAKE_BINARY_DIR}/_deps/googletest-src/googletest/include")

add_executable(${PROJECT_NAME}-test ${POCKET_TEST})

target_link_libraries(${PROJECT_NAME}-test
        ${PROJECT_NAME}
        ${CURL_LIBRARIES}
        ${OPENSSL_LIBRARIES}
        ${SQLITE3_LIBRARIES}
        nlohmann_json::nlohmann_json
        tinyxml2
        dl
        GTest::gtest
        GTest::gtest_main
)

include(GoogleTest)
gtest_discover_tests(${PROJECT_NAME}-test)
