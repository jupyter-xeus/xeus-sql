/***************************************************************************
* Copyright (c) 2020, QuantStack and xeus-sql contributors                *
*                                                                          *
*                                                                          *
* Distributed under the terms of the BSD 3-Clause License.                 *
*                                                                          *
* The full license is in the file LICENSE, distributed with this software. *
****************************************************************************/

#ifndef XEUS_SQL_INTERPRETER_HPP
#define XEUS_SQL_INTERPRETER_HPP

#include "nlohmann/json.hpp"
#include "xeus/xinterpreter.hpp"
#include "soci/soci.h"
#include "xvega-bindings/xvega_bindings.hpp"

#include "xeus_sql_interpreter.hpp"
#include "xeus_sql_config.hpp"


namespace nl = nlohmann;

namespace xeus_sql
{
    class XEUS_SQL_API interpreter : public xeus::xinterpreter
    {
    public:
        interpreter() = default;
        virtual ~interpreter() = default;

    private:
        void configure_impl() override;
        void execute_request_impl(send_reply_callback cb,
                                  int /*execution_counter*/,
                                  const std::string& /*code*/,
                                  xeus::execute_request_config /*config*/,
                                  nl::json /*user_expressions*/)
        nl::json complete_request_impl(const std::string& code,
                                       int cursor_pos) override;
        nl::json inspect_request_impl(const std::string& code,
                                      int cursor_pos,
                                      int detail_level) override;
        nl::json is_complete_request_impl(const std::string& code) override;
        nl::json kernel_info_request_impl() override;
        void shutdown_request_impl() override;

        nl::json process_SQL_input(const std::string& code,
                                   xv::df_type& xv_sqlite_df);

        std::unique_ptr<soci::session> sql;
        std::map<std::string, nl::json> specs;
    };
}

#endif
