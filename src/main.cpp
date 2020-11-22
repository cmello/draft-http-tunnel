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
awaitable<void> listener(tcp::acceptor acceptor, asio::io_context& io_context)
{
    while (true) {
        //cout << "waiting connection..." << endl;
        make_shared<HttpTunnel>(co_await acceptor.async_accept(use_awaitable), io_context)->start();
    }
}

int main() {

    try {
        asio::io_context io_context;

        const int port = 8080;
        co_spawn(io_context, listener(tcp::acceptor(io_context, {tcp::v4(), port}), io_context), detached);

        asio::signal_set signals(io_context, SIGINT, SIGTERM);
        signals.async_wait([&](auto, auto) { io_context.stop(); });

        //asio::executor_work_guard executor_word_guard(io_context);
        vector<thread> threads;
        for (int i = 0; i < thread::hardware_concurrency(); i++) {
            threads.emplace_back([&io_context] { io_context.run(); });
        }
        cout << "Started " << threads.size() << " threads" << endl;

        for (auto& thread : threads) {
            thread.join();
        }
    } catch (exception& e) {
        cout << "unhandled exception: " << e.what() << endl;
        return -1;
    }

    return 0;
}
