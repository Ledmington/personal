cmake_minimum_required(VERSION 3.14)
project(uncertain-value-arithmetic LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include_directories(${PROJECT_SOURCE_DIR}/include)

add_library(uncertain_value
    src/uncertain_value.cpp
)
target_include_directories(uncertain_value PUBLIC
    ${PROJECT_SOURCE_DIR}/include
)

add_executable(example
    exec/main.cpp
)
target_link_libraries(example PRIVATE uncertain_value)

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
    test/test_uncertain_value.cpp
)
target_link_libraries(uv_tests
    PRIVATE uncertain_value
    PRIVATE gtest_main
)

include(GoogleTest)
gtest_discover_tests(uv_tests)
