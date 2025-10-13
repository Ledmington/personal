cmake_minimum_required(VERSION 3.14)

project(uncertain-value-arithmetic LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(UV_WERROR "Treat warnings as errors" ON)
option(UV_TEST "Enable building the test suite" ON)

if(NOT CMAKE_BUILD_TYPE)
    message(STATUS "CMAKE_BUILD_TYPE not set, setting to Debug")
    set(CMAKE_BUILD_TYPE Debug)
endif()

if(NOT UV_FLAGS)
    set(UV_FLAGS -Wall -Wextra -pedantic -Wshadow -Wsign-conversion -Wfloat-conversion -Wfloat-equal)
else()
    string(REPLACE " " ";" UV_FLAGS "${UV_FLAGS}")
endif()

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    list(APPEND UV_FLAGS -O0 -g -fno-omit-frame-pointer -fno-optimize-sibling-calls)
elseif(CMAKE_BUILD_TYPE STREQUAL "Release")
    list(APPEND UV_FLAGS -O3 -DNDEBUG -march=native -mtune=native)
endif()

if(UV_WERROR)
    list(APPEND UV_FLAGS -Werror)
endif()

include_directories(${PROJECT_SOURCE_DIR}/include)

add_library(uncertain_value
    src/nominal_uncertain_value.cpp
)
target_include_directories(uncertain_value PUBLIC
    ${PROJECT_SOURCE_DIR}/include
)
target_compile_options(uncertain_value PUBLIC ${UV_FLAGS})

add_executable(example
    exec/main.cpp
)
target_link_libraries(example PRIVATE uncertain_value)

if(UV_TEST)
    include(CTest)
    enable_testing()

    include(FetchContent)
    FetchContent_Declare(
        googletest
        URL https://github.com/google/googletest/archive/refs/heads/main.zip
    )
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(googletest)

    add_executable(uv_tests
        test/test_nominal_uncertain_value.cpp
    )
    target_link_libraries(uv_tests
        PRIVATE uncertain_value
        PRIVATE gtest_main
    )

    include(GoogleTest)
    gtest_discover_tests(uv_tests)
endif()

# Convert UV_FLAGS back to string before printing
string(REPLACE ";" " " UV_FLAGS "${UV_FLAGS}")

message(STATUS)
message(STATUS "Project uncertain_value_arithmetic successfully configured!")
message(STATUS "CMAKE_BUILD_TYPE : ${CMAKE_BUILD_TYPE}")
message(STATUS "UV_FLAGS         : ${UV_FLAGS}")
message(STATUS "UV_WERROR        : ${UV_WERROR}")
message(STATUS "UV_TEST          : ${UV_TEST}")
message(STATUS)
