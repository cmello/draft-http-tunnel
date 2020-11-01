cmake_minimum_required(VERSION 3.17)
project(draft_http_tunnel)

set(CMAKE_CXX_STANDARD 20)

#set(CMAKE_CXX_STANDARD_REQUIRED ON)
#set(CMAKE_CXX_EXTENSIONS OFF)
#set(PREBUILT_MODULE_PATH ${CMAKE_BINARY_DIR}/modules)

function(add_module name)
    file(MAKE_DIRECTORY ${PREBUILT_MODULE_PATH})
    add_custom_target(${name}.pcm
            COMMAND
            ${CMAKE_CXX_COMPILER}
            -std=c++2a
            -stdlib=libc++
            -fmodules
            -c
            ${CMAKE_CURRENT_SOURCE_DIR}/${ARGN}
            -Xclang -emit-module-interface
            -o ${PREBUILT_MODULE_PATH}/${name}.pcm

            )
endfunction()

#add_compile_options(-fmodules)
#add_compile_options(-stdlib=libc++)
#add_compile_options(-fbuiltin-module-map)
#add_compile_options(-fimplicit-module-maps)
#add_compile_options(-fprebuilt-module-path=${PREBUILT_MODULE_PATH})

add_subdirectory(dependencies/googletest)
add_subdirectory(tests)

include_directories(dependencies/asio/asio/include)

#add_module(HttpTunnelModule src/HttpTunnel.cpp)
add_executable(draft_http_tunnel src/main.cpp src/HttpTunnel.h)

#add_dependencies(draft_http_tunnel HttpTunnelModule.pcm)