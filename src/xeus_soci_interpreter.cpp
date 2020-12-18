/***************************************************************************
* Copyright (c) 2020, QuantStack and xeus-soci contributors                *
*                                                                          *
*                                                                          *
* Distributed under the terms of the BSD 3-Clause License.                 *
*                                                                          *
* The full license is in the file LICENSE, distributed with this software. *
****************************************************************************/

#include <cctype>
#include <cstdio>
#include <ctime>
#include <fstream>
#include <memory>
#include <set>
#include <sstream>
#include <vector>
#include <tuple>


#include "tabulate/table.hpp"
#include "xeus/xinterpreter.hpp"

#include "xeus-soci/xeus_soci_interpreter.hpp"

#include "xeus-soci/soci_handler.hpp"

// #include "soci/soci.h"
// #include "soci/postgresql/soci-postgresql.h"

namespace xeus_soci
{

    void interpreter::configure_impl()
    {
    }

    void interpreter::process_SQL_input(int execution_counter,
                                        const std::string& code,
                                        xv::df_type& xv_soci_df)
    {
        nl::json pub_data;
        std::vector<std::string> plain_table_header;
        std::vector<std::string> plain_table_row;

        //IF IS A SELECT IF NOT DONT ENTER HERE, THIS IS NOT LIKE SQLITE3 & UR ALLOWED TO DO OPERATIONS

        tabulate::Table plain_table;
        std::stringstream html_table("");

        soci::rowset<soci::row> rows = ((*this->sql).prepare << code);

        /* Builds table header */
        const soci::row& first_row = *rows.begin();

        html_table << "<table>\n<tr>\n";
        for(std::size_t i = 0; i < first_row.size(); ++i)
        {
            std::string name = first_row.get_properties(i).get_name();
            html_table << "<th>" << name << "</th>\n";
            xv_soci_df[name] = { "name" };
            plain_table_header.push_back(name);
        }
        html_table << "</tr>\n";

        /* Builds table body */
        for (const soci::row& r : rows)
        {
            /* Iterates through cols' rows and builds different kinds of
               outputs
            */
            html_table << "<tr>\n";
            for(std::size_t i = 0; i != r.size(); ++i)
            {
                std::string cell;

                soci::column_properties props = r.get_properties(i);
                switch(props.get_data_type())
                {
                    case soci::dt_string:
                        cell = r.get<std::string>(i);
                        break;
                    case soci::dt_double:
                        cell = std::to_string(r.get<double>(i));
                        break;
                    case soci::dt_integer:
                        cell = std::to_string(r.get<int>(i));
                        break;
                    case soci::dt_long_long:
                        cell = std::to_string(r.get<long long>(i));
                        break;
                    case soci::dt_unsigned_long_long:
                        cell = std::to_string(r.get<unsigned long long>(i));
                        break;
                    // case soci::dt_date:
                    //     std::tm when = r.get<std::tm>(i);
                    //     cell = std::to_string(asctime(&when));
                    //     break;
                }
                html_table << "<td>" << cell << "</td>\n";
                plain_table_row.push_back(cell);
                xv_soci_df[r.get_properties(i).get_name()].push_back(cell);
            }
                html_table << "</tr>\n";
        }
        html_table << "</table>";

        pub_data["text/plain"] = plain_table.str();
        pub_data["text/html"] = html_table.str();

        publish_execution_result(execution_counter,
                                std::move(pub_data),
                                nl::json::object());
        }
    //     else
    //     {
    //         query.exec();
    //     }

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

        xv::df_type xv_soci_df;
        try
        {
            /* Runs magic */
            if(xv_bindings::is_magic(tokenized_input))
            {
                /* Removes "%" symbol */
                tokenized_input[0].erase(0, 1);

                /* Runs xvega magic and SQL code */
                if(xv_bindings::is_xvega(tokenized_input))
                {
                    /* Removes XVEGA_PLOT command */
                    tokenized_input.erase(tokenized_input.begin());

                    nl::json chart;
                    std::vector<std::string> xvega_input, sql_input;

                    std::tie(xvega_input, sql_input) = split_xv_soci_input(tokenized_input);
                    std::stringstream stringfied_soci_input;
                    for (size_t i = 0; i < sql_input.size(); i++) {
                        stringfied_soci_input << " " << sql_input[i];
                    }

                    process_SQL_input(execution_counter, stringfied_soci_input.str(), xv_soci_df);

                    chart = xv_bindings::process_xvega_input(xvega_input,
                                                             xv_soci_df);

                    publish_execution_result(execution_counter,
                                             std::move(chart),
                                             nl::json::object());

                    nl::json jresult;
                    jresult["status"] = "ok";
                    jresult["payload"] = nl::json::array();
                    jresult["user_expressions"] = nl::json::object();
                    return jresult;
                }

                /* Parses SQL magic */
                this->sql = parse_SQL_magic(tokenized_input);

                /* Runs SQL magic */
                // isso aqui tava rodando o SQL ate quando era magic
                //process_SQL_input(execution_counter, code, xv_soci_df);
            }
            /* Runs SQL code */
            else
            {
                if (this->sql)
                {
                    process_SQL_input(execution_counter, code, xv_soci_df);
                }
                else
                {
                    throw("Database was not loaded.");
                }
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

        /* The jupyter-console banner for xeus-soci is the following:
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
            "  SOCI version: ";
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
