# cmake -H. -B_project -G Xcode
# cmake -H. -B_project -G  "Visual Studio 15 2017 Win64"

# !!XCODE!!
	# set following env variable
	# 1: VK_ICD_FILENAMES 	/Users/Luke/vulkansdk-macos-1.1.85.0/macOS/etc/vulkan/icd.d/MoltenVK_icd.json
	# 2: VK_LAYER_PATH 		/Users/luke/vulkansdk-macos-1.1.85.0/macOS/etc/vulkan/explicit_layer.d

cmake_minimum_required(VERSION 3.10.1)
project(Quake_III_Arena)

set(APP_MACOS macos)
set(LIB_RENDERER renderer)
set(LIB_GAME game)
set(LIB_CGAME cgame)
set(LIB_BOTLIB botlib)
set(LIB_SPLINES splines)
set(LIB_UI ui)
set(LIB_Q3UI q3ui)

# disable ZERO_CHECK target
set(CMAKE_SUPPRESS_REGENERATION true)

if(WIN32)
	IF(MSVC)
		message(STATUS "Setting MSVC flags")
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /EHc /std:c++latest")
	ENDIF(MSVC)
	
	add_definitions(-DVK_USE_PLATFORM_WIN32_KHR)
elseif(APPLE)
	find_package(OpenGL REQUIRED)
	if (OpenGL_FOUND)
		message(STATUS "Found OpenGL")
	endif (OpenGL_FOUND)

	#add_definitions(-DVK_USE_PLATFORM_MACOS_MVK)
	#set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
	set(CMAKE_XCODE_GENERATE_SCHEME TRUE)
else(UNIX)
	find_package(X11 REQUIRED)
	if (X11_FOUND)
		message(STATUS "Found X11")
	endif (X11_FOUND)
	find_package(Threads REQUIRED)
	#add_definitions(-DVK_USE_PLATFORM_XCB_KHR)
	set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
endif()


# cmake dirs
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/bin")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/bin")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/bin")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/Debug" )

add_subdirectory(code)


