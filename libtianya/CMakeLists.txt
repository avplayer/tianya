
find_package(Threads)
find_package(Boost REQUIRED COMPONENTS regex thread system locale context coroutine date_time)

include_directories(${OPENSSL_INCLUDE_DIR})
link_libraries(${OPENSSL_LIBRARIES})
link_directories(${Boost_LIBRARY_DIR})

include_directories(include)

add_library(libtianya STATIC src/tianya_list.cpp include/util.hpp include/tianya_context.hpp include/tianya_list.hpp)
