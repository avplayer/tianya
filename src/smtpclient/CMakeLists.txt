
include_directories(${Boost_INCLUDE_DIRS})

add_library(smtpclient STATIC smtp.cpp smtp.hpp)

