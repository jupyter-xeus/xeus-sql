/***************************************************************************
* Copyright (c) 2020, QuantStack and xeus-sql contributors                *
*                                                                          *
*                                                                          *
* Distributed under the terms of the BSD 3-Clause License.                 *
*                                                                          *
* The full license is in the file LICENSE, distributed with this software. *
****************************************************************************/

#include <algorithm>
#include <cctype>
#include <chrono>
#include <cstdio>
#include <ctime>
#include <fstream>
#include <locale>
#include <memory>
#include <set>
#include <sstream>
#include <tuple>
#include <vector>

#include "tabulate/table.hpp"
#include "xeus/xinterpreter.hpp"

#include "xeus-sql/xeus_sql_interpreter.hpp"
#include "xeus-sql/soci_handler.hpp"

#ifdef USE_POSTGRE_SQL
#include "soci/postgresql/soci-postgresql.h"
#endif
#ifdef USE_MYSQL
#include "soci/mysql/soci-mysql.h"
#endif
#ifdef USE_SQLITE3
#include "soci/sqlite3/soci-sqlite3.h"
#endif

namespace xeus_sql
{

    void interpreter::configure_impl()
    {
    }

    // trim string https://stackoverflow.com/a/217605/1203241

    // trim from start (in place)
    static inline void ltrim(std::string &s) {
        s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](unsigned char ch) {
            return !std::isspace(ch);
        }));
    }

    // trim from end (in place)
    static inline void rtrim(std::string &s) {
        s.erase(std::find_if(s.rbegin(), s.rend(), [](unsigned char ch) {
            return !std::isspace(ch);
        }).base(), s.end());
    }

    // trim from both ends (in place)
    static inline void trim(std::string &s) {
        ltrim(s);
        rtrim(s);
    }

    using clock = std::chrono::system_clock;
    using sec = std::chrono::duration<double>;
    nl::json interpreter::process_SQL_input(const std::string& code,
                                            xv::df_type& xv_sql_df)
    {
        const auto before = clock::now();
        soci::rowset<soci::row> rows = ((*this->sql).prepare << code);

        nl::json pub_data;

        tabulate::Table plain_table;
        std::stringstream html_table("");
        /* Builds table body */
        int row_count = 0;
        for (const soci::row& r : rows)
        {
            if (row_count == 0) {
                std::vector<variant<std::string, const char*, tabulate::Table>> col_names;
                html_table << "<table>\n<tr>\n";
                for (std::size_t i = 0; i != r.size(); ++i) {
                    std::string name = r.get_properties(i).get_name();
                    html_table << "<th>" << name << "</th>\n";
                    col_names.push_back(name);
                }
                html_table << "</tr>\n";
                plain_table.add_row(col_names);
            }
            row_count++;
            /* Iterates through cols' rows and builds different kinds of
               outputs
            */
            std::vector<variant<std::string, const char*, tabulate::Table>> row;
            html_table << "<tr>\n";
            for(std::size_t i = 0; i != r.size(); ++i)
            {
                std::string cell;

                soci::column_properties props = r.get_properties(i);
                try {
                    switch(props.get_data_type())
                    {
                        case soci::dt_string:
                            cell = r.get<std::string>(i, "NULL");
                            break;
                        case soci::dt_double:
                            cell = std::to_string(r.get<double>(i));
                            cell.erase(cell.find_last_not_of('0') + 1, std::string::npos);
                            if (cell.back() == '.') {
                                cell.pop_back();
                            }
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
                        case soci::dt_xml:
                        case soci::dt_blob:
                            break;
                        case soci::dt_date:
                            std::tm when = r.get<std::tm>(i);
                            cell = std::asctime(&when);
                            break;
                    }
                } catch (...) {
                    cell = "NULL";
                }
                html_table << "<td>" << cell << "</td>\n";
                row.push_back(cell);
                xv_sql_df[r.get_properties(i).get_name()].push_back(cell);
            }
            plain_table.add_row(row);
            html_table << "</tr>\n";
        }
        html_table << "</table>";
        const sec duration = clock::now() - before;
        std::stringstream footer;
        footer << "\n";
        if (row_count == 0) {
            footer << std::fixed << std::setprecision(2) << "Empty set ("
                       << duration.count() << " sec)";
        } else if (row_count == 1) {
            footer << std::fixed << std::setprecision(2) << "1 row in set ("
                       << duration.count() << " sec)";
        } else {
            footer << std::fixed << std::setprecision(2) << row_count
                       << " rows in set (" << duration.count() << " sec)";
        }

        pub_data["text/plain"] = plain_table.str() + footer.str();
        pub_data["text/html"] = html_table.str() + footer.str();

        return pub_data;
    }

    nl::json interpreter::execute_request_impl(int execution_counter,
                                               const std::string& code,
                                               bool /*silent*/,
                                               bool /*store_history*/,
                                               nl::json /*user_expressions*/,
                                               bool /*allow_stdin*/)
    {
        auto ok = []() {
            nl::json jresult;
            jresult["status"] = "ok";
            jresult["payload"] = nl::json::array();
            jresult["user_expressions"] = nl::json::object();
            return jresult;
        };

        std::vector<std::string> traceback;
        auto handle_exception = [&](std::string what) {
            nl::json jresult;
            jresult["status"] = "error";
            jresult["ename"] = "Error";
            jresult["evalue"] = what;
            traceback.push_back((std::string)jresult["ename"] + ": " + what);
            publish_execution_error(jresult["ename"], jresult["evalue"], traceback);
            traceback.clear();
            return jresult;
        };

        // we only need to tokenize the first line
        std::istringstream iss(code);
        std::string first_line;
        while (std::getline(iss, first_line)) {
            trim(first_line);
            if (first_line.size() > 0) break;
        }
        std::vector<std::string> tokenized_input = xv_bindings::tokenizer(first_line);
        xv::df_type xv_sql_df;
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
                    tokenized_input = xv_bindings::tokenizer(code);
                    /* Removes XVEGA_PLOT command */
                    tokenized_input.erase(tokenized_input.begin());

                    nl::json chart;
                    std::vector<std::string> xvega_input, sql_input;

                    std::tie(xvega_input, sql_input) = split_xv_sql_input(tokenized_input);
                    std::stringstream stringfied_sql_input;
                    for (size_t i = 0; i < sql_input.size(); i++) {
                        stringfied_sql_input << " " << sql_input[i];
                    }

                    process_SQL_input(stringfied_sql_input.str(), xv_sql_df);

                    chart = xv_bindings::process_xvega_input(xvega_input,
                                                             xv_sql_df);

                    publish_execution_result(execution_counter,
                                             std::move(chart),
                                             nl::json::object());

                    return ok();
                } else if (xv_bindings::case_insentive_equals("VEGA_LITE", tokenized_input[0])) {
                    if (tokenized_input.size() < 2) {
                        throw std::runtime_error("invalid input: " + code);
                    }
                    if (xv_bindings::case_insentive_equals("SET", tokenized_input[1])) {
                        // define vega-lite specs
                        if (tokenized_input.size() < 3) {
                            throw std::runtime_error("invalid input: " + code);
                        }
                        std::string spec_name = tokenized_input[2];
                        std::string json_str = code;
                        json_str.erase(0, code.find(first_line) + first_line.length());
                        trim(json_str);
                        if (json_str.length() == 0) {
                            throw std::runtime_error("spec is empty: " + code);
                        }
                        nl::json spec_value = nl::json::parse(json_str);
                        specs[spec_name] = spec_value;
                        auto bundle = nl::json::object();
                        bundle["text/plain"] = "SET " + spec_name + " success.";
                        publish_execution_result(execution_counter, std::move(bundle), nl::json::object());
                        return ok();
                    }
                    nl::json j;
                    auto v = specs.find(tokenized_input[1]);
                    if (v != specs.end()) {
                        j = v->second;
                    } else {
                        std::ifstream i(tokenized_input[1]);
                        if (!i.good()) {
                            throw std::runtime_error("invalid file name: " + tokenized_input[1]);
                        }
                        i >> j;
                    }
                    std::string sql = code;
                    sql.erase(0, code.find(first_line) + first_line.length());
                    trim(sql);
                    if (sql.length() > 0) {
                        process_SQL_input(sql, xv_sql_df);
                        if (xv_sql_df.size() == 0) {
                            throw std::runtime_error("Empty result from sql, can't render");
                        }
                        xv::data_frame data_frame;
                        data_frame.values = xv_sql_df;
                        j["data"] = data_frame;
                    }
                    auto bundle = nl::json::object();
                    bundle["application/vnd.vegalite.v3+json"] = j;
                    publish_execution_result(execution_counter, std::move(bundle), nl::json::object());
                    return ok();
                }

                /* Parses LOAD magic */
                this->sql = parse_SQL_magic(tokenized_input);
            }
            /* Runs SQL code */
            else
            {
                if (this->sql)
                {
                    /* Shows rich output for tables */
                    if (xv_bindings::case_insentive_equals("SELECT", tokenized_input[0]) ||
                        xv_bindings::case_insentive_equals("DESC", tokenized_input[0]) ||
                        xv_bindings::case_insentive_equals("DESCRIBE", tokenized_input[0]) ||
                        xv_bindings::case_insentive_equals("SHOW", tokenized_input[0]))
                    {
                        nl::json data = process_SQL_input(code, xv_sql_df);

                        publish_execution_result(execution_counter,
                                                 std::move(data),
                                                 nl::json::object());

                    }
                    /* Execute all SQL commands that don't output tables */
                    else
                    {
                        *this->sql << code;
                    }
                }
                else
                {
                    throw("Database was not loaded.");
                }
            }
        } catch (const std::runtime_error &err) {
            return handle_exception((std::string)err.what());
#ifdef USE_POSTGRE_SQL
        } catch (const soci::postgresql_soci_error &err) {
            return handle_exception((std::string)err.what());
#endif
#ifdef USE_MYSQL
        } catch (const soci::mysql_soci_error &err) {
            return handle_exception((std::string)err.what());
#endif
#ifdef USE_SQLITE3
        } catch (const soci::sqlite3_soci_error &err) {
            return handle_exception((std::string)err.what());
#endif
        } catch (...) {
            // https:  // stackoverflow.com/a/54242936/1203241
            try {
                std::exception_ptr curr_excp;
                if ((curr_excp = std::current_exception())) {
                std::rethrow_exception(curr_excp);
                }
            } catch (const std::exception &err) {
                return handle_exception((std::string)err.what());
            }
        }
        return ok();
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
        result["implementation"] = "xsql";
        result["implementation_version"] = XSQL_VERSION;

        /* The jupyter-console banner for xeus-sql is the following:
                                            _ 
                                           | |
            __  _____ _   _ ___   ___  __ _| |
            \ \/ / _ \ | | / __| / __|/ _` | |
             >  <  __/ |_| \__ \ \__ \ (_| | |
            /_/\_\___|\__,_|___/ |___/\__, |_|
                                         | |  
                                         |_| 
           xeus-sql: a Jupyter kernel for SOCI
           SOCI version: x.x.x
        */

        std::string banner = R"V0G0N(
            "                                _    "
            "                               | |   "
            "__  _____ _   _ ___   ___  __ _| |   "
            "\ \/ / _ \ | | / __| / __|/ _` | |   "
            " >  <  __/ |_| \__ \ \__ \ (_| | |   "
            "/_/\_\___|\__,_|___/ |___/\__, |_|   "
            "                             | |     "
            "                             |_|     "
            "  xeus-sql: a Jupyter kernel for SOCI"
            "  XSQL version: ")V0G0N";
        banner.append(XSQL_VERSION);

        result["banner"] = banner;
        //TODO: This should change with the language
        result["language_info"]["name"] = "mysql";
        result["language_info"]["codemirror_mode"] = "sql";
        result["language_info"]["version"] = XSQL_VERSION;
        result["language_info"]["mimetype"] = "";
        result["language_info"]["file_extension"] = "";
        return result;
    }

    void interpreter::shutdown_request_impl()
    {
    }

}
