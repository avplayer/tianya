cmake_minimum_required(VERSION 3.0)

project(tianya)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(ENABLE_QT_STATIC "link to static qt" ON)

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

if(MSVC)
set (CMAKE_CONFIGURATION_TYPES "Release;Debug")
set(CompilerFlags
        CMAKE_CXX_FLAGS
        CMAKE_CXX_FLAGS_DEBUG
        CMAKE_CXX_FLAGS_RELEASE
		CMAKE_CXX_FLAGS_MinSizeRel
        CMAKE_C_FLAGS
        CMAKE_C_FLAGS_DEBUG
        CMAKE_C_FLAGS_RELEASE
		CMAKE_C_FLAGS_MinSizeRel
   )

foreach(CompilerFlag ${CompilerFlags})
  string(REPLACE "/MD" "/MT" ${CompilerFlag} "${${CompilerFlag}}")
endforeach()

foreach(CompilerFlag ${CompilerFlags})
  string(REPLACE "/W3" "/W1" ${CompilerFlag} "${${CompilerFlag}}")
endforeach()

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
find_package(Qt5Widgets REQUIRED)
find_package(Boost REQUIRED COMPONENTS regex thread system locale context coroutine date_time)

include_directories(${OPENSSL_INCLUDE_DIR})
link_libraries(${OPENSSL_LIBRARIES})
link_directories(${Boost_LIBRARY_DIR})
include_directories(${Boost_INCLUDE_DIR})

if(WIN32 AND MSVC AND ENABLE_QT_STATIC)
	# For static Windows builds, we also need to pull in the Qt5 Platform
	# support library, which is not exposed to CMake properly, unfortunately
	GET_TARGET_PROPERTY(QT_LIB_DIR "${Qt5Widgets_LIBRARIES}" LOCATION)
	GET_FILENAME_COMPONENT(QT_LIB_DIR "${QT_LIB_DIR}" PATH)
	FIND_LIBRARY(Qt5Platform_LIBRARIES_RELEASE Qt5PlatformSupport
		HINTS "${QT_LIB_DIR}"
	)
	FIND_LIBRARY(Qt5Platform_LIBRARIES_DEBUG Qt5PlatformSupportd
		HINTS "${QT_LIB_DIR}"
	)
	FIND_LIBRARY(Qt5QWindows_LIBRARIES_RELEASE qwindows
		HINTS "${QT_LIB_DIR}/../plugins/platforms"
	)
	FIND_LIBRARY(Qt5QWindows_LIBRARIES_DEBUG qwindowsd
		HINTS "${QT_LIB_DIR}/../plugins/platforms"
	)
	FIND_LIBRARY(Qt5QICO_LIBRARIES_RELEASE qico
		HINTS "${QT_LIB_DIR}/../plugins/imageformats"
	)
	FIND_LIBRARY(Qt5QICO_LIBRARIES_DEBUG qicod
		HINTS "${QT_LIB_DIR}/../plugins/imageformats"
	)

	FIND_LIBRARY(Qt5HB_LIBRARIES_RELEASE qtharfbuzzng
		HINTS "${QT_LIB_DIR}"
	)
	FIND_LIBRARY(Qt5HB_LIBRARIES_DEBUG qtharfbuzzngd
		HINTS "${QT_LIB_DIR}"
	)

	FIND_LIBRARY(Qt5FT_LIBRARIES_RELEASE qtfreetype
		HINTS "${QT_LIB_DIR}"
	)
	FIND_LIBRARY(Qt5FT_LIBRARIES_DEBUG qtfreetyped
		HINTS "${QT_LIB_DIR}"
	)

	FIND_LIBRARY(Qt5PCRE_LIBRARIES_RELEASE qtpcre
		HINTS "${QT_LIB_DIR}"
	)
	FIND_LIBRARY(Qt5PCRE_LIBRARIES_DEBUG qtpcred
		HINTS "${QT_LIB_DIR}"
	)

	SET(Qt5Platform_LIBRARIES optimized ${Qt5Platform_LIBRARIES_RELEASE} debug ${Qt5Platform_LIBRARIES_DEBUG})
	SET(Qt5QWindows_LIBRARIES optimized ${Qt5QWindows_LIBRARIES_RELEASE} debug ${Qt5QWindows_LIBRARIES_DEBUG})
	SET(Qt5QICO_LIBRARIES optimized ${Qt5QICO_LIBRARIES_RELEASE} debug ${Qt5QICO_LIBRARIES_DEBUG})
	if (Qt5HB_LIBRARIES_RELEASE)
		SET(Qt5HB_LIBRARIES optimized ${Qt5HB_LIBRARIES_RELEASE} debug ${Qt5HB_LIBRARIES_DEBUG})
	endif()
	if (Qt5FT_LIBRARIES_RELEASE)
		SET(Qt5FT_LIBRARIES optimized ${Qt5FT_LIBRARIES_RELEASE} debug ${Qt5FT_LIBRARIES_DEBUG})
	endif()
	if (Qt5PCRE_LIBRARIES_RELEASE)
		SET(Qt5PCRE_LIBRARIES optimized ${Qt5PCRE_LIBRARIES_RELEASE} debug ${Qt5PCRE_LIBRARIES_DEBUG})
	endif()
	ADD_DEFINITIONS(-DSTATIC_QT5)
	set(Qt5StaticExtra ${Qt5Platform_LIBRARIES} ${Qt5QWindows_LIBRARIES} ${Qt5QICO_LIBRARIES} ${Qt5HB_LIBRARIES} ${Qt5FT_LIBRARIES} ${Qt5PCRE_LIBRARIES})
	link_libraries(${Qt5Platform_LIBRARIES} ${Qt5QWindows_LIBRARIES} ${Qt5QICO_LIBRARIES} ${Qt5HB_LIBRARIES} ${Qt5FT_LIBRARIES} ${Qt5PCRE_LIBRARIES})
endif()

add_subdirectory(libtianya)

include_directories(libtianya/include)
include_directories(avboost/include)

link_directories(${QT_LIB_DIR})
add_subdirectory(src)

