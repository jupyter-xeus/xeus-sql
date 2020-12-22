/***************************************************************************
* Copyright (c) 2020, QuantStack and xeus-SQLite contributors              *
*                                                                          *
*                                                                          *
* Distributed under the terms of the BSD 3-Clause License.                 *
*                                                                          *
* The full license is in the file LICENSE, distributed with this software. *
****************************************************************************/

#ifndef XSOCI_CONFIG_HPP
#define XSOCI_CONFIG_HPP

// Project version
#define XSOCI_VERSION_MAJOR 0
#define XSOCI_VERSION_MINOR 0
#define XSOCI_VERSION_PATCH 2

// Composing the version string from major, minor and patch
#define XSOCI_CONCATENATE(A, B) XSOCI_CONCATENATE_IMPL(A, B)
#define XSOCI_CONCATENATE_IMPL(A, B) A##B
#define XSOCI_STRINGIFY(a) XSOCI_STRINGIFY_IMPL(a)
#define XSOCI_STRINGIFY_IMPL(a) #a

#define XSOCI_VERSION XSOCI_STRINGIFY(XSOCI_CONCATENATE(XSOCI_VERSION_MAJOR,     \
                      XSOCI_CONCATENATE(.,XSOCI_CONCATENATE(XSOCI_VERSION_MINOR, \
                      XSOCI_CONCATENATE(.,XSOCI_VERSION_PATCH)))))

#ifdef _WIN32
    #ifdef XEUS_SOCI_EXPORTS
        #define XEUS_SOCI_API __declspec(dllexport)
    #else
        #define XEUS_SOCI_API __declspec(dllimport)
    #endif
#else
    #define XEUS_SOCI_API
#endif

#endif
