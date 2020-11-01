project(Tests)

#include_directories(../dependencies/googletest/googletest/include)
include_directories(../dependencies/asio/asio/include)

# adding the Google_Tests_run target
add_executable(Google_Tests_run HttpTunnelTest.cpp)

# linking Google_Tests_run with DateConverter_lib which will be tested
#target_link_libraries(Google_Tests_run DateConverter_lib)

target_link_libraries(Google_Tests_run gtest gtest_main)
