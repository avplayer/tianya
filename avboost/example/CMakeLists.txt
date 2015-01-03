cmake_minimum_required(VERSION 2.6)
find_package(Threads)

add_executable( test1 autoproxy.cpp)
add_executable( test2 multi_proxychain.cpp)

target_link_libraries( test1 ${Boost_LIBRARIES} ${OPENSSL_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT})
target_link_libraries( test2 ${Boost_LIBRARIES} ${OPENSSL_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT})