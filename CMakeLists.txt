cmake_minimum_required(VERSION 3.9)
project(mcomp)

set(CMAKE_C_FLAGS_MINSIZEREL "${CMAKE_C_FLAGS_RELEASE} -s")
set(CMAKE_CXX_FLAGS_MINSIZEREL "${CMAKE_CXX_FLAGS_RELEASE} -s")

set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -s")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -s")

add_library(mcomp mcomp.h mcomp.c)

add_executable(test_mcomp test/main.c test/entity.c test/named.c test/owned.c)
target_link_libraries(test_mcomp mcomp)
