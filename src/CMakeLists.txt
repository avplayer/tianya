set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)

cmake_policy(SET CMP0020 NEW)
add_definitions(-DQT_NO_KEYWORDS)

file(GLOB SRCS *.cpp *.h*)
file(GLOB UIS *.ui)

qt5_wrap_ui(UI_SRCS ${UIS})
# qt5_add_resources(RC_SRCS tianya.qrc)

set(SRCS ${SRCS} ${UI_SRCS} ${RC_SRCS})

add_executable(tianya WIN32 ${SRCS})

target_link_libraries(tianya Qt5::Widgets)
if( NOT MSVC)
target_link_libraries(tianya  ${Boost_LIBRARIES})
endif()
target_link_libraries(tianya ${CMAKE_THREAD_LIBS_INIT})

if(WIN32 AND MSVC AND ENABLE_QT_STATIC)
	target_link_libraries(tianya ${Qt5StaticExtra})
endif()
