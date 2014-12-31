cmake_minimum_required(VERSION 3.0)

project(tianya)

include(CheckCXXCompilerFlag)
check_cxx_compiler_flag(-std=c++11 HAS_CXX11)
if (HAS_CXX11)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 ")
endif()


find_package(Qt5Widgets REQUIRED)
find_package(Boost REQUIRED COMPONENTS regex thread system locale)

include_directories(libtianya/include)

add_subdirectory(src)

