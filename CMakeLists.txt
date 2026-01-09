cmake_minimum_required(VERSION 3.14)
project(Netsim)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)
FetchContent_Declare(
        googletest
        URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip
)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

add_library(netsim_lib
        src/package.cpp
        src/storage_types.cpp
)

target_include_directories(netsim_lib
        PUBLIC include
)

add_executable(${PROJECT_NAME} main.cpp)
add_executable(tests test/tests.cpp)


target_link_libraries(tests
        netsim_lib
        gtest
)

enable_testing()
add_test(NAME NetsimTests COMMAND tests)
