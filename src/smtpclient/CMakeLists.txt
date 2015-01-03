
file(GLOB SRCS src/*.cpp include/*.h*)

add_library(mailmine STATIC ${SRCS})

target_include_directories(mailmine PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

target_link_libraries(mailmine Qt5::Widgets)
