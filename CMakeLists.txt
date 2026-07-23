cmake_minimum_required(VERSION 3.20)
project(mini_exchange LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_library(exchange_core
    src/order_book.cpp
    src/matching_engine.cpp
    src/parser.cpp
    src/reporter.cpp
)
target_include_directories(exchange_core PUBLIC include)
target_compile_options(exchange_core PRIVATE
    -Wall -Wextra -Wpedantic
    $<$<CONFIG:Release>:-O2>
)

add_executable(exchange_app src/main.cpp)
target_link_libraries(exchange_app PRIVATE exchange_core)

include(FetchContent)
FetchContent_Declare(
    googletest
    URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip
)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

enable_testing()

add_executable(exchange_tests
    tests/test_order_book.cpp
    tests/test_matching.cpp
    tests/test_fifo.cpp
    tests/test_cancel.cpp
    tests/test_market_order.cpp
    tests/test_replay.cpp
)
target_link_libraries(exchange_tests PRIVATE exchange_core GTest::gtest_main)

include(GoogleTest)
gtest_discover_tests(exchange_tests)
