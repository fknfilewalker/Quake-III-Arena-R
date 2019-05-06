# cmake -H. -B_project -G Xcode
# cmake -H. -B_project -G  "Visual Studio 15 2017 Win64"

cmake_minimum_required(VERSION 3.14.0)
project(Quake_III_Arena)

set(APP_MACOS quake3)
set(APP_WIN32 quake3)
set(APP_UNIX quake3)
set(LIB_RENDERER renderer)
set(LIB_GAME qagame)
set(LIB_CGAME cgame)
set(LIB_BOTLIB botlib)
set(LIB_SPLINES Splines)
set(LIB_UI ui_new)
set(LIB_Q3UI ui)

# Arguments for IDE
set(Q3_ARGUMENTS "+set sv_pure 0 +set vm_game 0 +set vm_cgame 0 +set vm_ui 0")
# Set path to baseq3 folder
if(WIN32)
	set(BASEQ3_DIR "+set fs_basePath \"C:\\GOG Games\\Quake III\"")
elseif(APPLE)
	#set(BASEQ3_DIR "+set fs_basePath \\\"/dir/to/Quake III\\\"")
else(UNIX)
	#quake III files in home/.q3a
endif()
# disable ZERO_CHECK target
set(CMAKE_SUPPRESS_REGENERATION true)

if(WIN32)
	IF(MSVC)
		message(STATUS "Setting MSVC flags")
		set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /EHc /std:clatest")
	ENDIF(MSVC)
	
	#add_definitions(-DVK_USE_PLATFORM_WIN32_KHR)
elseif(APPLE)
	find_package(OpenGL REQUIRED)
	if (OpenGL_FOUND)
		message(STATUS "Found OpenGL")
	endif (OpenGL_FOUND)

	#add_definitions(-DVK_USE_PLATFORM_MACOS_MVK)
	#set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
	set(CMAKE_XCODE_GENERATE_SCHEME TRUE)
else(UNIX)
	find_package(OpenGL REQUIRED)
	if (OpenGL_FOUND)
		message(STATUS "Found OpenGL")
	endif (OpenGL_FOUND)

	find_package(X11 REQUIRED)
	if (X11_FOUND)
		message(STATUS "Found X11")
	endif (X11_FOUND)
	#find_package(Threads REQUIRED)
	#add_definitions(-DVK_USE_PLATFORM_XCB_KHR)
	#set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
endif()

# OpenGL
find_package(OpenGL REQUIRED)

# cmake dirs
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/bin")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/bin")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/bin")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/Debug" )

add_subdirectory(code)


