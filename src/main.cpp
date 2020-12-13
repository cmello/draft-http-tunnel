#include <iostream>
#include <memory>
#include <string>
#include <utility>
#include <cstdlib>
#include <thread>
#include <sstream>

#include <trantor/net/TcpServer.h>
#include <trantor/utils/Logger.h>
#include <trantor/net/EventLoopThread.h>

#include "HttpTunnel.h"

using namespace std;
using namespace http_tunnel;
using namespace trantor;

class InvalidCommandLineException : public std::exception {
    const char* what() const noexcept override {
        return "Invalid command-line parameters";
    }
};

class Configuration {

public:
    static void print_help_and_quit() {
        cout << "usage: draft_http_tunnel [OPTIONS] --bind <BIND>" << endl;
        cout << "example: draft_http_tunnel --bind 0.0.0.0:8888 [tcp] [--destination host:port]" << endl;
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

        auto iTcpParameter = find(args.begin(), args.end(), "tcp");
        bool is_tcp_proxy = false;
        if (iTcpParameter != args.end()) {
            is_tcp_proxy = true;
        }

        unsigned short destination_port = 80;
        string destination_host;
        auto iDestinationParameter = find(args.begin(), args.end(), "--destination");

        if (iDestinationParameter != args.end()) {
            iDestinationParameter++;
            if (iDestinationParameter == args.end()) {
                // --bind is the last parameter, missing bind value
                print_help_and_quit();
            }

            string destination_param_value = *iDestinationParameter;
            auto destination_parts = split_string(destination_param_value, ":");
            if (destination_parts.size() != 2) {
                cout << "Value specified for --destination is invalid: " << bind_param_value << ". valid example: 0.0.0.0:8080" << endl;
                print_help_and_quit();
            }

            destination_host = destination_parts[0];
            istringstream is(string { destination_parts[1] });
            is >> destination_port;
        }

        return Configuration(port, is_tcp_proxy, destination_host, destination_port);
    }

    unsigned short GetPort() const {
        return _port;
    }

    bool IsTcpProxy() const {
        return _is_tcp_proxy;
    }

    string GetDestinationHost() const {
        return _destination_host;
    }

    unsigned short GetDestinationPort() const {
        return _destination_port;
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
    Configuration(unsigned short port, bool is_tcp_proxy, string destination_host, unsigned int destination_port)
        : _port(port), _is_tcp_proxy(is_tcp_proxy), _destination_host(destination_host), _destination_port(destination_port) {
    }

    int _port;
    bool _is_tcp_proxy;
    int _destination_port;
    string _destination_host;
};

class Content {
public:
    Content() {
        cout << "Content()" << this << endl;
    }

    Content(const Content& other) {
        cout << "Content(Content& other)" << this  << endl;
    }

    ~Content() {
        cout << "~Content()" << this << endl;
    }
};

void start_server(Configuration configuration, InetAddress( addr), string server_name) {
    EventLoop loop;
    TcpServer server(&loop, addr, server_name);
    server.setRecvMessageCallback(
            [](const TcpConnectionPtr &connectionPtr, MsgBuffer *buffer) {
                auto tunnel = connectionPtr->getContext<HttpTunnel>();
                if (tunnel != nullptr) {
                    tunnel->on_client_message_received(buffer);
                } else {
                    cout << "ignoring message received to null tunnel" << endl;
                }
            });
    server.setConnectionCallback([configuration](const TcpConnectionPtr &connPtr) {
        if (connPtr->connected())
        {
            //LOG_DEBUG << "New connection";
            auto tunnel = make_shared<HttpTunnel> (connPtr,
                                                   configuration.IsTcpProxy(),
                                                   configuration.GetDestinationPort(),
                                                   configuration.GetDestinationHost());
            connPtr->setContext(tunnel);
            connPtr->setTcpNoDelay(true);
            tunnel->start();
        }
        else if (connPtr->disconnected())
        {
            //LOG_DEBUG << "connection disconnected";
            auto tunnel = connPtr->getContext<HttpTunnel>();
            if (tunnel != nullptr) {
                tunnel->graceful_shutdown();
            } else {
                cout << "ignoring disconnect notification sent to null tunnel" << endl;
            }
        }
    });
    //server.setIoLoopNum(thread::hardware_concurrency());
    server.start();

    loop.loop();
}

int main(int argc, char** argv) {

    try {
        auto configuration = Configuration::from_command_line(argc, argv);
        auto port = configuration.GetPort();

        cout << "Bind port: " << port << endl;
        if (configuration.IsTcpProxy()) {
            cout << "TCP Proxy mode. destination host: " << configuration.GetDestinationHost()
                 << ", destination port: " << configuration.GetDestinationPort() << endl;
        }

        InetAddress addr(port);

        vector<thread> threads;
        for (int i = 0; i < thread::hardware_concurrency(); i++) {
            threads.emplace_back(
                    [addr, configuration] {
                        start_server(configuration, addr, "t1");
                    }
            );
        }

        for(auto& thread : threads) {
            thread.join();
        }

    } catch (exception& e) {
        cout << endl << "Unhandled exception: " << e.what() << endl;
        return -1;
    }

    return 0;
}
