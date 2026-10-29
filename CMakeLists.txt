cmake_minimum_required(VERSION 3.15)
project(DisasterRescueSystem VERSION 1.0.0 LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Include directories
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/third_party
)

# SQLite3 Amalgamation Library
add_library(sqlite3 STATIC
    third_party/sqlite3.c
)

# Main Web Backend Server
add_executable(disaster_server
    main.cpp
)
target_link_libraries(disaster_server PRIVATE sqlite3)
if(WIN32)
    target_link_libraries(disaster_server PRIVATE ws2_32)
endif()

# Core Data Structures Test Suite Executable
add_executable(test_structures
    tests/test_structures.cpp
)
if(WIN32)
    target_link_libraries(test_structures PRIVATE ws2_32)
endif()

# Database Service Test Suite Executable
add_executable(test_database
    tests/test_database.cpp
)
target_link_libraries(test_database PRIVATE sqlite3)
if(WIN32)
    target_link_libraries(test_database PRIVATE ws2_32)
endif()

enable_testing()
add_test(NAME CoreDataStructuresTest COMMAND test_structures)
set_tests_properties(CoreDataStructuresTest PROPERTIES WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}")

add_test(NAME DatabaseServiceTest COMMAND test_database)
set_tests_properties(DatabaseServiceTest PROPERTIES WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}")
