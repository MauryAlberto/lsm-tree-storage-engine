cmake_minimum_required(VERSION 3.20)
project(lsmtse CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug)
endif()

set(LSMTSE_WARNING_FLAGS -Wall -Wextra -Wpedantic -Wconversion -Wsign-conversion -Werror)

find_package(Threads REQUIRED)

add_library(lsmtse_core
    src/MemTable.cpp
)
target_include_directories(lsmtse_core PUBLIC include)
target_link_libraries(lsmtse_core PUBLIC Threads::Threads)
target_compile_options(lsmtse_core PRIVATE ${LSMTSE_WARNING_FLAGS})

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_compile_options(lsmtse_core PUBLIC
        -fsanitize=address,undefined
        -fno-omit-frame-pointer
        -D_GLIBCXX_ASSERTIONS
        -g
    )
    target_link_options(lsmtse_core PUBLIC
        -fsanitize=address,undefined
    )
endif()

add_executable(lsmtse src/main.cpp)
target_link_libraries(lsmtse PRIVATE lsmtse_core)
target_compile_options(lsmtse PRIVATE ${LSMTSE_WARNING_FLAGS})

enable_testing()

include(FetchContent)
FetchContent_Declare(
    Catch2
    GIT_REPOSITORY https://github.com/catchorg/Catch2.git
    GIT_TAG v3.7.1
    SYSTEM
)

FetchContent_Declare(
    json
    URL https://github.com/nlohmann/json/releases/download/v3.12.0/json.tar.xz
    SYSTEM
)

FetchContent_MakeAvailable(json Catch2)

add_executable(unit_tests
    tests/Test_MemTable.cpp
)

target_link_libraries(lsmtse PRIVATE nlohmann_json::nlohmann_json)

target_link_libraries(unit_tests PRIVATE 
    lsmtse_core Catch2::Catch2WithMain
    nlohmann_json::nlohmann_json 
)
target_compile_options(unit_tests PRIVATE ${LSMTSE_WARNING_FLAGS})

include(CTest)
include(Catch)
catch_discover_tests(unit_tests)