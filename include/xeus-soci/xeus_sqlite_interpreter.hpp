/***************************************************************************
* Copyright (c) 2020, QuantStack and Xeus-SQLite contributors              *
*                                                                          *
*                                                                          *
* Distributed under the terms of the BSD 3-Clause License.                 *
*                                                                          *
* The full license is in the file LICENSE, distributed with this software. *
****************************************************************************/

#ifndef XEUS_SQLITE_INTERPRETER_HPP
#define XEUS_SQLITE_INTERPRETER_HPP

#include "xeus_sqlite_config.hpp"
#include "xvega_sqlite.hpp"

#include <SQLiteCpp/SQLiteCpp.h>
#include <SQLiteCpp/VariadicBind.h>

#include "nlohmann/json.hpp"
#include "xeus/xinterpreter.hpp"

namespace nl = nlohmann;

namespace xeus_sqlite
{
    class XEUS_SQLITE_API interpreter : public xeus::xinterpreter
    {
    friend class SQLite::Database;

    public:

        interpreter() = default;
        virtual ~interpreter() = default;

    private:
        std::unique_ptr<SQLite::Database> m_db = nullptr;
        std::unique_ptr<SQLite::Database> m_backup_db = nullptr;
        bool m_bd_is_loaded = false;
        std::string m_db_path;

        void configure_impl() override;
        nl::json execute_request_impl(int execution_counter,
                                      const std::string& code,
                                      bool silent,
                                      bool store_history,
                                      nl::json user_expressions,
                                      bool allow_stdin) override;
        nl::json complete_request_impl(const std::string& code,
                                       int cursor_pos) override;
        nl::json inspect_request_impl(const std::string& code,
                                      int cursor_pos,
                                      int detail_level) override;
        nl::json is_complete_request_impl(const std::string& code) override;
        nl::json kernel_info_request_impl() override;
        void shutdown_request_impl() override;

        /**
         * Parses magic and calls the correct function.
         */
        void parse_SQLite_magic(int execution_counter, const std::vector<std::string>& tokenized_input);

        /*! \brief load_db - loads a database.
         *
         * Receives three arguments: the command %LOAD, the path of the database
         * location and a third optional parameter that might be RW or R to set the
         * read and write or the read mode, respectively.
         * If no third arguments are passed to this method, it will default to read
         * and write mode.
         *
         * param accList std::vector<std::string>& tokenized_input
         * return void
         */
        void load_db(const std::vector<std::string> tokenized_input);

        /*! \brief create_db - creates a database.
         *
         * Creates the a database in read and write mode.
         * Receives two arguments: the command %CREATE, the path to where the
         * database will be created and the name of the database.
         *
         * param accList std::vector<std::string>& tokenized_input
         * return void
         */
        void create_db(const std::vector<std::string> tokenized_input);

        /*! \brief delete_db - deletes a database.
         *
         * Deletes the last database that was either loaded or created.
         *
         * param accList std::vector<std::string>& tokenized_input
         * return void
         */
        void delete_db();

        /*! \brief table_exists - checks if a table exists in a database.
         *
         * Outputs a message in the Jupyter interface if the table exists in the
         * latest created or loaded database.
         *
         * param accList const std::string table_name
         * return void
         */
        nl::json table_exists(const std::string table_name);

        /*! \brief set_key - set or create primary key.
         *
         * param accList const std::string& aKey
         * return void
         */
        void set_key(const std::string& aKey) const;

        /*! \brief is_unencrypted - checks if a database is unencrypted.
         *
         * Outputs a message in the Jupyter interface if the latest database created
         * or loaded is encrypted or not.
         *
         * return void
         */
        nl::json is_unencrypted();

        /*! \brief get_header_info - returns information about the database.
         *
         * Outputs a message in the Jupyter interface containing information about
         the database. This method outputs the following:
         * Magic header string
         * Page size bytes
         * File format write version
         * File format read version
         * Reserved space bytes
         * Max embedded payload fraction
         * Min embedded payload fraction
         * Leaf payload fraction
         * File change counter
         * Database size pages
         * First freelist trunk page
         * Total freelist trunk pages
         * Schema cookie
         * Schema format number
         * Default page cache size bytes
         * Largest B tree page number
         * Database text encoding
         * User version
         * Incremental vaccum mode
         * Application ID
         * Version valid for
         * SQLite version
         *
         * return void
         */
        nl::json get_header_info();

        /*! \brief get_header_info - backups a database.
         *
         * This function is used to load the contents of a database file on disk
         * into the "main" database of open database connection pInMemory, or
         * to save the current contents of the database opened by pInMemory into
         * a database file on disk.
         *
         * return void
         */
        void backup(std::string backup_type);


        /*! \brief get_header_info - backups a database.
         *
         * Runs pure SQLite code. Sends the result as HTML or Text to the front
         * end.
         *
         * return void
         */
        void process_SQLite_input(int execution_counter,
                                        std::unique_ptr<SQLite::Database> &m_db,
                                        const std::string& code,
                                        xv::df_type& xv_sqlite_df);
    };
}

#endif
