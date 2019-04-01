
add_subdirectory(q3_ui)
add_subdirectory(splines)


# NO DEP
add_subdirectory(botlib)
add_subdirectory(renderer)

# DEP
add_subdirectory(cgame)
add_subdirectory(game)

#add_subdirectory(macosx)
add_subdirectory(win32)