set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)

file(GLOB SRCS src/*.cpp include/*.h*)

add_library(mailmime STATIC ${SRCS})

target_include_directories(mailmime PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

target_link_libraries(mailmime PUBLIC Qt6::Widgets)
