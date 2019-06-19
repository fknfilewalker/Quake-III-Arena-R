Quake III Arena R
==================================

##### Branches:
`original` : A refactor of the original Quake III source code, to make it compatible with modern computers.

GENERAL NOTES
=============
##### Changes:
* `VMI_COMPILED` was removed

##### Requirements:
* [CMake](https://cmake.org/ "CMake") >= 3.14.0

##### Submodules:
Please clone this git with submodules
- [TinyJPEG](https://github.com/serge-rgb/TinyJPEG "TinyJPEG")
- [stb](https://github.com/nothings/stb.git "stb")

COMPILING ON WIN
==================

Use the provided `bat` file to generate a Visual Studio project. In order to run the game, make sure to set `fs_basePath` to a directory containing all the assets. Binarys can be found in `bin/`.

##### Bugs:

COMPILING ON GNU/LINUX
==================

Use the provided `makefile` to build Quake III. Binarys can be found in `bin/`. In order to run the game, make sure to set `fs_basePath` to a directory containing all the assets or put them in `~/.q3a/baseq3`. 

##### Requirements:
* `libglu-dev`
* `libxxf86dga-dev`

##### Bugs:
* no audio on some distros because of OSS no longer supported (OSS needs to be ported to ALSA)

COMPILING ON MAC
================
Use the provided `makefile` to generate a Xcode project. Open `Quake_III_Arena.xcodeproj` from `_project/` and build the `quake3` target. In order to run the game, make sure to set `fs_basePath` to a directory containing all the assets or put them in `~/Library/Application Support/baseq3`. Binarys can be found in `bin/`.

##### Bugs: