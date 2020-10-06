cmake_minimum_required(VERSION 3.17)
project(oplb VERSION 1.0.0)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_FLAGS "-pedantic -ansi -Wall -Wextra -Werror -O3")

add_executable(oplb main.cpp util.h util.cpp host.cpp host.h)

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
