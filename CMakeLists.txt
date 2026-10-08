cmake_minimum_required(VERSION 3.16)
project(vehicle_driver_assistance C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

add_library(vehicle_assist src/vehicle_assist.c)
target_include_directories(vehicle_assist PUBLIC include)

add_executable(vehicle_assist_tests tests/test_vehicle_assist.c)
target_link_libraries(vehicle_assist_tests PRIVATE vehicle_assist)

enable_testing()
add_test(NAME vehicle_assist_tests COMMAND vehicle_assist_tests)
