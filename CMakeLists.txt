cmake_minimum_required(VERSION 3.16)
project(oplb VERSION 1.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_CXX_FLAGS "-pedantic -ansi -Wall -Wextra -Werror -O3")

add_subdirectory(src)
