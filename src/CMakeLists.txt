cmake_minimum_required(VERSION 3.16)
project(oplb VERSION 1.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_CXX_FLAGS "-pedantic -ansi -Wall -Wextra -O3 -fpermissive")
set(CMAKE_BUILD_TYPE RelWithDebInfo)

add_executable(oplb main.cpp util.hpp util.cpp proxy.cpp proxy.hpp timer.cpp timer.hpp sharding/hasher.hpp sharding/node.hpp sharding/sharder.hpp sharding/hasher.cpp sharding/node.cpp sharding/sharder.cpp ../proxy.conf)

include_directories(/usr/include)
include_directories(/usr/lib)
include_directories(/usr/local/lib)
include_directories(/usr/lib/x86_64-linux-gnu)

target_link_libraries(oplb stdc++)
target_link_libraries(oplb pthread)
target_link_libraries(oplb boost_thread)
target_link_libraries(oplb boost_system)
target_link_libraries(oplb curl)
target_link_libraries(oplb jsoncpp)
target_link_libraries(oplb rt)
target_link_libraries(oplb ssl)
target_link_libraries(oplb crypto)

install(TARGETS oplb DESTINATION /usr/bin COMPONENT binaries)
install(FILES ../proxy.conf DESTINATION /etc/oplb COMPONENT config)
