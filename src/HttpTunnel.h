#ifndef DRAFT_HTTP_TUNNEL_HTTPTUNNEL_H
#define DRAFT_HTTP_TUNNEL_HTTPTUNNEL_H

#include <iostream>
#include <memory>
#include <string>
#include <string_view>
#include <utility>
#include <cstdlib>

#include <trantor/net/TcpServer.h>
#include <trantor/net/TcpClient.h>
#include <trantor/net/Resolver.h>

namespace http_tunnel {
    using namespace std;
    using namespace trantor;

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
        const char* what() const noexcept override {
            return "InvalidHttpConnectRequest";
        }
    };

    class UnsupportedHttpVerb : public InvalidHttpConnectRequest {
        const char* what() const noexcept override {
            return "UnsupportedVerb";
        }
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
        explicit HttpTunnel( trantor::TcpConnectionPtr client_connection_ptr,
                             bool is_tcp_proxy,
                             unsigned int destination_port,
                             string destination_host)
        : _client_connection(client_connection_ptr),
          _is_tcp_proxy(is_tcp_proxy),
          _destination_port(destination_port),
          _destination_host(destination_host)
        {
        }

        ~HttpTunnel() {
            //cout << "destroying tunnel" << this << endl;
        }

        void start() {
            //cout << "starting tunnel " << endl;
        }

        void on_client_message_received(MsgBuffer* buffer) {
            //cout << "client message received" << endl;
            if (!relay) {
                if (_is_tcp_proxy) {
                    process_request(_destination_port, _destination_host);
                    relay.store(true);
                    pending.store(true);
                } else {
                    string_view view(buffer->peek(), buffer->readableBytes());
                    auto pos_end_of_request = view.find("\r\n\r\n");
                    if (pos_end_of_request != string_view::npos) {
                        string request_string(view.substr(0, pos_end_of_request));
                        buffer->retrieve(pos_end_of_request + 4); // includes \r\n\r\n
                        process_http_connect_request(request_string);
                    }
                }
            } else {
                if (!_target_tcp_client || _target_tcp_client->connection() == nullptr) {
                    pending.store(true);
                    return;
                }
                //cout << "relaying from client to target..." << endl;
                _target_tcp_client->connection()->send(buffer->peek(), buffer->readableBytes());
                //cout << "client -> target: " << buffer->readableBytes() << endl;
                buffer->retrieveAll();
            }
        }

        void graceful_shutdown() {
            //cout << "graceful shutdown" << endl;
            if (_target_tcp_client != nullptr) {
                _target_tcp_client->setConnectionCallback(nullptr);
                _target_tcp_client->setMessageCallback(nullptr);
                auto target_connection = _target_tcp_client->connection();
                if (target_connection != nullptr) {
                    target_connection->clearContext();
                    target_connection->forceClose();
                }
                _target_tcp_client = nullptr;
            }

            if (_client_connection != nullptr) {
                _client_connection->clearContext();
                if (_client_connection->connected()) {
                    _client_connection->forceClose();
                }
            }
        }

    private:
        void process_http_connect_request(string request_string) {
            _request = make_unique<HttpConnectRequest> (HttpConnectRequest::FromRequestString(request_string));
            // send reply
            string response = "HTTP/1.1 200 OK\r\n\r\n";
            _client_connection->send(response);
            //cout << "Connecting to host " << _request->GetHost() << ", port " << _request->GetPort() << endl;
            auto port = atoi(_request->GetPort().c_str());
            //cout << "port number: " << port << endl;
            process_request(port, _request->GetHost());
        }

        void process_request(unsigned int destination_port, string destination_host) {
            try {
                auto loop = _client_connection->getLoop();
                auto resolver = Resolver::newResolver(loop);
                auto shared_this = shared_from_this();
                resolver->resolve(destination_host, [shared_this, loop, destination_port](const InetAddress& address) {
                    auto address_with_port = InetAddress(address.toIp(), destination_port, address.isIpV6());
                    //cout << "resolved address: " << address_with_port.toIpPort() << endl;
                    shared_this->_target_tcp_client = make_shared<TcpClient> (loop, address_with_port, "target connection");
                    shared_this->_target_tcp_client->setConnectionCallback([shared_this] (const TcpConnectionPtr& target_connection) {
                        //cout << "beginning target connection callback " << endl;
                        if (target_connection->connected()) {
                            //cout << "connection to target succeeded!" << endl;
                            shared_this->start_relay_client_loop();
                        } else {
                            //cout << "Target connection closed" << endl;
                            shared_this->graceful_shutdown();
                        }
                    });
                    shared_this->_target_tcp_client->setMessageCallback([shared_this](const TcpConnectionPtr &connectionPtr, MsgBuffer *buffer) {
                        //cout << "target -> client..." << endl;
                        shared_this->_client_connection->send(buffer->peek(), buffer->readableBytes());
                        //cout << "target -> client: " << buffer->readableBytes() << endl;
                        buffer->retrieveAll();
                    });
                    shared_this->_target_tcp_client->setConnectionErrorCallback([shared_this] () {
                        cout << "CONNECTION ERROR" << endl;
                        shared_this->graceful_shutdown();
                    });

                    shared_this->_target_tcp_client->connect();
                });
            } catch (exception& ex) {
                cout << "process_requests exception: " << ex.what() << endl;
            }

            //cout << "leaving process_request()" << endl;
        }


        void start_relay_client_loop() {
            if (pending) {
                on_client_message_received(_client_connection->getRecvBuffer());
                pending.store(false);
            }
        }

    private:
        trantor::TcpConnectionPtr  _client_connection;
        shared_ptr<trantor::TcpClient> _target_tcp_client;
        unique_ptr<HttpConnectRequest> _request;
        atomic<bool> relay = false;
        atomic<bool> pending = false;
        bool _is_tcp_proxy;
        unsigned int _destination_port;
        string _destination_host;
    };
}

#endif //DRAFT_HTTP_TUNNEL_HTTPTUNNEL_H
