cmake_minimum_required (VERSION 2.6)

link_directories($ENV{BOOST}lib)

project (db)

FILE(GLOB SRC_ALGO_FILES
../../algo/*.cpp
../../algo/*.h
../../algo/*.hpp
)

FILE(GLOB SRC_DB_FILES
../*.cpp
../*.h
../*.hpp
)

FILE(GLOB SRC_EXTERN_FILES
../../extern/*.cpp
../../extern/*.h
../../extern/*.hpp
)


SOURCE_GROUP(algo FILES ${SRC_ALGO_FILES} )
SOURCE_GROUP(db FILES ${SRC_DB_FILES} )
SOURCE_GROUP(extern FILES ${SRC_EXTERN_FILES} )


add_executable(db ${SRC_ALGO_FILES} ${SRC_DB_FILES} ${SRC_EXTERN_FILES})

add_definitions(-D_CRT_SECURE_NO_WARNINGS)
include_directories($ENV{BOOST})

if(MSVC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP")
endif()
