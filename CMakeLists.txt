############################################################################
# Copyright (c) 2020, QuantStack and xeus-sql contributors                #
#                                                                          #
#                                                                          #
# Distributed under the terms of the BSD 3-Clause License.                 #
#                                                                          #
# The full license is in the file LICENSE, distributed with this software. #
############################################################################

cmake_minimum_required(VERSION 3.4.3)
project(xeus-sql)

set(XEUS_SQL_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include)
set(XEUS_TEST_DIR ${CMAKE_CURRENT_SOURCE_DIR}/test)

set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

# Versioning
# ==========

file(STRINGS "${XEUS_SQL_INCLUDE_DIR}/xeus-sql/xeus_sql_config.hpp" xsql_version_defines
     REGEX "#define XSQL_VERSION_(MAJOR|MINOR|PATCH)")
foreach(ver ${xsql_version_defines})
    if(ver MATCHES "#define XSQL_VERSION_(MAJOR|MINOR|PATCH) +([^ ]+) *$")
        set(XSQL_VERSION_${CMAKE_MATCH_1} "${CMAKE_MATCH_2}" CACHE INTERNAL "")
    endif()
endforeach()
set(${PROJECT_NAME}_VERSION
${XSQL_VERSION_MAJOR}.${XSQL_VERSION_MINOR}.${XSQL_VERSION_PATCH})
message(STATUS "Building xeus-sql v${${PROJECT_NAME}_VERSION}")

# Configuration
# =============

include(GNUInstallDirs)

# We generate the kernel.json file, given the installation prefix and the executable name
configure_file (
    "${CMAKE_CURRENT_SOURCE_DIR}/share/jupyter/kernels/xsql/kernel.json.in"
    "${CMAKE_CURRENT_SOURCE_DIR}/share/jupyter/kernels/xsql/kernel.json"
)

# Build options
# =============

OPTION(XSQL_DISABLE_ARCH_NATIVE "disable -march=native flag" OFF)
OPTION(XSQL_DISABLE_TUNE_GENERIC "disable -mtune=generic flag" OFF)

OPTION(XSQL_BUILD_STATIC "Build xeus-sql static library" ON)
OPTION(XSQL_BUILD_SHARED "Split xsql build into executable and library" ON)
OPTION(XSQL_BUILD_XSQL_EXECUTABLE "Build the xsql executable" ON)

OPTION(XSQL_USE_SHARED_XEUS "Link xsql with the xeus shared library (instead of the static library)" ON)
OPTION(XSQL_USE_SHARED_XEUS_SQL "Link xsql with the xeus-sql shared library (instead of the static library)" ON)

OPTION(XSQL_BUILD_TESTS "xeus-sql test suite" OFF)

OPTION(CMAKE_USE_WIN32_THREADS_INIT "using WIN32 threads" ON)

# Dependencies
# ============

# Be sure to use recent versions
set(xeus-zmq_REQUIRED_VERSION 1.0.1)

find_package(xeus-zmq ${xeus-zmq_REQUIRED_VERSION} REQUIRED)
find_package(xvega)

find_package(tabulate REQUIRED)
find_package(Threads REQUIRED)

find_package(Soci REQUIRED MODULE)

# https://stackoverflow.com/a/15212881/1203241
OPTION(XSQL_WITH_POSTGRE_SQL "Option to require PostgreSQL" OFF)
OPTION(XSQL_WITH_MYSQL "Option to require MySQL" OFF)
OPTION(XSQL_WITH_SQLITE3 "Option to require SQLite3" OFF)

if(XSQL_WITH_POSTGRE_SQL)
    find_package(PostgreSQL REQUIRED)
    add_definitions(-DUSE_POSTGRE_SQL=1)
else()
    find_package(PostgreSQL)
endif()

if(XSQL_WITH_MYSQL)
    find_package(mysql REQUIRED)
    add_definitions(-DUSE_MYSQL=1)
else()
    find_package(mysql)
endif()

if(XSQL_WITH_SQLITE3)
    find_package(SQLite3 REQUIRED)
    add_definitions(-DUSE_SQLITE3=1)
else()
    find_package(SQLite3)
endif()


# Target and link
# ===============

string(TOUPPER "${CMAKE_BUILD_TYPE}" U_CMAKE_BUILD_TYPE)

# xeus-sql source directory
set(XEUS_SQL_SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)

# xeus-sql source files
set(XEUS_SQL_SRC
    ${XEUS_SQL_SRC_DIR}/xeus_sql_interpreter.cpp
)

set(XEUS_SQL_HEADERS
    include/xeus-sql/soci_handler.hpp
    include/xeus-sql/xeus_sql_config.hpp
    include/xeus-sql/xeus_sql_interpreter.hpp
)

set(XSQL_SRC ${XEUS_SQL_SRC_DIR}/main.cpp)

# Targets and link - Macros
# =========================

include(CheckCXXCompilerFlag)

set(CMAKE_BUILD_TYPE Release CACHE STRING "Choose the type of build." FORCE)
string(TOUPPER "${CMAKE_BUILD_TYPE}" CMAKE_BUILD_TYPE)

set(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib:${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR}")

macro(xsql_set_common_options target_name)
    if (CMAKE_CXX_COMPILER_ID MATCHES "Clang" OR
        CMAKE_CXX_COMPILER_ID MATCHES "GNU" OR
        CMAKE_CXX_COMPILER_ID MATCHES "Intel")

        target_compile_options(${target_name} PUBLIC -Wunused-parameter -Wextra -Wreorder)
    
        # Mtune generic/native
        if (XSQL_DISABLE_ARCH_NATIVE AND NOT XSQL_DISABLE_TUNE_GENERIC)
            target_compile_options(${target_name} PUBLIC -mtune=generic)
        elseif (XSQL_DISABLE_TUNE_GENERIC)
        else ()
            target_compile_options(${target_name} PUBLIC -march=native)
        endif ()

        # C++14 flag, setting this flag is failing on Clang
        if (NOT CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
            CHECK_CXX_COMPILER_FLAG("-std=c++14" HAS_CPP14_FLAG)
            if (HAS_CPP14_FLAG)
            target_compile_features(${target_name} PRIVATE cxx_std_14)
            else ()
                message(FATAL_ERROR "Unsupported compiler -- xeus-sql requires C++14 support!")
            endif ()
        else ()
            target_compile_features(${target_name} PRIVATE cxx_std_14)
        endif ()

    endif ()

    if (APPLE)
        set_target_properties(${target_name} PROPERTIES
            MACOSX_RPATH ON
        )
    else ()
        set_target_properties(${target_name} PROPERTIES
            BUILD_WITH_INSTALL_RPATH 1
            SKIP_BUILD_RPATH FALSE
        )
    endif ()

    set_target_properties(${target_name} PROPERTIES
        INSTALL_RPATH_USE_LINK_PATH TRUE
    )
endmacro()


# Common macro for shared and static library xeus-sql
macro(xsql_create_target target_name linkage output_name)
    string(TOUPPER "${linkage}" linkage_upper)

    if (NOT ${linkage_upper} MATCHES "^(SHARED|STATIC)$")
        message(FATAL_ERROR "Invalid library linkage: ${linkage}")
    endif ()

    add_library(${target_name} ${linkage_upper} ${XEUS_SQL_SRC} ${XEUS_SQL_HEADERS})
    xsql_set_common_options(${target_name})

    set_target_properties(${target_name} PROPERTIES
                          PUBLIC_HEADER "${XEUS_SQL_HEADERS}"
                          # COMPILE_DEFINITIONS "XEUS_SQL_EXPORTS"
                          PREFIX ""
                          VERSION ${${PROJECT_NAME}_VERSION}
                          SOVERSION ${XSQL_VERSION_MAJOR}
                          OUTPUT_NAME "lib${output_name}")

    target_compile_definitions(${target_name} PUBLIC "XEUS_SQL_EXPORTS")

    target_include_directories(${target_name}
                               PUBLIC
                               $<BUILD_INTERFACE:${XEUS_SQL_INCLUDE_DIR}>
                               $<INSTALL_INTERFACE:include>)

    if (XSQL_USE_SHARED_XEUS)
        set(XSQL_XEUS_TARGET xeus-zmq)
    else ()
        set(XSQL_XEUS_TARGET xeus-zmq-static)
    endif ()

    target_link_libraries(${target_name} PUBLIC 
      ${XSQL_XEUS_TARGET}
      xvega
      ${SOCI_LIBRARY}
    )

    # find_package(Threads) # TODO: add Threads as a dependence of xeus-static?
    target_link_libraries(${target_name} PRIVATE ${CMAKE_THREAD_LIBS_INIT})
endmacro()

# xeus-sql
# ===========

set(XEUS_SQL_TARGETS "")

if (XSQL_BUILD_SHARED)
    # Build libraries
    xsql_create_target(xeus-sql SHARED xeus-sql)
    list(APPEND XEUS_SQL_TARGETS xeus-sql)
endif ()

if (XSQL_BUILD_STATIC)
    # On Windows, a static library should use a different output name
    # to avoid the conflict with the import library of a shared one.
    if (CMAKE_HOST_WIN32)
        xsql_create_target(xeus-sql-static STATIC xeus-sql-static)
    else ()
        xsql_create_target(xeus-sql-static STATIC xeus-sql)
    endif ()
    list(APPEND XEUS_SQL_TARGETS xeus-sql-static)
endif ()

# xsql
# =======

if (XSQL_BUILD_XSQL_EXECUTABLE)
    add_executable(xsql ${XSQL_SRC})
    set_target_properties(xsql PROPERTIES ENABLE_EXPORTS 1)

    xsql_set_common_options(xsql)

    if (XSQL_USE_SHARED_XEUS_SQL)
        target_link_libraries(xsql PRIVATE xeus-sql)

        if(CMAKE_DL_LIBS)
            target_link_libraries(xsql PRIVATE ${CMAKE_DL_LIBS} util)
        endif()
    else ()
        target_link_libraries(xsql PRIVATE xeus-sql-static)
    endif()
endif()


# Tests
# =====

if(XSQL_BUILD_TESTS)
    add_subdirectory(test)
endif()

# Installation
# ============

include(CMakePackageConfigHelpers)

set(XEUS_SQL_CMAKECONFIG_INSTALL_DIR "${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}" CACHE STRING "install path for xeus-sqlConfig.cmake")

# Install xeus-sql and xsql
if (XSQL_BUILD_SHARED OR XSQL_BUILD_STATIC)
    install(TARGETS ${XEUS_SQL_TARGETS}
            EXPORT ${PROJECT_NAME}-targets
            ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
            LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
            RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
            PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/xeus-sql)

    # Makes the project importable from the build directory
    export(EXPORT ${PROJECT_NAME}-targets
           FILE "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}Targets.cmake")
endif ()

# Install xsql
if (XSQL_BUILD_XSQL_EXECUTABLE)
    install(TARGETS xsql
            RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})

    # Configuration and data directories for jupyter and xeus_sql
    set(XJUPYTER_DATA_DIR "share/jupyter" CACHE STRING "Jupyter data directory")

    # Install xsql Jupyter kernelspecs
    set(XSQL_KERNELSPEC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/share/jupyter/kernels)
    install(DIRECTORY ${XSQL_KERNELSPEC_DIR}
            DESTINATION ${XJUPYTER_DATA_DIR}
            PATTERN "*.in" EXCLUDE)


    # Extra path for installing Jupyter kernelspec
    if (XEXTRA_JUPYTER_DATA_DIR)
        install(DIRECTORY ${XSQL_KERNELSPEC_DIR}
                DESTINATION ${XEXTRA_JUPYTER_DATA_DIR}
                PATTERN "*.in" EXCLUDE)
    endif(XEXTRA_JUPYTER_DATA_DIR)
endif()

# Configure 'xeus-sqlConfig.cmake' for a build tree
set(XEUS_SQL_CONFIG_CODE "####### Expanded from \@XEUS_SQL_CONFIG_CODE\@ #######\n")
set(XEUS_SQL_CONFIG_CODE "${XEUS_SQL_CONFIG_CODE}set(CMAKE_MODULE_PATH \"${CMAKE_CURRENT_SOURCE_DIR}/cmake;\${CMAKE_MODULE_PATH}\")\n")
set(XEUS_SQL_CONFIG_CODE "${XEUS_SQL_CONFIG_CODE}##################################################")
configure_package_config_file(${PROJECT_NAME}Config.cmake.in
                              "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}Config.cmake"
                              INSTALL_DESTINATION ${PROJECT_BINARY_DIR})

# Configure 'xeus-sqlConfig.cmake' for an install tree
set(XEUS_SQL_CONFIG_CODE "")
configure_package_config_file(${PROJECT_NAME}Config.cmake.in
                              "${CMAKE_CURRENT_BINARY_DIR}/CMakeFiles/${PROJECT_NAME}Config.cmake"
                              INSTALL_DESTINATION ${XEUS_SQL_CMAKECONFIG_INSTALL_DIR})

write_basic_package_version_file(${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}ConfigVersion.cmake
                                 VERSION ${${PROJECT_NAME}_VERSION}
                                 COMPATIBILITY AnyNewerVersion)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/CMakeFiles/${PROJECT_NAME}Config.cmake
              ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}ConfigVersion.cmake
              DESTINATION ${XEUS_SQL_CMAKECONFIG_INSTALL_DIR})

if (XSQL_BUILD_SHARED)
    install(EXPORT ${PROJECT_NAME}-targets
            FILE ${PROJECT_NAME}Targets.cmake
            DESTINATION ${XEUS_SQL_CMAKECONFIG_INSTALL_DIR})
endif ()
