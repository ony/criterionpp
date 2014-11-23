cmake_minimum_required(VERSION 2.8.7)

project(criterionpp CXX)

include(CheckCXXCompilerFlag)

# Enable the latest C++11 flag
check_cxx_compiler_flag(-std=c++11 HAVE_FLAG_CXX_11)
if(HAVE_FLAG_CXX_11)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
else()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x")
endif()

# Turn compiler warnings
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Werror")

add_library(${PROJECT_NAME} STATIC criterion.cpp)

add_executable(test_simple test_simple.cpp)
target_link_libraries(test_simple ${PROJECT_NAME})
