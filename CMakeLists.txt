cmake_minimum_required(VERSION 3.30)
set(CMAKE_CXX_STANDARD 20)

project(tianya)

SET(CPACK_PACKAGE_VERSION_MAJOR "0")
SET(CPACK_PACKAGE_VERSION_MINOR "1")
SET(CPACK_PACKAGE_VERSION_PATCH "0")

if(MSVC)
	set(Boost_USE_STATIC_LIBS        ON)
	set(Boost_USE_STATIC_RUNTIME    ON)
endif()

if(WIN32)
	add_definitions(-DBOOST_ALL_STATIC_LINK)
	add_definitions(-DBOOST_THREAD_USE_LIB)
	add_definitions(-DBOOST_FILESYSTEM_STATIC_LINK)
	add_definitions(-DBOOST_ALL_STATIC_LINK)
	add_definitions(-DWIN32_LEAN_AND_MEAN)
	add_definitions(-DNOMINMAX)
	add_definitions(-D_CRT_SECURE_NO_WARNINGS)
	add_definitions(-D_WIN32_WINNT=0x0501)
endif()

set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

if(MSVC)


set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /bigobj /MP")

#set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MT")
set(CMAKE_EXE_LINKER_FLAGS_DEBUG "${CMAKE_EXE_LINKER_FLAGS_DEBUG}  /ignore:4099 /NODEFAULTLIB:libcmt.lib ")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER}  /SAFESEH:NO ")

endif(MSVC)

if(WIN32)
add_definitions(-DOPENSSL_NO_ENGINE)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DOPENSSL_NO_ENGINE -DOPENSSL_NO_COMP")
include_directories(third_party/openssl/include)
add_subdirectory(third_party/openssl)
else()
find_package(OpenSSL REQUIRED)
endif()

find_package(Threads)
find_package(Qt6 REQUIRED COMPONENTS Widgets Network Core)
find_package(Boost CONFIG REQUIRED COMPONENTS regex thread system locale context coroutine date_time)

qt_standard_project_setup()

include_directories(${OPENSSL_INCLUDE_DIR})
link_libraries(${OPENSSL_LIBRARIES})
link_directories(${Boost_LIBRARY_DIR})
include_directories(${Boost_INCLUDE_DIR})

add_subdirectory(libtianya)

include_directories(libtianya/include)
include_directories(avboost/include)

link_directories(${QT_LIB_DIR})
add_subdirectory(src)

