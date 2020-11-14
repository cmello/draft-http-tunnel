#ifndef DRAFT_HTTP_TUNNEL_HTTPTUNNEL_H
#define DRAFT_HTTP_TUNNEL_HTTPTUNNEL_H

#include <iostream>
#include <memory>
#include <string>
#include <string_view>
#include <utility>
#include <cstdlib>
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

namespace http_tunnel {
    using asio::ip::tcp;
    using asio::awaitable;
    using asio::co_spawn;
    using asio::detached;
    using asio::redirect_error;
    using asio::use_awaitable;
    using namespace std;

    /**
    * Splits a string using the specified delimiter, returns string_views.
    */
    vector<string_view> split_string(const string_view& input, const string& delimiter) {
        vector<string_view> result;

        size_t last = 0;
        size_t next;

        while ((next = input.find(delimiter, last)) != string::npos) {
            result.emplace_back(&input[last], next - last);
            last = next + delimiter.size();
        }

        if (last < input.size()) {
            result.emplace_back(&input[last], input.size() - last);
        }

        return result;
    }

    /**
     * HttpConnect parse exceptions
     */
    class InvalidHttpConnectRequest : public std::exception {
    };

    class UnsupportedHttpVerb : public InvalidHttpConnectRequest {
    };

    /**
     * Parses an HTTP CONNECT request, returns the target address.
     */
    class HttpConnectRequest {
    public:
        static HttpConnectRequest FromRequestString(const string& request_string) {
            vector<string_view> request_lines = split_string(request_string, "\r\n");
            if (request_lines.empty()) {
                throw InvalidHttpConnectRequest();
            }

            vector<string_view> connect_parameters = split_string(request_lines[0], " ");
            if (connect_parameters.size() < 3) { // [0] = CONNECT, [1] = host:port, [2] = HTTP/1.1
                throw InvalidHttpConnectRequest();
            }

            if (connect_parameters[0] != "CONNECT") {
                throw UnsupportedHttpVerb();
            }

            vector<string_view> target = split_string(connect_parameters[1], ":");
            const string DEFAULT_PORT = "80";
            if (target.size() == 1) {
                return HttpConnectRequest(string(target[0]), DEFAULT_PORT);
            } else {
                return HttpConnectRequest(string(target[0]), string(target[1]));
            }
        }

        HttpConnectRequest() = delete;

        string GetHost() const {
            return _host;
        }

        string GetPort() const {
            return _port;
        }

    private:
        HttpConnectRequest(const string& host, const string& port)
                : _host(host), _port(port) {
        }

        string _host;
        string _port;
    };

    class HttpTunnel : public enable_shared_from_this<HttpTunnel> {
    public:
        explicit HttpTunnel(tcp::socket accepting_socket, asio::io_context &io_context)
                : _accepting_socket(move(accepting_socket)),
                  _io_context(io_context),
                  _target_socket(io_context),
                  _resolver(io_context) {
        }

        void start() {
            cout << "Starting tunnel" << endl;
            co_spawn(_accepting_socket.get_executor(),
                     [self = shared_from_this()] { return self->process_request(); },
                     detached);
        }

    private:
        awaitable<void> process_request() {
            try {
                string request_string;
                std::size_t n = co_await asio::async_read_until(_accepting_socket,
                                                                asio::dynamic_buffer(request_string, 1024), "\r\n",
                                                                use_awaitable);
                auto request = HttpConnectRequest::FromRequestString(request_string);
                // send reply
                string response = "HTTP/1.1 200 OK\r\n\r\n";
                co_await _accepting_socket.async_send(asio::buffer(response), use_awaitable);

                cout << "Connecting to host " << request.GetHost() << ", port " << request.GetPort() << endl;
                auto endpoints = co_await _resolver.async_resolve(request.GetHost(), request.GetPort(), use_awaitable);
                assert(endpoints.size() >= 1);

                co_await _target_socket.async_connect(*endpoints.begin(), use_awaitable);
                cout << "target native handle: " << _target_socket.native_handle() << endl;
                cout << "connected!" << endl;
                co_spawn(_io_context,
                         [self = shared_from_this()] { return self->relay_read(); },
                         detached);
                co_spawn(_io_context,
                         [self = shared_from_this()] { return self->relay_write(); },
                         detached);
            } catch (exception& ex) {
                cout << "process_requests exception: " << ex.what() << endl;
            }
        }

        awaitable<void> relay_read() {
            char data[1024 * 1024];
            try {
                while (true) {
                    std::size_t n = co_await _accepting_socket.async_read_some(asio::buffer(data, sizeof(data)),
                                                                               use_awaitable);
                    cout << "read " << n << " bytes from source. writing to target..." << endl;

                    n = co_await _target_socket.async_write_some(asio::buffer(data, n), use_awaitable);
                    cout << "wrote " << n << " bytes to target" << endl;
                }
            } catch (exception& ex) {
                cout << "relay_read exception: " << ex.what() << endl;
            }
        }

        awaitable<void> relay_write() {
            char data[1024 * 1024];
            try {

                while (true) {
                    cout << "reading from target..." << endl;
                    std::size_t n = co_await _target_socket.async_read_some(asio::buffer(data, sizeof(data)),
                                                                            use_awaitable);
                    cout << "read " << n << " bytes from target. writing to source..." << endl;

                    co_await _accepting_socket.async_write_some(asio::buffer(data, n), use_awaitable);
                    cout << "wrote " << n << " bytes to source" << endl;
                }
            } catch (exception& ex) {
                cout << "relay_write exception: " << ex.what() << endl;
            }
        }

    private:
        tcp::socket _accepting_socket;
        tcp::socket _target_socket;
        tcp::resolver _resolver;
        asio::io_context &_io_context;
    };
}

#endif //DRAFT_HTTP_TUNNEL_HTTPTUNNEL_H
