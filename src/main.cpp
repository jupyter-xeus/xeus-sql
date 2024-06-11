/***************************************************************************
* Copyright (c) 2020, QuantStack and xeus-SQLite contributors              *
*                                                                          *
*                                                                          *
* Distributed under the terms of the BSD 3-Clause License.                 *
*                                                                          *
* The full license is in the file LICENSE, distributed with this software. *
****************************************************************************/

#include <memory>
#include <iostream>
#include <signal.h>

#ifdef __GNUC__
#include <stdio.h>
#include <execinfo.h>
#include <stdlib.h>
#include <unistd.h>
#endif

#include "xeus/xkernel.hpp"
#include "xeus/xkernel_configuration.hpp"

#include "xeus-zmq/xserver_zmq_split.hpp"
#include "xeus-zmq/xzmq_context.hpp"

#include "xeus-sql/xeus_sql_interpreter.hpp"

#ifdef __GNUC__
void handler(int sig)
{
    void* array[10];

    // get void*'s for all entries on the stack
    size_t size = backtrace(array, 10);

    // print out all the frames to stderr
    fprintf(stderr, "Error: signal %d:\n", sig);
    backtrace_symbols_fd(array, size, STDERR_FILENO);
    exit(1);
}
#endif

void stop_handler(int /*sig*/)
{
    exit(0);
}

std::string extract_filename(int& argc, char* argv[])
{
    std::string res = "";
    for (int i = 0; i < argc; ++i)
    {
        if ((std::string(argv[i]) == "-f") && (i + 1 < argc))
        {
            res = argv[i + 1];
            for (int j = i; j < argc - 2; ++j)
            {
                argv[j] = argv[j + 2];
            }
            argc -= 2;
            break;
        }
    }
    return res;
}

int main(int argc, char* argv[])
{
   // Registering SIGSEGV handler
#ifdef __GNUC__
    std::clog << "registering handler for SIGSEGV" << std::endl;
    signal(SIGSEGV, handler);

    // Registering SIGINT and SIGKILL handlers
    signal(SIGKILL, stop_handler);
#endif
    signal(SIGINT, stop_handler);

    // Load configuration file
    std::string file_name = extract_filename(argc, argv);

    std::unique_ptr<xeus::xcontext> context = xeus::make_zmq_context();

    // Create interpreter instance
    using interpreter_ptr = std::unique_ptr<xeus_sql::interpreter>;
    interpreter_ptr interpreter = std::make_unique<xeus_sql::interpreter>();

    // Create kernel instance and start it
    // xeus::xkernel kernel(config, xeus::get_user_name(), std::move(interpreter));
    // kernel.start();
    using history_manager_ptr = std::unique_ptr<xeus::xhistory_manager>;
    history_manager_ptr hist = xeus::make_in_memory_history_manager();

    try
    {
        if (!file_name.empty())
        {
            xeus::xconfiguration config = xeus::load_configuration(file_name);

            xeus::xkernel kernel(config,
                                 xeus::get_user_name(),
                                 std::move(context),
                                 std::move(interpreter),
                                 xeus::make_xserver_shell_main,
                                 std::move(hist),
                                 xeus::make_console_logger(xeus::xlogger::msg_type,
                                                           xeus::make_file_logger(xeus::xlogger::content, "xeus.log")));

            std::clog <<
                "Starting xeus-sql kernel...\n\n"
                "If you want to connect to this kernel from an other client, you can use"
                " the " + file_name + " file."
                << std::endl;

            kernel.start();
        }
        else
        {
            xeus::xkernel kernel(xeus::get_user_name(),
                                 std::move(context),
                                 std::move(interpreter),
                                 xeus::make_xserver_shell_main,
                                 std::move(hist),
                                 nullptr);

            const auto& config = kernel.get_config();
            std::clog <<
                "Starting xeus-sql kernel...\n\n"
                "If you want to connect to this kernel from an other client, just copy"
                " and paste the following content inside of a `kernel.json` file. And then run for example:\n\n"
                "# jupyter console --existing kernel.json\n\n"
                "kernel.json\n```\n{\n"
                "    \"transport\": \"" + config.m_transport + "\",\n"
                "    \"ip\": \"" + config.m_ip + "\",\n"
                "    \"control_port\": " + config.m_control_port + ",\n"
                "    \"shell_port\": " + config.m_shell_port + ",\n"
                "    \"stdin_port\": " + config.m_stdin_port + ",\n"
                "    \"iopub_port\": " + config.m_iopub_port + ",\n"
                "    \"hb_port\": " + config.m_hb_port + ",\n"
                "    \"signature_scheme\": \"" + config.m_signature_scheme + "\",\n"
                "    \"key\": \"" + config.m_key + "\"\n"
                "}\n```\n";

            kernel.start();
        }
    }
    catch(std::exception& e)
    {
        std::clog << e.what() << std::endl;
        // This is not clean, but we need an additional API in xeus to
        // stop the kernel in a clean way
        throw e;
    }
    catch(const char* p)
    {
        std::clog << p << std::endl;
        // This is not clean, but we need an additional API in xeus to
        // stop the kernel in a clean way
        throw p;
    }

    return 0;
}

