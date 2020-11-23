#include <iostream>
#include <memory>
#include <string>
#include <utility>
#include <cstdlib>
#include <thread>
#include <asio/awaitable.hpp>
#include <asio/detached.hpp>
#include <asio/co_spawn.hpp>
#include <asio/io_context.hpp>
#include <asio/ip/tcp.hpp>
#include <asio/read_until.hpp>
#include <asio/redirect_error.hpp>
#include <asio/signal_set.hpp>
#include <asio/steady_timer.hpp>
#include <asio/use_awaitable.hpp>
#include <asio/write.hpp>
#include <sstream>
#include "HttpTunnel.h"

using asio::ip::tcp;
using asio::awaitable;
using asio::co_spawn;
using asio::detached;
using asio::redirect_error;
using asio::use_awaitable;
using namespace std;
using namespace http_tunnel;

/**
 * TCP listener
 */
awaitable<void> listener(tcp::acceptor& acceptor, asio::io_context& io_context)
{
    while (true) {
        try {
            make_shared<HttpTunnel>(co_await acceptor.async_accept(use_awaitable), io_context)->start();
        } catch (exception& ex) {
            cout << "Error accepting more connections. Details: " << ex.what() << endl;
            this_thread::sleep_for(1s);
        }
    }
}

class InvalidCommandLineException : public std::exception {
    const char* what() const noexcept override {
        return "Invalid command-line parameters";
    }
};

class Configuration {

public:
    static void print_help_and_quit() {
        cout << "usage: draft_http_tunnel [OPTIONS] --bind <BIND>" << endl;
        cout << "example: draft_http_tunnel --bind 0.0.0.0:8888" << endl;
        throw InvalidCommandLineException();
    }

    static Configuration from_command_line(int argc, char** argv) {
        auto args = get_args_from_main(argc, argv);
        if (args.size() < 3) {
            print_help_and_quit();
        }

        auto iBindParameter = find(args.begin(), args.end(), "--bind");
        if (iBindParameter == args.end()) {
            print_help_and_quit();
        }

        iBindParameter++;
        if (iBindParameter == args.end()) {
            // --bind is the last parameter, missing bind value
            print_help_and_quit();
        }

        string bind_param_value = *iBindParameter;
        auto bind_parts = split_string(bind_param_value, ":");
        if (bind_parts.size() != 2) {
            cout << "Value specified for --bind is invalid: " << bind_param_value << ". valid example: 0.0.0.0:8080" << endl;
            print_help_and_quit();
        }

        unsigned short port = 8080;
        istringstream is(string { bind_parts[1] });
        is >> port;

        return Configuration(port);
    }

    unsigned short GetPort() const {
        return _port;
    }

private:
    static vector<string> get_args_from_main(int argc, char** argv) {
        vector<string> args;
        for (int i = 0; i < argc; i++) {
            args.push_back(argv[i]);
        }
        return args;
    }

    Configuration() = delete;
    Configuration(unsigned short port) : _port(port) {

    }

    int _port;
};

int main(int argc, char** argv) {

    try {
        auto port = Configuration::from_command_line(argc, argv).GetPort();
        cout << "Bind port: " << port << endl;

        asio::io_context io_context;
        asio::signal_set signals(io_context, SIGINT, SIGTERM);
        signals.async_wait([&](auto, auto) { io_context.stop(); });

        tcp::acceptor acceptor (io_context, {tcp::v4(), port});
        acceptor.set_option(asio::ip::tcp::acceptor::reuse_address(true));

        co_spawn(io_context, listener(acceptor, io_context), detached);

        try {
            io_context.run();
        } catch (exception& ex) {
            cout << "io_context exception: " << endl;
        }
    } catch (exception& e) {
        cout << endl << "Unhandled exception: " << e.what() << endl;
        return -1;
    }

    return 0;
}
