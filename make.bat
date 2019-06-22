@ECHO OFF
if "%~1"=="" goto BLANK
if "%~1"=="clean" goto CLEAN
@ECHO ON

:BLANK
cmake -H. -B_project -G  "Visual Studio 16 2019"
GOTO DONE

:CLEAN
rmdir /Q /S _project
rmdir /Q /S bin
GOTO DONE

:DONE