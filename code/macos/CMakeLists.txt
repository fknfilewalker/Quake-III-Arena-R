# SOURCE AND HEADER FILES
# add all src files and create file hierarchy for IDE
file(GLOB_RECURSE SOURCES *.cpp *.c *.m *.mm *.hpp *.h ../qcommon/*.cpp ../qcommon/*.c ../qcommon/*.m ../qcommon/*.mm ../qcommon/*.hpp ../qcommon/*.h ../client/*.cpp ../client/*.c ../client/*.m ../client/*.mm ../client/*.hpp ../client/*.h ../server/*.cpp ../server/*.c ../server/*.m ../server/*.mm ../server/*.hpp ../server/*.h)


list(APPEND SOURCES "../unix/unix_shared.c")
list(APPEND SOURCES "../unix/unix_net.c")
list(APPEND SOURCES "../game/q_math.c")
list(APPEND SOURCES "../game/q_shared.c")

foreach(FILE ${SOURCES}) 
    if("${FILE}" MATCHES "macosx_glsmp_mutex.m" OR "${FILE}" MATCHES "macosx_glsmp_null.m" OR "${FILE}" MATCHES "macosx_snddma.m")
        # "${FILE}" MATCHES "macosx_snddma.m" OR "${FILE}" MATCHES "macosx_sndcore.m" OR 
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
include_directories(${OPENGL_INCLUDE_DIR})
include_directories(${CMAKE_SOURCE_DIR}/code/renderer)
include_directories(${CMAKE_SOURCE_DIR}/code/macos)
include_directories(${CMAKE_SOURCE_DIR}/code/qcommon)
include_directories(${CMAKE_SOURCE_DIR}/code/client)
include_directories(${CMAKE_SOURCE_DIR}/code/server)

# EXECUTABLE
set(CMAKE_XCODE_GENERATE_SCHEME TRUE)
add_executable(${APP_MACOS} MACOSX_BUNDLE ${SOURCES} Quake3.icns)
# +set fs_basePath \"Volumes/data_mac/Games/GOG Games/Quake III\"
FIND_LIBRARY(OpenGL_LIBRARY OpenGL)
target_link_libraries(${APP_MACOS} ${OpenGL_LIBRARY})
target_link_libraries(${APP_MACOS} "-framework AppKit")
target_link_libraries(${APP_MACOS} "-framework IOKit")
target_link_libraries(${APP_MACOS} "-framework CoreAudio")
target_link_libraries(${APP_MACOS} ${LIB_BOTLIB})
target_link_libraries(${APP_MACOS} ${LIB_RENDERER})
target_link_libraries(${APP_MACOS} ${LIB_SPLINES})

# PROPERTIES
set_target_properties(${APP_MACOS} PROPERTIES MACOSX_BUNDLE_INFO_PLIST ${CMAKE_SOURCE_DIR}/code/macos/plist.in)
set_source_files_properties(Quake3.icns PROPERTIES MACOSX_PACKAGE_LOCATION Resources)
IF(Q3_ARGUMENTS)
    set_target_properties(${APP_MACOS} PROPERTIES XCODE_SCHEME_ARGUMENTS ${Q3_ARGUMENTS})
endif()
IF(BASEQ3_DIR)
    set_target_properties(${APP_MACOS} PROPERTIES XCODE_SCHEME_ARGUMENTS ${BASEQ3_DIR})
endif()
# still waiting for support :(
set_property(DIRECTORY ${CMAKE_SOURCE_DIR} PROPERTY XCODE_STARTUP_PROJECT ${APP_MACOS})

# DEPENDENCIES
add_dependencies(${APP_MACOS} ${LIB_RENDERER} ${LIB_BOTLIB} ${LIB_SPLINES} ${LIB_GAME} ${LIB_CGAME} ${LIB_Q3UI})