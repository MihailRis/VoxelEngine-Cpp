@echo off

:: Разбор флагов запуска.
set BUILD=1
set BUILD_MODE=Release
set INSTALL_MODE="0"
set HELP="0"

:parsing
if not "%1" == "" (
  if "%1" == "install" (
    set INSTALL_MODE=1
  ) else if "%1" == "mode" (
    set BUILD_MODE=%2
    shift
  ) else if "%1" == "help" (
    set HELP=1
  ) else (
    echo Unexpected argument: "%1".
  )
  shift
  goto parsing
)

:: Запуск.
if "%INSTALL_MODE%" == "1" (
  goto install
) else if "%HELP%" == "1" (
  goto help
)
goto build

:: Команда помощи.
:help
echo Commands list:
echo   make install
echo         ^\ Initialize all required settings and download all dependencies.
echo   make
echo         ^\ Default building with "Release" mode.
echo   make mode Release
echo         ^\ Manually setting building mode.
echo   make help
echo         ^\ Get a list of commands.
goto end

:: Сборка.
:build
if not exist build (
  mkdir build
  echo Be sure "run install" have beed runned.
)
cd build
cmake -DCMAKE_BUILD_TYPE=%BUILD_MODE% ../
cd ..
cmake --build build
build/VoxelEngine
goto end

:install
set VCPKG_DEFAULT_TRIPLET="x64-windows"
vcpkg integrate install
cmake --preset=default
goto build

:end