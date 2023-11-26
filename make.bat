@echo off

:: Разбор флагов запуска.
set BUILD=1
set BUILD_MODE=Release

:parsing
if not "%1" == "" (
  if "%1" == "install" (
    set INSTALL_MODE=1
    shift
  ) else if "%1" == "mode" (
    set BUILD_MODE=%2
    shift
    shift
  )
  goto parsing
)

:: Запуск.
if "%INSTALL_MODE%" == "1" (
  goto install
)
goto build

:: Сборка.
:build
if not exist build (
  mkdir build
  echo Be sure "run install" have beed runned.
)
cd build
cmake -DCMAKE_BUILD_TYPE=%BUILD_MODE% ../
if NOT "%errorlevel%" == "0" goto end
cd ..
cmake --build ./build
if NOT "%errorlevel%" == "0" goto end
build/VoxelEngine
goto end

:install
cmake --preset=default
goto end

:end