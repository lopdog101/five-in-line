cmake_minimum_required (VERSION 2.6)

if( WIN32)
    link_directories($ENV{BOOST}/stage/lib)
    link_directories(/distr/mysql-win32/lib)
endif()

project (five_in_line_tests)

FILE(GLOB SRC_ALGO_FILES
../../algo/*.cpp
../../algo/*.h
../../algo/*.hpp
)

FILE(GLOB SRC_DB_FILES
../../db/bin_index.*
../../db/bin_index_utils.*
../../db/bin_index_solution_base.*
../../db/solution_tree.*
../../db/solution_tree_utils.*
../../db/regular_file_access.*
../../db/paged_file_access.*
)

FILE(GLOB SRC_EXTERN_FILES
../../extern/*.cpp
../../extern/*.h
../../extern/*.hpp
)

FILE(GLOB SRC_TESTS_FILES
../*.cpp
../*.h
../*.hpp
)

FILE(GLOB SRC_GTEST_FILES
../gtest/*.cc
../gtest/*.h
)

SOURCE_GROUP(tests FILES ${SRC_TESTS_FILES} )
SOURCE_GROUP(algo FILES ${SRC_ALGO_FILES} )
SOURCE_GROUP(db FILES ${SRC_DB_FILES} )
SOURCE_GROUP(extern FILES ${SRC_EXTERN_FILES} )
SOURCE_GROUP(gtest FILES ${SRC_GTEST_FILES} )


add_executable(five_in_line_tests ${SRC_ALGO_FILES} ${SRC_DB_FILES} ${SRC_EXTERN_FILES} ${SRC_TESTS_FILES} ${SRC_GTEST_FILES})


if(MSVC)
    TARGET_LINK_LIBRARIES(five_in_line_tests libmysql)
	add_definitions(-D_CRT_SECURE_NO_WARNINGS -D_SCL_SECURE_NO_WARNINGS)
	include_directories($ENV{BOOST})
	include_directories(/distr/mysql-win32/include)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP")
endif()
