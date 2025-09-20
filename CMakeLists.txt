cmake_minimum_required(VERSION  3.25)
project(pocket VERSION "5.0.0" LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_C_STANDARD 17)
set(CMAKE_C_STANDARD_REQUIRED ON)

# Pocket Library Configuration Options
option(POCKET_DISABLE_LOCK "Disable locking mechanisms in Pocket library" ON)
option(POCKET_DISABLE_DB_LOCK "Disable database locking in Pocket library" ON)
option(POCKET_ENABLE_LOG "Enable logging in Pocket library" ON)
option(POCKET_ENABLE_AES "Enable AES encryption in Pocket library" ON)
option(POCKET_ENABLE_TEST "Enable test compilation in Pocket library" OFF)

# Advanced configuration options
set(POCKET_MAX_BUFFER_RESPONSE_SIZE 10485760 CACHE STRING "Maximum buffer response size for Pocket library")
set(POCKET_FORCE_TIMESTAMP_LAST_UPDATE 0 CACHE STRING "Force timestamp last update to specific value (0 = disabled)")

# Display configuration summary
message(STATUS "Pocket Library Configuration:")
message(STATUS "  POCKET_DISABLE_LOCK: ${POCKET_DISABLE_LOCK}")
message(STATUS "  POCKET_DISABLE_DB_LOCK: ${POCKET_DISABLE_DB_LOCK}")
message(STATUS "  POCKET_ENABLE_LOG: ${POCKET_ENABLE_LOG}")
message(STATUS "  POCKET_ENABLE_AES: ${POCKET_ENABLE_AES}")
message(STATUS "  POCKET_ENABLE_TEST: ${POCKET_ENABLE_TEST}")
message(STATUS "  POCKET_MAX_BUFFER_RESPONSE_SIZE: ${POCKET_MAX_BUFFER_RESPONSE_SIZE}")
message(STATUS "  POCKET_FORCE_TIMESTAMP_LAST_UPDATE: ${POCKET_FORCE_TIMESTAMP_LAST_UPDATE}")

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
    message(STATUS "Using CURL ${CURL_VERSION}")
else(CURL_FOUND)
    message(FATAL_ERROR "Could not find the CURL library and development files.")
endif(CURL_FOUND)

pkg_search_module(OPENSSL REQUIRED openssl)
if(OPENSSL_FOUND)
    message(STATUS "Using OpenSSL ${OPENSSL_VERSION}")
else()
    message(FATAL_ERROR "Could not find the OpenSSL library and development files.")
endif()

find_package (SQLite3)
pkg_search_module(SQLITE3 REQUIRED sqlite3)
if (SQLITE3_FOUND)
    message(STATUS "Using sqlite3 ${SQLITE3_VERSION}")
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
# Prevent nlohmann_json from being included in our install export
set(nlohmann_json_INSTALL OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(nlohmann_json)

message(STATUS "start tinyxml2 download")
FetchContent_Declare(
        tinyxml2
        GIT_REPOSITORY https://github.com/leethomason/tinyxml2.git
        GIT_TAG 11.0.0
)
# Prevent tinyxml2 from being included in our install export
set(tinyxml2_INSTALL OFF CACHE BOOL "" FORCE)
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

# Create alias target for better usability
add_library(Pocket::pocket ALIAS ${PROJECT_NAME})

# Configure compile definitions based on options
target_compile_definitions(${PROJECT_NAME} PUBLIC
    POCKET_MAX_BUFFER_RESPONSE_SIZE=${POCKET_MAX_BUFFER_RESPONSE_SIZE}
)

if(POCKET_DISABLE_LOCK)
    target_compile_definitions(${PROJECT_NAME} PUBLIC POCKET_DISABLE_LOCK)
endif()

if(POCKET_DISABLE_DB_LOCK)
    target_compile_definitions(${PROJECT_NAME} PUBLIC POCKET_DISABLE_DB_LOCK)
endif()

if(POCKET_ENABLE_LOG)
    target_compile_definitions(${PROJECT_NAME} PUBLIC POCKET_ENABLE_LOG)
endif()

if(POCKET_ENABLE_AES)
    target_compile_definitions(${PROJECT_NAME} PUBLIC POCKET_ENABLE_AES)
endif()

if(POCKET_FORCE_TIMESTAMP_LAST_UPDATE AND NOT POCKET_FORCE_TIMESTAMP_LAST_UPDATE EQUAL 0)
    target_compile_definitions(${PROJECT_NAME} PUBLIC POCKET_FORCE_TIMESTAMP_LAST_UPDATE=${POCKET_FORCE_TIMESTAMP_LAST_UPDATE})
endif()

if(POCKET_ENABLE_TEST)
    target_compile_definitions(${PROJECT_NAME} PUBLIC POCKET_ENABLE_TEST)
endif()

target_link_libraries(${PROJECT_NAME}
    PRIVATE
        ${CURL_LIBRARIES}
        ${OPENSSL_LIBRARIES}
        ${SQLITE3_LIBRARIES}
        nlohmann_json::nlohmann_json
        tinyxml2
        dl
)

target_include_directories(${PROJECT_NAME} 
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/inc>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/pocket-controllers/inc>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/pocket-daos/inc>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/pocket-iface/inc>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/pocket-pods/inc>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/pocket-services/inc>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/pocket-views/inc>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/thread-pool/include>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/pocket-controllers>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/pocket-daos>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/pocket-iface>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/pocket-pods>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/pocket-services>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/pocket-views>
    PRIVATE
        ${CURL_INCLUDE_DIR}
        ${OPENSSL_INCLUDE_DIRS}
        ${SQLITE3_INCLUDE_DIRS}
)

# Test configuration - only enabled if POCKET_ENABLE_TEST is ON
if(POCKET_ENABLE_TEST)
    message(STATUS "start googletest download")
    FetchContent_Declare(
            googletest
            GIT_REPOSITORY https://github.com/google/googletest.git
            GIT_TAG v1.15.2
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
    
    message(STATUS "Tests enabled - target '${PROJECT_NAME}-test' available")
else()
    message(STATUS "Tests disabled - set POCKET_ENABLE_TEST=ON to enable tests")
endif()

# Per ora, disabilitiamo l'installazione per evitare il problema con le dipendenze esterne
# L'export e l'installazione possono essere abilitati quando si gestiscono le dipendenze tramite il sistema di pacchetti

# Installation and packaging - DISABLED for now due to FetchContent dependencies
# include(GNUInstallDirs)
# include(CMakePackageConfigHelpers)

# Install the library
# install(TARGETS ${PROJECT_NAME}
#     EXPORT PocketTargets
#     ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
#     LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
#     RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
# )

# Export targets - DISABLED for now
# install(EXPORT PocketTargets
#     FILE PocketTargets.cmake
#     NAMESPACE Pocket::
#     DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/Pocket
# )

# Configure package config file - DISABLED for now
# configure_package_config_file(
#     "${CMAKE_CURRENT_SOURCE_DIR}/PocketConfig.cmake.in"
#     "${CMAKE_CURRENT_BINARY_DIR}/PocketConfig.cmake"
#     INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/Pocket
# )

# Generate version file - DISABLED for now
# write_basic_package_version_file(
#     "${CMAKE_CURRENT_BINARY_DIR}/PocketConfigVersion.cmake"
#     VERSION ${PROJECT_VERSION}
#     COMPATIBILITY SameMajorVersion
# )

# Install config files - DISABLED for now
# install(FILES
#     "${CMAKE_CURRENT_BINARY_DIR}/PocketConfig.cmake"
#     "${CMAKE_CURRENT_BINARY_DIR}/PocketConfigVersion.cmake"
#     DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/Pocket
# )
