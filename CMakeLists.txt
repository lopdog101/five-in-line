cmake_minimum_required (VERSION 2.6)

project(five_in_line)

add_subdirectory (db/cmake)
add_subdirectory (solver/cmake)

if(WIN32)
add_subdirectory (gui/cmake)
add_subdirectory (tests/cmake)
endif()
