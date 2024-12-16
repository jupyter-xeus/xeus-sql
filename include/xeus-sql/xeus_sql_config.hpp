/***************************************************************************
* Copyright (c) 2020, QuantStack and xeus-SQLite contributors              *
*                                                                          *
*                                                                          *
* Distributed under the terms of the BSD 3-Clause License.                 *
*                                                                          *
* The full license is in the file LICENSE, distributed with this software. *
****************************************************************************/

#ifndef XEUS_SQL_CONFIG_HPP
#define XEUS_SQL_CONFIG_HPP

// Project version
#define XSQL_VERSION_MAJOR 0
#define XSQL_VERSION_MINOR 3
#define XSQL_VERSION_PATCH 0

// Composing the version string from major, minor and patch
#define XSQL_CONCATENATE(A, B) XSQL_CONCATENATE_IMPL(A, B)
#define XSQL_CONCATENATE_IMPL(A, B) A##B
#define XSQL_STRINGIFY(a) XSQL_STRINGIFY_IMPL(a)
#define XSQL_STRINGIFY_IMPL(a) #a

#define XSQL_VERSION XSQL_STRINGIFY(XSQL_CONCATENATE(XSQL_VERSION_MAJOR,     \
                      XSQL_CONCATENATE(.,XSQL_CONCATENATE(XSQL_VERSION_MINOR, \
                      XSQL_CONCATENATE(.,XSQL_VERSION_PATCH)))))

#ifdef _WIN32
    #ifdef XEUS_SQL_EXPORTS
        #define XEUS_SQL_API __declspec(dllexport)
    #else
        #define XEUS_SQL_API __declspec(dllimport)
    #endif
#else
    #define XEUS_SQL_API
#endif

#endif
