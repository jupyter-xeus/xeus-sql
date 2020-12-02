############################################################################
# Copyright (c) 2020, QuantStack and xeus-soci contributors                #
#                                                                          #
#                                                                          #
# Distributed under the terms of the BSD 3-Clause License.                 #
#                                                                          #
# The full license is in the file LICENSE, distributed with this software. #
############################################################################

cmake_minimum_required(VERSION 3.4.3)
project(xeus-soci)

set(XEUS_SOCI_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include)
set(XEUS_TEST_DIR ${CMAKE_CURRENT_SOURCE_DIR}/test)

set(CMAKE_FIND_ROOT_PATH ${CMAKE_PREFIX_PATH})
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)
set(CMAKE_FIND_FRAMEWORK NEVER)
set(CMAKE_FIND_APPBUNDLE NEVER)
set(CMAKE_PROGRAM_PATH ${CMAKE_PREFIX_PATH})
set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

# Versioning
# ==========

file(STRINGS "${XEUS_SOCI_INCLUDE_DIR}/xeus-soci/xeus_sOCI_config.hpp" xsoci_version_defines
     REGEX "#define XSOCI_VERSION_(MAJOR|MINOR|PATCH)")
foreach(ver ${xsoci_version_defines})
    if(ver MATCHES "#define XSOCI_VERSION_(MAJOR|MINOR|PATCH) +([^ ]+) *$")
        set(XSOCI_VERSION_${CMAKE_MATCH_1} "${CMAKE_MATCH_2}" CACHE INTERNAL "")
    endif()
endforeach()
set(${PROJECT_NAME}_VERSION
${XSOCI_VERSION_MAJOR}.${XSOCI_VERSION_MINOR}.${XSOCI_VERSION_PATCH})
message(STATUS "Building xeus-soci v${${PROJECT_NAME}_VERSION}")

# Configuration
# =============

include(GNUInstallDirs)

# We generate the kernel.json file, given the installation prefix and the executable name
configure_file (
    "${CMAKE_CURRENT_SOURCE_DIR}/share/jupyter/kernels/xsoci/kernel.json.in"
    "${CMAKE_CURRENT_SOURCE_DIR}/share/jupyter/kernels/xsoci/kernel.json"
)

# Build options
# =============

OPTION(XSOCI_DISABLE_ARCH_NATIVE "disable -march=native flag" OFF)
OPTION(XSOCI_DISABLE_TUNE_GENERIC "disable -mtune=generic flag" OFF)

OPTION(XSOCI_BUILD_STATIC "Build xeus-soci static library" ON)
OPTION(XSOCI_BUILD_SHARED "Split xsoci build into executable and library" ON)
OPTION(XSOCI_BUILD_XSOCI_EXECUTABLE "Build the xsoci executable" ON)

OPTION(XSOCI_USE_SHARED_XEUS "Link xsoci with the xeus shared library (instead of the static library)" ON)
OPTION(XSOCI_USE_SHARED_XEUS_SOCI "Link xsoci with the xeus-soci shared library (instead of the static library)" ON)

OPTION(XSOCI_DOWNLOAD_GTEST "build gtest from downloaded sources" OFF)
OPTION(XSOCI_BUILD_TESTS "xeus-soci test suite" OFF)

# Dependencies
# ============

# Be sure to use recent versions
set(xeus_REQUIRED_VERSION 0.23.10)

find_package(xeus ${xeus_REQUIRED_VERSION} REQUIRED)
find_package(xvega REQUIRED)
find_package(xvega-bindings REQUIRED)

find_package(Soci REQUIRED MODULE)
find_package(SQLite3)
find_package(postgresql)
find_package(mysql)

find_package(tabulate REQUIRED)
find_package(Threads REQUIRED)

add_definitions(-DSQLITE_ENABLE_EXPLAIN_COMMENTS=1 -DSQLITE_DEBUG=1 -DSQLITE_MEMDEBUG=1)

# Target and link
# ===============

# xeus-soci source directory
set(XEUS_SOCI_SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)

# xeus-soci source files
set(XEUS_SOCI_SRC
)

set(XEUS_SOCI_HEADERS
)

set(XSOCI_SRC ${XEUS_SOCI_SRC_DIR}/main.cpp)

# Targets and link - Macros
# =========================

include(CheckCXXCompilerFlag)

set(CMAKE_BUILD_TYPE Release CACHE STRING "Choose the type of build." FORCE)
string(TOUPPER "${CMAKE_BUILD_TYPE}" CMAKE_BUILD_TYPE)

set(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib; ${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR}")

macro(xsoci_set_common_options target_name)
    if (CMAKE_CXX_COMPILER_ID MATCHES "Clang" OR
        CMAKE_CXX_COMPILER_ID MATCHES "GNU" OR
        CMAKE_CXX_COMPILER_ID MATCHES "Intel")

        target_compile_options(${target_name} PUBLIC -Wunused-parameter -Wextra -Wreorder)
    
        # Mtune generic/native
        if (XSOCI_DISABLE_ARCH_NATIVE AND NOT XSOCI_DISABLE_TUNE_GENERIC)
            target_compile_options(${target_name} PUBLIC -mtune=generic)
        elseif (XSOCI_DISABLE_TUNE_GENERIC)
        else ()
            target_compile_options(${target_name} PUBLIC -march=native)
        endif ()

        # C++17 flag
        CHECK_CXX_COMPILER_FLAG("-std=c++17" HAS_CPP17_FLAG)
        if (HAS_CPP17_FLAG)
            target_compile_features(${target_name} PRIVATE cxx_std_17)
        else ()
            message(FATAL_ERROR "Unsupported compiler -- xeus-soci requires C++17 support!")
        endif ()

        # Enable link time optimization and set the default symbol
        # visibility to hidden (very important to obtain small binaries)
        if (NOT ${U_CMAKE_BUILD_TYPE} MATCHES DEBUG)
            # Check for Link Time Optimization support
            # (GCC/Clang)
            CHECK_CXX_COMPILER_FLAG("-flto" HAS_LTO_FLAG)
            if (HAS_LTO_FLAG)
                target_compile_options(${target_name} PUBLIC -flto)
            endif ()

            # Avoids removing symbols from the static library
            CHECK_CXX_COMPILER_FLAG("-ffat-lto-objects" HAS_FATLTO_FLAG)
            if (${linkage_upper} MATCHES "STATIC" AND  HAS_FATLTO_FLAG)
                message(STATUS "ENABLE FAT LTO OBJECTS")
                target_compile_options(${target_name} PUBLIC -ffat-lto-objects)
            endif ()

            # Intel equivalent to LTO is called IPO
            if (CMAKE_CXX_COMPILER_ID MATCHES "Intel")
                CHECK_CXX_COMPILER_FLAG("-ipo" HAS_IPO_FLAG)
                if (HAS_IPO_FLAG)
                    target_compile_options(${target_name} PUBLIC -ipo)
                endif ()
            endif ()
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


# Common macro for shared and static library xeus-soci
macro(xsoci_create_target target_name linkage output_name)
    string(TOUPPER "${linkage}" linkage_upper)

    if (NOT ${linkage_upper} MATCHES "^(SHARED|STATIC)$")
        message(FATAL_ERROR "Invalid library linkage: ${linkage}")
    endif ()

    add_library(${target_name} ${linkage_upper} ${XEUS_SOCI_SRC} ${XEUS_SOCI_HEADERS})
    xsoci_set_common_options(${target_name})

    set_target_properties(${target_name} PROPERTIES
                          PUBLIC_HEADER "${XEUS_SOCI_HEADERS}"
                          # COMPILE_DEFINITIONS "XEUS_SOCI_EXPORTS"
                          PREFIX ""
                          VERSION ${${PROJECT_NAME}_VERSION}
                          SOVERSION ${XSOCI_VERSION_MAJOR}
                          OUTPUT_NAME "lib${output_name}")

    target_compile_definitions(${target_name} PUBLIC "XEUS_SOCI_EXPORTS")

    target_include_directories(${target_name}
                               PUBLIC
                               ${XSOCI_INCLUDE_DIRS}
                               $<BUILD_INTERFACE:${XEUS_SOCI_INCLUDE_DIR}>
                               $<INSTALL_INTERFACE:include>)

    if (XSOCI_USE_SHARED_XEUS)
        set(XSOCI_XEUS_TARGET xeus)
    else ()
        set(XSOCI_XEUS_TARGET xeus-static)
    endif ()

    target_link_libraries(${target_name} PUBLIC 
      ${XSOCI_XEUS_TARGET}
      xvega
      ${SOCI_LIBRARY}
      SQLite::SQLite3
    )

    # find_package(Threads) # TODO: add Threads as a dependence of xeus-static?
    target_link_libraries(${target_name} PRIVATE ${CMAKE_THREAD_LIBS_INIT})
endmacro()

# xeus-soci
# ===========

set(XEUS_SOCI_TARGETS "")

if (XSOCI_BUILD_SHARED)
    # Build libraries
    xsoci_create_target(xeus-soci SHARED xeus-soci)
    list(APPEND XEUS_SOCI_TARGETS xeus-soci)
endif ()

if (XSOCI_BUILD_STATIC)
    # On Windows, a static library should use a different output name
    # to avoid the conflict with the import library of a shared one.
    if (CMAKE_HOST_WIN32)
        xsoci_create_target(xeus-soci-static STATIC xeus-soci-static)
    else ()
        xsoci_create_target(xeus-soci-static STATIC xeus-soci)
    endif ()
    list(APPEND XEUS_SOCI_TARGETS xeus-soci-static)
endif ()

# xsoci
# =======

if (XSOCI_BUILD_XSOCI_EXECUTABLE)
    add_executable(xsoci ${XSOCI_SRC})
    set_target_properties(xsoci PROPERTIES ENABLE_EXPORTS 1)

    xsoci_set_common_options(xsoci)

    if (XSOCI_USE_SHARED_XEUS_SOCI)
        target_link_libraries(xsoci PRIVATE xeus-soci)

        if(CMAKE_DL_LIBS)
            target_link_libraries(xsoci PRIVATE ${CMAKE_DL_LIBS} util)
        endif()
    else ()
        target_link_libraries(xsoci PRIVATE xeus-soci-static)
    endif()
endif()


# Tests
# =====

if(XSOCI_DOWNLOAD_GTEST OR GTEST_SRC_DIR)
    set(XSOCI_BUILD_TESTS ON)
endif()


if(XSOCI_BUILD_TESTS)
    add_subdirectory(test)
endif()

# Installation
# ============

include(CMakePackageConfigHelpers)

set(XEUS_SOCI_CMAKECONFIG_INSTALL_DIR "${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}" CACHE STRING "install path for xeus-sociConfig.cmake")

# Install xeus-soci and xsoci
if (XSOCI_BUILD_SHARED OR XSOCI_BUILD_STATIC)
    install(TARGETS ${XEUS_SOCI_TARGETS}
            EXPORT ${PROJECT_NAME}-targets
            ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
            LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
            RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
            PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/xeus-soci)

    # Makes the project importable from the build directory
    export(EXPORT ${PROJECT_NAME}-targets
           FILE "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}Targets.cmake")
endif ()

# Install xsoci
if (XSOCI_BUILD_XSOCI_EXECUTABLE)
    install(TARGETS xsoci
            RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})

    # Configuration and data directories for jupyter and xeus_soci
    set(XJUPYTER_DATA_DIR "share/jupyter" CACHE STRING "Jupyter data directory")

    # Install xsoci Jupyter kernelspecs
    set(XSOCI_KERNELSPEC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/share/jupyter/kernels)
    install(DIRECTORY ${XSOCI_KERNELSPEC_DIR}
            DESTINATION ${XJUPYTER_DATA_DIR}
            PATTERN "*.in" EXCLUDE)


    # Extra path for installing Jupyter kernelspec
    if (XEXTRA_JUPYTER_DATA_DIR)
        install(DIRECTORY ${XSOCI_KERNELSPEC_DIR}
                DESTINATION ${XEXTRA_JUPYTER_DATA_DIR}
                PATTERN "*.in" EXCLUDE)
    endif(XEXTRA_JUPYTER_DATA_DIR)
endif()

# Configure 'xeus-sociConfig.cmake' for a build tree
set(XEUS_SOCI_CONFIG_CODE "####### Expanded from \@XEUS_SOCI_CONFIG_CODE\@ #######\n")
set(XEUS_SOCI_CONFIG_CODE "${XEUS_SOCI_CONFIG_CODE}set(CMAKE_MODULE_PATH \"${CMAKE_CURRENT_SOURCE_DIR}/cmake;\${CMAKE_MODULE_PATH}\")\n")
set(XEUS_SOCI_CONFIG_CODE "${XEUS_SOCI_CONFIG_CODE}##################################################")
configure_package_config_file(${PROJECT_NAME}Config.cmake.in
                              "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}Config.cmake"
                              INSTALL_DESTINATION ${PROJECT_BINARY_DIR})

# Configure 'xeus-sociConfig.cmake' for an install tree
set(XEUS_SOCI_CONFIG_CODE "")
configure_package_config_file(${PROJECT_NAME}Config.cmake.in
                              "${CMAKE_CURRENT_BINARY_DIR}/CMakeFiles/${PROJECT_NAME}Config.cmake"
                              INSTALL_DESTINATION ${XEUS_SOCI_CMAKECONFIG_INSTALL_DIR})

write_basic_package_version_file(${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}ConfigVersion.cmake
                                 VERSION ${${PROJECT_NAME}_VERSION}
                                 COMPATIBILITY AnyNewerVersion)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/CMakeFiles/${PROJECT_NAME}Config.cmake
              ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}ConfigVersion.cmake
              DESTINATION ${XEUS_SOCI_CMAKECONFIG_INSTALL_DIR})

if (XSOCI_BUILD_SHARED)
    install(EXPORT ${PROJECT_NAME}-targets
            FILE ${PROJECT_NAME}Targets.cmake
            DESTINATION ${XEUS_SOCI_CMAKECONFIG_INSTALL_DIR})
endif ()
