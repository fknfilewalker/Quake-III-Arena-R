# SOURCE AND HEADER FILES
# add all src files and create file hierarchy for IDE
file(GLOB_RECURSE SOURCES *.cpp *.c *.m *.mm *.hpp *.h)
list(APPEND SOURCES "../game/bg_pmove.c")
list(APPEND SOURCES "../game/bg_misc.c")
list(APPEND SOURCES "../game/bg_slidemove.c")
list(APPEND SOURCES "../game/q_shared.c")
list(APPEND SOURCES "../game/q_math.c")
foreach(FILE ${SOURCES})
    if("${FILE}" MATCHES "cg_newdraw.c" OR "${FILE}" MATCHES "cg_particles.c")
        #message( ${FILE} )
        list(REMOVE_ITEM SOURCES "${FILE}")
        continue()
    endif()

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
#include_directories(${CMAKE_SOURCE_DIR}/code/macosx)
#include_directories(${CMAKE_SOURCE_DIR}/code/game)
#include_directories(${CMAKE_SOURCE_DIR}/code/splines)

# LIB
add_library(${LIB_CGAME} SHARED ${SOURCES})

# DEPENDENCIES
add_dependencies(${LIB_CGAME} ${LIB_RENDERER} ${LIB_SPLINES})

set_target_properties(${LIB_CGAME} PROPERTIES LINKER_LANGUAGE C)

# LIBS
if(WIN32)

elseif(APPLE)
    target_link_libraries(${LIB_CGAME} "${CMAKE_SOURCE_DIR}/bin/Debug/libsplines.a")

else(UNIX)

endif()