cmake_minimum_required(VERSION 3.14)
project(Netsim)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

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
		src/helpers.cpp
		src/nodes.cpp
		src/factory.cpp
		src/reports.cpp
		src/simulation.cpp
)

target_include_directories(netsim_lib
        PUBLIC include
)

add_executable(${PROJECT_NAME} main.cpp
		src/factory.cpp)
add_executable(tests test/tests.cpp)



target_link_libraries(${PROJECT_NAME}
        netsim_lib
	GTest::gtest_main
)

target_link_libraries(
	tests
	GTest::gtest_main
	netsim_lib
)
enable_testing()

include(GoogleTest)
add_test(NAME NetsimTests COMMAND tests)
