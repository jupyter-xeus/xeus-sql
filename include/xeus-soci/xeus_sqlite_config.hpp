/***************************************************************************
* Copyright (c) 2020, QuantStack and Xeus-SQLite contributors              *
*                                                                          *
*                                                                          *
* Distributed under the terms of the BSD 3-Clause License.                 *
*                                                                          *
* The full license is in the file LICENSE, distributed with this software. *
****************************************************************************/

#ifndef XSQLITE_CONFIG_HPP
#define XSQLITE_CONFIG_HPP

// Project version
#define XSQLITE_VERSION_MAJOR 0
#define XSQLITE_VERSION_MINOR 1
#define XSQLITE_VERSION_PATCH 1

// Composing the version string from major, minor and patch
#define XSQLITE_CONCATENATE(A, B) XSQLITE_CONCATENATE_IMPL(A, B)
#define XSQLITE_CONCATENATE_IMPL(A, B) A##B
#define XSQLITE_STRINGIFY(a) XSQLITE_STRINGIFY_IMPL(a)
#define XSQLITE_STRINGIFY_IMPL(a) #a

#define XSQLITE_VERSION XSQLITE_STRINGIFY(XSQLITE_CONCATENATE(XSQLITE_VERSION_MAJOR,   \
                 XSQLITE_CONCATENATE(.,XSQLITE_CONCATENATE(XSQLITE_VERSION_MINOR,   \
                                  XSQLITE_CONCATENATE(.,XSQLITE_VERSION_PATCH)))))

#ifdef _WIN32
    #ifdef XEUS_SQLITE_EXPORTS
        #define XEUS_SQLITE_API __declspec(dllexport)
    #else
        #define XEUS_SQLITE_API __declspec(dllimport)
    #endif
#else
    #define XEUS_SQLITE_API
#endif

#endif
