/***************************************************************************
* Copyright (c) 2020, QuantStack and xeus-sql contributors                *
*                                                                          *
*                                                                          *
* Distributed under the terms of the BSD 3-Clause License.                 *
*                                                                          *
* The full license is in the file LICENSE, distributed with this software. *
****************************************************************************/

#ifndef XEUS_SQL_HANDLER_HPP
#define XEUS_SQL_HANDLER_HPP

#include <vector>
#include <string>

#include "nlohmann/json.hpp"
#include "soci/soci.h"
#include "xeus/xinterpreter.hpp"
#include "xvega-bindings/xvega_bindings.hpp"

#include "xeus_sql_interpreter.hpp"

namespace nl = nlohmann;

namespace xeus_sql
{
    static std::unique_ptr<soci::session> load_db(
            const std::vector<std::string> tokenized_input)
    {
        std::string aux;
        for (std::size_t i = 2; i < tokenized_input.size(); i++)
        {
            aux += tokenized_input[i] + ' ';
        }
        return std::make_unique<soci::session>(
                xv_bindings::to_lower(tokenized_input[1]), aux);
    }

    static std::unique_ptr<soci::session> parse_SQL_magic(
            const std::vector<std::string>& tokenized_input)
    {
        if (xv_bindings::case_insentive_equals(tokenized_input[0], "LOAD"))
        {
            return load_db(tokenized_input);
        }
        throw std::runtime_error("Command is not valid.");
    }

    static std::pair<std::vector<std::string>, std::vector<std::string>> 
        split_xv_sql_input(std::vector<std::string> complete_input)
    {
        //TODO: test edge cases
        auto found = std::find(complete_input.begin(),
                               complete_input.end(),
                               "<>");

        std::vector<std::string> xvega_input(complete_input.begin(), found);
        std::vector<std::string> sql_input(found + 1, complete_input.end());

        return std::make_pair(xvega_input, sql_input);
    }
}

#endif
