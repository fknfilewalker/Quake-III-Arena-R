# SOURCE AND HEADER FILES
# add all src files and create file hierarchy for IDE
file(GLOB_RECURSE SOURCES *.cpp *.c *.m *.mm *.hpp *.h ../qcommon/*.cpp ../qcommon/*.c ../qcommon/*.m ../qcommon/*.mm ../qcommon/*.hpp ../qcommon/*.h ../client/*.cpp ../client/*.c ../client/*.m ../client/*.mm ../client/*.hpp ../client/*.h ../server/*.cpp ../server/*.c ../server/*.m ../server/*.mm ../server/*.hpp ../server/*.h)

list(APPEND SOURCES "../game/q_math.c")
list(APPEND SOURCES "../game/bg_misc.c")
list(APPEND SOURCES "../game/q_shared.c")
list(APPEND SOURCES "../game/q_shared.h")
list(APPEND SOURCES "../ui/ui_syscalls.c")

foreach(FILE ${SOURCES}) 
    # Get the directory of the source file
    get_filename_component(PARENT_DIR "${FILE}" DIRECTORY)

    # Remove common directory prefix to make the group
    string(REPLACE "${CMAKE_CURRENT_SOURCE_DIR}" "" GROUP "${PARENT_DIR}")

    # Make sure we are using windows slashes
    string(REPLACE "/" "\\" GROUP "${GROUP}")

    # Group into "Source Files" and "Header Files"
    if ("${FILE}" MATCHES ".*\\.\(cpp|c|m\)")
       set(GROUP "Source Files${GROUP}")
    elseif("${FILE}" MATCHES ".*\\.\(hpp|h\)")
       set(GROUP "Header Files${GROUP}")
    endif()

    source_group("${GROUP}" FILES "${FILE}")
endforeach()

# INCLUDE
include_directories(${OPENGL_INCLUDE_DIR})
include_directories(${VULKAN_INCLUDE_DIR})
include_directories(${CMAKE_SOURCE_DIR}/code/renderer)
include_directories(${CMAKE_SOURCE_DIR}/code/qcommon)
include_directories(${CMAKE_SOURCE_DIR}/code/client)
include_directories(${CMAKE_SOURCE_DIR}/code/botlib)

# EXECUTABLE
add_executable(${APP_WIN32} WIN32 ${SOURCES} "winquake.rc")
target_link_libraries(${APP_WIN32} wsock32 ws2_32 winmm)
target_link_libraries(${APP_WIN32} debug "${CMAKE_SOURCE_DIR}/bin/Debug/${LIB_BOTLIB}.lib" optimized "${CMAKE_SOURCE_DIR}/bin/Release/${LIB_BOTLIB}.lib")
target_link_libraries(${APP_WIN32} debug "${CMAKE_SOURCE_DIR}/bin/Debug/${LIB_RENDERER}.lib" optimized "${CMAKE_SOURCE_DIR}/bin/Release/${LIB_RENDERER}.lib")
target_link_libraries(${APP_WIN32} debug "${CMAKE_SOURCE_DIR}/bin/Debug/${LIB_SPLINES}.lib" optimized "${CMAKE_SOURCE_DIR}/bin/Release/${LIB_SPLINES}.lib")

# PROPERTIES
# when Quake III Arena was installed with GOG
# +set fs_basePath "C:\GOG Games\Quake III"
set_target_properties(${APP_WIN32} PROPERTIES LINKER_LANGUAGE C)
set(VS_ARGUMENTS "")
IF(BASEQ3_DIR)
    set(VS_ARGUMENTS "${BASEQ3_DIR}")
endif()
IF(Q3_VM_DYNAMIC)
    set(VS_ARGUMENTS "${VS_ARGUMENTS} +set sv_pure 0 +set vm_game 0 +set vm_cgame 0 +set vm_ui 0")
else()
    set(VS_ARGUMENTS "${VS_ARGUMENTS} +set sv_pure 1 +set vm_game 1 +set vm_cgame 1 +set vm_ui 1")
endif()
IF(Q3_ARGUMENTS)
    set(VS_ARGUMENTS "${VS_ARGUMENTS} ${Q3_ARGUMENTS}")
endif()

set_target_properties(${APP_WIN32} PROPERTIES VS_DEBUGGER_COMMAND_ARGUMENTS ${VS_ARGUMENTS})
set_property(DIRECTORY ${CMAKE_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT ${APP_WIN32})   

# DEPENDENCIES
add_dependencies(${APP_WIN32} ${LIB_RENDERER} ${LIB_BOTLIB} ${LIB_SPLINES} ${LIB_GAME} ${LIB_CGAME} ${LIB_Q3UI})