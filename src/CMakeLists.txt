set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)

cmake_policy(SET CMP0020 NEW)

add_subdirectory(mime/)
add_subdirectory(smtpclient/)

configure_file(tianya.rc.in ${CMAKE_CURRENT_BINARY_DIR}/tianya.rc)

file(GLOB SRCS model/*.cpp *.cpp *.h*)
file(GLOB UIS *.ui)

qt_add_executable(tianya ${UIS} ${SRCS} tianya.qrc dpi.manifest  ${CMAKE_CURRENT_BINARY_DIR}/tianya.rc)

target_include_directories(tianya PUBLIC ${Boost_INCLUDE_DIRS})

target_link_libraries(tianya  PUBLIC libtianya mailmime smtpclient)

target_link_libraries(tianya PUBLIC Qt6::Widgets)

set_target_properties(tianya PROPERTIES
    WIN32_EXECUTABLE ON
    MACOSX_BUNDLE ON
)

target_link_libraries(tianya PUBLIC  ${Boost_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT})

if(MSVC)
	set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /MANIFESTINPUT:${CMAKE_CURRENT_SOURCE_DIR}/dpi.manifest")
endif()

install(TARGETS tianya RUNTIME DESTINATION bin)

install(PROGRAMS tianya-radar.desktop DESTINATION share/applications)
install(FILES tianya.svg DESTINATION share/icons)

