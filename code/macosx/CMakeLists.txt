# SOURCE AND HEADER FILES
# add all src files and create file hierarchy for IDE
file(GLOB_RECURSE SOURCES *.cpp *.c *.m *.mm *.hpp *.h)
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

# HEADER
include_directories(${OPENGL_INCLUDE_DIR})
include_directories(${CMAKE_SOURCE_DIR}/code/renderer)
include_directories(${CMAKE_SOURCE_DIR}/code/macosx)
include_directories(${CMAKE_SOURCE_DIR}/code/qcommon)
include_directories(${CMAKE_SOURCE_DIR}/code/client)

# EXECUTABLE
add_executable(${APP_MACOS} ${SOURCES})

# LIBS
if(WIN32)

elseif(APPLE)
    target_link_libraries(${APP_MACOS} OpenGL)
    target_link_libraries(${APP_MACOS} "-framework AppKit")
    target_link_libraries(${APP_MACOS} "-framework IOKit")

else(UNIX)

endif()

# DEPENDENCIES
add_dependencies(${APP_MACOS} ${LIB_RENDERER} )