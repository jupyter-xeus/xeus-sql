/***************************************************************************
* Copyright (c) 2020, QuantStack and xeus-sql contributors                *
*                                                                          *
*                                                                          *
* Distributed under the terms of the BSD 3-Clause License.                 *
*                                                                          *
* The full license is in the file LICENSE, distributed with this software. *
****************************************************************************/

#ifndef TEST_DB_HPP
#define TEST_DB_HPP

#include "gtest/gtest.h"

#include "xeus-sql/xeus_sql_interpreter.hpp"
#include "xvega-bindings/utils.hpp"

namespace xeus_sql
{

TEST(xeus_sql_interpreter, tokenizer)
{
    std::string code = "\%LOAD database.db";
    std::vector<std::string> tokenized_code;
    tokenized_code = xv_bindings::tokenizer(code);
    EXPECT_EQ(tokenized_code[1], "database.db");
}

}

#endif
