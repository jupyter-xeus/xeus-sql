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
    // implemented at the end of the file
    // for better  readability
    const std::array<std::string, 826> & get_keywords();

    inline bool startswith(const std::string& str, const std::string& cmp)
    {
      return str.compare(0, cmp.length(), cmp) == 0;
    }
    inline static bool is_identifier(char c)
    {
        return std::isalpha(c) || std::isdigit(c) || c == '_';
    }

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
                tabulate::Table::Row_t col_names;
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
            tabulate::Table::Row_t row;
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
                            char buffer [20];
                            std::strftime(buffer, 20, "%Y-%m-%d %H:%M:%S", &when);
                            cell = buffer;
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
        std::stringstream rows_info;
        rows_info << "\n";
        if (row_count == 0) {
            rows_info << std::fixed << std::setprecision(2) << "Empty set ("
                       << duration.count() << " sec)";
        } else if (row_count == 1) {
            rows_info << std::fixed << std::setprecision(2) << "1 row in set ("
                       << duration.count() << " sec)";
        } else {
            rows_info << std::fixed << std::setprecision(2) << row_count
                       << " rows in set (" << duration.count() << " sec)";
        }

        pub_data["text/plain"] = rows_info.str() + plain_table.str();
        pub_data["text/html"] = rows_info.str() + html_table.str();

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
                    throw std::runtime_error("Database was not loaded.");
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
    nl::json interpreter::complete_request_impl(const std::string& raw_code,
                                                int cursor_pos)
    {

        nl::json result;

        const auto & keywords = get_keywords();

        nl::json matches = nl::json::array();

        // first we get  a substring from string[0:curser_pos+1]std
        // and discard the right side of the curser pos
        const auto code = raw_code.substr(0, cursor_pos);


        // keyword matches
        // ............................
        {
            auto pos = -1;
            for(int i=code.size()-1; i>=0; --i)
            {   
                if(!is_identifier(code[i]))
                {
                    pos = i;
                    break;
                }
            }
            result["cursor_start"] =  pos == -1 ? 0 : pos +1;
            auto to_match = pos == -1 ? code : code.substr(pos+1, code.size() -(pos+1));

            // check for kw matches
            for(auto kw : keywords)
            {
                if(startswith(kw, to_match))
                {
                    matches.push_back(kw);
                }
            }
        }

        result["status"] = "ok";
        result["cursor_end"] = cursor_pos;
        result["matches"] =matches;

        return result;
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


    const std::array<std::string, 826> & get_keywords()
    {
        static const std::array<std::string, 826> keywords =  
        {
            "A",
            "ABORT",
            "ABS",
            "ABSOLUTE",
            "ACCESS",
            "ACTION",
            "ADA",
            "ADD",
            "ADMIN",
            "AFTER",
            "AGGREGATE",
            "ALIAS",
            "ALL",
            "ALLOCATE",
            "ALSO",
            "ALTER",
            "ALWAYS",
            "ANALYSE",
            "ANALYZE",
            "AND",
            "ANY",
            "ARE",
            "ARRAY",
            "AS",
            "ASC",
            "ASENSITIVE",
            "ASSERTION",
            "ASSIGNMENT",
            "ASYMMETRIC",
            "AT",
            "ATOMIC",
            "ATTRIBUTE",
            "ATTRIBUTES",
            "AUDIT",
            "AUTHORIZATION",
            "AUTO_INCREMENT",
            "AVG",
            "AVG_ROW_LENGTH",
            "BACKUP",
            "BACKWARD",
            "BEFORE",
            "BEGIN",
            "BERNOULLI",
            "BETWEEN",
            "BIGINT",
            "BINARY",
            "BIT",
            "BIT_LENGTH",
            "BITVAR",
            "BLOB",
            "BOOL",
            "BOOLEAN",
            "BOTH",
            "BREADTH",
            "BREAK",
            "BROWSE",
            "BULK",
            "BY",
            "C",
            "CACHE",
            "CALL",
            "CALLED",
            "CARDINALITY",
            "CASCADE",
            "CASCADED",
            "CASE",
            "CAST",
            "CATALOG",
            "CATALOG_NAME",
            "CEIL",
            "CEILING",
            "CHAIN",
            "CHANGE",
            "CHAR",
            "CHAR_LENGTH",
            "CHARACTER",
            "CHARACTER_LENGTH",
            "CHARACTER_SET_CATALOG",
            "CHARACTER_SET_NAME",
            "CHARACTER_SET_SCHEMA",
            "CHARACTERISTICS",
            "CHARACTERS",
            "CHECK",
            "CHECKED",
            "CHECKPOINT",
            "CHECKSUM",
            "CLASS",
            "CLASS_ORIGIN",
            "CLOB",
            "CLOSE",
            "CLUSTER",
            "CLUSTERED",
            "COALESCE",
            "COBOL",
            "COLLATE",
            "COLLATION",
            "COLLATION_CATALOG",
            "COLLATION_NAME",
            "COLLATION_SCHEMA",
            "COLLECT",
            "COLUMN",
            "COLUMN_NAME",
            "COLUMNS",
            "COMMAND_FUNCTION",
            "COMMAND_FUNCTION_CODE",
            "COMMENT",
            "COMMIT",
            "COMMITTED",
            "COMPLETION",
            "COMPRESS",
            "COMPUTE",
            "CONDITION",
            "CONDITION_NUMBER",
            "CONNECT",
            "CONNECTION",
            "CONNECTION_NAME",
            "CONSTRAINT",
            "CONSTRAINT_CATALOG",
            "CONSTRAINT_NAME",
            "CONSTRAINT_SCHEMA",
            "CONSTRAINTS",
            "CONSTRUCTOR",
            "CONTAINS",
            "CONTAINSTABLE",
            "CONTINUE",
            "CONVERSION",
            "CONVERT",
            "COPY",
            "CORR",
            "CORRESPONDING",
            "COUNT",
            "COVAR_POP",
            "COVAR_SAMP",
            "CREATE",
            "CREATEDB",
            "CREATEROLE",
            "CREATEUSER",
            "CROSS",
            "CSV",
            "CUBE",
            "CUME_DIST",
            "CURRENT",
            "CURRENT_DATE",
            "CURRENT_DEFAULT_TRANSFORM_GROUP",
            "CURRENT_PATH",
            "CURRENT_ROLE",
            "CURRENT_TIME",
            "CURRENT_TIMESTAMP",
            "CURRENT_TRANSFORM_GROUP_FOR_TYPE",
            "CURRENT_USER",
            "CURSOR",
            "CURSOR_NAME",
            "CYCLE",
            "DATA",
            "DATABASE",
            "DATABASES",
            "DATE",
            "DATETIME",
            "DATETIME_INTERVAL_CODE",
            "DATETIME_INTERVAL_PRECISION",
            "DAY",
            "DAY_HOUR",
            "DAY_MICROSECOND",
            "DAY_MINUTE",
            "DAY_SECOND",
            "DAYOFMONTH",
            "DAYOFWEEK",
            "DAYOFYEAR",
            "DBCC",
            "DEALLOCATE",
            "DEC",
            "DECIMAL",
            "DECLARE",
            "DEFAULT",
            "DEFAULTS",
            "DEFERRABLE",
            "DEFERRED",
            "DEFINED",
            "DEFINER",
            "DEGREE",
            "DELAY_KEY_WRITE",
            "DELAYED",
            "DELETE",
            "DELIMITER",
            "DELIMITERS",
            "DENSE_RANK",
            "DENY",
            "DEPTH",
            "DEREF",
            "DERIVED",
            "DESC",
            "DESCRIBE",
            "DESCRIPTOR",
            "DESTROY",
            "DESTRUCTOR",
            "DETERMINISTIC",
            "DIAGNOSTICS",
            "DICTIONARY",
            "DISABLE",
            "DISCONNECT",
            "DISK",
            "DISPATCH",
            "DISTINCT",
            "DISTINCTROW",
            "DISTRIBUTED",
            "DIV",
            "DO",
            "DOMAIN",
            "DOUBLE",
            "DROP",
            "DUAL",
            "DUMMY",
            "DUMP",
            "DYNAMIC",
            "DYNAMIC_FUNCTION",
            "DYNAMIC_FUNCTION_CODE",
            "EACH",
            "ELEMENT",
            "ELSE",
            "ELSEIF",
            "ENABLE",
            "ENCLOSED",
            "ENCODING",
            "ENCRYPTED",
            "END",
            "END-EXEC",
            "ENUM",
            "EQUALS",
            "ERRLVL",
            "ESCAPE",
            "ESCAPED",
            "EVERY",
            "EXCEPT",
            "EXCEPTION",
            "EXCLUDE",
            "EXCLUDING",
            "EXCLUSIVE",
            "EXEC",
            "EXECUTE",
            "EXISTING",
            "EXISTS",
            "EXIT",
            "EXP",
            "EXPLAIN",
            "EXTERNAL",
            "EXTRACT",
            "FALSE",
            "FETCH",
            "FIELDS",
            "FILE",
            "FILLFACTOR",
            "FILTER",
            "FINAL",
            "FIRST",
            "FLOAT",
            "FLOAT4",
            "FLOAT8",
            "FLOOR",
            "FLUSH",
            "FOLLOWING",
            "FOR",
            "FORCE",
            "FOREIGN",
            "FORTRAN",
            "FORWARD",
            "FOUND",
            "FREE",
            "FREETEXT",
            "FREETEXTTABLE",
            "FREEZE",
            "FROM",
            "FULL",
            "FULLTEXT",
            "FUNCTION",
            "FUSION",
            "G",
            "GENERAL",
            "GENERATED",
            "GET",
            "GLOBAL",
            "GO",
            "GOTO",
            "GRANT",
            "GRANTED",
            "GRANTS",
            "GREATEST",
            "GROUP",
            "GROUPING",
            "HANDLER",
            "HAVING",
            "HEADER",
            "HEAP",
            "HIERARCHY",
            "HIGH_PRIORITY",
            "HOLD",
            "HOLDLOCK",
            "HOST",
            "HOSTS",
            "HOUR",
            "HOUR_MICROSECOND",
            "HOUR_MINUTE",
            "HOUR_SECOND",
            "IDENTIFIED",
            "IDENTITY",
            "IDENTITY_INSERT",
            "IDENTITYCOL",
            "IF",
            "IGNORE",
            "ILIKE",
            "IMMEDIATE",
            "IMMUTABLE",
            "IMPLEMENTATION",
            "IMPLICIT",
            "IN",
            "INCLUDE",
            "INCLUDING",
            "INCREMENT",
            "INDEX",
            "INDICATOR",
            "INFILE",
            "INFIX",
            "INHERIT",
            "INHERITS",
            "INITIAL",
            "INITIALIZE",
            "INITIALLY",
            "INNER",
            "INOUT",
            "INPUT",
            "INSENSITIVE",
            "INSERT",
            "INSERT_ID",
            "INSTANCE",
            "INSTANTIABLE",
            "INSTEAD",
            "INT",
            "INT1",
            "INT2",
            "INT3",
            "INT4",
            "INT8",
            "INTEGER",
            "INTERSECT",
            "INTERSECTION",
            "INTERVAL",
            "INTO",
            "INVOKER",
            "IS",
            "ISAM",
            "ISNULL",
            "ISOLATION",
            "ITERATE",
            "JOIN",
            "K",
            "KEY",
            "KEY_MEMBER",
            "KEY_TYPE",
            "KEYS",
            "KILL",
            "LANCOMPILER",
            "LANGUAGE",
            "LARGE",
            "LAST",
            "LAST_INSERT_ID",
            "LATERAL",
            "LEAD",
            "LEADING",
            "LEAST",
            "LEAVE",
            "LEFT",
            "LENGTH",
            "LESS",
            "LEVEL",
            "LIKE",
            "LIMIT",
            "LINENO",
            "LINES",
            "LISTEN",
            "LN",
            "LOAD",
            "LOCAL",
            "LOCALTIME",
            "LOCALTIMESTAMP",
            "LOCATION",
            "LOCATOR",
            "LOCK",
            "LOGIN",
            "LOGS",
            "LONG",
            "LONGBLOB",
            "LONGTEXT",
            "LOOP",
            "LOW_PRIORITY",
            "LOWER",
            "M",
            "MAP",
            "MATCH",
            "MATCHED",
            "MAX",
            "MAX_ROWS",
            "MAXEXTENTS",
            "MAXVALUE",
            "MEDIUMBLOB",
            "MEDIUMINT",
            "MEDIUMTEXT",
            "MEMBER",
            "MERGE",
            "MESSAGE_LENGTH",
            "MESSAGE_OCTET_LENGTH",
            "MESSAGE_TEXT",
            "METHOD",
            "MIDDLEINT",
            "MIN",
            "MIN_ROWS",
            "MINUS",
            "MINUTE",
            "MINUTE_MICROSECOND",
            "MINUTE_SECOND",
            "MINVALUE",
            "MLSLABEL",
            "MOD",
            "MODE",
            "MODIFIES",
            "MODIFY",
            "MODULE",
            "MONTH",
            "MONTHNAME",
            "MORE",
            "MOVE",
            "MULTISET",
            "MUMPS",
            "MYISAM",
            "NAME",
            "NAMES",
            "NATIONAL",
            "NATURAL",
            "NCHAR",
            "NCLOB",
            "NESTING",
            "NEW",
            "NEXT",
            "NO",
            "NO_WRITE_TO_BINLOG",
            "NOAUDIT",
            "NOCHECK",
            "NOCOMPRESS",
            "NOCREATEDB",
            "NOCREATEROLE",
            "NOCREATEUSER",
            "NOINHERIT",
            "NOLOGIN",
            "NONCLUSTERED",
            "NONE",
            "NORMALIZE",
            "NORMALIZED",
            "NOSUPERUSER",
            "NOT",
            "NOTHING",
            "NOTIFY",
            "NOTNULL",
            "NOWAIT",
            "NULL",
            "NULLABLE",
            "NULLIF",
            "NULLS",
            "NUMBER",
            "NUMERIC",
            "OBJECT",
            "OCTET_LENGTH",
            "OCTETS",
            "OF",
            "OFF",
            "OFFLINE",
            "OFFSET",
            "OFFSETS",
            "OIDS",
            "OLD",
            "ON",
            "ONLINE",
            "ONLY",
            "OPEN",
            "OPENDATASOURCE",
            "OPENQUERY",
            "OPENROWSET",
            "OPENXML",
            "OPERATION",
            "OPERATOR",
            "OPTIMIZE",
            "OPTION",
            "OPTIONALLY",
            "OPTIONS",
            "OR",
            "ORDER",
            "ORDERING",
            "ORDINALITY",
            "OTHERS",
            "OUT",
            "OUTER",
            "OUTFILE",
            "OUTPUT",
            "OVER",
            "OVERLAPS",
            "OVERLAY",
            "OVERRIDING",
            "OWNER",
            "PACK_KEYS",
            "PAD",
            "PARAMETER",
            "PARAMETER_MODE",
            "PARAMETER_NAME",
            "PARAMETER_ORDINAL_POSITION",
            "PARAMETER_SPECIFIC_CATALOG",
            "PARAMETER_SPECIFIC_NAME",
            "PARAMETER_SPECIFIC_SCHEMA",
            "PARAMETERS",
            "PARTIAL",
            "PARTITION",
            "PASCAL",
            "PASSWORD",
            "PATH",
            "PCTFREE",
            "PERCENT",
            "PERCENT_RANK",
            "PERCENTILE_CONT",
            "PERCENTILE_DISC",
            "PLACING",
            "PLAN",
            "PLI",
            "POSITION",
            "POSTFIX",
            "POWER",
            "PRECEDING",
            "PRECISION",
            "PREFIX",
            "PREORDER",
            "PREPARE",
            "PREPARED",
            "PRESERVE",
            "PRIMARY",
            "PRINT",
            "PRIOR",
            "PRIVILEGES",
            "PROC",
            "PROCEDURAL",
            "PROCEDURE",
            "PROCESS",
            "PROCESSLIST",
            "PUBLIC",
            "PURGE",
            "QUOTE",
            "RAID0",
            "RAISERROR",
            "RANGE",
            "RANK",
            "RAW",
            "READ",
            "READS",
            "READTEXT",
            "REAL",
            "RECHECK",
            "RECONFIGURE",
            "RECURSIVE",
            "REF",
            "REFERENCES",
            "REFERENCING",
            "REGEXP",
            "REGR_AVGX",
            "REGR_AVGY",
            "REGR_COUNT",
            "REGR_INTERCEPT",
            "REGR_R2",
            "REGR_SLOPE",
            "REGR_SXX",
            "REGR_SXY",
            "REGR_SYY",
            "REINDEX",
            "RELATIVE",
            "RELEASE",
            "RELOAD",
            "RENAME",
            "REPEAT",
            "REPEATABLE",
            "REPLACE",
            "REPLICATION",
            "REQUIRE",
            "RESET",
            "RESIGNAL",
            "RESOURCE",
            "RESTART",
            "RESTORE",
            "RESTRICT",
            "RESULT",
            "RETURN",
            "RETURNED_CARDINALITY",
            "RETURNED_LENGTH",
            "RETURNED_OCTET_LENGTH",
            "RETURNED_SQLSTATE",
            "RETURNS",
            "REVOKE",
            "RIGHT",
            "RLIKE",
            "ROLE",
            "ROLLBACK",
            "ROLLUP",
            "ROUTINE",
            "ROUTINE_CATALOG",
            "ROUTINE_NAME",
            "ROUTINE_SCHEMA",
            "ROW",
            "ROW_COUNT",
            "ROW_NUMBER",
            "ROWCOUNT",
            "ROWGUIDCOL",
            "ROWID",
            "ROWNUM",
            "ROWS",
            "RULE",
            "SAVE",
            "SAVEPOINT",
            "SCALE",
            "SCHEMA",
            "SCHEMA_NAME",
            "SCHEMAS",
            "SCOPE",
            "SCOPE_CATALOG",
            "SCOPE_NAME",
            "SCOPE_SCHEMA",
            "SCROLL",
            "SEARCH",
            "SECOND",
            "SECOND_MICROSECOND",
            "SECTION",
            "SECURITY",
            "SELECT",
            "SELF",
            "SENSITIVE",
            "SEPARATOR",
            "SEQUENCE",
            "SERIALIZABLE",
            "SERVER_NAME",
            "SESSION",
            "SESSION_USER",
            "SET",
            "SETOF",
            "SETS",
            "SETUSER",
            "SHARE",
            "SHOW",
            "SHUTDOWN",
            "SIGNAL",
            "SIMILAR",
            "SIMPLE",
            "SIZE",
            "SMALLINT",
            "SOME",
            "SONAME",
            "SOURCE",
            "SPACE",
            "SPATIAL",
            "SPECIFIC",
            "SPECIFIC_NAME",
            "SPECIFICTYPE",
            "SQL",
            "SQL_BIG_RESULT",
            "SQL_BIG_SELECTS",
            "SQL_BIG_TABLES",
            "SQL_CALC_FOUND_ROWS",
            "SQL_LOG_OFF",
            "SQL_LOG_UPDATE",
            "SQL_LOW_PRIORITY_UPDATES",
            "SQL_SELECT_LIMIT",
            "SQL_SMALL_RESULT",
            "SQL_WARNINGS",
            "SQLCA",
            "SQLCODE",
            "SQLERROR",
            "SQLEXCEPTION",
            "SQLSTATE",
            "SQLWARNING",
            "SQRT",
            "SSL",
            "STABLE",
            "START",
            "STARTING",
            "STATE",
            "STATEMENT",
            "STATIC",
            "STATISTICS",
            "STATUS",
            "STDDEV_POP",
            "STDDEV_SAMP",
            "STDIN",
            "STDOUT",
            "STORAGE",
            "STRAIGHT_JOIN",
            "STRICT",
            "STRING",
            "STRUCTURE",
            "STYLE",
            "SUBCLASS_ORIGIN",
            "SUBLIST",
            "SUBMULTISET",
            "SUBSTRING",
            "SUCCESSFUL",
            "SUM",
            "SUPERUSER",
            "SYMMETRIC",
            "SYNONYM",
            "SYSDATE",
            "SYSID",
            "SYSTEM",
            "SYSTEM_USER",
            "TABLE",
            "TABLE_NAME",
            "TABLES",
            "TABLESAMPLE",
            "TABLESPACE",
            "TEMP",
            "TEMPLATE",
            "TEMPORARY",
            "TERMINATE",
            "TERMINATED",
            "TEXT",
            "TEXTSIZE",
            "THAN",
            "THEN",
            "TIES",
            "TIME",
            "TIMESTAMP",
            "TIMEZONE_HOUR",
            "TIMEZONE_MINUTE",
            "TINYBLOB",
            "TINYINT",
            "TINYTEXT",
            "TO",
            "TOAST",
            "TOP",
            "TOP_LEVEL_COUNT",
            "TRAILING",
            "TRAN",
            "TRANSACTION",
            "TRANSACTION_ACTIVE",
            "TRANSACTIONS_COMMITTED",
            "TRANSACTIONS_ROLLED_BACK",
            "TRANSFORM",
            "TRANSFORMS",
            "TRANSLATE",
            "TRANSLATION",
            "TREAT",
            "TRIGGER",
            "TRIGGER_CATALOG",
            "TRIGGER_NAME",
            "TRIGGER_SCHEMA",
            "TRIM",
            "TRUE",
            "TRUNCATE",
            "TRUSTED",
            "TSEQUAL",
            "TYPE",
            "UESCAPE",
            "UID",
            "UNBOUNDED",
            "UNCOMMITTED",
            "UNDER",
            "UNDO",
            "UNENCRYPTED",
            "UNION",
            "UNIQUE",
            "UNKNOWN",
            "UNLISTEN",
            "UNLOCK",
            "UNNAMED",
            "UNNEST",
            "UNSIGNED",
            "UNTIL",
            "UPDATE",
            "UPDATETEXT",
            "UPPER",
            "USAGE",
            "USE",
            "USER",
            "USER_DEFINED_TYPE_CATALOG",
            "USER_DEFINED_TYPE_CODE",
            "USER_DEFINED_TYPE_NAME",
            "USER_DEFINED_TYPE_SCHEMA",
            "USING",
            "UTC_DATE",
            "UTC_TIME",
            "UTC_TIMESTAMP",
            "VACUUM",
            "VALID",
            "VALIDATE",
            "VALIDATOR",
            "VALUE",
            "VALUES",
            "VAR_POP",
            "VAR_SAMP",
            "VARBINARY",
            "VARCHAR",
            "VARCHAR2",
            "VARCHARACTER",
            "VARIABLE",
            "VARIABLES",
            "VARYING",
            "VERBOSE",
            "VIEW",
            "VOLATILE",
            "WAITFOR",
            "WHEN",
            "WHENEVER",
            "WHERE",
            "WHILE",
            "WIDTH_BUCKET",
            "WINDOW",
            "WITH",
            "WITHIN",
            "WITHOUT",
            "WORK",
            "WRITE",
            "WRITETEXT",
            "X509",
            "XOR",
            "YEAR",
            "YEAR_MONTH",
            "ZEROFILL",
            "ZONE"
        };
        return keywords;
    }


}
