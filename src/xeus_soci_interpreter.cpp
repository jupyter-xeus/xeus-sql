/***************************************************************************
* Copyright (c) 2020, QuantStack and xeus-SQLite contributors              *
*                                                                          *
*                                                                          *
* Distributed under the terms of the BSD 3-Clause License.                 *
*                                                                          *
* The full license is in the file LICENSE, distributed with this software. *
****************************************************************************/

#include <cctype>
#include <cstdio>
#include <fstream>
#include <memory>
#include <sstream>
#include <stack>
#include <vector>
#include <tuple>

#include "xvega-bindings/xvega-bindings.hpp"
#include "xeus/xinterpreter.hpp"
#include "tabulate/table.hpp"

#include "xeus-soci/xeus_soci_interpreter.hpp"

#include "soci/soci.h"
#include "soci/sqlite3/soci-sqlite3.h"

#include <SQLiteCpp/VariadicBind.h>
#include <SQLiteCpp/SQLiteCpp.h>

namespace xeus_sqlite
{

    void interpreter::configure_impl()
    {
    }


    void interpreter::process_SQL_input(int execution_counter,
                                        std::unique_ptr<SQLite::Database> &m_db,
                                        const std::string& code,
                                        xv::df_type& xv_sqlite_df)
    {
        std::cout << "Happily processing code\n";
        std::cout << "Finished processing code\n";
    //     SQLite::Statement query(*m_db, code);
    //     nl::json pub_data;

    //     /* Builds text/plain output */
    //     tabulate::Table plain_table;

    //     /* Builds text/html output */
    //     std::stringstream html_table("");

    //     if (query.getColumnCount() != 0)
    //     {
    //         std::vector<std::variant<
    //                         std::string,
    //                         const char*,
    //                         tabulate::Table>> col_names;

    //         /* Builds text/html output */
    //         html_table << "<table>\n<tr>\n";

    //         /* Iterates through cols name and build table's title row */
    //         for (int col = 0; col < query.getColumnCount(); col++) {
    //             std::string name = query.getColumnName(col);

    //             /* Builds text/plain output */
    //             col_names.push_back(name);

    //             /* Builds text/html output */
    //             html_table << "<th>" << name << "</th>\n";

    //             /* Build application/vnd.vegalite.v3+json output */
    //             xv_sqlite_df[name] = { "name" };
    //         }
    //         /* Builds text/plain output */
    //         plain_table.add_row(col_names);

    //         /* Builds text/html output */
    //         html_table << "</tr>\n";

    //         /* Iterates through cols' rows and builds different kinds of
    //            outputs
    //         */
    //         while (query.executeStep())
    //         {
    //              Builds text/html output 
    //             html_table << "<tr>\n";

    //             std::vector<std::variant<
    //                             std::string,
    //                             const char*,
    //                             tabulate::Table>> row;

    //             for (int col = 0; col < query.getColumnCount(); col++) {
    //                 std::string col_name;
    //                 col_name = query.getColumnName(col);
    //                 std::string cell = query.getColumn(col);

    //                 /* Builds text/plain output */
    //                 row.push_back(cell);

    //                 /* Builds text/html output */
    //                 html_table << "<td>" << cell << "</td>\n";

    //                 /* Build application/vnd.vegalite.v3+json output */
    //                 xv_sqlite_df[col_name].push_back(cell);
    //             }
    //             /* Builds text/html output */
    //             html_table << "</tr>\n";

    //             /* Builds text/plain output */
    //             plain_table.add_row(row);
    //         }
    //         /* Builds text/html output */
    //         html_table << "</table>";

    //         pub_data["text/plain"] = plain_table.str();
    //         pub_data["text/html"] = html_table.str();

    //         publish_execution_result(execution_counter,
    //                                     std::move(pub_data),
    //                                     nl::json::object());
    //     }
    //     else
    //     {
    //         query.exec();
    //     }
    }

    nl::json interpreter::execute_request_impl(int execution_counter,
                                               const std::string& code,
                                               bool /*silent*/,
                                               bool /*store_history*/,
                                               nl::json /*user_expressions*/,
                                               bool /*allow_stdin*/)
    {
        std::vector<std::string> traceback;
        std::string sanitized_code = xv_bindings::sanitize_string(code);
        std::vector<std::string> tokenized_input = xv_bindings::tokenizer(sanitized_code);

        /* This structure is only used when xvega code is run */
        //TODO: but it ends up being used in process_SQL_input, that's why
        // //it's initialized here. Not the best approach, this should be
        // //compartimentilized under xvega domain.
        // xv::df_type xv_sqlite_df;

        try
        {
            /* Runs magic */
            if(xv_bindings::is_magic(tokenized_input))
            {
                /* Removes "%" symbol */
                tokenized_input[0].erase(0, 1);

                /* Runs SQL magic */
                parse_SQL_magic(execution_counter, tokenized_input);

                /* Runs xvega magic and SQLite code */
                if(xv_bindings::is_xvega(tokenized_input))
                {
                    /* Removes XVEGA_PLOT command */
                    tokenized_input.erase(tokenized_input.begin());

                    nl::json chart;
                    std::vector<std::string> xvega_input, sqlite_input;

                    std::tie(xvega_input, sqlite_input) = 
                        xv_sqlite::split_xv_sqlite_input(tokenized_input);

                    process_SQL_input(execution_counter,
                                         soci_tabl,
                                         xv_sqlite_df);

                    chart = xv_bindings::process_xvega_input(xvega_input,
                                                           xv_sqlite_df);

                    publish_execution_result(execution_counter,
                                             std::move(chart),
                                             nl::json::object());
                }
            }
            /* Runs SQL code */
            else
            {
                process_SQL_input(execution_counter, m_db, code, xv_sqlite_df);
            }

            nl::json jresult;
            jresult["status"] = "ok";
            jresult["payload"] = nl::json::array();
            jresult["user_expressions"] = nl::json::object();
            return jresult;
        }

        catch (const std::runtime_error& err)
        {
            nl::json jresult;
            jresult["status"] = "error";
            jresult["ename"] = "Error";
            jresult["evalue"] = err.what();
            traceback.push_back((std::string)jresult["ename"] + ": " + (std::string)err.what());
            publish_execution_error(jresult["ename"], jresult["evalue"], traceback);
            traceback.clear();
            return jresult;
        }
    }

    nl::json interpreter::complete_request_impl(const std::string& /*code*/,
                                                int /*cursor_pos*/)
    {
        nl::json jresult;
        jresult["status"] = "ok";
        return jresult;
    };

    nl::json interpreter::inspect_request_impl(const std::string& /*code*/,
                                               int /*cursor_pos*/,
                                               int /*detail_level*/)
    {
        nl::json jresult;
        jresult["status"] = "ok";
        return jresult;
    };

    nl::json interpreter::is_complete_request_impl(const std::string& /*code*/)
    {
        nl::json jresult;
        jresult["status"] = "complete";
        return jresult;
    };

    nl::json interpreter::kernel_info_request_impl()
    {
        nl::json result;
        result["implementation"] = "xsoci";
        result["implementation_version"] = XSOCI_VERSION;

        /* The jupyter-console banner for xeus-sqlite is the following:
                                                                                              _|
            _|    _|    _|_|    _|    _|    _|_|_|                _|_|_|    _|_|      _|_|_|    
              _|_|    _|_|_|_|  _|    _|  _|_|      _|_|_|_|_|  _|_|      _|    _|  _|        _|
            _|    _|  _|        _|    _|      _|_|                  _|_|  _|    _|  _|        _|
            _|    _|    _|_|_|    _|_|_|  _|_|_|                _|_|_|      _|_|      _|_|_|  _|
           xeus-soci: a Jupyter kernel for SOCI
           SOCI version: x.x.x
        */

        std::string banner = ""
            "                                                                                  _|"
            "_|    _|    _|_|    _|    _|    _|_|_|                _|_|_|    _|_|      _|_|_|    "
            "  _|_|    _|_|_|_|  _|    _|  _|_|      _|_|_|_|_|  _|_|      _|    _|  _|        _|"
            "_|    _|  _|        _|    _|      _|_|                  _|_|  _|    _|  _|        _|"
            "_|    _|    _|_|_|    _|_|_|  _|_|_|                _|_|_|      _|_|      _|_|_|  _|"
            "  xeus-soci: a Jupyter kernel for SOCI\n"
            "  soci version: ";
        banner.append(XSOCI_VERSION);

        result["banner"] = banner;
        //TODO: This should change with the language
        result["language_info"]["name"] = "mysql";
        result["language_info"]["codemirror_mode"] = "sql";
        result["language_info"]["version"] = XSOCI_VERSION;
        result["language_info"]["mimetype"] = "";
        result["language_info"]["file_extension"] = "";
        return result;
    }

    void interpreter::shutdown_request_impl()
    {
    }

}
