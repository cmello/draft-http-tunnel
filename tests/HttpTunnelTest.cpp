//#include <gtest/gtest.h>
//#include "../src/HttpTunnel.h"
//
//#include <chrono>
//#include <iostream>
//using namespace http_tunnel;
//using namespace std;
//
///**
// * Slow implementation of split string, returns copies instead of string_views. Used in tests to compare
// * performance with the implementation using string_view.
// */
//vector<string> split_string_copy(const string& input, const string& delimiter) {
//    vector<string> result;
//
//    size_t last = 0;
//    size_t next;
//
//    while ((next = input.find(delimiter, last)) != string::npos) {
//        result.push_back(input.substr(last, next - last));
//        last = next + delimiter.size();
//    }
//
//    if (last < input.size()) {
//        result.push_back(input.substr(last));
//    }
//
//    return result;
//}
//
//TEST(HttpTunnelTestSuite, TestSplitString) {
//    string input { "CONNECT some.domain.com:443 HTTP/1.1\r\n"
//                   "Host: some.domain.com\r\n"
//                   "User-Agent: test (unknown version) CFNetwork/1128.0.1 Darwin/19.6.0 (x86_64)\r\n"
//                   "Connection: keep-alive\r\n"
//                   "Proxy-Connection: keep-alive\r\n\r\n"
//    };
//
//    auto split = split_string(input, "\r\n");
//    EXPECT_EQ(6, split.size());
//    EXPECT_EQ("CONNECT some.domain.com:443 HTTP/1.1", split[0]);
//    EXPECT_EQ("Host: some.domain.com", split[1]);
//    EXPECT_EQ("User-Agent: test (unknown version) CFNetwork/1128.0.1 Darwin/19.6.0 (x86_64)", split[2]);
//    EXPECT_EQ("Connection: keep-alive", split[3]);
//    EXPECT_EQ("Proxy-Connection: keep-alive", split[4]);
//    EXPECT_EQ("", split[5]);
//}
//
//TEST(HttpTunnelTestSuite, TestSlowSplitStringPerf) {
//    string input { "CONNECT some.domain.com:443 HTTP/1.1\r\n"
//                   "Host: some.domain.com\r\n"
//                   "User-Agent: test (unknown version) CFNetwork/1128.0.1 Darwin/19.6.0 (x86_64)\r\n"
//                   "Connection: keep-alive\r\n"
//                   "Proxy-Connection: keep-alive\r\n\r\n"
//    };
//
//    const int repetitions = 1000000;
//    auto start = chrono::steady_clock::now();
//
//    for (int i = 0; i < repetitions; i++) {
//        input[24] = '0' + (i % 10);
//        auto split = split_string_copy(input, "\r\n");
//        EXPECT_EQ(6, split.size());
//        EXPECT_EQ("Host: some.domain.com", split[1]);
//        EXPECT_EQ("User-Agent: test (unknown version) CFNetwork/1128.0.1 Darwin/19.6.0 (x86_64)", split[2]);
//        EXPECT_EQ("Connection: keep-alive", split[3]);
//        EXPECT_EQ("Proxy-Connection: keep-alive", split[4]);
//        EXPECT_EQ("", split[5]);
//    }
//
//    auto end = chrono::steady_clock::now();
//    cout << "Elapsed time in microseconds : "
//         << chrono::duration_cast<chrono::microseconds>(end - start).count()
//         << " µs" << endl;
//}
//
//TEST(HttpTunnelTestSuite, TestSplitStringPerf) {
//    string input { "CONNECT some.domain.com:443 HTTP/1.1\r\n"
//                   "Host: some.domain.com\r\n"
//                   "User-Agent: test (unknown version) CFNetwork/1128.0.1 Darwin/19.6.0 (x86_64)\r\n"
//                   "Connection: keep-alive\r\n"
//                   "Proxy-Connection: keep-alive\r\n\r\n"
//    };
//
//    const int repetitions = 1000000;
//    auto start = chrono::steady_clock::now();
//
//    for (int i = 0; i < repetitions; i++) {
//        input[24] = '0' + (i % 10);
//        auto split = split_string(input, "\r\n");
//        EXPECT_EQ(6, split.size());
//        EXPECT_EQ("Host: some.domain.com", split[1]);
//        EXPECT_EQ("User-Agent: test (unknown version) CFNetwork/1128.0.1 Darwin/19.6.0 (x86_64)", split[2]);
//        EXPECT_EQ("Connection: keep-alive", split[3]);
//        EXPECT_EQ("Proxy-Connection: keep-alive", split[4]);
//        EXPECT_EQ("", split[5]);
//    }
//
//    auto end = chrono::steady_clock::now();
//    cout << "Elapsed time in microseconds : "
//         << chrono::duration_cast<chrono::microseconds>(end - start).count()
//         << " µs" << endl;
//}
//
//TEST(HttpTunnelTestSuite, ValidHttpRequest) {
//    auto request = HttpConnectRequest::FromRequestString("CONNECT some.domain.com:443 HTTP/1.1\r\n"
//                                                                 "Host: some.domain.com\r\n"
//                                                                 "User-Agent: test (unknown version) CFNetwork/1128.0.1 Darwin/19.6.0 (x86_64)\r\n"
//                                                                 "Connection: keep-alive\r\n"
//                                                                 "Proxy-Connection: keep-alive\r\n\r\n");
//    EXPECT_EQ(request.GetHost(), "some.domain.com");
//    EXPECT_EQ(request.GetPort(), "443");
//}
