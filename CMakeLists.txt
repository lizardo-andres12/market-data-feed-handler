cmake_minimum_required(VERSION 3.14)
project(TradingEngine VERSION 1.0)
find_package(Threads REQUIRED)

# --- Project Settings ---
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON) 

add_compile_options(-march=native -Wall -pedantic-errors)


# --- GTest Setup ---
include(FetchContent)

FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip
  SOURCE_DIR ${CMAKE_SOURCE_DIR}/lib/googletest
  DOWNLOAD_EXTRACT_TIMESTAMP FALSE
)

# prevent gtest from overriding our compiler warning flags
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

FetchContent_MakeAvailable(googletest)


# --- Component Libraries ---

# orderbook lib
add_library(orderbook STATIC src/orderbook/orderbook.cpp)
target_include_directories(orderbook PUBLIC include)

# vwap_tracker lib
add_library(vwap_tracker STATIC src/vwap_tracker/vwap_tracker.cpp)
target_include_directories(vwap_tracker PUBLIC include)


# --- Main Application ---
add_executable(main.out src/main.cpp)
target_link_libraries(main.out 
    PRIVATE 
    orderbook 
    vwap_tracker
)


# --- Test Suite ---
enable_testing()

file(GLOB_RECURSE TEST_SOURCES "tests/*.cpp")
if(TEST_SOURCES)
    add_executable(run_tests ${TEST_SOURCES})
    target_link_libraries(run_tests PRIVATE 
        gtest_main
        orderbook
        vwap_tracker
	Threads::Threads
    )

    # register tests for ctest
    include(GoogleTest)
    gtest_discover_tests(run_tests)
else()
    message(STATUS "No test files found in 'tests/' directory.")
endif()
