/***************************************************************************
* Copyright (c) 2020, QuantStack and Xeus-SQLite contributors              *
*                                                                          *
*                                                                          *
* Distributed under the terms of the BSD 3-Clause License.                 *
*                                                                          *
* The full license is in the file LICENSE, distributed with this software. *
****************************************************************************/

#ifndef TEST_DB_HPP
#define TEST_DB_HPP

#include "gtest/gtest.h"

#include "xeus-/xeus_soci_interpreter.hpp"
#include "xvega-bindings/utils.hpp"

namespace xeus_soci
{

TEST(xeus_soci_interpreter, tokenizer_check)
{
    std::string code = "%LOAD  database.db   rw";
    std::vector<std::string> tokenized_code;
    tokenized_code = xv_bindings::tokenizer(code);
    EXPECT_EQ(tokenized_code[1], "database.db");
}

// TEST(xeus_sqlite_interpreter, is_magic_check)
// {
//     std::string code = "%LOAD database.db rw";
//     EXPECT_TRUE(is_magic(code));

//     std::string code = "SELECT *";
//     EXPECT_FALSE(is_magic(code));
// }

// TEST(xeus_sqlite_interpreter, create_destroy_db)
// {
//     std::vector<std::string> tokenized_code; 
//     tokenized_code[2] = ".";
//     tokenized_code[3] = "test.db";
//     tokenized_code = create_db(tokenized_code);
// }
}

#endif
