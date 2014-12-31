
add_executable(tianya WIN32 main.cpp)

target_link_libraries(tianya Qt5::Widgets ${Boost_LIBRARIES})
