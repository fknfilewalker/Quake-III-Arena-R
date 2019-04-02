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

# LIB
add_library(${LIB_BOTLIB} STATIC ${SOURCES})
set_target_properties(${LIB_BOTLIB} PROPERTIES LINKER_LANGUAGE C)

# PRECOMPILER DEFINITIONS
add_compile_definitions(BOTLIB) # see l_struct.c Ln 32 for example
