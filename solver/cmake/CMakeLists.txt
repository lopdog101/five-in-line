cmake_minimum_required (VERSION 2.6)

if( WIN32)
    link_directories($ENV{BOOST}/stage/lib)
endif()

project (solver)

FILE(GLOB SRC_ALGO_FILES
../../algo/*.cpp
../../algo/*.h
../../algo/*.hpp
)

FILE(GLOB SRC_SOLVER_FILES
../*.cpp
../*.h
../*.hpp
)

FILE(GLOB SRC_DB_FILES
../../db/bin_index_utils.cpp
../../db/bin_index_utils.h
../../db/solution_tree_utils.cpp
../../db/solution_tree_utils.h
)

FILE(GLOB SRC_EXTERN_FILES
../../extern/*.cpp
../../extern/*.h
../../extern/*.hpp
)


SOURCE_GROUP(algo FILES ${SRC_ALGO_FILES} )
SOURCE_GROUP(solver FILES ${SRC_SOLVER_FILES} )
SOURCE_GROUP(db FILES ${SRC_DB_FILES} )
SOURCE_GROUP(extern FILES ${SRC_EXTERN_FILES} )


add_executable(solver ${SRC_ALGO_FILES} ${SRC_SOLVER_FILES} ${SRC_DB_FILES} ${SRC_EXTERN_FILES})

add_definitions(-D_CRT_SECURE_NO_WARNINGS)

if(NOT WIN32)
      TARGET_LINK_LIBRARIES(solver boost_filesystem.a boost_signals.a boost_system.a)
endif()

link_directories($ENV{BOOST}/stage/lib)
if( MSVC)
    include_directories($ENV{BOOST})
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP")
endif()
