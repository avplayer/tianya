

file(GLOB SRCS src/*.cpp include/*.h*)


add_library(smtpclient STATIC ${SRCS})

target_include_directories(smtpclient PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

target_link_libraries(smtpclient Qt5::Widgets Qt5::Network)
