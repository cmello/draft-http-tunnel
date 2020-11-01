#include <gtest/gtest.h>
#include "../src/HttpTunnel.h"

using namespace http_tunnel;

TEST(HttpTunnelTestSuite, TestSplitString) {
    string input { "CONNECT some.domain.com:443 HTTP/1.1\r\n"
                   "Host: some.domain.com\r\n"
                   "User-Agent: test (unknown version) CFNetwork/1128.0.1 Darwin/19.6.0 (x86_64)\r\n"
                   "Connection: keep-alive\r\n"
                   "Proxy-Connection: keep-alive\r\n\r\n"
    };

    auto split = split_string(input, "\r\n");
    EXPECT_EQ(6, split.size());
    EXPECT_EQ("CONNECT some.domain.com:443 HTTP/1.1", split[0]);
    EXPECT_EQ("Host: some.domain.com", split[1]);
    EXPECT_EQ("User-Agent: test (unknown version) CFNetwork/1128.0.1 Darwin/19.6.0 (x86_64)", split[2]);
    EXPECT_EQ("Connection: keep-alive", split[3]);
    EXPECT_EQ("Proxy-Connection: keep-alive", split[4]);
    EXPECT_EQ("", split[5]);
}

TEST(HttpTunnelTestSuite, ValidHttpRequest) {
    auto request = HttpConnectRequest::FromRequestString("CONNECT some.domain.com:443 HTTP/1.1\r\n"
                                                                 "Host: some.domain.com\r\n"
                                                                 "User-Agent: test (unknown version) CFNetwork/1128.0.1 Darwin/19.6.0 (x86_64)\r\n"
                                                                 "Connection: keep-alive\r\n"
                                                                 "Proxy-Connection: keep-alive\r\n\r\n");
    EXPECT_EQ(request.GetHost(), "some.domain.com");
    EXPECT_EQ(request.GetPort(), "443");
}