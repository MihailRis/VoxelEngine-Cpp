# SConstruct file for VoxelEngine
# Файл SConstruct для VoxelEngine

import os
import platform
import sys
import subprocess

# Find Visual Studio installation and return SCons environment
# Поиск установленной Visual Studio и возврат окружения SCons
def find_visual_studio():
    try:
        # Create environment with MSVC toolchain
        # Создание окружения с инструментами MSVC
        env = Environment(tools=['default', 'msvc'])
        print(f"Found Visual Studio version: {env['MSVC_VERSION']}")
        return env
    except Exception as e:
        print("Error: Could not find Visual Studio installation.")
        print("Make sure Visual Studio or Build Tools are installed.")
        Exit(1)

# Initialize SCons environment with VS detection
# Инициализация окружения SCons с определением Visual Studio
if platform.system() == "Windows":
    env = find_visual_studio()
else:
    print("This SConstruct is configured for Windows builds only")
    Exit(1)

# Configure vcpkg paths - vcpkg is a C++ package manager
# Настройка путей vcpkg - менеджера пакетов для C++
vcpkg_root = os.path.join(os.getcwd(), 'vcpkg')
vcpkg_installed = os.path.join(os.getcwd(), 'vcpkg_installed', 'x64-windows')

# Custom command to initialize/update vcpkg
# Пользовательская команда для инициализации/обновления vcpkg
def setup_vcpkg():
    # Clone vcpkg if not exists
    # Клонирование vcpkg если отсутствует
    if not os.path.exists(vcpkg_root):
        print("Initializing vcpkg...")
        try:
            # Add vcpkg as git submodule
            # Добавление vcpkg как подмодуля git
            subprocess.run(['git', 'submodule', 'add', '-f', 'https://github.com/microsoft/vcpkg.git', 'vcpkg'], 
                         check=True, capture_output=True, text=True)
            subprocess.run(['git', 'submodule', 'update', '--init', '--recursive'], 
                         check=True, capture_output=True, text=True)
        except subprocess.CalledProcessError as e:
            print("Git operation failed:")
            print(e.stdout)
            print(e.stderr)
            Exit(1)
        
    # Bootstrap vcpkg if not already bootstrapped
    # Инициализация vcpkg если еще не инициализирован
    if not os.path.exists(os.path.join(vcpkg_root, 'vcpkg.exe')):
        print("Bootstrapping vcpkg...")
        try:
            bootstrap_cmd = os.path.join(vcpkg_root, 'bootstrap-vcpkg.bat')
            result = subprocess.run([bootstrap_cmd], capture_output=True, text=True)
            print("Bootstrap output:")
            print(result.stdout)
            if result.returncode != 0:
                print("Bootstrap stderr:")
                print(result.stderr)
                Exit(1)
        except subprocess.CalledProcessError as e:
            print("Vcpkg bootstrap failed:")
            print(e.stdout)
            print(e.stderr)
            Exit(1)
    
    # Install all dependencies specified in vcpkg.json
    # Установка всех зависимостей, указанных в vcpkg.json
    print("Installing dependencies via vcpkg...")
    try:
        vcpkg_cmd = os.path.join(vcpkg_root, 'vcpkg')
        print("Running:", f'{vcpkg_cmd} install --triplet x64-windows')
        result = subprocess.run(
            [vcpkg_cmd, 'install', '--triplet', 'x64-windows'],
            capture_output=True,
            text=True
        )
        print("Vcpkg output:")
        print(result.stdout)
        if result.stderr:
            print("Vcpkg stderr:")
            print(result.stderr)
        if result.returncode != 0:
            print("Vcpkg install failed with return code:", result.returncode)
            Exit(1)
        print("Successfully installed all dependencies")
        
        # Check installed files
        # Проверка установленных файлов
        print("\nChecking installed files:")
        include_dir = os.path.join(vcpkg_installed, 'include')
        print(f"Looking in {include_dir}")
        if os.path.exists(include_dir):
            files = os.listdir(include_dir)
            print(f"Found files/directories: {', '.join(files)}")
        else:
            print("Include directory not found!")
            
    except subprocess.CalledProcessError as e:
        print("Vcpkg install failed:")
        print(e.stdout)
        print(e.stderr)
        Exit(1)

# Run vcpkg setup before proceeding
# Запуск настройки vcpkg перед продолжением
setup_vcpkg()

# Verify GLM header as indicator of successful dependency installation
# Проверка наличия заголовка GLM как индикатора успешной установки зависимостей
if not os.path.exists(os.path.join(vcpkg_installed, 'include', 'glm', 'glm.hpp')):
    print("Error: Dependencies installation failed. Check vcpkg output for details.")
    Exit(1)

# Set C++17 standard
# Установка стандарта C++17
env.Append(CXXFLAGS='/std:c++17')

# Windows-specific compile flags (/EHsc - exception handling, /W4 - warning level, /O2 - optimization)
# Флаги компиляции для Windows (/EHsc - обработка исключений, /W4 - уровень предупреждений, /O2 - оптимизация)
env.Append(CCFLAGS=['/EHsc', '/W4', '/O2'])
env.Append(CPPDEFINES=['WIN32', '_WINDOWS'])

# Debug/Release configuration with appropriate flags
# Настройка конфигураций Debug/Release с соответствующими флагами
if GetOption('debug'):
    # Debug flags: /MTd - debug runtime, /Zi - debug info, /Od - disable optimization
    # Флаги отладки: /MTd - отладочная версия runtime, /Zi - отладочная информация, /Od - отключение оптимизации
    env.Append(CCFLAGS=['/MTd', '/Zi', '/Od'])
    env.Append(CPPDEFINES=['_DEBUG'])
    build_dir = 'build/debug'
else:
    # Release flags: /MT - release runtime
    # Флаги релиза: /MT - релизная версия runtime
    env.Append(CCFLAGS=['/MT'])
    env.Append(CPPDEFINES=['NDEBUG'])
    build_dir = 'build/release'

# Add include paths for all dependencies
# Добавление путей для заголовочных файлов всех зависимостей
env.Append(CPPPATH=[
    'src',
    os.path.join(vcpkg_installed, 'include'),
    os.path.join(vcpkg_installed, 'include', 'luajit'),
])

# Add library paths for linking
# Добавление путей для библиотек при линковке
env.Append(LIBPATH=[
    os.path.join(vcpkg_installed, 'lib'),
])

# Required libraries for linking
# Необходимые библиотеки для линковки
libs = [
    'opengl32',
    'glew32',
    'glfw3dll',
    'lua51',
    'libpng16',
    'zlib',
    'OpenAL32',
    'vorbis',
    'vorbisfile',
    'winmm',
    'shell32',
    'libcurl',
]

env.Append(LIBS=libs)

# Source files configuration
# Настройка исходных файлов
src_dir = 'src'
src_files = []

# Recursive function to get all source files from directories
# Рекурсивная функция для получения всех исходных файлов из директорий
def get_source_files(dir):
    files = []
    for f in os.listdir(dir):
        path = os.path.join(dir, f)
        if os.path.isfile(path) and (path.endswith('.cpp') or path.endswith('.c')):
            files.append(path)
        elif os.path.isdir(path):
            files.extend(get_source_files(path))
    return files

src_files = get_source_files(src_dir)

# Separate main file from library sources for proper linking
# Отделение главного файла от исходников библиотеки для правильной линковки
main_file = os.path.join('src', 'voxel_engine.cpp')
if main_file in src_files:
    src_files.remove(main_file)

# Set up build directory without source duplication
# Настройка директории сборки без дублирования исходников
VariantDir(build_dir, 'src', duplicate=0)

# Adjust source paths for build directory
# Корректировка путей исходников для директории сборки
def adjust_build_path(files):
    return [f.replace('src/', build_dir + '/') for f in files]

src_files = adjust_build_path(src_files)
main_file = adjust_build_path([main_file])[0]

# Build static library from engine sources
# Сборка статической библиотеки из исходников движка
lib = env.StaticLibrary(os.path.join(build_dir, 'VoxelEngineSrc'), src_files)

# Build final executable linking with the library
# Сборка финального исполняемого файла с линковкой библиотеки
exe = env.Program(os.path.join(build_dir, 'VoxelEngine'), [main_file, lib])

# Set default build targets
# Установка целей сборки по умолчанию
Default(exe)

res_dir = 'res'
target_res_dir = os.path.join(build_dir, 'res')


def copy_resources_and_dlls(target, source, env):
    import shutil
    
    if os.path.exists(target_res_dir):
        try:
            shutil.rmtree(target_res_dir)
        except Exception as e:
            print(f"Warning: Failed to remove old resources directory: {e}")
            import time
            time.sleep(1)
            try:
                shutil.rmtree(target_res_dir)
            except Exception as e:
                print(f"Error: Could not remove resources directory even after retry: {e}")
                return 1

    try:
        print(f"Copying resources to {target_res_dir}")
        shutil.copytree(res_dir, target_res_dir)
    except Exception as e:
        print(f"Error copying resources: {e}")
        return 1

    vcpkg_bin_dir = os.path.join(vcpkg_installed, 'bin')
    print(f"Copying DLLs from {vcpkg_bin_dir}")
    
    dll_files = [
        'glew32.dll',
        'glfw3.dll',
        'lua51.dll',
        'libpng16.dll',
        'zlib1.dll',
        'OpenAL32.dll',
        'ogg.dll',
        'vorbis.dll',
        'vorbisfile.dll',
        'libcurl.dll'
    ]
    
    for dll in dll_files:
        src_path = os.path.join(vcpkg_bin_dir, dll)
        dst_path = os.path.join(build_dir, dll)
        if os.path.exists(src_path):
            try:
                print(f"Copying {dll}")
                shutil.copy2(src_path, dst_path)
            except Exception as e:
                print(f"Error copying {dll}: {e}")
        else:
            print(f"Warning: {dll} not found in {vcpkg_bin_dir}")
            
    return None

copy_cmd = env.Command('copy_resources_and_dlls', [], copy_resources_and_dlls)
env.Depends(copy_cmd, exe)
Default(copy_cmd)