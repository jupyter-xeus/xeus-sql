/***************************************************************************
* Copyright (c) 2020, QuantStack and Xeus-SQLite contributors              *
*                                                                          *
*                                                                          *
* Distributed under the terms of the BSD 3-Clause License.                 *
*                                                                          *
* The full license is in the file LICENSE, distributed with this software. *
****************************************************************************/

#ifndef XEUS_SQLITE_XVEGA_SQLITE_HPP
#define XEUS_SQLITE_XVEGA_SQLITE_HPP

#include <iterator>
#include <functional>
#include <map>
#include <string>
#include <utility>
#include <variant>
#include <vector>

#include "xvega/xvega.hpp"
#include "xeus_sqlite_config.hpp"

namespace nl = nlohmann;

namespace xeus_sqlite
{
    class XEUS_SQLITE_API xv_sqlite
    {
    public:

        virtual ~xv_sqlite();

        static std::pair<std::vector<std::string>, std::vector<std::string>>
               split_xv_sqlite_input(std::vector<std::string>);
    };
}

#endif
