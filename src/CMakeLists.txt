cmake_minimum_required(VERSION 3.17)
project(oplb VERSION 1.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_CXX_FLAGS "-pedantic -ansi -Wall -Wextra -Werror -O3")

add_executable(oplb main.cpp util.h util.cpp host.cpp host.h ../proxy.conf)

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

install(TARGETS oplb DESTINATION /usr/bin COMPONENT binaries)
install(FILES ../proxy.conf DESTINATION /etc/oplb COMPONENT config)
