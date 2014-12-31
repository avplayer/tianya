cmake_minimum_required(VERSION 3.0)

project(tianya)

include(CheckCXXCompilerFlag)
check_cxx_compiler_flag(-std=c++11 HAS_CXX11)
if (HAS_CXX11)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 ")
endif()

find_package(Threads REQUIRED)
find_package(Qt5Widgets REQUIRED)
find_package(Boost REQUIRED COMPONENTS regex thread system locale context coroutine)

include_directories(libtianya/include)

add_subdirectory(src)

