/***************************************************************************
* Copyright (c) 2020, QuantStack and Xeus-SQLite contributors              *
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

#include "xeus-sqlite/xeus_sqlite_interpreter.hpp"

#include "soci/soci.h"
#include "soci/sqlite3/soci-sqlite3.h"

#include <SQLiteCpp/VariadicBind.h>
#include <SQLiteCpp/SQLiteCpp.h>

namespace xeus_sqlite
{

    void interpreter::load_db(const std::vector<std::string> tokenized_input)
    {
        /*
            Loads the database. If the open mode is not specified it defaults
            to read and write mode.
        */

        if (tokenized_input.back().find("rw") != std::string::npos)
        {
            m_bd_is_loaded = true;
            m_db = std::make_unique<SQLite::Database>(m_db_path,
                        SQLite::OPEN_READWRITE);
        }
        else if (tokenized_input.back().find("r") != std::string::npos)
        {
            m_bd_is_loaded = true;
            m_db = std::make_unique<SQLite::Database>(m_db_path,
                        SQLite::OPEN_READONLY);
        }
        /* Opening as read and write because mode is unspecified */
        else if (tokenized_input.size() < 4)
        {
            m_bd_is_loaded = true;
            m_db = std::make_unique<SQLite::Database>(m_db_path,
                        SQLite::OPEN_READWRITE);
        }
        else
        {
            throw std::runtime_error("Wasn't able to load the database correctly.");
        }
    }

    void interpreter::create_db(const std::vector<std::string> tokenized_input)
    {
        m_bd_is_loaded = true;
        m_db_path = tokenized_input[2];

        /* Creates the file */
        std::ofstream(m_db_path.c_str()).close();

        /* Creates the database */
        m_db = std::make_unique<SQLite::Database>(m_db_path,
                                                    SQLite::OPEN_READWRITE |
                                                    SQLite::OPEN_CREATE);
    }

    void interpreter::delete_db()
    {
        /*
            Deletes the database.
        */

        if(std::remove(m_db_path.c_str()) != 0)
        {
            throw std::runtime_error("Error deleting file.");
        }
    }

    nl::json interpreter::table_exists(const std::string table_name)
    {
        nl::json pub_data;
        if (m_db->SQLite::Database::tableExists(table_name.c_str()))
        {
            pub_data["text/plain"] = "The table " + table_name + " exists.";
            return pub_data;
        }
        else
        {
            pub_data["text/plain"] = "The table " + table_name + " doesn't exist.";
            return pub_data;
        }
    }

    nl::json interpreter::is_unencrypted()
    {
        nl::json pub_data;
        if (SQLite::Database::isUnencrypted(m_db_path))
        {
            pub_data["text/plain"] = "The database is unencrypted.";
            return pub_data;
        }
        else
        {
            pub_data["text/plain"] = "The database is encrypted.";
            return pub_data;
        }
    }

    nl::json interpreter::get_header_info()
    {
        SQLite::Header header;
        header = SQLite::Database::getHeaderInfo(m_db_path);

    /* Official documentation for fields can be found here:
        https://www.sqlite.org/fileformat.html#the_database_header*/
        nl::json pub_data;
        pub_data["text/plain"] =
            "Magic header string: "           + std::string(&header.headerStr[0], &header.headerStr[15]) + "\n" +
            "Page size bytes: "               + std::to_string(header.pageSizeBytes)                     + "\n" +
            "File format write version: "     + std::to_string(header.fileFormatWriteVersion)            + "\n" +
            "File format read version: "      + std::to_string(header.fileFormatReadVersion)             + "\n" +
            "Reserved space bytes: "          + std::to_string(header.reservedSpaceBytes)                + "\n" +
            "Max embedded payload fraction "  + std::to_string(header.maxEmbeddedPayloadFrac)            + "\n" +
            "Min embedded payload fraction: " + std::to_string(header.minEmbeddedPayloadFrac)            + "\n" +
            "Leaf payload fraction: "         + std::to_string(header.leafPayloadFrac)                   + "\n" +
            "File change counter: "           + std::to_string(header.fileChangeCounter)                 + "\n" +
            "Database size pages: "           + std::to_string(header.databaseSizePages)                 + "\n" +
            "First freelist trunk page: "     + std::to_string(header.firstFreelistTrunkPage)            + "\n" +
            "Total freelist trunk pages: "    + std::to_string(header.totalFreelistPages)                + "\n" +
            "Schema cookie: "                 + std::to_string(header.schemaCookie)                      + "\n" +
            "Schema format number: "          + std::to_string(header.schemaFormatNumber)                + "\n" +
            "Default page cache size bytes: " + std::to_string(header.defaultPageCacheSizeBytes)         + "\n" +
            "Largest B tree page number: "    + std::to_string(header.largestBTreePageNumber)            + "\n" +
            "Database text encoding: "        + std::to_string(header.databaseTextEncoding)              + "\n" +
            "User version: "                  + std::to_string(header.userVersion)                       + "\n" +
            "Incremental vaccum mode: "       + std::to_string(header.incrementalVaccumMode)             + "\n" +
            "Application ID: "                + std::to_string(header.applicationId)                     + "\n" +
            "Version valid for: "             + std::to_string(header.versionValidFor)                   + "\n" +
            "SQLite version: "                + std::to_string(header.sqliteVersion)                     + "\n";
        return pub_data;
    }

    void interpreter::backup(std::string backup_type)
    {
        if (backup_type.size() > 1 && (int)backup_type[0] <= 1)
        {
            throw std::runtime_error("This is not a valid backup type.");
        }
        else
        {
            m_backup_db->SQLite::Database::backup(m_db_path.c_str(),
                (SQLite::Database::BackupType)backup_type[0]);
        }
    }

    void interpreter::parse_SQLite_magic(int execution_counter,
                                    const std::vector<
                                        std::string>& tokenized_input)
    {
        if (xv_bindings::case_insentive_equals(tokenized_input[0], "LOAD"))
        {
            m_db_path = tokenized_input[1];
            std::ifstream path_is_valid(m_db_path);
            if (!path_is_valid.is_open())
            {
                throw std::runtime_error("The path doesn't exist.");
            }
            else
            {
                return load_db(tokenized_input);
            }
        }
        else if (xv_bindings::case_insentive_equals(tokenized_input[0], "CREATE"))
        {
            return create_db(tokenized_input);
        }
        if (m_bd_is_loaded)
        {
            if (xv_bindings::case_insentive_equals(tokenized_input[0], "DELETE"))
            {
                delete_db();
            }
            else if (xv_bindings::case_insentive_equals(tokenized_input[0], "TABLE_EXISTS"))
            {
                publish_execution_result(execution_counter,
                    std::move(table_exists(tokenized_input[1])),
                    nl::json::object());
            }
            else if (xv_bindings::case_insentive_equals(tokenized_input[0], "LOAD_EXTENSION"))
            {
                //TODO: add a try catch to treat all void functions
                m_db->SQLite::Database::loadExtension(tokenized_input[1].c_str(),
                        tokenized_input[2].c_str());
            }
            else if (xv_bindings::case_insentive_equals(tokenized_input[0], "SET_KEY"))
            {
                m_db->SQLite::Database::key(tokenized_input[1]);
            }
            else if (xv_bindings::case_insentive_equals(tokenized_input[0], "REKEY"))
            {
                m_db->SQLite::Database::rekey(tokenized_input[1]);
            }
            else if (xv_bindings::case_insentive_equals(tokenized_input[0], "IS_UNENCRYPTED"))
            {
                publish_execution_result(execution_counter,
                    std::move(is_unencrypted()),
                    nl::json::object());
            }
            else if (xv_bindings::case_insentive_equals(tokenized_input[0], "GET_INFO"))
            {
                publish_execution_result(execution_counter,
                    std::move(get_header_info()),
                    nl::json::object());
            }
            else if (xv_bindings::case_insentive_equals(tokenized_input[0], "BACKUP"))
            {
                backup(tokenized_input[1]);
            }
        }
        else
        {
            throw std::runtime_error("Load a database to run this command.");
        }
    }

    void interpreter::configure_impl()
    {
    }

    void interpreter::process_SQLite_input(int execution_counter,
                                        std::unique_ptr<SQLite::Database> &m_db,
                                        const std::string& code,
                                        xv::df_type& xv_sqlite_df)
    {
        SQLite::Statement query(*m_db, code);
        nl::json pub_data;

        /* Builds text/plain output */
        tabulate::Table plain_table;

        /* Builds text/html output */
        std::stringstream html_table("");

        if (query.getColumnCount() != 0)
        {
            std::vector<std::variant<
                            std::string,
                            const char*,
                            tabulate::Table>> col_names;

            /* Builds text/html output */
            html_table << "<table>\n<tr>\n";

            /* Iterates through cols name and build table's title row */
            for (int col = 0; col < query.getColumnCount(); col++) {
                std::string name = query.getColumnName(col);

                /* Builds text/plain output */
                col_names.push_back(name);

                /* Builds text/html output */
                html_table << "<th>" << name << "</th>\n";

                /* Build application/vnd.vegalite.v3+json output */
                xv_sqlite_df[name] = { "name" };
            }
            /* Builds text/plain output */
            plain_table.add_row(col_names);

            /* Builds text/html output */
            html_table << "</tr>\n";

            /* Iterates through cols' rows and builds different kinds of
               outputs
            */
            while (query.executeStep())
            {
                /* Builds text/html output */
                html_table << "<tr>\n";

                std::vector<std::variant<
                                std::string,
                                const char*,
                                tabulate::Table>> row;

                for (int col = 0; col < query.getColumnCount(); col++) {
                    std::string col_name;
                    col_name = query.getColumnName(col);
                    std::string cell = query.getColumn(col);

                    /* Builds text/plain output */
                    row.push_back(cell);

                    /* Builds text/html output */
                    html_table << "<td>" << cell << "</td>\n";

                    /* Build application/vnd.vegalite.v3+json output */
                    xv_sqlite_df[col_name].push_back(cell);
                }
                /* Builds text/html output */
                html_table << "</tr>\n";

                /* Builds text/plain output */
                plain_table.add_row(row);
            }
            /* Builds text/html output */
            html_table << "</table>";

            pub_data["text/plain"] = plain_table.str();
            pub_data["text/html"] = html_table.str();

            publish_execution_result(execution_counter,
                                        std::move(pub_data),
                                        nl::json::object());
        }
        else
        {
            query.exec();
        }
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
        //TODO: but it ends up being used in process_SQLite_input, that's why
        //it's initialized here. Not the best approach, this should be
        //compartimentilized under xvega domain.
        xv::df_type xv_sqlite_df;

        try
        {
            /* Runs magic */
            if(xv_bindings::is_magic(tokenized_input))
            {
                /* Removes "%" symbol */
                tokenized_input[0].erase(0, 1);

                /* Runs SQLite magic */
                parse_SQLite_magic(execution_counter, tokenized_input);

                /* Runs xvega magic and SQLite code */
                if(xv_bindings::is_xvega(tokenized_input))
                {
                    /* Removes XVEGA_PLOT command */
                    tokenized_input.erase(tokenized_input.begin());

                    nl::json chart;
                    std::vector<std::string> xvega_input, sqlite_input;

                    std::tie(xvega_input, sqlite_input) = 
                        xv_sqlite::split_xv_sqlite_input(tokenized_input);

                    /* Stringfies SQLite code again */
                    std::stringstream stringfied_sqlite_input;
                    for (size_t i = 0; i < sqlite_input.size(); i++) {
                        stringfied_sqlite_input << " " << sqlite_input[i];
                    }

                    process_SQLite_input(execution_counter,
                                         m_db,
                                         stringfied_sqlite_input.str(),
                                         xv_sqlite_df);

                    chart = xv_bindings::process_xvega_input(xvega_input,
                                                           xv_sqlite_df);

                    publish_execution_result(execution_counter,
                                             std::move(chart),
                                             nl::json::object());
                }
            }
            /* Runs SQLite code */
            else
            {
                process_SQLite_input(execution_counter, m_db, code, xv_sqlite_df);
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
        result["implementation"] = "xsqlite";
        result["implementation_version"] = "0.1.0";

        /* The jupyter-console banner for xeus-sqlite is the following:
            _  _ ____ _  _ ____    ____ ____ _    _ ___ ____
             \/  |___ |  | [__  __ [__  |  | |    |  |  |___
            _/\_ |___ |__| ___]    ___] |_\| |___ |  |  |___
           xeus-SQLite: a Jupyter kernel for SQLite
           SQLite version: x.x.x
        */

        std::string banner = ""
              "_  _ ____ _  _ ____    ____ ____ _    _ ___ ____\n"
              " \\/  |___ |  | [__  __ [__  |  | |    |  |  |___\n"
              "_/\\_ |___ |__| ___]    ___] |_\\| |___ |  |  |___\n"
              "  xeus-SQLite: a Jupyter kernel for SQLite\n"
              "  SQLite version: ";
        banner.append(SQLite::VERSION);

        result["banner"] = banner;
        result["language_info"]["name"] = "sqlite3";
        result["language_info"]["codemirror_mode"] = "sql";
        result["language_info"]["version"] = SQLite::VERSION;
        result["language_info"]["mimetype"] = "";
        result["language_info"]["file_extension"] = "";
        return result;
    }

    void interpreter::shutdown_request_impl()
    {
    }

}
