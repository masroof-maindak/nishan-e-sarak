cmake_minimum_required(VERSION 3.10)
project(Nishan-e-Sarak LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Linker
find_program(MOLD_BIN mold)
find_program(LLD_BIN lld)
if(MOLD_BIN)
    set(CMAKE_LINKER_TYPE MOLD)
elseif(LLD_BIN)
    set(CMAKE_LINKER_TYPE LLD)
endif()

include(FetchContent)

FetchContent_Declare(
    argparse
    GIT_REPOSITORY https://github.com/p-ranav/argparse.git
)

FetchContent_Declare(
    KinaraDaryaft
    GIT_REPOSITORY https://github.com/masroof-maindak/kinara-daryaft.git
    GIT_TAG origin/main
)

FetchContent_MakeAvailable(argparse KinaraDaryaft)

find_package(OpenCV REQUIRED)

add_executable(nshn "src/main.cpp" "src/args.cpp" "src/io.cpp")
target_include_directories(nshn PRIVATE "include/" ${OpenCV_INCLUDE_DIRS})
target_link_libraries(nshn PRIVATE argparse KinaraDaryaft ${OpenCV_LIBS})
